#include "DzIncOs.h"
#include "DzResourceMgr.h"

void AddMallocRecord( DzHost *host, void *p )
{
    DzQNode *node = AllocQNode( host );
    node->content = p;
    PushQItr( &host->mallocList, &node->qItr );
    host->mallocCount++;
}

void ReleaseAllMalloc( DzHost *host )
{
    void **p = (void**)malloc( sizeof(void*) * host->mallocCount );
    DzQItr *qItr = host->mallocList.next;
    void **tmp = p;
    while( qItr ){
        *tmp++ = MEMBER_BASE( qItr, DzQNode, qItr )->content;
        qItr = qItr->next;
    }
    while( p != tmp ){
        free( *p++ );
    }
    free( p );
    host->mallocList.next = NULL;
}

BOOL AllocQueueNodePool( DzHost *host, int count )
{
    DzQNode *p;
    DzQNode *end;
    DzQNode *tmp;
    DzQItr *qItr;

    if( !count ){
        count = PAGE_SIZE / sizeof( int );
    }
    p = (DzQNode*)malloc( count * sizeof( DzQNode ) );
    if( !p ){
        return FALSE;
    }

    host->qNodePool.next = &p->qItr;
    end = p + count - 1;
    end->qItr.next = NULL;
    tmp = p;
    while( tmp != end ){
        qItr = &tmp->qItr;
        qItr->next = &(++tmp)->qItr;
    }
    AddMallocRecord( host, p );
    return TRUE;
}

BOOL AllocAsynIoPool( DzHost *host, int count )
{
    DzAsynIo *p;
    DzAsynIo *end;
    DzQItr *qItr;

    if( !host->qNodePool.next ){
        if( !AllocQueueNodePool( host, 0 ) ){
            return FALSE;
        }
    }

    if( !count ){
        count = PAGE_SIZE / sizeof( int );
    }
    p = (DzAsynIo*)malloc( count * sizeof( DzAsynIo ) );
    if( !p ){
        return FALSE;
    }

    AddMallocRecord( host, p );
    host->asynIoPool.next = &p->qItr;
    end = p + count - 1;
    end->qItr.next = NULL;
    while( p != end ){
        qItr = &p->qItr;
        qItr->next = &(++p)->qItr;
    }
    return TRUE;
}

/*
BOOL AllocAsynIoPool( DzHost *host, int count )
{
    DzFd *p;
    if( !count ){
        count = PAGE_SIZE / sizeof( int );
    }
    p = (DzFd*)malloc( count * sizeof( DzFd ) );
    if( !p ){
        return FALSE;
    }

    AddMallocRecord( host, p );
    host->fdPool.next = &p->qItr;
    DzFd *end = p + count - 1;
    end->qItr.next = NULL;
    while( p != end ){
        DzQItr *qItr = &p->qItr;
        qItr->next = &(++p)->qItr;
    }
    return TRUE;
}
//*/

BOOL AllocSynObjPool( DzHost *host, int count )
{
    DzSynObj *p;
    DzSynObj *end;
    DzQItr *qItr;

    if( !host->qNodePool.next ){
        if( !AllocQueueNodePool( host, 0 ) ){
            return FALSE;
        }
    }

    if( !count ){
        count = PAGE_SIZE / sizeof( int );
    }
    p = (DzSynObj*)malloc( count * sizeof( DzSynObj ) );
    if( !p ){
        return FALSE;
    }

    AddMallocRecord( host, p );
    host->synObjPool.next = &p->qItr;
    end = p + count - 1;
    end->qItr.next = NULL;
    InitDeque( &end->waitQ );
    InitDeque( &end->waitAllQ );
    while( p != end ){
        InitDeque( &p->waitQ );
        InitDeque( &p->waitAllQ );
        qItr = &p->qItr;
        qItr->next = &(++p)->qItr;
    }
    return TRUE;
}

BOOL AllocDzThreadPool( DzHost *host, int sSize, int count )
{
    DzThread *p;
    DzThread *end;
    DzQItr *qItr;

    if( !host->qNodePool.next ){
        if( !AllocQueueNodePool( host, 0 ) ){
            return FALSE;
        }
    }

    if( !count ){
        count = PAGE_SIZE / sizeof( int );
    }
    p = (DzThread*)malloc( count * sizeof( DzThread ) );
    if( !p ){
        return FALSE;
    }

    AddMallocRecord( host, p );
    host->threadPools[ sSize ].next = &p->qItr;
    end = p + count - 1;
    end->qItr.next = NULL;
    end->stack = NULL;
    end->stackLimit = NULL;
    end->stackSize = sSize;
    end->finishEvent = NULL;
    while( p != end ){
        p->stack = NULL;
        p->stackLimit = NULL;
        p->stackSize = sSize;
        p->finishEvent = NULL;
        qItr = &p->qItr;
        qItr->next = &(++p)->qItr;
    }
    return TRUE;
}