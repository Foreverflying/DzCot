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
#include "DzBase.h"

#ifdef __cplusplus
extern "C"{
#endif

BOOL MemeryPoolGrow( DzHost* host );
void ReleaseMemoryPool( DzHost* host );
BOOL AllocListNodePool( DzHost* host );
BOOL AllocSynObjPool( DzHost* host );
BOOL AllocDzCotPool( DzHost* host );
BOOL AllocDzFdPool( DzHost* host );

inline void* AllocChunk( DzHost* host, size_t size )
{
    char* p = host->memPoolPos;

    host->memPoolPos += size;
    if( host->memPoolPos > host->memPoolEnd ){
        if( !MemeryPoolGrow( host ) ){
            return NULL;
        }
        p = host->memPoolPos;
        host->memPoolPos += size;
    }
    return PageCommit( p, size );
}

inline DzLNode* AllocLNode( DzHost* host )
{
    DzLNode* node;

    if( !host->lNodePool ){
        if( !AllocListNodePool( host ) ){
            return NULL;
        }
    }
    node = MEMBER_BASE( host->lNodePool, DzLNode, lItr );
    host->lNodePool = host->lNodePool->next;
    return node;
}

inline void FreeLNode( DzHost* host, DzLNode* node )
{
    node->lItr.next = host->lNodePool;
    host->lNodePool = &node->lItr;
}

inline void FreeChainLNode( DzHost* host, DzLItr* head, DzLItr* tail )
{
    tail->next = host->lNodePool;
    host->lNodePool = head;
}

#ifdef __cplusplus
};
#endif

#endif // __DzResourceMgr_h__
