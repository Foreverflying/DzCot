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
    int         evt,
    DzRoutine   entry,
    intptr_t    context,
    int         priority,
    int         sSize
    )
{
    DzSynObj* obj;
    DzHost* host = GetHost();
    assert( host );
    assert( evt >= 0 );
    assert( evt >> HANDLE_HOST_ID_SHIFT == host->hostId );
    if( priority == CP_DEFAULT ){
        priority = host->dftPri;
    }
    if( sSize == SS_DEFAULT ){
        sSize = host->dftSSize;
    }
    obj = (DzSynObj*)( host->handleBase + ( evt & HANDLE_ADDR_MASK ) );
    assert( obj->type >= TYPE_EVT_AUTO && obj->type <= TYPE_EVT_COUNT );
    assert( entry );
    assert( priority >= CP_FIRST && priority <= host->lowestPri );
    assert( sSize >= SS_FIRST && sSize < STACK_SIZE_COUNT );

    return EvtStartCot( host, obj, entry, context, priority, sSize );
}

int DzEvtStartCotInstant(
    int         evt,
    DzRoutine   entry,
    intptr_t    context,
    int         priority,
    int         sSize
    )
{
    DzSynObj* obj;
    DzHost* host = GetHost();
    assert( host );
    assert( evt >= 0 );
    assert( evt >> HANDLE_HOST_ID_SHIFT == host->hostId );
    if( priority == CP_DEFAULT ){
        priority = host->dftPri;
    }
    if( sSize == SS_DEFAULT ){
        sSize = host->dftSSize;
    }
    obj = (DzSynObj*)( host->handleBase + ( evt & HANDLE_ADDR_MASK ) );
    assert( obj->type >= TYPE_EVT_AUTO && obj->type <= TYPE_EVT_COUNT );
    assert( entry );
    assert( priority >= CP_FIRST && priority <= host->lowestPri );
    assert( sSize >= SS_FIRST && sSize < STACK_SIZE_COUNT );

    return EvtStartCotInstant( host, obj, entry, context, priority, sSize );
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
    DzSynObj* obj;
    DzHost* host = GetHost();
    assert( host );
    assert( evt >= 0 );
    assert( evt >> HANDLE_HOST_ID_SHIFT == host->hostId );
    if( priority == CP_DEFAULT ){
        priority = host->dftPri;
    }
    if( sSize == SS_DEFAULT ){
        sSize = host->dftSSize;
    }
    obj = (DzSynObj*)( host->handleBase + ( evt & HANDLE_ADDR_MASK ) );
    assert( obj->type >= TYPE_EVT_AUTO && obj->type <= TYPE_EVT_COUNT );
    assert( rmtId >= 0 && rmtId < host->hostCount );
    assert( host->hostId != rmtId );
    assert( host->servMask & ( 1 << rmtId ) );
    assert( entry );
    assert( priority >= CP_FIRST && priority <= host->lowestPri );
    assert( sSize >= SS_FIRST && sSize < STACK_SIZE_COUNT );

    return EvtStartRemoteCot( host, obj, rmtId, entry, context, priority, sSize );
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
    DzSynObj* synObj;
    DzHost* host = GetHost();
    assert( host );
    assert( obj >= 0 );
    assert( obj >> HANDLE_HOST_ID_SHIFT == host->hostId );
    synObj = (DzSynObj*)( host->handleBase + ( obj & HANDLE_ADDR_MASK ) );
    assert( synObj->type <= TYPE_MAX_USER_CAN_WAIT );
    assert( timeout <= 0 || host->timerCount < TIME_HEAP_SIZE );

    return WaitSynObj( host, synObj, timeout );
}

int DzWaitMultiSynObj( int count, int* objs, BOOL waitAll, int timeout )
{
    int i;
    DzSynObj** synObjs;
    DzHost* host = GetHost();
    assert( host );
    assert( timeout <= 0 || host->timerCount < TIME_HEAP_SIZE );
    assert( objs );
    assert( count > 0 );
    synObjs = (DzSynObj**)alloca( sizeof( DzSynObj* ) * count );
    for( i = 0; i < count; i++ ){
        assert( objs[i] >= 0 );
        assert( objs[i] >> HANDLE_HOST_ID_SHIFT == host->hostId );
        synObjs[i] = (DzSynObj*)( host->handleBase + ( objs[i] & HANDLE_ADDR_MASK ) );
        assert( synObjs[i]->type <= TYPE_MAX_USER_CAN_WAIT );
    }

    return WaitMultiSynObj( host, count, synObjs, waitAll, timeout );
}

