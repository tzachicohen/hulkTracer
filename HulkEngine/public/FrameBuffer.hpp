//////////////////////////////////////////////////////////////////
// Hulk renderer - Create by Tzachi Cohen - all rights reserved //
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////

#ifndef HULK_FRAME_BUFFER
#define HULK_FRAME_BUFFER

#include <stdio.h>
#include <windows.h>

#include "hulk_assert.h"
#include "SPMatrix.hpp"
#include "SseUtils.hpp"

//! this class in templated over the pixel format
//! can be instanciated over different pixel sizes.
template <class PixelFormat>
class FrameBuffer
{
public:
	//////////////////////////////////////
	//public initialization and cleanup //
	//////////////////////////////////////
	FrameBuffer();
	virtual ~FrameBuffer();
	EHulkResult initialize(unsigned int width ,unsigned int height , PixelFormat initVal);

	////////////////////
	// public methods //
	////////////////////
	EHulkResult saveToFile (std::string fileName);
	inline void replacePixel(unsigned int row , unsigned int column ,const SPVector & value);
	inline unsigned int getWidth();
	inline unsigned int getHeight();
	void * getBufferBits();
	inline void clearBuffer(PixelFormat value =0 ) 
		{ for (uint i = 0 ; i < m_height*m_width; i++) m_pBuffer[i] = value;}
private:
	/////////////////////
	// private members //
	/////////////////////
	PixelFormat * m_pBuffer;
	unsigned int m_width;
	unsigned int m_height;
	unsigned int m_size; //size of the frame buffer in bytes;
	//////////////////////////////
	// private support function //
	//////////////////////////////
	PixelFormat & getPixel(unsigned int row , unsigned int column);
	///////////////////////////////////////
	//private initialization and cleanup //
	///////////////////////////////////////
	FrameBuffer & operator=(FrameBuffer & f);
	FrameBuffer(FrameBuffer & f);
};

template <class PixelFormat>
FrameBuffer<PixelFormat>::FrameBuffer()
:m_pBuffer(NULL),
m_width(0),
m_height(0),
m_size(0)
{
}

template <class PixelFormat>
FrameBuffer<PixelFormat>::~FrameBuffer()
{
	if (m_pBuffer)
	{
		delete m_pBuffer;
		m_pBuffer = NULL;
	}
}

template <class PixelFormat>
EHulkResult FrameBuffer<PixelFormat>::initialize(unsigned int width ,unsigned  int height , PixelFormat initVal)
{
	HULK_ASSERT(width != 0 ,"trying to initialize a frame buffer with width =0");
	HULK_ASSERT(height != 0 ,"trying to initialize a frame buffer with height =0");

	//clear the old buffer if exists.
	if (NULL != m_pBuffer)
	{
		delete m_pBuffer;
		m_pBuffer = NULL;
	}
	// allocate memory
	m_size = width* height * sizeof(PixelFormat);
	m_pBuffer = new PixelFormat[width * height];
	
	if (NULL == m_pBuffer)
	{
		return e_allocFail;
	}
	else
	{
		//set initial value
		PixelFormat * pPixel = m_pBuffer;
		m_width = width;
		m_height = height;
		for (unsigned int i = 0 ;i < width *height ;i++)
		{
			*pPixel = initVal;
			pPixel++;
		}
		return e_ok;
	}
}


