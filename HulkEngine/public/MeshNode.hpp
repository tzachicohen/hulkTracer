//////////////////////////////////////////////////////////////////
// MeshNode.hpp											//
// Hulk renderer - Create by Tzachi Cohen 2011					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////

#ifndef MESH_NODE_HPP
#define MESH_NODE_HPP

#include <string>
#include <list>
#include <map>
#include <utility>

#include "Stack.hpp"
#include "SPMatrix.hpp"
#include "Intersectable.hpp"


class MeshNode : public Intersectable
{
public:


	//////////////////////////////////////
	//public initialization and cleanup //
	//////////////////////////////////////
	MeshNode(ENodeType nodeType):Intersectable(nodeType){}
	virtual ~MeshNode(){}

	//////////////////////////////////////
	// interface implementation methods //
	//////////////////////////////////////
	//! returns true if ray hit the object , false if the ray does not
	////////////////////
	// public methods //
	////////////////////
private:	

};




#endif