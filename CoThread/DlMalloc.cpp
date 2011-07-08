#include "stdafx.h"
#include "malloc.h"
#include "../Dzcot/Dzcot.h"

int mallocCount = 0;

void* Malloc( size_t size )
{
    mallocCount++;
    return dlmalloc( size );
}

void Free( void *buff )
{
    mallocCount--;
    return dlfree( buff );
}

void __stdcall TraceMalloc( void *context )
{
    int traceCount = 0;
    while( 1 ){
        traceCount++;
        printf( "mallocCount : %d, traceCount: %d\r\n", mallocCount, traceCount );
        DzSleep( 3000 );
    }
}
