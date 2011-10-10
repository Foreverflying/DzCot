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
#include "DzIoOs.h"
#include "dlmalloc.h"

#ifdef __cplusplus
extern "C"{
#endif

void __stdcall DelayFreeTheadRoutine( intptr_t context );
void __stdcall EventNotifyCotRoutine( intptr_t context );

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
        host->cotPoolDepth[ sSize ] ++;
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

    if( host->cotPoolDepth[ dzThread->sSize ] > 0 ){
        dzThread->lItr.next = host->cotPools[ dzThread->sSize ];
        host->cotPools[ dzThread->sSize ] = &dzThread->lItr;
        host->cotPoolDepth[ dzThread->sSize ] --;
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
            StartCot( host, DelayFreeTheadRoutine, (intptr_t)dzThread, host->lowestPriority, SS_FIRST );
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
    if( host->threadCount > host->maxThreadCount ){
        host->maxThreadCount++;
    }
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
    if( host->threadCount > host->maxThreadCount ){
        host->maxThreadCount++;
    }
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
    ret = StartCot( host, EventNotifyCotRoutine, (intptr_t)node, priority, sSize );
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
    ret = StartCot( host, EventNotifyCotRoutine, (intptr_t)node, priority, sSize );
    if( ret != DS_OK ){
        CloseSynObj( host, (DzSynObj*)node->context2 );
        FreeLNode( host, node );
    }
    return ret;
}

// RunHost:
// Initial and start the cot host
// the function will block while there are cots running
// after all cots exit, the host will stop and the block ends
inline int RunHost(
    DzHost*     parentHost,
    int         lowestPriority,
    int         defaultPri,
    int         defaultSSize,
    DzRoutine   firstEntry,
    intptr_t    context,
    int         priority,
    int         sSize
    )
{
    int i;
    DzHost host;
    int ret;
    BOOL tlsOk;
    void* mallocSpace;
    DzTimerNode** timerHeap;

    tlsOk = parentHost ? TRUE : AllocTlsIndex();
    timerHeap = (DzTimerNode**)PageReserv( sizeof(DzTimerNode*) * TIME_HEAP_SIZE );
    mallocSpace = create_mspace( 0, 0 );

    if( ! ( tlsOk && timerHeap && mallocSpace ) ){
        if( mallocSpace ){
            destroy_mspace( mallocSpace );
        }
        if( timerHeap ){
            PageFree( timerHeap, sizeof(DzTimerNode*) * TIME_HEAP_SIZE );
        }
        if( tlsOk && !parentHost ){
            FreeTlsIndex();
        }
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
        host.cotPoolDepth[i] = DZ_MAX_COT_POOL_DEPTH;
    }
    for( ; i < STACK_SIZE_COUNT; i++ ){
        host.cotPools[i] = NULL;
        host.cotPoolDepth[i] = 0;
    }
    host.threadCount = 0;
    host.maxThreadCount = 0;
    host.timerCount = 0;
    host.timerHeapSize = 0;
    host.timerHeap = timerHeap;
    host.mallocSpace = mallocSpace;
    host.synObjPool = NULL;
    host.lNodePool = NULL;
    host.poolGrowList = NULL;
    host.memPoolPos = NULL;
    host.memPoolEnd = NULL;
    host.defaultPri = defaultPri == CP_DEFAULT ? host.lowestPriority : defaultPri;
    host.defaultSSize = defaultSSize;

    SetHost( &host );
    if( InitOsStruct( &host, parentHost ) ){
        ret = StartCot( &host, firstEntry, context, priority, sSize );
        if( ret == DS_OK ){
            Schedule( &host );
        }
        IoMgrRoutine( &host );

        //after all cot finished, IoMgrRoutine will return.
        //so cleanup the host struct
        DeleteOsStruct( &host, parentHost );
    }else{
        ret = DS_NO_MEMORY;
    }

    ReleaseAllPoolStack( &host );
    PageFree( host.timerHeap, sizeof(DzTimerNode*) * TIME_HEAP_SIZE );
    ReleaseMemoryPool( &host );
    destroy_mspace( host.mallocSpace );
    SetHost( NULL );
    if( !parentHost ){
        FreeTlsIndex();
    }

    return ret;
}

inline int SetCurrCotPriority( DzHost* host, int priority )
{
    int ret;

    ret = host->currThread->priority;
    host->currThread->priority = priority;
    if( priority < ret ){
        host->currPriority = priority;
    }
    return ret;
}

inline BOOL GrowCotPoolDepth( DzHost* host, int sSize, int deta )
{
    host->cotPoolDepth[ sSize ] += deta;
    return TRUE;
}

inline int GetCotCount( DzHost* host )
{
    return host->threadCount;
}

inline int GetMaxCotCount( DzHost* host, BOOL reset )
{
    int ret;

    ret = host->maxThreadCount;
    if( reset ){
        host->maxThreadCount = host->threadCount;
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
