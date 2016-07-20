
#include <vector>
#include "hulk_assert.h"

#include "BoundingBoxVisitor.hpp"
#include "SseUtils.hpp"
//////////////////////////////////////
//public initialization and cleanup //
//////////////////////////////////////
BoundingBoxVisitor::BoundingBoxVisitor (Scene & scene)
:m_scene( scene)
{
}
BoundingBoxVisitor::~BoundingBoxVisitor ()
{
}

////////////////////
// public methods //
////////////////////

void BoundingBoxVisitor::preAct(Node *)
{
	
}

void BoundingBoxVisitor::postAct(Node * node)
{
	if (node->getNodeType() == Node::e_static_geometry ||
		node->getNodeType() == Node::e_dynamic_geometry)
	{
		Intersectable * pMesh = static_cast<Intersectable*> ( node);
		Scene::ObjectBox nodeBox;
		Stack<SPVector>::iterator  pVerts;

		uint offset = pMesh->getvertexOffset();
		
		m_scene.getVertices(offset,pVerts);
		
		//place the first vertex in the bouding box to begin with
		nodeBox.box.m_max = *pVerts;
		nodeBox.box.m_min = *pVerts;
		//iterate all the triangle
		++pVerts;
		for (uint i = 1 ; i < pMesh->m_vertexCount ; i++)
		{
			nodeBox.box.m_min = sse::min(nodeBox.box.m_min.vec,pVerts->vec);
			nodeBox.box.m_max = sse::max(nodeBox.box.m_max.vec,pVerts->vec);
			++pVerts;
		}
		nodeBox.pNode = pMesh;
		//if this is the first traverse
		if (-1 == pMesh->m_boxIndex) {
			uint32 index = m_scene.m_boundingBox.size() ;
			m_scene.m_boundingBox.push_back(nodeBox);
			pMesh->m_boxIndex = index;
		}
		else
		{
			m_scene.m_boundingBox[pMesh->m_boxIndex]  = nodeBox;
		}


	}
}

bool BoundingBoxVisitor::traverseBegin(Node *)
{
	return true;
}

void BoundingBoxVisitor::traverseEnd(Node *)
{
	m_scene.m_sceneBox = m_scene.m_boundingBox[0].box;
	//calculate the scene bounding box
	for (uint i =1 ; i < m_scene.m_boundingBox.size();i++) {
		m_scene.m_sceneBox.m_min = sse::min(m_scene.m_boundingBox[i].box.m_min.vec,m_scene.m_sceneBox.m_min.vec);
		m_scene.m_sceneBox.m_max = sse::max(m_scene.m_boundingBox[i].box.m_max.vec,m_scene.m_sceneBox.m_max.vec);
	}
}