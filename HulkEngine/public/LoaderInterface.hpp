//////////////////////////////////////////////////////////////////
// Scene.hpp													//
// Hulk renderer - Create by Tzachi Cohen 2013					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////


#ifndef HULD_LOADER_INTERFACE
#define HULD_LOADER_INTERFACE

#include <string>
#include <stdio.h>

#include "Scene.hpp"
#include "Visitor.hpp"

class LoaderInterface
{
public:
	virtual bool viParseFile(FILE *f,const std::string & relPath,Scene & scene)=0;
	//! @param rot - [0-2] - rotation axis, [3] - rotation in radians
	//! @param name - name of node to be transformed
	virtual bool getTransformation(Scene & scene,const char name [] , OUT double rot [4] , OUT double trans[3] ) = 0 ;
	virtual Visitor * getTransformVisitor(Scene & scene) = 0;
	virtual void freeTransformVisitor(Visitor *) = 0;
};


#endif // HULD_LOADER_INTERFACE