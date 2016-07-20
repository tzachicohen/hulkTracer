//////////////////////////////////////////////////////////////////
// node.hpp														//
// Hulk renderer - Create by Tzachi Cohen 2008					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////

#include <list>

#include "hulk_assert.h"

#ifndef HULK_NODE
#define HULK_NODE

class Node
{
public:
	//////////////////////////////
	// public enums and classes //
	//////////////////////////////
	class SonIterator
	{
	public:
		SonIterator & operator++();
		friend bool operator== (SonIterator &,SonIterator &);
		friend bool operator!= (SonIterator & ,SonIterator &);
		friend Node *  operator*(SonIterator & it);
		SonIterator & operator= (SonIterator &);
	private:
		std::list<Node*>::iterator m_iterator;
		friend class Node;
	};

	enum ENodeType
	{
		e_node,
		e_static_geometry,
		e_dynamic_geometry,
		e_light,
		e_static_transform,
		e_dynamic_transform
	};
	//////////////////////////////////////
	//public initialization and cleanup //
	//////////////////////////////////////
	Node(ENodeType type);
	virtual ~Node();
	////////////////////
	// public methods //
	////////////////////
	EHulkResult removeSon(Node *);
	EHulkResult addSon(Node *);
	//! returns an iterator to the first son
	SonIterator  begin();
	//! returns an interator to the end of the son list
	SonIterator  end();

	//! this function check is the node is has no predecessor and no sons. if so the function delete the node 
	//! and returns true. if the node has predecessors or sons the object is not deleted and the function returns false.
	bool deleteNode();
	//!this node receives a time value and updates a node. time is in milliseconds
	virtual void updateNode(unsigned int time);

	ENodeType getNodeType() {return m_type;}
private:
	//////////////////////////////
	// private support function //
	//////////////////////////////
	//! removes a certain node from the Son container
	EHulkResult removeFromSonContainer(Node * );
	//! removes a certain node from the Predecessor container
	EHulkResult removeFromPredContainer(Node *);
	/////////////////////
	// private members //
	/////////////////////
	std::list<Node*> m_sons;
	std::list<Node*> m_predecessors;
	ENodeType m_type;
	///////////////////////////////////////////////
	//private initialization to prevent override //
	///////////////////////////////////////////////
	Node(Node & n);
	Node& operator=(Node & n);
};

#endif
