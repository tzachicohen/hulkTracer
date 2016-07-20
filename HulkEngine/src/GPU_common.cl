
#ifdef DEBUG
//CodeXL cannot debug constant buffers, hence in debug mode all buffers are global
#define __CONSTANT __global

#else
#define __CONSTANT __constant
#endif 

bool castRay(unsigned int row , 
			 unsigned int column,
			 unsigned int screenWidth,
			 unsigned int screenHeight,
			 IN __CONSTANT CameraPos * camPos,
			 OUT Ray * ray)
{
	//by default the camera looks at the Negative Z.
	ray->m_rayStart = camPos->m_from;

	//
	float relativeRow = float (row)/ float(screenHeight);
	float relativecolumn = float(column) / float( screenWidth );

	ray->m_rayDir = (camPos->m_lowerLeft + (camPos->m_verticalFactor  *relativecolumn  ) +
		(camPos->m_horizontalFactor  *relativeRow  )) - camPos->m_from;
	
	normalize(ray->m_rayDir);

	return true;
}

float getCoord(float4 coord, EPlaneAxis::EPlaneAxis axis)
{
	switch (axis)
	{
	case EPlaneAxis::e_planeYZ:
		return coord.s0;
	case EPlaneAxis::e_planeXZ:
		return coord.s1;
	case EPlaneAxis::e_planeXY:
		return coord.s2;	
	}
	return 0;
}

bool testNodePolygons(	__global const restrict float4 *pVetices,
						__global const TriangleIndices*	pIndices,
						__global const LeafBases   &	bases,
						const KDTreeNode &              node, 
						const Ray &                     ray,
						OUT HitData &                   hit)
{
	bool retVal =false;
	HitData hitPos;

	uint EndIndex = node.data.leaf.m_polygonCount +  bases.m_indexBase;
	for (unsigned int i = bases.m_indexBase; i < EndIndex ;i++)
	{
		float4 p1 = ( pVetices[pIndices[i].indices[0] + bases.m_vertexBase]);
		float4 p2 = ( pVetices[pIndices[i].indices[1] + bases.m_vertexBase]);
		float4 p3 = ( pVetices[pIndices[i].indices[2] + bases.m_vertexBase]);
		if (checkRayToPoly(ray,&p1,&p2,&p3,hitPos)) {
			if (hitPos.time < hit.time) {
                retVal = true;
				hit.position = hitPos.position;
				hit.normal = hitPos.normal;
				hit.time = hitPos.time;
			}
		}
	}
	return retVal;
}