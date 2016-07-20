//////////////////////////////////////////////////////////////////
// Scene.cpp													//
// Hulk renderer - Create by Tzachi Cohen 2008					//
// all rights reserved(c)										//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////
#include <math.h>
#include "stack.hpp"
#include "SPMatrix.hpp"
#include "Scene.hpp"
#include "StaticMeshNode.hpp"
#include "DynamicMeshNode.hpp"
#include "RigidTransform.hpp"
#include "DeleteVisitor.hpp"
#include "BoundingBoxVisitor.hpp"


//////////////////////////////////////
//public initialization and cleanup //
//////////////////////////////////////
Scene::Scene(const std::string sceneName):
m_pRoot(NULL)
,m_fov(45.0f)
,m_aspect(800.0f/600.0f)
,m_resx(800) //define default resolution
,m_resy(600) //define default resolution
,m_time(0)
,m_sceneName(sceneName)
{
	m_pRoot = new Node(Node::e_node);
}

Scene::~Scene()
{
	cleanup();
}

////////////////////
// public methods //
////////////////////

bool Scene::postLoadInitialize()
{
	// add default material to the material list;
	Material mat;
	m_materials.push_back(mat);
	
	m_frameBuffer.initialize(m_resx,m_resy,0);
	
	// set bounding boxes to all the geometry
	BoundingBoxVisitor bbv(*this);
	bbv.traverse(m_pRoot);


	return true;
}


//load a scene from file
void Scene::loadScene(std::string fileName)
{
	//empty implementation for a meantime
}
//make time dependant changes to scene
void Scene::updateScene(unsigned int time)
{
	//empty implementation for a meantime
}
void Scene::setTime(float time)
{
	m_time = time;
}

//////////////////////////////
// private support function //
//////////////////////////////

void Scene::cleanup()
{
	//free the graph from memory
	DeleteVisitor delVisitor;
	delVisitor.traverse(m_pRoot);
	m_pRoot = NULL;
	//clear all containers
	m_sceneName.clear();
	m_lights.clear();
	m_vertices.clear();
	m_normals.clear();
	m_vertexIndices.clear();
	m_normIndices.clear();
	m_materials.clear();
	m_boundingBox.clear(); // a list 
}

//! @ param the pNorms number of vertices in 3*numNorms format
unsigned int Scene::setVertexIndices(const unsigned int  * pIndices,unsigned int numIndices,uint vertexOffset)
{
	unsigned int retVal = m_vertexIndices.size();
	for (unsigned int i =0 ;i<numIndices;i++)
	{
		TriangleIndices index;
		index.indices[0] = vertexOffset+pIndices[i*3];
		index.indices[1] = vertexOffset+pIndices[i*3+1];
		index.indices[2] = vertexOffset+pIndices[i*3+2];
		m_vertexIndices.push_back(index);
	}
	return retVal;
}

unsigned int  Scene::setVertexIndices(const unsigned short * pIndices,unsigned int numIndices,uint vertexOffset)
{
	unsigned int retVal = m_vertexIndices.size();
	for (unsigned int i =0 ;i<numIndices;i++)
	{
		TriangleIndices index;
		index.indices[0] = vertexOffset+(unsigned int)pIndices[i*3];
		index.indices[1] = vertexOffset+(unsigned int)pIndices[i*3+1];
		index.indices[2] = vertexOffset+(unsigned int)pIndices[i*3+2];
		m_vertexIndices.push_back(index);
	}
	return retVal;
}
//! @ param the pNorms number of vertices in 3*numNorms format
unsigned int Scene::setNormIndices(const unsigned int  * pIndices,unsigned int numIndices,uint vertexOffset)
{
	unsigned int retVal = m_normIndices.size();
	for (unsigned int i =0 ;i<numIndices;i++)
	{
		TriangleIndices index;
		index.indices[0] = vertexOffset+pIndices[i*3];
		index.indices[1] = vertexOffset+pIndices[i*3+1];
		index.indices[2] = vertexOffset+pIndices[i*3+2];
		m_normIndices.push_back(index);
	}
	return retVal;
}

unsigned int  Scene::setNormIndices(const unsigned short * pIndices,unsigned int numIndices,uint vertexOffset)
{
	unsigned int retVal = m_normIndices.size();
	for (unsigned int i =0 ;i<numIndices;i++)
	{
		TriangleIndices index;
		index.indices[0] = vertexOffset+(unsigned int)pIndices[i*3];
		index.indices[1] = vertexOffset+(unsigned int)pIndices[i*3+1];
		index.indices[2] = vertexOffset+(unsigned int)pIndices[i*3+2];
		m_normIndices.push_back(index);
	}
	return retVal;
}



// allocate space for the vertices, they will be place with the transform routine
uint Scene::allocateVertices(uint numVertices)
{
	SPVector temp;
	unsigned int offset = m_vertices.size();
	
	//todo: improve the performance of this routine
	for (unsigned int i =0 ; i < numVertices;i++)
	{
		m_vertices.push_back(temp);
	}
	return offset;
}

// allocate space for the vertices, they will be place with the transform routin
uint Scene::allocateNormals(uint numNorms)
{
	SPVector temp;
	unsigned int offset = m_normals.size();
	
	for (unsigned int i =0 ; i < numNorms;i++)
	{
		m_normals.push_back(temp);
	}
	return offset;
}

bool  Scene::getVertices(unsigned int offset, OUT Stack<SPVector>::iterator & it)
{
	if (offset >= m_vertices.size())
		return false;

	it = m_vertices.getIteratorAtIndex(offset);
	return true;
}

bool  Scene::getNorms(unsigned int offset, OUT Stack<SPVector>::iterator & it)
{
	if (offset >= m_normals.size())
		return false;

	it = m_normals.getIteratorAtIndex(offset);
	return true;
}

bool  Scene::getVertexIndices(unsigned int offset, OUT Stack<TriangleIndices>::iterator & it)
{
	if (offset >= m_vertexIndices.size())
		return false;
	it = m_vertexIndices.getIteratorAtIndex(offset);
	return true;
}
bool  Scene::getNormalIndices(unsigned int offset, OUT Stack<TriangleIndices>::iterator & it)
{
	if (offset >= m_normIndices.size())
		return false;
	it = m_normIndices.getIteratorAtIndex(offset);
	return true;
}

