/********************************************************************
    created:    2010/02/11 22:12
    file:       DzCoreLnx.c
    author:     Foreverflying
    purpose:
*********************************************************************/

#include "../DzIncOs.h"
#include "../DzCore.h"

void* SysThreadEntry( void* context )
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

BOOL InitOsStruct( DzHost* host, DzHost* firstHost )
{
    int flag;
    struct rlimit fdLimit;
    struct epoll_event evt;

    if( firstHost ){
        host->osStruct.maxFdCount = firstHost->osStruct.maxFdCount;
    }else{
        if( getrlimit( RLIMIT_NOFILE, &fdLimit ) != 0 ){
            return FALSE;
        }
        host->osStruct.maxFdCount = fdLimit.rlim_cur;
    }
    host->osStruct.epollFd = epoll_create( host->osStruct.maxFdCount );
    if( host->osStruct.epollFd < 0 ){
        return FALSE;
    }
    if( pipe( host->osStruct.pipe ) != 0 ){
        close( host->osStruct.epollFd );
        return FALSE;
    }
    host->osStruct.fdTable =
        ( DzAsyncIo** )PageAlloc( sizeof( DzAsyncIo* ) * host->osStruct.maxFdCount );
    if( !host->osStruct.fdTable ){
        close( host->osStruct.pipe[1] );
        close( host->osStruct.pipe[0] );
        close( host->osStruct.epollFd );
        return FALSE;
    }
    host->osStruct.asyncIoPool = NULL;
    host->osStruct.pipeAsyncIo = CreateAsyncIo( host );
    evt.data.ptr = host->osStruct.pipeAsyncIo;
    evt.events = EPOLLIN;
    flag = fcntl( host->osStruct.pipe[0], F_GETFL, 0 );
    fcntl( host->osStruct.pipe[0], F_SETFL, flag | O_NONBLOCK );
    flag = fcntl( host->osStruct.pipe[1], F_GETFL, 0 );
    fcntl( host->osStruct.pipe[1], F_SETFL, flag | O_NONBLOCK );
    epoll_ctl( host->osStruct.epollFd, EPOLL_CTL_ADD, host->osStruct.pipe[0], &evt );
    return TRUE;
}

void DeleteOsStruct( DzHost* host, DzHost* firstHost )
{
    CloseAsyncIo( host, host->osStruct.pipeAsyncIo );
    PageFree( host->osStruct.fdTable, sizeof( DzAsyncIo* ) * host->osStruct.maxFdCount );
    close( host->osStruct.pipe[1] );
    close( host->osStruct.pipe[0] );
    close( host->osStruct.epollFd );
}

// CotScheduleCenter:
// the Cot Schedule Center thread uses the host's origin thread's stack
// manager all kernel objects that may cause real block
void CotScheduleCenter( DzHost* host )
{
    int i;
    int timeout;
    int listCount;
    int n;
    DzAsyncIo* asyncIo;
    struct epoll_event *evtList;

    evtList = (struct epoll_event*)AllocChunk( host, sizeof( struct epoll_event ) * EPOLL_EVT_LIST_SIZE );
    while( 1 ){
        timeout = NotifyMinTimersAndRmtCalls( host );
        while( host->threadCount ){
            listCount = epoll_wait( host->osStruct.epollFd, evtList, EPOLL_EVT_LIST_SIZE, timeout );
            AtomAndInt( &host->checkRmtSign, ~RMT_CHECK_SLEEP_SIGN );
            if( listCount != 0 ){
                n = 0;
                while( 1 ){
                    for( i = 0; i < listCount; i++ ){
                        asyncIo = (DzAsyncIo*)evtList[i].data.ptr;
                        if( IsEasyEvtWaiting( &asyncIo->inEvt ) && ( evtList[i].events & EPOLLIN ) ){
                            NotifyEasyEvt( host, &asyncIo->inEvt );
                            CleanEasyEvt( &asyncIo->inEvt );
                            n++;
                        }
                        if( IsEasyEvtWaiting( &asyncIo->outEvt ) && ( evtList[i].events & EPOLLOUT ) ){
                            NotifyEasyEvt( host, &asyncIo->outEvt );
                            CleanEasyEvt( &asyncIo->outEvt );
                            n++;
                        }
                    }
                    if( listCount == EPOLL_EVT_LIST_SIZE ){
                        listCount = epoll_wait( host->osStruct.epollFd, evtList, EPOLL_EVT_LIST_SIZE, 0 );
                        continue;
                    }
                    break;
                }
                read( host->osStruct.pipe[0], evtList, 2048 );
                if( n ){
                    host->currPriority = CP_FIRST;
                    Schedule( host );
                }
            }
            timeout = NotifyMinTimersAndRmtCalls( host );
        }
        if( AtomAndInt( &host->hostMgr->exitSign, ~host->hostMask ) != host->hostMask ){
            listCount = epoll_wait( host->osStruct.epollFd, evtList, EPOLL_EVT_LIST_SIZE, -1 );
            if( AtomReadInt( &host->hostMgr->exitSign ) ){
                AtomAndInt( &host->checkRmtSign, ~RMT_CHECK_SLEEP_SIGN );
                read( host->osStruct.pipe[0], evtList, 2048 );
                continue;
            }
        }else{
            //be sure quit id 0 host at the end, for hostMgr is in id 0 host's stack
            for( n = host->hostMgr->hostCount - 1; n >= 0; n-- ){
                if( n != host->hostId ){
                    AwakeRemoteHost( host->hostMgr->hostArr[ n ] );
                }
            }
        }
        break;
    }
}
