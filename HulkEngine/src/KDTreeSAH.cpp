
#include <vector>
#include <algorithm>
#include <string>

#include "KDTreeCPU.hpp"
#include "SharedFunctions.hpp"
#include "Statistics.hpp"

EHulkResult KDTreeCPU::buildKDTreeSAH()
{	


    if (m_firstSplit.polygonList.size() == 0) {
	    //build a list of all ploygon indices.
	    for (unsigned int i = 0 ; i< m_scene->m_vertexIndices.size();i++)
	    {
		    m_firstSplit.polygonList.push_back(i);
	    }
    }
	//set root node.
	KDTreeNode root;
	m_treeNodes.push_back(root);
	
	m_firstSplit.nodeIndex = 0;
	m_firstSplit.iterationNumber = 0;
	m_firstSplit.nodeBox = m_scene->m_sceneBox;
    m_firstSplit.boxSurface = getHalfBoxSurface(m_scene->m_sceneBox);

	buildNodeSAH(m_firstSplit);

	return e_ok;
}

void KDTreeCPU::buildNodeSAH(SplitIteration & itrData)
{
	//in case of absolute termination criterions
	if (itrData.iterationNumber >= c_maxKDTreeDepth ||
		( itrData.polygonList.size() < 10 ))
	{
		makeLeafNode(itrData);
		return;
	}

	//generate split candidates three arrays one for each dimension.
	std::vector<SplitCandidate> candidates[3];
#ifdef _DEBUG
	std::vector<SplitCandidate> & cand0 = candidates[0];
	std::vector<SplitCandidate> & cand1 = candidates[1];
	std::vector<SplitCandidate> & cand2 = candidates[2];
#endif
	for (uint i =0 ; i < itrData.polygonList.size();i +=c_reductionFactor)
	{
		uint polyIndex = itrData.polygonList[i];
		SPVector min,max;
		const SPVector  p1 =  m_scene->m_vertices[m_scene->m_vertexIndices[polyIndex].indices[0]];
		const SPVector  p2 =  m_scene->m_vertices[m_scene->m_vertexIndices[polyIndex].indices[1]];
		const SPVector  p3 =  m_scene->m_vertices[m_scene->m_vertexIndices[polyIndex].indices[2]];
		//get the minimum and the maximum vertices
		min = sse::min(sse::min(p1.vec,p2.vec),p3.vec);
		max = sse::max(sse::max(p1.vec,p2.vec),p3.vec);
		//clip the vertex to the bounding box
		min = sse::max(min.vec,itrData.nodeBox.m_min.vec);
		max = sse::min(max.vec,itrData.nodeBox.m_max.vec);
		SplitCandidate triCandidate;
		//in case the the triangle is planar with the split axis
		for (uint j =0 ; j < 3;j++) {
			if ( fabs (min.vector[j] - max.vector[j]) <= FLT_EPSILON) {
				triCandidate.marker = e_triangleCoPlanar;
				triCandidate.value  = min.vector[j];
				candidates[j].push_back(triCandidate);
			}
			else {
				triCandidate.marker = e_triangleBegin;
				triCandidate.value = min.vector[j];
				candidates[j].push_back(triCandidate);
				triCandidate.marker = e_triangleEnd;
				triCandidate.value = max.vector[j];
				candidates[j].push_back(triCandidate);
			}
		}

	}
	float bestSplitScore =FLT_MAX;
    float bestSplitUpperSurface = 0;
    float bestSplitLowerSurface = 0;
	float bestSplitValue =0 ;
    //dimensions of the bounding box
    SPVector boundingDim = itrData.nodeBox.m_max - itrData.nodeBox.m_min;
	EPlaneAxis::EPlaneAxis bestSplitDimension;
	float invBoxSurface = 1/itrData.boxSurface;
	//iterate over thre dimensions
	for (uint j =0 ; j < 3;j++) {
		//now we sort all the split candidates
		std::sort<std::vector<SplitCandidate>::iterator>(candidates[j].begin(),candidates[j].end());

		uint primCountBelow =0 ;
		uint primCountCoPlanar = 0;
		uint primCountAbove = 0;
	
		// read Havran's '../HulkTracer/doc/nlog(n).pdf' paper to understand how the split plane is chosen
		uint PPlus = 0 ; //the number of primitives starting with this candidate
		uint PCoPlanar = 0 ; //the number of primitives co-planar with this candidate
		uint Pminus = 0 ; //the number of  primitves ending with this candidate

		uint PPlusPrev = 0 ; //the number of primitives starting with the previous candidate
		uint PCoPlanarPrev = 0 ; //the number of primitives co-planar with the previous candidate
		uint PminusPrev = 0 ; //the number of  primitves ending with the previous candidate

		
		//the invariant dimensions
        float x = boundingDim.vector[(j +1) % 3];
        float y = boundingDim.vector[(j +2) % 3];
        float XplusY = x+y;
        float intersectionPlaneArea = x*y;

		auto it = candidates[j].begin();
		bool firstCandidate =true;
		while ( it != candidates[j].end()) 
		{
			float currentValue = it->value;
			//todo manipulate primCounts + optimize for cases the there are many candidate with the same value
			while (it != candidates[j].end() && it->value == currentValue )
			{
				switch (it->marker)
				{
					case e_triangleBegin:
						PPlus++;
						break;
					case e_triangleCoPlanar: //cases where the triangle is planar with split plane
						PCoPlanar++;
						break;
					case e_triangleEnd:
						Pminus++;
						break;
						//todo: add support for boxes.
					default:{
							HULK_ASSERT(false,"should not be here undefined marker\n")
						}
				}
				it++;
			}
			primCountBelow = primCountBelow + PCoPlanarPrev + PPlusPrev;
			primCountCoPlanar = PCoPlanarPrev;
			primCountAbove = primCountAbove - PCoPlanar - Pminus;

			if (firstCandidate){
				//with the first candidate primitive count should be updated slightly different.
				primCountBelow = 0;
				primCountCoPlanar = PCoPlanar;
				primCountAbove =  itrData.polygonList.size()/c_reductionFactor - PCoPlanar;
				firstCandidate =false;
			}
			PPlusPrev = PPlus;
			PminusPrev = Pminus;
			PCoPlanarPrev = PCoPlanar;
			//zero the counters for next iteration
			PPlus = Pminus = PCoPlanar = 0;

            float upperSurface = intersectionPlaneArea + (itrData.nodeBox.m_max.vector[j] - currentValue)*(XplusY);
            float lowerSurface = intersectionPlaneArea + (currentValue - itrData.nodeBox.m_min.vector[j] )*(XplusY);
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
	float leafcost = c_intersectionCost* (  itrData.polygonList.size()/c_reductionFactor);
	if (bestSplitScore >= leafcost)
	{
		makeLeafNode(itrData);
		return;
	}
	//now split the geometry according to the chosen split plane and reiterate.
	m_treeNodes[itrData.nodeIndex].data.junction.m_planeValue = bestSplitValue;
	m_treeNodes[itrData.nodeIndex].m_flags = bestSplitDimension;
	m_treeNodes[itrData.nodeIndex].m_flags |= EPlaneAxis::e_junction;

	//divid the polygons in to two lists
	SplitIteration aboveSplit,belowSplit;
	AAPlane splitPlane;
	splitPlane.planeAxis = bestSplitDimension;
	splitPlane.value = bestSplitValue;
	for (uint i =0 ; i < itrData.polygonList.size();i ++)
	{	
		const unsigned int poly = itrData.polygonList[i];
		EPolyToPlane relation = checkPolygonToAAPlane(splitPlane,poly);
		switch (relation)
		{
		case e_abovePlane:
			aboveSplit.polygonList.push_back(poly);
			break;
		case e_belowPlane:
			belowSplit.polygonList.push_back(poly);
			break;
		case e_intersectingPlane:
        case e_coPlanar:
			aboveSplit.polygonList.push_back(poly);
			belowSplit.polygonList.push_back(poly);
			break;
		default:
			HULK_ASSERT(false,"undefined polygon to plane orientation");
		}
	}


	//create sub branches in the tree
	if (aboveSplit.polygonList.size() > 0)
	{
		aboveSplit.iterationNumber = itrData.iterationNumber +1;
		aboveSplit.nodeBox = itrData.nodeBox;
        aboveSplit.boxSurface = bestSplitUpperSurface;
		aboveSplit.nodeBox.m_min[bestSplitDimension] = bestSplitValue;
		unsigned int aboveIndex = m_treeNodes.size();
		aboveSplit.nodeIndex = aboveIndex;
		KDTreeNode tmp2;
		m_treeNodes.push_back(tmp2);
		m_treeNodes[itrData.nodeIndex].data.junction.m_abovePlane = aboveIndex;
		buildNodeSAH(aboveSplit);
	}
	if (belowSplit.polygonList.size() > 0)
	{
		belowSplit.iterationNumber = itrData.iterationNumber +1;
		belowSplit.nodeBox = itrData.nodeBox;
        belowSplit.boxSurface = bestSplitLowerSurface;
		belowSplit.nodeBox.m_max[bestSplitDimension] = bestSplitValue;
		unsigned int belowIndex = m_treeNodes.size();
		belowSplit.nodeIndex = belowIndex;
		KDTreeNode tmp2;
		m_treeNodes.push_back(tmp2);
		m_treeNodes[itrData.nodeIndex].data.junction.m_belowPlane = belowIndex;
		buildNodeSAH(belowSplit);
	}
}


EHulkResult KDTreeCPU::buildKDTreeSAH_NlogN()
{
    if (m_firstSplit.polygonList.size() == 0) {
	    //build a list of all ploygon indices.
	    for (unsigned int i = 0 ; i< m_scene->m_vertexIndices.size();i++)
	    {
		    m_firstSplit.polygonList.push_back(i);
	    }
    }
	//allocate polygon to plane relation cache for the split candidate sort.
	if (m_polyRelationSize != m_scene->m_vertexIndices.size()) {
		if (NULL != m_pPolyRelations) {
			delete m_pPolyRelations;
		}
		m_pPolyRelations = new EPolyToPlane[m_scene->m_vertexIndices.size()];
		m_polyRelationSize = m_scene->m_vertexIndices.size();
	}
	m_firstSplit.candidates[0].clear();
    m_firstSplit.candidates[1].clear();
    m_firstSplit.candidates[2].clear();

	m_firstSplit.nodeIndex = 0;
	m_firstSplit.iterationNumber = 0;
	m_firstSplit.nodeBox = m_scene->m_sceneBox;
    m_firstSplit.boxSurface = getBoxSurface(m_scene->m_sceneBox);
	m_firstSplit.polygonsEval = 0;
    for (uint i =0 ; i < m_firstSplit.polygonList.size();i +=c_reductionFactor)
	{
        m_firstSplit.polygonsEval++;
		uint polyIndex = m_firstSplit.polygonList[i];
		SPVector min,max;
		const SPVector  p1 =  m_scene->m_vertices[m_scene->m_vertexIndices[polyIndex].indices[0]];
		const SPVector  p2 =  m_scene->m_vertices[m_scene->m_vertexIndices[polyIndex].indices[1]];
		const SPVector  p3 =  m_scene->m_vertices[m_scene->m_vertexIndices[polyIndex].indices[2]];
		//get the minimum and the maximum vertices
		min = sse::min(sse::min(p1.vec,p2.vec),p3.vec);
		max = sse::max(sse::max(p1.vec,p2.vec),p3.vec);
		
		SplitCandidate triCandidate;
        triCandidate.polyIndex = polyIndex;
		//in case the the triangle is planar with the split axis
		for (uint j =0 ; j < 3;j++) {
				triCandidate.marker = e_triangleBegin;
				triCandidate.value = min.vector[j];
				m_firstSplit.candidates[j].push_back(triCandidate);
				triCandidate.marker = e_triangleEnd;
				triCandidate.value = max.vector[j];
				m_firstSplit.candidates[j].push_back(triCandidate);
		}
	}
#ifdef _DEBUG
    std::vector<SplitCandidate> & cand1 = m_firstSplit.candidates[0];
    std::vector<SplitCandidate> & cand2 = m_firstSplit.candidates[1];
    std::vector<SplitCandidate> & cand3 = m_firstSplit.candidates[2];
#endif

    for (int j =0 ; j < 3;j++) {
    //now we sort all the split candidates
		std::sort<std::vector<SplitCandidate>::iterator>(m_firstSplit.candidates[j].begin(),m_firstSplit.candidates[j].end());
    }
	m_treeNodes.reserve(5000);
	buildNodeSAHNlogN(m_firstSplit);

	return e_ok;
}

void KDTreeCPU::buildNodeSAHNlogN(SplitIteration & itrData)
{
	//in case of absolute termination criterions
	if (itrData.iterationNumber >= c_maxKDTreeDepth ||
		( itrData.polygonList.size() < 10 ))
	{
		makeLeafNode(itrData);
		return;
	}

 #ifdef _DEBUG
    std::vector<SplitCandidate> & cand1 = itrData.candidates[0];
    std::vector<SplitCandidate> & cand2 = itrData.candidates[1];
    std::vector<SplitCandidate> & cand3 = itrData.candidates[2];
#endif
	//generate split candidates three arrays one for each dimension.
	
	float bestSplitScore =FLT_MAX;
    float bestSplitUpperSurface = 0;
    float bestSplitLowerSurface = 0;
	float bestSplitValue =0 ;
	EPlaneAxis::EPlaneAxis bestSplitDimension;
	SPVector boundingDim = itrData.nodeBox.m_max - itrData.nodeBox.m_min;
    float invBoxSurface = 1/itrData.boxSurface;
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

		Box lowerBox = itrData.nodeBox;
		auto it = itrData.candidates[j].begin();
		bool firstCandidate =true;
		while ( it != itrData.candidates[j].end()) 
		{
			float currentValue = it->value;
			//todo manipulate primCounts + optimize for cases the there are many candidate with the same value
			while (it != itrData.candidates[j].end() && it->value == currentValue )
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
				primCountAbove =  ((unsigned int)itrData.polygonsEval) - counters[e_triangleCoPlanar];
				firstCandidate =false;
			}
			PPlusPrev = counters[e_triangleBegin];
			PminusPrev = counters[e_triangleEnd];
			PCoPlanarPrev = counters[e_triangleCoPlanar];
			
			//zero the counters for next iteration
			memset(counters,0,sizeof(counters));

		    float upperSurface = intersectionPlaneArea + (itrData.nodeBox.m_max.vector[j] - currentValue)*(XplusY);
            float lowerSurface = intersectionPlaneArea + (currentValue - itrData.nodeBox.m_min.vector[j] )*(XplusY);
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
	float leafcost = c_intersectionCost* itrData.polygonsEval;
	if (bestSplitScore >= leafcost)
	{
		makeLeafNode(itrData);
		return;
	}
	//now split the geometry according to the chosen split plane and reiterate.
	m_treeNodes[itrData.nodeIndex].data.junction.m_planeValue = bestSplitValue;
	m_treeNodes[itrData.nodeIndex].m_flags = bestSplitDimension;
	m_treeNodes[itrData.nodeIndex].m_flags |= EPlaneAxis::e_junction;

	//divid the polygons in to two lists
	SplitIteration aboveSplit,belowSplit;

	AAPlane splitPlane;
	splitPlane.planeAxis = bestSplitDimension;
	splitPlane.value = bestSplitValue;
  
	for (uint i =0 ; i < itrData.polygonList.size();i ++)
	{	
		const unsigned int polyIndex = itrData.polygonList[i];
		EPolyToPlane relation = checkPolygonToAAPlane(splitPlane,polyIndex);
        m_pPolyRelations[polyIndex] = relation;
		switch (relation)
		{
		case e_abovePlane:
			aboveSplit.polygonList.push_back(polyIndex);
			break;
		case e_belowPlane:
			belowSplit.polygonList.push_back(polyIndex);
			break;
		case e_intersectingPlane:
        case e_coPlanar:
			aboveSplit.polygonList.push_back(polyIndex);
			belowSplit.polygonList.push_back(polyIndex);
			break;
		default:
			HULK_ASSERT(false,"undefined polygon to plane orientation");
		}
	}

 #ifdef _DEBUG
    std::vector<SplitCandidate> & bcand1 = belowSplit.candidates[0];
    std::vector<SplitCandidate> & bcand2 = belowSplit.candidates[1];
    std::vector<SplitCandidate> & bcand3 = belowSplit.candidates[2];
#endif
    //rebuild split candidate lists
    //iterate over three dimensions
	for (uint j =0 ; j < 3;j++) {
		//reserve memory in advance
		aboveSplit.candidates[j].reserve( itrData.candidates[j].size());
		belowSplit.candidates[j].reserve( itrData.candidates[j].size());
        for (auto  it =  itrData.candidates[j].begin();it !=   itrData.candidates[j].end();it++)
        {
            switch(m_pPolyRelations[it->polyIndex])
		    {
		    case e_abovePlane:
			    aboveSplit.candidates[j].push_back(*it);
			    break;
		    case e_belowPlane:
			    belowSplit.candidates[j].push_back(*it);
			    break;
		    case e_intersectingPlane:
                 
                if (j == bestSplitDimension) {
                    SplitCandidate tmp = *it;
                    tmp.value = MAX(bestSplitValue,it->value);
			        aboveSplit.candidates[j].push_back(tmp);
                    tmp.value = MIN(bestSplitValue,it->value);
			        belowSplit.candidates[j].push_back(tmp);
                }
                else
                {
                    aboveSplit.candidates[j].push_back(*it);
                    belowSplit.candidates[j].push_back(*it);
                }
			    break;
            }
        }
        aboveSplit.polygonsEval += aboveSplit.candidates[j].size();
        belowSplit.polygonsEval += belowSplit.candidates[j].size();
    }

     aboveSplit.polygonsEval /= 6;
     belowSplit.polygonsEval /= 6;

	//create sub branches in the tree
	if (aboveSplit.polygonList.size() > 0)
	{
		aboveSplit.iterationNumber = itrData.iterationNumber +1;
		aboveSplit.nodeBox = itrData.nodeBox;
        aboveSplit.boxSurface = bestSplitUpperSurface;
		aboveSplit.nodeBox.m_min[bestSplitDimension] = bestSplitValue;
		unsigned int aboveIndex = m_treeNodes.size();
		aboveSplit.nodeIndex = aboveIndex;
		KDTreeNode tmp2;
		m_treeNodes.push_back(tmp2);
		m_treeNodes[itrData.nodeIndex].data.junction.m_abovePlane = aboveIndex;
		buildNodeSAHNlogN(aboveSplit);
	}
	if (belowSplit.polygonList.size() > 0)
	{
		belowSplit.iterationNumber = itrData.iterationNumber +1;
		belowSplit.nodeBox = itrData.nodeBox;
        belowSplit.boxSurface = bestSplitLowerSurface;
		belowSplit.nodeBox.m_max[bestSplitDimension] = bestSplitValue;
		unsigned int belowIndex = m_treeNodes.size();
		belowSplit.nodeIndex = belowIndex;
		KDTreeNode tmp2;
		m_treeNodes.push_back(tmp2);
		m_treeNodes[itrData.nodeIndex].data.junction.m_belowPlane = belowIndex;
		buildNodeSAHNlogN(belowSplit);
	}
}