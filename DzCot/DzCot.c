/********************************************************************
    created:    2010/02/11 22:12
    file:       DzCot.c
    author:     Foreverflying
    purpose:    
********************************************************************/

#include "DzInc.h"
#include "DzSynObj.h"
#include "DzCore.h"
#include "DzIoOs.h"
#include "DzRmtCore.h"

#ifdef __cplusplus
extern "C"{
#endif

int DzRunHosts(
    int         hostCount,
    int*        servMask,
    int         lowestPri,
    int         dftPri,
    int         dftSSize,
    DzRoutine   firstEntry,
    intptr_t    context,
    DzRoutine   cleanEntry
    )
{
    assert( !GetHost() );
    assert( hostCount > 0 && hostCount < DZ_MAX_HOST );
    assert(
        lowestPri >= CP_FIRST &&
        lowestPri < COT_PRIORITY_COUNT
        );
    assert(
        dftPri >= CP_FIRST &&
        dftPri <= lowestPri
        );
    assert(
        dftSSize >= SS_FIRST &&
        dftSSize < STACK_SIZE_COUNT
        );
    assert( firstEntry );

    return RunHosts(
        hostCount, servMask, lowestPri, dftPri, dftSSize,
        firstEntry, context, cleanEntry
        );
}

int DzStartCot(
    DzRoutine   entry,
    intptr_t    context,
    int         priority,
    int         sSize
    )
{
    DzHost* host = GetHost();
    assert( host );
    if( priority == CP_DEFAULT ){
        priority = host->dftPri;
    }
    if( sSize == SS_DEFAULT ){
        sSize = host->dftSSize;
    }
    assert( entry );
    assert( priority >= CP_FIRST && priority <= host->lowestPri );
    assert( sSize >= SS_FIRST && sSize < STACK_SIZE_COUNT );

    return StartCot( host, entry, context, priority, sSize );
}

int DzStartCotInstant(
    DzRoutine   entry,
    intptr_t    context,
    int         priority,
    int         sSize
    )
{
    DzHost* host = GetHost();
    assert( host );
    if( priority == CP_DEFAULT ){
        priority = host->dftPri;
    }
    if( sSize == SS_DEFAULT ){
        sSize = host->dftSSize;
    }
    assert( entry );
    assert( priority >= CP_FIRST && priority <= host->lowestPri );
    assert( sSize >= SS_FIRST && sSize < STACK_SIZE_COUNT );

    return StartCotInstant( host, entry, context, priority, sSize );
}

int DzEvtStartCot(
    int         evt,
    DzRoutine   entry,
    intptr_t    context,
    int         priority,
    int         sSize
    )
{
    DzHost* host = GetHost();
    assert( host );
    assert( evt >= 0 );
    assert( ( evt & HANDLE_HOST_ID_MASK ) == host->hostId );
    if( priority == CP_DEFAULT ){
        priority = host->dftPri;
    }
    if( sSize == SS_DEFAULT ){
        sSize = host->dftSSize;
    }
    assert(
        ( (DzSynObj*)( host->handleBase + evt ) )->type >= TYPE_EVT_AUTO &&
        ( (DzSynObj*)( host->handleBase + evt ) )->type <= TYPE_EVT_COUNT
        );
    assert( entry );
    assert( priority >= CP_FIRST && priority <= host->lowestPri );
    assert( sSize >= SS_FIRST && sSize < STACK_SIZE_COUNT );

    return EvtStartCot( host, evt, entry, context, priority, sSize );
}

int DzEvtStartCotInstant(
    int         evt,
    DzRoutine   entry,
    intptr_t    context,
    int         priority,
    int         sSize
    )
{
    DzHost* host = GetHost();
    assert( host );
    assert( evt >= 0 );
    assert( ( evt & HANDLE_HOST_ID_MASK ) == host->hostId );
    if( priority == CP_DEFAULT ){
        priority = host->dftPri;
    }
    if( sSize == SS_DEFAULT ){
        sSize = host->dftSSize;
    }
    assert(
        ( (DzSynObj*)( host->handleBase + evt ) )->type >= TYPE_EVT_AUTO &&
        ( (DzSynObj*)( host->handleBase + evt ) )->type <= TYPE_EVT_COUNT
        );
    assert( entry );
    assert( priority >= CP_FIRST && priority <= host->lowestPri );
    assert( sSize >= SS_FIRST && sSize < STACK_SIZE_COUNT );

    return EvtStartCotInstant( host, evt, entry, context, priority, sSize );
}

int DzStartRemoteCot(
    int         rmtId,
    DzRoutine   entry,
    intptr_t    context,
    int         priority,
    int         sSize
    )
{
    DzHost* host = GetHost();
    assert( host );
    if( priority == CP_DEFAULT ){
        priority = host->dftPri;
    }
    if( sSize == SS_DEFAULT ){
        sSize = host->dftSSize;
    }
    assert( rmtId >= 0 && rmtId < host->hostCount );
    assert( host->hostId != rmtId );
    assert( host->servMask & ( 1 << rmtId ) );
    assert( entry );
    assert( priority >= CP_FIRST && priority <= host->lowestPri );
    assert( sSize >= SS_FIRST && sSize < STACK_SIZE_COUNT );

    return StartRemoteCot( host, rmtId, entry, context, priority, sSize );
}

int DzEvtStartRemoteCot(
    int         evt,
    int         rmtId,
    DzRoutine   entry,
    intptr_t    context,
    int         priority,
    int         sSize
    )
{
    DzHost* host = GetHost();
    assert( host );
    assert( evt >= 0 );
    assert( ( evt & HANDLE_HOST_ID_MASK ) == host->hostId );
    if( priority == CP_DEFAULT ){
        priority = host->dftPri;
    }
    if( sSize == SS_DEFAULT ){
        sSize = host->dftSSize;
    }
    assert(
        ( (DzSynObj*)( host->handleBase + evt ) )->type >= TYPE_EVT_AUTO &&
        ( (DzSynObj*)( host->handleBase + evt ) )->type <= TYPE_EVT_COUNT
        );
    assert( rmtId >= 0 && rmtId < host->hostCount );
    assert( host->hostId != rmtId );
    assert( host->servMask & ( 1 << rmtId ) );
    assert( entry );
    assert( priority >= CP_FIRST && priority <= host->lowestPri );
    assert( sSize >= SS_FIRST && sSize < STACK_SIZE_COUNT );

    return EvtStartRemoteCot( host, evt, rmtId, entry, context, priority, sSize );
}

int DzRunRemoteCot(
    int         rmtId,
    DzRoutine   entry,
    intptr_t    context,
    int         priority,
    int         sSize
    )
{
    DzHost* host = GetHost();
    assert( host );
    if( priority == CP_DEFAULT ){
        priority = host->dftPri;
    }
    if( sSize == SS_DEFAULT ){
        sSize = host->dftSSize;
    }
    assert( rmtId >= 0 && rmtId < host->hostCount );
    assert( host->hostId != rmtId );
    assert( host->servMask & ( 1 << rmtId ) );
    assert( entry );
    assert( priority >= CP_FIRST && priority <= host->lowestPri );
    assert( sSize >= SS_FIRST && sSize < STACK_SIZE_COUNT );

    return RunRemoteCot( host, rmtId, entry, context, priority, sSize );
}

int DzRunWorker( DzRoutine entry, intptr_t context )
{
    DzHost* host = GetHost();
    assert( host );
    assert( host->servMask & host->hostMask );
    assert( entry );

    return RunWorker( host, entry, context );
}

int DzGetCotCount()
{
    DzHost* host = GetHost();
    assert( host );

    return GetCotCount( host );
}

int DzSetPriority( int priority )
{
    DzHost* host = GetHost();
    assert( host );
    assert( priority <= host->lowestPri );

    return SetCurrCotPriority( host, priority );
}

int DzSetCotPoolDepth( int sSize, int depth )
{
    DzHost* host = GetHost();
    assert( host );
    assert( sSize >= SS_FIRST && sSize < STACK_SIZE_COUNT );
    assert( depth <= DZ_MAX_COT_POOL_DEPTH );

    return SetCotPoolDepth( host, sSize, depth );
}

int DzSetWorkerPoolDepth( int depth )
{
    DzHost* host = GetHost();
    assert( host );
    assert( depth >= 0 && depth <= DZ_MAX_COT_POOL_DEPTH );

    return SetWorkerPoolDepth( host, depth );
}

int DzSetHostParam( int lowestPri, int dftPri, int dftSSize )
{
    DzHost* host = GetHost();
    assert( host );
    assert(
        lowestPri >= CP_FIRST &&
        lowestPri < COT_PRIORITY_COUNT
        );
    assert(
        dftPri >= CP_FIRST &&
        dftPri <= lowestPri
        );
    assert(
        dftSSize >= SS_FIRST &&
        dftSSize < STACK_SIZE_COUNT
        );

    return SetHostParam( host, lowestPri, dftPri, dftSSize );
}

int DzWaitSynObj( int obj, int timeout )
{
    DzHost* host = GetHost();
    assert( host );
    assert( obj >= 0 );
    assert( ( obj & HANDLE_HOST_ID_MASK ) == host->hostId );
    assert( ( (DzSynObj*)( host->handleBase + obj ) )->type <= TYPE_MAX_USER_CAN_WAIT );
    assert( timeout <= 0 || host->timerCount < TIME_HEAP_SIZE );

    return WaitSynObj( host, obj, timeout );
}

int DzWaitMultiSynObj( int count, int* objs, BOOL waitAll, int timeout )
{
    int i;
    DzHost* host = GetHost();
    assert( host );
    assert( timeout <= 0 || host->timerCount < TIME_HEAP_SIZE );
    assert( objs );
    assert( count > 0 );
    for( i = 0; i < count; i++ ){
        assert( objs[i] >= 0 );
        assert( ( objs[i] & HANDLE_HOST_ID_MASK ) == host->hostId );
        assert( ( (DzSynObj*)( host->handleBase + objs[i] ) )->type <= TYPE_MAX_USER_CAN_WAIT );
    }

    return WaitMultiSynObj( host, count, objs, waitAll, timeout );
}

int DzCreateMtx( BOOL owner )
{
    DzHost* host = GetHost();
    assert( host );

    return CreateAutoEvt( host, !owner );
}

BOOL DzReleaseMtx( int mtx )
{
    DzHost* host = GetHost();
    assert( host );
    assert( mtx >= 0 );
    assert( ( mtx & HANDLE_HOST_ID_MASK ) == host->hostId );
    assert( ( (DzSynObj*)( host->handleBase + mtx ) )->type == TYPE_EVT_AUTO );

    SetEvt( host, mtx );
    return TRUE;
}

int DzCreateManualEvt( BOOL notified )
{
    DzHost* host = GetHost();
    assert( host );

    return CreateManualEvt( host, notified );
}

int DzCreateAutoEvt( BOOL notified )
{
    DzHost* host = GetHost();
    assert( host );

    return CreateAutoEvt( host, notified );
}

int DzCreateCdEvt( u_int count )
{
    DzHost* host = GetHost();
    assert( host );
    assert( (int)count >= 0 );

    return CreateCdEvt( host, count );
}

BOOL DzSetEvt( int evt )
{
    DzHost* host = GetHost();
    assert( host );
    assert( evt >= 0 );
    assert( ( evt & HANDLE_HOST_ID_MASK ) == host->hostId );
    assert(
        ( (DzSynObj*)( host->handleBase + evt ) )->type == TYPE_EVT_AUTO ||
        ( (DzSynObj*)( host->handleBase + evt ) )->type == TYPE_EVT_MANUAL ||
        ( (DzSynObj*)( host->handleBase + evt ) )->type == TYPE_EVT_COUNT
        );

    SetEvt( host, evt );
    return TRUE;
}

BOOL DzResetEvt( int evt )
{
    DzHost* host = GetHost();
    assert( host );
    assert( evt >= 0 );
    assert( ( evt & HANDLE_HOST_ID_MASK ) == host->hostId );
    assert(
        ( (DzSynObj*)( host->handleBase + evt ) )->type == TYPE_EVT_AUTO ||
        ( (DzSynObj*)( host->handleBase + evt ) )->type == TYPE_EVT_MANUAL ||
        ( (DzSynObj*)( host->handleBase + evt ) )->type == TYPE_EVT_COUNT
        );

    ResetEvt( host, evt );
    return TRUE;
}

int DzCreateSem( int count )
{
    DzHost* host = GetHost();
    assert( host );
    assert( count >= 0 );

    return CreateSem( host, count );
}

int DzReleaseSem( int sem, int count )
{
    DzHost* host = GetHost();
    assert( host );
    assert( sem >= 0 );
    assert( ( sem & HANDLE_HOST_ID_MASK ) == host->hostId );
    assert( ( (DzSynObj*)( host->handleBase + sem ) )->type == TYPE_SEM );
    assert( count > 0 );
    assert(
        ( (DzSynObj*)( host->handleBase + sem ) )->notifyCount + count >
        ( (DzSynObj*)( host->handleBase + sem ) )->notifyCount
        );

    return ReleaseSem( host, sem, count );
}

int DzCloneSynObj( int obj )
{
    DzHost* host = GetHost();
    assert( host );
    assert( obj >= 0 );
    assert( ( obj & HANDLE_HOST_ID_MASK ) == host->hostId );
    assert( ( (DzSynObj*)( host->handleBase + obj ) )->type <= TYPE_MAX_USER_CAN_WAIT );

    return CloneSynObj( host, obj );
}

BOOL DzCloseSynObj( int obj )
{
    DzHost* host = GetHost();
    assert( host );
    assert( obj >= 0 );
    assert( ( obj & HANDLE_HOST_ID_MASK ) == host->hostId );
    assert(
        ( (DzSynObj*)( host->handleBase + obj ) )->type != TYPE_TIMER &&
        ( (DzSynObj*)( host->handleBase + obj ) )->type != TYPE_CALLBACK_TIMER
        );

    CloseSynObj( host, obj );
    return TRUE;
}

int DzCreateTimer( int milSec, int repeat )
{
    DzHost* host = GetHost();
    assert( host );
    assert( milSec > 0 );
    assert( repeat >= 0 );
    assert( host->timerCount < TIME_HEAP_SIZE );

    return CreateTimer( host, milSec, repeat );
}

BOOL DzCloseTimer( int timer )
{
    DzHost* host = GetHost();
    assert( host );
    assert( timer >= 0 );
    assert( ( timer & HANDLE_HOST_ID_MASK ) == host->hostId );
    assert( ( (DzSynObj*)( host->handleBase + timer ) )->type == TYPE_TIMER );

    CloseTimer( host, timer );
    return TRUE;
}

int DzCreateCallbackTimer(
    int             milSec,
    int             repeat,
    DzRoutine       callback,
    intptr_t        context,
    int             priority,
    int             sSize
    )
{
    DzHost* host = GetHost();
    assert( host );
    if( priority == CP_DEFAULT ){
        priority = host->dftPri;
    }
    if( sSize == SS_DEFAULT ){
        sSize = host->dftSSize;
    }
    assert( milSec > 0 );
    assert( repeat >= 0 );
    assert( host->timerCount < TIME_HEAP_SIZE );
    assert( callback );
    assert( priority >= CP_FIRST && priority <= host->lowestPri );
    assert( sSize >= SS_FIRST && sSize < STACK_SIZE_COUNT );

    return CreateCallbackTimer( host, milSec, repeat, callback, context, priority, sSize );
}

BOOL DzCloseCallbackTimer( int timer )
{
    DzHost* host = GetHost();
    assert( host );
    assert( timer >= 0 );
    assert( ( timer & HANDLE_HOST_ID_MASK ) == host->hostId );
    assert( ( (DzSynObj*)( host->handleBase + timer ) )->type == TYPE_CALLBACK_TIMER );

    CloseCallbackTimer( host, timer );
    return TRUE;
}

void DzSleep( int milSec )
{
    DzHost* host = GetHost();
    assert( host );
    assert( milSec >= 0 );
    assert( milSec == 0 || host->timerCount < TIME_HEAP_SIZE );

    if( milSec > 0 ){
        DelayCurrCot( host, milSec );
    }else{
        DispatchCurrCot( host );
    }
}

int DzOpenFileA( const char* fileName, int flags )
{
    DzHost* host = GetHost();
    assert( host );

    return OpenA( host, fileName, flags );
}

int DzCloseFile( int fd )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return Close( host, fd );
}

