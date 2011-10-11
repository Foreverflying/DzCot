/********************************************************************
    created:    2011/09/14 0:32
    file:       CotTestUtil.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#pragma once

#include "stdafx.h"
#include "DzDbgTrace.h"
#include "../Dzcot/Inc_Dzcot.h"

#ifdef TEST_COT_JUST_RUN_TRY

#include <iostream>

#define EXPECT_EQ( ... ) ( std::cout << "hello" )
#define EXPECT_NE( ... ) ( std::cout << "hello" )
#define EXPECT_GE( ... ) ( std::cout << "hello" )
#define EXPECT_LE( ... ) ( std::cout << "hello" )
#define EXPECT_GT( ... ) ( std::cout << "hello" )
#define EXPECT_LT( ... ) ( std::cout << "hello" )
#define EXPECT_HRESULT_SUCCEEDED( ... ) ( std::cout << "hello" )
#define EXPECT_HRESULT_FAILED( ... ) ( cout << "hello" )
#define ASSERT_EQ( ... ) ( std::cout << "hello" )
#define ASSERT_NE( ... ) ( std::cout << "hello" )
#define ASSERT_GE( ... ) ( std::cout << "hello" )
#define ASSERT_LE( ... ) ( std::cout << "hello" )
#define ASSERT_GT( ... ) ( std::cout << "hello" )
#define ASSERT_LT( ... ) ( std::cout << "hello" )
#define ASSERT_HRESULT_SUCCEEDED( ... ) ( std::cout << "hello" )
#define ASSERT_HRESULT_FAILED( ... ) ( std::cout << "hello" )
#define FAIL( ... ) ( std::cout << "hello" )
#define ADD_FAILURE( ... ) ( std::cout << "hello" )
#define SUCCEED( ... ) ( std::cout << "hello" )

#define TEST( caseName, testName )\
    void __Test##CaseName##testName( void )

#else

#include <gtest/gtest.h>

#endif

void TestCot( DzRoutine entry, intptr_t context = 0 );
int MainEntry( int argc, _TCHAR* argv[] );
