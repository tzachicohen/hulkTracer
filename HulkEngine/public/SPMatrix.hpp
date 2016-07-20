//////////////////////////////////////////////////////////////////////////////
//
// SPMATRIX.H
//
//  Declaration of the SPMatrix, SPVector and the SPVector3 classes.
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

#pragma once

#include <xmmintrin.h> 
#include <emmintrin.h>

#include "util.hpp"

class SPMatrix;
class SPVector;


__declspec(align(16))
class SPMatrix :public AllignedS 
{
public:
	union 
	{
        struct 
		{
            __m128 _L1, _L2, _L3, _L4;
        };
        struct 
		{
            float   _11, _12, _13, _14;
            float   _21, _22, _23, _24;
            float   _31, _32, _33, _34;
            float   _41, _42, _43, _44;
        };
    };


	void IdentityMatrix();

	void TranslateMatrix(const float dx, const float dy, const float dz);
    void ScaleMatrix(const float a, const float b, const float c);
	void rotateMatrix(float x,float y,float z,float angle);
	void transpose();
	SPMatrix& operator *= (const SPMatrix&);
};


__declspec(align(16))
class SPVector  :public AllignedS 
{
public:
	union {
        __m128 vec;
        struct { 
            float   x,y,z,w;
        };
		float vector[4];
    };

	SPVector() {};
	SPVector(const __m128 &m) : vec(m) {};
	SPVector(const float x, const float y, const float z, const float w=1.0f) { vec = _mm_set_ps(w,z,y,x);};

	friend SPVector operator * (const SPVector&, const SPMatrix&);
	friend float	operator * (const SPVector&, const SPVector&);             // Dot Product
	friend SPVector	operator * (const SPVector&, const float);
	friend SPVector operator - (const SPVector&, const SPVector&);
	friend SPVector operator % (const SPVector&, const SPVector&);          // Cross Product
	friend SPVector operator | (const SPVector&, const SPVector&);          // Elements Product

	SPVector& operator *= (const SPMatrix&);
	SPVector& operator *= (const float);
	SPVector& operator += (const SPVector&);
    SPVector& operator -= (const SPVector&);

	SPVector& normalize();

	float& operator[] (unsigned int i )  { return vector[i];}
};

inline SPVector cross(const SPVector& a, const SPVector& b)
{
	return a % b;
}

inline float dot(const SPVector& a, const SPVector& b)
{
	return a * b;
}

inline SPVector &  normalize(SPVector& a)
{
	return a.normalize();
}

extern const ALIGN16 int   __0FFF_[4];
extern const ALIGN16 float __ZERONE_[4];
#define _0FFF_      (*(__m128*)&__0FFF_)           // 0 * * *
#define _ZERONE_    (*(__m128*)&__ZERONE_)         // 1 0 0 1

// ----------------------------------------------------------------------------
//  Name:   IdentityMatrix
//  Desc:   Returns the Identity Matrix.
// ----------------------------------------------------------------------------
inline void SPMatrix::IdentityMatrix() {
    __m128 zerone = _ZERONE_;
    _mm_storel_pi((__m64 *)&_11, zerone);
    _mm_storel_pi((__m64 *)&_13, _mm_setzero_ps());
    _mm_storeh_pi((__m64 *)&_21, zerone);
    _mm_storel_pi((__m64 *)&_23, _mm_setzero_ps());
    _mm_storel_pi((__m64 *)&_31, _mm_setzero_ps());
    _mm_storel_pi((__m64 *)&_33, zerone);
    _mm_storel_pi((__m64 *)&_41, _mm_setzero_ps());
    _mm_storeh_pi((__m64 *)&_43, zerone);
}


