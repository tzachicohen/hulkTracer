//////////////////////////////////////////////////////////////////
// StaticMeshNode.cpp												//
// Hulk renderer - Create by Tzachi Cohen 2008					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////
#include <math.h>
#include <stdio.h>
#include <limits>


#include "StaticMeshNode.hpp"

//////////////////////////////////////
//public initialization and cleanup //
//////////////////////////////////////


StaticMeshNode::StaticMeshNode()
:MeshNode(e_static_geometry)
{
	
}

StaticMeshNode::~StaticMeshNode()
{
	resetNode();
}

void StaticMeshNode::resetNode()
{
	
}









