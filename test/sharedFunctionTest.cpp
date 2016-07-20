

#include "gtest\gtest.h"
#include "SPMatrix.hpp"
#include "Sharedfunctions.hpp"

TEST(Sharedfunctions, GetBoxVolume) {

	Box a;
	a.m_min.x = 0;
	a.m_min.y = 0;
	a.m_min.z = 0;
	a.m_max.x = 1.0f;
	a.m_max.y = 2.0f;
	a.m_max.z = 3.0f;
	float ans =  (1.0f*2.0f *3.0f);
   EXPECT_EQ(ans, getBoxVolume(a));

}

TEST(Sharedfunctions, GetBoxSurface) {

	Box a;
	a.m_min.x = 0;
	a.m_min.y = 0;
	a.m_min.z = 0;
	a.m_max.x = 1.0f;
	a.m_max.y = 2.0f;
	a.m_max.z = 3.0f;
	float ans =  2.0f*(1.0f*2.0f+ 2.0f*3.0f + 1.0f*3.0f);
   EXPECT_EQ(ans, getBoxSurface(a));

}

TEST(Sharedfunctions, isPowerOfTwo) {
 
   EXPECT_TRUE(isPowerOfTwo(2));
   EXPECT_TRUE(isPowerOfTwo(128));
   EXPECT_TRUE(isPowerOfTwo(256));
   EXPECT_TRUE(isPowerOfTwo(512));

   EXPECT_FALSE(isPowerOfTwo(333));
   EXPECT_FALSE(isPowerOfTwo(100));
   EXPECT_FALSE(isPowerOfTwo(77));
}

TEST(Sharedfunctions, alignUp) 
{
	for (size_t i = 0 ; i < 1024; i++) 
	{
		size_t number = alignUp(i,64);
		EXPECT_TRUE( number % 64 == 0);
		EXPECT_TRUE( number >= i );
		EXPECT_TRUE( number - i < 64 );
	}
}