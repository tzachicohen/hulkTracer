//////////////////////////////////////////////////////////////////
// OBJLoader.hpp												//
// Hulk renderer - Create by Tzachi Cohen 2013					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////

#ifndef HULK_OBJ_LOADER_HPP
#define HULK_OBJ_LOADER_HPP

#include "hulk_assert.h"
#include "LoaderInterface.hpp"

//! this class parses and OBJ file
class OBJLoader : public LoaderInterface
{
public:
	//! load the object from file
	virtual bool viParseFile(FILE *f,const std::string & relPath,Scene & scene);
	virtual bool getTransformation(Scene & scene,const char name [] , OUT double rot [4] , OUT double trans[3] ) ;
	Visitor * getTransformVisitor(Scene & scene) ;
	void freeTransformVisitor(Visitor * vis) ;
private:
	//reconstruct vertices normals
	bool getVerticesNormals(const Stack<unsigned int> & indices,const Stack<SPVector> &vertices, 
		OUT Stack<SPVector>  & normals);
	virtual EHulkResult loadMaterial(std::string fileName);
};

#endif