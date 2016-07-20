
#include "SharedFunctions.hpp"
#include "GPU_common.cl"

__kernel void gpu_trace( __write_only image2d_t					frame_buffer,
						__global const restrict float4 *		pVetices,
						__global const TriangleIndices*			pIndices,
						const uint								trisCount, //number of triangle polygons in the list
						__CONSTANT CameraPos *					pCameraPos,
						__CONSTANT Light *						pLights,
						const uint								lightCount)
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

	HitData closestHit;
	closestHit.time = HUGE_VALF;
	bool hitDetected = false;
	for (uint i = 0 ; i < trisCount ;i++)
	{
		HitData hitData;
		float4 p1 = ( pVetices[pIndices[i].indices[0]]);
		float4 p2 = ( pVetices[pIndices[i].indices[1]]);
		float4 p3 = ( pVetices[pIndices[i].indices[2]]);
		if (checkRayToPoly(ray,&p1,&p2,&p3,hitData)) {
			hitDetected = true;
			if (hitData.time < closestHit.time) {
				closestHit = hitData;
			}
		}
	}
	
	//calculate light source - todo: should be made in second pass
	const float4 ambiant= {0.1f,0.1f,0.1f,0.0f};
	float4 color = {0.0f,0.0f,0.0f,0.0f};
	if (hitDetected) {
		color += ambiant;
		for (uint i = 0 ; i < lightCount ;i++)
		{
			float intensity = 0 ;
			if (E_directionalLight == pLights[i].color.w) {
				intensity = dot (closestHit.normal , pLights[i].data.dir);
			}
			//if this is a point light
			if (E_pointLight == pLights[i].color.w) {
				SPVector toLight =  pLights[i].data.pos - closestHit.position;
				toLight = normalize(toLight);
				intensity =dot ( closestHit.normal , toLight);
			}
			color +=  pLights[i].color * max(intensity,0.0f);
		}
	}
	int2 coord  = {column,row};
	write_imagef(frame_buffer,coord,color);

}