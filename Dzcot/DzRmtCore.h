/********************************************************************
    created:    2011/10/13 14:45
    file:       DzRmtCore.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzRmtCore_h__
#define __DzRmtCore_h__

#include "DzStructs.h"
#include "DzResourceMgr.h"
#include "DzBaseOs.h"

#ifdef __cplusplus
extern "C"{
#endif

DzCot* CreateWaitFifoCot( DzHost* host );
void __stdcall MainHostEntry( intptr_t context );
void __stdcall RemoteCotEntry( intptr_t context );

inline void NotifyRmtFifo( DzHost* rmtHost, int* writePos )
{
    int nowCheck;

    if( *writePos == RMT_CALL_FIFO_SIZE - 1 ){
        *writePos = 0;
    }else{
        ( *writePos )++;
    }
    nowCheck = AtomIncInt( &rmtHost->checkRmtSign );
    if( nowCheck == RMT_CHECK_SLEEP_SIGN ){
        AtomOrInt( &rmtHost->hostMgr->exitSign, rmtHost->hostMask );
        AwakeRemoteHost( rmtHost );
    }
}

inline void DispatchRmtCot(
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

    rmtHost = host->hostMgr->hostArr[ rmtId ];
    fifo = rmtHost->rmtFifoArr + host->hostId;
    if( emergency ){
        fifo->rmtCotArr[ fifo->writePos ] = cot;
        NotifyRmtFifo( rmtHost, &fifo->writePos );
        return;
    }else if( host->pendRmtCot[ rmtId ].tail ){
        AddLItrToTail( &host->pendRmtCot[ rmtId ], &cot->lItr );
        return;
    }
    empty = AtomReadInt( &fifo->readPos ) - fifo->writePos;
    if( empty <= 0 ){
        empty += RMT_CALL_FIFO_SIZE;
    }
    if( empty > 2 ){
        fifo->rmtCotArr[ fifo->writePos ] = cot;
    }else{
        waitFifoCot = CreateWaitFifoCot( host );
        fifo->rmtCotArr[ fifo->writePos ] = waitFifoCot;
        AddLItrToTail( &host->pendRmtCot[ rmtId ], &cot->lItr );
    }
    NotifyRmtFifo( rmtHost, &fifo->writePos );
}

#ifdef __cplusplus
};
#endif

#endif // __DzRmtCore_h__
