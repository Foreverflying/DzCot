/********************************************************************
    created:    2010/02/11 22:03
    file:       DzResourceMgr.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzResourceMgr_h__
#define __DzResourceMgr_h__

#include "DzStructs.h"
#include "DzQueue.h"
#include "DzStructsIoOs.h"
#include "DzCoreOs.h"

#ifdef __cplusplus
extern "C"{
#endif

void AddMallocRecord( DzHost *host, void *p );
void ReleaseAllMalloc( DzHost *host );
BOOL AllocQueueNodePool( DzHost *host, int count );
BOOL AllocAsynIoPool( DzHost *host, int count );
BOOL AllocSynObjPool( DzHost *host, int count );
BOOL AllocDzThreadPool( DzHost *host, int sSize, int count );

inline DzQNode* AllocQNode( DzHost *host )
{
    DzQNode *node;
    DzQItr *head;

    head = &host->qNodePool;
    if( !head->next ){
        if( !AllocQueueNodePool( host, 0 ) ){
            return NULL;
        }
    }
    node = MEMBER_BASE( head->next, DzQNode, qItr );
    PopQItr( head );
    return node;
}

inline void FreeQNode( DzHost *host, DzQNode *node )
{
    PushQItr( &host->qNodePool, &node->qItr );
}

inline DzAsynIo* AllocAsynIo( DzHost *host )
{
    DzQItr *head;
    DzAsynIo *asynIo;

    head = &host->asynIoPool;
    if( !head->next ){
        if( !AllocAsynIoPool( host, 0 ) ){
            return NULL;
        }
    }
    asynIo = MEMBER_BASE( head->next, DzAsynIo, qItr );
    PopQItr( head );
    return asynIo;
}

inline void FreeAsynIo( DzHost *host, DzAsynIo *asynIo )
{
    PushQItr( &host->asynIoPool, &asynIo->qItr );
}

inline DzSynObj* AllocSynObj( DzHost *host )
{
    DzQItr *head;
    DzSynObj *obj;

    head = &host->synObjPool;
    if( !head->next ){
        if( !AllocSynObjPool( host, 0 ) ){
            return NULL;
        }
    }
    obj = MEMBER_BASE( head->next, DzSynObj, qItr );
    PopQItr( head );
    return obj;
}

inline void FreeSynObj( DzHost *host, DzSynObj *obj )
{
    PushQItr( &host->synObjPool, &obj->qItr );
}

inline DzThread* AllocDzThread( DzHost *host, int sSize )
{
    DzThread *dzThread;
    DzQItr *head;

    head = &host->threadPools[ sSize ];
    if( !head->next ){
        if( !AllocDzThreadPool( host, sSize, 0 ) ){
            return NULL;
        }
    }

    dzThread = MEMBER_BASE( head->next, DzThread, qItr );
    if( !dzThread->stack ){
        dzThread->stack = AllocStack( sSize );
        if( !dzThread->stack ){
            return NULL;
        }
        host->poolCotCounts[ sSize ]++;
    }
    if( !dzThread->stackLimit ){
        dzThread->stackLimit = CommitStack( dzThread->stack, PAGE_SIZE * 3 );
        if( !dzThread->stackLimit )
        {
            return NULL;
        }
        DzInitCot( host, dzThread );
    }
    PopQItr( head );
    return dzThread;
}

inline void FreeDzThread( DzHost *host, DzThread *dzThread )
{
    PushQItr( &host->threadPools[ dzThread->stackSize ], &dzThread->qItr );
    //DeCommitStack( dzThread->stack, dzThread->stackLimit );
    //dzThread->stackLimit = NULL;
}

#ifdef __cplusplus
};
#endif

#endif // __DzResourceMgr_h__
