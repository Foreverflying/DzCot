/**
 *  @file       DzStructsOs.h
 *  @brief      for linux
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzStructsOs_h__
#define __DzStructsOs_h__

#include "../DzInc.h"
#include "../DzDeclareStructs.h"
#include "../DzStructsDebug.h"
#include "../DzStructsList.h"
#include "DzStructsArch.h"

struct _DzOsStruct
{
    int                         epollFd;
    int                         maxFdCount;
    int                         pipe[2];
    DzFd*                       pipeFd;
    struct epoll_event*         evtList;
};

struct _DzOsReadOnly
{
    int                         _unusedHolder;
};

struct _DzCot
{
    union {
        struct {
            DzLItr              lItr;
            void*               sp;
            char*               stack;
            int                 sType;
            int                 priority;
            union {
                DzEntry         entry;
                intptr_t        cotData;
            };
            union {
                DzEasyEvt*      easyEvt;
                DzSynObj*       evt;
            };
            // used for remote cot
            int                 hostId;
            short               feedType;
            short               evtType;
        };
        DzCacheChunk            _padding;
    };

    __DBG_STRUCT(DzCot)
};

struct _DzSysAutoEvt
{
    sem_t               sem;
};

#endif // __DzStructsOs_h__
