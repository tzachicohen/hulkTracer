//////////////////////////////////////////////////////////////////
// KDTreeCPU.hpp												//
// Hulk renderer - Create by Tzachi Cohen 2010					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////

#include <vector>
#include <algorithm>
#include <string>
#include <stdio.h>
#include "KDTreeCPU.hpp"
#include "SharedFunctions.hpp"
#include "Statistics.hpp"
#include "Settings.hpp"
#include "SseUtils.hpp"
const float KDTreeCPU::c_traverseCost = 1.0f;
const float KDTreeCPU::c_intersectionCost = 5.0f;

using namespace sse;

KDTreeCPU::KDTreeCPU()
:m_scene(NULL)
,m_pPolyRelations(NULL)
,m_polyRelationSize (0)
,m_leafCount(0)
{

}
KDTreeCPU::~KDTreeCPU()
{
	resetTree();
	if (NULL != m_pPolyRelations) {
		delete [] m_pPolyRelations;
	}

}

void KDTreeCPU::resetTree()
{
	m_treeNodes.clear();
	m_leafPolygons.clear();
    m_leafCount = 0;
}

void KDTreeCPU::makeLeafNode(SplitIteration & itrData)
{
	m_treeNodes[itrData.nodeIndex].m_flags = EPlaneAxis::e_leaf;
    m_treeNodes[itrData.nodeIndex].data.leaf.m_leafID = m_leafCount++;
	m_treeNodes[itrData.nodeIndex].data.leaf.m_polygonCount = itrData.polygonList.size();
	m_treeNodes[itrData.nodeIndex].data.leaf.m_startIndex = m_leafPolygons.size();

	//move all the polygon indices to the global list
	for (uint i = 0 ; i < itrData.polygonList.size();i++) {
		m_leafPolygons.push_back( itrData.polygonList[i]);
	}
}



EHulkResult KDTreeCPU::buildKDTree(Scene & scene)
{
    m_scene = &scene;

	resetTree();

    //rebuild the ray to polygon intersection test acceleration structure.
    if (0 == wcscmp(Settings::getInstance().getValue(L"--renderType"),L"CPUTrace")) {
	    m_polygonCache.resize(m_scene->m_vertexIndices.size());
	    buildPolygonCache();
    }

    //set root node.
	KDTreeNode root;
	m_treeNodes.push_back(root);

    if (0 == wcscmp(Settings::getInstance().getValue(L"--treeType"),L"median")) {
        buildKDTreeMedian();
    }
    if (0 == wcscmp(Settings::getInstance().getValue(L"--treeType"),L"SAH")) {
        buildKDTreeSAH();
    }
     if (0 == wcscmp(Settings::getInstance().getValue(L"--treeType"),L"SAHNlogN")) {
        buildKDTreeSAH_NlogN();
    }
    Statistics::KDTreeStats stats;
    stats.m_kdtreeNodes = m_treeNodes.size();
    stats.m_treeSize = m_treeNodes.size()*sizeof(KDTreeNode);
    stats.m_leafPoly = m_leafPolygons.size();
    stats.m_polygonIndexBufferSize = m_leafPolygons.size()*sizeof(uint);
    Statistics::getInstance().registerKDTreeStat(stats);

    return e_ok;
}

EHulkResult KDTreeCPU::buildKDTreeMedian()
{

	//build a list of all ploygon indices.
	//todo: cache this array.
	Stack<unsigned int> polyList ; 
	for (unsigned int i = 0 ; i< m_scene->m_vertexIndices.size();i++)
	{
		polyList.push_back(i);
	}
	
	buildNodeMedian(0,0,polyList,m_scene->m_sceneBox);

	return e_ok;
}


