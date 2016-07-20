
//file close and open headers
#include <cstring>
#include <cstdlib>
#include <fstream>

#include <windows.h>
#include "glut.h"
#include "Settings.hpp"

#include "CL/cl_gl.h"

#include "GLVisitorGPUTrace.hpp"
#include "KDTreeGPU.hpp"
#include "Settings.hpp"
#include "Statistics.hpp"

const size_t GLVisitorGPUTrace::c_workgroupSizes []= {8,8};

GLVisitorGPUTrace::GLVisitorGPUTrace()
:m_deviceId(0),
m_platformId(0),
m_context(0),
m_program(0),
m_setValueProgram(0),
m_traceKernel(0),
m_setValueKernel(0),
m_queue(0),
m_clIndices(0),
m_clVertexList(0),
m_clLights(0),
m_clFrameBuffer(0),
m_clGlobalSceneData(0),
m_clTreeNodes(0),
m_clLeafBases(0),
m_clPolygonCache(0)
{
}

GLVisitorGPUTrace::~GLVisitorGPUTrace()
{
	clReleaseMemObject(m_clIndices);
	clReleaseMemObject(m_clVertexList);
	clReleaseMemObject(m_clLights);
	clReleaseMemObject(m_clGlobalSceneData);
	clReleaseMemObject(m_clFrameBuffer);
	clReleaseMemObject(m_clPolygonCache);
	clReleaseMemObject(m_clLeafBases);
	clReleaseMemObject(m_clTreeNodes);
    clReleaseMemObject(m_clHitData);
    clReleaseMemObject(m_clThreadContext);

	clReleaseKernel(m_traceKernel);
    clReleaseKernel(m_setValueKernel);
    clReleaseKernel(m_drawKernel);
	clReleaseProgram(m_program);
    clReleaseProgram(m_setValueProgram);
    clReleaseProgram(m_drawProgram);
	clReleaseCommandQueue(m_queue);
	clReleaseDevice(m_deviceId);
	clReleaseContext(m_context);
}