ssize_t DzReadFile( int fd, void* buf, size_t count )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return Read( host, fd, buf, count );
}

ssize_t DzWriteFile( int fd, const void* buf, size_t count )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return Write( host, fd, buf, count );
}

size_t DzSeekFile( int fd, ssize_t offset, int whence )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return Seek( host, fd, offset, whence );
}

size_t DzGetFileSize( int fd )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return FileSize( host, fd );
}

int DzSocket( int domain, int type, int protocol )
{
    DzHost* host = GetHost();
    assert( host );

    return Socket( host, domain, type, protocol );
}

int DzCloseSocket( int fd )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return CloseSocket( host, fd );
}

int DzGetSockOpt( int fd, int level, int name, void* option, int* len )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return GetSockOpt( host, fd, level, name, option, len );
}

int DzSetSockOpt( int fd, int level, int name, const void* option, int len )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );
    assert( !( level == SOL_SOCKET && name == SO_LINGER ) );

    return SetSockOpt( host, fd, level, name, option, len );
}

int DzGetSockName( int fd, struct sockaddr* addr, int* addrLen )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return GetSockName( host, fd, addr, addrLen );
}

int DzGetPeerName( int fd, struct sockaddr* addr, int* addrLen )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return GetPeerName( host, fd, addr, addrLen );
}

