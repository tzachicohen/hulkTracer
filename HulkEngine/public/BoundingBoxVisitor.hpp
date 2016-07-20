//////////////////////////////////////////////////////////////////
// visitor.hpp													//
// Hulk renderer - Create by Tzachi Cohen 2011					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////
#ifndef HULK_BOUNDING_BOX_VISITOR_HPP
#define HULK_BOUNDING_BOX_VISITOR_HPP

#include "node.hpp"
#include "Visitor.hpp"
#include "Scene.hpp"

//! @brief calculate mesh nodes bounding boxes in object space
class BoundingBoxVisitor : public Visitor
{
public:
	//////////////////////////////////////
	//public initialization and cleanup //
	//////////////////////////////////////
	
	BoundingBoxVisitor (Scene & scene);
	virtual ~BoundingBoxVisitor ();

	////////////////////
	// public methods //
	////////////////////
	virtual bool traverseBegin(Node *);
	virtual void traverseEnd(Node *);
	virtual void preAct(Node *) ;
	virtual void postAct(Node *);
private:
	Scene & m_scene;
};

#endif
