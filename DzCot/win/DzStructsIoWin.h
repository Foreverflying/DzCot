/**
 *  @file       DzStructsIoWin.h
 *  @brief      
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzStructsIoWin_h__
#define __DzStructsIoWin_h__

#include "../DzStructs.h"

struct _DzFd
{
    union{
        DzLItr      lItr;
        HANDLE      fd;
        SOCKET      s;
    };
    intptr_t        fdData;
    int             ref;
    int             err;
    BOOL            notSock;
    BOOL            isFile;
    void*           unused2;
    void*           unused3;
};

struct _DzIoHelper
{
    OVERLAPPED      overlapped;
    DzEasyEvt       easyEvt;
};

typedef struct _DzIoHelper DzIoHelper;

#endif // __DzStructsIoWin_h__
