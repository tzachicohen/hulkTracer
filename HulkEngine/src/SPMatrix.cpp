//////////////////////////////////////////////////////////////////////////////
//
// SPMATRIX.INL
//
//  Implementation of the SPMatrix, SPVector and the SPVector3 classes.
//
//  Version 1.0
//  Written by Zvi Devir, Intel MSL
//
//////////////////////////////////////////////////////////////////////////////

//
//   Copyright (c) 2001 Intel Corporation.
//
// Permition is granted to use, copy, distribute and prepare derivative works 
// of this library for any purpose and without fee, provided, that the above 
// copyright notice and this statement appear in all copies.  
// Intel makes no representations about the suitability of this library for 
// any purpose, and specifically disclaims all warranties. 
// See LEGAL.TXT for all the legal information.
//



#include "SPMatrix.hpp"
#include "math.h"
const ALIGN16 int     __0FFF_[4] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 };
const ALIGN16 float __ZERONE_[4] = { 1.0f, 0.0f, 0.0f, 1.0f };

// ----------------------------------------------------------------------------
//  Name:   TranslateMatrix
//  Desc:   Returns the translation matrix for specific translation.
// ----------------------------------------------------------------------------
void SPMatrix::TranslateMatrix(const float dx, const float dy, const float dz)
{
    IdentityMatrix();
    _41 = dx;
    _42 = dy;
    _43 = dz;
} 

// ----------------------------------------------------------------------------
//  Name:   ScaleMatrix
//  Desc:   Returns the scaling matrix for x,y,z axis.
// ----------------------------------------------------------------------------
void SPMatrix::ScaleMatrix(const float a, const float b, const float c)
{
    IdentityMatrix();
    _11 = a;
    _22 = b;
    _33 = c;
} 

void SPMatrix::rotateMatrix(float x,float y,float z,float angle) 
{

		float cosAngle=cosf(angle);
		float sinAngle=sinf(angle);
		float oneCosAngle=1-cosAngle;
		float xy=oneCosAngle*x*y;
		float xz=oneCosAngle*x*z;
		float yz=oneCosAngle*y*z;

	
		_11=cosAngle + oneCosAngle*x*x;
		_12=xy - sinAngle*z;
		_13=xz + sinAngle*y;
		_14=0.0f;

		_21=xy + sinAngle*z;
		_22=cosAngle + oneCosAngle*y*y;
		_23=yz - sinAngle*x;
		_24=0.0f;

		_31=xz - sinAngle*y;
		_32=yz + sinAngle*x;
		_33=cosAngle + oneCosAngle*z*z;
		_34=0.0f;

		_41=0.0f;
		_42=0.0f;
		_43=0.0f;
		_44=1.0f;
}

void  SPMatrix::transpose()
{
	//todo: fill an implementation
}