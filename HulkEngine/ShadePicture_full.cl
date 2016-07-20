

//////////////////////////////////////////////////////////////////
// SharedStruct.hpp												//
// Hulk renderer - Create by Tzachi Cohen 2012					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////


#ifndef HULK_SHARE_FUNCTIONS_HPP
#define HULK_SHARE_FUNCTIONS_HPP

//! this file holds code common to CPU and GPU

//////////////////////////////////////////////////////////////////
// SharedStruct.hpp												//
// Hulk renderer - Create by Tzachi Cohen 2012					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////


#ifndef HULK_SHARE_STRUCTS_HPP
#define HULK_SHARE_STRUCTS_HPP



#ifndef HULD_UTIL_H
#define HULD_UTIL_H



#ifdef OPENCL_BUILD
// empty definition for the ALIGN16 macro
#	define ALIGN16
	struct AllignedS {
	};
	typedef float4 SPVector;
	typedef float4 __m128;
#	define inline
#else //if this is a CPU build
#	define SAFE_ARRAY_DELETE(pArray) if (pArray) { delete [] pArray; pArray = NULL;}
#	define SAFE_DELETE(pObject) if (pObject) { delete  pObject; pObject = NULL;}
#	define NULL 0

	typedef unsigned int uint;
    
  //  inline float fma(float a,float b,float c) { return a*b+c;}

	#ifdef _MSC_VER
    #	define ALIGN16 _CRT_ALIGN(16)
	#endif

	ALIGN16 struct AllignedS {
	void* operator new(size_t size)   {
		return _mm_malloc(size,16);
	};
	void  operator delete(void *addr) {
		return _mm_free(addr);
	};

	void* operator new[](size_t size)   {
		return _mm_malloc(size,16);
	};

	void  operator delete[](void *addr) {
		return _mm_free(addr);
	};
   
};


#endif

typedef unsigned int uint32;

#define OUT 
#define INOUT
#define IN

#define DEG_TO_RAD(a)			((a) * 0.017453292519943f)
#define RAD_TO_DEG(a)			((a) * 57.29577951308232f)



#ifndef  M_PI
#define  M_PI 3.1415926535897932384626433
#endif

#define MAX(a,b) ((a > b) ? a : b)
#define MIN(a,b) ((a < b) ? a : b)

inline size_t alignUp(size_t inputNumber,size_t  powerOfTwo) {
	size_t mask = powerOfTwo -1 ;
	return (inputNumber + mask) & ~mask;
}

inline bool isPowerOfTwo(uint inputNumber) {
	return 0 == (inputNumber & (inputNumber-1));
}

#endif 

//maximum depth of the tree
#define c_maxKDTreeDepth 20

//this depth is great for 
#define c_shortStackDepth 6

//structs that are shared between CPU and GPU
namespace EPlaneAxis {
	enum EPlaneAxis
	{
		e_planeYZ = 0,
		e_planeXZ = 1,
		e_planeXY = 2,

		//memory compaction - in the context of a KDtree node 
		//EPlaneAxis will also hold a flag specifing if this node
		//is a junction or a leaf
		e_leaf = 4,
		e_junction = 8 
	};
}

struct TriangleIndices
 {
    //should be 3 but is four for alignment purposes
	 unsigned int indices[4];
 };

//! @brief Axis aligned 2d plane in 3d space
struct AAPlane
{
	EPlaneAxis::EPlaneAxis planeAxis;
	float value;
};

#define E_directionalLight  0.0f
#define E_pointLight  1.0f

struct Light:public AllignedS
{
	// the 'ELightType' enumeration is kept in the 
	// .w component of color field
	SPVector color;
	union Data {
		__m128  dir; // for directional light
		__m128  pos; // for point light
	}data ;
	
};

struct Box :public AllignedS
{
	SPVector m_min;
	SPVector m_max;
};

struct Ray :public AllignedS
{
	SPVector m_rayStart;
	SPVector m_rayDir;
	//SPVector m_sign; // for every one of the 4 compenent 0xffffffffff if m_rayDir is positive and 0x0 if negtive;
};

// this struct holds cached values for 
//ray to polygon intersection test.
struct PolyToRayCache  : public AllignedS
{
	SPVector		 m_normal;
	// cached calculations for ray to polygon detection
	SPVector		m_e0;//the vector between p1 and p2
	SPVector		m_e1;//the vector between p1 and p3s
	float		m_e00;
	float		m_e01;
	float		m_e11;
	float		m_det;
};
//! @brief intermediate struct that holds data on the closest hit in the current node.
struct HitData : public AllignedS
{
	SPVector position;
	SPVector normal;
	float time;
};

struct CameraPos : public AllignedS 
{
	SPVector m_from;
	SPVector m_verticalFactor; // a vector which stretches from lower to upper edges of the screen
	SPVector m_horizontalFactor; // a vector which stretches from left to right edges of the screen
	SPVector m_lowerLeft; // a vector pointing at the lower left side of the sceen
};

