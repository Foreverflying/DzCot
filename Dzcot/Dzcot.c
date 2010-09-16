#include "DzIncOs.h"
#include "DzcotExports.h"

#include "Dzcot.h"
#include "DzSynObj.h"
#include "DzCore.h"
#include <assert.h>

//using namespace Dzcot;

// InitHost:
// create struct needed
int DzInitHost(
    int         lowestPriority,
    int         defaultPri,
    int         defaultSSize
    )
{
    assert(
        lowestPriority >= CP_HIGH &&
        lowestPriority < COT_PRIORITY_COUNT
        );
    assert(
        defaultPri >= CP_HIGH &&
        defaultPri < COT_PRIORITY_COUNT
        );
    assert(
        defaultSSize >= SS_64K &&
        defaultSSize < STACK_SIZE_COUNT
        );
    assert( !GetHost() );

    return InitHost( lowestPriority, defaultPri, defaultSSize );
}

// StartHost:
// create the Io mgr co thread, so the host can serve requests
int DzStartHost(
    DzRoutine   firstEntry,
    void*       context,
    int         priority,
    int         sSize
    )
{
    DzHost *host = GetHost();
    assert( host );
    assert( host->threadCount == 0 );

    return StartHost( host, firstEntry, context, priority, sSize );
}

// StartCot:
// create a new co thread
int DzStartCot(
    DzRoutine   entry,
    void*       context,
    int         priority,
    int         sSize
    )
{
    DzHost *host = GetHost();
    assert( host );
    assert( entry );
    assert(
        priority >= CP_INSTANT &&
        priority <= CP_DEFAULT &&
        priority != COT_PRIORITY_COUNT
        );
    assert(
        sSize >= SS_64K &&
        sSize <= SS_DEFAULT &&
        sSize != STACK_SIZE_COUNT
        );
    assert( host->threadCount > 0 );

    return StartCot( host, entry, context, priority, sSize );
}

int DzGetCotCount()
{
    DzHost *host = GetHost();
    assert( host );

    return GetCotCount( host );
}

int DzGetMaxCotCount( BOOL reset )
{
    DzHost *host = GetHost();
    assert( host );

    return GetMaxCotCount( host, reset );
}

void DzInitCotPool( u_int count, u_int depth, int sSize )
{
    DzHost *host = GetHost();
    assert( host );
    assert(
        sSize >= SS_64K &&
        sSize <= SS_DEFAULT &&
        sSize != STACK_SIZE_COUNT
        );

    InitCotPool( host, count, depth, sSize );
}

int DzSleep( int milSec )
{
    DzHost *host = GetHost();
    assert( host );
    assert( milSec >= 0 );

    if( !milSec ){
        DispatchCurrThread( host );
    }else{
        DelayCurrThread( host, milSec );
    }
    return DS_OK;
}

DzHandle DzCreateEvt( BOOL notified, BOOL autoReset )
{
    DzHost *host = GetHost();
    assert( host );

    return CreateEvt( host, notified, autoReset );
}

BOOL DzSetEvt( DzHandle evt )
{
    DzHost *host = GetHost();
    assert( host );

    SetEvt( host, evt );
    return TRUE;
}

BOOL DzResetEvt( DzHandle evt )
{
    ResetEvt( evt );
    return TRUE;
}

DzHandle DzCreateSem( u_int count )
{
    DzHost *host = GetHost();
    assert( host );

    return CreateSem( host, count );
}

u_int DzReleaseSem( DzHandle sem, int count )
{
    DzHost *host = GetHost();
    assert( host );
    assert( count > 0 );

    return ReleaseSem( host, sem, count );
}

DzHandle DzCloneSynObj( DzHandle obj )
{
    return CloneSynObj( obj );
}

BOOL DzCloseSynObj( DzHandle obj )
{
    DzHost *host = GetHost();
    assert( host );

    CloseSynObj( host, obj );
    return TRUE;
}

DzHandle DzCreateTimer( int milSec, int repeat )
{
    DzHost *host = GetHost();
    assert( host );
    assert( milSec > 0 );

    return CreateTimer( host, milSec, repeat );
}

BOOL DzCloseTimer( DzHandle timer )
{
    DzHost *host = GetHost();
    assert( host );

    CloseTimer( host, timer );
    return TRUE;
}


DzHandle DzCreateCallbackTimer( DzRoutine callback, int priority, int sSize )
{
    DzHost *host = GetHost();
    assert( host );

    return CreateCallbackTimer( host, callback, priority, sSize );
}

