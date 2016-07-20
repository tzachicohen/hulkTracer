
#include <stdlib.h>
#include "AFFParser.hpp"
#include "AFFTransformVisitor.hpp"
#include "AnimatedTransform.hpp"
#include "RigidTransform.hpp"
#include "animation.hpp"
//////////////////////////////////////
//public initialization and cleanup //
//////////////////////////////////////
AFFTransformVisitor::AFFTransformVisitor (Scene & scene,AFFParser & parser)
: m_pScene(&scene)
, m_pParser(&parser)
, m_calcStatics(false)
{
}
AFFTransformVisitor::~AFFTransformVisitor ()
{
}

////////////////////
// public methods //
////////////////////

void AFFTransformVisitor::preAct(Node * pNode)
{
	Animation *anim;

	if (pNode->getNodeType() == Node::e_dynamic_transform)
	{
	   AnimatedTransform * pTrans = static_cast<AnimatedTransform *> (pNode);
	   anim=FindAnimation(pTrans->getName(),& (m_pParser->m_animation));
	  
	   SPMatrix mat = *(m_matrixStack.getBackObject());
	   if(anim)  
		{ 
			if(anim->translations) 
			{       
			double trans[3];
			_GetTranslation(anim,m_pScene->m_time,trans);
			SPMatrix transMat;
			transMat.TranslateMatrix(trans[0],trans[1],trans[2]);
			transMat *= mat;
			mat = transMat;
		 }   
	         
		 if(anim->rotations) 
		 {       
			double rot[4];
			_GetRotation(anim,m_pScene->m_time,rot);
			SPMatrix rotMat;
			rotMat.rotateMatrix(rot[0],rot[1],rot[2],-rot[3]);
			rotMat *= mat;
			mat = rotMat;
		 }   
		 if(anim->scales)
		 {       
			double scale[3];
			_GetScale(anim,m_pScene->m_time,scale);
			SPMatrix scaleMat;
			scaleMat.ScaleMatrix(scale[0],scale[1],scale[2]);
			scaleMat *= mat;
			mat = scaleMat;
		 }          
	   }//if (anim)
		m_matrixStack.push_back(mat);
	}// if (pNode->getNodeType() == Node::e_dynamic_transform)
	if (pNode->getNodeType() == Node::e_static_transform)
	{
	   RigidTransform * pTrans = static_cast<RigidTransform *> (pNode);
	   SPMatrix scaleMat;
	   SPMatrix transMat;
	   SPMatrix rotMat;

	   transMat.TranslateMatrix(pTrans->m_translate.x,pTrans->m_translate.y,pTrans->m_translate.z);
	   rotMat.rotateMatrix(pTrans->m_rot.x,pTrans->m_rot.y,pTrans->m_rot.z,pTrans->m_deg*M_PI/180.0);
	   scaleMat.ScaleMatrix(pTrans->m_scale.x,pTrans->m_scale.y,pTrans->m_scale.z);
	   SPMatrix mat = *(m_matrixStack.getBackObject());
	
	   transMat *= mat;
	   scaleMat *= rotMat;
	  
	   scaleMat *= transMat;
	   m_matrixStack.push_back(scaleMat);
	}
	//calulate static geometry transform only once
	if (pNode->getNodeType() == Node::e_dynamic_geometry ||
		(pNode->getNodeType() == Node::e_static_geometry && m_calcStatics))
	{
		Intersectable * pMesh = static_cast<Intersectable*> (pNode);
		//transform the vertices
		Stack<SPVector>::iterator destVertexIt;
		m_pScene->getVertices(pMesh->getvertexOffset(),destVertexIt);
		SPMatrix mat = *(m_matrixStack.getBackObject());
		SPVector * pSource = pMesh->m_pVerts;
		for (uint i = 0; i< pMesh->m_vertexCount;i++)
		{
			*destVertexIt =  (*pSource) * mat;
			++pSource;
			++destVertexIt;
		}
		//cancel any translations
		mat._41 = 0;
		mat._42 = 0;
		mat._43 = 0;
		// transform the normals
		if (1 == pMesh->m_meshData.e_normalData)
		{
			Stack<SPVector>::iterator destNormalIt;
			m_pScene->getNorms(pMesh->getNormalOffset(),destNormalIt);
			pSource = pMesh->m_pNorms;
			for (uint i = 0; i< pMesh->m_normalCount;i++)
			{
				*destNormalIt =  (*pSource) * mat;
				(*destNormalIt).normalize();
				++pSource;
				++destNormalIt;
			}
		}
	}

}
void AFFTransformVisitor::postAct(Node * pNode )
{
	if ((pNode->getNodeType() == Node::e_dynamic_transform )||
		(pNode->getNodeType() == Node::e_static_transform))
	{
		m_matrixStack.pop_back();
	}
}

bool AFFTransformVisitor::traverseBegin(Node *node)
{
	//transform static geometry only in the first travesre
	static int count = 0;
	if (0 == count)
	{
		m_calcStatics= true;
		count++;
	}
	//clean the stack from previous traverses
	m_matrixStack.clear();

	SPMatrix mat;
	mat.IdentityMatrix();

	m_matrixStack.push_back(mat);
	return true;
}

void AFFTransformVisitor::traverseEnd(Node *node )
{
	m_calcStatics = false;
}

