//////////////////////////////////////////////////////////////////
// Statistics.cpp													//
// Hulk renderer - Create by Tzachi Cohen 2013					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////

#include <stdio.h>

#include "Statistics.hpp"
#include "hulk_assert.h"

Statistics::Statistics()
:m_totalIntersections(0)
,m_totalTraverse(0)
,m_pScene(0)
, m_statsDumped(FALSE)
{
	m_KDBuildStart.QuadPart =0;
	m_KDBuildEnd.QuadPart = 0;
	m_frameStart.QuadPart = 0;
	m_frameEnd.QuadPart = 0;
    memset(&m_treeStats, 0, sizeof(m_treeStats));
}

Statistics::~Statistics()
{

}

void Statistics::initStats(Scene * pScene)
{
	dumpFileName = pScene->m_sceneName + "Stats.txt";
	m_pScene = pScene;

	m_numRays = (float) (m_pScene->m_resx * m_pScene->m_resy);
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);

	//to get the results in mili seconds
	freq.QuadPart /= 1000;
	m_freq = (float) freq.QuadPart;

    QueryPerformanceCounter(&m_runStart);

}


void Statistics::KDTreeBuildStart()
{
	QueryPerformanceCounter(&m_KDBuildStart);
}

void Statistics::KDTreeBuildEnd()
{
	QueryPerformanceCounter(&m_KDBuildEnd);
}
void Statistics::registerKDTreeStat(const KDTreeStats & stats)
{
    m_treeStats = stats;
}

void Statistics::frameStart()
{
	HULK_ASSERT(m_numRays != 0.0f , "'frameStart' called before 'initStats'\n")
	QueryPerformanceCounter(&m_frameStart);
	m_totalIntersections = 0;
	m_totalTraverse = 0 ;

}

void Statistics::frameEnd()
{
	FrameStats frameStats = {0};
	QueryPerformanceCounter(&m_frameEnd);
	
	frameStats.m_intersectionsPerRay = m_totalIntersections / m_numRays;
	frameStats.m_traversesPerRay = m_totalTraverse / m_numRays;
	//render time
	__int64 diff =  m_frameEnd.QuadPart - m_frameStart.QuadPart;
	frameStats.m_frameTime = ((float)diff) / m_freq;
	//KD Tree construction time
	diff =  m_KDBuildEnd.QuadPart - m_KDBuildStart.QuadPart;
	frameStats.m_buildtime = ((float)diff) / m_freq;
	
    frameStats.m_treeStats = m_treeStats;
    frameStats.m_backingStoreStats = m_backingStoreStats;
	
	m_statArray.push_back(frameStats);
}


void Statistics::dumpResults()
{
    if (m_statsDumped)
    {
        return;
    }
    m_statsDumped = TRUE;
    LARGE_INTEGER frameEnd;
    QueryPerformanceCounter(&frameEnd);

    LARGE_INTEGER totalRun;
    totalRun.QuadPart = frameEnd.QuadPart - m_runStart.QuadPart;
    float totalRuntime = (((float)totalRun.QuadPart) / m_freq)/1000;
	FILE * f = fopen(dumpFileName.c_str(),"w");
	float averageFrameTime = 0;
	float averageBuildTime = 0;
	for (uint i = 0 ; i < m_statArray.size();i++)
	{
		averageFrameTime += m_statArray[i].m_frameTime;
		averageBuildTime += m_statArray[i].m_buildtime;
	}
	averageFrameTime /= m_statArray.size();
	averageBuildTime /= m_statArray.size();
    fprintf(f, "Total run time: %f sec for %u frames\n", totalRuntime, m_statArray.size());
    fprintf(f, "%f sec per frame\n", totalRuntime / m_statArray.size());
	fprintf(f,"The scene holds %d triangles in %d objects res (%d*%d)\n",
		m_pScene->m_vertexIndices.size(),m_pScene->m_boundingBox.size(),m_pScene->m_resx,m_pScene->m_resy);
    uint indexBufferSize = m_pScene->m_vertexIndices.size()* sizeof(TriangleIndices);
    uint vertexBufferSize = m_pScene->m_vertices.size()*sizeof(SPVector);
    uint collisionExcelSize = m_pScene->m_vertexIndices.size()* sizeof(PolyToRayCache);
    fprintf(f, "Vertex buffer size %u bytes\n", indexBufferSize);
    fprintf(f, "Index buffer size %u bytes\n", vertexBufferSize);
    fprintf(f, "Polygon intersection acceleration cache size %u bytes\n",collisionExcelSize);
    fprintf(f, "Total scene data %u\n", indexBufferSize + vertexBufferSize + collisionExcelSize);
	fprintf(f,"Average trace time is %f\n",averageFrameTime);
	fprintf(f,"Average build time is %f\n",averageBuildTime);
	for (uint i = 0 ; i < m_statArray.size();i++)
	{
		fprintf(f,"Frame %d : KDTree build time %f, KDTree node count: %d ,KD tree leaf polys: %d\n",i,
			m_statArray[i].m_buildtime,
            m_statArray[i].m_treeStats.m_kdtreeNodes,
            m_statArray[i].m_treeStats.m_leafPoly);
        fprintf(f, "\t: Tree size %u bytes. Polygon index buffer Size is %u bytes\n", 
            m_statArray[i].m_treeStats.m_treeSize,
            m_statArray[i].m_treeStats.m_polygonIndexBufferSize);
        fprintf(f, "\t: backing store index buffer compression %f, vertex buffer compression %f bytes\n",
            m_statArray[i].m_backingStoreStats.m_indexBufferUtilization,
            m_statArray[i].m_backingStoreStats.m_vertexBufferUtilization);
	}
	fclose(f);
}
