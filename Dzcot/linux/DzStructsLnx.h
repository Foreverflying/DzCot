/********************************************************************
    created:    2010/02/11 22:06
    file:       DzStructsLnx.h
    author:     Foreverflying
    purpose:
*********************************************************************/

#ifndef __DzStructsLnx_h__
#define __DzStructsLnx_h__

#include "../DzStructsList.h"

struct _DzOsStruct
{
    int             epollFd;
    int             maxFd;
    DzAsynIo**      fdTable;
    DzLItr*         asynIoPool;
};

struct _DzThread
{
    DzLItr          lItr;
    void*           sp;
    char*           stack;
    int             sSize;
    int             lastErr;
    int             priority;
};

#endif // __DzStructsLnx_h__
