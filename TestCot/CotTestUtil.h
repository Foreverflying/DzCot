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

#define DZ_EXPECT_EQ( a, b ) ( std::cout << (a) << (b) )
#define DZ_EXPECT_NE( a, b ) ( std::cout << (a) << (b) )
#define DZ_EXPECT_GE( a, b ) ( std::cout << (a) << (b) )
#define DZ_EXPECT_LE( a, b ) ( std::cout << (a) << (b) )
#define DZ_EXPECT_GT( a, b ) ( std::cout << (a) << (b) )
#define DZ_EXPECT_LT( a, b ) ( std::cout << (a) << (b) )
#define DZ_EXPECT_HRESULT_SUCCEEDED( a ) ( std::cout << (a) )
#define DZ_EXPECT_HRESULT_FAILED( a ) ( cout << (a) )
#define DZ_ASSERT_EQ( a, b ) ( std::cout << (a) << (b) )
#define DZ_ASSERT_NE( a, b ) ( std::cout << (a) << (b) )
#define DZ_ASSERT_GE( a, b ) ( std::cout << (a) << (b) )
#define DZ_ASSERT_LE( a, b ) ( std::cout << (a) << (b) )
#define DZ_ASSERT_GT( a, b ) ( std::cout << (a) << (b) )
#define DZ_ASSERT_LT( a, b ) ( std::cout << (a) << (b) )
#define DZ_ASSERT_HRESULT_SUCCEEDED( a ) ( std::cout << (a) )
#define DZ_ASSERT_HRESULT_FAILED( a ) ( std::cout << (a) )
#define DZ_FAIL() ( std::cout << 0 )
#define DZ_ADD_FAILURE() ( std::cout << 0 )
#define DZ_SUCCEED() ( std::cout << 0 )

#define TEST( caseName, testName )\
    void __Test##CaseName##testName( void )

#else

#include <gtest/gtest.h>

#define DZ_EXPECT_EQ( a, b ) EXPECT_EQ( (ssize_t)(a), (ssize_t)(b) )
#define DZ_EXPECT_NE( a, b ) EXPECT_NE( (ssize_t)(a), (ssize_t)(b) )
#define DZ_EXPECT_GE( a, b ) EXPECT_GE( (ssize_t)(a), (ssize_t)(b) )
#define DZ_EXPECT_LE( a, b ) EXPECT_LE( (ssize_t)(a), (ssize_t)(b) )
#define DZ_EXPECT_GT( a, b ) EXPECT_GT( (ssize_t)(a), (ssize_t)(b) )
#define DZ_EXPECT_LT( a, b ) EXPECT_LT( (ssize_t)(a), (ssize_t)(b) )
#define DZ_EXPECT_HRESULT_SUCCEEDED( a ) EXPECT_HRESULT_SUCCEEDED( (a) )
#define DZ_EXPECT_HRESULT_FAILED( a ) EXPECT_HRESULT_FAILED( (a) )
#define DZ_ASSERT_EQ( a, b ) ASSERT_EQ( (ssize_t)(a), (ssize_t)(b) )
#define DZ_ASSERT_NE( a, b ) ASSERT_NE( (ssize_t)(a), (ssize_t)(b) )
#define DZ_ASSERT_GE( a, b ) ASSERT_GE( (ssize_t)(a), (ssize_t)(b) )
#define DZ_ASSERT_LE( a, b ) ASSERT_LE( (ssize_t)(a), (ssize_t)(b) )
#define DZ_ASSERT_GT( a, b ) ASSERT_GT( (ssize_t)(a), (ssize_t)(b) )
#define DZ_ASSERT_LT( a, b ) ASSERT_LT( (ssize_t)(a), (ssize_t)(b) )
#define DZ_ASSERT_HRESULT_SUCCEEDED( a ) ASSERT_HRESULT_SUCCEEDED( (a) )
#define DZ_ASSERT_HRESULT_FAILED( a ) ASSERT_HRESULT_FAILED( (a) )
#define DZ_FAIL() FAIL()
#define DZ_ADD_FAILURE() ADD_FAILURE()
#define DZ_SUCCEED() SUCCEED()

#endif

void TestCot( DzEntry entry, intptr_t context = 0 );
int MainEntry( int argc, _TCHAR* argv[] );
