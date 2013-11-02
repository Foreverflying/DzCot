/**
 *  @file       DzCore.h
 *  @brief      
 *  @author	    Foreverflying <foreverflying@live.cn>
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
#include "dlmalloc.h"
#include "DzCoreOs.h"

#ifdef __cplusplus
extern "C"{
#endif

void __stdcall DelayFreeCotHelper( intptr_t context );
void __stdcall EventNotifyCotEntry( intptr_t context );
void CotScheduleCenter( DzHost* host );
void CotScheduleCenterNoRmtCheck( DzHost* host );
void ReleaseAllPoolStack( DzHost* host );

inline DzCot* AllocDzCot( DzHost* host, int sType )
{
    DzCot* dzCot;

    if( host->cotPools[ sType ] ){
        dzCot = MEMBER_BASE( host->cotPools[ sType ], DzCot, lItr );
        host->cotPools[ sType ] = host->cotPools[ sType ]->next;
        host->cotPoolNowDepth[ sType ] ++;
    }else{
        if( !host->cotPool ){
            if( !AllocDzCotPool( host ) ){
                return NULL;
            }
        }
        dzCot = MEMBER_BASE( host->cotPool, DzCot, lItr );
        if( InitCot( host, dzCot, sType ) ){
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

    __Dbg( InitDzCot )( host, dzCot );
    if( host->cotPoolNowDepth[ dzCot->sType ] > 0 ){
        dzCot->lItr.next = host->cotPools[ dzCot->sType ];
        host->cotPools[ dzCot->sType ] = &dzCot->lItr;
        host->cotPoolNowDepth[ dzCot->sType ] --;
    }else{
        //can not FreeCotStack( dzCot ) here!!
        //the stack is still in use before switch
        if( host->cotPools[ dzCot->sType ] ){
            //if the cotPool is not empty swap and free the head
            tmp = MEMBER_BASE( host->cotPools[ dzCot->sType ], DzCot, lItr );
            dzCot->lItr.next = tmp->lItr.next;
            host->cotPools[ dzCot->sType ] = &dzCot->lItr;
            FreeCotStack( host, tmp );
            tmp->lItr.next = host->cotPool;
            host->cotPool = &tmp->lItr;
        }else{
            //switch to a helper cot to free it
            StartCot( host, DelayFreeCotHelper, (intptr_t)dzCot, host->currPri, ST_FIRST );
        }
    }
}

// StartCot:
// create a new cot
inline int StartCot(
    DzHost*     host,
    DzEntry     entry,
    intptr_t    context,
    int         priority,
    int         sType
    )
{
    DzCot* dzCot;

    dzCot = AllocDzCot( host, sType );
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
    DzEntry     entry,
    intptr_t    context,
    int         priority,
    int         sType
    )
{
    DzCot* dzCot;

    dzCot = AllocDzCot( host, sType );
    if( !dzCot ){
        return DS_NO_MEMORY;
    }
    dzCot->priority = priority;
    host->cotCount++;
    host->scheduleCd++;
    SetCotEntry( dzCot, entry, context );
    PushCotToTop( host, host->currCot );
    SwitchToCot( host, dzCot );
    return DS_OK;
}

// EvtStartCot
// equal to StartCot, notify an manual event SynObj when cot finished
inline int EvtStartCot(
    DzHost*     host,
    DzSynObj*   evt,
    DzEntry     entry,
    intptr_t    context,
    int         priority,
    int         sType
    )
{
    DzCot* dzCot;

    dzCot = AllocDzCot( host, sType );
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
    DzEntry     entry,
    intptr_t    context,
    int         priority,
    int         sType
    )
{
    DzCot* dzCot;

    dzCot = AllocDzCot( host, sType );
    if( !dzCot ){
        return DS_NO_MEMORY;
    }
    dzCot->priority = priority;
    host->cotCount++;
    host->scheduleCd++;
    SetCotEntry( dzCot, EventNotifyCotEntry, context );
    dzCot->evt = CloneSynObj( evt );
    dzCot->evt->ref++;
    PushCotToTop( host, host->currCot );
    SwitchToCot( host, dzCot );
    return DS_OK;
}

inline int StartRemoteCot(
    DzHost*     host,
    int         rmtId,
    DzEntry     entry,
    intptr_t    context,
    int         priority,
    int         sType
    )
{
    DzCot* dzCot;

    dzCot = AllocDzCot( host, sType );
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
    DzSynObj*   evt,
    int         rmtId,
    DzEntry     entry,
    intptr_t    context,
    int         priority,
    int         sType
    )
{
    DzCot* dzCot;

    dzCot = AllocDzCot( host, sType );
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
    DzEntry     entry,
    intptr_t    context,
    int         priority,
    int         sType
    )
{
    DzCot* dzCot;
    DzEasyEvt easyEvt;

    dzCot = AllocDzCot( host, sType );
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

inline int RunWorker( DzHost* host, DzEntry entry, intptr_t context )
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

    tmp = (uintptr_t)alloca( sizeof( DzHost ) + CPU_CACHE_ALIGN );
    host = (DzHost*)( ( tmp + CPU_CACHE_ALIGN_MASK ) & ~CPU_CACHE_ALIGN_MASK );

    tmp = (uintptr_t)PageReserv( sizeof(DzTimerNode*) * TIME_HEAP_SIZE );
    if( !tmp ){
        return DS_NO_MEMORY;
    }
    mallocSpace = create_mspace( 0, 0 );
    if( !mallocSpace ){
        PageFree( (void*)tmp, sizeof(DzTimerNode*) * TIME_HEAP_SIZE );
        return DS_NO_MEMORY;
    }
    handlePool = (char*)PageReserv( HANDLE_POOL_SIZE );
    if( !handlePool ){
        destroy_mspace( mallocSpace );
        PageFree( (void*)tmp, sizeof(DzTimerNode*) * TIME_HEAP_SIZE );
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
    host->latestMilUnixTime = MilUnixTime( host );
    host->cotCount = 0;
    host->hostId = hostId;
    host->hostMask = 1 << hostId;
    host->rmtCheckSignPtr = hostMgr->rmtCheckSign + hostId;
    host->rmtFifoArr = hostMgr->rmtFifoRes + hostMgr->hostCount * hostId;
    for( i = 0; i < hostMgr->hostCount; i++ ){
        host->rmtFifoArr[i].readPos = hostMgr->rmtReadPos[ i ] + hostId;
        host->rmtFifoArr[i].writePos = hostMgr->rmtWritePos[ hostId ] + i;
        *host->rmtFifoArr[i].readPos = 0;
        *host->rmtFifoArr[i].writePos = 0;
        host->rmtFifoArr[i].rmtCotArr = NULL;
        host->rmtFifoArr[i].pendRmtCot = NULL;
    }
    for( i = 0; i <= host->lowestPri; i++ ){
        InitSList( &host->taskLs[i] );
    }
    host->mSpace = mallocSpace;
    host->handleBase = (intptr_t)handlePool - host->hostId;
    host->cotStackSize[ ST_FIRST ] = DZ_MIN_STACK_SIZE;
    host->cotStackSize[ ST_US ] = hostMgr->smallStackSize;
    host->cotStackSize[ ST_UM ] = hostMgr->middleStackSize;
    host->cotStackSize[ ST_UL ] = hostMgr->largeStackSize;
    for( i = ST_FIRST; i < STACK_TYPE_COUNT; i++ ){
        host->cotPools[i] = NULL;
        if( host->cotStackSize[i] < DZ_MIN_PAGE_STACK_SIZE ){
            host->cotPoolNowDepth[i] = DZ_MAX_COT_POOL_DEPTH;
        }else{
            host->cotPoolNowDepth[i] = 0;
        }
    }
    host->cotPool = NULL;
    host->synObjPool = NULL;
    host->lNodePool = NULL;
    host->dzFdPool = NULL;
    host->mgr = hostMgr;
    host->pendRmtCot = (DzSList*)alloca( sizeof( DzSList ) * hostMgr->hostCount );
    host->lazyRmtCot = (DzSList*)alloca( sizeof( DzSList ) * hostMgr->hostCount );
    host->lazyFreeMem = (DzSList*)alloca( sizeof( DzSList ) * hostMgr->hostCount );
    for( i = 0; i < hostMgr->hostCount; i++ ){
        InitSList( host->pendRmtCot + i );
        InitSList( host->lazyRmtCot + i );
        InitSList( host->lazyFreeMem + i );
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
    for( i = 0; i < STACK_TYPE_COUNT; i++ ){
        host->cotPoolSetDepth[i] = 0;
    }
    if( host->cotStackSize[ host->dftSType ] >= DZ_MIN_PAGE_STACK_SIZE ){
        host->cotPoolNowDepth[ host->dftSType ] = DFT_SSIZE_POOL_DEPTH;
        host->cotPoolSetDepth[ host->dftSType ] = DFT_SSIZE_POOL_DEPTH;
    }
    __Dbg( InitDzHost )( host );

    if( MemeryPoolGrow( host ) && InitOsStruct( host ) ){
        AtomOrInt( host->rmtCheckSignPtr, RMT_CHECK_AWAKE_SIGN );
        AtomOrInt( &host->mgr->liveSign, host->hostMask );
        SetHost( host );
        host->mgr->hostArr[ hostId ] = host;

        ret = StartCot( host, firstEntry, context, host->dftPri, host->dftSType );
        if( ret == DS_OK ){
            Schedule( host );
        }
        if( host->hostCount > 0 ){
            CotScheduleCenter( host );
        }else{
            CotScheduleCenterNoRmtCheck( host );
        }
        if( cleanEntry ){
            cleanEntry( context );
        }

        //after all cot finished, CotScheduleCenter will return.
        //so cleanup the host struct
        SetHost( NULL );
        CleanOsStruct( host );
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

    if( smallStackSize < DZ_MIN_PAGE_STACK_SIZE ){
        smallStackSize += DZ_PERMENENT_STACK_BOUNDARY - 1;
        smallStackSize &= ~( DZ_PERMENENT_STACK_BOUNDARY - 1 );
    }else{
        smallStackSize += DZ_PAGE_STACK_BOUNDARY - 1;
        smallStackSize &= ~( DZ_PAGE_STACK_BOUNDARY - 1 );
    }

    if( middleStackSize < DZ_MIN_PAGE_STACK_SIZE ){
        middleStackSize += DZ_PERMENENT_STACK_BOUNDARY - 1;
        middleStackSize &= ~( DZ_PERMENENT_STACK_BOUNDARY - 1 );
    }else{
        middleStackSize += DZ_PAGE_STACK_BOUNDARY - 1;
        middleStackSize &= ~( DZ_PAGE_STACK_BOUNDARY - 1 );
    }

    if( largeStackSize < DZ_MIN_PAGE_STACK_SIZE ){
        largeStackSize += DZ_PERMENENT_STACK_BOUNDARY - 1;
        largeStackSize &= ~( DZ_PERMENENT_STACK_BOUNDARY - 1 );
    }else{
        largeStackSize += DZ_PAGE_STACK_BOUNDARY - 1;
        largeStackSize &= ~( DZ_PAGE_STACK_BOUNDARY - 1 );
    }

    hostMgr->smallStackSize = smallStackSize;
    hostMgr->middleStackSize = middleStackSize;
    hostMgr->largeStackSize = largeStackSize;

    for( i = 0; i < hostCount; i++ ){
        InitSysAutoEvt( hostMgr->sysAutoEvt + i );
        NotifySysAutoEvt( hostMgr->sysAutoEvt + i );
    }

    param.result = DS_OK;
    if( hostCount > 0 ){
        param.cs.entry = firstEntry;
        param.cs.context = context;
        firstEntry = MainHostFirstEntry;
        context = (intptr_t)&param;
    }
    ret = RunHost( hostMgr, 0, firstEntry, context, cleanEntry );
    while( hostMgr->workerPool ){
        worker = MEMBER_BASE( hostMgr->workerPool, DzWorker, lItr );
        hostMgr->workerPool = hostMgr->workerPool->next;
        worker->dzCot = NULL;
        NotifySysAutoEvt( &worker->sysEvt );
    }
    for( i = 0; i < hostCount; i++ ){
        FreeSysAutoEvt( hostMgr->sysAutoEvt + i );
    }
    FreeTlsIndex();
    return ret == DS_OK ? param.result : ret;
}

inline int GetCotCount( DzHost* host )
{
    return host->cotCount;
}

inline int GetHostId( DzHost* host )
{
    return host->hostId;
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

inline int SetCotPoolDepth( DzHost* host, int sType, int depth )
{
    int deta;
    int ret;

    ret = host->cotPoolSetDepth[ sType ];
    if( depth >= 0 && host->cotStackSize[ sType ] >= DZ_MIN_PAGE_STACK_SIZE ){
        deta = depth - host->cotPoolSetDepth[ sType ];
        host->cotPoolNowDepth[ sType ] += deta;
        host->cotPoolSetDepth[ sType ] = depth;
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
    int         dftSType
    )
{
    int ret;

    ret = host->lowestPri;
    if( lowestPri > host->lowestPri ){
        host->lowestPri = lowestPri;
    }
    if( dftPri >= CP_FIRST ){
        host->dftPri = dftPri;
    }
    if( dftSType >= ST_FIRST ){
        host->dftSType = dftSType;
        if( host->cotPoolSetDepth[ dftSType ] < DFT_SSIZE_POOL_DEPTH ){
            SetCotPoolDepth( host, dftSType, DFT_SSIZE_POOL_DEPTH );
        }
    }
    return ret;
}

inline int SetHostIoReaction( DzHost* host, int rate )
{
    int ret = host->ioReactionRate;
    if( rate > 0 ){
        host->ioReactionRate = rate;
    }
    return ret;
}

inline void* Malloc( DzHost* host, size_t size )
{
    void* ret;

    ret = mspace_malloc( host->mSpace, size );
    __Dbg( AllocHeap )( host, ret, size );
    return ret;
}

inline void* Calloc( DzHost* host, size_t num, size_t size )
{
    void* ret;

    ret = mspace_calloc( host->mSpace, num, size );
    __Dbg( AllocHeap )( host, ret, size * num );
    return ret;
}

inline void* ReAlloc( DzHost* host, void* mem, size_t size )
{
    return mspace_realloc( host->mSpace, mem, size );
}

inline void Free( DzHost* host, void* mem )
{
    __Dbg( FreeHeap )( host, mem );
    mspace_free( host->mSpace, mem );
}

inline void* MallocEx( DzHost* host, size_t size )
{
    DzMemExTag* ret;

    ret = (DzMemExTag*)Malloc( host, size + sizeof( int64 ) );
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
        currTime = MilUnixTime( host );
        cmpTime = currTime + MIN_TIME_INTERVAL;
        while( GetMinTimerNode( host )->timestamp <= cmpTime ){
            timerNode = GetMinTimerNode( host );
            if( timerNode->repeat ){
                //for timerNode->interval is a negative value, use minus
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

inline void DealRmtFifo( DzHost* host, DzRmtCotFifo* fifo )
{
    int readPos;
    int writePos;
    DzCot* dzCot;
    DzLItr* nextItr;

    readPos = AtomReadInt( fifo->readPos );
    writePos = AtomReadInt( fifo->writePos );
    while( readPos != writePos ){
        dzCot = fifo->rmtCotArr[ readPos ];
        readPos++;
        if( readPos == RMT_CALL_FIFO_SIZE ){
            readPos = 0;
        }
        AtomSetInt( fifo->readPos, readPos );
        if( dzCot->priority >= 0 ){
            host->cotCount++;
            if( dzCot->priority > host->lowestPri ){
                dzCot->priority = host->lowestPri;
            }
            DispatchCot( host, dzCot );
        }else{
            dzCot->priority += CP_DEFAULT + 1;
            nextItr = &dzCot->lItr;
            do{
                dzCot = MEMBER_BASE( nextItr, DzCot, lItr );
                nextItr = dzCot->lItr.next;
                host->cotCount++;
                if( dzCot->priority > host->lowestPri ){
                    dzCot->priority = host->lowestPri;
                }
                DispatchCot( host, dzCot );
            }while( nextItr );
        }
    }
}

inline int DispatchRmtCots( DzHost* host, int timeout )
{
    int idx;
    u_int sign;

    if( timeout ){
        sign = (u_int)AtomCasInt( host->rmtCheckSignPtr, RMT_CHECK_AWAKE_SIGN, 0 );
        if( sign == RMT_CHECK_AWAKE_SIGN ){
            return timeout;
        }
    }
    
    idx = 0;
    sign = (u_int)AtomAndInt( host->rmtCheckSignPtr, RMT_CHECK_AWAKE_SIGN );
    sign &= ~RMT_CHECK_AWAKE_SIGN;
    while( sign ){
        if( sign & 1 ){
            DealRmtFifo( host, host->rmtFifoArr + idx );
        }
        sign >>= 1;
        idx++;
    }
    return 0;
}

#ifdef __cplusplus
};
#endif

#endif // __DzCore_h__
