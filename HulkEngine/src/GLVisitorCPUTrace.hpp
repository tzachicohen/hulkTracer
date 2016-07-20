//////////////////////////////////////////////////////////////////
// GLVisitorCPUTrace.hpp										//
// Hulk renderer - Create by Tzachi Cohen 2013					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////

#ifndef HULK_GL_VISITOR_CPU_TRACE_HPP
#define HULK_GL_VISITOR_CPU_TRACE_HPP

#include "GLVisitorT.hpp"
#include "KDTreeCPU.hpp"

//! @brief This class performs ray tracing using the CPU and
//! draws the frame buffer using OpenGL.
class GLVisitorCPUTrace : public GLVisitorT
{
public:
	GLVisitorCPUTrace();
	virtual ~GLVisitorCPUTrace();
	virtual void initializeVisitor();
protected:
	virtual bool traverseBegin(Node *) ;
	virtual void preAct(Node *) {}
	virtual void postAct(Node *);
	bool castRay(unsigned int row , unsigned int column,OUT Ray & ray);
	virtual void traceScene();
	virtual void buildTree();
	virtual void updateFrameBuffer();
	void updateCamera();
	virtual bool saveFrameBufferToFile();
	//todo: move this function to a dedicate OS services layer.
	static bool dirExists(const std::string& dirName_in);
	/////////////////////////
	//camera point of view //
	/////////////////////////
	CameraPos * m_pCamPos;
	GLuint m_texture;
	KDTreeCPU * m_pKdtree;
};
    

#endif //HULK_GL_VISITOR_CPU_TRACE_HPP
