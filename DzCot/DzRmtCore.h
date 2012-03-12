/********************************************************************
    created:    2011/10/13 14:45
    file:       DzRmtCore.h
    author:     Foreverflying
    purpose:    
********************************************************************/

#ifndef __DzRmtCore_h__
#define __DzRmtCore_h__

#include "DzStructs.h"
#include "DzResourceMgr.h"
#include "DzBase.h"
#include "DzSynObj.h"

#ifdef __cplusplus
extern "C"{
#endif

DzCot* CreateWaitFifoCot( DzHost* host );
void __stdcall MainHostFirstEntry( intptr_t context );
void __stdcall RemoteCotEntry( intptr_t context );
void __stdcall DealLazyResEntry( intptr_t context );
void __stdcall WorkerMain( intptr_t context );

inline void NotifyRmtFifo( DzHost* rmtHost, int volatile* writePos, int nowPos )
{
    int nowCheck;

    if( nowPos == RMT_CALL_FIFO_SIZE - 1 ){
        nowPos = 1;
    }else{
        nowPos++;
    }
    AtomSetInt( writePos, nowPos );
    nowCheck = AtomIncInt( rmtHost->rmtCheckSignPtr );
    if( nowCheck == RMT_CHECK_SLEEP_SIGN ){
        AtomOrInt( &rmtHost->mgr->exitSign, rmtHost->hostMask );
        AwakeRemoteHost( rmtHost );
    }
}

inline BOOL SendRmtCot(
    DzHost*         host,
    int             rmtId,
    BOOL            emergency,
    DzCot*          cot
    )
{
    int empty;
    DzHost* rmtHost;
    DzRmtCotFifo* fifo;
    DzCot* waitFifoCot;
    int writePos;

    rmtHost = host->mgr->hostArr[ rmtId ];
    fifo = rmtHost->rmtFifoArr + host->hostId;
    writePos = AtomReadInt( fifo->writePos );
    if( emergency ){
        fifo->rmtCotArr[ writePos ] = cot;
        NotifyRmtFifo( rmtHost, fifo->writePos, writePos );
        return TRUE;
    }else if( fifo->rmtCotArr[0] ){
        AddLItrToNonEptTail( &host->pendRmtCot[ rmtId ], &cot->lItr );
        return FALSE;
    }
    empty = AtomReadInt( fifo->readPos ) - writePos;
    if( empty <= 0 ){
        empty += RMT_CALL_FIFO_SIZE - 1;
    }
    if( empty > 2 ){
        fifo->rmtCotArr[ writePos ] = cot;
        NotifyRmtFifo( rmtHost, fifo->writePos, writePos );
        return TRUE;
    }else{
        waitFifoCot = CreateWaitFifoCot( host );
        fifo->rmtCotArr[ writePos ] = waitFifoCot;
        fifo->rmtCotArr[0] = (DzCot*)1;
        AddLItrToEptSList( &host->pendRmtCot[ rmtId ], &cot->lItr );
        NotifyRmtFifo( rmtHost, fifo->writePos, writePos );
        return FALSE;
    }
}

inline void StartLazyTimer( DzHost* host )
{
    host->lazyTimer = CreateCallbackTimer(
        host, LAZY_TIMER_INTERVAL, 1,
        DealLazyResEntry, 0, CP_FIRST, SS_FIRST
        );
    host->cotCount--;
}

inline void StopLazyTimer( DzHost* host )
{
    host->cotCount++;
    CloseCallbackTimer( host, host->lazyTimer );
    host->lazyTimer = NULL;
}

#ifdef __cplusplus
};
#endif

#endif // __DzRmtCore_h__
