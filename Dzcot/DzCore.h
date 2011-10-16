/********************************************************************
    created:    2010/02/11 21:49
    file:       DzCore.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzCore_h__
#define __DzCore_h__

#include "DzStructs.h"
#include "DzBaseOs.h"
#include "DzResourceMgr.h"
#include "DzSchedule.h"
#include "DzCoreOs.h"
#include "DzRmtCore.h"
#include "dlmalloc.h"

#ifdef __cplusplus
extern "C"{
#endif

void __stdcall DelayFreeTheadEntry( intptr_t context );
void __stdcall EventNotifyCotEntry( intptr_t context );

inline void ReleaseAllPoolStack( DzHost* host )
{
    DzLItr* lItr;
    DzThread* dzThread;
    int i;

    for( i = DZ_MAX_PERSIST_STACK_SIZE + 1; i <= host->lowestPriority; i++ ){
        lItr = host->cotPools[i];
        while( lItr ){
            dzThread = MEMBER_BASE( lItr, DzThread, lItr );
            FreeCotStack( dzThread );
            lItr = lItr->next;
        }
    }
}

inline DzThread* AllocDzThread( DzHost* host, int sSize )
{
    DzThread* dzThread;

    if( host->cotPools[ sSize ] ){
        dzThread = MEMBER_BASE( host->cotPools[ sSize ], DzThread, lItr );
        host->cotPools[ sSize ] = host->cotPools[ sSize ]->next;
        host->cotPoolNowDepth[ sSize ] ++;
    }else{
        if( !host->threadPool ){
            if( !AllocDzThreadPool( host ) ){
                return NULL;
            }
        }
        dzThread = MEMBER_BASE( host->threadPool, DzThread, lItr );
        if( InitCot( host, dzThread, sSize ) ){
            host->threadPool = host->threadPool->next;
        }else{
            return NULL;
        }
    }
    return dzThread;
}

inline void FreeDzThread( DzHost* host, DzThread* dzThread )
{
    DzThread* tmp;

    if( host->cotPoolNowDepth[ dzThread->sSize ] > 0 ){
        dzThread->lItr.next = host->cotPools[ dzThread->sSize ];
        host->cotPools[ dzThread->sSize ] = &dzThread->lItr;
        host->cotPoolNowDepth[ dzThread->sSize ] --;
    }else{
        //can not FreeCotStack( dzThread ) here!!
        //the stack is still in use before switch
        if( host->cotPools[ dzThread->sSize ] ){
            //if the cotPool is not empty swap and free the head
            tmp = MEMBER_BASE( host->cotPools[ dzThread->sSize ], DzThread, lItr );
            dzThread->lItr.next = tmp->lItr.next;
            host->cotPools[ dzThread->sSize ] = &dzThread->lItr;
            FreeCotStack( tmp );
            tmp->lItr.next = host->threadPool;
            host->threadPool = &tmp->lItr;
        }else{
            //start a cot to free it later
            StartCot( host, DelayFreeTheadEntry, (intptr_t)dzThread, host->lowestPriority, SS_FIRST );
        }
    }
}

// StartCot:
// create a new co thread
inline int StartCot(
    DzHost*     host,
    DzRoutine   entry,
    intptr_t    context,
    int         priority,
    int         sSize
    )
{
    DzThread* dzThread;

    if( sSize == SS_DEFAULT ){
        sSize = host->defaultSSize;
    }

    dzThread = AllocDzThread( host, sSize );
    if( !dzThread ){
        return DS_NO_MEMORY;
    }

    host->threadCount++;
    dzThread->priority = priority == CP_DEFAULT ? host->defaultPri : priority;
    SetThreadEntry( dzThread, entry, context );

    if( dzThread->priority < host->currPriority ){
        host->currPriority = dzThread->priority;
    }
    DispatchThread( host, dzThread );
    return DS_OK;
}

// StartCotInstant:
// create a new co thread and run it at once
// current co thread is paused and will continue at next schedule
inline int StartCotInstant(
    DzHost*     host,
    DzRoutine   entry,
    intptr_t    context,
    int         priority,
    int         sSize
    )
{
    DzThread* dzThread;

    if( sSize == SS_DEFAULT ){
        sSize = host->defaultSSize;
    }

    dzThread = AllocDzThread( host, sSize );
    if( !dzThread ){
        return DS_NO_MEMORY;
    }

    host->threadCount++;
    dzThread->priority = priority == CP_DEFAULT ? host->defaultPri : priority;
    SetThreadEntry( dzThread, entry, context );

    TemporaryPushThread( host, host->currThread );
    SwitchToCot( host, dzThread );
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
    int ret;
    DzLNode* node;

    node = AllocLNode( host );
    node->content = (intptr_t)entry;
    node->context1 = (intptr_t)context;
    node->context2 = (intptr_t)CloneSynObj( evt );
    ret = StartCot( host, EventNotifyCotEntry, (intptr_t)node, priority, sSize );
    if( ret != DS_OK ){
        CloseSynObj( host, (DzSynObj*)node->context2 );
        FreeLNode( host, node );
    }
    return ret;
}

// EvtStartCotInstant:
// equal to StartCotInstant, notify an manual event SynObj when cot finished
inline int EvtStartCotInstant(
    DzHost*     host,
    DzSynObj*   evt,
    DzRoutine   entry,
    void*       context,
    int         priority,
    int         sSize
    )
{
    int ret;
    DzLNode* node;

    node = AllocLNode( host );
    node->content = (intptr_t)entry;
    node->context1 = (intptr_t)context;
    node->context2 = (intptr_t)CloneSynObj( evt );
    ret = StartCot( host, EventNotifyCotEntry, (intptr_t)node, priority, sSize );
    if( ret != DS_OK ){
        CloseSynObj( host, (DzSynObj*)node->context2 );
        FreeLNode( host, node );
    }
    return ret;
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
    DzRmtCallPkg pkg;

    pkg.entry = entry;
    pkg.context = context;
    pkg.priority = priority;
    pkg.sSize = sSize;
    pkg.evt = NULL;
    SendRmtCall( host, rmtId, FALSE, &pkg );
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
    DzRmtCallPkg pkg;

    pkg.entry = entry;
    pkg.context = context;
    pkg.priority = priority;
    pkg.sSize = sSize;
    pkg.evt = CloneSynObj( evt );
    pkg.hostId = host->hostId;
    SendRmtCall( host, rmtId, FALSE, &pkg );
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
    DzSynObj* evt;
    DzRmtCallPkg pkg;

    evt = CreateManualEvt( host, FALSE );
    pkg.entry = entry;
    pkg.context = context;
    pkg.priority = priority;
    pkg.sSize = sSize;
    pkg.evt = CloneSynObj( evt );
    pkg.hostId = host->hostId;
    SendRmtCall( host, rmtId, FALSE, &pkg );
    WaitSynObj( host, pkg.evt, -1 );
    CloseSynObj( host, pkg.evt );
    return DS_OK;
}

// RunHost:
// Initial and start the cot host
// the function will block while there are cots running
// after all cots exit, the host will stop and the block ends
inline int RunHost(
    DzHostsMgr* hostMgr,
    int         hostId,
    int         lowestPriority,
    int         defaultPri,
    int         defaultSSize,
    DzRoutine   firstEntry,
    intptr_t    context
    )
{
    int i;
    int ret;
    void* mallocSpace;
    DzTimerNode** timerHeap;
    DzHost host;

    timerHeap = (DzTimerNode**)PageReserv( sizeof(DzTimerNode*) * TIME_HEAP_SIZE );
    if( !timerHeap ){
        PageFree( timerHeap, sizeof(DzTimerNode*) * TIME_HEAP_SIZE );
        return DS_NO_MEMORY;
    }
    mallocSpace = create_mspace( 0, 0 );
    if( !mallocSpace ){
        destroy_mspace( mallocSpace );
        PageFree( timerHeap, sizeof(DzTimerNode*) * TIME_HEAP_SIZE );
        return DS_NO_MEMORY;
    }

    host.originThread.sSize = SS_DEFAULT;
    host.currThread = &host.originThread;
    host.lowestPriority = lowestPriority;
    host.currPriority = lowestPriority + 1;
    for( i = 0; i <= lowestPriority; i++ ){
        InitSList( &host.taskLs[i] );
    }
    host.threadPool = NULL;
    for( i = SS_FIRST; i <= DZ_MAX_PERSIST_STACK_SIZE; i++ ){
        host.cotPools[i] = NULL;
        host.cotPoolNowDepth[i] = DZ_MAX_COT_POOL_DEPTH;
    }
    for( ; i < STACK_SIZE_COUNT; i++ ){
        host.cotPools[i] = NULL;
        host.cotPoolNowDepth[i] = 0;
    }
    host.threadCount = 0;
    host.hostId = hostId;
    host.timerCount = 0;
    host.timerHeapSize = 0;
    host.timerHeap = timerHeap;
    host.defaultPri = defaultPri;
    host.defaultSSize = defaultSSize;
    host.mallocSpace = mallocSpace;
    host.synObjPool = NULL;
    host.lNodePool = NULL;
    host.hostMgr = hostMgr;
    host.checkRmtSign = 0;
    host.hostMask = 1 << hostId;
    host.checkFifo = NULL;
    host.rmtFifoArr = hostMgr->rmtFifoRes + hostMgr->hostCount * hostId;
    host.pendingPkgs = hostMgr->pendingPkgRes + hostMgr->hostCount * hostId;
    host.memPoolPos = NULL;
    host.memPoolEnd = NULL;
    host.poolGrowList = NULL;
    for( i = 0; i < STACK_SIZE_COUNT; i++ ){
        host.cotPoolSetDepth[i] = 0;
    }
    if( defaultSSize > DZ_MAX_PERSIST_STACK_SIZE ){
        host.cotPoolNowDepth[ defaultSSize ] = DFT_SSIZE_POOL_DEPTH;
        host.cotPoolSetDepth[ defaultSSize ] = DFT_SSIZE_POOL_DEPTH;
    }

    if( InitOsStruct( &host, hostMgr->hostArr[0] ) ){
        AtomOrInt( &host.hostMgr->exitSign, 1 << hostId );
        SetHost( &host );
        host.hostMgr->hostArr[ hostId ] = &host;

        ret = StartCot( &host, firstEntry, context, defaultPri, defaultSSize );
        if( ret == DS_OK ){
            Schedule( &host );
        }
        CotScheduleCenter( &host );

        //after all cot finished, IoMgrRoutine will return.
        //so cleanup the host struct
        host.hostMgr->hostArr[ hostId ] = NULL;
        SetHost( NULL );
        DeleteOsStruct( &host, hostMgr->hostArr[0] );
    }else{
        ret = DS_NO_MEMORY;
    }

    ReleaseAllPoolStack( &host );
    PageFree( host.timerHeap, sizeof(DzTimerNode*) * TIME_HEAP_SIZE );
    ReleaseMemoryPool( &host );
    destroy_mspace( host.mallocSpace );

    return ret;
}

inline int RunHosts(
    int         hostCount,
    int*        servMask,
    int         lowestPriority,
    int         defaultPri,
    int         defaultSSize,
    DzRoutine   firstEntry,
    intptr_t    context
    )
{
    int i;
    int ret;
    DzSysParam param;
    DzHostsMgr hostMgr;
    DzHostsMgr* hostMgrPtr;

    if( !AllocTlsIndex() ){
        return DS_NO_MEMORY;
    }
    hostMgr.hostArr = (DzHost**)
        alloca( sizeof( DzHost* ) * hostCount );
    for( i = 0; i < hostCount; i++ ){
        hostMgr.hostArr[i] = NULL;
    }
    hostMgr.rmtFifoRes = (DzRmtCallFifo*)
        alloca( sizeof( DzRmtCallFifo ) * hostCount * hostCount );
    for( i = 0; i < hostCount * hostCount; i++ ){
        hostMgr.rmtFifoRes[i].readPos = 0;
        hostMgr.rmtFifoRes[i].writePos = 0;
        hostMgr.rmtFifoRes[i].callPkgArr = NULL;
    }
    hostMgr.pendingPkgRes = (DzSList*)
        alloca( sizeof( DzSList ) * hostCount * hostCount );
    for( i = 0; i < hostCount * hostCount; i++ ){
        InitSList( hostMgr.pendingPkgRes + i );
    }
    hostMgrPtr = (DzHostsMgr*)alloca( sizeof( DzHostsMgr ) );
    hostMgrPtr->hostArr = hostMgr.hostArr;
    hostMgrPtr->rmtFifoRes = hostMgr.rmtFifoRes;
    hostMgrPtr->pendingPkgRes = hostMgr.pendingPkgRes;
    hostMgrPtr->servMask = servMask;
    hostMgrPtr->hostCount = hostCount;
    hostMgrPtr->exitSign = 0;

    param.result = DS_OK;
    if( hostCount > 1 ){
        param.cotStart.entry = firstEntry;
        param.cotStart.context = context;
        firstEntry = MainHostEntry;
        context = (intptr_t)&param;
    }
    ret = RunHost(
        hostMgrPtr, 0, lowestPriority, defaultPri, defaultSSize,
        firstEntry, context
        );
    FreeTlsIndex();
    return ret == DS_OK ? param.result : ret;;
}

inline int GetCotCount( DzHost* host )
{
    return host->threadCount;
}

inline int SetCurrCotPriority( DzHost* host, int priority )
{
    int ret;

    ret = host->currThread->priority;
    if( priority >= CP_FIRST ){
        host->currThread->priority = priority;
        if( priority < ret ){
            host->currPriority = priority;
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

inline int SetHostParam(
    DzHost*     host,
    int         lowestPriority,
    int         defaultPri,
    int         defaultSSize
    )
{
    int ret;

    ret = host->lowestPriority;
    if( lowestPriority > host->lowestPriority ){
        host->lowestPriority = lowestPriority;
    }
    host->defaultPri = defaultPri;
    host->defaultSSize = defaultSSize;
    if( host->cotPoolSetDepth[ defaultSSize ] < DFT_SSIZE_POOL_DEPTH ){
        SetCotPoolDepth( host, defaultSSize, DFT_SSIZE_POOL_DEPTH );
    }

    return ret;
}

inline void* Malloc( DzHost* host, size_t size )
{
    return mspace_malloc( host->mallocSpace, size );
}

inline void* Calloc( DzHost* host, size_t num, size_t size )
{
    return mspace_calloc( host->mallocSpace, num, size );
}

inline void* ReAlloc( DzHost* host, void* mem, size_t size )
{
    return mspace_realloc( host->mallocSpace, mem, size );
}

inline void Free( DzHost* host, void* mem )
{
    mspace_free( host->mallocSpace, mem );
}

#ifdef __cplusplus
};
#endif

#endif // __DzCore_h__
