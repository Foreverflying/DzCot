#include "StdAfx.h"
#include "TestCallbackTimer.h"
#include "../Dzcot/Dzcot.h"

static int count = 0;
static DzHandle handle;

int __stdcall TestCallbackTimer( void *context )
{
    count++;
    printf( "called %d !\r\n", count );
    if( count == 5 ){
        DzStopCallbackTimer( handle );
    }
    return 0;
}

int __stdcall StartTestCallbackTimer( void *context )
{
    handle = DzCreateCallbackTimer( TestCallbackTimer );
    DzStartCallbackTimer( handle, 3000, (int)context, 0 );
    return 0;
}
