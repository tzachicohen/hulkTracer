//////////////////////////////////////////////////////////////////
// GLVisitor.hpp											//
// Hulk renderer - Create by Tzachi Cohen 2012					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////
#ifndef HULK_GL_VISITOR_HPP
#define HULK_GL_VISITOR_HPP

#include <Windows.h>
#include "Visitor.hpp"
#include "Scene.hpp"

//this class draws the scene using openGL
class GLVisitor : public Visitor
{
public:
	GLVisitor() :m_pScene(NULL)  {}
	virtual bool traverseBegin(Node *) ;
	virtual void preAct(Node *) ;
	virtual void postAct(Node *);
	virtual void traverseEnd(Node *) ;
	Scene * m_pScene;
};

#endif //HULK_GL_VISITOR_HPP
