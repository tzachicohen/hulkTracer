
#include <vector>

#include "DeleteVisitor.hpp"

//////////////////////////////////////
//public initialization and cleanup //
//////////////////////////////////////
DeleteVisitor::DeleteVisitor ()
{
}
DeleteVisitor::~DeleteVisitor ()
{
}

////////////////////
// public methods //
////////////////////

void DeleteVisitor::preAct(Node *)
{
}

void DeleteVisitor::postAct(Node * node)
{
	//keep refernce to all of the node sons
	std::vector<Node*> nodeVec;
	Node::SonIterator it = node->begin();
	while (it != node->end())
	{
		nodeVec.push_back(*it);
		++it;
	}
	
	for (unsigned int i = 0 ; i <nodeVec.size();i++)
	{
		//remove the connection between the node and it's sons
		node->removeSon(nodeVec[i]);
		//try to delete the son node
		nodeVec[i]->deleteNode();
	}
}

bool DeleteVisitor::traverseBegin(Node *)
{
	return true;
}

void DeleteVisitor::traverseEnd(Node *)
{
}