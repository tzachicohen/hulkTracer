// Intersectable.hpp											//
// Hulk renderer - Create by Tzachi Cohen 2008					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////

#ifndef HULK_INTERSECTABLE_HPP
#define HULK_INTERSECTABLE_HPP



#include "util.hpp"
#include "SPMatrix.hpp"
#include "Node.hpp"
#include "stack.hpp"
#include "Material.hpp"
#include "SharedStructs.hpp"
class Scene;

//todo: add drived drived class for mesh
class Intersectable : public Node
{
public:
	////////////////////
	// public structs //
	////////////////////
	struct MeshData {
	  union {
		struct {
		  uint32 e_vertexData  :1;
		  uint32 e_normalData  :1;
		  uint32 e_textureData  :1;
		  uint32 reserved :29;
		};
		uint32   Value;
	  };
	  MeshData() { Value = 0;}
	} ;

	//////////////////////////////////////
	//public initialization and cleanup //
	//////////////////////////////////////
	Intersectable(ENodeType type);
	virtual ~Intersectable() ;
	////////////////////
	// public methods //
	////////////////////
	bool setVertices(const SPVector * vertices,unsigned int numVertices,unsigned int offset);
	bool setNorms(const SPVector * pNorms,unsigned int numNorms,uint offset);
	//! @param the pVertices vertices coordinates in 3*numVertices format
	bool setVertices(float * pVertices,unsigned int numVertices,unsigned int offset);
	bool setNorms(float * pNorms,unsigned int numNorms,uint offset);
	void setVertexIndices(const unsigned short * pIndices,unsigned int NumTriangle);
	
	void setIndexInfo(uint  indicesOffset,unsigned int num)
		{m_vertexIndicesOffset = indicesOffset; m_trigCount = num;}
	
	void setNormalIndexInfo(uint normalIndexoffset)
		{ m_normalIndicesOffset = normalIndexoffset;}

	void update(const SPMatrix & mat,Scene & scene);
	
	//! @brief returns offset of the transformed vertices in the main scene vertex list
	uint getvertexOffset() {return m_vertexOffset;}
	//! @brief returns the offset of the vertex indices in the main index list
	uint getVertexIndicesOffset() {return m_vertexIndicesOffset;}
	//! @brief returns offset of the transformed normals in the main scene vertex list
	uint getNormalOffset() {return m_normalOffset;}
	//! @brief returns the offset of the vertex indices in the main index list
	uint getNomalIndicesOffset() {return m_normalIndicesOffset;}
	//! @brief returns the number of polygons in the object
	uint getTrigCount() {return m_trigCount;}

	/////////////////////
	// private members //
	/////////////////////
//TODO: return reinstate the protected attribute.
//protected:
	//! the number of vertices in current node
	uint m_vertexCount; //should be  = m_trigCount*3
	//! the number of normals in current node
	uint m_normalCount; 
	//! the offset of the transformed vertices in the main scene vertex list
	uint m_vertexOffset;
	//! the offset of the transformed normals in the main scene normal list
	uint m_normalOffset;
	//! untransformed normals
	SPVector * m_pNorms;
	//! untransformed vertices - change to stack as well
	SPVector * m_pVerts;
	//! the offset of the vertices indices in the global vertices index list
	uint  m_vertexIndicesOffset;
	//! the offset of vertices in the primary normal index list;
	uint  m_normalIndicesOffset;
	//! the number of polygons in the object
	uint m_trigCount;
	//! holds which attribute exists for the object. (some meshes don't have normal data).
	MeshData m_meshData;
	//! the index of the mesh bounding box in the scene class list
	uint m_boxIndex;
	// for debug purposes we store indices in the mesh node as well
	Stack<TriangleIndices> m_vertexIndices;

private:
	///////////////////////////////////////////////
	//private initialization to prevent override //
	///////////////////////////////////////////////
	Intersectable(Intersectable & n):Node(n.getNodeType()) {}
	Intersectable& operator=(Intersectable & n) {return *this;}
};



#endif //end of HULK_INTERSECTABLE_HPP