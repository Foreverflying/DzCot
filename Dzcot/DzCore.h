/********************************************************************
    created:    2010/02/11 21:49
    file:       DzCore.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzCore_h__
#define __DzCore_h__

#include "DzType.h"
#include "DzStructs.h"
#include "DzBaseOs.h"
#include "DzResourceMgr.h"
#include "DzSchedule.h"
#include "DzIoOs.h"
#include <assert.h>

#ifdef __cplusplus
extern "C"{
#endif

inline DzThread* AllocDzThread( DzHost* host, int sSize )
{
    DzThread* dzThread;
    DzLItr* head;

    head = &host->threadPools[ sSize ];
    if( !head->next ){
        if( !AllocDzThreadPool( host, sSize ) ){
            return NULL;
        }
    }

    dzThread = MEMBER_BASE( head->next, DzThread, lItr );
    PopSList( head );
    return InitCot( host, dzThread, sSize );
}

inline void FreeDzThread( DzHost* host, DzThread* dzThread )
{
    PushSList( &host->threadPools[ dzThread->stackSize ], &dzThread->lItr );
    //DeCommitStack( dzThread->stack, dzThread->stackLimit );
    //dzThread->stackLimit = NULL;
}

// StartCot:
// create a new co thread
inline int StartCot(
    DzHost*     host,
    DzRoutine   entry,
    void*       context,
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
// create a new co thread
inline int StartCotInstant(
    DzHost*     host,
    DzRoutine   entry,
    void*       context,
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

// RunHost:
// Initial and start the cot host
// the function will block while there are cots running
// after all cots exit, the host will stop and the block ends
inline int RunHost(
    int         lowestPriority,
    int         defaultPri,
    int         defaultSSize,
    DzRoutine   firstEntry,
    void*       context,
    int         priority,
    int         sSize
    )
{
    int ret = DS_OK;
    DzHost host;
    int i;

    InitTlsIndex();

    host.currThread = &host.originThread;
    host.lowestPriority = lowestPriority;
    host.currPriority = lowestPriority + 1;
    for( i=0; i<=lowestPriority; i++ ){
        InitSList( &host.taskLs[i] );
    }
    //host.originThread.priority = CP_FIRST;
    host.lNodePool.next = NULL;
    host.poolDepth = 0;
    for( i=0; i<=STACK_SIZE_COUNT; i++ ){
        host.threadPools[i].next = NULL;
    }
    host.threadCount = 0;
    host.maxThreadCount = 0;
    host.timerCount = 0;
    host.timerHeapSize = 0;
    host.timerHeap = (DzTimerNode**)PageReserv( sizeof(DzTimerNode*) * TIME_HEAP_SIZE );
    host.poolGrowList.next = NULL;
    host.memPoolPos = NULL;
    host.memPoolEnd = NULL;
    host.synObjPool.next = NULL;
    host.asynIoPool.next = NULL;
    host.defaultPri = defaultPri == CP_DEFAULT ? host.lowestPriority : defaultPri;
    host.defaultSSize = defaultSSize;

    InitOsStruct( &host );
    SetHost( &host );

    ret = StartCot( &host, firstEntry, context, priority, sSize );
    Schedule( &host );
    IoMgrRoutine( &host );
    PageFree( host.timerHeap, sizeof(DzTimerNode*) * TIME_HEAP_SIZE );
    ReleaseMemoryPool( &host );
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

inline void InitCotPool( DzHost* host, uint count, uint depth, int sSize )
{
    DzThread* head;
    DzThread* thd;
    DzLItr* lItr;
    uint i;

    if( sSize == SS_DEFAULT ){
        sSize = host->defaultSSize;
    }

    head = AllocDzThread( host, sSize );
    lItr = &head->lItr;
    for( i=1; i<count; i++ ){
        thd = AllocDzThread( host, sSize );
        if( !thd ){
            break;
        }
        lItr->next = &thd->lItr;
        lItr = lItr->next;
    }
    lItr->next = NULL;
    lItr = &head->lItr;
    while( lItr ){
        head = MEMBER_BASE( lItr, DzThread, lItr );
        lItr = lItr->next;
        FreeDzThread( host, head );
    }
}

#ifdef __cplusplus
};
#endif

#endif // __DzCore_h__
