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
#include "DzResourceMgr.h"
#include "DzSchedule.h"
#include "DzCoreOs.h"
#include "DzIoOs.h"
#include <assert.h>

#ifdef __cplusplus
extern "C"{
#endif

// InitHost:
// create struct needed
inline int InitHost(
    int         lowestPriority,
    int         defaultPri,
    int         defaultSSize
    )
{
    DzHost *host;
    int i;

    InitTlsIndex();
    host = (DzHost*)malloc( sizeof( DzHost ) );
    if( !host ){
        return DS_NO_MEMORY;
    }

    host->currThread = NULL;
    host->lowestPriority = lowestPriority;
    host->currPriority = lowestPriority + 1;
    for( i=0; i<=lowestPriority; i++ ){
        InitQueue( &host->taskQs[i] );
    }
    host->originThread.priority = CP_FIRST;
    host->qNodePool.next = NULL;
    host->taskCheckTag = 0;
    host->poolDepth = 0;
    for( i=0; i<=STACK_SIZE_COUNT; i++ ){
        host->threadPools[i].next = NULL;
        host->poolCotCounts[i] = 0;
    }
    host->threadCount = 0;
    host->maxThreadCount = 0;
    host->timerCount = 0;
    host->timerHeapSize = 0;
    host->timerHeap = NULL;
    host->mallocList.next = NULL;
    host->mallocCount = 0;
    host->synObjPool.next = NULL;
    host->asynIoPool.next = NULL;
    host->originExceptPtr = GetExceptPtr();
    host->defaultPri = defaultPri == CP_DEFAULT ? host->lowestPriority : defaultPri;
    host->defaultSSize = defaultSSize;
    host->isExiting = FALSE;
    host->isBlocking = FALSE;

    InitOsAppend( host );
    SetHost( host );
    return DS_OK;
}

// StartCot:
// create a new co thread
inline int StartCot(
    DzHost*     host,
    DzRoutine   entry,
    void        *context,
    int         priority,
    int         sSize
    )
{
    DzThread *dzThread;

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
    void        *context,
    int         priority,
    int         sSize
    )
{
    DzThread *dzThread;

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

// StartHost:
// create the Io mgr co thread, so the host can serve requests
inline int StartHost(
    DzHost*     host,
    DzRoutine   firstEntry,
    void*       context,
    int         priority,
    int         sSize
    )
{
    int ret = DS_OK;
    host->currThread = &host->originThread;

    InitIoMgr( host );
    if( firstEntry ){
        ret = StartCot( host, firstEntry, context, priority, sSize );
        Schedule( host );
    }
    IoMgrRoutine( host, TRUE );
    return ret;
}

inline int SetCurrCotPriority( DzHost *host, int priority )
{
    int ret;

    ret = host->currThread->priority;
    host->currThread->priority = priority;
    if( priority < ret ){
        host->currPriority = priority;
    }
    return ret;
}

inline int GetCotCount( DzHost *host )
{
    return host->threadCount;
}

inline int GetMaxCotCount( DzHost *host, BOOL reset )
{
    int ret;

    ret = host->maxThreadCount;
    if( reset ){
        host->maxThreadCount = host->threadCount;
    }
    return ret;
}

inline void InitCotPool( DzHost *host, u_int count, u_int depth, int sSize )
{
    DzThread *head;
    DzThread *thd;
    DzQItr *qItr;
    u_int i;

    if( sSize == SS_DEFAULT ){
        sSize = host->defaultSSize;
    }

    head = AllocDzThread( host, sSize );
    qItr = &head->qItr;
    for( i=1; i<count; i++ ){
        thd = AllocDzThread( host, sSize );
        if( !thd ){
            break;
        }
        qItr->next = &thd->qItr;
        qItr = qItr->next;
    }
    qItr->next = NULL;
    qItr = &head->qItr;
    while( qItr ){
        head = MEMBER_BASE( qItr, DzThread, qItr );
        qItr = qItr->next;
        FreeDzThread( host, head );
    }
}

#ifdef __cplusplus
};
#endif

#endif // __DzCore_h__