//this function calculates the average number of polygons per node.
float KDTreeCPU::averagePolyPerLeaf()
{
	uint leafCount =0;
	for (uint i = 0 ; i < m_treeNodes.size();i++) {
		if (m_treeNodes[i].m_flags & EPlaneAxis::e_leaf) {
			leafCount++;
		}
	}
	return ((float)m_leafPolygons.size()) / ((float) leafCount) ;
}
void KDTreeCPU::buildNodeMedian(unsigned int iteration , unsigned int  nodeIndex , Stack<unsigned int> & polygonList,const Box & BoundingBox)
{
	//recursive stop condition
	if (c_maxKDTreeDepth == iteration || polygonList.size() < 10)
	{
		m_treeNodes[nodeIndex].m_flags = EPlaneAxis::e_leaf;
        m_treeNodes[nodeIndex].data.leaf.m_leafID = m_leafCount++;
		m_treeNodes[nodeIndex].data.leaf.m_polygonCount = polygonList.size();
		m_treeNodes[nodeIndex].data.leaf.m_startIndex = m_leafPolygons.size();
		//move all the polygon indices to the global list
		for (uint i = 0 ; i < polygonList.size();i++) {
			m_leafPolygons.push_back(polygonList[i]);
		}
	}
	else
	{
		Stack<unsigned int> aboveList,belowList;
		Box	aboveBox , belowBox;

		AAPlane splitPlane;
		splitVolume(iteration,BoundingBox,aboveBox,belowBox,splitPlane);
		
		m_treeNodes[nodeIndex].data.junction.m_planeValue = splitPlane.value;
		m_treeNodes[nodeIndex].m_flags = splitPlane.planeAxis;
		m_treeNodes[nodeIndex].m_flags |= EPlaneAxis::e_junction;

		//divid the polygons in the list
        for (uint i =0 ; i < polygonList.size();i ++)
		{	
			const unsigned int poly = polygonList[i];	
			EPolyToPlane relation = checkPolygonToAAPlane(splitPlane,poly);
			switch (relation)
			{
			case e_abovePlane:
				aboveList.push_back(poly);
				break;
			case e_belowPlane:
				belowList.push_back(poly);
				break;
			case e_intersectingPlane:
            case e_coPlanar:
				aboveList.push_back(poly);
				belowList.push_back(poly);
				break;
			default:
				HULK_ASSERT(false,"undefined polygon to plane orientation");
			}
		}

		iteration++;

		//create sub branches in the tree
		if (aboveList.size() > 0)
		{
			KDTreeNode tmp;
			unsigned int aboveIndex = m_treeNodes.size();
			m_treeNodes.push_back(tmp);
			m_treeNodes[nodeIndex].data.junction.m_abovePlane = aboveIndex;
			buildNodeMedian(iteration,aboveIndex,aboveList,aboveBox);
		}
		if (belowList.size() > 0)
		{
			KDTreeNode tmp;
			unsigned int belowIndex = m_treeNodes.size();
			m_treeNodes.push_back(tmp);
			m_treeNodes[nodeIndex].data.junction.m_belowPlane = belowIndex;
			buildNodeMedian(iteration,belowIndex,belowList,belowBox);
		}
	} //end of else . [if (0 == iteration)]
}

void KDTreeCPU::splitVolume(	unsigned int iteration ,
								  const Box & boundingBox,
								  OUT Box & subBoxPos,
								  OUT Box & subBoxNeg,
								  OUT AAPlane & partitionPlane)
{	
	SPVector centerPoint = getCenterPoint(boundingBox);
	memcpy (&subBoxPos,&boundingBox,sizeof(Box));
	memcpy (&subBoxNeg,&boundingBox,sizeof(Box));
	switch (iteration % 3)
	{
	case 0: //partition will be on the X axis
		partitionPlane.planeAxis = EPlaneAxis::e_planeYZ;
		partitionPlane.value = centerPoint[0];
		subBoxPos.m_min.x = centerPoint[0];
		subBoxNeg.m_max.x = centerPoint[0];
		break;
	case 1: //partition will be on the Y axis
		partitionPlane.planeAxis = EPlaneAxis::e_planeXZ;
		partitionPlane.value = centerPoint[1];
		subBoxPos.m_min.y = centerPoint[1];
		subBoxNeg.m_max.y = centerPoint[1];
		break;
	case 2: //patition will be on the Z axis
		partitionPlane.planeAxis = EPlaneAxis::e_planeXY;
		partitionPlane.value = centerPoint[2];
		subBoxPos.m_min.z = centerPoint[2];
		subBoxNeg.m_max.z = centerPoint[2];
		break;
	}
}

