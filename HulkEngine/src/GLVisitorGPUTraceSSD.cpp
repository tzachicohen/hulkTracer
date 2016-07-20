
#include "GLVisitorGPUTraceSSD.hpp"
#include "statistics.hpp"
#include "Settings.hpp"
#include "CL\cl_ext.h"

GLVisitorGPUTraceSSD::GLVisitorGPUTraceSSD()
:m_fileHandle(NULL),
m_pKdtreeBuffer(NULL),
m_pLeafExtDataBuffer(NULL),
m_leafCount(0)
{
    m_event = CreateEvent(NULL, FALSE, FALSE, NULL);
}


GLVisitorGPUTraceSSD::~GLVisitorGPUTraceSSD()
{
    if (m_pKdtreeBuffer)
    {
        free(m_pKdtreeBuffer);
        m_pKdtreeBuffer = NULL;
    }
    if (m_pKdtreeBuffer)
    {
        free(m_pKdtreeBuffer);
        m_pKdtreeBuffer = NULL;
    }
    if (m_fileHandle)
    {
        CloseHandle(m_fileHandle);
        m_fileHandle = NULL;
    }
    
    if (m_pFile)
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }
}

//to do move to initialize
void GLVisitorGPUTraceSSD::buildTree()
{
    static int firstTime = 1;
    if (firstTime)
    {
        firstTime = 0;
    }
    else
    {
        return;
    }

    m_pFile = fopen("paris.bin", "rb");
    
    fread(&m_header, sizeof(m_header), 1, m_pFile);

    m_pKdtreeBuffer = malloc(m_header.KDTreeSize);
    if (NULL != m_pKdtreeBuffer)
    {
        fseek(m_pFile, m_header.KDTreeOffset, SEEK_SET);
        fread(m_pKdtreeBuffer, m_header.KDTreeSize, 1, m_pFile);
    }
    m_pLeafExtDataBuffer = (LeafExtData *) malloc(m_header.leafExtDataSize);
    if (NULL != m_pLeafExtDataBuffer)
    {
        fseek(m_pFile, m_header.leafExtDataOffset, SEEK_SET);
        fread(m_pLeafExtDataBuffer, m_header.leafExtDataSize, 1, m_pFile);
    }
    fclose(m_pFile);
    m_pFile = NULL;

    m_fileHandle = CreateFile(L"paris.bin",
        GENERIC_READ, ///! Desired access
        FILE_SHARE_READ, ///! ShareMode allows others to read concurrently
        NULL, ///! security attributes
        OPEN_EXISTING, ///! open the file only if it exists.
        FILE_FLAG_NO_BUFFERING ///! do not buffer file content
        //| FILE_FLAG_OVERLAPPED ///! asynchronous IO
        , NULL); //optional template file


    cl_mem_flags additionalFlag = 0;

    if (0 == wcscmp(Settings::getInstance().getValue(L"--p2p"), L"1"))
    {
        additionalFlag = CL_MEM_USE_PERSISTENT_MEM_AMD;
    }

    m_leafCount = m_header.leafExtDataSize / sizeof(LeafExtData);
    realocateResource(m_clLeafBases, m_leafCount*sizeof(LeafBases),CL_MEM_READ_WRITE);

    realocateResource(m_clVertexList, m_header.vertexPerLeafSize,CL_MEM_READ_ONLY | additionalFlag);

    realocateResource(m_clIndices, m_header.indexPerLeafSize, CL_MEM_READ_ONLY | additionalFlag);

    realocateResource(m_clTreeNodes, m_header.KDTreeSize,CL_MEM_READ_ONLY);

    //update the CL mem buffers with the data
    cl_int status = clEnqueueWriteBuffer(m_queue, m_clTreeNodes, CL_FALSE, 0, m_header.KDTreeSize, m_pKdtreeBuffer, 0, NULL, NULL);
    HULK_ASSERT(CL_SUCCESS == status, "clEnqueueWriteBuffer failed with status %d\n", status);

    m_indexRing.reset(m_header.indexPerLeafSize / sizeof(TriangleIndices));
    m_vertexRing.reset(m_header.vertexPerLeafSize / sizeof(SPVector));

}