/////////////////////////////
// KD Tree related structs //
/////////////////////////////

typedef unsigned int NodeFlags;

struct KDTreeNode
{
	KDTreeNode():m_flags(0)
	{
		data.junction.m_abovePlane = 0;
		data.junction.m_belowPlane = 0;
		data.junction.m_planeValue = 0;
	}
	NodeFlags m_flags;
	//geometry held by the node. if this node is a junction, it holds co-planar polygons

    union {
	    struct Leaf {
		    unsigned int m_startIndex; // an start index of the leaf polygon list in the global tree list
		    unsigned int m_polygonCount; // the number of polygons in the leaf
            unsigned int m_leafID;
	    } leaf ;

	    struct Junction{
		    unsigned int m_abovePlane; //index of the above plane 
		    unsigned int m_belowPlane; //index of the below plane
		    float m_planeValue;
	    } junction ;
    } data;
};

//This struct is for the non-recursive implementation of the KD tree search
struct SearchFrame
{
	float timeStart;
	float timeEnd;
	uint nodeIndex;
};

struct LeafBases
{
    uint m_indexBase; //offset into the GPU index memory pool in 'TriangleIndices' structs
    uint m_vertexBase; //offset into the GPU vertex memory pool in 'SPvector structs
    uint m_residency;
};


struct ThreadContext
{
    float tmin;
    float tmax;
    uint nodeID; //node ID to test before resuming travesal. Zero means no leaf since the first node is always a junction
};

#endif //HULK_SHARE_STRUCTS_HPP

//calculates box center point.
inline SPVector getCenterPoint(const Box & box)
{
	return (box.m_max + box.m_min) * 0.5f;
}

//calculates box volume.
inline float getBoxVolume(const Box & box)
{
	SPVector sizes = box.m_max - box.m_min;
	return sizes.x*sizes.y*sizes.z;
}

//todo: implement this function with a single SSE multiplication
//calculates box surface.
inline float getBoxSurface(const Box & box)
{
	SPVector sizes = box.m_max - box.m_min;
	return 2*(sizes.x*sizes.y + sizes.z*sizes.y + sizes.x*sizes.z);
}

//todo: implement this function with a single SSE multiplication
//calculates box surface.
inline float getHalfBoxSurface(const Box & box)
{
	SPVector sizes = box.m_max - box.m_min;
	return (sizes.x*sizes.y + sizes.z*sizes.y + sizes.x*sizes.z);
}


inline bool checkRayToPoly(const Ray & ray ,
							const SPVector * p1,
							const SPVector * p2,
							const SPVector * p3,
							OUT HitData & hitData)  
{
	SPVector e0 = *p2 - *p1;
	SPVector e1 = *p3 - *p1;
	SPVector e2 = *p3 - *p2;

	SPVector normal =cross(e0 , e1)+ cross(e1 , e2) + cross(e2 ,e0);
	//todo: normalize only if the ray hit the polygon.
	normal = normalize(normal);

	//check if the ray is parrallel to the polygon
	float denum = dot(normal , ray.m_rayDir);

	if ( fabs (denum) < FLT_EPSILON)
	{
		return false; //the ray is parrallel the polygon
	}
	denum = 1/denum;
	float time = dot(normal ,(*p1 - ray.m_rayStart)) * denum;

	if (time < 0 ) //the polygon plane is behind the ray
	{
		return false;
	}

	SPVector hitPoint = ray.m_rayStart + (ray.m_rayDir * time);
	
	SPVector hitPointToP1 = hitPoint - *p1;

	float q0 = dot(e0,hitPointToP1);
	float q1 = dot(e1,hitPointToP1);
	
	float e00 = dot(e0,e0);
	float e01 = dot(e0,e1);
	float e11 = dot(e1,e1);
	float det = fma(e00 ,e11, -pow(e01,2));

	float s1 = fma(e11,q0,-(e01*q1));
	float s2 = fma(e00,q1,-(e01*q0));

	if (s1 >= 0 && s2 >= 0 && (s1+s2) <= det)
	{
		hitData.position = hitPoint;
		hitData.time = time;
		hitData.normal = normal;
		return true;
	}
	else
	{
		return false;
	}
}

