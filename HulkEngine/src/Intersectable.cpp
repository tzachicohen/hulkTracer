	// Hulk renderer - Create by Tzachi Cohen 2008					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////
#include <limits>

#include "Intersectable.hpp"
#include "util.hpp"
#include "scene.hpp"
#include "SharedStructs.hpp"
#include "SseUtils.hpp"

using namespace sse;

Intersectable::Intersectable(ENodeType type)
:Node(type) 
, m_vertexCount(0)
, m_normalCount(0)
, m_vertexOffset(0)
, m_normalOffset(0)
, m_pNorms(NULL)
, m_pVerts(NULL)
, m_vertexIndicesOffset(0)
, m_normalIndicesOffset(0)
, m_trigCount(0)
, m_boxIndex(-1)
{
}

Intersectable:: ~Intersectable() 
{
	SAFE_ARRAY_DELETE( m_pNorms)
	SAFE_ARRAY_DELETE( m_pVerts)
}

//! @ param the pVertices vertices coordinates in 3*numVertices format
bool Intersectable::setVertices(float * pVertices,unsigned int numVertices,unsigned int offset)
{
	SAFE_ARRAY_DELETE(m_pVerts);

	m_pVerts = new SPVector[numVertices];
	if (!m_pVerts)
		return false;

	m_vertexOffset =offset;
	m_vertexCount = numVertices;

	for (unsigned int i = 0 ; i < numVertices;i++)
	{
		unsigned int index = 3*i;
		m_pVerts[i].x = pVertices[index];
		m_pVerts[i].y = pVertices[index+1];
		m_pVerts[i].z = pVertices[index+2];
		m_pVerts[i].w = 1.0f;
	}
	return true;
}

bool Intersectable::setVertices(const SPVector * vertices,unsigned int numVertices,unsigned int offset)
{
	m_vertexOffset =offset;
	m_vertexCount = numVertices;
	m_pVerts = new SPVector[numVertices];
	HULK_ASSERT(m_pVerts != NULL, "failureto allocate vertices\n")
	memcpy(m_pVerts,vertices,sizeof(SPVector)*numVertices);
	return true;
}

//! @ param the pNorms number of vertices in 3*numNorms format
bool Intersectable::setNorms(float * pNorms,unsigned int numNorms,uint offset)
{
	SAFE_ARRAY_DELETE(m_pNorms);
	if (NULL == pNorms)
	{
		return false;
	}
	m_normalOffset=offset;
	m_normalCount = numNorms;

	m_pNorms = new SPVector[numNorms];
	if (!m_pNorms)
		return false;

	for (unsigned int i = 0 ; i < numNorms;i++)
	{
		unsigned int index = 3*i;
		m_pNorms[i].x = pNorms[index];
		m_pNorms[i].y = pNorms[index+1];
		m_pNorms[i].z = pNorms[index+2];
		m_pNorms[i].w = 1.0f;
	}
	return true;
}

bool Intersectable::setNorms(const SPVector * pNorms,unsigned int numNorms,uint offset)
{
	m_normalOffset=offset;
	m_normalCount = numNorms;
	m_pNorms = new SPVector[numNorms];
	HULK_ASSERT(m_pNorms != NULL, "failure to allocate vertices\n")
	memcpy(m_pNorms,pNorms,sizeof(SPVector)*numNorms);
	return true;
}
void  Intersectable::setVertexIndices(const unsigned short * pIndices,unsigned int NumTriangle)
{
	for (unsigned int i =0 ;i<NumTriangle;i++)
	{
		TriangleIndices index;
		index.indices[0] = (unsigned int)pIndices[i*3];
		index.indices[1] = (unsigned int)pIndices[i*3+1];
		index.indices[2] = (unsigned int)pIndices[i*3+2];
		m_vertexIndices.push_back(index);
	}
}

void Intersectable::update(const SPMatrix & mat,Scene & scene)
{
	//update the vertices;
	SPVector* pUntransforms = m_pVerts;
	Stack<SPVector>::iterator vertIt ;
	scene.getVertices(m_vertexOffset,vertIt);
	for (unsigned int i =0; i<m_vertexCount ;i++)
	{
		*vertIt = (*pUntransforms)*mat;
		++vertIt;
		pUntransforms++;
	}
	// transform the normals with the inverse transpose
	pUntransforms = m_pNorms;
	Stack<SPVector>::iterator normIT;
	scene.getNorms(m_vertexOffset,normIT);
	SPMatrix normMat = mat;
	normMat._14 = normMat._24 = normMat._34 = 0.0f;
	normMat._41 = normMat._42 = normMat._43 = 0.0f;
	for (unsigned int i =0; i<m_vertexCount ;i++)
	{
		*normIT = (*pUntransforms)*normMat;
		++normIT;
		pUntransforms++;
	}
}


// implementation with SSE

//SPVector timeStart = zeroesInit;
//	
//SPVector tmin,tmax;
//
//tmin = (box.m_min - ray.m_rayStart).vec / ray.m_rayDir.vec;
//tmax = (box.m_max - ray.m_rayStart).vec /  ray.m_rayDir.vec;
//
//tmin.vec = (tmin.vec & ray.m_sign.vec) & (tmax.vec & not(ray.m_sign.vec));
//tmax.vec = (tmax.vec & ray.m_sign.vec) & (tmin.vec & not(ray.m_sign.vec));
//
//	
//	
//return retVal;