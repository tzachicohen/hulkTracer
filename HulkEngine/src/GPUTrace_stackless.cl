
#include "SharedFunctions.hpp"
#include "GPU_common.cl"


bool trace_ray(Ray  *ray,
                HitData * closestHit,
                __global const restrict float4 *    pVetices,
                __global const TriangleIndices*     pIndices,
                const Box        *                  sceneBox,
                __global const KDTreeNode *         pTreeNodes,
                __global LeafBases *                pLeafBases,
                uint                                index,
                __global ThreadContext*             pThreadContext)
{
    // traverse variable initialization
    bool hitDetected = false;
    float timeStart = 0;
    float timeEnd = FLT_MAX;
    float4 invDir4 = 1 / ray->m_rayDir;

    float tmax,tmin;

    bool hitBoundingBox = checkRayToBoundingBoxGPU(*sceneBox,*ray,timeStart,timeEnd);
    if (hitBoundingBox)
    {
        ThreadContext threadContext = pThreadContext[index];
        tmax = threadContext.tmax;
        tmin = threadContext.tmin;
        
        if (threadContext.nodeID != 0)
        {
            KDTreeNode node = pTreeNodes[threadContext.nodeID];
            LeafBases base = pLeafBases[node.data.leaf.m_leafID];
            if (base.m_residency == 2)
            {
                hitDetected |= testNodePolygons(pVetices,pIndices,base,node,*ray,*closestHit);
            }
            else
            {
                // request the leaf data again.
                pLeafBases[node.data.leaf.m_leafID].m_residency = 1;
                return false;
            }
        }
        //early return in case the the closest point has already been detected.
        if (closestHit->time < tmax)
        {
            pThreadContext[index].nodeID = 0;
             return hitDetected;
        }
        
        while ( tmax < timeEnd)
        {
            tmin = tmax;
            tmax = timeEnd;
            uint nodeIndex = 0;
            KDTreeNode currentNode = pTreeNodes[nodeIndex];
            while (currentNode.m_flags & EPlaneAxis::e_junction) //if this is a leaf node
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

                if ( splitTime >= tmax || splitTime <= 0 )
                {
                    nodeIndex = firstVoxel;
                }
                else if (splitTime <= tmin)
                {
                    nodeIndex = secondVoxel;
                }
                else
                {
                    nodeIndex = firstVoxel;
                    tmax= splitTime;
                }
                currentNode = pTreeNodes[nodeIndex];
                if (nodeIndex == 0)
                    break;
            } // end of 'while (currentNode.m_flags & EPlaneAxis::e_junction)'
            
            ThreadContext times;
            times.tmax = tmax;
            times.tmin = tmin;
            times.nodeID = 0;
            pThreadContext[index] = times;
                
            if (currentNode.m_flags & EPlaneAxis::e_leaf )
            {
                LeafBases bases = pLeafBases[currentNode.data.leaf.m_leafID];
                if (bases.m_residency == 2)
                {
                    hitDetected |= testNodePolygons(pVetices,pIndices,bases,currentNode,*ray,*closestHit);
                }
                else
                {
                    pLeafBases[currentNode.data.leaf.m_leafID].m_residency = 1;
                    pThreadContext[index].nodeID = nodeIndex;
                    break;
                }
            }
        } // end of 'while ( tmax < timeEnd)'
    }//if (hitBoundingBox)
    return hitDetected;
}

__kernel void gpu_trace( __write_only image2d_t                    frame_buffer,
                        __global const restrict float4 *           pVetices,
                        __global const TriangleIndices*            pIndices,
                        const uint                                 trisCount, //number of triangle polygons in the list
                        __CONSTANT CameraPos *                     pCameraPos,
                        __CONSTANT Light *                         pLights,
                        const uint                                 lightCount,
                        const Box                                  sceneBox,
                        __global const KDTreeNode *                pTreeNodes,
                        __global LeafBases *                 pNodeBases,
                       __global float4 *                           pHitDataArray,
                       __global ThreadContext*                     pThreadContext)
{
    Ray ray;
    uint column =get_global_id(0);
    uint row =get_global_id(1);
    uint imageWidth = get_image_width(frame_buffer);
    uint imageHeight = get_image_height(frame_buffer);
    int index  = column + (row*imageWidth);
    float4 hitResult; //xyz are the hit normal and 'w' component it the time.

    //kill edge work items residing out of the image
    if (column > imageWidth || row > imageHeight) {
        return ;
    }
    // calculate ray direction
    castRay(row,column,imageWidth,imageHeight,pCameraPos,&ray);
    
    HitData closestHit ;
    closestHit.time = pHitDataArray[index].w;
    
    if (trace_ray(&ray,
              &closestHit,
              pVetices,
              pIndices,
              &sceneBox,
              pTreeNodes,
              pNodeBases,
              index,
              pThreadContext))
    {
        hitResult.xyz = closestHit.normal.xyz;
        hitResult.w = closestHit.time;
        pHitDataArray[index] = hitResult;
    }

}