//this function performs ray to 
inline bool checkRayToPolyACC(const Ray & ray ,
							const SPVector * p1,
							const SPVector * p2,
							const SPVector * p3,
							const PolyToRayCache & cache,
							OUT HitData & hitData) 
{
	
	//check if the ray is parrallel to the polygon
	float denum = dot(cache.m_normal,ray.m_rayDir);

	if ( fabs (denum) < FLT_EPSILON)
	{
		return false; //the ray is parrallel the polygon
	}
	denum = 1/denum;
	float time = dot(cache.m_normal , (*p1 - ray.m_rayStart)) * denum;

	if (time < 0 ) //the polygon plane is behind the ray
	{
		return false;
	}

	SPVector hitPoint = ray.m_rayStart + (ray.m_rayDir * time);
	
	SPVector hitPointToP1 = hitPoint - *p1;

	float q0 = dot(cache.m_e0,hitPointToP1);
	float q1 = dot(cache.m_e1,hitPointToP1);
	
	float s1 = (cache.m_e11*q0) - (cache.m_e01*q1);
	float s2 = (cache.m_e00*q1) - (cache.m_e01*q0);

	if (s1 >= 0 && s2 >= 0 && (s1+s2) <= cache.m_det)
	{
		hitData.position = hitPoint;
		hitData.time = time;
		hitData.normal = cache.m_normal;
		return true;
	}
	else
	{
		return false;
	}
}

//! checks if the ray intersect the space which is above an axis aligned plane
//this function checks a single dimension vector againt a value
//the equation is p0+ t*d0 >= e0 ; 
inline  bool  isRayToAbovePlane(float p0,float d0, float e0,float & t0,float & t1)
{
	if (d0 > 0 )
	{
		float ti = (e0-p0)/d0;
		if (ti > t1)
		{
			return false;
		}
		if (ti > t0)
		{
			t0 = ti;
		}
		return true;
	}
	else if (d0 < 0)
	{
		float ti = (e0-p0)/d0;
		if (ti < t0)
		{
			return false;
		}
		if (ti < t1)
		{
			t1 = ti;
		}
		return true;
	} 
	else //d0 == 0
	{
		return e0 < p0  ;
	}
}
//! checks if the ray intersect the space which is below an axis aligned plane
inline  bool  isRayToBelowPlane(float p0,float d0, float e0,float & t0,float & t1)
{
	if (d0 < 0 )
	{
		float ti = (e0-p0)/d0;
		if (ti > t1)
		{
			return false;
		}
		if (ti > t0)
		{
			t0 = ti;
		}
		return true;
	}
	else if (d0 > 0)
	{
		float ti = (e0-p0)/d0;
		if (ti < t0)
		{
			return false;
		}
		if (ti < t1)
		{
			t1 = ti;
		}
		return true;
	} 
	else //d0 == 0
	{
		return e0 > p0  ;
	}
}

//ray to box intersection test
inline bool checkRayToBoundingBox(const Box & box ,const Ray & ray,OUT float & timeStartResult,OUT float & timeEndResult )
{
	bool retVal;
	float timeStart = 0;
	float timeEnd = FLT_MAX;

	retVal =isRayToAbovePlane(ray.m_rayStart.x,ray.m_rayDir.x,box.m_min.x,timeStart,timeEnd) &&
			isRayToBelowPlane(ray.m_rayStart.x,ray.m_rayDir.x,box.m_max.x,timeStart,timeEnd) &&
			isRayToAbovePlane(ray.m_rayStart.y,ray.m_rayDir.y,box.m_min.y,timeStart,timeEnd) &&
			isRayToBelowPlane(ray.m_rayStart.y,ray.m_rayDir.y,box.m_max.y,timeStart,timeEnd) &&
			isRayToAbovePlane(ray.m_rayStart.z,ray.m_rayDir.z,box.m_min.z,timeStart,timeEnd) &&
			isRayToBelowPlane(ray.m_rayStart.z,ray.m_rayDir.z,box.m_max.z,timeStart,timeEnd);
	if (true == retVal)
	{
		timeEndResult = timeEnd;
		timeStartResult = timeStart;
	}
	
	return retVal;	
}

//ray to box intersection test
inline bool checkRayToBoundingBoxGPU(const Box & box ,const Ray & ray,OUT float & timeStartResult,OUT float & timeEndResult )
{
	bool retVal;
	float timeStart = 0;
	float timeEnd = FLT_MAX;

	retVal =isRayToAbovePlane(ray.m_rayStart.x,ray.m_rayDir.x,box.m_min.x,timeStart,timeEnd) &
			isRayToBelowPlane(ray.m_rayStart.x,ray.m_rayDir.x,box.m_max.x,timeStart,timeEnd) &
			isRayToAbovePlane(ray.m_rayStart.y,ray.m_rayDir.y,box.m_min.y,timeStart,timeEnd) &
			isRayToBelowPlane(ray.m_rayStart.y,ray.m_rayDir.y,box.m_max.y,timeStart,timeEnd) &
			isRayToAbovePlane(ray.m_rayStart.z,ray.m_rayDir.z,box.m_min.z,timeStart,timeEnd) &
			isRayToBelowPlane(ray.m_rayStart.z,ray.m_rayDir.z,box.m_max.z,timeStart,timeEnd);
	if (true == retVal)
	{
		timeEndResult = timeEnd;
		timeStartResult = timeStart;
	}
	
	return retVal;	
}


#endif //HULK_SHARE_FUNCTIONS_HPP

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
