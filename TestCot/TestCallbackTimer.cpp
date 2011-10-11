
#include "CotTestUtil.h"

static int gCount = 0;
static int helpCalledCount = 0;

void __stdcall CallbackTimerRoutine( intptr_t context )
{
    gCount++;
}

void __stdcall HelpTestCallback( intptr_t context )
{
    DzHandle timer = (DzHandle)context;

    DzCloseCallbackTimer( timer );
}

void __stdcall TestFiveRepeatCallbackTimer( intptr_t context )
{
    gCount = 0;
    DzHandle timer = DzCreateCallbackTimer( 500, 5, CallbackTimerRoutine );

    DzSleep( 800 );
    EXPECT_EQ( 1, gCount );
    DzSleep( 800 );
    EXPECT_EQ( 3, gCount );
    DzSleep( 500 );
    EXPECT_EQ( 4, gCount );
    DzSleep( 500 );
    EXPECT_EQ( 5, gCount );
    DzSleep( 500 );
    EXPECT_EQ( 5, gCount );

    DzCloseCallbackTimer( timer );
}

void __stdcall TestInfiniteRepeatCallbackTimer( intptr_t context )
{
    gCount = 0;
    DzHandle timer = DzCreateCallbackTimer( 500, 0, CallbackTimerRoutine );

    DzSleep( 800 );
    EXPECT_EQ( 1, gCount );
    DzSleep( 800 );
    EXPECT_EQ( 3, gCount );
    DzSleep( 500 );
    EXPECT_EQ( 4, gCount );
    DzSleep( 500 );
    EXPECT_EQ( 5, gCount );
    DzSleep( 500 );
    EXPECT_EQ( 6, gCount );

    DzCloseCallbackTimer( timer );

    DzSleep( 500 );
    EXPECT_EQ( 6, gCount );
}

void __stdcall TestEnsureTimerCancelAfterClose( intptr_t context )
{
    gCount = 0;
    DzHandle timer = DzCreateCallbackTimer( 500, 0, CallbackTimerRoutine, NULL, CP_NORMAL );

    DzSleep( 500 );
    EXPECT_EQ( 1, gCount );
    DzSleep( 500 );
    EXPECT_EQ( 2, gCount );

    helpCalledCount = 0;
    DzHandle helpTimer = DzCreateCallbackTimer( 500, 1, HelpTestCallback, (intptr_t)timer, CP_HIGH );

    DzSleep( 500 );
    EXPECT_EQ( 2, gCount );

    DzCloseCallbackTimer( helpTimer );
}

TEST( TestCallbackTimer, FiveRepeatCallbackTimer )
{
    TestCot( TestFiveRepeatCallbackTimer );
}

TEST( TestCallbackTimer, InfiniteRepeatCallbackTimer )
{
    TestCot( TestInfiniteRepeatCallbackTimer );
}

TEST( TestCallbackTimer, EnsureTimerCancelAfterClose )
{
    TestCot( TestEnsureTimerCancelAfterClose );
}