//explicit specialization for unsigned int 
template <class PixelFormat>
EHulkResult FrameBuffer<PixelFormat>::saveToFile (std::string fileName)
{
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER bmpHeader;
	
	if ((m_width %4) != 0 || (m_height % 4) !=0) 
	{
		return  e_illegalInput;
	}
	
	//BITMAPFILEHEADER
	fileHeader.bfType = 19778;//unique identifier for BMP
	fileHeader.bfReserved1 = 0;
	fileHeader.bfReserved2 = 0;
	fileHeader.bfOffBits = sizeof (fileHeader)+sizeof(bmpHeader);
	fileHeader.bfSize = sizeof(fileHeader)+sizeof(bmpHeader) + m_width*m_height*3; //size of file

	//BITMAPINFOHEADER
	bmpHeader.biBitCount = 24;
	bmpHeader.biSize = sizeof (bmpHeader);
	bmpHeader.biWidth = m_width;
	bmpHeader.biHeight = m_height;
	bmpHeader.biCompression = BI_RGB;
	bmpHeader.biPlanes = 1;
	bmpHeader.biSizeImage = m_width*m_height*3;
	bmpHeader.biXPelsPerMeter = 0;
	bmpHeader.biYPelsPerMeter = 0 ;
	bmpHeader.biClrUsed = 0;
	bmpHeader.biClrImportant = 0;


	// add ".bmp" suffx
	std::string name;
	name = fileName;
	name += ".bmp";

	FILE * file = NULL;
	fopen_s (&file, name.c_str(),"wb");
	if (!file)
	{
		return e_fileOpenFail;
	}
	//allocate a temporary buffer TODO: try avoiding this.
	char * pNoAlphaBuffer = new char [m_width*m_height*3];
	char * pDest = pNoAlphaBuffer;
	char * pSource = (char*) m_pBuffer;
	for (unsigned int i = 0 ; i < m_width*m_height;i++)
	{
		//copy the first color component
		*pDest = *pSource;
		pDest++;pSource++;
		
		//copy the second color component
		*pDest = *pSource;
		pDest++;pSource++;
		
		//copy the third color component
		*pDest = *pSource;
		pDest++;pSource++;

		pSource++;
	}

	fwrite (&fileHeader,sizeof(fileHeader),1,file);
	fwrite (&bmpHeader,sizeof(bmpHeader),1,file);
	fwrite (pNoAlphaBuffer,m_width*m_height*3,1,file);
	
	//release temporary buffer
	delete [] pNoAlphaBuffer;

	if (fclose(file))
	{
		return e_fileCloseFail;
	}
	
	
	return e_ok;
}

//! remark: row and column are in screen space (openGL convention)
template <class PixelFormat>
inline void FrameBuffer<PixelFormat>::replacePixel(unsigned int row , unsigned int column ,const SPVector & valueVec)
{
	//HULK_ASSERT (row < m_width,"row index out of bound")
	//HULK_ASSERT (column <  m_height,"row index out of bound")
	//translate from float values to 
	unsigned int value = 0 ;
	SPVector clampedVector = valueVec;
	//iterate for each color component
	static const SPVector ones(1.0,1.0,1.0,1.0);
	//clamp to [0..1] 
	clampedVector.vec = sse::min(ones.vec,valueVec.vec);
	// normalize from [0..1] to  [0 -255] 
	clampedVector *=  255.0f;
	//convert to int
	__m128i intVaule = sse::f4i(clampedVector.vec);

	value |= intVaule.m128i_i32[0];
	value |= intVaule.m128i_i32[1] << 8;
	value |= intVaule.m128i_i32[2] << 16;
	
	getPixel(row,column) = value;
}

template <class PixelFormat>
inline unsigned int FrameBuffer<PixelFormat>::getWidth()
{
	return m_width;
}
template <class PixelFormat>
inline unsigned int FrameBuffer<PixelFormat>::getHeight()
{
	return m_height;
}

template <class PixelFormat>
void *  FrameBuffer<PixelFormat>::getBufferBits()
{
	return m_pBuffer;
}
//////////////////////////////
// private support function //
//////////////////////////////
template <class PixelFormat>
PixelFormat & FrameBuffer<PixelFormat>::getPixel(unsigned int row , unsigned int column)
{
	int index = column*m_width + row;
	return m_pBuffer[index];
}

///////////////////////////////////////////////////
// private copy constructors to prevent override //
///////////////////////////////////////////////////
template <class PixelFormat>
FrameBuffer<PixelFormat> & FrameBuffer<PixelFormat>::operator=(FrameBuffer<PixelFormat> & f)
{
	return *this;
}

template <class PixelFormat>
FrameBuffer<PixelFormat>::FrameBuffer(FrameBuffer<PixelFormat> & f)
{
}

#endif // end of HULD_FRAME_BUFFER