bool GLVisitorGPUTrace::initializeKernel(const std::wstring & fileName, 
										const std::string & kernelName,
										cl_program & program , 
										cl_kernel& kernel)
{
	std::string source;
	bool retVal = loadSourceFile(fileName.c_str(),source);
	HULK_ASSERT(0 != retVal,"loadSourceFile failed \n")
	cl_int status;

	const char * pSource = source.c_str();
	size_t SourceSize = source.size();
	program = clCreateProgramWithSource(m_context,1,&pSource,&SourceSize,&status);
	HULK_ASSERT(0 != program,"clCreateProgramWithSource failed with status %d\n",status)

	std::string compileFlags = "-D OPENCL_BUILD -x clc++";
#ifdef _DEBUG
	compileFlags += " -D DEBUG ";
#endif
	status = clBuildProgram(program,1,&m_deviceId,compileFlags.c_str(),NULL,NULL);
	if (CL_BUILD_PROGRAM_FAILURE == status) {
		printBuildLog();
		return false;
	}
	
	kernel = clCreateKernel(program,kernelName.c_str(),&status);
	HULK_ASSERT(0 != kernel,"clCreateKernel failed with status %d\n",status)

	return true;
}
void GLVisitorGPUTrace::initializeVisitor()
{
	HULK_ASSERT(NULL == m_pKdtree ,"m_pKdtree not null when initialized\n");
	m_pKdtree = new KDTreeCPU();

	//initialize openCL
	cl_uint numPlatforms;
	cl_int status;
	status = clGetPlatformIDs(1,&m_platformId,&numPlatforms);
	HULK_ASSERT(CL_SUCCESS == status ,"clGetPlatformIDs failed with status %d\n",status);

	const size_t c_stringLength= 256;
	char platformName[c_stringLength];
	size_t returnedSize;
	status = clGetPlatformInfo(m_platformId,CL_PLATFORM_NAME,c_stringLength,platformName,&returnedSize);
	HULK_ASSERT(CL_SUCCESS == status ,"clGetPlatformInfo failed with status %d\n",status);
	printf("selected platform: %s\n",platformName);
	
	cl_uint numDevices;
	status = clGetDeviceIDs(m_platformId,CL_DEVICE_TYPE_GPU,1,&m_deviceId,&numDevices);
	HULK_ASSERT(CL_SUCCESS == status ,"clGetDeviceIDs failed with status %d\n",status);

	HGLRC hRC = wglGetCurrentContext();
	HDC hDC = wglGetCurrentDC();

	cl_context_properties properties[] = 
    {
        CL_CONTEXT_PLATFORM, (cl_context_properties) m_platformId,
        CL_GL_CONTEXT_KHR,   (cl_context_properties) hRC,
        CL_WGL_HDC_KHR,      (cl_context_properties) hDC,
        0
    };
	m_context =clCreateContext(properties,1,&m_deviceId,NULL,NULL,&status);
	HULK_ASSERT(0 != m_context,"clCreateContext failed with status %d\n",status)

	//create the interop resources only after the CL context is created
	GLVisitorCPUTrace::initializeVisitor();

	m_queue = clCreateCommandQueue(m_context,m_deviceId,NULL,&status);
	HULK_ASSERT(0 != m_queue,"clCreateCommandQueue failed with status %d\n",status)
	
	std::wstring sourceName = Settings::getInstance().getValue(L"--kernelType");
	sourceName += L"_full.cl";

	bool success = initializeKernel(sourceName,"gpu_trace",m_program,m_traceKernel);

	success = initializeKernel(L"GPU_setValues_full.cl","SetInitialValues",m_setValueProgram,m_setValueKernel);

    success = initializeKernel(L"ShadePicture_full.cl", "ShadePicture", m_drawProgram, m_drawKernel);
	////////////////////////////////////////////////////////
	//create the various memory objects
	///////////////////////////////////////////////////////////
	// acquire the frame buffer from OGL
	m_clFrameBuffer = clCreateFromGLTexture(m_context,CL_MEM_WRITE_ONLY,GL_TEXTURE_2D,0,m_texture,&status);
	HULK_ASSERT(0 != m_clFrameBuffer,"clCreateFromGLTexture failed with status %d\n",status)

    m_clHitData = clCreateBuffer(m_context, CL_MEM_READ_WRITE, sizeof(float)*4*m_pScene->m_resx* m_pScene->m_resy, NULL, &status);
    HULK_ASSERT(0 != m_clHitData, "clCreateBuffer failed with status %d\n", status)

    m_clThreadContext = clCreateBuffer(m_context, CL_MEM_READ_WRITE, sizeof (ThreadContext)* m_pScene->m_resx* m_pScene->m_resy, NULL, &status);
    HULK_ASSERT(0 != m_clThreadContext, "clCreateBuffer failed with status %d\n", status)

	m_clLights = clCreateBuffer(m_context,CL_MEM_READ_ONLY,m_pScene->m_lights.size() * sizeof(Light),NULL,&status);
	HULK_ASSERT(0 != m_clLights,"clCreateBuffer failed with status %d\n",status)

	m_clGlobalSceneData = clCreateBuffer(m_context,CL_MEM_READ_ONLY,sizeof(CameraPos),NULL,&status);
	HULK_ASSERT(0 != m_clGlobalSceneData,"clCreateBuffer failed with status %d\n",status)
}

