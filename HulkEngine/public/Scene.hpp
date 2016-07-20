//////////////////////////////////////////////////////////////////
// Scene.hpp													//
// Hulk renderer - Create by Tzachi Cohen 2008					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////

#include "node.hpp"
#include "Intersectable.hpp"
#include "Stack.hpp"
#include "SPMatrix.hpp"
#include "FrameBuffer.hpp"
#include "SharedStructs.hpp"

#ifndef HULK_SCENE_HPP
#define HULK_SCENE_HPP

class Scene
{
public:	
	//////////////////////////////////////
	//public initialization and cleanup //
	//////////////////////////////////////
	virtual ~Scene();
	Scene(const std::string sceneName);
	////////////////////
	// public structs //
	////////////////////
	struct ObjectBox :public AllignedS
	{
		Box box;
		Intersectable * pNode;
	};
	////////////////////
	// public methods //
	////////////////////
	// initialize a scene - should be called after the scene has be loaded from file
	bool postLoadInitialize();
	//load a scene from file
	void loadScene(std::string fileName);
	//make time dependant changes to scene
	void updateScene(unsigned int time);
	//! @param pIndices pointer to an array which is numIndices*3 in size
	//! @return is the offset of the index list in the global list
	unsigned int setVertexIndices(const unsigned int * pIndices,unsigned int numIndices,uint vertexOffset);
	unsigned int setVertexIndices(const unsigned short * pIndices,unsigned int numIndices,uint vertexOffset);
	//! @param pIndices pointer to an array which is numIndices*3 in size
	//! @return is the offset of the index list in the global list
	unsigned int setNormIndices(const unsigned int * pIndices,unsigned int numIndices,uint vertexOffset);
	unsigned int setNormIndices(const unsigned short * pIndices,unsigned int numIndices,uint vertexOffset);
	//these functions are called by the loader to allocate space for vertices and
	//normals for a certain object
	uint allocateVertices(uint numVertices);
	uint allocateNormals(uint numNorms);

	bool  getVertices(unsigned int offset, OUT Stack<SPVector>::iterator & it);
	bool  getNorms(unsigned int offset, OUT Stack<SPVector>::iterator & it);
	bool  getVertexIndices(unsigned int offset, OUT Stack<TriangleIndices>::iterator & it);
	bool  getNormalIndices(unsigned int offset, OUT Stack<TriangleIndices>::iterator & it);
	
	void setTime(float time);
	void traceScene();
	/////////////////////
	// private members //
	/////////////////////
	Node * m_pRoot; //pointer to root node
	Stack<Light> m_lights;
	Stack<SPVector > m_vertices;
	Stack<SPVector > m_normals;
	Stack<TriangleIndices> m_vertexIndices;
	Stack<TriangleIndices> m_normIndices;
	Stack<Material> m_materials; // scene materials
	Stack<ObjectBox> m_boundingBox; // a list of the bounding boxes off all the objects in the scene
	Box m_sceneBox; //a bounding box that encapsulate the entire scene.
	FrameBuffer<unsigned int> m_frameBuffer;
	//camera characteristics - TODO: to be moved to a dedicated class.
	float m_fov ; //in field of view in degrees
	float m_aspect;// aspect = (height /width)

	uint m_resx;
	uint m_resy;
	float m_time; // the current time in the animation

	std::string m_sceneName;
	//////////////////////////////
	// private support function //
	//////////////////////////////
	
	void cleanup();
private:
	//////////////////////////////////////////////
	//private initialization prevent of copies  //
	//////////////////////////////////////////////	
	Scene & operator=(Scene & f);
	Scene(Scene & f);
};


#endif