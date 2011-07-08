#include "stdafx.h"
#include "TestCallbackTimer.h"
#include "../Dzcot/Dzcot.h"
#include <sys/timeb.h>

#define MAX_TEST_TIMER 50

static int count = 0;
static DzHandle handle;
static DzHandle handles[ MAX_TEST_TIMER ];
static __int64 startTime = 0;
static __int64 timerInterval[ MAX_TEST_TIMER ];

__int64 CurrentTime()
{
    struct timeb t;
    __int64 ret;

    ftime( &t );
    ret = t.time;
    ret *= 1000;
    ret += t.millitm;
    return ret;
}

void __stdcall TimerRountine( void *context )
{
    int idx = (int)context;
    __int64 deta = timerInterval[ idx ] - CurrentTime();
    if( deta > 10 || deta < -10 ){
        printf( "bad timer!! i: %d, deta: %d\r\n", idx, (int)deta );
    }else{
        printf( "timer ok: i: %d.\r\n", idx );
    }
}

void __stdcall TestCallbackTimer( void *context )
{
    int count = (int)context;
    while( 1 ){
        for( int i = 0; i < count; i++ ){
            int random = rand();
            DzCloseCallbackTimer( handles[i] );
            handles[i] = DzCreateCallbackTimer( random, 1, TimerRountine, (void*)i );
            timerInterval[i] = CurrentTime() + random;
            DzSleep( 5000 );
        }
    }
}

void __stdcall StartTestCallbackTimer( void *context )
{
    int count = (int)context;
    for( int i = 0; i < count-3; i++ ){
        startTime = CurrentTime();
        handles[i] = DzCreateCallbackTimer( (i+1) * 3000, 1, TimerRountine, (void*)i );
        timerInterval[i] = startTime + (i+1) * 3000;

        i++;

        handles[i] = DzCreateCallbackTimer( (i+1) * 2000, 1, TimerRountine, (void*)i );
        timerInterval[i] = startTime + (i+1) * 2000;

        i++;

        handles[i] = DzCreateCallbackTimer( (i+1) * 3000, 1, TimerRountine, (void*)i );
        timerInterval[i] = startTime + (i+1) * 3000;
    }
    DzStartCot( TestCallbackTimer, (void*)count );
}
