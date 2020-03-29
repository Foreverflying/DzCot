/**
 *  @file       DzCore.h
 *  @brief      
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzCore_h__
#define __DzCore_h__

#include "DzStructs.h"
#include "DzBase.h"
#include "DzResourceMgr.h"
#include "DzSchedule.h"
#include "DzRmtCore.h"
#include "DzCoreOs.h"
#include "thirdparty/dlmalloc.h"

void __stdcall DelayFreeCotHelper(intptr_t context);
void __stdcall EventNotifyCotEntry(intptr_t context);
void ReleaseAllPoolStack(DzHost* host);
int RunHost(
    DzHostsMgr* hostMgr,
    int         hostId,
    DzEntry     firstEntry,
    intptr_t    context,
    DzEntry     cleanEntry
    );

int RunHosts(
    int         hostCount,
    int         smallStackSize,
    int         middleStackSize,
    int         largeStackSize,
    int         lowestPri,
    int         dftPri,
    int         dftSType,
    DzEntry     firstEntry,
    intptr_t    context,
    DzEntry     cleanEntry
    );

static inline
DzCot* AllocDzCot(DzHost* host, int sType)
{
    DzCot* dzCot;

    if (host->cotPools[sType]) {
        dzCot = MEMBER_BASE(host->cotPools[sType], DzCot, lItr);
        host->cotPools[sType] = host->cotPools[sType]->next;
        host->cotPoolNowDepth[sType] ++;
    } else {
        if (!host->cotPool) {
            if (!AllocDzCotPool(host)) {
                return NULL;
            }
        }
        dzCot = MEMBER_BASE(host->cotPool, DzCot, lItr);
        if (InitCot(host, dzCot, sType)) {
            host->cotPool = host->cotPool->next;
        } else {
            return NULL;
        }
    }
    return dzCot;
}

static inline
void FreeDzCot(DzHost* host, DzCot* dzCot)
{
    DzCot* tmp;

    __Dbg(InitDzCot)(host, dzCot);
    if (host->cotPoolNowDepth[dzCot->sType] > 0) {
        dzCot->lItr.next = host->cotPools[dzCot->sType];
        host->cotPools[dzCot->sType] = &dzCot->lItr;
        host->cotPoolNowDepth[dzCot->sType] --;
    } else {
        // can not FreeCotStack(dzCot) here!!
        // the stack is still in use before switch
        if (host->cotPools[dzCot->sType]) {
            // if the cotPool is not empty swap and free the head
            tmp = MEMBER_BASE(host->cotPools[dzCot->sType], DzCot, lItr);
            dzCot->lItr.next = tmp->lItr.next;
            host->cotPools[dzCot->sType] = &dzCot->lItr;
            FreeCotStack(host, tmp);
            tmp->lItr.next = host->cotPool;
            host->cotPool = &tmp->lItr;
        } else {
            // switch to a helper cot to free it
            StartCot(host, DelayFreeCotHelper, (intptr_t)dzCot, host->currPri, ST_FIRST);
        }
    }
}

// StartCot:
// create a new cot
static inline
int StartCot(
    DzHost*     host,
    DzEntry     entry,
    intptr_t    context,
    int         priority,
    int         sType
    )
{
    DzCot* dzCot;

    dzCot = AllocDzCot(host, sType);
    if (!dzCot) {
        return DS_NO_MEMORY;
    }
    dzCot->priority = priority;
    if (priority < host->currPri) {
        host->currPri = priority;
    }
    host->cotCount++;
    SetCotEntry(dzCot, entry, context);
    DispatchCot(host, dzCot);
    return DS_OK;
}

// StartCotInstant:
// create a new cot and run it at once
// current cot is paused and will continue at next schedule
static inline
int StartCotInstant(
    DzHost*     host,
    DzEntry     entry,
    intptr_t    context,
    int         priority,
    int         sType
    )
{
    DzCot* dzCot;

    dzCot = AllocDzCot(host, sType);
    if (!dzCot) {
        return DS_NO_MEMORY;
    }
    dzCot->priority = priority;
    host->cotCount++;
    host->scheduleCd++;
    SetCotEntry(dzCot, entry, context);
    PushCotToTop(host, host->currCot);
    SwitchToCot(host, dzCot);
    return DS_OK;
}

// EvtStartCot
// equal to StartCot, notify an manual event SynObj when cot finished
static inline
int EvtStartCot(
    DzHost*     host,
    DzSynObj*   evt,
    DzEntry     entry,
    intptr_t    context,
    int         priority,
    int         sType
    )
{
    DzCot* dzCot;

    dzCot = AllocDzCot(host, sType);
    if (!dzCot) {
        return DS_NO_MEMORY;
    }
    dzCot->priority = priority;
    if (priority < host->currPri) {
        host->currPri = priority;
    }
    host->cotCount++;
    SetCotEntry(dzCot, EventNotifyCotEntry, context);
    dzCot->entry = entry;
    dzCot->evt = CloneSynObj(evt);
    DispatchCot(host, dzCot);
    return DS_OK;
}

// EvtStartCotInstant:
// equal to StartCotInstant, notify an manual event SynObj when cot finished
static inline
int EvtStartCotInstant(
    DzHost*     host,
    DzSynObj*   evt,
    DzEntry     entry,
    intptr_t    context,
    int         priority,
    int         sType
    )
{
    DzCot* dzCot;

    dzCot = AllocDzCot(host, sType);
    if (!dzCot) {
        return DS_NO_MEMORY;
    }
    dzCot->priority = priority;
    host->cotCount++;
    host->scheduleCd++;
    SetCotEntry(dzCot, EventNotifyCotEntry, context);
    dzCot->entry = entry;
    dzCot->evt = CloneSynObj(evt);
    PushCotToTop(host, host->currCot);
    SwitchToCot(host, dzCot);
    return DS_OK;
}

static inline
int StartRemoteCot(
    DzHost*     host,
    int         rmtId,
    DzEntry     entry,
    intptr_t    context,
    int         priority,
    int         sType
    )
{
    DzCot* dzCot;

    dzCot = AllocDzCot(host, sType);
    if (!dzCot) {
        return DS_NO_MEMORY;
    }
    dzCot->hostId = host->hostId;
    dzCot->entry = entry;
    dzCot->feedType = 0;
    dzCot->priority = priority;
    SetCotEntry(dzCot, RemoteCotEntry, context);
    SendRmtCot(host, rmtId, FALSE, dzCot);
    return DS_OK;
}

static inline
int EvtStartRemoteCot(
    DzHost*     host,
    DzSynObj*   evt,
    int         rmtId,
    DzEntry     entry,
    intptr_t    context,
    int         priority,
    int         sType
    )
{
    DzCot* dzCot;

    dzCot = AllocDzCot(host, sType);
    if (!dzCot) {
        return DS_NO_MEMORY;
    }
    dzCot->hostId = host->hostId;
    dzCot->entry = entry;
    dzCot->feedType = 1;
    dzCot->evtType = 0;
    dzCot->evt = CloneSynObj(evt);
    dzCot->priority = priority;
    SetCotEntry(dzCot, RemoteCotEntry, context);
    SendRmtCot(host, rmtId, FALSE, dzCot);
    return DS_OK;
}

static inline
int RunRemoteCot(
    DzHost*     host,
    int         rmtId,
    DzEntry     entry,
    intptr_t    context,
    int         priority,
    int         sType
    )
{
    DzCot* dzCot;
    DzEasyEvt easyEvt;

    dzCot = AllocDzCot(host, sType);
    if (!dzCot) {
        return DS_NO_MEMORY;
    }
    dzCot->hostId = host->hostId;
    dzCot->entry = entry;
    dzCot->feedType = 1;
    dzCot->evtType = 1;
    dzCot->easyEvt = &easyEvt;
    dzCot->priority = priority;
    SetCotEntry(dzCot, RemoteCotEntry, context);
    SendRmtCot(host, rmtId, FALSE, dzCot);
    WaitEasyEvt(host, &easyEvt);
    return DS_OK;
}

static inline
int RunWorker(DzHost* host, DzEntry entry, intptr_t context)
{
    DzLItr* lItr;
    DzSysParam param;
    DzWorker* worker;

    lItr = AtomPopStack(&host->mgr->workerPool);
    if (lItr) {
        AtomIncInt(&host->mgr->workerNowDepth);
        worker = MEMBER_BASE(lItr, DzWorker, lItr);
        worker->entry = entry;
        worker->context = context;
        worker->dzCot = host->currCot;
        worker->dzCot->hostId = host->hostId;
        NotifySysAutoEvt(&worker->sysEvt);
    } else {
        param.threadEntry = WorkerMain;
        param.wk.entry = entry;
        param.wk.context = context;
        param.wk.dzCot = host->currCot;
        param.wk.dzCot->hostId = host->hostId;
        param.wk.hostMgr = host->mgr;
        StartSystemThread(&param, WORKER_STACK_SIZE);
    }
    Schedule(host);
    host->cotCount--;
    return DS_OK;
}

static inline
intptr_t GetCotData(DzHost* host)
{
    return host->currCot->cotData;
}

static inline
void SetCotData(DzHost* host, intptr_t data)
{
    host->currCot->cotData = data;
}

static inline
int GetCotCount(DzHost* host)
{
    return host->cotCount;
}

static inline
int GetHostId(DzHost* host)
{
    return host->hostId;
}

static inline
int SetCurrCotPriority(DzHost* host, int priority)
{
    int ret;

    ret = host->currCot->priority;
    if (priority >= CP_FIRST) {
        host->currCot->priority = priority;
        if (priority < ret) {
            host->currPri = priority;
        }
    }
    return ret;
}

static inline
int SetCotPoolDepth(DzHost* host, int sType, int depth)
{
    int deta;
    int ret;

    ret = host->cotPoolSetDepth[sType];
    if (depth >= 0 && host->cotStackSize[sType] >= DZ_MIN_PAGE_STACK_SIZE) {
        deta = depth - host->cotPoolSetDepth[sType];
        host->cotPoolNowDepth[sType] += deta;
        host->cotPoolSetDepth[sType] = depth;
    }
    return ret;
}

static inline
int SetWorkerPoolDepth(DzHost* host, int depth)
{
    int setDepth;
    int nowSet;

    setDepth = AtomGetInt(&host->mgr->workerSetDepth);
    do {
        nowSet = setDepth;
        setDepth = AtomCasInt(&host->mgr->workerSetDepth, setDepth, depth);
    } while (setDepth != nowSet);
    AtomAddInt(&host->mgr->workerNowDepth, depth - setDepth);
    return setDepth;
}

static inline
int SetHostParam(
    DzHost*     host,
    int         lowestPri,
    int         dftPri,
    int         dftSType
    )
{
    int ret;

    ret = host->lowestPri;
    if (lowestPri > host->lowestPri) {
        host->lowestPri = lowestPri;
    }
    if (dftPri >= CP_FIRST) {
        host->dftPri = dftPri;
    }
    if (dftSType >= ST_FIRST) {
        host->dftSType = dftSType;
        if (host->cotPoolSetDepth[dftSType] < DFT_SSIZE_POOL_DEPTH) {
            SetCotPoolDepth(host, dftSType, DFT_SSIZE_POOL_DEPTH);
        }
    }
    return ret;
}

static inline
int SetHostIoReaction(DzHost* host, int rate)
{
    int ret = host->ioReactionRate;
    if (rate > 0) {
        host->ioReactionRate = rate;
    }
    return ret;
}

static inline
void* Malloc(DzHost* host, size_t size)
{
    void* ret;

    ret = mspace_malloc(host->mSpace, size);
    __Dbg(AllocHeap)(host, ret, size);
    return ret;
}

static inline
void Free(DzHost* host, void* mem)
{
    __Dbg(FreeHeap)(host, mem);
    mspace_free(host->mSpace, mem);
}

static inline
void* MallocEx(DzHost* host, size_t size)
{
    DzMemExTag* ret;

    ret = (DzMemExTag*)Malloc(host, size + sizeof(DzMemExTag));
    ret->hostId = host->hostId;
    return ret + 1;
}

static inline
void FreeEx(DzHost* host, void* mem)
{
    DzLNode* node;
    DzMemExTag* base;

    base = ((DzMemExTag*)mem) - 1;
    if (base->hostId == host->hostId) {
        mspace_free(host->mSpace, base);
    } else {
        node = AllocLNode(host);
        node->d1 = (intptr_t)base;
        AddLItrToTail(host->lazyFreeMem + base->hostId, &node->lItr);
        if (!host->lazyTimer) {
            StartLazyTimer(host);
        }
    }
}

static inline
int DispatchMinTimers(DzHost* host)
{
    DzTimerNode* timerNode;
    int64_t currTime;
    int64_t cmpTime;
    BOOL ret;

    while (host->timerCount > 0) {
        ret = FALSE;
        currTime = MilUnixTime(host);
        cmpTime = currTime + MIN_TIME_INTERVAL;
        while (GetMinTimerNode(host)->timestamp <= cmpTime) {
            timerNode = GetMinTimerNode(host);
            if (timerNode->repeat) {
                // for timerNode->interval is a negative value, use minus
                timerNode->timestamp -= timerNode->interval;
                AdjustMinTimer(host, timerNode);
            } else {
                RemoveMinTimer(host);
            }
            ret = NotifyTimerNode(host, timerNode) || ret;
            if (host->timerCount == 0) {
                return ret ? 0 : -1;
            }
        }
        if (ret) {
            return 0;
        } else {
            return (int)(GetMinTimerNode(host)->timestamp - currTime);
        }
    }
    return -1;
}

static inline
void DealRmtFifo(DzHost* host, DzRmtCotFifo* fifo)
{
    int readPos;
    int writePos;
    DzCot* dzCot;
    DzLItr* nextItr;

    readPos = AtomGetInt(fifo->readPos);
    writePos = AtomGetInt(fifo->writePos);
    while (readPos != writePos) {
        dzCot = fifo->rmtCotArr[readPos];
        readPos++;
        if (readPos == RMT_CALL_FIFO_SIZE) {
            readPos = 0;
        }
        AtomSetInt(fifo->readPos, readPos);
        if (dzCot->priority >= 0) {
            host->cotCount++;
            if (dzCot->priority > host->lowestPri) {
                dzCot->priority = host->lowestPri;
            }
            DispatchCot(host, dzCot);
        } else {
            dzCot->priority += CP_DEFAULT + 1;
            nextItr = &dzCot->lItr;
            do {
                dzCot = MEMBER_BASE(nextItr, DzCot, lItr);
                nextItr = dzCot->lItr.next;
                host->cotCount++;
                if (dzCot->priority > host->lowestPri) {
                    dzCot->priority = host->lowestPri;
                }
                DispatchCot(host, dzCot);
            } while (nextItr);
        }
    }
}

static inline
int DispatchRmtCots(DzHost* host, int timeout)
{
    int idx;
    u_int sign;

    if (timeout) {
        sign = (u_int)AtomCasInt(host->rmtCheckSignPtr, RMT_CHECK_AWAKE_SIGN, 0);
        if (sign == RMT_CHECK_AWAKE_SIGN) {
            return timeout;
        }
    }
    
    idx = 0;
    sign = (u_int)AtomAndInt(host->rmtCheckSignPtr, RMT_CHECK_AWAKE_SIGN);
    sign &= ~RMT_CHECK_AWAKE_SIGN;
    while (sign) {
        if (sign & 1) {
            DealRmtFifo(host, host->rmtFifoArr + idx);
        }
        sign >>= 1;
        idx++;
    }
    return 0;
}

#endif // __DzCore_h__
