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
        DzQItr              qItr;
        DzRoutine           callback;
    };
    struct epoll_event      epollEvt;
    int             fd;
    int             ref;
    DzFastEvt       fastEvt;
    OVERLAPPED      overlapped;
}DzAsynIo;

inline void InitAsynIo( DzAsynIo *asynIo )
{
    asynIo->epollEvt.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLET;
    asynIo->epollEvt.data = asynIo;
}

#endif // __DzStructsIoLnx_h__
