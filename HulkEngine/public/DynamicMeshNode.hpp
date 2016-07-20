//////////////////////////////////////////////////////////////////
// DynamicMeshNode.hpp											//
// Hulk renderer - Create by Tzachi Cohen 2011					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////

#ifndef DYNAMIC_MESH_NODE_HPP
#define DYNAMIC_MESH_NODE_HPP

#include <string>
#include <list>
#include <map>
#include <utility>

#include "Stack.hpp"
#include "SPMatrix.hpp"
#include "MeshNode.hpp"


class DynamicMeshNode : public MeshNode
{
public:
	struct TriIndices
	{
		unsigned int m_index[3];
	};

	//////////////////////////////////////
	//public initialization and cleanup //
	//////////////////////////////////////
	DynamicMeshNode():MeshNode(e_dynamic_geometry){}
	virtual ~DynamicMeshNode(){}
	void resetNode();
	//////////////////////////////////////
	// interface implementation methods //
	//////////////////////////////////////
	//! returns true if ray hit the object , false if the ray does not
	////////////////////
	// public methods //
	////////////////////
private:	
	/////////////////////////////
	// private BSP functions   //
	/////////////////////////////
	//! bounding box of the entire object

	Stack<SPVector> m_vertices;
	Stack<SPVector> m_normal;

};




#endif