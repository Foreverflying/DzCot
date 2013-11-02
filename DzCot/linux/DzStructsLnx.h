/**
 *  @file       DzStructsLnx.h
 *  @brief      
 *  @author	    Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

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
    DzFd*                       pipeFd;
    struct epoll_event*         evtList;
};

struct _DzCot
{
    union{
        struct{
            DzLItr              lItr;
            void*               sp;
            char*               stack;
            int                 sType;
            int                 priority;
            //used for remote cot
            DzEntry             entry;
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

    __DBG_STRUCT( DzCot )
};

struct _DzSysAutoEvt
{
    sem_t               sem;
};

#endif // __DzStructsLnx_h__