void GLVisitorGPUTraceSSD::launchTraceIterations()
{
    cl_int status;
    bool missingLeafFound = false;
    int iterationCount = 0;
    int readCount = 0;
    int transferSize =0 ;
	static unsigned int frame = 0;
    do
    {
        iterationCount++;
        missingLeafFound = false;
        launchKernel(m_traceKernel);
        LeafBases * pBases = (LeafBases*)clEnqueueMapBuffer(m_queue,
            m_clLeafBases,
            CL_TRUE, CL_MAP_READ | CL_MAP_WRITE,
            0,
            m_leafCount*sizeof(LeafBases),
            0, NULL, NULL, &status);

        SPVector * pVertices = (SPVector*)clEnqueueMapBuffer(m_queue,
            m_clVertexList,
            CL_TRUE, CL_MAP_READ | CL_MAP_WRITE,
            0,
            m_header.vertexPerLeafSize,
            0, NULL, NULL, &status);
        TriangleIndices * pIndices = (TriangleIndices*)clEnqueueMapBuffer(m_queue,
            m_clIndices,
            CL_TRUE, CL_MAP_READ | CL_MAP_WRITE,
            0,
            m_header.indexPerLeafSize,
            0, NULL, NULL, &status);
        uint clearedMemory = 0;
        uint maxMemoryToClear = (m_vertexRing.getBufferSize() + m_indexRing.getBufferSize()) / 2;
        for (int i = 0; i < m_leafCount; i++)
        {
            if (pBases[i].m_residency == 1) // residency requested
            {
                //todo: learn to evac resource if we do not have space
                missingLeafFound = true;
                uint indexSize = m_pLeafExtDataBuffer[i].indexSize;
                uint vertexSize = m_pLeafExtDataBuffer[i].vertxSize;
                uint indexOffset = 0; //offset to be placed in the GPU buffer
                uint vertexOffset = 0;
                while (!m_indexRing.contiguousInsert(indexSize, indexOffset) &&
                    clearedMemory <= maxMemoryToClear)
                {//clear least recently used leafs;
                    clearedMemory += clearLeaf(pBases);
                }
                while (!m_vertexRing.contiguousInsert(vertexSize, vertexOffset) &&
                    clearedMemory <= maxMemoryToClear)
                {//clear least recently used leafs;
                    clearedMemory += clearLeaf(pBases);
                }
                if (clearedMemory > maxMemoryToClear)
                {
                    break; //we do not clear more than half of the occupied buffer
                }
                // copy vertex data
                
                OVERLAPPED overlapped = { 0 };
                DWORD bytesRead;
                BOOL retVal;
                overlapped.Offset = m_header.vertexPerLeafOffset + (m_pLeafExtDataBuffer[i].vertexOffset*sizeof(SPVector));
                //stream the index data from disk to the GPU
                retVal = ReadFile(m_fileHandle, pVertices + vertexOffset, vertexSize*sizeof(SPVector), &bytesRead, &overlapped);
				HULK_PRINT("ReadFile size %u, offset %u\n", bytesRead, pVertices + vertexOffset);
                readCount++;
                transferSize += bytesRead;
                //stream vertex data from disk to GPU
                memset(&overlapped, 0, sizeof(overlapped));
                overlapped.Offset = m_header.indexPerLeafOffset + (m_pLeafExtDataBuffer[i].indexOffset*sizeof(TriangleIndices));

                retVal = ReadFile(m_fileHandle, pIndices + indexOffset, indexSize*sizeof(TriangleIndices), &bytesRead, &overlapped);
				HULK_PRINT("ReadFile size %u, offset %u\n", bytesRead, pIndices + indexOffset);
                readCount++;
                transferSize += bytesRead;
                m_residentLeafs.push(i);
                pBases[i].m_residency = 2;
                pBases[i].m_vertexBase = vertexOffset;
                pBases[i].m_indexBase = indexOffset;
            }
        }

        clEnqueueUnmapMemObject(m_queue, m_clLeafBases, pBases, 0, NULL, NULL);
        clEnqueueUnmapMemObject(m_queue, m_clVertexList, pVertices, 0, NULL, NULL);
        clEnqueueUnmapMemObject(m_queue, m_clIndices, pIndices, 0, NULL, NULL);

    } while (missingLeafFound);
	HULK_PRINT("Frame #%u iterations %u  read count %u , transfer size %u bytes ",frame++, iterationCount, readCount,transferSize)
    HULK_PRINT(",Average Transfer size for frame:%u bytes\n", transferSize / readCount)

    Statistics::BackingStoreStats stats;
    stats.m_indexBufferUtilization = ((float)(m_indexRing.getOccuppiedSpace())) / (m_header.indexPerLeafSize / sizeof(TriangleIndices));
    stats.m_vertexBufferUtilization = ((float)(m_vertexRing.getOccuppiedSpace())) / (m_header.vertexPerLeafSize / sizeof(SPVector));
    Statistics::getInstance().registerBackingStoreStat(stats);

}