BOOL DzStartCallbackTimer( DzHandle timer, int milSec, int repeat, void* context )
{
    DzHost *host = GetHost();
    assert( host );
    assert( repeat >= 0 && repeat < 65536 );

    return StartCallbackTimer( host, timer, milSec, (unsigned short)repeat, context );
}

BOOL DzStopCallbackTimer( DzHandle timer )
{
    DzHost *host = GetHost();
    assert( host );

    return StopCallbackTimer( host, timer );
}

BOOL DzCloseCallbackTimer( DzHandle timer )
{
    DzHost *host = GetHost();
    assert( host );

    CloseCallbackTimer( host, timer );
    return TRUE;
}

int DzWaitSynObj( DzHandle obj, int timeOut )
{
    DzHost *host = GetHost();
    assert( host );
    assert( obj );

    return WaitSynObj( host, obj, timeOut );
}

int DzWaitMultiSynObj( int count, DzHandle *obj, BOOL waitAll, int timeOut )
{
    DzHost *host = GetHost();
    assert( host );
    assert( obj );

    return WaitMultiSynObj( host, count, obj, waitAll, timeOut );
}

int DzOpenFileA( char *fileName, int flags )
{
    DzHost *host = GetHost();
    assert( host );

    return OpenA( host, fileName, flags );
}

int DzOpenFileW( wchar_t *fileName, int flags )
{
    DzHost *host = GetHost();
    assert( host );

    return OpenW( host, fileName, flags );
}

int DzCloseFd( int fd )
{
    DzHost *host = GetHost();
    assert( host );

    return Close( host, fd );
}

size_t DzReadFile( int fd, void *buff, size_t count )
{
    DzHost *host = GetHost();
    assert( host );

    return Read( host, fd, buff, count );
}

size_t DzWriteFile( int fd, void *buff, size_t count )
{
    DzHost *host = GetHost();
    assert( host );

    return Write( host, fd, buff, count );
}

size_t DzSeekFile( int fd, size_t offset, int whence )
{
    DzHost *host = GetHost();
    assert( host );

    return Seek( host, fd, offset, whence );
}

size_t DzGetFileSize( int fd )
{
    return FileSize( fd );
}

BOOL DzSockStartup()
{
    return SockStartup();
}

BOOL DzSockCleanup()
{
    return SockCleanup();
}

int DzSocket( int domain, int type, int protocol )
{
    DzHost *host = GetHost();
    assert( host );
    assert( isSocketStarted );

    return Socket( host, domain, type, protocol );
}

int DzShutdown( int fd, int how )
{
    assert( isSocketStarted );

    return Shutdown( fd, how );
}

int DzCloseSocket( int fd )
{
    DzHost *host = GetHost();
    assert( host );
    assert( isSocketStarted );

    return CloseSocket( host, fd );
}

int DzBind( int fd, struct sockaddr *addr, int addrLen )
{
    assert( isSocketStarted );

    return Bind( fd, addr, addrLen );
}

int DzListen( int fd, int backlog )
{
    assert( isSocketStarted );

    return Listen( fd, backlog );
}

int DzConnect( int fd, struct sockaddr *addr, int addrLen )
{
    DzHost *host = GetHost();
    assert( host );
    assert( isSocketStarted );

    return Connect( host, fd, addr, addrLen );
}

int DzAccept( int fd, struct sockaddr *addr, int *addrLen )
{
    DzHost *host = GetHost();
    assert( host );
    assert( isSocketStarted );

    return Accept( host, fd, addr, addrLen );
}

int DzSend( int fd, void *msg, int len, int flag )
{
    DzHost *host = GetHost();
    assert( host );
    assert( isSocketStarted );

    return Send( host, fd, msg, len, flag );
}

int DzRecv( int fd, void *buf, int len, int flag )
{
    DzHost *host = GetHost();
    assert( host );
    assert( isSocketStarted );

    return Recv( host, fd, buf, len, flag );
}

DzParamNode* DzAllocParamNode()
{
    DzHost *host = GetHost();
    assert( host );

    return (DzParamNode*)AllocQNode( host );
}

void DzFreeParamNode( DzParamNode *node )
{
    DzHost *host = GetHost();
    assert( host );

    FreeQNode( host, (DzQNode*)node );
}

unsigned long long DzUnixTime()
{
    return (unsigned long long)UnixTime();
}

unsigned long long DzMilUnixTime()
{
    return (unsigned long long)MilUnixTime();
}