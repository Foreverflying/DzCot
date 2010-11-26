/********************************************************************
    created:    2010/02/11 22:03
    file:       DzResourceMgr.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzResourceMgr_h__
#define __DzResourceMgr_h__

#include "DzStructs.h"
#include "DzList.h"
#include "DzStructsIoOs.h"
#include "DzBaseOs.h"

#ifdef __cplusplus
extern "C"{
#endif

BOOL MemeryPoolGrow( DzHost* host );
void ReleaseMemoryPool( DzHost* host );
BOOL AllocQueueNodePool( DzHost* host );
BOOL AllocAsynIoPool( DzHost* host );
BOOL AllocSynObjPool( DzHost* host );
BOOL AllocDzThreadPool( DzHost* host, int sSize );

inline void* AllocChunk( DzHost* host, size_t size )
{
    host->memPoolPos += size;
    if( host->memPoolPos > host->memPoolEnd ){
        if( !MemeryPoolGrow( host ) ){
            return NULL;
        }
    }
    return PageCommit( host->memPoolPos, size );
}

inline DzLNode* AllocQNode( DzHost* host )
{
    DzLNode* node;
    DzLItr* head;

    head = &host->lNodePool;
    if( !head->next ){
        if( !AllocQueueNodePool( host ) ){
            return NULL;
        }
    }
    node = MEMBER_BASE( head->next, DzLNode, lItr );
    PopSList( head );
    return node;
}

inline void FreeQNode( DzHost* host, DzLNode* node )
{
    PushSList( &host->lNodePool, &node->lItr );
}

#ifdef __cplusplus
};
#endif

#endif // __DzResourceMgr_h__
