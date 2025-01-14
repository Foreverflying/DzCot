/**
 *  @file       DzRmtCore.h
 *  @brief      
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2011/10/13
 *
 */

#ifndef __DzRmtCore_h__
#define __DzRmtCore_h__

#include "DzStructs.h"
#include "DzResourceMgr.h"
#include "DzBase.h"
#include "DzSynObj.h"

DzCot* CreateWaitFifoCot(DzHost* host);
void __stdcall MainHostFirstEntry(intptr_t context);
void __stdcall RemoteCotEntry(intptr_t context);
void __stdcall DealLazyResEntry(intptr_t context);
void __stdcall WorkerMain(intptr_t context);

static inline
void NotifyRmtFifo(
    DzHostsMgr*     hostMgr,
    DzHost*         rmtHost,
    int             hostMask,
    int volatile*   writePos,
    int             nowPos
    )
{
    int nowSign;

    if (nowPos == RMT_CALL_FIFO_SIZE - 1) {
        nowPos = 0;
    } else {
        nowPos++;
    }
    AtomSetInt(writePos, nowPos);
    nowSign = AtomOrInt(rmtHost->rmtCheckSignPtr, hostMask);
    if (nowSign == 0) {
        AtomOrInt(&hostMgr->liveSign, rmtHost->hostMask);
        AwakeRemoteHost(rmtHost);
    }
}

static inline
BOOL SendRmtCot(
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

    rmtHost = host->mgr->hostArr[rmtId];
    fifo = rmtHost->rmtFifoArr + host->hostId;
    writePos = AtomGetInt(fifo->writePos);
    if (emergency) {
        fifo->rmtCotArr[writePos] = cot;
        NotifyRmtFifo(host->mgr, rmtHost, host->hostMask, fifo->writePos, writePos);
        return TRUE;
    } else if (fifo->pendRmtCot->tail) {
        AddLItrToNonEptTail(fifo->pendRmtCot, &cot->lItr);
        return FALSE;
    }
    empty = AtomGetInt(fifo->readPos) - writePos;
    if (empty <= 0) {
        empty += RMT_CALL_FIFO_SIZE;
    }
    if (empty > 2) {
        fifo->rmtCotArr[writePos] = cot;
        NotifyRmtFifo(host->mgr, rmtHost, host->hostMask, fifo->writePos, writePos);
        return TRUE;
    } else {
        waitFifoCot = CreateWaitFifoCot(host);
        fifo->rmtCotArr[writePos] = waitFifoCot;
        AddLItrToEptSList(fifo->pendRmtCot, &cot->lItr);
        NotifyRmtFifo(host->mgr, rmtHost, host->hostMask, fifo->writePos, writePos);
        return FALSE;
    }
}

static inline
void StartLazyTimer(DzHost* host)
{
    host->lazyTimer = CreateCallbackTimer(
        host, LAZY_TIMER_INTERVAL, 1,
        DealLazyResEntry, 0, CP_FIRST, ST_FIRST
        );
    host->cotCount--;
}

static inline
void StopLazyTimer(DzHost* host)
{
    host->cotCount++;
    CloseCallbackTimer(host, host->lazyTimer);
    host->lazyTimer = NULL;
}

#endif // __DzRmtCore_h__