int DzBind( int fd, struct sockaddr* addr, int addrLen )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return Bind( host, fd, addr, addrLen );
}

int DzListen( int fd, int backlog )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return Listen( host, fd, backlog );
}

int DzShutdown( int fd, int how )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return Shutdown( host, fd, how );
}

int DzConnect( int fd, struct sockaddr* addr, int addrLen )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return Connect( host, fd, addr, addrLen );
}

int DzAccept( int fd, struct sockaddr* addr, int* addrLen )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return Accept( host, fd, addr, addrLen );
}

int DzSendEx( int fd, DzBuf* bufs, u_int bufCount, int flags )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );
    assert( bufCount > 0 && bufCount <= DZ_MAX_IOV );

    return SendEx( host, fd, bufs, bufCount, flags );
}

int DzRecvEx( int fd, DzBuf* bufs, u_int bufCount, int flags )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );
    assert( bufCount > 0 && bufCount <= DZ_MAX_IOV );

    return RecvEx( host, fd, bufs, bufCount, flags );
}

int DzSend( int fd, const void* buf, u_int len, int flags )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return Send( host, fd, buf, len, flags );
}

int DzRecv( int fd, void* buf, u_int len, int flags )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return Recv( host, fd, buf, len, flags );
}

int DzSendToEx(
    int                     fd,
    DzBuf*                  bufs,
    u_int                   bufCount,
    int                     flags,
    const struct sockaddr*  to,
    int                     tolen
    )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );
    assert( bufCount > 0 && bufCount <= DZ_MAX_IOV );

    return SendToEx( host, fd, bufs, bufCount, flags, to, tolen );
}