void GLVisitorGPUTrace::buildPerLeafData()
{
    m_indicesByLeaf.clear();
    m_verticesByLeaf.clear();
    m_leafBases.clear();
    m_leafExtData.clear();
    //run over the tree to find the leafs
    for (Stack<KDTreeNode>::iterator it = m_pKdtree->m_treeNodes.begin(); it != m_pKdtree->m_treeNodes.end(); ++it)
    {
        //this map holds which vertices have already been places in the node
        //kay is the original vertex index and value is the index in the leaf structure
        std::map<uint, uint> m_vertexMap;
        if (EPlaneAxis::e_leaf == it->m_flags)
        {
            unsigned int verticesForthisNode = 0;
            m_vertexMap.clear();
           
            LeafExtData leafExtData;
            leafExtData.indexOffset = m_indicesByLeaf.size() ;
            leafExtData.vertexOffset = m_verticesByLeaf.size();

            LeafBases bases = { 0 };
            m_leafBases.push_back(bases);
            
            //iterate over all polygons
            for (uint i = 0; i < it->data.leaf.m_polygonCount; i++)
            {
                TriangleIndices originalIndices = m_pScene->m_vertexIndices[m_pKdtree->m_leafPolygons[it->data.leaf.m_startIndex + i]];
                TriangleIndices newIndices;
                for (uint v = 0; v < 3; v++)
                {
                    //if this vertex is already in the node local data
                    if (m_vertexMap.find(originalIndices.indices[v]) != m_vertexMap.end())
                    {
                        newIndices.indices[v] = m_vertexMap[originalIndices.indices[v]];
                    }
                    else
                    {
                        newIndices.indices[v] = verticesForthisNode++;
                        m_vertexMap[originalIndices.indices[v]] = newIndices.indices[v];
                        m_verticesByLeaf.push_back(m_pScene->m_vertices[originalIndices.indices[v]]);
                    }
                }
                m_indicesByLeaf.push_back(newIndices);
            }
            leafExtData.indexSize = m_indicesByLeaf.size() - leafExtData.indexOffset;
            leafExtData.vertxSize = m_verticesByLeaf.size() - leafExtData.vertexOffset;
            //making sure that each leaf index/vertex buffers are 512 byte aligned
            uint roundedIndexSize = (leafExtData.indexSize + (c_ObjectPerLeafMult - 1)) & ~(c_ObjectPerLeafMult - 1);
            uint indexDiff = roundedIndexSize - leafExtData.indexSize;
            for (int i = 0; i < indexDiff; i++)
            {
                TriangleIndices emptyIndices;
                m_indicesByLeaf.push_back(emptyIndices);
            }
            uint roundedVertexSize = (leafExtData.vertxSize + (c_ObjectPerLeafMult - 1)) & ~(c_ObjectPerLeafMult - 1);
            uint vertexDiff = roundedVertexSize - leafExtData.vertxSize;
            for (int i = 0; i < vertexDiff; i++)
            {
                SPVector emptyVector;
                m_verticesByLeaf.push_back(emptyVector);
            }
            leafExtData.indexSize = m_indicesByLeaf.size() - leafExtData.indexOffset;
            leafExtData.vertxSize = m_verticesByLeaf.size() - leafExtData.vertexOffset;
            m_leafExtData.push_back(leafExtData);
        }
    }
    printf("index buffer by leaf %u bytes, vertex buffer by leaf %u bytes\n", m_indicesByLeaf.size()*sizeof(TriangleIndices),
                                                                        m_verticesByLeaf.size()*sizeof(SPVector));
    m_indexRing.reset(m_indicesByLeaf.size());
    m_vertexRing.reset(m_verticesByLeaf.size());
}

void GLVisitorGPUTrace::realocateResource(cl_mem & buffer, uint expectedSize, cl_mem_flags createFlags)
{
    cl_int status;
    // check the size of the current buffers, if they are too small we release them and re-allocate
    if (buffer){
        size_t currentResourceSize = 0;
        size_t returnSize;
        status = clGetMemObjectInfo(buffer, CL_MEM_SIZE, sizeof(currentResourceSize), &currentResourceSize, &returnSize);
        if (currentResourceSize < expectedSize) {
            clReleaseMemObject(buffer);
            buffer = 0;
        }
    }

    if (0 == buffer) {
        buffer = clCreateBuffer(m_context, createFlags, expectedSize, NULL, &status);
        HULK_ASSERT(0 != buffer, "clCreateBuffer failed with status %d\n", status)
    }
}

