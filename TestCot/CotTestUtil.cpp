/********************************************************************
    created:    2011/09/14 0:31
    file:       CotTestUtil.cpp
    author:     Foreverflying
    purpose:    
********************************************************************/

#include "stdafx.h"
#include "CotTestUtil.h"
#include "TryCot.h"

CotEntry CleanEntry( intptr_t context )
{
    __DzTceEnableScopePrint( 1 );
    for( int i = 0; i < 4; i++ ){
        __DzTce1( "stack %d used size: %d", i, __DzDbgMaxStackUse( i ) );
    }
    __DzTce1( "Hosts exit" );
}

#ifdef TEST_COT_JUST_RUN_TRY

//#include "../DzCot/Inc_DzFastNewDelEx.h"

int MainEntry( int argc, _TCHAR* argv[] )
{
    return DzRunHosts( gHostCount, 4000, 64000, 128000, CP_LOW, CP_LOW, ST_UM, TestCotTryEntry, 0, CleanEntry );
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

void TestCot( DzEntry entry, intptr_t context )
{
    int ret = DzRunHosts( 0, 4000, 64000, 1024 * 1024, CP_LOW, CP_LOW, ST_UM, entry, context, NULL );
    DZ_EXPECT_EQ( DS_OK, ret );
}
