/**
 *  @file       DzStructsIoOs.h
 *  @brief      for windows
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzStructsIoOs_h__
#define __DzStructsIoOs_h__

#include "../DzStructs.h"
#include "DzStructsArch.h"

struct _DzFd
{
    union {
        DzLItr      lItr;
        HANDLE      fd;
        SOCKET      s;
    };
    intptr_t        fdData;
    int             ref;
    int             err;
    BOOL            notSock;
    BOOL            isFile;
    socklen_t       addrLen;
    void*           unused3;
};

struct _DzIoHelper
{
    OVERLAPPED      overlapped;
    DzEasyEvt       easyEvt;
};

typedef struct _DzIoHelper DzIoHelper;

#endif // __DzStructsIoOs_h__
