//////////////////////////////////////////////////////////////////
// GLVisitorGPUTrace.hpp										//
// Hulk renderer - Create by Tzachi Cohen 2013					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////

#ifndef HULK_GL_VISITOR_GPU_TRACE_HPP
#define HULK_GL_VISITOR_GPU_TRACE_HPP

#include "GLVisitorCPUTrace.hpp"
#include "KDTreeCPU.hpp"
#include "CL/cl.h"
#include "RingBuffer.h"
#include <queue>
//#include <hash_map>
//! @brief This class performs ray tracing using the GPU and
//! draws the frame buffer using OpenGL.
class GLVisitorGPUTrace : public GLVisitorCPUTrace
{
public:
	GLVisitorGPUTrace();
	virtual ~GLVisitorGPUTrace();
	
	// Do nothing, we render directly to the GL texture
	virtual void initializeVisitor();
	virtual bool saveFrameBufferToFile();
	virtual bool traverseBegin(Node *);
	virtual void traverseEnd(Node *);
protected:
	static const size_t c_workgroupSizes [2];
    static const uint c_ObjectPerLeafMult = 32;
    bool			launchKernel(cl_kernel & kernel);
	virtual void	traceScene();
	virtual void	buildTree();
	bool			initializeKernel(const std::wstring & fileName,
									const std::string & kernelName,
									cl_program & program, 
									cl_kernel & kernel);
	static bool		loadSourceFile(const wchar_t * fileName,std::string & source);
	void			printBuildLog();
    void            saveDataSetToFile();

    struct LeafExtData
    {
        uint indexOffset; //offset to the leaf index list CPU backing store
        uint vertexOffset; //offset to the leaf vertex list CPU backing store
        uint vertxSize; // amount of SPVector structs that are associated with the leaf
        uint indexSize; // amount if TriangleIndices structs that are associated with the leaf
    };

    struct KdFileHeader
    {
        UINT64 headerSize;
        UINT64 KDTreeSize;
        UINT64 KDTreeOffset;
        UINT64 leafExtDataSize;
        UINT64 leafExtDataOffset;
        UINT64 indexPerLeafSize;
        UINT64 indexPerLeafOffset;
        UINT64 vertexPerLeafSize;
        UINT64 vertexPerLeafOffset;
    };

    void buildPerLeafData();

    void realocateResource(cl_mem & buffer, uint expectedSize,cl_mem_flags flags);
    //clears the least recently used leaf from the residency list
    // return type is the amount of memory released (index+vertex);
    uint clearLeaf(LeafBases * pBases);

    virtual void launchTraceIterations();
	/////////////////////////////
	// protected OpenCL members//
	/////////////////////////////
	cl_platform_id	m_platformId;
	cl_device_id	m_deviceId;
	cl_context		m_context;
	cl_program		m_program;
	cl_program		m_setValueProgram;
    cl_program      m_drawProgram;
	cl_kernel		m_traceKernel;
    cl_kernel       m_setValueKernel;
    cl_kernel       m_drawKernel;
	cl_command_queue m_queue;

	//memory objects
	cl_mem m_clIndices;
	cl_mem m_clVertexList;
	cl_mem m_clLights;
	cl_mem m_clFrameBuffer;
	cl_mem m_clGlobalSceneData;
	cl_mem m_clPolygonCache;
    cl_mem m_clHitData;
    cl_mem m_clThreadContext; // this buffer holds the context

	// KDTree related cl Mem Objects
	cl_mem m_clTreeNodes; //holds the tree nodes, CL counterpart of KDTreeCPU::m_treeNodes
	cl_mem m_clLeafBases; //holds the tree polygon list, CL counterpart of KDTreeCPU::m_leafPolygons
    
    RingBuffer m_indexRing;
    RingBuffer m_vertexRing;
    // this array will be used be the CPU side for management
    std::vector<LeafExtData>     m_leafExtData; //leaf to node mapping;
    std::queue<uint>             m_residentLeafs;
    // this array will be read by the GPU.
    std::vector<LeafBases>       m_leafBases;
    std::vector<TriangleIndices> m_indicesByLeaf;
    std::vector<SPVector>        m_verticesByLeaf;

};
    

#endif //HULK_GL_VISITOR_CPU_TRACE_HPP