//! @Brief this function tests against node polygons. polygons are held be the leaf nodes
//! Junction nodes hold only polygons co-planar with the intersections plane.
//! return - true if ray hit the object , false if the ray does not
bool KDTreeCPU::checkCollision(const Ray & ray,OUT HitData & hitData) const 
{
	//initialize a pair. hit time and a pointer to the polygon
	HitData closestHit; 
	closestHit.time = std::numeric_limits<float>::infinity();

	float timeStart = 0;
	float timeEnd = std::numeric_limits<float>::infinity();
	bool retVal = false;
	retVal = checkRayToBoundingBoxGPU(m_scene->m_sceneBox,ray,timeStart,timeEnd);
	if (retVal) {
        //retVal = searchKDTree(ray,timeStart,timeEnd,closestHit);
		retVal = searchKDTreeStackless(ray,timeStart,timeEnd,closestHit);
		if (retVal)
		{
			//fill hitData struct
			hitData.time = closestHit.time;
			hitData.position = closestHit.position;
			hitData.normal = closestHit.normal;
		}
	}

	return retVal;
}

bool KDTreeCPU::testNodePolygons(const KDTreeNode & node , const Ray & ray ,OUT HitData & hit) const
{
	bool retVal = false;
	//check againt the intersecting polygons
	HitData hitPos;
	uint EndIndex = node.data.leaf.m_polygonCount + node.data.leaf.m_startIndex;
	for (unsigned int i = node.data.leaf.m_startIndex; i < EndIndex ;i++)
	{
#ifndef PRODUCTION
		Statistics::getInstance().countIntersectionTestMT();
#endif
		// reference is on page 182 in the book
		unsigned int polyIndex  = m_leafPolygons[i];
		const SPVector  p1 =  m_scene->m_vertices[m_scene->m_vertexIndices[polyIndex].indices[0]];
		const SPVector  p2 =  m_scene->m_vertices[m_scene->m_vertexIndices[polyIndex].indices[1]];
		const SPVector  p3 =  m_scene->m_vertices[m_scene->m_vertexIndices[polyIndex].indices[2]];
		if (checkRayToPolyACC(ray,&p1,&p2,&p3,m_polygonCache[polyIndex],hitPos))
		{
			if (hitPos.time < hit.time)
			{
				retVal = true;
				hit.time = hitPos.time;
				hit.position = hitPos.position;
				hit.normal = hitPos.normal;
			}
		}
	}
	return retVal;
}

bool KDTreeCPU::searchKDTree( const Ray & ray ,const float timeStart,const float timeEnd,OUT HitData & hit) const
{
	bool retVal = false;
    float tmin = timeStart;
	float tmax = timeEnd;
    uint todoCount = 0;
    SearchFrame todos[c_maxKDTreeDepth];
	
	SPVector invDir = onesInit / ray.m_rayDir.vec;


	const KDTreeNode * pNode = &(m_treeNodes[0]);
    do
    {
        if (hit.time < tmin)
			break;
      
	    if (pNode->m_flags & EPlaneAxis::e_leaf) //if this is a leaf node
	    {
		    retVal |= testNodePolygons(*pNode,ray,hit);
			pNode = &(m_treeNodes[0]);
	    }
	    else
	    {
		    #ifndef PRODUCTION
			    Statistics::getInstance().countKDTreeTraverseMT();
		    #endif
          	//check on which side of the plane we are
            uint planeAxis = (unsigned int) ( pNode->m_flags & 3);    
            float DirCoord = ray.m_rayDir.vector[planeAxis];
            float rayStart = ray.m_rayStart.vector[planeAxis];
		  
            float splitTime =  ( pNode->data.junction.m_planeValue - rayStart)*invDir.vector[planeAxis];
		    //if we are above the plane
		    uint firstPlane = pNode->data.junction.m_abovePlane;
		    uint secondPlane =  pNode->data.junction.m_belowPlane;

		    if (rayStart < pNode->data.junction.m_planeValue ||
                (rayStart  ==  pNode->data.junction.m_planeValue && DirCoord <= 0) )
		    {
			    firstPlane = pNode->data.junction.m_belowPlane;
			    secondPlane =  pNode->data.junction.m_abovePlane;
		    }
		    
            if ( splitTime > tmax || splitTime <= 0 ) //if we are completely on the close side
		    {
				pNode = &(m_treeNodes[firstPlane]);
		    }
			else if (splitTime < tmin) //if we are completely on the far side
			{
				pNode = &(m_treeNodes[secondPlane]);
			}
			else  // if we intersect the plane
		    {
				if (0 != secondPlane)
				{
					todos[todoCount].nodeIndex = secondPlane;
					todos[todoCount].timeStart = splitTime;
					todos[todoCount].timeEnd = tmax;
					todoCount++;
				} 
				pNode = &(m_treeNodes[firstPlane]);
				tmax = splitTime;
		    }
	    } // end of leaf/junction 'if' statement
		if (pNode ==  &(m_treeNodes[0]) && todoCount > 0 )
		{
			todoCount--;
			tmin = todos[todoCount].timeStart;
			tmax = todos[todoCount].timeEnd;
			pNode = &(m_treeNodes[todos[todoCount].nodeIndex]);
		}

    } while (pNode !=  &(m_treeNodes[0]));

	return retVal;
}

