
#ifndef _DZ_STRUCTS_WIN_H_
#define _DZ_STRUCTS_WIN_H_

#include "../DzStructsQueue.h"

typedef struct _DzIoMgr
{
    union{
        BOOL        inited;
        HANDLE      iocp;
    };
}DzIoMgr;

#endif  //#ifdef _DZ_STRUCTS_WIN_H_