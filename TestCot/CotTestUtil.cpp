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

#include "../Dzcot/Inc_DzFastNewDel.h"

int MainEntry( int argc, _TCHAR* argv[] )
{
    return DzRunHost( CP_LOW, CP_LOW, SS_64K, DzcotEntry, 0 );
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
    int ret = DzRunHost( CP_LOW, CP_LOW, SS_64K, entry, context );
    EXPECT_EQ( DS_OK, ret );
}