int DzRecvFromEx(
    int                     fd,
    DzBuf*                  bufs,
    u_int                   bufCount,
    int                     flags,
    struct sockaddr*        from,
    int*                    fromlen
    )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );
    assert( bufCount > 0 && bufCount <= DZ_MAX_IOV );

    return RecvFromEx( host, fd, bufs, bufCount, flags, from, fromlen );
}

int DzSendTo(
    int                     fd,
    const void*             buf,
    u_int                   len,
    int                     flags,
    const struct sockaddr*  to,
    int                     tolen
    )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return SendTo( host, fd, buf, len, flags, to, tolen );
}

int DzRecvFrom(
    int                     fd,
    void*                   buf,
    u_int                   len,
    int                     flags,
    struct sockaddr*        from,
    int*                    fromlen
    )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return RecvFrom( host, fd, buf, len, flags, from, fromlen );
}

int DzGetNameInfoA(
    const struct sockaddr*  sa,
    int                     salen,
    char*                   host,
    size_t                  hostlen,
    char*                   serv,
    size_t                  servlen,
    int                     flags
    )
{
    DzHost* dzHost = GetHost();
    assert( dzHost );
    assert( dzHost->servMask & ( 1 << dzHost->hostId ) );

    return DGetNameInfoA( dzHost, sa, salen, host, (int)hostlen, serv, (int)servlen, flags );
}

