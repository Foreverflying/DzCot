/********************************************************************
    created:    2010/02/11 22:06
    file:       DzStructsLnx.h
    author:     Foreverflying
    purpose:
*********************************************************************/

#ifndef __DzStructsLnx_h__
#define __DzStructsLnx_h__

#include "../DzIncOs.h"
#include "../DzDeclareStructs.h"
#include "../DzStructsDebug.h"
#include "../DzStructsList.h"

struct _DzOsStruct
{
    int             epollFd;
    int             maxFdCount;
    int             pipe[2];
    DzAsyncIo**     fdTable;
    DzLItr*         asyncIoPool;
    DzAsyncIo*      pipeAsyncIo;
};

struct _DzThread
{
    DzLItr          lItr;
    void*           sp;
    char*           stack;
    int             sSize;
    int             priority;

    __DBG_DATA_DEFINE( DzThread )
};

#endif // __DzStructsLnx_h__
