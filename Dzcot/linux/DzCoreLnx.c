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
        host->osStruct.maxFd = parentHost->osStruct.maxFd;
    }else{
        if( getrlimit( RLIMIT_NOFILE, &fdLimit ) != 0 ){
            return FALSE;
        }
        host->osStruct.maxFd = fdLimit.rlim_cur;
    }
    host->osStruct.epollFd = epoll_create( host->osStruct.maxFd );
    host->osStruct.fdTable = ( DzAsynIo** )PageAlloc( sizeof( DzAsynIo* ) * host->osStruct.maxFd );
    if( host->osStruct.epollFd < 0 || !host->osStruct.fdTable ){
        if( host->osStruct.epollFd >= 0 ){
            close( host->osStruct.epollFd );
        }
        if( host->osStruct.fdTable ){
            PageFree( host->osStruct.fdTable, sizeof(int) * host->osStruct.maxFd );
        }
        return FALSE;
    }
    host->osStruct.asynIoPool = NULL;
    return TRUE;
}

void DeleteOsStruct( DzHost* host, DzHost* parentHost )
{
    PageFree( host->osStruct.fdTable, sizeof( DzAsynIo* ) * host->osStruct.maxFd );
    close( host->osStruct.epollFd );
}

BOOL AllocAsynIoPool( DzHost* host )
{
    DzAsynIo* p;
    DzAsynIo* end;
    DzLItr* lItr;

    p = (DzAsynIo*)AllocChunk( host, OBJ_POOL_GROW_COUNT * sizeof( DzAsynIo ) );
    if( !p ){
        return FALSE;
    }

    host->osStruct.asynIoPool = &p->lItr;
    end = p + OBJ_POOL_GROW_COUNT - 1;
    end->lItr.next = NULL;
    InitAsynIo( end );
    while( p != end ){
        InitAsynIo( p );
        lItr = &p->lItr;
        lItr->next = &(++p)->lItr;
    }
    return TRUE;
}

// DzcotRoutine:
// the real entry the co thread starts, it call the user entry
// after that, the thread is finished, so put itself to the thread pool
// schedule next thread
#ifdef __i386
void __stdcall DzcotRoutine(
    DzRoutine entry,
    void* context
    )
#elif defined __amd64
void __stdcall DzcotRoutine(
    void* unused1,
    void* unused2,
    void* unused3,
    void* unused4,
    void* unused5,
    void* unused6,
    DzRoutine entry,
    void* context
    )
#endif
{
    DzHost* host = GetHost();
    while(1){
        //call the entry
        ( *(DzRoutine volatile *)(&entry) )( *(void* volatile *)(&context) );

        //free the thread
        host->threadCount--;
        FreeDzThread( host, host->currThread );

        //then schedule another thread
        Schedule( host );
    }
}