int DzGetAddrInfoA(
    const char*             node,
    const char*             service,
    const struct addrinfo*  hints,
    struct addrinfo**       res
    )
{
    DzHost* host = GetHost();
    assert( host );
    assert( host->servMask & ( 1 << host->hostId ) );

    return DGetAddrInfoA( host, node, service, hints, res );
}

void DzFreeAddrInfoA( struct addrinfo *res )
{
    DFreeAddrInfoA( res );
}

int DzInetPtonA( int af, const char* src, void* dst )
{
    return DInetPtonA( af, src, dst );
}

const char* DzInetNtopA( int af, const void* src, char* dst, int size )
{
    return DInetNtopA( af, src, dst, size );
}

DzParamNode* DzAllocParamNode()
{
    DzHost* host = GetHost();
    assert( host );

    return AllocLNode( host );
}

void DzFreeParamNode( DzParamNode* node )
{
    DzHost* host = GetHost();
    assert( host );
    assert( node );

    FreeLNode( host, (DzLNode*)node );
}

void* DzAllocPermanentChunk( size_t size )
{
    DzHost* host = GetHost();
    assert( host );
    assert( size % PAGE_SIZE == 0 );

    return AllocChunk( host, size );
}

void* DzMalloc( size_t size )
{
    DzHost* host = GetHost();
    assert( host );

    return Malloc( host, size );
}

