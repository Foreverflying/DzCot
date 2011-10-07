/********************************************************************
    created:    2010/02/11 22:05
    file:       DzStructsIoLnx.h
    author:     Foreverflying
    purpose:
*********************************************************************/

#ifndef __DzStructsIoLnx_h__
#define __DzStructsIoLnx_h__

#include "../DzStructs.h"

struct _DzAsynIo
{
    union{
        DzLItr      lItr;
        struct{
            short   sign;
            short   ref;
        };
    };
    DzFastEvt       inEvt;
    DzFastEvt       outEvt;
};

#endif // __DzStructsIoLnx_h__
