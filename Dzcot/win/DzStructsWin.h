/********************************************************************
    created:    2010/02/11 22:06
    file:       DzStructsWin.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzStructsWin_h__
#define __DzStructsWin_h__

#include "../DzStructsQueue.h"

struct _DzSynObj;

typedef struct _DzOsStruct
{
    HANDLE          iocp;
    void*           originExceptPtr;
    char*           originalStack;
    DzQItr*         reservedStack;
}DzOsStruct;

typedef struct _DzThread
{
    DzQItr              qItr;
    void*               sp;
    char*               stack;
    char*               stackLimit;
    struct _DzSynObj*   finishEvent;
    int                 stackSize;
    int                 priority;
}DzThread;

#endif // __DzStructsWin_h__