void* DzCalloc( size_t num, size_t size )
{
    DzHost* host = GetHost();
    assert( host );

    return Calloc( host, num, size );
}

void* DzReAlloc( void* mem, size_t size )
{
    DzHost* host = GetHost();
    assert( host );

    return ReAlloc( host, mem, size );
}

void DzFree( void* mem )
{
    DzHost* host = GetHost();
    assert( host );

    Free( host, mem );
}

void* DzMallocEx( size_t size )
{
    DzHost* host = GetHost();
    assert( host );

    return MallocEx( host, size );
}

void DzFreeEx( void* mem )
{
    DzHost* host = GetHost();
    assert( host );

    FreeEx( host, mem );
}

unsigned long long DzUnixTime()
{
    return (unsigned long long)UnixTime();
}

unsigned long long DzMilUnixTime()
{
    return (unsigned long long)MilUnixTime();
}

int __DzMakeServMask( BOOL notServ, ... )
{
    va_list ap;
    int ret;
    int op;

    ret = notServ ? -1 : 0;
    va_start( ap, notServ );
    for( op = va_arg( ap, int ); op != -1; op = va_arg( ap, int ) ){
        if( notServ ){
            ret &= ~( 1 << op );
        }else{
            ret |= ( 1 << op );
        }
    }
    va_end( ap );
    return ret;
}

int __DzDbgLastErr()
{
    DzHost* host = GetHost();
    assert( host );

    return __DbgGetLastErr( host );
}

#ifdef __cplusplus
};
#endif