const int c_stackDepth = 5;

bool KDTreeCPU::searchKDTreeStackless( const Ray & ray ,const float timeStart,const float timeEnd,OUT HitData & hit) const
{
	SearchFrame frame[c_stackDepth] = {0};
	int stackSize= 0;
	int stackIndex = 0;
	const KDTreeNode * pNode;
    float tmin = timeStart;
	float tmax = timeStart;
	bool retVal = false;

	SPVector invDir = onesInit / ray.m_rayDir.vec;
    while (tmax < timeEnd)
    {
		if (  0 == stackSize)
		{
			pNode = &(m_treeNodes[0]);
			tmin = tmax;
			tmax = timeEnd;
		}
		else
		{
			--stackSize;
			--stackIndex;
			if (stackIndex == -1) {
				stackIndex += c_stackDepth;
			}
			tmin = frame[stackIndex].timeStart;
			tmax = frame[stackIndex].timeEnd;
			pNode = &(m_treeNodes[frame[stackIndex].nodeIndex]);
		}

	    while (pNode->m_flags & EPlaneAxis::e_junction) //if this is a leaf node
	    {
          	//check on which side of the plane we are
            uint planeAxis = (unsigned int) ( pNode->m_flags & 3);    
            float DirCoord = ray.m_rayDir.vector[planeAxis];
            float rayStart = ray.m_rayStart.vector[planeAxis];
		  
            float splitTime =  ( pNode->data.junction.m_planeValue - rayStart)*invDir.vector[planeAxis];
		    //if we are above the plane
		    uint firstPlane = pNode->data.junction.m_abovePlane;
		    uint secondPlane =  pNode->data.junction.m_belowPlane;

		    if (rayStart < pNode->data.junction.m_planeValue ||
                (rayStart  ==  pNode->data.junction.m_planeValue && DirCoord <= 0) )
		    {
			    firstPlane = pNode->data.junction.m_belowPlane;
			    secondPlane =  pNode->data.junction.m_abovePlane;
		    }
		    
            if ( splitTime >= tmax || splitTime <= 0 ) //if we are completely on the close side
		    {
				pNode = &(m_treeNodes[firstPlane]);
		    }
			else if (splitTime <= tmin) //if we are completely on the far side
			{
				pNode = &(m_treeNodes[secondPlane]);
			}
			else  // if we intersect the plane
		    {
				frame[stackIndex].timeStart = splitTime;
				frame[stackIndex].timeEnd = tmax;
				frame[stackIndex].nodeIndex = secondPlane;
				stackIndex = (stackIndex+1) % c_stackDepth;
				stackSize++;
				stackSize= stackSize > c_stackDepth ? c_stackDepth : stackSize;

				pNode = &(m_treeNodes[firstPlane]);
				tmax = splitTime;
		    }
			if (pNode ==  &(m_treeNodes[0]))
				break;
	    } // end of leaf/junction 'if' statement
		if  (pNode->m_flags & EPlaneAxis::e_leaf)
		{
			retVal |= testNodePolygons(*pNode,ray,hit);
			if (hit.time < tmax)
				break;
		}
    } 

	return retVal;
}

