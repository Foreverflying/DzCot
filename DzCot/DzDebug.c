/********************************************************************
    created:    2011/10/06 20:58
    file:       DzDebug.c
    author:     Foreverflying
    purpose:    
*********************************************************************/

#include "DzDebug.h"
#include "DzStructsDebug.h"
#include "DzStructs.h"

#ifdef __DBG_DEBUG_CHECK_MODE

int __DbgGetLastErr( DzHost* host )
{
    return __DBG_DATA( host->currCot ).lastErr;
}

void __DbgSetLastErr( DzHost* host, int err )
{
    __DBG_DATA( host->currCot ).lastErr = err;
}

void __DbgCheckCotStackOverflow( DzCot* dzCot )
{
}

#endif