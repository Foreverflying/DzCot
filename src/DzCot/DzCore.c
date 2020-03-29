/**
 *  @file       DzCore.c
 *  @brief      
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#include "DzInc.h"
#include "DzCore.h"
#include "DzIoOs.h"

void ReleaseAllPoolStack(DzHost* host)
{
    DzLItr* lItr;
    DzCot* dzCot;
    int i;

    for (i = 0; i < STACK_TYPE_COUNT; i++) {
        if (host->cotStackSize[i] >= DZ_MIN_PAGE_STACK_SIZE) {
            lItr = host->cotPools[i];
            while (lItr) {
                dzCot = MEMBER_BASE(lItr, DzCot, lItr);
                FreeCotStack(host, dzCot);
                lItr = lItr->next;
            }
        }
    }
}

void __stdcall DelayFreeCotHelper(intptr_t context)
{
    DzHost* host = GetHost();
    DzCot* dzCot = (DzCot*)context;

    FreeCotStack(host, dzCot);
    dzCot->lItr.next = host->cotPool;
    host->cotPool = &dzCot->lItr;
}

void __stdcall EventNotifyCotEntry(intptr_t context)
{
    DzHost* host = GetHost();

    host->currCot->entry(context);
    SetEvt(host, host->currCot->evt);
    DelSynObj(host, host->currCot->evt);
}

void __stdcall CallbackTimerEntry(intptr_t context)
{
    DzSynObj* timer = (DzSynObj*)context;

    if (timer->routine) {
        timer->routine(timer->context);
    }
    timer->ref--;
    if (timer->ref == 0) {
        InitDList(&timer->waitQ[CP_HIGH]);
        InitDList(&timer->waitQ[CP_NORMAL]);
        FreeSynObj(GetHost(), timer);
    }
}

// CotScheduleCenter:
// the Cot Schedule Center uses the host's origin thread's stack
// manager all kernel objects that may cause real block
static
void CotScheduleCenter(DzHost* host)
{
    int n;

    while (1) {
        while (1) {
            if (host->scheduleCd) {
                n = DispatchMinTimers(host);
                n = DispatchRmtCots(host, n);
                if (n == 0) {
                    host->currPri = CP_FIRST;
                    Schedule(host);
                    continue;
                } else if (!host->cotCount) {
                    break;
                }
            } else {
                n = 0;
            }
            BlockAndDispatchIo(host, n);
            DispatchMinTimers(host);
            DispatchRmtCots(host, 0);
            host->currPri = CP_FIRST;
            host->scheduleCd = host->ioReactionRate;
            Schedule(host);
        }
        if (host->lazyTimer) {
            DealLazyResEntry(0);
        }
        if (AtomAndInt(&host->mgr->liveSign, ~host->hostMask) != host->hostMask) {
            BlockAndDispatchIo(host, -1);
            if (AtomGetInt(&host->mgr->liveSign)) {
                AtomOrInt(host->rmtCheckSignPtr, RMT_CHECK_AWAKE_SIGN);
                continue;
            }
        } else {
            // be sure quit id 0 host at last, for hostMgr is in id 0 host's stack
            for (n = host->hostCount - 1; n >= 0; n--) {
                if (n != host->hostId) {
                    AwakeRemoteHost(host->mgr->hostArr[n]);
                }
            }
        }
        return;
    }
}

static
void CotScheduleCenterNoRmtCheck(DzHost* host)
{
    int n;

    while (1) {
        if (host->scheduleCd) {
            n = DispatchMinTimers(host);
            if (n == 0) {
                host->currPri = CP_FIRST;
                Schedule(host);
                continue;
            } else if (!host->cotCount) {
                return;
            }
        } else {
            n = 0;
        }
        BlockAndDispatchIoNoRmtCheck(host, n);
        DispatchMinTimers(host);
        host->currPri = CP_FIRST;
        host->scheduleCd = host->ioReactionRate;
        Schedule(host);
    }
}

// RunHost:
// Initial and start the cot host
// the function will block while there are cots running
// after all cots exit, the host will stop and the blocking ends
int RunHost(
    DzHostsMgr* hostMgr,
    int         hostId,
    DzEntry     firstEntry,
    intptr_t    context,
    DzEntry     cleanEntry
    )
{
    int i;
    int ret;
    void* mallocSpace;
    char* handlePool;
    uintptr_t tmp;
    DzHost* host;

    tmp = (uintptr_t)alloca(sizeof(DzHost) + CPU_CACHE_ALIGN);
    host = (DzHost*)((tmp + CPU_CACHE_ALIGN_MASK) & ~CPU_CACHE_ALIGN_MASK);

    tmp = (uintptr_t)PageReserv(sizeof(DzTimerNode*) * TIME_HEAP_SIZE);
    if (!tmp) {
        return DS_NO_MEMORY;
    }
    mallocSpace = create_mspace(0, 0);
    if (!mallocSpace) {
        PageFree((void*)tmp, sizeof(DzTimerNode*) * TIME_HEAP_SIZE);
        return DS_NO_MEMORY;
    }
    handlePool = (char*)PageReserv(HANDLE_POOL_SIZE);
    if (!handlePool) {
        destroy_mspace(mallocSpace);
        PageFree((void*)tmp, sizeof(DzTimerNode*) * TIME_HEAP_SIZE);
        return DS_NO_MEMORY;
    }

    host->currCot = &host->centerCot;
    host->ioReactionRate = SCHEDULE_COUNTDOWN;
    host->scheduleCd = host->ioReactionRate;
    host->lowestPri = hostMgr->lowestPri;
    host->currPri = host->lowestPri + 1;
    host->timerHeap = (DzTimerNode**)tmp;
    host->timerCount = 0;
    host->timerHeapSize = 0;
    host->latestMilUnixTime = MilUnixTime(host);
    host->cotCount = 0;
    host->hostId = hostId;
    host->hostMask = 1 << hostId;
    host->rmtCheckSignPtr = hostMgr->rmtCheckSign + hostId;
    host->rmtFifoArr = hostMgr->rmtFifoRes + hostMgr->hostCount * hostId;
    for (i = 0; i < hostMgr->hostCount; i++) {
        host->rmtFifoArr[i].readPos = hostMgr->rmtReadPos[i] + hostId;
        host->rmtFifoArr[i].writePos = hostMgr->rmtWritePos[hostId] + i;
        *host->rmtFifoArr[i].readPos = 0;
        *host->rmtFifoArr[i].writePos = 0;
        host->rmtFifoArr[i].rmtCotArr = NULL;
        host->rmtFifoArr[i].pendRmtCot = NULL;
    }
    for (i = 0; i <= host->lowestPri; i++) {
        InitSList(&host->taskLs[i]);
    }
    host->mSpace = mallocSpace;
    host->handleBase = (intptr_t)handlePool - host->hostId;
    host->cotStackSize[ST_FIRST] = DZ_MIN_STACK_SIZE;
    host->cotStackSize[ST_US] = hostMgr->smallStackSize;
    host->cotStackSize[ST_UM] = hostMgr->middleStackSize;
    host->cotStackSize[ST_UL] = hostMgr->largeStackSize;
    for (i = ST_FIRST; i < STACK_TYPE_COUNT; i++) {
        host->cotPools[i] = NULL;
        if (host->cotStackSize[i] < DZ_MIN_PAGE_STACK_SIZE) {
            host->cotPoolNowDepth[i] = DZ_MAX_COT_POOL_DEPTH;
        } else {
            host->cotPoolNowDepth[i] = 0;
        }
    }
    host->cotPool = NULL;
    host->synObjPool = NULL;
    host->lNodePool = NULL;
    host->dzFdPool = NULL;
    host->mgr = hostMgr;
    host->pendRmtCot = (DzSList*)alloca(sizeof(DzSList) * hostMgr->hostCount);
    host->lazyRmtCot = (DzSList*)alloca(sizeof(DzSList) * hostMgr->hostCount);
    host->lazyFreeMem = (DzSList*)alloca(sizeof(DzSList) * hostMgr->hostCount);
    for (i = 0; i < hostMgr->hostCount; i++) {
        InitSList(host->pendRmtCot + i);
        InitSList(host->lazyRmtCot + i);
        InitSList(host->lazyFreeMem + i);
    }
    host->memPoolPos = NULL;
    host->memPoolEnd = NULL;
    host->poolGrowList = NULL;
    host->handlePoolPos = handlePool;
    host->handlePoolEnd = handlePool + HANDLE_POOL_SIZE;
    host->lazyTimer = NULL;
    host->hostCount = hostMgr->hostCount;
    host->dftPri = hostMgr->dftPri;
    host->dftSType = hostMgr->dftSType;
    for (i = 0; i < STACK_TYPE_COUNT; i++) {
        host->cotPoolSetDepth[i] = 0;
    }
    if (host->cotStackSize[host->dftSType] >= DZ_MIN_PAGE_STACK_SIZE) {
        host->cotPoolNowDepth[host->dftSType] = DFT_SSIZE_POOL_DEPTH;
        host->cotPoolSetDepth[host->dftSType] = DFT_SSIZE_POOL_DEPTH;
    }
    __Dbg(InitDzHost)(host);

    if (MemeryPoolGrow(host) && InitOsStruct(host)) {
        AtomOrInt(host->rmtCheckSignPtr, RMT_CHECK_AWAKE_SIGN);
        AtomOrInt(&host->mgr->liveSign, host->hostMask);
        SetHost(host);
        host->mgr->hostArr[hostId] = host;

        ret = StartCot(host, firstEntry, context, host->dftPri, host->dftSType);
        if (ret == DS_OK) {
            Schedule(host);
        }
        if (host->hostCount > 0) {
            CotScheduleCenter(host);
        } else {
            CotScheduleCenterNoRmtCheck(host);
        }
        if (cleanEntry) {
            cleanEntry(context);
        }

        // after all cot finished, CotScheduleCenter will return.
        // so cleanup the host struct
        SetHost(NULL);
        CleanOsStruct(host);
    } else {
        ret = DS_NO_MEMORY;
    }

    ReleaseAllPoolStack(host);
    ReleaseMemoryPool(host);
    PageFree(handlePool, HANDLE_POOL_SIZE);
    PageFree(host->timerHeap, sizeof(DzTimerNode*) * TIME_HEAP_SIZE);
    destroy_mspace(host->mSpace);
    return ret;
}

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
    )
{
    int i;
    int ret;
    DzSysParam param;
    DzHostsMgr* hostMgr;
    DzWorker* worker;
    uintptr_t tmp;

    if (!AllocTlsIndex()) {
        return DS_NO_MEMORY;
    }
    tmp = sizeof(DzHostsMgr) + CPU_CACHE_ALIGN;
    tmp += sizeof(DzRmtCotFifo) * hostCount * hostCount + CPU_CACHE_ALIGN;
    tmp = (intptr_t)alloca(tmp);
    tmp = (tmp + CPU_CACHE_ALIGN_MASK) & ~CPU_CACHE_ALIGN_MASK;
    hostMgr = (DzHostsMgr*)tmp;
    tmp += sizeof(DzHostsMgr);
    tmp = (tmp + CPU_CACHE_ALIGN_MASK) & ~CPU_CACHE_ALIGN_MASK;

    for (i = 0; i < DZ_MAX_HOST; i++) {
        hostMgr->hostArr[i] = NULL;
        hostMgr->rmtCheckSign[i] = 0;
        for (ret = 0; ret < DZ_MAX_HOST; ret++) {
            hostMgr->rmtReadPos[i][ret] = 0;
            hostMgr->rmtWritePos[i][ret] = 0;
        }
    }
    hostMgr->liveSign = 0;
    hostMgr->hostCount = hostCount;
    hostMgr->workerNowDepth = 0;
    hostMgr->workerSetDepth = 0;
    hostMgr->workerPool = NULL;
    hostMgr->rmtFifoRes = (DzRmtCotFifo*)tmp;
    hostMgr->rmtFifoCotArrRes = NULL;
    hostMgr->lowestPri = lowestPri;
    hostMgr->dftPri = dftPri;
    hostMgr->dftSType = dftSType;

    if (smallStackSize < DZ_MIN_PAGE_STACK_SIZE) {
        smallStackSize += DZ_PERMENENT_STACK_BOUNDARY - 1;
        smallStackSize &= ~(DZ_PERMENENT_STACK_BOUNDARY - 1);
    } else {
        smallStackSize += DZ_PAGE_STACK_BOUNDARY - 1;
        smallStackSize &= ~(DZ_PAGE_STACK_BOUNDARY - 1);
    }

    if (middleStackSize < DZ_MIN_PAGE_STACK_SIZE) {
        middleStackSize += DZ_PERMENENT_STACK_BOUNDARY - 1;
        middleStackSize &= ~(DZ_PERMENENT_STACK_BOUNDARY - 1);
    } else {
        middleStackSize += DZ_PAGE_STACK_BOUNDARY - 1;
        middleStackSize &= ~(DZ_PAGE_STACK_BOUNDARY - 1);
    }

    if (largeStackSize < DZ_MIN_PAGE_STACK_SIZE) {
        largeStackSize += DZ_PERMENENT_STACK_BOUNDARY - 1;
        largeStackSize &= ~(DZ_PERMENENT_STACK_BOUNDARY - 1);
    } else {
        largeStackSize += DZ_PAGE_STACK_BOUNDARY - 1;
        largeStackSize &= ~(DZ_PAGE_STACK_BOUNDARY - 1);
    }

    hostMgr->smallStackSize = smallStackSize;
    hostMgr->middleStackSize = middleStackSize;
    hostMgr->largeStackSize = largeStackSize;

    for (i = 0; i < hostCount; i++) {
        InitSysAutoEvt(hostMgr->sysAutoEvt + i);
        NotifySysAutoEvt(hostMgr->sysAutoEvt + i);
    }

    param.result = DS_OK;
    if (hostCount > 0) {
        param.cs.entry = firstEntry;
        param.cs.context = context;
        firstEntry = MainHostFirstEntry;
        context = (intptr_t)&param;
    }
    ret = RunHost(hostMgr, 0, firstEntry, context, cleanEntry);
    while (hostMgr->workerPool) {
        worker = MEMBER_BASE(hostMgr->workerPool, DzWorker, lItr);
        hostMgr->workerPool = hostMgr->workerPool->next;
        worker->dzCot = NULL;
        NotifySysAutoEvt(&worker->sysEvt);
    }
    for (i = 0; i < hostCount; i++) {
        FreeSysAutoEvt(hostMgr->sysAutoEvt + i);
    }
    FreeTlsIndex();
    return ret == DS_OK ? param.result : ret;
}
