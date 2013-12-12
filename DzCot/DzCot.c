/**
 *  @file       DzCot.c
 *  @brief      
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

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
    int         smallStackSize,
    int         middleStackSize,
    int         largeStackSize,
    int         lowestPri,
    int         dftPri,
    int         dftSType,
    DzEntry     firstEntry,
    intptr_t    context,
    DzEntry     cleanEntry
    )
{
    assert( !GetHost() );
    assert( hostCount >= 0 && hostCount <= DZ_MAX_HOST );
    assert( smallStackSize > 0 && smallStackSize <= DZ_MAX_STACK_SIZE );
    assert( middleStackSize > 0 && middleStackSize <= DZ_MAX_STACK_SIZE );
    assert( largeStackSize > 0 && largeStackSize <= DZ_MAX_STACK_SIZE );
    assert(
        lowestPri >= CP_FIRST &&
        lowestPri < COT_PRIORITY_COUNT
        );
    assert(
        dftPri >= CP_FIRST &&
        dftPri <= lowestPri
        );
    assert(
        dftSType >= ST_FIRST &&
        dftSType < STACK_TYPE_COUNT
        );
    assert( firstEntry );

    return RunHosts(
        hostCount, smallStackSize, middleStackSize, largeStackSize,
        lowestPri, dftPri, dftSType, firstEntry, context, cleanEntry
        );
}

int DzStartCot(
    DzEntry     entry,
    intptr_t    context,
    int         priority,
    int         sType
    )
{
    DzHost* host = GetHost();
    assert( host );
    if( priority == CP_DEFAULT ){
        priority = host->dftPri;
    }
    if( sType == ST_DEFAULT ){
        sType = host->dftSType;
    }
    assert( entry );
    assert( priority >= CP_FIRST && priority <= host->lowestPri );
    assert( sType >= ST_FIRST && sType < STACK_TYPE_COUNT );

    return StartCot( host, entry, context, priority, sType );
}

int DzStartCotInstant(
    DzEntry     entry,
    intptr_t    context,
    int         priority,
    int         sType
    )
{
    DzHost* host = GetHost();
    assert( host );
    if( priority == CP_DEFAULT ){
        priority = host->dftPri;
    }
    if( sType == ST_DEFAULT ){
        sType = host->dftSType;
    }
    assert( entry );
    assert( priority >= CP_FIRST && priority <= host->lowestPri );
    assert( sType >= ST_FIRST && sType < STACK_TYPE_COUNT );

    return StartCotInstant( host, entry, context, priority, sType );
}

int DzEvtStartCot(
    DzHandle    evt,
    DzEntry     entry,
    intptr_t    context,
    int         priority,
    int         sType
    )
{
    DzHost* host = GetHost();
    assert( host );
    if( priority == CP_DEFAULT ){
        priority = host->dftPri;
    }
    if( sType == ST_DEFAULT ){
        sType = host->dftSType;
    }
    assert( evt->type >= TYPE_EVT_AUTO && evt->type <= TYPE_EVT_COUNT );
    assert( entry );
    assert( priority >= CP_FIRST && priority <= host->lowestPri );
    assert( sType >= ST_FIRST && sType < STACK_TYPE_COUNT );

    return EvtStartCot( host, evt, entry, context, priority, sType );
}

int DzEvtStartCotInstant(
    DzHandle    evt,
    DzEntry     entry,
    intptr_t    context,
    int         priority,
    int         sType
    )
{
    DzHost* host = GetHost();
    assert( host );
    if( priority == CP_DEFAULT ){
        priority = host->dftPri;
    }
    if( sType == ST_DEFAULT ){
        sType = host->dftSType;
    }
    assert( evt->type >= TYPE_EVT_AUTO && evt->type <= TYPE_EVT_COUNT );
    assert( entry );
    assert( priority >= CP_FIRST && priority <= host->lowestPri );
    assert( sType >= ST_FIRST && sType < STACK_TYPE_COUNT );

    return EvtStartCotInstant( host, evt, entry, context, priority, sType );
}

int DzStartRemoteCot(
    int         rmtId,
    DzEntry     entry,
    intptr_t    context,
    int         priority,
    int         sType
    )
{
    DzHost* host = GetHost();
    assert( host );
    if( priority == CP_DEFAULT ){
        priority = host->dftPri;
    }
    if( sType == ST_DEFAULT ){
        sType = host->dftSType;
    }
    assert( rmtId >= 0 && rmtId < host->hostCount );
    assert( host->hostId != rmtId );
    assert( entry );
    assert( priority >= CP_FIRST && priority <= host->lowestPri );
    assert( sType >= ST_FIRST && sType < STACK_TYPE_COUNT );

    return StartRemoteCot( host, rmtId, entry, context, priority, sType );
}

int DzEvtStartRemoteCot(
    DzHandle    evt,
    int         rmtId,
    DzEntry     entry,
    intptr_t    context,
    int         priority,
    int         sType
    )
{
    DzHost* host = GetHost();
    assert( host );
    if( priority == CP_DEFAULT ){
        priority = host->dftPri;
    }
    if( sType == ST_DEFAULT ){
        sType = host->dftSType;
    }
    assert( evt->type >= TYPE_EVT_AUTO && evt->type <= TYPE_EVT_COUNT );
    assert( rmtId >= 0 && rmtId < host->hostCount );
    assert( host->hostId != rmtId );
    assert( entry );
    assert( priority >= CP_FIRST && priority <= host->lowestPri );
    assert( sType >= ST_FIRST && sType < STACK_TYPE_COUNT );

    return EvtStartRemoteCot( host, evt, rmtId, entry, context, priority, sType );
}

int DzRunRemoteCot(
    int         rmtId,
    DzEntry     entry,
    intptr_t    context,
    int         priority,
    int         sType
    )
{
    DzHost* host = GetHost();
    assert( host );
    if( priority == CP_DEFAULT ){
        priority = host->dftPri;
    }
    if( sType == ST_DEFAULT ){
        sType = host->dftSType;
    }
    assert( rmtId >= 0 && rmtId < host->hostCount );
    assert( host->hostId != rmtId );
    assert( entry );
    assert( priority >= CP_FIRST && priority <= host->lowestPri );
    assert( sType >= ST_FIRST && sType < STACK_TYPE_COUNT );

    return RunRemoteCot( host, rmtId, entry, context, priority, sType );
}

int DzRunWorker( DzEntry entry, intptr_t context )
{
    DzHost* host = GetHost();
    assert( host );
    assert( entry );

    return RunWorker( host, entry, context );
}

intptr_t DzGetCotData()
{
    DzHost* host = GetHost();
    assert( host );

    return GetCotData( host );
}

void DzSetCotData( intptr_t data )
{
    DzHost* host = GetHost();
    assert( host );

    SetCotData( host, data );
}

int DzGetCotCount()
{
    DzHost* host = GetHost();
    assert( host );

    return GetCotCount( host );
}

int DzGetHostId()
{
    DzHost* host = GetHost();
    assert( host );

    return GetHostId( host );
}

int DzSetPriority( int priority )
{
    DzHost* host = GetHost();
    assert( host );
    assert( priority <= host->lowestPri );

    return SetCurrCotPriority( host, priority );
}

int DzSetCotPoolDepth( int sType, int depth )
{
    DzHost* host = GetHost();
    assert( host );
    assert( sType >= ST_FIRST && sType < STACK_TYPE_COUNT );
    assert( depth <= DZ_MAX_COT_POOL_DEPTH );

    return SetCotPoolDepth( host, sType, depth );
}

int DzSetWorkerPoolDepth( int depth )
{
    DzHost* host = GetHost();
    assert( host );
    assert( depth >= 0 && depth <= DZ_MAX_WORKER_POOL_DEPTH );

    return SetWorkerPoolDepth( host, depth );
}

int DzSetHostParam( int lowestPri, int dftPri, int dftSType )
{
    DzHost* host = GetHost();
    assert( host );
    assert(
        lowestPri >= CP_FIRST &&
        lowestPri < COT_PRIORITY_COUNT
        );
    assert( dftPri <= lowestPri );
    assert( dftSType < STACK_TYPE_COUNT );

    return SetHostParam( host, lowestPri, dftPri, dftSType );
}

int DzSetHostIoReaction( int rate )
{
    DzHost* host = GetHost();
    assert( host );

    return SetHostIoReaction( host, rate );
}

int DzWaitSynObj( DzHandle obj, int timeout )
{
    DzHost* host = GetHost();
    assert( host );
    assert( obj->type <= TYPE_MAX_USER_CAN_WAIT );
    assert( timeout <= 0 || host->timerCount < TIME_HEAP_SIZE );

    return WaitSynObj( host, obj, timeout );
}

int DzWaitMultiSynObj( int count, DzHandle* objs, BOOL waitAll, int timeout )
{
    int i;
    DzHost* host = GetHost();
    assert( host );
    assert( timeout <= 0 || host->timerCount < TIME_HEAP_SIZE );
    assert( objs );
    assert( count > 0 );
    for( i = 0; i < count; i++ ){
        assert( objs[i]->type <= TYPE_MAX_USER_CAN_WAIT );
    }

    return WaitMultiSynObj( host, count, objs, waitAll, timeout );
}

DzHandle DzCreateMtx( BOOL owner )
{
    DzHost* host = GetHost();
    assert( host );

    return CreateAutoEvt( host, !owner );
}

BOOL DzReleaseMtx( DzHandle mtx )
{
    DzHost* host = GetHost();
    assert( host );
    assert( mtx->type == TYPE_EVT_AUTO );

    SetEvt( host, mtx );
    return TRUE;
}

DzHandle DzCreateManualEvt( BOOL notified )
{
    DzHost* host = GetHost();
    assert( host );

    return CreateManualEvt( host, notified );
}

DzHandle DzCreateAutoEvt( BOOL notified )
{
    DzHost* host = GetHost();
    assert( host );

    return CreateAutoEvt( host, notified );
}

DzHandle DzCreateCdEvt( u_int count )
{
    DzHost* host = GetHost();
    assert( host );
    assert( count >= 0 );

    return CreateCdEvt( host, count );
}

BOOL DzSetEvt( DzHandle evt )
{
    DzHost* host = GetHost();
    assert( host );
    assert(
        evt->type == TYPE_EVT_AUTO ||
        evt->type == TYPE_EVT_MANUAL ||
        evt->type == TYPE_EVT_COUNT
        );

    SetEvt( host, evt );
    return TRUE;
}

BOOL DzResetEvt( DzHandle evt )
{
    assert( GetHost() );
    assert(
        evt->type == TYPE_EVT_AUTO ||
        evt->type == TYPE_EVT_MANUAL ||
        evt->type == TYPE_EVT_COUNT
        );

    ResetEvt( evt );
    return TRUE;
}

DzHandle DzCreateSem( int count )
{
    DzHost* host = GetHost();
    assert( host );
    assert( count >= 0 );

    return CreateSem( host, count );
}

int DzReleaseSem( DzHandle sem, int count )
{
    DzHost* host = GetHost();
    assert( host );
    assert( sem->type == TYPE_SEM );
    assert( count > 0 );
    assert( sem->notifyCount + count > sem->notifyCount );

    return ReleaseSem( host, sem, count );
}

DzHandle DzCreateTimer( int milSec, BOOL repeat )
{
    DzHost* host = GetHost();
    assert( host );
    assert( milSec > 0 );
    assert( host->timerCount < TIME_HEAP_SIZE );

    return CreateTimer( host, milSec, repeat );
}

DzHandle DzCloneSynObj( DzHandle obj )
{
    assert( GetHost() );
    assert( obj->type <= TYPE_MAX_USER_CAN_WAIT );

    return CloneSynObj( obj );
}

BOOL DzDelSynObj( DzHandle obj )
{
    DzHost* host = GetHost();
    assert( host );
    assert( obj->type != TYPE_CALLBACK_TIMER );

    DelSynObj( host, obj );
    return TRUE;
}

DzHandle DzCreateCallbackTimer(
    int             milSec,
    BOOL            repeat,
    DzEntry         callback,
    intptr_t        context,
    int             priority,
    int             sType
    )
{
    DzHost* host = GetHost();
    assert( host );
    if( priority == CP_DEFAULT ){
        priority = host->dftPri;
    }
    if( sType == ST_DEFAULT ){
        sType = host->dftSType;
    }
    assert( milSec > 0 );
    assert( host->timerCount < TIME_HEAP_SIZE );
    assert( callback );
    assert( priority >= CP_FIRST && priority <= host->lowestPri );
    assert( sType >= ST_FIRST && sType < STACK_TYPE_COUNT );

    return CreateCallbackTimer( host, milSec, repeat, callback, context, priority, sType );
}

BOOL DzDelCallbackTimer( DzHandle timer )
{
    DzHost* host = GetHost();
    assert( host );
    assert( timer->type == TYPE_CALLBACK_TIMER );

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

int DzSocket( int domain, int type, int protocol )
{
    DzHost* host = GetHost();
    assert( host );

    return Socket( host, domain, type, protocol );
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

int DzBind( int fd, const struct sockaddr* addr, int addrLen )
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

int DzConnect( int fd, const struct sockaddr* addr, int addrLen )
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

int DzGetNameInfo(
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

    return DGetNameInfo( dzHost, sa, salen, host, (int)hostlen, serv, (int)servlen, flags );
}

int DzGetAddrInfo(
    const char*             node,
    const char*             service,
    const struct addrinfo*  hints,
    struct addrinfo**       res
    )
{
    DzHost* host = GetHost();
    assert( host );

    return DGetAddrInfo( host, node, service, hints, res );
}

void DzFreeAddrInfo( struct addrinfo *res )
{
    DFreeAddrInfo( res );
}

int DzInetPton( int af, const char* src, void* dst )
{
    return DInetPton( af, src, dst );
}

const char* DzInetNtop( int af, const void* src, char* dst, int size )
{
    return DInetNtop( af, src, dst, size );
}

int DzOpen( const char* fileName, int flags )
{
    DzHost* host = GetHost();
    assert( host );

    return Open( host, fileName, flags );
}

ssize_t DzRead( int fd, void* buf, size_t count )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return Read( host, fd, buf, count );
}

ssize_t DzWrite( int fd, const void* buf, size_t count )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return Write( host, fd, buf, count );
}

size_t DzSeek( int fd, ssize_t offset, int whence )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return Seek( host, fd, offset, whence );
}

size_t DzFileSize( int fd )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return FileSize( host, fd );
}

intptr_t DzGetFdData( int fd )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return GetFdData( host, fd );
}

void DzSetFdData( int fd, intptr_t data )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    SetFdData( host, fd, data );
}

int DzClose( int fd )
{
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( ( fd & HANDLE_HOST_ID_MASK ) == host->hostId );

    return Close( host, fd );
}

DzParamNode* DzAllocParamNode()
{
    DzParamNode* ret;
    DzHost* host = GetHost();
    assert( host );

    ret = AllocLNode( host );
    __Dbg( AllocParamNode )( host, ret );
    return ret;
}

void DzFreeParamNode( DzParamNode* node )
{
    DzHost* host = GetHost();
    assert( host );
    assert( node );

    __Dbg( FreeParamNode )( host, node );
    FreeLNode( host, (DzLNode*)node );
}

void* DzAllocPermanentChunk( size_t size )
{
    DzHost* host = GetHost();
    assert( host );
    assert( size % MIN_PERMANENT_CHUNK_SIZE == 0 );

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
    DzHost* host = GetHost();
    assert( host );

    return (unsigned long long)MilUnixTime( host );
}

unsigned long long DzLatestMilUnixTime()
{
    DzHost* host = GetHost();
    assert( host );

    return (unsigned long long)host->latestMilUnixTime;
}

int __DzDbgLastErr()
{
    DzHost* host = GetHost();
    assert( host );

    return __Dbg( GetLastErr )( host );
}

int __DzDbgMaxStackUse( int sType )
{
    DzHost* host = GetHost();
    assert( host );
    assert( sType >= ST_FIRST && sType < STACK_TYPE_COUNT );

    return __Dbg( GetMaxStackUse )( host, sType );
}

int __DzDbgSynObjLeak()
{
    DzHost* host = GetHost();
    assert( host );

    return __Dbg( GetSynObjLeak )( host );
}

int __DzDbgFdLeak()
{
    DzHost* host = GetHost();
    assert( host );

    return __Dbg( GetFdLeak )( host );
}

int __DzDbgHeapLeak()
{
    DzHost* host = GetHost();
    assert( host );

    return __Dbg( GetHeapLeak )( host );
}

int __DzDbgParamNodeLeak()
{
    DzHost* host = GetHost();
    assert( host );

    return __Dbg( GetParamNodeLeak )( host );
}

#ifdef __cplusplus
};
#endif
