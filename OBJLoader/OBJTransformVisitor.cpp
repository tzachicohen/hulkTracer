
#include <stdlib.h>
#include "OBJLoader.hpp"
#include "OBJTransformVisitor.hpp"
#include "AnimatedTransform.hpp"
#include "RigidTransform.hpp"

//////////////////////////////////////
//public initialization and cleanup //
//////////////////////////////////////
OBJTransformVisitor::OBJTransformVisitor (Scene & scene)
:m_scene(scene)
{
}
OBJTransformVisitor::~OBJTransformVisitor ()
{
}

////////////////////
// public methods //
////////////////////

void OBJTransformVisitor::preAct(Node * pNode)
{
	if (pNode->getNodeType() == Node::e_dynamic_transform)
	{
	   AnimatedTransform * pTrans = static_cast<AnimatedTransform *> (pNode);
	   m_rotation.IdentityMatrix();
	   m_rotation.rotateMatrix(0.0f,1.0f,0.0f,m_scene.m_time);
	}
	if (pNode->getNodeType() == Node::e_dynamic_geometry)
	{
		Intersectable * pMesh = static_cast<Intersectable*> (pNode);
		//transform the vertices
		Stack<SPVector>::iterator destVertexIt;
		m_scene.getVertices(pMesh->getvertexOffset(),destVertexIt);
		SPVector * pSource = pMesh->m_pVerts;
		for (uint i = 0; i< pMesh->m_vertexCount;i++)
		{
			*destVertexIt =  (*pSource) * m_rotation;
			++pSource;
			++destVertexIt;
		}
		//TODO: transform the normals
	}

}
void OBJTransformVisitor::postAct(Node * pNode )
{
	
}

bool OBJTransformVisitor::traverseBegin(Node *node)
{
	return true;
}

void OBJTransformVisitor::traverseEnd(Node *node )
{
}

