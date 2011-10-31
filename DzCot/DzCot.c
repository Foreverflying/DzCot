/********************************************************************
    created:    2010/02/11 22:12
    file:       DzCot.c
    author:     Foreverflying
    purpose:    
*********************************************************************/

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
    intptr_t    context
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
        firstEntry, context
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
    DzSynObj*   evt,
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
    assert( evt );
    assert( evt->type >= TYPE_EVT_AUTO && evt->type <= TYPE_EVT_COUNT );
    assert( entry );
    assert( priority >= CP_FIRST && priority <= host->lowestPri );
    assert( sSize >= SS_FIRST && sSize < STACK_SIZE_COUNT );

    return EvtStartCot( host, evt, entry, context, priority, sSize );
}

int DzEvtStartCotInstant(
    DzSynObj*   evt,
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
    assert( evt );
    assert( evt->type >= TYPE_EVT_AUTO && evt->type <= TYPE_EVT_COUNT );
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
    DzHandle    evt,
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
    assert( evt );
    assert( evt->type >= TYPE_EVT_AUTO && evt->type <= TYPE_EVT_COUNT );
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

int DzWaitSynObj( DzHandle obj, int timeout )
{
    DzHost* host = GetHost();
    assert( host );
    assert( timeout <= 0 || host->timerCount < TIME_HEAP_SIZE );
    assert( obj );
    assert( obj->type <= TYPE_MAX_USER_CAN_WAIT );

    return WaitSynObj( host, obj, timeout );
}

int DzWaitMultiSynObj( int count, DzHandle* obj, BOOL waitAll, int timeout )
{
    int i;
    DzHost* host = GetHost();
    assert( host );
    assert( timeout <= 0 || host->timerCount < TIME_HEAP_SIZE );
    assert( obj );
    assert( count > 0 );
    for( i = 0; i < count; i++ ){
        assert( obj[i] && obj[i]->type <= TYPE_MAX_USER_CAN_WAIT );
    }

    return WaitMultiSynObj( host, count, obj, waitAll, timeout );
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
    assert( mtx );
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
    assert( (int)count >= 0 );

    return CreateCdEvt( host, count );
}

BOOL DzSetEvt( DzHandle evt )
{
    DzHost* host = GetHost();
    assert( host );
    assert( evt );
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
    assert( evt );
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
    assert( sem );
    assert( sem->type == TYPE_SEM );
    assert( count > 0 );
    assert( sem->notifyCount + count > sem->notifyCount );

    return ReleaseSem( host, sem, count );
}

DzHandle DzCloneSynObj( DzHandle obj )
{
    assert( GetHost() );
    assert( obj );
    assert( obj->type <= TYPE_MAX_USER_CAN_WAIT );

    return CloneSynObj( obj );
}

BOOL DzCloseSynObj( DzHandle obj )
{
    DzHost* host = GetHost();
    assert( host );
    assert( obj );
    assert( obj->type != TYPE_TIMER && obj->type != TYPE_CALLBACK_TIMER );

    CloseSynObj( host, obj );
    return TRUE;
}

DzHandle DzCreateTimer( int milSec, int repeat )
{
    DzHost* host = GetHost();
    assert( host );
    assert( milSec > 0 );
    assert( repeat >= 0 );
    assert( host->timerCount < TIME_HEAP_SIZE );

    return CreateTimer( host, milSec, repeat );
}

BOOL DzCloseTimer( DzHandle timer )
{
    DzHost* host = GetHost();
    assert( host );
    assert( timer );
    assert( timer->type == TYPE_TIMER );

    CloseTimer( host, timer );
    return TRUE;
}

DzHandle DzCreateCallbackTimer(
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

BOOL DzCloseCallbackTimer( DzHandle timer )
{
    DzHost* host = GetHost();
    assert( host );
    assert( timer );
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

int DzOpenFileA( const char* fileName, int flags )
{
    DzHost* host = GetHost();
    assert( host );

    return OpenA( host, fileName, flags );
}

int DzOpenFileW( const wchar_t* fileName, int flags )
{
    DzHost* host = GetHost();
    assert( host );

    return OpenW( host, fileName, flags );
}

int DzCloseFile( int fd )
{
    assert( GetHost() );

    return Close( fd );
}

ssize_t DzReadFile( int fd, void* buf, size_t count )
{
    DzHost* host = GetHost();
    assert( host );

    return Read( host, fd, buf, count );
}

ssize_t DzWriteFile( int fd, const void* buf, size_t count )
{
    DzHost* host = GetHost();
    assert( host );

    return Write( host, fd, buf, count );
}

size_t DzSeekFile( int fd, ssize_t offset, int whence )
{
    assert( GetHost() );

    return Seek( fd, offset, whence );
}

size_t DzGetFileSize( int fd )
{
    assert( GetHost() );

    return FileSize( fd );
}

int DzSocket( int domain, int type, int protocol )
{
    DzHost* host = GetHost();
    assert( host );

    return Socket( host, domain, type, protocol );
}

int DzCloseSocket( int fd )
{
    assert( GetHost() );

    return CloseSocket( fd );
}

int DzGetSockOpt( int fd, int level, int name, void* option, int* len )
{
    assert( GetHost() );

    return GetSockOpt( fd, level, name, option, len );
}

int DzSetSockOpt( int fd, int level, int name, const void* option, int len )
{
    assert( GetHost() );
    assert( !( level == SOL_SOCKET && name == SO_LINGER ) );

    return SetSockOpt( fd, level, name, option, len );
}

int DzGetSockName( int fd, struct sockaddr* addr, int* addrLen )
{
    assert( GetHost() );

    return GetSockName( fd, addr, addrLen );
}

int DzBind( int fd, struct sockaddr* addr, int addrLen )
{
    assert( GetHost() );

    return Bind( fd, addr, addrLen );
}

int DzListen( int fd, int backlog )
{
    assert( GetHost() );

    return Listen( fd, backlog );
}

int DzShutdown( int fd, int how )
{
    assert( GetHost() );

    return Shutdown( fd, how );
}

int DzConnect( int fd, struct sockaddr* addr, int addrLen )
{
    DzHost* host = GetHost();
    assert( host );

    return Connect( host, fd, addr, addrLen );
}

int DzAccept( int fd, struct sockaddr* addr, int* addrLen )
{
    DzHost* host = GetHost();
    assert( host );

    return Accept( host, fd, addr, addrLen );
}

int DzSendEx( int fd, DzBuf* bufs, u_int bufCount, int flags )
{
    DzHost* host = GetHost();
    assert( host );
    assert( bufCount > 0 && bufCount <= DZ_MAX_IOV );

    return SendEx( host, fd, bufs, bufCount, flags );
}

int DzRecvEx( int fd, DzBuf* bufs, u_int bufCount, int flags )
{
    DzHost* host = GetHost();
    assert( host );
    assert( bufCount > 0 && bufCount <= DZ_MAX_IOV );

    return RecvEx( host, fd, bufs, bufCount, flags );
}

int DzSend( int fd, const void* buf, u_int len, int flags )
{
    DzHost* host = GetHost();
    assert( host );

    return Send( host, fd, buf, len, flags );
}

int DzRecv( int fd, void* buf, u_int len, int flags )
{
    DzHost* host = GetHost();
    assert( host );

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

    return RecvFrom( host, fd, buf, len, flags, from, fromlen );
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
