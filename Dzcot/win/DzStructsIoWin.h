/********************************************************************
    created:    2010/02/11 22:05
    file:       DzStructsIoWin.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzStructsIoWin_h__
#define __DzStructsIoWin_h__

#include "../DzStructs.h"

struct _DzAsynIo
{
    OVERLAPPED      overlapped;
    DzFastEvt       fastEvt;
};

#endif // __DzStructsIoWin_h__
