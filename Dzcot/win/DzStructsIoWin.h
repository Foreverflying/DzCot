/********************************************************************
    created:    2010/02/11 22:05
    file:       DzStructsIoWin.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzStructsIoWin_h__
#define __DzStructsIoWin_h__

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

#endif // __DzStructsIoWin_h__
