//////////////////////////////////////////////////////////////////
// KDTreeCPUMT.hpp												//
// Hulk renderer - Create by Tzachi Cohen 2010					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////

#include "KDTreeCPUMT.hpp"
#include "SharedFunctions.hpp"

KDTreeCPUMT::KDTreeCPUMT()
{
	InitializeCriticalSection(&m_nodeCS);
	m_threadPool.initialize();
    
    m_memoryPool.initialize(20,20,false);
    
}

KDTreeCPUMT::~KDTreeCPUMT()
{
	DeleteCriticalSection(&m_nodeCS);
}

void KDTreeCPUMT::buildNodeSAHNlogN(SplitIteration & ItrData)
{
	Concurrency::TaskSet taskSet;
	BuildBranch * pBranch = new BuildBranch(taskSet);
	pBranch->m_itr = m_memoryPool.allocate();
	pBranch->m_itr->memPool = true;
	pBranch->m_itr->clearContainers();
	//pBranch->m_itr = new SplitIteration();
	*(pBranch->m_itr) = ItrData;
	pBranch->m_pTree = this;

	m_threadPool.enterLowLatencyMode();

	m_threadPool.enqueueTask(pBranch);

	taskSet.waitFortaskSet();

	m_threadPool.leaveLowLatencyMode();
}

