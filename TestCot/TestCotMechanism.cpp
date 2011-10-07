
#include "stdafx.h"
#include <gtest/gtest.h>
#include "Util.h"
#include "DRandom.h"
#include <malloc.h>

static int gCount = 0;
static DRandom* gRand = NULL;

void __stdcall CotStackGrowRoutine( void* context )
{
    gCount++;

    int len = (int)context;
    int m = 0;
    int n = 0;

    char* buff = (char*)alloca( len );
    for( int i = 0; i < len; i++ ){
        buff[i] = (char)( i % 128 );
        m += buff[i];
    }
    DzSleep( gRand->rand( 0, 30 ) );
    for( int i = 0; i < len; i++ ){
        n += buff[ len - i - 1 ];
    }
    EXPECT_EQ( m, n );
}

void __stdcall TestCotStackGrow( void* context )
{
    gCount = 0;
    int count = (int)context;
    gRand = new DRandom( 542234 );
    DzHandle evt = DzCreateCountDownEvt( count );
    for( int i = 0; i < count; i++ ){
        DzEvtStartCot( evt, CotStackGrowRoutine, (void*)32768 );
    }
    DzWaitSynObj( evt );
    EXPECT_EQ( count, gCount );
    delete gRand;
    gRand = NULL;
}

TEST( TestCotMechanism, CotStackGrow )
{
    TestCot( TestCotStackGrow, (void*)2000 );
}