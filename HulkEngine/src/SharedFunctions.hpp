//////////////////////////////////////////////////////////////////
// SharedStruct.hpp												//
// Hulk renderer - Create by Tzachi Cohen 2012					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////


#ifndef HULK_SHARE_FUNCTIONS_HPP
#define HULK_SHARE_FUNCTIONS_HPP

//! this file holds code common to CPU and GPU

#include "sharedStructs.hpp"


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
