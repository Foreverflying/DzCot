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

void __stdcall DelayFreeCotHelper( intptr_t context );
void __stdcall EventNotifyCotEntry( intptr_t context );

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
    DzRmtCotParam* param;
    DzCot* dzCot;

    dzCot = AllocDzCot( host, sSize );
    if( !dzCot ){
        return DS_NO_MEMORY;
    }
    param = (DzRmtCotParam*)AllocLNode( host );
    param->hostId = host->hostId;
    param->entry = entry;
    param->context = context;
    param->type = 0;
    dzCot->priority = priority;
    SetCotEntry( dzCot, RemoteCotEntry, (intptr_t)param );
    DispatchRmtCot( host, rmtId, FALSE, dzCot );
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
    DzRmtCotParam* param;
    DzCot* dzCot;

    dzCot = AllocDzCot( host, sSize );
    if( !dzCot ){
        return DS_NO_MEMORY;
    }
    param = (DzRmtCotParam*)AllocLNode( host );
    param->hostId = host->hostId;
    param->entry = entry;
    param->context = context;
    param->type = 1;
    param->evtType = 0;
    param->evt = CloneSynObj( evt );
    dzCot->priority = priority;
    SetCotEntry( dzCot, RemoteCotEntry, (intptr_t)param );
    DispatchRmtCot( host, rmtId, FALSE, dzCot );
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
    DzRmtCotParam* param;
    DzCot* dzCot;
    DzEasyEvt easyEvt;

    dzCot = AllocDzCot( host, sSize );
    if( !dzCot ){
        return DS_NO_MEMORY;
    }
    param = (DzRmtCotParam*)AllocLNode( host );
    param->hostId = host->hostId;
    param->entry = entry;
    param->context = context;
    param->type = 1;
    param->evtType = 1;
    param->easyEvt = &easyEvt;
    dzCot->priority = priority;
    SetCotEntry( dzCot, RemoteCotEntry, (intptr_t)param );
    DispatchRmtCot( host, rmtId, FALSE, dzCot );
    WaitEasyEvt( host, &easyEvt );
    return DS_OK;
}

// RunHost:
// Initial and start the cot host
// the function will block while there are cots running
// after all cots exit, the host will stop and the block ends
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

    host.currCot = &host.centerCot;
    host.lowestPri = lowestPri;
    host.currPri = lowestPri + 1;
    for( i = 0; i <= lowestPri; i++ ){
        InitSList( &host.taskLs[i] );
    }
    host.hostId = hostId;
    host.checkRmtSign = 0;
    host.hostMask = 1 << hostId;
    host.cotCount = 0;
    host.scheduleCd = SCHEDULE_COUNTDOWN;
    host.cotPool = NULL;
    for( i = SS_FIRST; i <= DZ_MAX_PERSIST_STACK_SIZE; i++ ){
        host.cotPools[i] = NULL;
        host.cotPoolNowDepth[i] = DZ_MAX_COT_POOL_DEPTH;
    }
    for( ; i < STACK_SIZE_COUNT; i++ ){
        host.cotPools[i] = NULL;
        host.cotPoolNowDepth[i] = 0;
    }
    host.timerCount = 0;
    host.timerHeapSize = 0;
    host.timerHeap = timerHeap;
    host.dftPri = dftPri;
    host.dftSSize = dftSSize;
    host.mallocSpace = mallocSpace;
    host.synObjPool = NULL;
    host.lNodePool = NULL;
    host.hostMgr = hostMgr;
    host.checkFifo = NULL;
    host.rmtFifoArr = hostMgr->rmtFifoRes + hostMgr->hostCount * hostId;
    host.pendRmtCot = hostMgr->pendRmtCotRes + hostMgr->hostCount * hostId;
    host.memPoolPos = NULL;
    host.memPoolEnd = NULL;
    host.poolGrowList = NULL;
    for( i = 0; i < STACK_SIZE_COUNT; i++ ){
        host.cotPoolSetDepth[i] = 0;
    }
    if( dftSSize > DZ_MAX_PERSIST_STACK_SIZE ){
        host.cotPoolNowDepth[ dftSSize ] = DFT_SSIZE_POOL_DEPTH;
        host.cotPoolSetDepth[ dftSSize ] = DFT_SSIZE_POOL_DEPTH;
    }

    if( InitOsStruct( &host, hostMgr->hostArr[0] ) ){
        AtomOrInt( &host.hostMgr->exitSign, 1 << hostId );
        SetHost( &host );
        host.hostMgr->hostArr[ hostId ] = &host;

        ret = StartCot( &host, firstEntry, context, dftPri, dftSSize );
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
    hostMgr.rmtFifoRes = (DzRmtCotFifo*)
        alloca( sizeof( DzRmtCotFifo ) * hostCount * hostCount );
    for( i = 0; i < hostCount * hostCount; i++ ){
        hostMgr.rmtFifoRes[i].readPos = 0;
        hostMgr.rmtFifoRes[i].writePos = 0;
        hostMgr.rmtFifoRes[i].rmtCotArr = NULL;
    }
    hostMgr.pendRmtCotRes = (DzSList*)
        alloca( sizeof( DzSList ) * hostCount * hostCount );
    for( i = 0; i < hostCount * hostCount; i++ ){
        InitSList( hostMgr.pendRmtCotRes + i );
    }
    hostMgrPtr = (DzHostsMgr*)alloca( sizeof( DzHostsMgr ) );
    hostMgrPtr->hostArr = hostMgr.hostArr;
    hostMgrPtr->rmtFifoRes = hostMgr.rmtFifoRes;
    hostMgrPtr->pendRmtCotRes = hostMgr.pendRmtCotRes;
    hostMgrPtr->servMask = servMask;
    hostMgrPtr->hostCount = hostCount;
    hostMgrPtr->exitSign = 0;

    for( i = 0; i < hostCount; i++ ){
        for( ret = 0; ret < hostCount; ret++ ){
            if( servMask[i] & ( 1 << ret ) ){
                servMask[ret] |= ( 1 << i );
            }
        }
    }
    param.result = DS_OK;
    if( hostCount > 1 ){
        param.cotStart.entry = firstEntry;
        param.cotStart.context = context;
        firstEntry = MainHostEntry;
        context = (intptr_t)&param;
    }
    ret = RunHost(
        hostMgrPtr, 0, lowestPri, dftPri, dftSSize,
        firstEntry, context
        );
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
    int writePos;
    DzCot* dzCot;

    if( timeout ){
        nowCount = AtomCasInt( &host->checkRmtSign, 0, RMT_CHECK_SLEEP_SIGN );
    }else{
        nowCount = AtomReadInt( &host->checkRmtSign );
    }
    if( nowCount == 0 ){
        return timeout;
    }
    count = nowCount;
    host->scheduleCd = 0;
    while( 1 ){
        writePos = AtomReadInt( &host->checkFifo->writePos );
        while( host->checkFifo->readPos != writePos ){
            dzCot = host->checkFifo->rmtCotArr[ host->checkFifo->readPos ];
            DealRmtCot( host, dzCot );
            host->checkFifo->readPos++;
            if( host->checkFifo->readPos == RMT_CALL_FIFO_SIZE ){
                host->checkFifo->readPos = 0;
            }
            nowCount--;
            if( nowCount == 0 ){
                nowCount = AtomSubInt( &host->checkRmtSign, count );
                nowCount -= count;
                if( nowCount == 0 ){
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
