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

void __stdcall MainHostEntry( intptr_t context );
void __stdcall SendFifoWritableEntry( intptr_t context );
void __stdcall FeedRmtCallEntry( intptr_t context );

inline void AwakeRemoteHostCheck( DzHost* rmtHost )
{
    AtomOrInt( &rmtHost->hostsMgr->exitSign, 1 << rmtHost->hostId );
}

inline void CopyRmtCallPkg( DzRmtCallPkg* src, DzRmtCallPkg* dst )
{
    dst->params = src->params;
    dst->entry = src->entry;
    dst->context = src->context;
    dst->evt = src->evt;
}

inline void NotifyRmtCall( DzHost* rmtHost, int* writePos, int addVal )
{
    int nowCheck;
    int newPos;

    newPos = *writePos;
    newPos += addVal;
    if( newPos >= RMT_CALL_FIFO_SIZE ){
        newPos -= RMT_CALL_FIFO_SIZE;
    }
    *writePos = newPos;
    nowCheck = AtomAddInt( rmtHost->checkSignPtr, addVal );
    if( nowCheck == RMT_CHECK_SLEEP_SIGN ){
        AwakeRemoteHostCheck( rmtHost );
    }
}

inline DzRmtCallPkg* PendingRmtCall( DzHost* host, int rmtId )
{
    DzLNode* node;

    node = AllocLNode( host );
    node->content = (intptr_t)AllocLNode( host );
    AddLItrToTail( &host->pendingPkgs[ rmtId ], &node->lItr );
    return (DzRmtCallPkg*)node->content;
}

inline void SendRmtCall(
    DzHost*         host,
    DzHost*         rmtHost,
    BOOL            emergency,
    DzRmtCallPkg*   srcPkg
    )
{
    int empty;
    int addVal;
    int writePos;
    DzRmtCallFifo* fifo;
    DzRmtCallPkg* pkg;

    addVal = 0;
    if( host->pendingPkgs[ rmtHost->hostId ].tail ){
        pkg = PendingRmtCall( host, rmtHost->hostId );
    }else{
        fifo = rmtHost->rmtFifoArr + host->hostId;
        empty = fifo->readPos - fifo->writePos;
        if( empty <= 0 ){
            empty += RMT_CALL_FIFO_SIZE;
        }
        writePos = fifo->writePos;
        pkg = fifo->callPkgArr + writePos;
        if( empty == 2 ){
            pkg->evt = 0;
            pkg->priority = CP_FIRST;
            pkg->sSize = SS_FIRST;
            pkg->entry = SendFifoWritableEntry;
            pkg->context = host->hostId;
            addVal++;
            if( !emergency ){
                pkg = PendingRmtCall( host, rmtHost->hostId );
            }else{
                writePos++;
                if( writePos == RMT_CALL_FIFO_SIZE ){
                    writePos = 0;
                }
                pkg = fifo->callPkgArr + writePos;
                addVal++;
            }
        }
    }
    CopyRmtCallPkg( srcPkg, pkg );
    if( addVal ){
        NotifyRmtCall( rmtHost, &fifo->writePos, addVal );
    }
}

#ifdef __cplusplus
};
#endif

#endif // __DzRmtCore_h__
