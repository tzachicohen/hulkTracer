/*
**	  GPU Raytrace - Diploma thesis, Czech technical university
**	  Copyright (C) 2008 Martin Zlatuska
**
**	  This program is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <xmmintrin.h>


extern __m128 zeroesInit;
extern __m128 onesInit;

extern __m128 minimumInit;
extern __m128 maximumInit;
extern __m128 minimumInit3;
extern __m128 maximumInit3;

#undef min
#undef max

namespace sse {
	inline void store(__m128i src,void* dst) {
		_mm_store_si128((__m128i*)dst,src);
	};

	inline void store(__m128  src,void* dst) {
		_mm_store_ps((float*)dst,src);
	};

	inline void store1(__m128 src,void* dst) {
		_mm_store_ss((float*)dst,src);
	};


	template<typename T>
	inline T load(void *src) {};

	template<>
	inline __m128i load<__m128i>(void *src) {
		return _mm_load_si128((__m128i*)src);
	};

	template<>
	inline __m128 load<__m128>(void *src) {
		return _mm_load_ps((float*)src);
	};

	inline int extract(__m128i src) {
		return _mm_cvtsi128_si32(src);
	};

	inline __m128i pack(int src) {
		return _mm_cvtsi32_si128(src);
	};

	inline __m128 operator+(__m128 a,__m128 b) {
		return _mm_add_ps(a,b);
	};

	inline __m128 operator-(__m128 a,__m128 b) {
		return _mm_sub_ps(a,b);
	};

	inline __m128 operator*(__m128 a,__m128 b) {
		return _mm_mul_ps(a,b);
	};

	inline __m128 operator/(__m128 a,__m128 b) {
		return _mm_div_ps(a,b);
	};


	inline __m128 operator<(__m128 a,__m128 b) {
		return _mm_cmplt_ps(a,b);
	};

	inline __m128 operator==(__m128 a,__m128 b) {
		return _mm_cmpeq_ps(a,b);
	};

	inline __m128 operator&(__m128 a,__m128 b) {
		return _mm_and_ps(a,b);
	};

	inline __m128 set(float a,float b,float c,float d) {
		return _mm_set_ps(a,b,c,d);
	};

	inline __m128i set(int a,int b,int c,int d) {
		return _mm_setr_epi32(a,b,c,d);
	};

	inline __m128 set(float a) {
		return _mm_set_ps(a,a,a,a);
	};

	inline __m128 set4(int a) {
		__m128 res;
		res = _mm_cvtsi32_ss(res,a);
		res = _mm_shuffle_ps(res,res,_MM_SHUFFLE(0,0,0,0));
		return res;
	};

	inline __m128i f4i(__m128 a) {
		return _mm_cvtps_epi32(a);
	};

	inline __m128 i4f(__m128i a) {
		return _mm_cvtepi32_ps(a);
	};

	inline __m128 min(__m128 a,__m128 b) {
		return _mm_min_ps(a,b);
	};	

	inline __m128 max(__m128 a,__m128 b) {
		return _mm_max_ps(a,b);
	};	

	inline void sum(__m128 &a) {
		__m128 temp;
		temp = _mm_shuffle_ps(a,a,_MM_SHUFFLE(2,3,0,1));
		a	 = a + temp;
		temp = _mm_shuffle_ps(a,a,_MM_SHUFFLE(1,0,3,2));
		a	 = a + temp;
	};

	inline void operator+=(__m128i &a,__m128i b) {
		a = _mm_add_epi64(a,b);
	}

	inline __m128i operator&(__m128i a,__m128i b) {
		return _mm_and_si128(a,b);
	};

	inline __m128 not(__m128 a) {
		__m128 b;
		static unsigned int stat = 0xffffffff;
		float * p = reinterpret_cast<float*> ( &stat);
		_mm_load1_ps(p);
		return _mm_andnot_ps(a,b);
	};

	inline __m128i operator|(__m128i a,__m128i b) {
		return _mm_or_si128(a,b);
	};
};

