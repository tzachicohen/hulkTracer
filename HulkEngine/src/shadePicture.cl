

#include "SharedFunctions.hpp"
#include "GPU_common.cl"


__kernel void ShadePicture( __write_only image2d_t    frame_buffer,
                            __CONSTANT CameraPos *    pCameraPos,
                            __CONSTANT Light *        pLights,
                            const uint                lightCount,
                            __global  float4*         hitDataArray)

{
    uint column =get_global_id(0);
    uint row =get_global_id(1);
    uint imageWidth = get_image_width(frame_buffer);
    uint imageHeight = get_image_height(frame_buffer);
    int2 coord  = {column,row};

    const float4 ambiant= {0.0f,0.0f,0.0f,0.0f};
    float4 color = {0.1f,0.1f,0.1f,0.0f};
    
    //xyz are the hit normal and 'w' component it the time.
    int index  = column + (row*imageWidth);
    float4 hitData = hitDataArray[index];

    if (hitData.w != FLT_MAX &&
        hitData.w > 0 ) 
    {
        Ray ray;
        castRay(row,column,imageWidth,imageHeight,pCameraPos,&ray);
        float3 position = ray.m_rayStart.xyz + hitData.w * ray.m_rayDir.xyz;
        color += ambiant;
        for (uint i = 0 ; i < lightCount ;i++)
        {
            float intensity = 0 ;
            //if this is a point light
            if (E_pointLight == pLights[i].color.w)
            {
                float3 toLight =  pLights[i].data.pos.xyz - position;

                float dist = length (toLight);
                toLight = normalize(toLight); //normalize the tolight vector
                //if the hit point is in shadow we reduce the lightening by half.
                intensity = dot ( hitData.xyz , toLight) /2;
            }
            if (E_directionalLight == pLights[i].color.w) 
			{
				intensity = dot (hitData.xyz , pLights[i].data.dir.xyz);
			}
            color +=  pLights[i].color * max(intensity,0.0f);
        }
    }
    write_imagef(frame_buffer,coord,color);
}
