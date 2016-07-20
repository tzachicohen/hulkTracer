
#include "GLVisitor.hpp"
#include "GLWindow.h"
#include "animatedTransform.hpp"
#include "RigidTransform.hpp"

bool GLVisitor::traverseBegin(Node *)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // draw outlines only
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(m_pScene->m_fov,m_pScene->m_aspect,0.01,2000);
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	//////////////////////////////////////////////////////////////
	// camera transformations 
	double trans[3] = {0};
	double rot[4] = {0} ;
	
	GLWindow::getInstance().m_pLoader->getTransformation(*m_pScene,"camera",rot,trans);
	glRotated(-rot[3]*180.0/M_PI,rot[0],rot[1],rot[2]);
	glTranslated(-trans[0],-trans[1],-trans[2]);
	///////////////////////////////////////////////////////////////
	glClearColor(0.0f,0.0f,0.0f,0.f);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glViewport(0,0,m_pScene->m_resx,m_pScene->m_resy);
	glColor3f(0.9,0.9,0.9);
	return true;
}

void GLVisitor::preAct(Node * pNode)
{
	if (pNode->getNodeType() == Node::e_dynamic_transform)
	{
	   AnimatedTransform * pTrans = static_cast<AnimatedTransform *> (pNode);

		double trans[3] = {0};
		double rot[4] = {0};
		GLWindow::getInstance().m_pLoader->getTransformation(*m_pScene,pTrans->getName(),rot,trans);
		glPushMatrix();
	
		glTranslated(trans[0],trans[1],trans[2]);
	
		glRotated(rot[3]*180.0/M_PI,rot[0],rot[1],rot[2]);

/*		 if(anim->scales)
		 {       
			double scale[3];
			_GetScale(anim,m_pScene->m_time,scale);
			glScaled(scale[0],scale[1],scale[2]);
		 }         */ 
	}

	if (pNode->getNodeType() == Node::e_static_transform)
	{
	   RigidTransform * pTrans = static_cast<RigidTransform *> (pNode);
	   glPushMatrix();
	   glTranslated(pTrans->m_translate.x,pTrans->m_translate.y,pTrans->m_translate.z);
	   glRotated(pTrans->m_deg*180.0/M_PI,pTrans->m_rot.x,pTrans->m_rot.y,pTrans->m_rot.z);
	   glScaled(pTrans->m_scale.x,pTrans->m_scale.y,pTrans->m_scale.z);     
	}
	if (pNode->getNodeType() == Node::e_dynamic_geometry ||
		pNode->getNodeType() == Node::e_static_geometry)
	{
		Intersectable * pMesh = static_cast<Intersectable*> (pNode);
		Stack<TriangleIndices>::iterator indexIt;
		m_pScene->getVertexIndices(pMesh->getVertexIndicesOffset(),indexIt);
		uint vertexOffset = pMesh->getvertexOffset();
		glBegin(GL_TRIANGLES);
		for (uint i = 0; i< pMesh->getTrigCount();i++)
		{
			uint v1 = indexIt->indices[0]- vertexOffset;
			uint v2 = indexIt->indices[1]- vertexOffset;
			uint v3 = indexIt->indices[2]- vertexOffset;
			glVertex3f(pMesh->m_pVerts[v1].x,pMesh->m_pVerts[v1].y,pMesh->m_pVerts[v1].z);
			
			glVertex3f(pMesh->m_pVerts[v2].x,pMesh->m_pVerts[v2].y,pMesh->m_pVerts[v2].z);
			
			glVertex3f(pMesh->m_pVerts[v3].x,pMesh->m_pVerts[v3].y,pMesh->m_pVerts[v3].z);
			++indexIt;
		}
		glEnd();
	}

}
void GLVisitor::postAct(Node * pNode )
{
	if ((pNode->getNodeType() == Node::e_dynamic_transform )||
		(pNode->getNodeType() == Node::e_static_transform))
	{
		glPopMatrix();
	}
}
void GLVisitor::traverseEnd(Node *) 
{
	glutSwapBuffers();
}




