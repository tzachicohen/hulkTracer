//////////////////////////////////////////////////////////////////
// Node.hpp														//
// Hulk renderer - Create by Tzachi Cohen 2008					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////
#include <algorithm>

#include "Node.hpp"

Node::Node(ENodeType type)
:m_type(type)
{
}

Node::~Node()
{
}


EHulkResult Node::removeSon(Node *node)
{
	EHulkResult retVal = node->removeFromPredContainer(this);
	if (e_ok == retVal)
	{
		retVal = removeFromSonContainer(node);
	}
	return retVal;
}

EHulkResult Node::addSon(Node *node)
{
	m_sons.push_back(node);
	node->m_predecessors.push_back(this);
	return e_ok;
}

Node::SonIterator  Node::begin()
{
	SonIterator it;
	it.m_iterator = m_sons.begin();
	return it;
}

Node::SonIterator  Node::end()
{
	SonIterator it;
	it.m_iterator = m_sons.end();
	return it;
}

bool Node::deleteNode()
{
	if (m_predecessors.size() == 0 && m_sons.size() == 0)
	{
		delete this;
		return true;
	}
	else
	{
		return false;
	}
}

//!this node receives a time value and updates a node
void Node::updateNode(unsigned int time)
{
	//empty Implementation
}
//////////////////////////////
// private support function //
//////////////////////////////
EHulkResult Node::removeFromSonContainer(Node * node)
{
	EHulkResult retVal = e_ok;
	std::list<Node*>::iterator it = find(m_sons.begin(),m_sons.end(),node);
	if (m_sons.end() == it)
	{
		retVal = e_nodeNotFound;	
	}
	else
	{
		m_sons.erase(it);
		retVal = e_ok;
	}
	return retVal;
}
EHulkResult Node::removeFromPredContainer(Node * node)
{
	EHulkResult retVal = e_ok;
	std::list<Node*>::iterator it = find(m_predecessors.begin(),m_predecessors.end(),node);
	if (m_predecessors.end() == it)
	{
		retVal = e_nodeNotFound;	
	}
	else
	{
		m_predecessors.erase(it);
		retVal = e_ok;
	}
	return retVal;
}


/////////////////////////////////////////////////
//private copy constructor to prevent override //
/////////////////////////////////////////////////
Node::Node(Node & n)
{
}

Node& Node::operator=(Node & n)
{
	return *this;
}

//////////////////////////////
// internal class functions //
//////////////////////////////
Node::SonIterator & Node::SonIterator::operator++()
{
	m_iterator++;
	return *this;
}

bool operator== (Node::SonIterator & lVal,Node::SonIterator & rVal)
{
	return (lVal.m_iterator == rVal.m_iterator);
}

bool operator!= (Node::SonIterator & lVal,Node::SonIterator & rVal)
{
	return (lVal.m_iterator != rVal.m_iterator);
}


Node * operator*(Node::SonIterator & it)
{
	return (*it.m_iterator);
}

Node::SonIterator & Node::SonIterator::operator= (SonIterator & a)
{
	m_iterator = a.m_iterator;
	return *this;
}