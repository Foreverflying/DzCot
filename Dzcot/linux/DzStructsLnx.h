
#ifndef _DZ_STRUCTS_WIN_H_
#define _DZ_STRUCTS_WIN_H_

#include "../DzStructsQueue.h"

typedef struct _DzIoMgr
{
    HANDLE      iocp;
}DzIoMgr;

typedef struct _DzOsAppend
{
    char*           originalStack;
    DzQItr*         reservedStack;
}DzOsAppend;

#endif  //#ifdef _DZ_STRUCTS_WIN_H_