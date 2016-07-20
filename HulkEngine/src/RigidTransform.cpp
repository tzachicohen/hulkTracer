//////////////////////////////////////////////////////////////////
// RigidTransform.hpp											//
// Hulk renderer - Create by Tzachi Cohen 2008					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////


#include "RigidTransform.hpp"
#include "Intersectable.hpp"


RigidTransform::RigidTransform()
:Node(e_static_transform)
{
	
}

void RigidTransform::setTransformation(float scale[3],float trans[3],float rot[3],float deg)
{
	 m_scale.x = scale[0];
	 m_scale.y = scale[1];
	 m_scale.z = scale[2];
	 m_rot.x = rot[0];
	 m_rot.y = rot[1];
	 m_rot.z = rot[2];
	 m_translate.x = trans[0];
	 m_translate.y = trans[1];
	 m_translate.z = trans[2];
	 m_deg =deg;
}
	