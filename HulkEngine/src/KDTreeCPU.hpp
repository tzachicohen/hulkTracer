//////////////////////////////////////////////////////////////////
// KDTreeCPU.hpp												//
// Hulk renderer - Create by Tzachi Cohen 2010					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////
#ifndef HULK_KDTREE_CPU_HPP
#define HULK_KDTREE_CPU_HPP

#include <vector>
#include "util.hpp"
#include "hulk_assert.h"
#include "Intersectable.hpp"
#include "Scene.hpp"


class KDTreeCPU
{
public:
	
	//this factor determines what fraction of the geomnetry will be used
	//to calculate SAH (1/c_reductionFactor).
	static const uint c_reductionFactor = 5;
	static const float c_traverseCost;
	static const float c_intersectionCost;
	//! @brief represents the positional relation between a polygon and a plane
	enum EPolyToPlane
	{
		e_abovePlane,
		e_belowPlane,
		e_intersectingPlane,
		e_coPlanar
	};

	enum EMarker
	{
		e_triangleBegin = 0 ,
		e_triangleCoPlanar = 1, //cases where the triangle is planar with split plane
		e_triangleEnd = 2,
	};

	struct SplitCandidate{
		float value;
		EMarker marker;
        unsigned int polyIndex;
		bool operator<(const SplitCandidate & b) {return value < b.value;}
	};
	
	KDTreeCPU();
	virtual ~KDTreeCPU();
	virtual void resetTree();

    virtual EHulkResult buildKDTree(Scene & scene);
	virtual EHulkResult buildKDTreeMedian();
	virtual EHulkResult buildKDTreeSAH();
    virtual EHulkResult buildKDTreeSAH_NlogN();
	bool		checkCollision(const Ray & ray,OUT HitData & hitData) const ;
     // dump tree to textual file.
    void dumpTree();

	//returns a pointer to the list of nodes in the tree
	const KDTreeNode *  getNodeArray(OUT uint & nodeCount)
	{
		nodeCount = m_treeNodes.size();
		return m_treeNodes.data();
	}
	//returns a pointer to the list of node polygons in the tree
	const uint * getPolygonArray(OUT uint & plolygonCount)
	{
		plolygonCount = m_leafPolygons.size();
		return m_leafPolygons.data();
	}

	//protected:
	struct SplitIteration
	{
		Box	nodeBox; //iteration bounding box
        float boxSurface; //surface of bounding box (only half)
		uint iterationNumber; // the depth of the iteration
		uint nodeIndex;//the index of the KD tree node in 'm_treeNodes' array
        size_t polygonsEval; // the number of polygons used to generate split candidates
		Stack<unsigned int> polygonList; //list of polygons to sort
        std::vector<SplitCandidate> candidates[3];
		bool memPool;
        void clearContainers() {
            polygonList.clear();
            candidates[0].clear();
            candidates[1].clear();
            candidates[2].clear();
			polygonsEval= 0;
			boxSurface= 0;
			nodeIndex=0;
        }
        SplitIteration(): polygonsEval(0),boxSurface(0), nodeIndex(0),memPool(false) {}
		SplitIteration & operator=(const SplitIteration & other);
	};

	//SAH implementation
	void buildNodeSAH(SplitIteration & ItrData);
    //SAH implementation
	virtual void buildNodeSAHNlogN(SplitIteration & ItrData);

	//this function takes all the geometry specified in 'SplitIteration'
	// and define a tree leaf.
	void makeLeafNode(SplitIteration & ItrData);
	
	//naive median implementation related functions
	void buildNodeMedian(unsigned int iteration, 
					unsigned int index , //index of the node in the node array 
					Stack<unsigned int> & poligonList,
					const Box & BoundingBox);
	// takes a bounding box and splits it to two sub boxes
	static void splitVolume(unsigned int iteration ,
					  const Box & boundingBox,
					  OUT Box & subBox1,
					  OUT Box & subBox2,
					  OUT AAPlane & partitionPlane);
	bool searchKDTree( const Ray & ray,const float timeStart,const float timeEnd, OUT HitData & hit) const ;
	bool searchKDTreeStackless( const Ray & ray ,const float timeStart,const float timeEnd,OUT HitData & hit) const;

	//! @brief checks the orietation of a polygon to an axis aligned plane.
	EPolyToPlane checkPolygonToAAPlane(const AAPlane & plane,const unsigned int polyIndex) const ;

	bool testNodePolygons(const KDTreeNode & node , const Ray & ray ,OUT HitData & hit) const;
	void buildPolygonCache();
	Scene * m_scene;
	// the first node is the root nodes
	Stack<KDTreeNode> m_treeNodes;
	//this array caches some calculations boosting ray to triangle intersection tests.
	Stack<PolyToRayCache> m_polygonCache;
	//this list holds which polygons held by the tree leafs
	Stack<unsigned int> m_leafPolygons; 
	//this function calculates the average number of polygons per node.
    
    //first split
    SplitIteration m_firstSplit;

	// an index for NlogN construction alogrithm, used to cache polygon to place relation
	EPolyToPlane * m_pPolyRelations; 
	uint m_polyRelationSize; // this variable holds the length of m_pPolyRelations array
    uint m_leafCount;
	
    float averagePolyPerLeaf();
};


#endif
