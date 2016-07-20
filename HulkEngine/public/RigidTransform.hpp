//////////////////////////////////////////////////////////////////
// RigidTransform.hpp											//
// Hulk renderer - Create by Tzachi Cohen 2011					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////
#ifndef HULK_RIGID_TRANSFORM_HPP
#define HULK_RIGID_TRANSFORM_HPP

#include "windows.h"
#include "SPMatrix.hpp"
#include "Node.hpp"
#include "Intersectable.hpp"

//TODO : change the name from rigid transform to static tranform
class RigidTransform : public Node
{
public:
	
	//////////////////////////////////////
	//public initialization and cleanup //
	//////////////////////////////////////
	RigidTransform(); 
	virtual ~RigidTransform() {}
	
	void setTransformation(float scale[3],float rot[3],float trans[3],float deg);
	
	////////////////////////////
	// public utility methods //
	////////////////////////////
	

	
	/////////////////////
	// private members //
	/////////////////////
	// todo write access methods
	SPVector m_scale;
	SPVector m_rot;
	SPVector m_translate;
	float m_deg;
	///////////////////////////////////////////////
	//private initialization to prevent override //
	///////////////////////////////////////////////
private:
	RigidTransform(RigidTransform & n):Node(e_static_transform) {}
	RigidTransform& operator=(RigidTransform & n) {return *this;}
};


#endif //end of HULK_RigidTransform_HPP