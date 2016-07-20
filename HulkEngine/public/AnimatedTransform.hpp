//////////////////////////////////////////////////////////////////
// AnimatedTransform.hpp											//
// Hulk renderer - Create by Tzachi Cohen 2011					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////
#ifndef HULK_ANIMATED_TRANSFORM_HPP
#define HULK_ANIMATED_TRANSFORM_HPP

#include "windows.h"
#include "SPMatrix.hpp"
#include "Node.hpp"
#include "Intersectable.hpp"


class AnimatedTransform : public Node
{
public:
	
	//////////////////////////////////////
	//public initialization and cleanup //
	//////////////////////////////////////
	AnimatedTransform():Node(e_dynamic_transform){}
	virtual ~AnimatedTransform() {}
	
	////////////////////////////
	// public utility methods //
	////////////////////////////
	void setName(char * name){ m_name = name;}
	const char * getName() {return m_name.c_str();}
private:
	std::string m_name;
	/////////////////////
	// private members //
	/////////////////////
	
	///////////////////////////////////////////////
	//private initialization to prevent override //
	///////////////////////////////////////////////
	AnimatedTransform(AnimatedTransform & n):Node(e_dynamic_transform) {}
	AnimatedTransform& operator=(AnimatedTransform & n) {return *this;}
};


#endif //end of HULK_AnimatedTransform_HPP