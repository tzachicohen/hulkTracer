//////////////////////////////////////////////////////////////////
// hulk_assert.h												//
// Hulk renderer - Create by Tzachi Cohen 2008					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////

#include <iostream>
#include <windows.h>

#ifndef HULK_ASSERT_HPP
#define HULK_ASSERT_HPP



#ifdef _DEBUG
#	define HULK_ASSERT(exp,str,...) \
		if (!(exp)) \
		{ \
			printf(str,__VA_ARGS__) ; \
			DebugBreak() ; \
		}

#	define HULK_WARNING( exp , str ) \
		if (!(exp)) \
		{ \
			std::cout << str ; \
		}
#else
#	define HULK_ASSERT( exp , str ,...)
#	define HULK_WARNING( exp , str,... )
#endif


#	define HULK_PRINT(str,...) \
        { \
        char msg[256]; \
        sprintf(msg,str, __VA_ARGS__); \
        OutputDebugStringA(msg); \
        }

enum EHulkResult
{
	e_ok = 0,
	e_allocFail,
	e_fileOpenFail,
	e_fileCloseFail,
	e_illegalInput,
	e_nodeNotFound
};

#endif