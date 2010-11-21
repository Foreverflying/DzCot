/********************************************************************
    created:    2010/02/11 22:06
    file:       DzStructsWin.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzStructsWin_h__
#define __DzStructsWin_h__

#include "../DzStructsQueue.h"

typedef struct _DzOsAppend
{
    HANDLE          iocp;
    char*           originalStack;
    DzQItr*         reservedStack;
}DzOsStruct;

#endif // __DzStructsWin_h__
