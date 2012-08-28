/********************************************************************
    created:    2011/09/14 0:31
    file:       CotTestUtil.cpp
    author:     Foreverflying
    purpose:    
********************************************************************/

#include "stdafx.h"
#include "CotTestUtil.h"
#include "TryCot.h"

void __stdcall CleanEntry( intptr_t context )
{
    __DzTceEnableScopePrint( 1 );
    __DzTce1( "Hosts exit" );
}

#ifdef TEST_COT_JUST_RUN_TRY

//#include "../DzCot/Inc_DzFastNewDelEx.h"

int MainEntry( int argc, _TCHAR* argv[] )
{
    return DzRunHosts( gHostCount, CP_LOW, CP_LOW, SS_64K, TestCotTryEntry, 0, CleanEntry );
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
    int ret = DzRunHosts( 0, CP_LOW, CP_LOW, SS_64K, entry, context, NULL );
    EXPECT_EQ( DS_OK, ret );
}
