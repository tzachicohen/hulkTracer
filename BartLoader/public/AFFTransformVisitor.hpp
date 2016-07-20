//////////////////////////////////////////////////////////////////
// visitor.hpp													//
// Hulk renderer - Create by Tzachi Cohen 2011					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////
#ifndef AFF_TRANSFORM_VISITOR_HPP
#define AFF_TRANSFORM_VISITOR_HPP

#include "node.hpp"
#include "Visitor.hpp"
#include "stack.hpp"
#include "SPMatrix.hpp"
#include "scene.hpp"

class AFFTransformVisitor : public Visitor
{
public:
	//////////////////////////////////////
	//public initialization and cleanup //
	//////////////////////////////////////
	
	AFFTransformVisitor (Scene & scene,AFFParser & parser);
	virtual ~AFFTransformVisitor ();

	////////////////////
	// public methods //
	////////////////////
	virtual bool traverseBegin(Node *);
	virtual void traverseEnd(Node *);
	virtual void preAct(Node *) ;
	virtual void postAct(Node *);

	Scene * m_pScene;
	AFFParser * m_pParser;
	bool m_calcStatics ;
private:
	Stack<SPMatrix> m_matrixStack;
};

#endif
