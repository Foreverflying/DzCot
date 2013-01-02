
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

    DzDelCallbackTimer( timer );
}

void __stdcall TestOneOffCallbackTimer( intptr_t context )
{
    gCount = 0;
    DzHandle timer = DzCreateCallbackTimer( 500, FALSE, CallbackTimerRoutine );

    DzSleep( 200 );
    EXPECT_EQ( 0, gCount );
    DzSleep( 500 );
    EXPECT_EQ( 1, gCount );
    DzSleep( 100 );
    EXPECT_EQ( 1, gCount );
    DzSleep( 500 );
    EXPECT_EQ( 1, gCount );

    DzDelCallbackTimer( timer );
}

void __stdcall TestRepeatCallbackTimer( intptr_t context )
{
    gCount = 0;
    DzHandle timer = DzCreateCallbackTimer( 500, TRUE, CallbackTimerRoutine );

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

    DzDelCallbackTimer( timer );

    DzSleep( 500 );
    EXPECT_EQ( 6, gCount );
}

void __stdcall TestEnsureTimerCancelAfterClose( intptr_t context )
{
    gCount = 0;
    DzHandle timer = DzCreateCallbackTimer( 500, TRUE, CallbackTimerRoutine, NULL, CP_NORMAL );

    DzSleep( 500 );
    EXPECT_EQ( 1, gCount );
    DzSleep( 500 );
    EXPECT_EQ( 2, gCount );

    helpCalledCount = 0;
    DzHandle helpTimer = DzCreateCallbackTimer( 500, FALSE, HelpTestCallback, (intptr_t)timer, CP_HIGH );

    DzSleep( 500 );
    EXPECT_EQ( 2, gCount );

    DzDelCallbackTimer( helpTimer );
}

TEST( TestCallbackTimer, OneOffCallbackTimer )
{
    TestCot( TestOneOffCallbackTimer );
}

TEST( TestCallbackTimer, RepeatCallbackTimer )
{
    TestCot( TestRepeatCallbackTimer );
}

TEST( TestCallbackTimer, EnsureTimerCancelAfterClose )
{
    TestCot( TestEnsureTimerCancelAfterClose );
}