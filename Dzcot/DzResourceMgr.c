/********************************************************************
    created:    2010/02/11 22:13
    file:       DzResourceMgr.c
    author:     Foreverflying
    purpose:    
*********************************************************************/

#include "DzIncOs.h"
#include "DzBaseOs.h"
#include "DzResourceMgr.h"
#include "DzIoOs.h"
#include "DzSynObj.h"
#include "DzCoreOs.h"
#include "DzCore.h"

BOOL MemeryPoolGrow( DzHost* host )
{
    char* pool;
    DzLNode* node;

    pool = (char*)PageReserv( MEMERY_POOL_GROW_SIZE );
    if( !pool ){
        return FALSE;
    }
    host->memPoolPos = pool;
    host->memPoolEnd = pool + MEMERY_POOL_GROW_SIZE;
    node = AllocQNode( host );
    node->content = pool;
    node->context1 = (void*)MEMERY_POOL_GROW_SIZE;
    PushSList( &host->poolGrowList, &node->lItr );
    return TRUE;
}

void ReleaseMemoryPool( DzHost* host )
{
    DzLNode* node;
    void** p;
    int* len;
    int count = 0;
    int poolGrowCount = 0;
    DzLItr* lItr = host->poolGrowList.next;

    while( lItr ){
        poolGrowCount++;
        lItr = lItr->next;
    }
    p = (void**)alloca( sizeof(void*) * poolGrowCount );
    len = (int*)alloca( sizeof(int) * poolGrowCount );
    lItr = host->poolGrowList.next;
    while( lItr ){
        node = MEMBER_BASE( lItr, DzLNode, lItr );
        *(p + count) = node->content;
        *(len + count++) = (int)node->context1;
        lItr = lItr->next;
    }
    for( count = 0; count < poolGrowCount; count++ ){
        PageFree( p[ count ], len[ count ] );
    }
    host->poolGrowList.next = NULL;
}

BOOL AllocQueueNodePool( DzHost* host )
{
    DzLNode* p;
    DzLNode* end;
    DzLNode* tmp;
    DzLItr* lItr;

    p = (DzLNode*)AllocChunk( host, PAGE_SIZE / sizeof( int ) * sizeof( DzLNode ) );
    if( !p ){
        return FALSE;
    }

    host->lNodePool.next = &p->lItr;
    end = p + PAGE_SIZE / sizeof( int ) - 1;
    end->lItr.next = NULL;
    tmp = p;
    while( tmp != end ){
        lItr = &tmp->lItr;
        lItr->next = &(++tmp)->lItr;
    }
    return TRUE;
}

BOOL AllocAsynIoPool( DzHost* host )
{
    DzAsynIo* p;
    DzAsynIo* end;
    DzLItr* lItr;

    if( !host->lNodePool.next ){
        if( !AllocQueueNodePool( host ) ){
            return FALSE;
        }
    }

    p = (DzAsynIo*)AllocChunk( host, PAGE_SIZE / sizeof( int ) * sizeof( DzAsynIo ) );
    if( !p ){
        return FALSE;
    }

    host->asynIoPool.next = &p->lItr;
    end = p + PAGE_SIZE / sizeof( int ) - 1;
    end->lItr.next = NULL;
    InitAsynIo( end );
    while( p != end ){
        InitAsynIo( p );
        lItr = &p->lItr;
        lItr->next = &(++p)->lItr;
    }
    return TRUE;
}

BOOL AllocSynObjPool( DzHost* host )
{
    DzSynObj* p;
    DzSynObj* end;
    DzLItr* lItr;

    if( !host->lNodePool.next ){
        if( !AllocQueueNodePool( host ) ){
            return FALSE;
        }
    }

    p = (DzSynObj*)AllocChunk( host, PAGE_SIZE / sizeof( int ) * sizeof( DzSynObj ) );
    if( !p ){
        return FALSE;
    }

    host->synObjPool.next = &p->lItr;
    end = p + PAGE_SIZE / sizeof( int ) - 1;
    end->lItr.next = NULL;
    InitDList( &end->waitQ[ CP_HIGH ] );
    InitDList( &end->waitQ[ CP_NORMAL ] );
    InitDList( &end->waitQ[ CP_LOW ] );
    while( p != end ){
        InitDList( &p->waitQ[ CP_HIGH ] );
        InitDList( &p->waitQ[ CP_NORMAL ] );
        InitDList( &p->waitQ[ CP_LOW ] );
        lItr = &p->lItr;
        lItr->next = &(++p)->lItr;
    }
    return TRUE;
}

BOOL AllocDzThreadPool( DzHost* host, int sSize )
{
    DzThread* p;
    DzThread* end;
    DzLItr* lItr;

    if( !host->lNodePool.next ){
        if( !AllocQueueNodePool( host ) ){
            return FALSE;
        }
    }

    p = (DzThread*)AllocChunk( host, PAGE_SIZE / sizeof( int ) * sizeof( DzThread ) );
    if( !p ){
        return FALSE;
    }

    host->threadPools[ sSize ].next = &p->lItr;
    end = p + PAGE_SIZE / sizeof( int ) - 1;
    end->lItr.next = NULL;
    InitDzThread( end, sSize );
    while( p != end ){
        InitDzThread( p, sSize );
        lItr = &p->lItr;
        lItr->next = &(++p)->lItr;
    }
    return TRUE;
}
