//////////////////////////////////////////////////////////////////
// visitor.hpp													//
// Hulk renderer - Create by Tzachi Cohen 2011					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////
#ifndef OBJ_TRANSFORM_VISITOR_HPP
#define OBJ_TRANSFORM_VISITOR_HPP

#include "node.hpp"
#include "Visitor.hpp"
#include "stack.hpp"
#include "SPMatrix.hpp"
#include "scene.hpp"

class OBJTransformVisitor : public Visitor , public AllignedS
{
public:
	//////////////////////////////////////
	//public initialization and cleanup //
	//////////////////////////////////////
	
	OBJTransformVisitor (Scene & scene);
	virtual ~OBJTransformVisitor ();

	////////////////////
	// public methods //
	////////////////////
	virtual bool traverseBegin(Node *);
	virtual void traverseEnd(Node *);
	virtual void preAct(Node *) ;
	virtual void postAct(Node *);

	
private:

	SPMatrix m_rotation;
	Scene & m_scene;
};

#endif
