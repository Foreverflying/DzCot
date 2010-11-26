/********************************************************************
    created:    2010/02/11 22:06
    file:       DzStructsWin.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzStructsWin_h__
#define __DzStructsWin_h__

#include "../DzStructsList.h"

struct _DzSynObj;

typedef struct _DzOsStruct
{
    HANDLE          iocp;
    void*           originExceptPtr;
    char*           originalStack;
    DzLItr*         reservedStack;
}DzOsStruct;

typedef struct _DzThread
{
    DzLItr              lItr;
    void*               sp;
    char*               stack;
    char*               stackLimit;
    struct _DzSynObj*   finishEvent;
    int                 stackSize;
    int                 priority;
}DzThread;

#endif // __DzStructsWin_h__
