//////////////////////////////////////////////////////////////////
// Material.hpp													//
// Hulk renderer - Create by Tzachi Cohen 2012					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////

#include "SPMatrix.hpp"

#ifndef HULK_MATERIAL_HPP
#define HULK_MATERIAL_HPP

struct Material : public AllignedS
{
	//////////////////////////////////////
	//public initialization and cleanup //
	//////////////////////////////////////
	inline Material();
	inline Material(const Material & mat);
	inline virtual ~Material();

	////////////////////
	// public members //
	////////////////////
	SPVector m_ambiant;
	SPVector m_diffuse;
	SPVector m_specular;
};


inline Material::Material()
:m_ambiant(0.1f,0.1f,0.1f)
,m_diffuse(0.85f,0.85f,0.85f)
,m_specular(0.2f,0.2f,0.2f)
{
}

inline Material::Material(const Material & mat)
{
	m_ambiant = mat.m_ambiant;
	m_diffuse = mat.m_diffuse;
	m_specular = mat.m_specular;
}

inline Material::~Material()
{
}


#endif