//checks the orietation of a polygon to an axis aligned plane
KDTreeCPU::EPolyToPlane KDTreeCPU::checkPolygonToAAPlane(const AAPlane & plane,const unsigned int polyIndex) const 
{
	float  c1,c2,c3;
	//retrieve the relevant vertex coordinates
	
	c1 = m_scene->m_vertices[m_scene->m_vertexIndices[polyIndex].indices[0]][(unsigned int)plane.planeAxis];
	c2 = m_scene->m_vertices[m_scene->m_vertexIndices[polyIndex].indices[1]][(unsigned int)plane.planeAxis];
	c3 = m_scene->m_vertices[m_scene->m_vertexIndices[polyIndex].indices[2]][(unsigned int)plane.planeAxis];
	
	bool exp1 = (c1 == plane.value);
	bool exp2 = (c2 == plane.value);
	bool exp3 = (c3 == plane.value);
	if (exp1 & exp2 & exp3)
		return e_coPlanar;

	exp1 = c1 >= plane.value;
	exp2 = c2 >= plane.value;
	exp3 = c3 >= plane.value;
	if (exp1 & exp2 & exp3)
		return e_abovePlane;
	exp1 = c1 <= plane.value;
	exp2 = c2 <= plane.value;
	exp3 = c3 <= plane.value;
	if (exp1 & exp2 & exp3)
		return e_belowPlane;
	
	return  e_intersectingPlane;
}

void KDTreeCPU::buildPolygonCache()
{
	Stack<PolyToRayCache>::iterator it = m_polygonCache.begin();
	Stack<TriangleIndices>::iterator trisIt = m_scene->m_vertexIndices.begin();
	for (uint i = 0 ; i < m_scene->m_vertexIndices.size();i++) 
	{
		const SPVector  p1 =  m_scene->m_vertices[trisIt->indices[0]];
		const SPVector  p2 =  m_scene->m_vertices[trisIt->indices[1]];
		const SPVector  p3 =  m_scene->m_vertices[trisIt->indices[2]];
		
		SPVector a1 = p2 - p1;
		SPVector a2 = p3 - p2;
		SPVector a3 = p1 - p3;

		SPVector normal = cross(a1,a2) + cross(a2,a3) + cross(a3,a1);
		normal.normalize();
		it->m_normal = normal;

		//cached calculations for ray to polygon intersection detections
		it->m_e0 = p2 - p1;
		it->m_e1 = p3 - p1;
		it->m_e00 = it->m_e0*it->m_e0;
		it->m_e01 = it->m_e0*it->m_e1;
		it->m_e11 = it->m_e1*it->m_e1;
		it->m_det = (it->m_e00 * it->m_e11) - pow (it->m_e01,2);
		++it;
		++trisIt;
	}
}

void KDTreeCPU::dumpTree()
{
    std::string dumpFileName = m_scene->m_sceneName + "KDTreeDump.txt";
    FILE * f = fopen(dumpFileName.c_str(),"w");
    for (uint i = 0 ; i < m_treeNodes.size();i++)
    {
        if (m_treeNodes[i].m_flags & EPlaneAxis::e_junction)
        {

            fprintf(f,"#%d, junc, axis %d value=%f above %d below %d \n",i,
            m_treeNodes[i].m_flags & 3,
            m_treeNodes[i].data.junction.m_planeValue,
            m_treeNodes[i].data.junction.m_abovePlane,
            m_treeNodes[i].data.junction.m_belowPlane);
        }
        if (m_treeNodes[i].m_flags & EPlaneAxis::e_leaf)
        {
            fprintf(f,"#%d, leaf, polys %d\n",i, m_treeNodes[i].data.leaf.m_polygonCount);
        }

    }

    fclose(f);

}

KDTreeCPU::SplitIteration & KDTreeCPU::SplitIteration::operator=(const KDTreeCPU::SplitIteration & other)
{
	nodeBox = other.nodeBox;
	boxSurface = other.boxSurface;
	iterationNumber = other.iterationNumber;
	nodeIndex = other.nodeIndex;
	polygonsEval = other.polygonsEval;
	polygonList = other.polygonList;
	candidates[0] = other.candidates[0];
	candidates[1] = other.candidates[1];
	candidates[2] = other.candidates[2];
	return *this;
}