int DzCreateMtx( BOOL owner )
{
    intptr_t obj;
    DzHost* host = GetHost();
    assert( host );

    obj = (intptr_t)CreateAutoEvt( host, !owner );
    if( !obj ){
        return -1;
    }
    return (int)( obj - host->handleBase ) | host->hostId << HANDLE_HOST_ID_SHIFT;
}

BOOL DzReleaseMtx( int mtx )
{
    DzSynObj* obj;
    DzHost* host = GetHost();
    assert( host );
    assert( mtx >= 0 );
    assert( mtx >> HANDLE_HOST_ID_SHIFT == host->hostId );
    obj = (DzSynObj*)( host->handleBase + ( mtx & HANDLE_ADDR_MASK ) );
    assert( obj->type == TYPE_EVT_AUTO );

    SetEvt( host, obj );
    return TRUE;
}

int DzCreateManualEvt( BOOL notified )
{
    intptr_t obj;
    DzHost* host = GetHost();
    assert( host );

    obj = (intptr_t)CreateManualEvt( host, notified );
    if( !obj ){
        return -1;
    }
    return (int)( obj - host->handleBase ) | host->hostId << HANDLE_HOST_ID_SHIFT;
}

int DzCreateAutoEvt( BOOL notified )
{
    intptr_t obj;
    DzHost* host = GetHost();
    assert( host );

    obj = (intptr_t)CreateAutoEvt( host, notified );
    if( !obj ){
        return -1;
    }
    return (int)( obj - host->handleBase ) | host->hostId << HANDLE_HOST_ID_SHIFT;
}

int DzCreateCdEvt( u_int count )
{
    intptr_t obj;
    DzHost* host = GetHost();
    assert( host );
    assert( (int)count >= 0 );

    obj = (intptr_t)CreateCdEvt( host, count );
    if( !obj ){
        return -1;
    }
    return (int)( obj - host->handleBase ) | host->hostId << HANDLE_HOST_ID_SHIFT;
}

BOOL DzSetEvt( int evt )
{
    DzSynObj* obj;
    DzHost* host = GetHost();
    assert( host );
    assert( evt >= 0 );
    assert( evt >> HANDLE_HOST_ID_SHIFT == host->hostId );
    obj = (DzSynObj*)( host->handleBase + ( evt & HANDLE_ADDR_MASK ) );
    assert(
        obj->type == TYPE_EVT_AUTO ||
        obj->type == TYPE_EVT_MANUAL ||
        obj->type == TYPE_EVT_COUNT
        );

    SetEvt( host, obj );
    return TRUE;
}

BOOL DzResetEvt( int evt )
{
    DzSynObj* obj;
    DzHost* host = GetHost();
    assert( host );
    assert( evt >= 0 );
    assert( evt >> HANDLE_HOST_ID_SHIFT == host->hostId );
    obj = (DzSynObj*)( host->handleBase + ( evt & HANDLE_ADDR_MASK ) );
    assert(
        obj->type == TYPE_EVT_AUTO ||
        obj->type == TYPE_EVT_MANUAL ||
        obj->type == TYPE_EVT_COUNT
        );

    ResetEvt( obj );
    return TRUE;
}

int DzCreateSem( int count )
{
    intptr_t obj;
    DzHost* host = GetHost();
    assert( host );
    assert( count >= 0 );

    obj = (intptr_t)CreateSem( host, count );
    if( !obj ){
        return -1;
    }
    return (int)( obj - host->handleBase ) | host->hostId << HANDLE_HOST_ID_SHIFT;
}