// ----------------------------------------------------------------------------
//  Name:   SPMatrix *= SPMatrix
//  Desc:   Matrix multiplication of A by B. [A] = [A]*[B].
// ----------------------------------------------------------------------------
inline SPMatrix& SPMatrix::operator *= (const SPMatrix& B) {
    __m128 r1, r2;
    __m128 B1 = B._L1, B2 = B._L2, B3 = B._L3, B4 = B._L4;

    r1 = _mm_mul_ps(_mm_shuffle_ps(_L1,_L1,0x00),B1);
    r2 = _mm_mul_ps(_mm_shuffle_ps(_L2,_L2,0x00),B1);
    r1 = _mm_add_ps(r1,_mm_mul_ps(_mm_shuffle_ps(_L1,_L1,0x55),B2));
    r2 = _mm_add_ps(r2,_mm_mul_ps(_mm_shuffle_ps(_L2,_L2,0x55),B2));
    r1 = _mm_add_ps(r1,_mm_mul_ps(_mm_shuffle_ps(_L1,_L1,0xAA),B3));
    r2 = _mm_add_ps(r2,_mm_mul_ps(_mm_shuffle_ps(_L2,_L2,0xAA),B3));
    r1 = _mm_add_ps(r1,_mm_mul_ps(_mm_shuffle_ps(_L1,_L1,0xFF),B4));
    r2 = _mm_add_ps(r2,_mm_mul_ps(_mm_shuffle_ps(_L2,_L2,0xFF),B4));
    _L1 = r1;
    _L2 = r2;

    r1 = _mm_mul_ps(_mm_shuffle_ps(_L3,_L3,0x00),B1);
    r2 = _mm_mul_ps(_mm_shuffle_ps(_L4,_L4,0x00),B1);
    r1 = _mm_add_ps(r1,_mm_mul_ps(_mm_shuffle_ps(_L3,_L3,0x55),B2));
    r2 = _mm_add_ps(r2,_mm_mul_ps(_mm_shuffle_ps(_L4,_L4,0x55),B2));
    r1 = _mm_add_ps(r1,_mm_mul_ps(_mm_shuffle_ps(_L3,_L3,0xAA),B3));
    r2 = _mm_add_ps(r2,_mm_mul_ps(_mm_shuffle_ps(_L4,_L4,0xAA),B3));
    r1 = _mm_add_ps(r1,_mm_mul_ps(_mm_shuffle_ps(_L3,_L3,0xFF),B4));
    r2 = _mm_add_ps(r2,_mm_mul_ps(_mm_shuffle_ps(_L4,_L4,0xFF),B4));
    _L3 = r1;
    _L4 = r2;
    return *this;
}

// ----------------------------------------------------------------------------
//  Name:   VectorMult                          ___     ___
//  Desc:   Vector multiplication with matrix. [Res] = [Vec]*[Mat].
// ----------------------------------------------------------------------------
inline void VectorMult(const SPVector& Vec, const SPMatrix& Mat, SPVector& res) 
{
    __m128 result;
    result = _mm_mul_ps(_mm_shuffle_ps(Vec.vec,Vec.vec,0x00),Mat._L1);
    result = _mm_add_ps(result,_mm_mul_ps(_mm_shuffle_ps(Vec.vec,Vec.vec,0x55),Mat._L2));
    result = _mm_add_ps(result,_mm_mul_ps(_mm_shuffle_ps(Vec.vec,Vec.vec,0xAA),Mat._L3));
    result = _mm_add_ps(result,_mm_mul_ps(_mm_shuffle_ps(Vec.vec,Vec.vec,0xFF),Mat._L4));
    res = result;
}

// ----------------------------------------------------------------------------
//  Name:   VectorMult                                  ___
//  Desc:   Vector multiplication with matrix. Returns [Vec]*[Mat].
// ----------------------------------------------------------------------------
inline SPVector VectorMult(const SPVector& Vec, const SPMatrix& Mat)
{
    __m128 result;
	result = _mm_mul_ps(_mm_shuffle_ps(Vec.vec,Vec.vec,0x00),Mat._L1);
    result = _mm_add_ps(result,_mm_mul_ps(_mm_shuffle_ps(Vec.vec,Vec.vec,0x55),Mat._L2));
    result = _mm_add_ps(result,_mm_mul_ps(_mm_shuffle_ps(Vec.vec,Vec.vec,0xAA),Mat._L3));
    result = _mm_add_ps(result,_mm_mul_ps(_mm_shuffle_ps(Vec.vec,Vec.vec,0xFF),Mat._L4));
    return result;
}

// ----------------------------------------------------------------------------
//  Name:   SPVector * SPMatrix                         ___
//  Desc:   Vector multiplication with matrix. Returns [Vec]*[Mat].
// ----------------------------------------------------------------------------
inline SPVector operator * (const SPVector& Vec, const SPMatrix& Mat) {
    return VectorMult(Vec, Mat);
}

// ----------------------------------------------------------------------------
//  Name:   SPVector *= SPMatrix                ___     ___
//  Desc:   Vector multiplication with matrix. [Vec] = [Vec]*[Mat].
// ----------------------------------------------------------------------------
inline SPVector& SPVector::operator *= (const SPMatrix& Mat) {
    VectorMult(*this, Mat, *this);
    return *this;
}

// ----------------------------------------------------------------------------
//  Name:   SPVector *= float                ___     ___
//  Desc:   Vector multiplication by float. [Vec] = [Vec]*s.
// ----------------------------------------------------------------------------
inline SPVector& SPVector::operator *= (const float s) {
    vec = _mm_mul_ps(vec,_mm_set_ps1(s));
    return *this;
}