void GLVisitorGPUTrace::buildTree()
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
	GLVisitorCPUTrace::buildTree();

    buildPerLeafData();

	cl_int status;
	
    realocateResource(m_clLeafBases, m_leafBases.size()*sizeof(LeafBases), CL_MEM_READ_WRITE);

    //update the CL mem buffers with the data
    status = clEnqueueWriteBuffer(m_queue, m_clLeafBases, CL_FALSE, 0, m_leafBases.size()*sizeof(LeafBases), m_leafBases.data(), 0, NULL, NULL);
    HULK_ASSERT(CL_SUCCESS == status, "clEnqueueWriteBuffer failed with status %d\n", status);

    realocateResource(m_clVertexList, m_verticesByLeaf.size() * sizeof(SPVector), CL_MEM_READ_ONLY);

    realocateResource(m_clIndices, m_indicesByLeaf.size() * sizeof(TriangleIndices), CL_MEM_READ_ONLY);

    realocateResource(m_clTreeNodes, sizeof(KDTreeNode) * m_pKdtree->m_treeNodes.size(),CL_MEM_READ_ONLY);

    //update the CL mem buffers with the data
    status = clEnqueueWriteBuffer(m_queue, m_clTreeNodes, CL_FALSE, 0, sizeof(KDTreeNode)* m_pKdtree->m_treeNodes.size(), m_pKdtree->m_treeNodes.data(), 0, NULL, NULL);
    HULK_ASSERT(CL_SUCCESS == status, "clEnqueueWriteBuffer failed with status %d\n", status);

    if (0 == wcscmp(Settings::getInstance().getValue(L"--saveBin"), L"1"))
    {
        saveDataSetToFile();
    }

}

