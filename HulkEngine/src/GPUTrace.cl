
#include "SharedFunctions.hpp"
#include "GPU_common.cl"


bool trace_ray(Ray  *ray,
				HitData * closestHit,
				__global const restrict float4 *		pVetices,
				__global const TriangleIndices*		pIndices,
				const Box		*					sceneBox,
				__global const KDTreeNode *			pTreeNodes,
				__global const uint *				pNodePolys,
				__global PolyToRayCache *			pPolygonCache)
{
	// traverse variable initialization
	uint stackSize = 0;
	bool hitDetected = false;
	float timeStart = 0;
	float timeEnd = FLT_MAX;
	__private SearchFrame traverseStack[c_maxKDTreeDepth];
	float4 invDir4 = 1 / ray->m_rayDir;

	bool hitBoundingBox = checkRayToBoundingBoxGPU(*sceneBox,*ray,timeStart,timeEnd);
	if (hitBoundingBox) 
	{
		uint nodeIndex = 0;
		do
		{
			if (closestHit->time < timeStart)
			{
				break;
			}
			KDTreeNode currentNode = pTreeNodes[nodeIndex];
			if (currentNode.m_flags & EPlaneAxis::e_leaf) //if this is a leaf node
			{
				hitDetected |= testNodePolygons(pVetices,pIndices,pNodePolys,pPolygonCache,currentNode,*ray,*closestHit);
				nodeIndex = 0;
			}
			else
			{
				EPlaneAxis::EPlaneAxis planeAxis = (EPlaneAxis::EPlaneAxis) (currentNode.m_flags & 3);
				//check in which side of the plane we are
                float rayStart = getCoord(ray->m_rayStart,planeAxis);
                float rayDir =  getCoord(ray->m_rayDir,planeAxis);
				float invDir =  getCoord(invDir4,planeAxis);

                float splitTime =  (currentNode.data.junction.m_planeValue - rayStart)*(invDir);
				//if we are above the plane
				uint firstVoxel = currentNode.data.junction.m_abovePlane;
				uint secondVoxel = currentNode.data.junction.m_belowPlane;

				if (rayStart < currentNode.data.junction.m_planeValue ||
                   (rayStart  ==  currentNode.data.junction.m_planeValue && rayDir <= 0) ) 
				{
					firstVoxel = currentNode.data.junction.m_belowPlane;
					secondVoxel = currentNode.data.junction.m_abovePlane;
				}// if (currentNode.m_flags & EPlaneAxis::e_leaf) - else

                if ( splitTime > timeEnd || splitTime <= 0 )
				{
					nodeIndex = firstVoxel;
				}	
				else if (splitTime < timeStart)
				{
					nodeIndex = secondVoxel;
				}
				else
				{
					if (0 != secondVoxel)
					{
						traverseStack[stackSize].nodeIndex = secondVoxel;
						traverseStack[stackSize].timeStart = splitTime;
						traverseStack[stackSize].timeEnd = timeEnd;
						stackSize++;
					}
					nodeIndex = firstVoxel;
					timeEnd = splitTime;
				}
			} // if (currentNode.m_flags & EPlaneAxis::e_leaf) //if this is a leaf node
			if (0 == nodeIndex  && stackSize > 0)
			{
				stackSize--;
				timeStart = traverseStack[stackSize].timeStart ;
				timeEnd = traverseStack[stackSize].timeEnd ;
				nodeIndex = traverseStack[stackSize].nodeIndex;
			}
		} while (nodeIndex != 0);
	}//if (hitBoundingBox) 
	return hitDetected;
}

__kernel void gpu_trace( __write_only image2d_t					frame_buffer,
						__global const restrict float4 *		pVetices,
						__global const TriangleIndices*			pIndices,
						const uint								trisCount, //number of triangle polygons in the list
						__CONSTANT CameraPos *					pCameraPos,
						__CONSTANT Light *						pLights,
						const uint								lightCount,
						const Box								sceneBox,
						__global const KDTreeNode *				pTreeNodes,
						__global const uint *					pNodePolys,
						__global PolyToRayCache *				pPolygonCache)
{
	Ray ray;
	uint column =get_global_id(0); 
	uint row =get_global_id(1);
	uint imageWidth = get_image_width(frame_buffer);
	uint imageHeight = get_image_height(frame_buffer);

	
	//kill edge work items residing out of the image
	if (column > imageWidth || row > imageHeight) {
		return ;
	}
	// calculate ray direction
	castRay(row,column,imageWidth,imageHeight,pCameraPos,&ray);
	HitData closestHit ;
	closestHit.time = HUGE_VALF;

	bool hitDetected = trace_ray(&ray,&closestHit,pVetices,pIndices,&sceneBox,pTreeNodes,pNodePolys,pPolygonCache);

	//calculate light source - todo: should be made in second pass
	const float4 ambiant= {0.1f,0.1f,0.1f,0.0f};
	float4 color = {0.0f,0.0f,0.0f,0.0f};
	if (hitDetected) {
		color += ambiant;
		for (uint i = 0 ; i < lightCount ;i++)
		{
			float intensity = 0 ;
			if (E_directionalLight == pLights[i].color.w) 
			{
				intensity = dot (closestHit.normal , pLights[i].data.dir);
			}
			//if this is a point light
			if (E_pointLight == pLights[i].color.w) 
			{
				SPVector toLight =  pLights[i].data.pos - closestHit.position;
				toLight.w = 0;
				float dist = length (toLight);
				toLight = normalize(toLight); //normalize the tolight vector
				//generate the shadow ray
				Ray shadowRay;
				HitData shadowHit;
				shadowHit.time = HUGE_VALF;
				//add i minor offset to the ray origin so it will not hit the same polygon bounced from due to rounding error.
				shadowRay.m_rayStart = closestHit.position + 0.001*toLight; 
				shadowRay.m_rayDir = toLight;
				bool hit = trace_ray(&shadowRay,&shadowHit,pVetices,pIndices,&sceneBox,pTreeNodes,pNodePolys,pPolygonCache);
				//if the hit point is in shadow we reduce the lightening by half.
				intensity = dot ( closestHit.normal , toLight);
				if ( dist > shadowHit.time )
				{
					intensity *= 0.5;
				}
			}
			color +=  pLights[i].color * max(intensity,0.0f);
		}
	}
	int2 coord  = {column,row};
	write_imagef(frame_buffer,coord,color);
}
