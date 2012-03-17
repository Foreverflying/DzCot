/********************************************************************
    created:    2010/02/11 22:05
    file:       DzStructsIoWin.h
    author:     Foreverflying
    purpose:    
********************************************************************/

#ifndef __DzStructsIoWin_h__
#define __DzStructsIoWin_h__

#include "../DzStructs.h"

struct _DzFd
{
    union{
        DzLItr      lItr;
        HANDLE      fd;
        SOCKET      s;
        int64       unused;
    };
    int             ref;
    int             err;
};

struct _DzIoHelper
{
    OVERLAPPED      overlapped;
    DzEasyEvt       easyEvt;
};

typedef struct _DzIoHelper DzIoHelper;

#endif // __DzStructsIoWin_h__
