//////////////////////////////////////////////////////////////////
// visitor.hpp													//
// Hulk renderer - Create by Tzachi Cohen 2011					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////
#ifndef HULK_DELETE_VISITOR_HPP
#define HULK_DELETE_VISITOR_HPP

#include "node.hpp"
#include "Visitor.hpp"

class DeleteVisitor : public Visitor
{
public:
	//////////////////////////////////////
	//public initialization and cleanup //
	//////////////////////////////////////
	
	DeleteVisitor ();
	virtual ~DeleteVisitor ();

	////////////////////
	// public methods //
	////////////////////
	virtual bool traverseBegin(Node *);
	virtual void traverseEnd(Node *);
	virtual void preAct(Node *) ;
	virtual void postAct(Node *);
private:
};

#endif
