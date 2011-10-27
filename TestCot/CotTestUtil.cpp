/********************************************************************
    created:    2011/09/14 0:31
    file:       CotTestUtil.cpp
    author:     Foreverflying
    purpose:    
*********************************************************************/

#include "stdafx.h"
#include "CotTestUtil.h"
#include "TryCot.h"

#ifdef TEST_COT_JUST_RUN_TRY

//#include "../Dzcot/Inc_DzFastNewDelEx.h"

int MainEntry( int argc, _TCHAR* argv[] )
{
    return DzRunHosts( gHostCount, gServMask, CP_LOW, CP_LOW, SS_64K, TestCotTryEntry, 0 );
}

#else

#include "ConfigTestCot.h"

int MainEntry( int argc, _TCHAR* argv[] )
{
    int ac = sizeof( gArgv ) / sizeof( char* );
    if( argc == 1 && ac > 0 && *gArgv[0] == 'T' ){
        testing::InitGoogleTest( &ac, (char**)gArgv );
    }else{
        testing::InitGoogleTest( &argc, argv );
    }
    return RUN_ALL_TESTS();
}

#endif

void TestCot( DzRoutine entry, intptr_t context )
{
    int servMask[] = {
        -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1
    };
    int ret = DzRunHosts( 1, servMask, CP_LOW, CP_LOW, SS_64K, entry, context );
    EXPECT_EQ( DS_OK, ret );
}