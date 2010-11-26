/********************************************************************
    created:    2010/02/11 22:06
    file:       DzStructsLnx.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzStructsLnx_h__
#define __DzStructsLnx_h__

#include "../DzStructsQueue.h"

struct _DzSynObj;
struct _DzAsynIo;

typedef struct _DzOsStruct
{
    int                 epollFd;
    int                 maxFd;
    struct _DzAsynIo**  fdTable;
}DzOsStruct;

typedef struct _DzThread
{
    DzQItr              qItr;
    void*               sp;
    char*               stack;
    struct _DzSynObj*   finishEvent;
    int                 stackSize;
    int                 priority;
}DzThread;

#endif // __DzStructsLnx_h__
