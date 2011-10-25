/********************************************************************
    created:    2010/02/11 22:06
    file:       DzStructsLnx.h
    author:     Foreverflying
    purpose:
*********************************************************************/

#ifndef __DzStructsLnx_h__
#define __DzStructsLnx_h__

#include "../DzInc.h"
#include "../DzDeclareStructs.h"
#include "../DzStructsDebug.h"
#include "../DzStructsList.h"

struct _DzOsStruct
{
    int                         epollFd;
    int                         maxFdCount;
    int                         pipe[2];
    DzAsyncIo**                 fdTable;
    DzAsyncIo*                  pipeAsyncIo;
    struct epoll_event*         evtList;
};

struct _DzCot
{
    union{
        struct{
            DzLItr              lItr;
            void*               sp;
            char*               stack;
            int                 sSize;
            int                 priority;
            //used for remote cot
            DzRoutine           entry;
            union{
                DzEasyEvt*      easyEvt;
                DzSynObj*       evt;
            };
            int                 hostId;
            short               feedType;
            short               evtType;
        };
        DzCacheChunk            _padding;
    };

    __DBG_DATA_DEFINE( DzCot )
};

#endif // __DzStructsLnx_h__
