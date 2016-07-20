//////////////////////////////////////////////////////////////////
// KDTreeCPUMT.hpp												//
// Hulk renderer - Create by Tzachi Cohen 2010					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////
#ifndef HULK_KDTREE_CPU_MT_HPP
#define HULK_KDTREE_CPU_MT_HPP

#include "KDTreeCPU.hpp"
#include "ThreadPool.hpp"
#include "MemoryPool.hpp"

class KDTreeCPUMT : public KDTreeCPU
{
public:


	KDTreeCPUMT();
	virtual ~KDTreeCPUMT();

	class BuildBranch : public Concurrency::Task
	{
	public:
		virtual ~BuildBranch() {} 
		BuildBranch(Concurrency::TaskSet &taskSet) :Task(taskSet) {}
		SplitIteration* m_itr;
		KDTreeCPUMT* m_pTree;
		virtual bool executeImp() ;
	};

protected:
	 //SAH implementation
	virtual void buildNodeSAHNlogN(SplitIteration & ItrData);
    //we have a pool for every level of the tree
    Concurrency::MemoryPool<SplitIteration> m_memoryPool;
	Concurrency::ThreadPool m_threadPool;
	CRITICAL_SECTION m_nodeCS;
};


#endif //HULK_KDTREE_GPU_HPP