void GLVisitorGPUTrace::traceScene()
{
	//todo : recreate resources if their size changed.
	cl_int status;

	//copy light
	status = clEnqueueWriteBuffer(m_queue,m_clLights,CL_FALSE,0,sizeof(Light)* m_pScene->m_lights.size()
		,m_pScene->m_lights.data(),0,NULL,NULL);
	HULK_ASSERT(CL_SUCCESS == status ,"clEnqueueWriteBuffer failed with status %d\n",status);
	
	//copy sceneData - must synchronize because of camera movement
    
	status = clEnqueueWriteBuffer(m_queue,m_clGlobalSceneData,CL_TRUE,0,sizeof(CameraPos),m_pCamPos,0,NULL,NULL);
	HULK_ASSERT(CL_SUCCESS == status ,"clEnqueueWriteBuffer failed with status %d\n",status);

	cl_int argCount;
	size_t argCountSize;

    //re-acquire ownership of the resource object.
    status = clEnqueueAcquireGLObjects(m_queue, 1, &m_clFrameBuffer, 0, NULL, NULL);
    HULK_ASSERT(CL_SUCCESS == status, "clEnqueueAcquireGLObjects failed with status %d\n", status);

    Statistics::getInstance().frameStart();

    //launch a kernel that sets flt_max values to depth
    clSetKernelArg(m_setValueKernel, 0, sizeof(cl_mem), &m_clFrameBuffer);
    clSetKernelArg(m_setValueKernel, 1, sizeof(cl_mem), &m_clHitData);
    //set initial values for the kernel
    launchKernel(m_setValueKernel);

    //clear thread context 
    unsigned int pattern = 0;
    clEnqueueFillBuffer(m_queue,
        m_clThreadContext,
        &pattern,
        sizeof(pattern),
        0, //offset
        sizeof (ThreadContext)* m_pScene->m_resx* m_pScene->m_resy,  //size
        0,
        NULL,
        NULL);

    //launch trace kernel	
    status = clGetKernelInfo(m_traceKernel,CL_KERNEL_NUM_ARGS,sizeof(cl_int),&argCount,&argCountSize);
	HULK_ASSERT(CL_SUCCESS == status ,"clGetKernelInfo failed with status %d\n",status);

	status = clSetKernelArg(m_traceKernel,0,sizeof(cl_mem),&m_clFrameBuffer);
	HULK_ASSERT(CL_SUCCESS == status ,"clSetKernelArg failed with status %d\n",status);
	
	status = clSetKernelArg(m_traceKernel,1,sizeof(cl_mem),&m_clVertexList);
	HULK_ASSERT(CL_SUCCESS == status ,"clSetKernelArg failed with status %d\n",status);
	
	status = clSetKernelArg(m_traceKernel,2,sizeof(cl_mem),&m_clIndices);
	HULK_ASSERT(CL_SUCCESS == status ,"clSetKernelArg failed with status %d\n",status);
	
	uint triangleCount = m_pScene->m_vertexIndices.size();
	status = clSetKernelArg(m_traceKernel,3,sizeof(uint),& triangleCount);
	HULK_ASSERT(CL_SUCCESS == status ,"clSetKernelArg failed with status %d\n",status);
	
	status = clSetKernelArg(m_traceKernel,4,sizeof(cl_mem),&m_clGlobalSceneData);
	HULK_ASSERT(CL_SUCCESS == status ,"clSetKernelArg failed with status %d\n",status);
	
	status = clSetKernelArg(m_traceKernel,5,sizeof(cl_mem),& m_clLights);
	HULK_ASSERT(CL_SUCCESS == status ,"clSetKernelArg failed with status %d\n",status);

    uint lightCount = m_pScene->m_lights.size();
	status = clSetKernelArg(m_traceKernel,6,sizeof(uint),& lightCount );
	HULK_ASSERT(CL_SUCCESS == status ,"clSetKernelArg failed with status %d\n",status);
	
	status = clSetKernelArg(m_traceKernel,7,sizeof(Box),& (m_pScene->m_sceneBox) );
	HULK_ASSERT(CL_SUCCESS == status ,"clSetKernelArg failed with status %d\n",status);

	status = clSetKernelArg(m_traceKernel,8,sizeof(cl_mem),& m_clTreeNodes );
	HULK_ASSERT(CL_SUCCESS == status ,"clSetKernelArg failed with status %d\n",status);

	status = clSetKernelArg(m_traceKernel,9,sizeof(cl_mem),& m_clLeafBases );
	HULK_ASSERT(CL_SUCCESS == status ,"clSetKernelArg failed with status %d\n",status);

    status = clSetKernelArg(m_traceKernel, 10, sizeof(cl_mem), &m_clHitData);
    HULK_ASSERT(CL_SUCCESS == status, "clSetKernelArg failed with status %d\n", status);

    status = clSetKernelArg(m_traceKernel, 11, sizeof(cl_mem), &m_clThreadContext);
    HULK_ASSERT(CL_SUCCESS == status, "clSetKernelArg failed with status %d\n", status);

    launchTraceIterations();

    ////////////////////////////////////////////////////////////
    //launch the draw kernel

    status = clSetKernelArg(m_drawKernel, 0, sizeof(cl_mem), &m_clFrameBuffer);
    HULK_ASSERT(CL_SUCCESS == status, "clSetKernelArg failed with status %d\n", status);

    status = clSetKernelArg(m_drawKernel, 1, sizeof(cl_mem), &m_clGlobalSceneData);
    HULK_ASSERT(CL_SUCCESS == status, "clSetKernelArg failed with status %d\n", status);

    status = clSetKernelArg(m_drawKernel, 2, sizeof(cl_mem), &m_clLights);
    HULK_ASSERT(CL_SUCCESS == status, "clSetKernelArg failed with status %d\n", status);

    status = clSetKernelArg(m_drawKernel, 3, sizeof(uint), &lightCount);
    HULK_ASSERT(CL_SUCCESS == status, "clSetKernelArg failed with status %d\n", status);

    status = clSetKernelArg(m_drawKernel, 4, sizeof(cl_mem), &m_clHitData);
    HULK_ASSERT(CL_SUCCESS == status, "clSetKernelArg failed with status %d\n", status);

    launchKernel(m_drawKernel);
     
    
    clFinish(m_queue);

    Statistics::getInstance().frameEnd();
    //release hold of the interop object for GL
    status = clEnqueueReleaseGLObjects(m_queue, 1, &m_clFrameBuffer, 0, NULL, NULL);
    HULK_ASSERT(CL_SUCCESS == status, "clEnqueueReleaseGLObjects failed with status %d\n", status);

}

