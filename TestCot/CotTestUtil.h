/********************************************************************
    created:    2011/09/14 0:32
    file:       CotTestUtil.h
    author:     Foreverflying
    purpose:    
********************************************************************/

#pragma once

#include "stdafx.h"
#include "DzTrace.h"
#include "../DzCot/Inc_DzCot.h"

#ifdef TEST_COT_JUST_RUN_TRY

#include <iostream>

#define EXPECT_EQ( a, b ) ( std::cout << (a) << (b) )
#define EXPECT_NE( a, b ) ( std::cout << (a) << (b) )
#define EXPECT_GE( a, b ) ( std::cout << (a) << (b) )
#define EXPECT_LE( a, b ) ( std::cout << (a) << (b) )
#define EXPECT_GT( a, b ) ( std::cout << (a) << (b) )
#define EXPECT_LT( a, b ) ( std::cout << (a) << (b) )
#define EXPECT_HRESULT_SUCCEEDED( a ) ( std::cout << (a) )
#define EXPECT_HRESULT_FAILED( a ) ( cout << (a) )
#define ASSERT_EQ( a, b ) ( std::cout << (a) << (b) )
#define ASSERT_NE( a, b ) ( std::cout << (a) << (b) )
#define ASSERT_GE( a, b ) ( std::cout << (a) << (b) )
#define ASSERT_LE( a, b ) ( std::cout << (a) << (b) )
#define ASSERT_GT( a, b ) ( std::cout << (a) << (b) )
#define ASSERT_LT( a, b ) ( std::cout << (a) << (b) )
#define ASSERT_HRESULT_SUCCEEDED( a ) ( std::cout << (a) )
#define ASSERT_HRESULT_FAILED( a ) ( std::cout << (a) )
#define FAIL() ( std::cout << 0 )
#define ADD_FAILURE() ( std::cout << 0 )
#define SUCCEED() ( std::cout << 0 )

#define TEST( caseName, testName )\
    void __Test##CaseName##testName( void )

#else

#include <gtest/gtest.h>

#endif

void TestCot( DzRoutine entry, intptr_t context = 0 );
int MainEntry( int argc, _TCHAR* argv[] );
