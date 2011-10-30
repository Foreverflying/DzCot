/********************************************************************
    created:    2010/02/11 22:12
    file:       DzCoreLnx.c
    author:     Foreverflying
    purpose:
*********************************************************************/

#include "../DzInc.h"
#include "../DzCore.h"

void* SysThreadMain( void* context )
{
    DzSysParam* param = (DzSysParam*)context;
    param->threadEntry( (intptr_t)param );
    return NULL;
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

    host->asyncIoPool = &p->lItr;
    end = p + OBJ_POOL_GROW_COUNT - 1;
    end->lItr.next = NULL;
    CleanEasyEvt( &end->inEvt );
    CleanEasyEvt( &end->outEvt );
    end->inEvt.dzCot = NULL;
    end->outEvt.dzCot = NULL;
    end->ref = 0;
    while( p != end ){
        CleanEasyEvt( &p->inEvt );
        CleanEasyEvt( &p->outEvt );
        p->inEvt.dzCot = NULL;
        p->outEvt.dzCot = NULL;
        p->ref = 0;
        lItr = &p->lItr;
        lItr->next = &(++p)->lItr;
    }
    return TRUE;
}

// DzCotEntry:
// the real function entry the cot starts, it call the user entry
// after that, when the cot is finished, put it into the cot pool
// schedule next cot
void __stdcall DzCotEntry(
    DzHost*             host,
    DzRoutine volatile* entryPtr,
    intptr_t volatile*  contextPtr
    )
{
    while(1){
        //call the entry
        ( *entryPtr )( *contextPtr );

        //free the cot
        host->cotCount--;
        FreeDzCot( host, host->currCot );

        //then schedule another cot
        Schedule( host );
    }
}

BOOL InitOsStruct( DzHost* host )
{
    int flag;
    struct rlimit fdLimit;
    struct epoll_event evt;
    DzHost* firstHost = host->hostId == 0 ? NULL : host->mgr->hostArr[0];

    if( !firstHost ){
        if( getrlimit( RLIMIT_NOFILE, &fdLimit ) != 0 ){
            return FALSE;
        }
        host->os.maxFdCount = fdLimit.rlim_cur;
    }else{
        host->os.maxFdCount = firstHost->os.maxFdCount;
    }
    host->os.epollFd = epoll_create( host->os.maxFdCount );
    if( host->os.epollFd < 0 ){
        return FALSE;
    }
    if( pipe( host->os.pipe ) != 0 ){
        close( host->os.epollFd );
        return FALSE;
    }
    if( !firstHost ){
        host->os.fdTable = ( DzAsyncIo** )
            PageAlloc( sizeof( DzAsyncIo* ) * host->os.maxFdCount );
        if( !host->os.fdTable ){
            close( host->os.pipe[1] );
            close( host->os.pipe[0] );
            close( host->os.epollFd );
            return FALSE;
        }
    }else{
        host->os.fdTable = firstHost->os.fdTable;
    }
    host->os.pipeAsyncIo = CreateAsyncIo( host );
    evt.data.ptr = host->os.pipeAsyncIo;
    evt.events = EPOLLIN;
    flag = fcntl( host->os.pipe[0], F_GETFL, 0 );
    fcntl( host->os.pipe[0], F_SETFL, flag | O_NONBLOCK );
    flag = fcntl( host->os.pipe[1], F_GETFL, 0 );
    fcntl( host->os.pipe[1], F_SETFL, flag | O_NONBLOCK );
    epoll_ctl( host->os.epollFd, EPOLL_CTL_ADD, host->os.pipe[0], &evt );
    host->os.evtList = (struct epoll_event*)
        AllocChunk( host, sizeof( struct epoll_event ) * EPOLL_EVT_LIST_SIZE );
    return TRUE;
}

void DeleteOsStruct( DzHost* host )
{
    CloseAsyncIo( host, host->os.pipeAsyncIo );
    if( host->hostId == 0 ){
        PageFree(
            host->os.fdTable,
            sizeof( DzAsyncIo* ) * host->os.maxFdCount
            );
    }
    close( host->os.pipe[1] );
    close( host->os.pipe[0] );
    close( host->os.epollFd );
}