int DzReleaseSem( int sem, int count )
{
    DzSynObj* obj;
    DzHost* host = GetHost();
    assert( host );
    assert( sem >= 0 );
    assert( sem >> HANDLE_HOST_ID_SHIFT == host->hostId );
    obj = (DzSynObj*)( host->handleBase + ( sem & HANDLE_ADDR_MASK ) );
    assert( obj->type == TYPE_SEM );
    assert( count > 0 );
    assert( obj->notifyCount + count > obj->notifyCount );

    return ReleaseSem( host, obj, count );
}

int DzCloneSynObj( int obj )
{
    DzSynObj* synObj;
    DzHost* host = GetHost();
    assert( host );
    assert( obj >= 0 );
    assert( obj >> HANDLE_HOST_ID_SHIFT == host->hostId );
    synObj = (DzSynObj*)( host->handleBase + ( obj & HANDLE_ADDR_MASK ) );
    assert( synObj->type <= TYPE_MAX_USER_CAN_WAIT );

    CloneSynObj( synObj );
    return obj;
}

BOOL DzCloseSynObj( int obj )
{
    DzSynObj* synObj;
    DzHost* host = GetHost();
    assert( host );
    assert( obj >= 0 );
    assert( obj >> HANDLE_HOST_ID_SHIFT == host->hostId );
    synObj = (DzSynObj*)( host->handleBase + ( obj & HANDLE_ADDR_MASK ) );
    assert( synObj->type != TYPE_TIMER && synObj->type != TYPE_CALLBACK_TIMER );

    CloseSynObj( host, synObj );
    return TRUE;
}

int DzCreateTimer( int milSec, int repeat )
{
    intptr_t obj;
    DzHost* host = GetHost();
    assert( host );
    assert( milSec > 0 );
    assert( repeat >= 0 );
    assert( host->timerCount < TIME_HEAP_SIZE );

    obj = (intptr_t)CreateTimer( host, milSec, repeat );
    if( !obj ){
        return -1;
    }
    return (int)( obj - host->handleBase ) | host->hostId << HANDLE_HOST_ID_SHIFT;
}

BOOL DzCloseTimer( int timer )
{
    DzSynObj* obj;
    DzHost* host = GetHost();
    assert( host );
    assert( timer >= 0 );
    assert( timer >> HANDLE_HOST_ID_SHIFT == host->hostId );
    obj = (DzSynObj*)( host->handleBase + ( timer & HANDLE_ADDR_MASK ) );
    assert( obj->type == TYPE_TIMER );

    CloseTimer( host, obj );
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
    intptr_t obj;
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

    obj = (intptr_t)CreateCallbackTimer( host, milSec, repeat, callback, context, priority, sSize );
    if( !obj ){
        return -1;
    }
    return (int)( obj - host->handleBase ) | host->hostId << HANDLE_HOST_ID_SHIFT;
}

BOOL DzCloseCallbackTimer( int timer )
{
    DzSynObj* obj;
    DzHost* host = GetHost();
    assert( host );
    assert( timer >= 0 );
    assert( timer >> HANDLE_HOST_ID_SHIFT == host->hostId );
    obj = (DzSynObj*)( host->handleBase + ( timer & HANDLE_ADDR_MASK ) );
    assert( obj->type == TYPE_CALLBACK_TIMER );

    CloseCallbackTimer( host, obj );
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
    intptr_t ret;
    DzHost* host = GetHost();
    assert( host );

    ret = (intptr_t)OpenA( host, fileName, flags );
    if( !ret ){
        return -1;
    }
    return (int)( ret - host->handleBase ) | host->hostId << HANDLE_HOST_ID_SHIFT;
}

int DzOpenFileW( const wchar_t* fileName, int flags )
{
    intptr_t ret;
    DzHost* host = GetHost();
    assert( host );

    ret = (intptr_t)OpenW( host, fileName, flags );
    if( !ret ){
        return -1;
    }
    return (int)( ret - host->handleBase ) | host->hostId << HANDLE_HOST_ID_SHIFT;
}

int DzCloseFile( int fd )
{
    DzFd* dzFd;
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( fd >> HANDLE_HOST_ID_SHIFT == host->hostId );
    dzFd = (DzFd*)( host->handleBase + ( fd & HANDLE_ADDR_MASK ) );

    return Close( host, dzFd );
}

