#pragma once

#include "GLVisitorGPUTrace.hpp"

class GLVisitorGPUTraceSSD : public GLVisitorGPUTrace
{
public:

    virtual void	buildTree();

    GLVisitorGPUTraceSSD();
    virtual ~GLVisitorGPUTraceSSD();
    virtual void launchTraceIterations();
private:
    HANDLE m_fileHandle;
    FILE * m_pFile;
    void * m_pKdtreeBuffer;
    LeafExtData * m_pLeafExtDataBuffer;
    HANDLE m_event;
    DWORD m_leafCount;
    KdFileHeader m_header;

};