void GLVisitorGPUTrace::launchTraceIterations()
{
    cl_int status;
    bool missingLeafFound = false;
    int count = 0;
	int copyCounts = 0 ;
	int CopySize= 0 ;
    do
    {
        missingLeafFound = false;
        launchKernel(m_traceKernel);
        LeafBases * pBases = (LeafBases*)clEnqueueMapBuffer(m_queue,
            m_clLeafBases,
            CL_TRUE, CL_MAP_READ | CL_MAP_WRITE,
            0,
            m_leafBases.size()*sizeof(LeafBases),
            0, NULL, NULL, &status);

        SPVector * pVertices = (SPVector*)clEnqueueMapBuffer(m_queue,
            m_clVertexList,
            CL_TRUE, CL_MAP_READ | CL_MAP_WRITE,
            0,
            m_verticesByLeaf.size() * sizeof(SPVector),
            0, NULL, NULL, &status);
        TriangleIndices * pIndices = (TriangleIndices*)clEnqueueMapBuffer(m_queue,
            m_clIndices,
            CL_TRUE, CL_MAP_READ | CL_MAP_WRITE,
            0,
            m_indicesByLeaf.size() * sizeof(TriangleIndices),
            0, NULL, NULL, &status);
        uint clearedMemory = 0;
        uint maxMemoryToClear = (m_vertexRing.getBufferSize() + m_indexRing.getBufferSize()) / 2;
        for (int i = 0; i < m_leafBases.size(); i++)
        {
            if (pBases[i].m_residency == 1) // residency requested
            {
                //todo: learn to evac resource if we do not have space
                missingLeafFound = true;
                uint indexSize = m_leafExtData[i].indexSize;
                uint vertexSize = m_leafExtData[i].vertxSize;
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
                memcpy(pVertices + vertexOffset, &(m_verticesByLeaf[m_leafExtData[i].vertexOffset]), vertexSize*sizeof(SPVector));
                memcpy(pIndices + indexOffset, &(m_indicesByLeaf[m_leafExtData[i].indexOffset]), indexSize*sizeof(TriangleIndices));
                m_residentLeafs.push(i);
                pBases[i].m_residency = 2;
                pBases[i].m_vertexBase = vertexOffset;
                pBases[i].m_indexBase = indexOffset;
            }
        }

        clEnqueueUnmapMemObject(m_queue, m_clLeafBases, pBases, 0, NULL, NULL);
        clEnqueueUnmapMemObject(m_queue, m_clVertexList, pVertices, 0, NULL, NULL);
        clEnqueueUnmapMemObject(m_queue, m_clIndices, pIndices, 0, NULL, NULL);
        count++;
    } while (missingLeafFound);

    Statistics::BackingStoreStats stats;
    stats.m_indexBufferUtilization = ((float)(m_indexRing.getOccuppiedSpace())) / m_indicesByLeaf.size();
    stats.m_vertexBufferUtilization = ((float)(m_vertexRing.getOccuppiedSpace())) / m_verticesByLeaf.size();
    Statistics::getInstance().registerBackingStoreStat(stats);

}

uint GLVisitorGPUTrace::clearLeaf(LeafBases * pBases)
{
    uint leafID = m_residentLeafs.front();
    if (m_residentLeafs.size() > 0)
    {
        m_vertexRing.free(m_leafExtData[leafID].vertxSize);
        m_indexRing.free(m_leafExtData[leafID].indexSize);
        pBases[leafID].m_residency = 0;
    }
    m_residentLeafs.pop();
    return m_leafExtData[leafID].vertxSize + m_leafExtData[leafID].indexSize;
}
// this function breaks a kernel to several 
//  invokations in case ND range size if bigger than
// the maximum the device support.
bool GLVisitorGPUTrace::launchKernel(cl_kernel & kernel)
{
	
	size_t range [3];
	

	range[0] = alignUp(m_pScene->m_resx,c_workgroupSizes[0]);
	range[1] = alignUp(m_pScene->m_resy,c_workgroupSizes[1]);
	//launch the trace kernel
    cl_int status = clEnqueueNDRangeKernel(m_queue, kernel, 2, 0, range, c_workgroupSizes, 0, NULL, NULL);

	HULK_ASSERT(CL_SUCCESS == status ,"clEnqueueNDRangeKernel failed with status %d\n",status);

	return true;
}

void GLVisitorGPUTrace::printBuildLog()
{
	 
        cl_int logStatus;
        char *buildLog = NULL;
        size_t buildLogSize = 0;
        logStatus = clGetProgramBuildInfo (
                        m_program, 
                        m_deviceId, 
                        CL_PROGRAM_BUILD_LOG, 
                        buildLogSize, 
                        buildLog, 
                        &buildLogSize);
        HULK_ASSERT(logStatus, "clGetProgramBuildInfo failed.");

        buildLog = (char*)malloc(buildLogSize);
        HULK_ASSERT(buildLog, "Failed to allocate host memory. (buildLog)");

        memset(buildLog, 0, buildLogSize);

        logStatus = clGetProgramBuildInfo (
                        m_program, 
                        m_deviceId, 
                        CL_PROGRAM_BUILD_LOG, 
                        buildLogSize, 
                        buildLog, 
                        NULL);
		HULK_ASSERT(logStatus, "clGetProgramBuildInfo failed.");
		printf(buildLog);
		free(buildLog);
}

