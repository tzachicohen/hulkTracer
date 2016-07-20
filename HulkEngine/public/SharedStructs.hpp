//////////////////////////////////////////////////////////////////
// SharedStruct.hpp												//
// Hulk renderer - Create by Tzachi Cohen 2012					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////


#ifndef HULK_SHARE_STRUCTS_HPP
#define HULK_SHARE_STRUCTS_HPP

#include "util.hpp"


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