// ----------------------------------------------------------------------------
//  Name:   SPVector * float                ___     ___
//  Desc:   Vector multiplication by float. [Vec] = [Vec]*s.
// ----------------------------------------------------------------------------
inline SPVector operator* (const SPVector& A,const float s) {
	return _mm_mul_ps(A.vec,_mm_set_ps1(s));
};
 

// ----------------------------------------------------------------------------
//  Name:   SPVector + SPVector       _   _
//  Desc:   Vector addition. Returns [A]+[B].
// ----------------------------------------------------------------------------
inline SPVector operator + (const SPVector& A, const SPVector& B) {
    return _mm_add_ps(A.vec, B.vec);
}

// ----------------------------------------------------------------------------
//  Name:   SPVector - SPVector           _   _
//  Desc:   Vector substraction. Returns [A]-[B].
// ----------------------------------------------------------------------------
inline SPVector operator - (const SPVector& A, const SPVector& B) {
    return _mm_sub_ps(A.vec, B.vec);
}

// ----------------------------------------------------------------------------
//  Name:   SPVector += SPVector    _     _   _
//  Desc:   Vector addition.       [A] = [A]+[B].
// ----------------------------------------------------------------------------
inline SPVector& SPVector::operator += (const SPVector& B) {
    vec = _mm_add_ps(vec, B.vec);
    return *this;
}

// ----------------------------------------------------------------------------
//  Name:   SPVector -= SPVector    _     _   _
//  Desc:   Vector substraction.   [A] = [A]-[B].
// ----------------------------------------------------------------------------
inline SPVector& SPVector::operator -= (const SPVector& B) {
    vec = _mm_sub_ps(vec, B.vec);
    return *this;
}



// ----------------------------------------------------------------------------
//  Name:   SPVector % SPVector                           _   _
//  Desc:   Cross product of the two 3D vectors. Returns [A]x[B].
// ----------------------------------------------------------------------------
inline SPVector operator % (const SPVector& A, const SPVector& B) {
    __m128 l1, l2, m1, m2;
	l1 = _mm_shuffle_ps(A.vec,A.vec, _MM_SHUFFLE(3,1,0,2));
    l2 = _mm_shuffle_ps(B.vec,B.vec, _MM_SHUFFLE(3,0,2,1));
    m2 = _mm_and_ps(_mm_mul_ps(l1,l2),_0FFF_);
    l1 = _mm_shuffle_ps(A.vec,A.vec, _MM_SHUFFLE(3,0,2,1));
    l2 = _mm_shuffle_ps(B.vec,B.vec, _MM_SHUFFLE(3,1,0,2));
    m1 = _mm_mul_ps(l1,l2);
    return _mm_sub_ps(m1,m2);
}

// ----------------------------------------------------------------------------
//  Name:   SPVector * SPVector                      _   _
//  Desc:   Dot product of the two vectors. Returns [A]*[B].
// ----------------------------------------------------------------------------
inline float operator * (const SPVector& A, const SPVector& B) {
	__m128 r = _mm_mul_ps(A.vec,B.vec);
    r = _mm_add_ps(_mm_movehl_ps(r,r),r);
    __m128 t = _mm_add_ss(_mm_shuffle_ps(r,r,1), r);
    return *(float *)&t;
}

// ----------------------------------------------------------------------------
//  Name:   SPVector | SPVector
//  Desc:   Vector elements product.
// ----------------------------------------------------------------------------
inline SPVector operator | (const SPVector& A, const SPVector& B) {
    return _mm_mul_ps(A.vec, B.vec);
}


//	NewtonRaphson Reciprocal Square Root
// 	0.5 * rsqrtss * (3 - x * rsqrtss(x) * rsqrtss(x))
inline __m128 rsqrt_nr(const __m128 &a) {
	static const __m128 fvecf0pt5 = {0.5f,0.5f,0.5f,0.5f};
	static const __m128 fvecf3pt0 = {3.0f,3.0f,3.0f,3.0f};
	__m128 Ra0 = _mm_rsqrt_ps(a);
	return _mm_mul_ps(_mm_mul_ps(fvecf0pt5,Ra0),(_mm_sub_ps(fvecf3pt0,_mm_mul_ps(_mm_mul_ps(a,Ra0),Ra0))));
}

// ----------------------------------------------------------------------------
//  Name:   ~SPVector [Normalize]
//  Desc:   Normalized the source vector.
// ----------------------------------------------------------------------------

inline SPVector& SPVector::normalize() {
    __m128 r;
	r = _mm_mul_ps(vec,vec);
    r = _mm_add_ps(_mm_movehl_ps(r,r),r);
    __m128 t;
	t = _mm_add_ss(_mm_shuffle_ps(r,r,1), r);
    t = rsqrt_nr(t);
    vec = _mm_mul_ps(vec, _mm_shuffle_ps(t,t,0x00));
    return *this;
}