ssize_t DzReadFile( int fd, void* buf, size_t count )
{
    DzFd* dzFd;
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( fd >> HANDLE_HOST_ID_SHIFT == host->hostId );
    dzFd = (DzFd*)( host->handleBase + ( fd & HANDLE_ADDR_MASK ) );

    return Read( host, dzFd, buf, count );
}

ssize_t DzWriteFile( int fd, const void* buf, size_t count )
{
    DzFd* dzFd;
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( fd >> HANDLE_HOST_ID_SHIFT == host->hostId );
    dzFd = (DzFd*)( host->handleBase + ( fd & HANDLE_ADDR_MASK ) );

    return Write( host, dzFd, buf, count );
}

size_t DzSeekFile( int fd, ssize_t offset, int whence )
{
    DzFd* dzFd;
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( fd >> HANDLE_HOST_ID_SHIFT == host->hostId );
    dzFd = (DzFd*)( host->handleBase + ( fd & HANDLE_ADDR_MASK ) );

    return Seek( dzFd, offset, whence );
}

size_t DzGetFileSize( int fd )
{
    DzFd* dzFd;
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( fd >> HANDLE_HOST_ID_SHIFT == host->hostId );
    dzFd = (DzFd*)( host->handleBase + ( fd & HANDLE_ADDR_MASK ) );

    return FileSize( dzFd );
}

int DzSocket( int domain, int type, int protocol )
{
    intptr_t ret;
    DzHost* host = GetHost();
    assert( host );

    ret = (intptr_t)Socket( host, domain, type, protocol );
    if( !ret ){
        return -1;
    }
    return (int)( ret - host->handleBase ) | host->hostId << HANDLE_HOST_ID_SHIFT;
}

int DzCloseSocket( int fd )
{
    DzFd* dzFd;
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( fd >> HANDLE_HOST_ID_SHIFT == host->hostId );
    dzFd = (DzFd*)( host->handleBase + ( fd & HANDLE_ADDR_MASK ) );

    return CloseSocket( host, dzFd );
}

int DzGetSockOpt( int fd, int level, int name, void* option, int* len )
{
    DzFd* dzFd;
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( fd >> HANDLE_HOST_ID_SHIFT == host->hostId );
    dzFd = (DzFd*)( host->handleBase + ( fd & HANDLE_ADDR_MASK ) );

    return GetSockOpt( dzFd, level, name, option, len );
}

int DzSetSockOpt( int fd, int level, int name, const void* option, int len )
{
    DzFd* dzFd;
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( fd >> HANDLE_HOST_ID_SHIFT == host->hostId );
    dzFd = (DzFd*)( host->handleBase + ( fd & HANDLE_ADDR_MASK ) );
    assert( !( level == SOL_SOCKET && name == SO_LINGER ) );

    return SetSockOpt( dzFd, level, name, option, len );
}

int DzGetSockName( int fd, struct sockaddr* addr, int* addrLen )
{
    DzFd* dzFd;
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( fd >> HANDLE_HOST_ID_SHIFT == host->hostId );
    dzFd = (DzFd*)( host->handleBase + ( fd & HANDLE_ADDR_MASK ) );

    return GetSockName( dzFd, addr, addrLen );
}

int DzBind( int fd, struct sockaddr* addr, int addrLen )
{
    DzFd* dzFd;
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( fd >> HANDLE_HOST_ID_SHIFT == host->hostId );
    dzFd = (DzFd*)( host->handleBase + ( fd & HANDLE_ADDR_MASK ) );

    return Bind( dzFd, addr, addrLen );
}

int DzListen( int fd, int backlog )
{
    DzFd* dzFd;
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( fd >> HANDLE_HOST_ID_SHIFT == host->hostId );
    dzFd = (DzFd*)( host->handleBase + ( fd & HANDLE_ADDR_MASK ) );

    return Listen( dzFd, backlog );
}

int DzShutdown( int fd, int how )
{
    DzFd* dzFd;
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( fd >> HANDLE_HOST_ID_SHIFT == host->hostId );
    dzFd = (DzFd*)( host->handleBase + ( fd & HANDLE_ADDR_MASK ) );

    return Shutdown( dzFd, how );
}

