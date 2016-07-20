//////////////////////////////////////////////////////////////////
// GLVisitor.hpp											//
// Hulk renderer - Create by Tzachi Cohen 2012					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////
#ifndef HULK_GL_VISITOR_CPU_TRANSFORM_HPP
#define HULK_GL_VISITOR_CPU_TRANSFORM_HPP

#include "GLVisitor.hpp"
#include "glut.h"
 
//this class draws the scene with openGL but the
//vertices are transformed using the CPU
class GLVisitorT : public GLVisitor
{
	inline virtual void preAct(Node *) ;
	virtual void postAct(Node *){}
};

inline void GLVisitorT::preAct(Node * pNode)
{
	if (pNode->getNodeType() == Node::e_dynamic_geometry ||
		pNode->getNodeType() == Node::e_static_geometry)
	{
		Intersectable * pMesh = static_cast<Intersectable*> (pNode);
		Stack<TriangleIndices>::iterator indexIt;
		m_pScene->getVertexIndices(pMesh->getVertexIndicesOffset(),indexIt);
		
		glBegin(GL_TRIANGLES);
		for (uint i = 0; i< pMesh->getTrigCount();i++)
		{
			uint v1 = indexIt->indices[0];
			uint v2 = indexIt->indices[1];
			uint v3 = indexIt->indices[2];
			glVertex3f(m_pScene->m_vertices[v1].x,m_pScene->m_vertices[v1].y,m_pScene->m_vertices[v1].z);
			glVertex3f(m_pScene->m_vertices[v2].x,m_pScene->m_vertices[v2].y,m_pScene->m_vertices[v2].z);
			glVertex3f(m_pScene->m_vertices[v3].x,m_pScene->m_vertices[v3].y,m_pScene->m_vertices[v3].z);
			++indexIt;
		}
		glEnd();
	}
}

#endif //HULK_GL_VISITOR_CPU_TRANSFORM_HPP
