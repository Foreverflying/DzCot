
#include "stdafx.h"
#include <gtest/gtest.h>
#include "Util.h"

static int gCount = 0;

void __stdcall TestCppException( void* context )
{

}

TEST( TestException, CppException )
{
    TestCot( TestCppException );
}

#ifdef _WIN32

struct TestExpData
{
    int     a;
    int     b;
    int     x;
    int     y;
};

void HandleExpFunc( int delay, TestExpData* data )
{
    __try{
        DzSleep( delay );
        data->b = 10 / data->a;
        data->x = data->b;
    }__except( EXCEPTION_EXECUTE_HANDLER ){
        data->y = 5;
    }
}

void ContinueExpFunc( int delay, TestExpData* data )
{
    __try{
        DzSleep( delay );
        data->b = 10 / data->a;
        data->x = data->b;
    }__except( data->a = 5, data->y = 5, EXCEPTION_CONTINUE_EXECUTION ){
    }
}

void __stdcall SehExpRoutine( void* context )
{
    gCount++;
    int delay = (int)context;

    TestExpData data;
    data.a = 0;
    data.b = 0;
    data.x = 0;
    data.y = 0;
    HandleExpFunc( delay, &data );
    EXPECT_EQ( 0, data.x );
    EXPECT_EQ( 5, data.y );

    data.a = 0;
    data.b = 0;
    data.x = 0;
    data.y = 0;
    ContinueExpFunc( delay, &data );
    EXPECT_EQ( 2, data.b );
    EXPECT_EQ( 2, data.x );
    EXPECT_EQ( 5, data.y );
}

void __stdcall TestWinSehException( void* context )
{
    gCount = 0;
    int n = (int)context;
    int delay = 10;
    DzHandle evt[16];
    for( int i = 0; i < n; i++ ){
        evt[i] = DzCreateEvt( TRUE, FALSE );
        DzEvtStartCot( evt[i], SehExpRoutine, (void*)delay );
        delay += 3;
    }
    DzWaitMultiSynObj( n, evt, TRUE );
    for( int i = 0; i < n; i++ ){
        DzCloseSynObj( evt[i] );
    }
    EXPECT_EQ( n, gCount );
}

TEST( TestException, WinSehException )
{
    TestCot( TestWinSehException, (void*)8 );
}

#endif