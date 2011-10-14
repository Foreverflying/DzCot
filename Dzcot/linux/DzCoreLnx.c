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

// CotScheduleCenter:
// the Cot Schedule Center thread uses the host's origin thread's stack
// manager all kernel objects that may cause real block
void CotScheduleCenter( DzHost* host )
{
    int i;
    int timeout;
    int listCount;
    int notifyCount;
    DzAsyncIo* asyncIo;
    struct epoll_event evtList[ EPOLL_EVT_LIST_SIZE ];

    timeout = NotifyMinTimersAndRmtCalls( host );
    while( host->threadCount ){
        listCount = epoll_wait( host->osStruct.epollFd, evtList, EPOLL_EVT_LIST_SIZE, timeout );
        if( listCount != 0 ){
            notifyCount = 0;
            for( i = 0; i < listCount; i++ ){
                asyncIo = (DzAsyncIo*)evtList[i].data.ptr;
                if( IsEasyEvtWaiting( &asyncIo->inEvt ) && ( evtList[i].events & EPOLLIN ) ){
                    NotifyEasyEvt( host, &asyncIo->inEvt );
                    CleanEasyEvt( &asyncIo->inEvt );
                    notifyCount++;
                }
                if( IsEasyEvtWaiting( &asyncIo->outEvt ) && ( evtList[i].events & EPOLLOUT ) ){
                    NotifyEasyEvt( host, &asyncIo->outEvt );
                    CleanEasyEvt( &asyncIo->outEvt );
                    notifyCount++;
                }
            }
            if( notifyCount ){
                host->currPriority = CP_FIRST;
                Schedule( host );
            }
        }
        timeout = NotifyMinTimersAndRmtCalls( host );
    }
}

