/**
 *  @file       DzStructsIoOs.h
 *  @brief      for linux
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzStructsIoOs_h__
#define __DzStructsIoOs_h__

#include "../DzStructs.h"

struct _DzFd
{
    union {
        DzLItr      lItr;
        int         fd;
    };
    intptr_t        fdData;
    int             ref;
    int             err;
    BOOL            notSock;
    int             unused;
    DzEasyEvt       inEvt;
    DzEasyEvt       outEvt;
};

#endif // __DzStructsIoOs_h__
