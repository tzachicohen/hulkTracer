//////////////////////////////////////////////////////////////////
// visitor.hpp													//
// Hulk renderer - Create by Tzachi Cohen 2008					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////

#ifndef HULK_VISITOR_HPP
#define HULK_VISITOR_HPP

#include "node.hpp"
//! the visitor class implement a traverse mechanism that is 
//! that is completely overridable
class Visitor
{
public:
	//////////////////////////////////////
	//public initialization and cleanup //
	//////////////////////////////////////
	inline Visitor ();
	inline virtual ~Visitor ();

	////////////////////
	// public methods //
	////////////////////
	//perform DFS search
	inline virtual  void traverse(Node *);
	//! optional override
	virtual void initializeVisitor() {}
	//! if 'traverseBegin' returns false than the visitor class will skip 
	// the traverse of the entire graph and go directly to 'traverseEnd'
	virtual bool traverseBegin(Node *) =0;
	virtual void preAct(Node *) = 0;
	virtual void postAct(Node *) = 0;
	virtual void traverseEnd(Node *) =0;
private:
	/////////////////////
	// private methods //
	/////////////////////
	inline void traverseGraph(Node*);
};


Visitor::Visitor ()
{
}

Visitor::~Visitor ()
{
}

////////////////////
// public methods //
////////////////////
void Visitor::traverse(Node *node)
{
	if (traverseBegin(node)) {
		traverseGraph(node);
	}
	traverseEnd(node);
}

void Visitor::traverseGraph(Node*node)
{
	
	preAct(node);
	Node::SonIterator it = node->begin();
	while (it != node->end())
	{
		traverseGraph(*it);
		++it;
	}
	postAct(node);
}
#endif