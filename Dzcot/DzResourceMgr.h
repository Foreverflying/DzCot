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

#ifdef __cplusplus
};
#endif

#endif // __DzResourceMgr_h__
