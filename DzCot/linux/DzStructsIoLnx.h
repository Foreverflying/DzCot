/**
 *  @file       DzStructsIoLnx.h
 *  @brief      
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzStructsIoLnx_h__
#define __DzStructsIoLnx_h__

#include "../DzStructs.h"

struct _DzFd
{
    union{
        DzLItr      lItr;
        int         fd;
    };
    intptr_t        fdData;
    int             ref;
    int             err;
    BOOL            isSock;
    int             unused;
    DzEasyEvt       inEvt;
    DzEasyEvt       outEvt;
};

#endif // __DzStructsIoLnx_h__
