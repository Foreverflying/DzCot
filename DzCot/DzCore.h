/********************************************************************
    created:    2010/02/11 21:49
    file:       DzCore.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzCore_h__
#define __DzCore_h__

#include "DzStructs.h"
#include "DzBase.h"
#include "DzResourceMgr.h"
#include "DzSchedule.h"
#include "DzRmtCore.h"
#include "dlmalloc.h"
#include "DzCoreOs.h"

#ifdef __cplusplus
extern "C"{
#endif

void __stdcall DelayFreeCotHelper( intptr_t context );
void __stdcall EventNotifyCotEntry( intptr_t context );
void CotScheduleCenter( DzHost* host );

inline void ReleaseAllPoolStack( DzHost* host )
{
    DzLItr* lItr;
    DzCot* dzCot;
    int i;

    for( i = DZ_MAX_PERSIST_STACK_SIZE + 1; i <= host->lowestPri; i++ ){
        lItr = host->cotPools[i];
        while( lItr ){
            dzCot = MEMBER_BASE( lItr, DzCot, lItr );
            FreeCotStack( dzCot );
            lItr = lItr->next;
        }
    }
}

inline DzCot* AllocDzCot( DzHost* host, int sSize )
{
    DzCot* dzCot;

    if( host->cotPools[ sSize ] ){
        dzCot = MEMBER_BASE( host->cotPools[ sSize ], DzCot, lItr );
        host->cotPools[ sSize ] = host->cotPools[ sSize ]->next;
        host->cotPoolNowDepth[ sSize ] ++;
    }else{
        if( !host->cotPool ){
            if( !AllocDzCotPool( host ) ){
                return NULL;
            }
        }
        dzCot = MEMBER_BASE( host->cotPool, DzCot, lItr );
        if( InitCot( host, dzCot, sSize ) ){
            host->cotPool = host->cotPool->next;
        }else{
            return NULL;
        }
    }
    return dzCot;
}

inline void FreeDzCot( DzHost* host, DzCot* dzCot )
{
    DzCot* tmp;

    if( host->cotPoolNowDepth[ dzCot->sSize ] > 0 ){
        dzCot->lItr.next = host->cotPools[ dzCot->sSize ];
        host->cotPools[ dzCot->sSize ] = &dzCot->lItr;
        host->cotPoolNowDepth[ dzCot->sSize ] --;
    }else{
        //can not FreeCotStack( dzCot ) here!!
        //the stack is still in use before switch
        if( host->cotPools[ dzCot->sSize ] ){
            //if the cotPool is not empty swap and free the head
            tmp = MEMBER_BASE( host->cotPools[ dzCot->sSize ], DzCot, lItr );
            dzCot->lItr.next = tmp->lItr.next;
            host->cotPools[ dzCot->sSize ] = &dzCot->lItr;
            FreeCotStack( tmp );
            tmp->lItr.next = host->cotPool;
            host->cotPool = &tmp->lItr;
        }else{
            //switch to a helper cot to free it
            StartCot( host, DelayFreeCotHelper, (intptr_t)dzCot, host->lowestPri, SS_FIRST );
        }
    }
}

// StartCot:
// create a new cot
inline int StartCot(
    DzHost*     host,
    DzRoutine   entry,
    intptr_t    context,
    int         priority,
    int         sSize
    )
{
    DzCot* dzCot;

    dzCot = AllocDzCot( host, sSize );
    if( !dzCot ){
        return DS_NO_MEMORY;
    }
    dzCot->priority = priority;
    if( priority < host->currPri ){
        host->currPri = priority;
    }
    host->cotCount++;
    SetCotEntry( dzCot, entry, context );
    DispatchCot( host, dzCot );
    return DS_OK;
}

// StartCotInstant:
// create a new cot and run it at once
// current cot is paused and will continue at next schedule
inline int StartCotInstant(
    DzHost*     host,
    DzRoutine   entry,
    intptr_t    context,
    int         priority,
    int         sSize
    )
{
    DzCot* dzCot;

    dzCot = AllocDzCot( host, sSize );
    if( !dzCot ){
        return DS_NO_MEMORY;
    }
    dzCot->priority = priority;
    host->cotCount++;
    host->scheduleCd++;
    SetCotEntry( dzCot, entry, context );
    TemporaryPushCot( host, host->currCot );
    SwitchToCot( host, dzCot );
    return DS_OK;
}

// EvtStartCot
// equal to StartCot, notify an manual event SynObj when cot finished
inline int EvtStartCot(
    DzHost*     host,
    DzSynObj*   evt,
    DzRoutine   entry,
    intptr_t    context,
    int         priority,
    int         sSize
    )
{
    DzCot* dzCot;

    dzCot = AllocDzCot( host, sSize );
    if( !dzCot ){
        return DS_NO_MEMORY;
    }
    dzCot->priority = priority;
    if( priority < host->currPri ){
        host->currPri = priority;
    }
    host->cotCount++;
    SetCotEntry( dzCot, EventNotifyCotEntry, context );
    dzCot->entry = entry;
    dzCot->evt = CloneSynObj( evt );
    DispatchCot( host, dzCot );
    return DS_OK;
}

// EvtStartCotInstant:
// equal to StartCotInstant, notify an manual event SynObj when cot finished
inline int EvtStartCotInstant(
    DzHost*     host,
    DzSynObj*   evt,
    DzRoutine   entry,
    intptr_t    context,
    int         priority,
    int         sSize
    )
{
    DzCot* dzCot;

    dzCot = AllocDzCot( host, sSize );
    if( !dzCot ){
        return DS_NO_MEMORY;
    }
    dzCot->priority = priority;
    host->cotCount++;
    host->scheduleCd++;
    SetCotEntry( dzCot, EventNotifyCotEntry, context );
    dzCot->entry = entry;
    dzCot->evt = CloneSynObj( evt );
    TemporaryPushCot( host, host->currCot );
    SwitchToCot( host, dzCot );
    return DS_OK;
}

inline int StartRemoteCot(
    DzHost*     host,
    int         rmtId,
    DzRoutine   entry,
    intptr_t    context,
    int         priority,
    int         sSize
    )
{
    DzCot* dzCot;

    dzCot = AllocDzCot( host, sSize );
    if( !dzCot ){
        return DS_NO_MEMORY;
    }
    dzCot->hostId = host->hostId;
    dzCot->entry = entry;
    dzCot->feedType = 0;
    dzCot->priority = priority;
    SetCotEntry( dzCot, RemoteCotEntry, context );
    SendRmtCot( host, rmtId, FALSE, dzCot );
    return DS_OK;
}

inline int EvtStartRemoteCot(
    DzHost*     host,
    DzHandle    evt,
    int         rmtId,
    DzRoutine   entry,
    intptr_t    context,
    int         priority,
    int         sSize
    )
{
    DzCot* dzCot;

    dzCot = AllocDzCot( host, sSize );
    if( !dzCot ){
        return DS_NO_MEMORY;
    }
    dzCot->hostId = host->hostId;
    dzCot->entry = entry;
    dzCot->feedType = 1;
    dzCot->evtType = 0;
    dzCot->evt = CloneSynObj( evt );
    dzCot->priority = priority;
    SetCotEntry( dzCot, RemoteCotEntry, context );
    SendRmtCot( host, rmtId, FALSE, dzCot );
    return DS_OK;
}

inline int RunRemoteCot(
    DzHost*     host,
    int         rmtId,
    DzRoutine   entry,
    intptr_t    context,
    int         priority,
    int         sSize
    )
{
    DzCot* dzCot;
    DzEasyEvt easyEvt;

    dzCot = AllocDzCot( host, sSize );
    if( !dzCot ){
        return DS_NO_MEMORY;
    }
    dzCot->hostId = host->hostId;
    dzCot->entry = entry;
    dzCot->feedType = 1;
    dzCot->evtType = 1;
    dzCot->easyEvt = &easyEvt;
    dzCot->priority = priority;
    SetCotEntry( dzCot, RemoteCotEntry, context );
    SendRmtCot( host, rmtId, FALSE, dzCot );
    WaitEasyEvt( host, &easyEvt );
    return DS_OK;
}

inline int RunWorker( DzHost* host, DzRoutine entry, intptr_t context )
{
    DzLItr* lItr;
    DzSysParam param;
    DzWorker* worker;

    lItr = AtomPopSList( &host->mgr->workerPool );
    if( lItr ){
        AtomIncInt( &host->mgr->workerNowDepth );
        worker = MEMBER_BASE( lItr, DzWorker, lItr );
        worker->entry = entry;
        worker->context = context;
        worker->dzCot = host->currCot;
        worker->dzCot->hostId = host->hostId;
        NotifySysAutoEvt( &worker->sysEvt );
    }else{
        param.threadEntry = WorkerMain;
        param.wk.entry = entry;
        param.wk.context = context;
        param.wk.dzCot = host->currCot;
        param.wk.dzCot->hostId = host->hostId;
        param.wk.hostMgr = host->mgr;
        StartSystemThread( &param, WORKER_STACK_SIZE );
    }
    Schedule( host );
    host->cotCount--;
    return DS_OK;
}

// RunHost:
// Initial and start the cot host
// the function will block while there are cots running
// after all cots exit, the host will stop and the blocking ends
inline int RunHost(
    DzHostsMgr* hostMgr,
    int         hostId,
    int         lowestPri,
    int         dftPri,
    int         dftSSize,
    DzRoutine   firstEntry,
    intptr_t    context
    )
{
    int i;
    int ret;
    void* mallocSpace;
    uintptr_t tmp;
    DzHost* host;

    tmp = (uintptr_t)alloca( sizeof( DzHost ) + CPU_CACHE_ALIGN );
    host = (DzHost*)( ( tmp + CPU_CACHE_ALIGN_MASK ) & ~CPU_CACHE_ALIGN_MASK );

    tmp = (uintptr_t)PageReserv( sizeof(DzTimerNode*) * TIME_HEAP_SIZE );
    if( !tmp ){
        PageFree( (void*)tmp, sizeof(DzTimerNode*) * TIME_HEAP_SIZE );
        return DS_NO_MEMORY;
    }
    mallocSpace = create_mspace( 0, 0 );
    if( !mallocSpace ){
        destroy_mspace( mallocSpace );
        PageFree( (void*)tmp, sizeof(DzTimerNode*) * TIME_HEAP_SIZE );
        return DS_NO_MEMORY;
    }

    host->currCot = &host->centerCot;
    host->cotCount = 0;
    host->scheduleCd = SCHEDULE_COUNTDOWN;
    host->dftPri = dftPri;
    host->dftSSize = dftSSize;
    host->timerHeap = (DzTimerNode**)tmp;
    host->timerCount = 0;
    host->timerHeapSize = 0;
    host->lowestPri = lowestPri;
    host->currPri = lowestPri + 1;
    host->mSpace = mallocSpace;
    host->hostId = hostId;
    host->hostMask = 1 << hostId;
    host->rmtCheckSignPtr = hostMgr->rmtCheckSign + hostId;
    host->rmtFifoArr = hostMgr->rmtFifoRes + hostMgr->hostCount * hostId;
    for( i = 0; i < hostMgr->hostCount; i++ ){
        host->rmtFifoArr[i].readPos = hostMgr->rmtReadPos[ i ] + hostId;
        host->rmtFifoArr[i].writePos = hostMgr->rmtWritePos[ hostId ] + i;
        host->rmtFifoArr[i].rmtCotArr = NULL;
    }
    for( i = 0; i <= lowestPri; i++ ){
        InitSList( &host->taskLs[i] );
    }
    host->mgr = hostMgr;
    host->checkFifo = NULL;
    for( i = SS_FIRST; i <= DZ_MAX_PERSIST_STACK_SIZE; i++ ){
        host->cotPools[i] = NULL;
        host->cotPoolNowDepth[i] = DZ_MAX_COT_POOL_DEPTH;
    }
    for( ; i < STACK_SIZE_COUNT; i++ ){
        host->cotPools[i] = NULL;
        host->cotPoolNowDepth[i] = 0;
    }
    host->cotPool = NULL;
    host->synObjPool = NULL;
    host->lNodePool = NULL;
    host->asyncIoPool = NULL;
    host->pendRmtCot = (DzSList*)alloca( sizeof( DzSList ) * hostMgr->hostCount );
    host->lazyRmtCot = (DzSList*)alloca( sizeof( DzSList ) * hostMgr->hostCount );
    host->lazyFreeMem = (DzSList*)alloca( sizeof( DzSList ) * hostMgr->hostCount );
    for( i = 0; i < hostMgr->hostCount; i++ ){
        InitSList( host->pendRmtCot + i );
        InitSList( host->lazyRmtCot + i );
        InitSList( host->lazyFreeMem + i );
    }
    host->lazyTimer = NULL;
    host->memPoolPos = NULL;
    host->memPoolEnd = NULL;
    host->poolGrowList = NULL;
    host->hostCount = hostMgr->hostCount;
    host->servMask = hostMgr->servMask[ hostId ];
    for( i = 0; i < STACK_SIZE_COUNT; i++ ){
        host->cotPoolSetDepth[i] = 0;
    }
    if( dftSSize > DZ_MAX_PERSIST_STACK_SIZE ){
        host->cotPoolNowDepth[ dftSSize ] = DFT_SSIZE_POOL_DEPTH;
        host->cotPoolSetDepth[ dftSSize ] = DFT_SSIZE_POOL_DEPTH;
    }

    if( MemeryPoolGrow( host ) && InitOsStruct( host ) ){
        AtomOrInt( &host->mgr->exitSign, 1 << hostId );
        SetHost( host );
        host->mgr->hostArr[ hostId ] = host;

        ret = StartCot( host, firstEntry, context, dftPri, dftSSize );
        if( ret == DS_OK ){
            Schedule( host );
        }
        CotScheduleCenter( host );

        //after all cot finished, IoMgrRoutine will return.
        //so cleanup the host struct
        host->mgr->hostArr[ hostId ] = NULL;
        SetHost( NULL );
        DeleteOsStruct( host );
    }else{
        ret = DS_NO_MEMORY;
    }

    ReleaseAllPoolStack( host );
    ReleaseMemoryPool( host );
    PageFree( host->timerHeap, sizeof(DzTimerNode*) * TIME_HEAP_SIZE );
    destroy_mspace( host->mSpace );

    return ret;
}

inline int RunHosts(
    int         hostCount,
    int*        servMask,
    int         lowestPri,
    int         dftPri,
    int         dftSSize,
    DzRoutine   firstEntry,
    intptr_t    context
    )
{
    int i;
    int ret;
    DzSysParam param;
    DzHostsMgr* hostMgr;
    DzWorker* worker;
    uintptr_t tmp;

    if( !AllocTlsIndex() ){
        return DS_NO_MEMORY;
    }
    tmp = sizeof( DzHostsMgr ) + CPU_CACHE_ALIGN;
    tmp += sizeof( DzRmtCotFifo ) * hostCount * hostCount + CPU_CACHE_ALIGN;
    tmp = (intptr_t)alloca( tmp );
    tmp = ( tmp + CPU_CACHE_ALIGN_MASK ) & ~CPU_CACHE_ALIGN_MASK;
    hostMgr = (DzHostsMgr*)tmp;
    tmp += sizeof( DzHostsMgr );
    tmp = ( tmp + CPU_CACHE_ALIGN_MASK ) & ~CPU_CACHE_ALIGN_MASK;

    for( i = 0; i < DZ_MAX_HOST; i++ ){
        hostMgr->hostArr[i] = NULL;
        hostMgr->rmtCheckSign[i] = 0;
        for( ret = 0; ret < DZ_MAX_HOST; ret++ ){
            hostMgr->rmtReadPos[i][ret] = 0;
            hostMgr->rmtWritePos[i][ret] = 0;
        }
    }
    hostMgr->exitSign = 0;
    hostMgr->hostCount = hostCount;
    hostMgr->workerNowDepth = 0;
    hostMgr->workerSetDepth = 0;
    hostMgr->workerPool = NULL;
    hostMgr->rmtFifoRes = (DzRmtCotFifo*)tmp;
    hostMgr->servMask = servMask;

    for( i = 0; i < hostCount; i++ ){
        for( ret = 0; ret < hostCount; ret++ ){
            if( servMask[i] & ( 1 << ret ) ){
                servMask[ret] |= ( 1 << i );
            }
        }
    }
    for( i = 0; i < hostCount; i++ ){
        if( servMask[i] & ( 1 << i ) ){
            InitSysAutoEvt( hostMgr->sysAutoEvt + i );
            NotifySysAutoEvt( hostMgr->sysAutoEvt + i );
        }
    }

    param.result = DS_OK;
    if( hostCount > 1 ){
        param.cs.entry = firstEntry;
        param.cs.context = context;
        firstEntry = MainHostFirstEntry;
        context = (intptr_t)&param;
    }
    ret = RunHost(
        hostMgr, 0, lowestPri, dftPri, dftSSize,
        firstEntry, context
        );
    while( hostMgr->workerPool ){
        worker = MEMBER_BASE( hostMgr->workerPool, DzWorker, lItr );
        hostMgr->workerPool = hostMgr->workerPool->next;
        worker->dzCot = NULL;
        NotifySysAutoEvt( &worker->sysEvt );
    }
    for( i = 0; i < hostCount; i++ ){
        if( servMask[i] & ( 1 << i ) ){
            FreeSysAutoEvt( hostMgr->sysAutoEvt + i );
        }
    }
    FreeTlsIndex();
    return ret == DS_OK ? param.result : ret;;
}

inline int GetCotCount( DzHost* host )
{
    return host->cotCount;
}

inline int SetCurrCotPriority( DzHost* host, int priority )
{
    int ret;

    ret = host->currCot->priority;
    if( priority >= CP_FIRST ){
        host->currCot->priority = priority;
        if( priority < ret ){
            host->currPri = priority;
        }
    }
    return ret;
}

inline int SetCotPoolDepth( DzHost* host, int sSize, int depth )
{
    int deta;
    int ret;

    ret = host->cotPoolSetDepth[ sSize ];
    if( depth >= 0 && sSize > DZ_MAX_PERSIST_STACK_SIZE ){
        deta = depth - host->cotPoolSetDepth[ sSize ];
        host->cotPoolNowDepth[ sSize ] += deta;
        host->cotPoolSetDepth[ sSize ] = depth;
    }
    return ret;
}

inline int SetWorkerPoolDepth( DzHost* host, int depth )
{
    int setDepth;
    int nowSet;

    setDepth = AtomReadInt( &host->mgr->workerSetDepth );
    do{
        nowSet = setDepth;
        setDepth = AtomCasInt( &host->mgr->workerSetDepth, setDepth, depth );
    }while( setDepth != nowSet );
    AtomAddInt( &host->mgr->workerNowDepth, depth - setDepth );
    return setDepth;
}

inline int SetHostParam(
    DzHost*     host,
    int         lowestPri,
    int         dftPri,
    int         dftSSize
    )
{
    int ret;

    ret = host->lowestPri;
    if( lowestPri > host->lowestPri ){
        host->lowestPri = lowestPri;
    }
    host->dftPri = dftPri;
    host->dftSSize = dftSSize;
    if( host->cotPoolSetDepth[ dftSSize ] < DFT_SSIZE_POOL_DEPTH ){
        SetCotPoolDepth( host, dftSSize, DFT_SSIZE_POOL_DEPTH );
    }

    return ret;
}

inline void* Malloc( DzHost* host, size_t size )
{
    return mspace_malloc( host->mSpace, size );
}

inline void* Calloc( DzHost* host, size_t num, size_t size )
{
    return mspace_calloc( host->mSpace, num, size );
}

inline void* ReAlloc( DzHost* host, void* mem, size_t size )
{
    return mspace_realloc( host->mSpace, mem, size );
}

inline void Free( DzHost* host, void* mem )
{
    mspace_free( host->mSpace, mem );
}

inline void* MallocEx( DzHost* host, size_t size )
{
    DzMemExTag* ret;

    ret = (DzMemExTag*)mspace_malloc( host->mSpace, size + sizeof( int64 ) );
    ret->hostId = host->hostId;
    return ret + 1;
}

inline void FreeEx( DzHost* host, void* mem )
{
    DzLNode* node;
    DzMemExTag* base;

    base = ( (DzMemExTag*)mem ) - 1;
    if( base->hostId == host->hostId ){
        mspace_free( host->mSpace, base );
    }else{
        node = AllocLNode( host );
        node->d1 = (intptr_t)base;
        AddLItrToTail( host->lazyFreeMem + base->hostId, &node->lItr );
        if( !host->lazyTimer ){
            StartLazyTimer( host );
        }
    }
}

inline int DispatchMinTimers( DzHost* host )
{
    DzTimerNode* timerNode;
    int64 currTime;
    int64 cmpTime;
    BOOL ret;

    while( host->timerCount > 0 ){
        ret = FALSE;
        currTime = MilUnixTime();
        cmpTime = currTime + MIN_TIME_INTERVAL;
        while( GetMinTimerNode( host )->timestamp <= cmpTime ){
            timerNode = GetMinTimerNode( host );
            timerNode->repeat--;
            if( timerNode->repeat != 0 ){
                if( timerNode->repeat < 0 ){
                    timerNode->repeat = 0;
                }
                timerNode->timestamp -= timerNode->interval;
                AdjustMinTimer( host, timerNode );
            }else{
                RemoveMinTimer( host );
            }
            ret = NotifyTimerNode( host, timerNode ) || ret;
            if( host->timerCount == 0 ){
                return ret ? 0 : -1;
            }
        }
        if( ret ){
            return 0;
        }else{
            return (int)( GetMinTimerNode( host )->timestamp - currTime );
        }
    }
    return -1;
}

inline void DealRmtCot( DzHost* host, DzCot* dzCot )
{
    DzLItr* nextItr;

    nextItr = NULL;
    while( 1 ){
        if( dzCot->priority < 0 ){
            dzCot->priority += CP_DEFAULT + 1;
            nextItr = dzCot->lItr.next;
        }
        host->cotCount++;
        if( dzCot->priority == CP_DEFAULT ){
            SwitchToCot( host, dzCot );
        }else{
            if( dzCot->priority > host->lowestPri ){
                dzCot->priority = host->lowestPri;
            }
            DispatchCot( host, dzCot );
        }
        if( !nextItr ){
            return;
        }
        dzCot = MEMBER_BASE( nextItr, DzCot, lItr );
        nextItr = dzCot->lItr.next;
    }
}

inline int DispatchRmtCots( DzHost* host, int timeout )
{
    int count;
    int nowCount;
    int readPos;
    int writePos;
    int scheduleCd;
    DzCot* dzCot;

    if( timeout ){
        nowCount = AtomCasInt( host->rmtCheckSignPtr, 0, RMT_CHECK_SLEEP_SIGN );
    }else{
        nowCount = AtomReadInt( host->rmtCheckSignPtr );
    }
    if( nowCount == 0 ){
        return timeout;
    }
    count = nowCount;
    scheduleCd = host->scheduleCd;
    host->scheduleCd = 0;
    while( 1 ){
        readPos = AtomReadInt( host->checkFifo->readPos );
        writePos = AtomReadInt( host->checkFifo->writePos );
        if( readPos != writePos ){
            dzCot = host->checkFifo->rmtCotArr[ readPos ];
            readPos++;
            if( readPos == RMT_CALL_FIFO_SIZE ){
                readPos = 1;
            }
            AtomSetInt( host->checkFifo->readPos, readPos );
            DealRmtCot( host, dzCot );
            nowCount--;
            if( nowCount == 0 ){
                nowCount = AtomSubInt( host->rmtCheckSignPtr, count );
                nowCount -= count;
                if( nowCount == 0 ){
                    host->scheduleCd = scheduleCd;
                    return 0;
                }
                count = nowCount;
            }
        }
        host->checkFifo = host->checkFifo->next;
    }
}

#ifdef __cplusplus
};
#endif

#endif // __DzCore_h__
