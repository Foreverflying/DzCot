/********************************************************************
    created:    2010/11/17 3:19
    file:       DzStructsIoLnx.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzStructsIoLnx_h__
#define __DzStructsIoLnx_h__

#include "../DzStructs.h"

typedef struct _DzAsynIo
{
    union{
        DzQItr      qItr;
        DzRoutine   callback;
    };
    void*           context;
    int             fd;
    int             ref;
    DzFastEvt       fastEvt;
    OVERLAPPED      overlapped;
}DzAsynIo;

#endif // __DzStructsIoLnx_h__
