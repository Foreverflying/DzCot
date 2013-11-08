/**
 *  @file       CotTestUtil.cpp
 *  @brief      
 *  @author	    Foreverflying <foreverflying@live.cn>
 *  @date       2011/09/14
 *
 */

#include "stdafx.h"
#include "CotTestUtil.h"
#include "ConfigTestCot.h"

CotEntry CleanEntry( intptr_t context )
{
    __DzTceEnableScopePrint( 1 );
    for( int i = 0; i < 4; i++ ){
        __DzTce1( "stack %d used size: %d", i, __DzDbgMaxStackUse( i ) );
    }
    __DzTce1( "Hosts exit" );
}

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

void TestCot( DzEntry entry, intptr_t context )
{
    int ret = DzRunHosts( 0, 4000, 64000, 1024 * 1024, CP_LOW, CP_LOW, ST_UM, entry, context, NULL );
    DZ_EXPECT_EQ( DS_OK, ret );
}

int main(int argc, _TCHAR* argv[])
{
    int ret = MainEntry( argc, argv );
    getchar();
    return ret;
}

int _tmain(int argc, _TCHAR* argv[])
{
    int ret = MainEntry( argc, argv );
    getchar();
    return ret;
}
