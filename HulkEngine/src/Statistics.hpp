//////////////////////////////////////////////////////////////////
// Statistics.hpp													//
// Hulk renderer - Create by Tzachi Cohen 2013					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////


#ifndef HULK_STATISTICS_HPP
#define HULK_STATISTICS_HPP
#include <string>

#include <Windows.h>

#include "util.hpp"
#include "Stack.hpp"
#include "scene.hpp"

class Statistics
{
public:
	Statistics();
	~Statistics();
	static Statistics& getInstance() {
		static Statistics settings;
		return settings;
	}

    struct KDTreeStats
    {
        // number of node in the tree
        uint m_kdtreeNodes;
        // number of polygons in the leafs
        uint m_leafPoly;
        // average number of polygons per leaf
        float m_polyPerLeaf;
        // size of tree nodes in bytes
        uint m_treeSize;
        //dedicated polygon index buffer size in bytes
        uint m_polygonIndexBufferSize;
    };

    struct BackingStoreStats
    {
        float m_indexBufferUtilization;
        float m_vertexBufferUtilization;
    };

	void initStats(Scene * pScene);
	void frameStart();
	void frameEnd();
	void KDTreeBuildStart();
	void KDTreeBuildEnd();
	void dumpResults();
    void registerKDTreeStat(const KDTreeStats & stats);
    void registerBackingStoreStat(const BackingStoreStats & stats) { m_backingStoreStats = stats; }
	//! MT signifies this function is thread safe
	inline void countIntersectionTestMT();
	//! MT count intersection test.
	inline void countKDTreeTraverseMT();

private:
    
	//!this struct holds all the statistics collected for a single frame.
	struct FrameStats {
		//the time is took to build the KD tree
		float m_buildtime;
		//time it takes to render the tree
		float m_frameTime;
		//the average number of ray to polygon intersection
		// tests per pixel
		float m_intersectionsPerRay; 
		//the average number of visited KDTree nodes
		// per pixel
		float m_traversesPerRay;
        KDTreeStats m_treeStats;
        BackingStoreStats m_backingStoreStats;
	};
	//! current start time of KD Tree construction
	LARGE_INTEGER m_KDBuildStart;
	//! current start time of KD Tree construction
	LARGE_INTEGER m_KDBuildEnd;
	//! current frame time start
	LARGE_INTEGER m_frameStart;
	//! current frame time end
	LARGE_INTEGER m_frameEnd;

    LARGE_INTEGER m_runStart;
	//! performance counter frequency.
	float m_freq;
	//! holds the execution Stat of each frame.
	Stack<FrameStats> m_statArray;
	//the total number of ray to polygon intersection tests 
	//in the current frame;
	volatile long long m_totalIntersections;
	//the total number of KDtree traverses in current frame
	volatile long long m_totalTraverse;

	//! the number of rays in a single frame;
	float m_numRays;

    KDTreeStats     m_treeStats;
    BackingStoreStats m_backingStoreStats;

	Scene * m_pScene;
    BOOL m_statsDumped;
	std::string dumpFileName;
};


//! MT signifies this function is thread safe
inline void Statistics::countIntersectionTestMT()
{
	InterlockedIncrement64(&m_totalIntersections);
}
//! MT signifies this function is thread safe
inline void Statistics::countKDTreeTraverseMT()
{
	InterlockedIncrement64(&m_totalTraverse);
}
#endif