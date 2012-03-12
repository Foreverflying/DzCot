/********************************************************************
    created:    2010/02/11 22:05
    file:       DzStructsIoLnx.h
    author:     Foreverflying
    purpose:
********************************************************************/

#ifndef __DzStructsIoLnx_h__
#define __DzStructsIoLnx_h__

#include "../DzStructs.h"

struct _DzFd
{
    union{
        DzLItr      lItr;
        int         fd;
    };
    int             ref;
    int             err;
    BOOL            isSock;
    DzEasyEvt       inEvt;
    DzEasyEvt       outEvt;
};

#endif // __DzStructsIoLnx_h__
