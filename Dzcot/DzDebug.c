/********************************************************************
    created:    2011/10/06 20:58
    file:       DzDebug.c
    author:     Foreverflying
    purpose:    
*********************************************************************/

#include "DzDebug.h"
#include "DzStructs.h"

#ifdef __DBG_DEBUG_CHECK_MODE

int __DbgGetLastErr( DzHost* host )
{
    return host->currThread->lastErr;
}

void __DbgSetLastErr( DzHost* host, int err )
{
    host->currThread->lastErr = err;
}

void __DbgCheckCotStackOverflow( DzThread* dzThread )
{
}

#endif