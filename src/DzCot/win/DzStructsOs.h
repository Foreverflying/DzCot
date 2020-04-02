/**
 *  @file       DzStructsOs.h
 *  @brief      for windows
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
    HANDLE                      iocp;
    void*                       originExceptPtr;
    // OVERLAPPED_ENTRY*           overlappedEntryList;
};

struct _DzOsReadOnly
{
    LPFN_ACCEPTEX               _AcceptEx;
    LPFN_CONNECTEX              _ConnectEx;
    LPFN_GETACCEPTEXSOCKADDRS   _GetAcceptExSockAddrs;
};

struct _DzCot
{
    union {
        struct {
            DzLItr              lItr;
            void*               sp;
            char*               stack;
            char*               stackLimit;
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
    HANDLE              event;
};

#endif // __DzStructsOs_h__
