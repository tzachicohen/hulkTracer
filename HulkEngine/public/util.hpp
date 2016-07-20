

#ifndef HULD_UTIL_H
#define HULD_UTIL_H

#include <malloc.h>


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