bool GLVisitorGPUTrace::loadSourceFile(
    const wchar_t * fileName,
	std::string & source)   //!< file name
{
    size_t      size;
    char*       str;
    // Open file stream
    std::fstream f(fileName, (std::fstream::in | std::fstream::binary));

    // Check if we have opened file stream
    if (f.is_open()) {
        size_t  sizeFile;
        // Find the stream size
        f.seekg(0, std::fstream::end);
        size = sizeFile = (size_t)f.tellg();
        f.seekg(0, std::fstream::beg);

        str = new char[size + 1];
        if (!str) {
            f.close();
            return  false;
        }

        // Read file
        f.read(str, sizeFile);
        f.close();
        str[size] = '\0';

        source  = str;

        delete[] str;

        return true;
    }

    return false;
}

bool GLVisitorGPUTrace::saveFrameBufferToFile()
{
	glReadPixels(0,0,m_pScene->m_resx,m_pScene->m_resy,GL_RGBA, GL_UNSIGNED_BYTE,m_pScene->m_frameBuffer.getBufferBits());
	return  GLVisitorCPUTrace::saveFrameBufferToFile();
}

bool GLVisitorGPUTrace::traverseBegin(Node *)
{
	
	buildTree();

	glBegin(GL_QUADS);

		glTexCoord2f(0,0);
		glVertex2f( -1, -1 );

		glTexCoord2f(0.0f,1.0f);
		glVertex2f( -1, 1.0f );

		glTexCoord2f(1.0f,1.0f);
		glVertex2f( 1.0f, 1.0f );

		glTexCoord2f(1,0);
		glVertex2f( 1.0f, -1 );
	
	glEnd( );

    glutSwapBuffers();
    glFinish();

	updateCamera();

	traceScene();

	if (0 == wcscmp(Settings::getInstance().getValue(L"--saveImages"),L"1")) {
		saveFrameBufferToFile();
	}
	//no need to traverse the graph.
	return false;
}

void GLVisitorGPUTrace::traverseEnd(Node *)
{
	
}

void GLVisitorGPUTrace::saveDataSetToFile()
{ 
    FILE * f = fopen("paris.bin", "wb");
    KdFileHeader header;
    header.headerSize = sizeof(KdFileHeader);
    //dump KD tree
    header.KDTreeOffset = header.headerSize;
    header.KDTreeSize = sizeof(KDTreeNode)*  m_pKdtree->m_treeNodes.size();

    //write empty header
    fwrite(&header, sizeof(header), 1, f);
    
    //write kd tree;
    fwrite(m_pKdtree->m_treeNodes.data(), header.KDTreeSize, 1, f);
    header.leafExtDataOffset = ftell(f);
    header.leafExtDataSize = m_leafExtData.size()*sizeof(LeafExtData);
    fwrite(m_leafExtData.data(), header.leafExtDataSize, 1, f);
    // per leaf index and buffer data must be 512 byte aligned.
    uint currentOffset = ftell(f);
    uint alignedoffset = currentOffset + (512 - 1) & ~(512 - 1);
    uint diff = alignedoffset - currentOffset;
    // advance the pointer
    fseek(f, diff, SEEK_CUR);

    header.indexPerLeafOffset = alignedoffset;
    header.indexPerLeafSize = m_indicesByLeaf.size() * sizeof(TriangleIndices);
    fwrite(m_indicesByLeaf.data(), header.indexPerLeafSize, 1, f);

    header.vertexPerLeafOffset = ftell(f);
    header.vertexPerLeafSize = m_verticesByLeaf.size() * sizeof(SPVector);
    fwrite(m_verticesByLeaf.data(), header.vertexPerLeafSize, 1, f);

    //go back to the beginning of the file and write the header
    fseek(f, 0, SEEK_SET);
    fwrite(&header, sizeof(header), 1, f);
    fclose(f);
}
