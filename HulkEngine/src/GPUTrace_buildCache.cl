
#include "sharedStructs.hpp"

//this kernel is building a cache for 
__kernel void build_cache(  __global const restrict float4 *		pVetices,
			   __global const TriangleIndices*		pTriangles,
			   __global PolyToRayCache * pPolygonCache	)
{
	uint index =get_global_id(0); 
	__global const TriangleIndices*	pTriangle =	pTriangles+ index;
	__global PolyToRayCache*  pCache = pPolygonCache+index;
	const float4  p1 =  pVetices[pTriangle->indices[0]];
	const float4  p2 =  pVetices[pTriangle->indices[1]];
	const float4  p3 =  pVetices[pTriangle->indices[2]];
		
	float4 e0 = p2 - p1;
	float4 a2 = p3 - p2;
	float4 a3 = p1 - p3;

	float4 normal = cross(e0,a2) + cross(a2,a3) + cross(a3,e0);

	pCache->m_normal = normalize(normal);

	//cached calculations for ray to polygon intersection detections
	
	float4 e1 = p3 - p1;
	float e00 = dot(e0,e0);
	float e01 = dot(e0,e1);
	float e11 = dot(e1,e1);
	float det = (e00 * e11) - pow (e01,2);

	pCache->m_e0 = e0;
	pCache->m_e00 = e00;
	pCache->m_e01 = e01;
	pCache->m_e11 = e11;
	pCache->m_det = det;
	pCache->m_e1 = e1;
}