bool KDTreeCPUMT::BuildBranch::executeImp() 
{
		//in case of absolute termination criterions
	if (m_itr->iterationNumber >= (c_maxKDTreeDepth-1) ||
		( m_itr->polygonList.size() < 10 ))
	{
		EnterCriticalSection(&(m_pTree->m_nodeCS));
		m_pTree->makeLeafNode(*m_itr);
		LeaveCriticalSection(&(m_pTree->m_nodeCS));
		//delete m_itr;
		if (m_itr->memPool) 
		{
			m_pTree->m_memoryPool.free(m_itr);
		}
		return true;
	}

 #ifdef _DEBUG
    std::vector<SplitCandidate> & cand1 = m_itr->candidates[0];
    std::vector<SplitCandidate> & cand2 = m_itr->candidates[1];
    std::vector<SplitCandidate> & cand3 = m_itr->candidates[2];
#endif
	//generate split candidates three arrays one for each dimension.
	
	float bestSplitScore =FLT_MAX;
    float bestSplitUpperSurface = 0;
    float bestSplitLowerSurface = 0;
	float bestSplitValue =0 ;
	EPlaneAxis::EPlaneAxis bestSplitDimension;
	SPVector boundingDim = m_itr->nodeBox.m_max - m_itr->nodeBox.m_min;
    float invBoxSurface = 1/m_itr->boxSurface;
	//iterate over three dimensions
	for (uint j =0 ; j < 3;j++) {
		uint primCountBelow =0 ;
		uint primCountCoPlanar = 0;
		uint primCountAbove = 0;
	
		// read Havran's '../HulkTracer/doc/nlog(n).pdf' paper to understand how the split plane is chosen
		uint counters[3] = {0};
		//counters[e_triangleBegin]  --the number of primitives starting with this candidate
		//counters[e_triangleCoPlanar] --the number of primitives co-planar with this candidate
		//counters[e_triangleEnd] -- the number of  primitves ending with this candidate

		uint PPlusPrev = 0 ; //the number of primitives starting with the previous candidate
		uint PCoPlanarPrev = 0 ; //the number of primitives co-planar with the previous candidate
		uint PminusPrev = 0 ; //the number of  primitves ending with the previous candidate

		//the invariant dimensions
        float x = boundingDim.vector[(j +1) % 3];
        float y = boundingDim.vector[(j +2) % 3];
        float XplusY = x+y;
        float intersectionPlaneArea = x*y;

		Box lowerBox = m_itr->nodeBox;
		auto it = m_itr->candidates[j].begin();
		bool firstCandidate =true;
		while ( it != m_itr->candidates[j].end()) 
		{
			float currentValue = it->value;
			//todo manipulate primCounts + optimize for cases the there are many candidate with the same value
			while (it != m_itr->candidates[j].end() && it->value == currentValue )
			{
				counters[it->marker]++;
				it++;
			}
			primCountBelow = primCountBelow + PCoPlanarPrev + PPlusPrev;
			primCountCoPlanar = PCoPlanarPrev;
			primCountAbove = primCountAbove - counters[e_triangleCoPlanar] - counters[e_triangleEnd];

			if (firstCandidate){
				//with the first candidate primitive count should be updated slightly different.
				primCountBelow = 0;
				primCountCoPlanar = counters[e_triangleCoPlanar];
                //todo : remove the convesion from size_t in to unsigned int
				primCountAbove =  ((unsigned int)m_itr->polygonsEval) - counters[e_triangleCoPlanar];
				firstCandidate =false;
			}
			PPlusPrev = counters[e_triangleBegin];
			PminusPrev = counters[e_triangleEnd];
			PCoPlanarPrev = counters[e_triangleCoPlanar];
			
			//zero the counters for next iteration
			memset(counters,0,sizeof(counters));

		    float upperSurface = intersectionPlaneArea + (m_itr->nodeBox.m_max.vector[j] - currentValue)*(XplusY);
            float lowerSurface = intersectionPlaneArea + (currentValue - m_itr->nodeBox.m_min.vector[j] )*(XplusY);
			float costBelow = lowerSurface*(primCountBelow );
			float costAbove = upperSurface*(primCountAbove );
			float costIntersect = intersectionPlaneArea *primCountCoPlanar;
			float SAHScore = c_traverseCost + c_intersectionCost * ((costIntersect+costBelow+costAbove)*invBoxSurface);
			// remember the best split
			if (SAHScore <  bestSplitScore) {
				bestSplitScore = SAHScore;
				bestSplitValue = currentValue;
				bestSplitDimension = (EPlaneAxis::EPlaneAxis) j;
                bestSplitUpperSurface = upperSurface;
                bestSplitLowerSurface = lowerSurface;
			}
		} // while ( it != candidates.end()) 
	} //for (uint j =0 ; j < 3;j++) {

	//automatic termination criterion
	float leafcost = c_intersectionCost* m_itr->polygonsEval;
	if (bestSplitScore >= leafcost)
	{
		EnterCriticalSection(&(m_pTree->m_nodeCS));
		m_pTree->makeLeafNode(*m_itr);
		LeaveCriticalSection(&(m_pTree->m_nodeCS));
		if (m_itr->memPool) 
		{
			m_pTree->m_memoryPool.free(m_itr);
		}
		return true;
	}
	//now split the geometry according to the chosen split plane and reiterate.
	m_pTree->m_treeNodes[m_itr->nodeIndex].data.junction.m_planeValue = bestSplitValue;
	m_pTree->m_treeNodes[m_itr->nodeIndex].m_flags = bestSplitDimension;
	m_pTree->m_treeNodes[m_itr->nodeIndex].m_flags |= EPlaneAxis::e_junction;

	//divid the polygons in to two lists
	BuildBranch * aboveSplit = new BuildBranch(m_taskSet);
	SplitIteration above,below;
	// the split iteration tructure can either be allocated in the pool or on the stack.
	// if we successfully allocated in the pool we will enqueue the task to be executed by other threads.
	// If the 'SplitIteration' struct is on the stack the the member 'mempool' is false and we will execute the split
	// iteration in the context of the current thread using recursion.
	aboveSplit->m_itr = m_pTree->m_memoryPool.allocate();
	if (aboveSplit->m_itr)
	{
		aboveSplit->m_itr->memPool = true;
		aboveSplit->m_itr->clearContainers();
	}
	else
	{
		aboveSplit->m_itr = &above;
	}

    BuildBranch * belowSplit = new BuildBranch(m_taskSet);
    belowSplit->m_itr = m_pTree->m_memoryPool.allocate();
	if (belowSplit->m_itr)
	{
		belowSplit->m_itr->memPool = true;
		belowSplit->m_itr->clearContainers();
	}
	else
	{
		belowSplit->m_itr = &below;
	}
    
	belowSplit->m_itr->iterationNumber = m_itr->iterationNumber +1;
    aboveSplit->m_itr->iterationNumber = m_itr->iterationNumber +1;

	AAPlane splitPlane;
	splitPlane.planeAxis = bestSplitDimension;
	splitPlane.value = bestSplitValue;
	EPolyToPlane * pPolyRelations = new EPolyToPlane[m_pTree->m_scene->m_vertexIndices.size()];
	for (uint i =0 ; i < m_itr->polygonList.size();i ++)
	{	
		const unsigned int polyIndex = m_itr->polygonList[i];
		EPolyToPlane relation = m_pTree->checkPolygonToAAPlane(splitPlane,polyIndex);
		//todo: make thread safe
        pPolyRelations[polyIndex] = relation;
		switch (relation)
		{
		case e_abovePlane:
			aboveSplit->m_itr->polygonList.push_back(polyIndex);
			break;
		case e_belowPlane:
			belowSplit->m_itr->polygonList.push_back(polyIndex);
			break;
		case e_intersectingPlane:
        case e_coPlanar:
			aboveSplit->m_itr->polygonList.push_back(polyIndex);
			belowSplit->m_itr->polygonList.push_back(polyIndex);
			break;
		default:
			HULK_ASSERT(false,"undefined polygon to plane orientation");
		}
	}


    //rebuild split candidate lists
    //iterate over three dimensions
	for (uint j =0 ; j < 3;j++) {
		//reserve memory in advance
		aboveSplit->m_itr->candidates[j].reserve( m_itr->candidates[j].size());
		belowSplit->m_itr->candidates[j].reserve( m_itr->candidates[j].size());
        for (auto  it =  m_itr->candidates[j].begin();it !=   m_itr->candidates[j].end();it++)
        {
            switch(pPolyRelations[it->polyIndex])
		    {
		    case e_abovePlane:
			    aboveSplit->m_itr->candidates[j].push_back(*it);
			    break;
		    case e_belowPlane:
			    belowSplit->m_itr->candidates[j].push_back(*it);
			    break;
		    case e_intersectingPlane:
                 
                if (j == bestSplitDimension) {
                    SplitCandidate tmp = *it;
                    tmp.value = MAX(bestSplitValue,it->value);
			        aboveSplit->m_itr->candidates[j].push_back(tmp);
                    tmp.value = MIN(bestSplitValue,it->value);
			        belowSplit->m_itr->candidates[j].push_back(tmp);
                }
                else
                {
                    aboveSplit->m_itr->candidates[j].push_back(*it);
                    belowSplit->m_itr->candidates[j].push_back(*it);
                }
			    break;
            }
        }
        aboveSplit->m_itr->polygonsEval += aboveSplit->m_itr->candidates[j].size();
        belowSplit->m_itr->polygonsEval += belowSplit->m_itr->candidates[j].size();
    }

	delete [] pPolyRelations;

    aboveSplit->m_itr->polygonsEval /= 6;
    belowSplit->m_itr->polygonsEval /= 6;

	//create sub branches in the tree
	if (aboveSplit->m_itr->polygonList.size() > 0)
	{
		aboveSplit->m_itr->nodeBox = m_itr->nodeBox;
        aboveSplit->m_itr->boxSurface = bestSplitUpperSurface;
		aboveSplit->m_itr->nodeBox.m_min[bestSplitDimension] = bestSplitValue;
		aboveSplit->m_pTree = m_pTree;
		EnterCriticalSection(&(m_pTree->m_nodeCS));
		unsigned int aboveIndex = m_pTree->m_treeNodes.size();
		aboveSplit->m_itr->nodeIndex = aboveIndex;
		KDTreeNode tmp2;
		m_pTree->m_treeNodes.push_back(tmp2);
		m_pTree->m_treeNodes[m_itr->nodeIndex].data.junction.m_abovePlane = aboveIndex;
		LeaveCriticalSection(&(m_pTree->m_nodeCS));

		if (aboveSplit->m_itr->memPool)
		{
			m_pTree->m_threadPool.enqueueTask(aboveSplit);
		}
		else
		{
			aboveSplit->executeImp();
			delete aboveSplit;
		}
	}
	else
	{
		if (aboveSplit->m_itr->memPool) {
			m_pTree->m_memoryPool.free(aboveSplit->m_itr);
		}
		delete aboveSplit;
	}

	if (belowSplit->m_itr->polygonList.size() > 0)
	{
		belowSplit->m_itr->nodeBox = m_itr->nodeBox;
        belowSplit->m_itr->boxSurface = bestSplitLowerSurface;
		belowSplit->m_itr->nodeBox.m_max[bestSplitDimension] = bestSplitValue;
		belowSplit->m_pTree = m_pTree;
		EnterCriticalSection(&(m_pTree->m_nodeCS));
		unsigned int belowIndex = m_pTree->m_treeNodes.size();
		belowSplit->m_itr->nodeIndex = belowIndex;
		KDTreeNode tmp2;
		m_pTree->m_treeNodes.push_back(tmp2);
		m_pTree->m_treeNodes[m_itr->nodeIndex].data.junction.m_belowPlane = belowIndex;
		LeaveCriticalSection(&(m_pTree->m_nodeCS));

		if (belowSplit->m_itr->memPool)
		{
			m_pTree->m_threadPool.enqueueTask(belowSplit);
		}
		else
		{
			belowSplit->executeImp();
			delete belowSplit;
		}
	}
	else
	{
		if (belowSplit->m_itr->memPool) {
			m_pTree->m_memoryPool.free(belowSplit->m_itr);
		}
		delete belowSplit;
	}
    
	if (m_itr->memPool) 
	{
		m_pTree->m_memoryPool.free(m_itr);
	}

	return true;
}