int DzConnect( int fd, struct sockaddr* addr, int addrLen )
{
    DzFd* dzFd;
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( fd >> HANDLE_HOST_ID_SHIFT == host->hostId );
    dzFd = (DzFd*)( host->handleBase + ( fd & HANDLE_ADDR_MASK ) );

    return Connect( host, dzFd, addr, addrLen );
}

int DzAccept( int fd, struct sockaddr* addr, int* addrLen )
{
    intptr_t ret;
    DzFd* dzFd;
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( fd >> HANDLE_HOST_ID_SHIFT == host->hostId );
    dzFd = (DzFd*)( host->handleBase + ( fd & HANDLE_ADDR_MASK ) );

    ret = (intptr_t)Accept( host, dzFd, addr, addrLen );
    if( !ret ){
        return -1;
    }
    return (int)( ret - host->handleBase ) | host->hostId << HANDLE_HOST_ID_SHIFT;

}

int DzSendEx( int fd, DzBuf* bufs, u_int bufCount, int flags )
{
    DzFd* dzFd;
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( fd >> HANDLE_HOST_ID_SHIFT == host->hostId );
    dzFd = (DzFd*)( host->handleBase + ( fd & HANDLE_ADDR_MASK ) );
    assert( bufCount > 0 && bufCount <= DZ_MAX_IOV );

    return SendEx( host, dzFd, bufs, bufCount, flags );
}

int DzRecvEx( int fd, DzBuf* bufs, u_int bufCount, int flags )
{
    DzFd* dzFd;
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( fd >> HANDLE_HOST_ID_SHIFT == host->hostId );
    dzFd = (DzFd*)( host->handleBase + ( fd & HANDLE_ADDR_MASK ) );
    assert( bufCount > 0 && bufCount <= DZ_MAX_IOV );

    return RecvEx( host, dzFd, bufs, bufCount, flags );
}

int DzSend( int fd, const void* buf, u_int len, int flags )
{
    DzFd* dzFd;
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( fd >> HANDLE_HOST_ID_SHIFT == host->hostId );
    dzFd = (DzFd*)( host->handleBase + ( fd & HANDLE_ADDR_MASK ) );

    return Send( host, dzFd, buf, len, flags );
}

int DzRecv( int fd, void* buf, u_int len, int flags )
{
    DzFd* dzFd;
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( fd >> HANDLE_HOST_ID_SHIFT == host->hostId );
    dzFd = (DzFd*)( host->handleBase + ( fd & HANDLE_ADDR_MASK ) );

    return Recv( host, dzFd, buf, len, flags );
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
    DzFd* dzFd;
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( fd >> HANDLE_HOST_ID_SHIFT == host->hostId );
    dzFd = (DzFd*)( host->handleBase + ( fd & HANDLE_ADDR_MASK ) );
    assert( bufCount > 0 && bufCount <= DZ_MAX_IOV );

    return SendToEx( host, dzFd, bufs, bufCount, flags, to, tolen );
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
    DzFd* dzFd;
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( fd >> HANDLE_HOST_ID_SHIFT == host->hostId );
    dzFd = (DzFd*)( host->handleBase + ( fd & HANDLE_ADDR_MASK ) );
    assert( bufCount > 0 && bufCount <= DZ_MAX_IOV );

    return RecvFromEx( host, dzFd, bufs, bufCount, flags, from, fromlen );
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
    DzFd* dzFd;
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( fd >> HANDLE_HOST_ID_SHIFT == host->hostId );
    dzFd = (DzFd*)( host->handleBase + ( fd & HANDLE_ADDR_MASK ) );

    return SendTo( host, dzFd, buf, len, flags, to, tolen );
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
    DzFd* dzFd;
    DzHost* host = GetHost();
    assert( host );
    assert( fd >= 0 );
    assert( fd >> HANDLE_HOST_ID_SHIFT == host->hostId );
    dzFd = (DzFd*)( host->handleBase + ( fd & HANDLE_ADDR_MASK ) );

    return RecvFrom( host, dzFd, buf, len, flags, from, fromlen );
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
