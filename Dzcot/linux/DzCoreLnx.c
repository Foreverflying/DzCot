/********************************************************************
    created:    2010/02/11 22:12
    file:       DzCoreLnx.c
    author:     Foreverflying
    purpose:
*********************************************************************/

#include "../DzIncOs.h"
#include "../DzCoreOs.h"
#include "../DzCore.h"

BOOL InitOsStruct( DzHost* host, DzHost* parentHost )
{
    struct rlimit fdLimit;

    if( parentHost ){
        host->osStruct.maxFdCount = parentHost->osStruct.maxFdCount;
    }else{
        if( getrlimit( RLIMIT_NOFILE, &fdLimit ) != 0 ){
            return FALSE;
        }
        host->osStruct.maxFdCount = fdLimit.rlim_cur;
    }
    host->osStruct.epollFd = epoll_create( host->osStruct.maxFdCount );
    host->osStruct.fdTable =
        ( DzAsyncIo** )PageAlloc( sizeof( DzAsyncIo* ) * host->osStruct.maxFdCount );
    if( host->osStruct.epollFd < 0 || !host->osStruct.fdTable ){
        if( host->osStruct.epollFd >= 0 ){
            close( host->osStruct.epollFd );
        }
        if( host->osStruct.fdTable ){
            PageFree( host->osStruct.fdTable, sizeof(int) * host->osStruct.maxFdCount );
        }
        return FALSE;
    }
    host->osStruct.asyncIoPool = NULL;
    return TRUE;
}

void DeleteOsStruct( DzHost* host, DzHost* parentHost )
{
    PageFree( host->osStruct.fdTable, sizeof( DzAsyncIo* ) * host->osStruct.maxFdCount );
    close( host->osStruct.epollFd );
}

BOOL AllocAsyncIoPool( DzHost* host )
{
    DzAsyncIo* p;
    DzAsyncIo* end;
    DzLItr* lItr;

    p = (DzAsyncIo*)AllocChunk( host, OBJ_POOL_GROW_COUNT * sizeof( DzAsyncIo ) );
    if( !p ){
        return FALSE;
    }

    host->osStruct.asyncIoPool = &p->lItr;
    end = p + OBJ_POOL_GROW_COUNT - 1;
    end->lItr.next = NULL;
    CleanEasyEvt( &end->inEvt );
    CleanEasyEvt( &end->outEvt );
    end->inEvt.dzThread = NULL;
    end->outEvt.dzThread = NULL;
    end->ref = 0;
    while( p != end ){
        CleanEasyEvt( &p->inEvt );
        CleanEasyEvt( &p->outEvt );
        p->inEvt.dzThread = NULL;
        p->outEvt.dzThread = NULL;
        p->ref = 0;
        lItr = &p->lItr;
        lItr->next = &(++p)->lItr;
    }
    return TRUE;
}

// DzcotEntry:
// the real function entry the cot starts, it call the user entry
// after that, when the cot is finished, put it into the thread pool
// schedule next cot
void __stdcall DzcotEntry(
    volatile DzRoutine* entryPtr,
    volatile intptr_t*  contextPtr
    )
{
    DzHost* host = GetHost();
    while(1){
        //call the entry
        ( *entryPtr )( *contextPtr );

        //free the thread
        host->threadCount--;
        FreeDzThread( host, host->currThread );

        //then schedule another thread
        Schedule( host );
    }
}
