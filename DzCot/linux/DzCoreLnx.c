/**
 *  @file       DzCoreLnx.c
 *  @brief      
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#include "../DzInc.h"
#include "../DzCore.h"

DzCot* InitCot( DzHost* host, DzCot* dzCot, int sType )
{
    int size;

    size = host->cotStackSize[ sType ];
    if( size < DZ_MIN_PAGE_STACK_SIZE ){
        dzCot->stack = (char*)AllocChunk( host, size );
        if( !dzCot->stack ){
            return NULL;
        }
        dzCot->stack += size;
    }else{
        dzCot->stack = AllocStack( size );
        if( !dzCot->stack ){
            return NULL;
        }
    }
    dzCot->sType = sType;
    InitCotStack( host, dzCot );
    return dzCot;
}

void* SysThreadMain( void* context )
{
    DzSysParam* param = (DzSysParam*)context;
    param->threadEntry( (intptr_t)param );
    return NULL;
}

// DzCotEntry:
// the real function entry the cot starts, it call the user entry
// after that, when the cot is finished, put it into the cot pool
// schedule next cot
void __stdcall DzCotEntry(
    DzHost*             host,
    DzEntry volatile*   entryPtr,
    intptr_t volatile*  contextPtr
    )
{
    __Dbg( MarkCurrStackForCheck )( host );
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

    if( host->hostId == 0 ){
        if( getrlimit( RLIMIT_NOFILE, &fdLimit ) != 0 ){
            return FALSE;
        }
        host->os.maxFdCount = fdLimit.rlim_cur;
    }else{
        host->os.maxFdCount = host->mgr->hostArr[0]->os.maxFdCount;
    }
    host->os.epollFd = epoll_create( host->os.maxFdCount );
    if( host->os.epollFd < 0 ){
        return FALSE;
    }
    if( pipe( host->os.pipe ) != 0 ){
        close( host->os.epollFd );
        return FALSE;
    }
    host->os.pipeFd = CreateDzFd( host );
    evt.data.ptr = host->os.pipeFd;
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

void CleanOsStruct( DzHost* host )
{
    CloseDzFd( host, host->os.pipeFd );
    close( host->os.pipe[1] );
    close( host->os.pipe[0] );
    close( host->os.epollFd );
}
