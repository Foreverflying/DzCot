/********************************************************************
    created:    2010/02/11 21:50
    file:       Inc_Dzcot.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __Inc_Dzcot_h__
#define __Inc_Dzcot_h__

#if defined( _WIN32 )
#include <stdarg.h>
#include <WinSock2.h>
#elif defined( __linux__ )
#include <stddef.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

enum
{
    DS_OK,
    DS_NO_MEMORY
};

enum
{
    CP_FIRST,
    CP_HIGH = CP_FIRST,
    CP_NORMAL,
    CP_LOW,
    COT_PRIORITY_COUNT,     //should not use this;
    CP_DEFAULT = COT_PRIORITY_COUNT
};

enum
{
    SS_FIRST,
    SS_4K = SS_FIRST,
    SS_16K,
    SS_64K,
    SS_256K,
    SS_1M,
    SS_4M,
    SS_16M,
    SS_64M,
    STACK_SIZE_COUNT,       //should not use this
    SS_DEFAULT = STACK_SIZE_COUNT
};

#define DZ_O_RD         0x0000
#define DZ_O_WR         0x0001
#define DZ_O_RDWR       0x0002
#define DZ_O_CREATE     0x0100
#define DZ_O_EXCL       0x0200
#define DZ_O_TRUNC      0x1000
#define DZ_O_APPEND     0x2000

#define DZ_SEEK_SET     0
#define DZ_SEEK_CUR     1
#define DZ_SEEK_END     2

#define DZ_SHUT_READ    0
#define DZ_SHUT_WRITE   1
#define DZ_SHUT_BOTH    2

#define DZ_MAX_IOV      64
#define DZ_MAX_HOST     16

typedef struct _DzParamNode
{
    union{
        struct{
            int     d8a;
            int     d8;
        };
        void*       p8;
    };
    union{
        struct{
            int     d7a;
            int     d7;
        };
        void*       p7;
    };
    union{
        struct{
            int     d6a;
            int     d6;
        };
        void*       p6;
    };
    union{
        struct{
            int     d5a;
            int     d5;
        };
        void*       p5;
    };
    union{
        struct{
            int     d4a;
            int     d4;
        };
        void*       p4;
    };
    union{
        struct{
            int     d3a;
            int     d3;
        };
        void*       p3;
    };
    union{
        struct{
            int     d2a;
            int     d2;
        };
        void*       p2;
    };
    union{
        struct{
            int     d1a;
            int     d1;
        };
        void*       p1;
    };
}DzParamNode;

#ifdef _WIN32

typedef intptr_t ssize_t;

typedef struct _DzBuf
{
    unsigned long   len;
    void*           buf;
}DzBuf;

#elif defined __linux__

typedef struct _DzBuf
{
    void*           buf;
    size_t          len;
}DzBuf;

typedef enum{
    FALSE,
    TRUE
}BOOL;

#endif

typedef void* DzHandle;

#if defined( __linux__ )
#if defined( __i386 )
#define __stdcall __attribute__((stdcall))
#else
#define __stdcall
#endif
#endif

typedef void (__stdcall *DzRoutine)( intptr_t context );


#define DZMAKEIPADDRESS( a1, a2, a3, a4 ) (\
    ((a1) << 24) | ((a2) << 16) | ((a3) << 8) | (a4)\
)

#define hton16( n ) (\
    ( ( (n) & 0xff ) << 8 ) | ( ( (n) & 0xff00 ) >> 8 )\
)

#define hton32( n ) (\
    ( ( (n) & 0xff ) << 24 ) | ( ( (n) & 0xff00 ) << 8 ) |\
    ( ( (n) & 0xff0000 ) >> 8 ) | ( ( (n) & 0xff000000 ) >> 24 )\
)

#define hton64( n ) (\
    ( ( (n) & 0xff ) << 56 )             | ( ( (n) & 0xff00 ) << 40 ) |\
    ( ( (n) & 0xff0000 ) << 24 )         | ( ( (n) & 0xff000000 ) << 8 ) |\
    ( ( (n) & 0xff00000000 ) >> 8 )      | ( ( (n) & 0xff0000000000 ) >> 24 ) |\
    ( ( (n) & 0xff000000000000 ) >> 40 ) | ( ( (n) & 0xff00000000000000 ) >> 56 )\
)

#ifndef __cplusplus
#define __DZ_DFT_ARG( x )
#else
#define __DZ_DFT_ARG( x ) = x
extern "C"{
#endif

static int __DzMakeServMask__( BOOL notServ, ... )
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

#define DzMakeServMask( notServ, ... )\
    __DzMakeServMask__( notServ, ##__VA_ARGS__, -1 )

int DzRunHosts(
    int         hostCount,
    int*        servMask,
    int         lowestPri,
    int         dftPri,
    int         dftSSize,
    DzRoutine   firstEntry,
    intptr_t    context         __DZ_DFT_ARG( 0 )
    );
int DzStartCot(
    DzRoutine   entry,
    intptr_t    context         __DZ_DFT_ARG( 0 ),
    int         priority        __DZ_DFT_ARG( CP_DEFAULT ),
    int         sSize           __DZ_DFT_ARG( SS_DEFAULT )
    );
int DzStartCotInstant(
    DzRoutine   entry,
    intptr_t    context         __DZ_DFT_ARG( 0 ),
    int         priority        __DZ_DFT_ARG( CP_DEFAULT ),
    int         sSize           __DZ_DFT_ARG( SS_DEFAULT )
    );
int DzEvtStartCot(
    DzHandle    evt,
    DzRoutine   entry,
    intptr_t    context         __DZ_DFT_ARG( 0 ),
    int         priority        __DZ_DFT_ARG( CP_DEFAULT ),
    int         sSize           __DZ_DFT_ARG( SS_DEFAULT )
    );
int DzEvtStartCotInstant(
    DzHandle    evt,
    DzRoutine   entry,
    intptr_t    context         __DZ_DFT_ARG( 0 ),
    int         priority        __DZ_DFT_ARG( CP_DEFAULT ),
    int         sSize           __DZ_DFT_ARG( SS_DEFAULT )
    );
int DzStartRemoteCot(
    int         rmtId,
    DzRoutine   entry,
    intptr_t    context         __DZ_DFT_ARG( 0 ),
    int         priority        __DZ_DFT_ARG( CP_DEFAULT ),
    int         sSize           __DZ_DFT_ARG( SS_DEFAULT )
    );
int DzEvtStartRemoteCot(
    DzHandle    evt,
    int         rmtId,
    DzRoutine   entry,
    intptr_t    context         __DZ_DFT_ARG( 0 ),
    int         priority        __DZ_DFT_ARG( CP_DEFAULT ),
    int         sSize           __DZ_DFT_ARG( SS_DEFAULT )
    );
int DzRunRemoteCot(
    int         rmtId,
    DzRoutine   entry,
    intptr_t    context         __DZ_DFT_ARG( 0 ),
    int         priority        __DZ_DFT_ARG( CP_DEFAULT ),
    int         sSize           __DZ_DFT_ARG( SS_DEFAULT )
    );
int DzRunWorker(
    DzRoutine   entry,
    intptr_t    context         __DZ_DFT_ARG( 0 )
    );
int DzGetCotCount();
int DzSetPriority( int priority );
int DzSetCotPoolDepth( int sSize, int depth );
int DzSetWorkerPoolDepth( int depth );
int DzSetHostParam( int lowestPri, int dftPri, int dftSSize );

int DzWaitSynObj(
    DzHandle    obj,
    int         timeout         __DZ_DFT_ARG( -1 )
    );
int DzWaitMultiSynObj(
    int         count,
    DzHandle*   obj,
    BOOL        waitAll,
    int         timeout         __DZ_DFT_ARG( -1 )
    );
DzHandle DzCreateMtx( BOOL owner );
BOOL DzReleaseMtx( DzHandle mtx );
DzHandle DzCreateManualEvt( BOOL notified );
DzHandle DzCreateAutoEvt( BOOL notified );
DzHandle DzCreateCdEvt( u_int count );
BOOL DzSetEvt( DzHandle evt );
BOOL DzResetEvt( DzHandle evt );
DzHandle DzCreateSem( int count );
int DzReleaseSem( DzHandle sem, int count );
DzHandle DzCloneSynObj( DzHandle obj );
BOOL DzCloseSynObj( DzHandle obj );
DzHandle DzCreateTimer( int milSec, int repeat );
BOOL DzCloseTimer( DzHandle timer );
DzHandle DzCreateCallbackTimer(
    int         milSec,
    int         repeat,
    DzRoutine   callback,
    intptr_t    context         __DZ_DFT_ARG( 0 ),
    int         priority        __DZ_DFT_ARG( CP_DEFAULT ),
    int         sSize           __DZ_DFT_ARG( SS_DEFAULT )
    );
BOOL DzCloseCallbackTimer( DzHandle timer );
void DzSleep( int milSec );

int DzOpenFileA( const char* fileName, int flags );
int DzCloseFile( int fd );
ssize_t DzReadFile( int fd, void* buf, size_t count );
ssize_t DzWriteFile( int fd, const void* buf, size_t count );
size_t DzSeekFile( int fd, ssize_t offset, int whence );
size_t DzGetFileSize( int fd );

#ifdef _WIN32
int DzOpenFileW( const wchar_t* fileName, int flags );
#endif

#ifndef UNICODE
#define DzOpenFile DzOpenFileA
#else
#define DzOpenFile DzOpenFileW
#endif

int DzSocket( int domain, int type, int protocol );
int DzCloseSocket( int fd );
int DzGetSockOpt( int fd, int level, int name, void* option, int* len );
int DzSetSockOpt( int fd, int level, int name, const void* option, int len );
int DzGetSockName( int fd, struct sockaddr* addr, int* addrLen );
int DzBind( int fd, struct sockaddr* addr, int addrLen );
int DzListen( int fd, int backlog );
int DzShutdown( int fd, int how );
int DzConnect( int fd, struct sockaddr* addr, int addrLen );
int DzAccept( int fd, struct sockaddr* addr, int* addrLen );
int DzSendEx( int fd, DzBuf* bufs, u_int bufCount, int flags );
int DzRecvEx( int fd, DzBuf* bufs, u_int bufCount, int flags );
int DzSend( int fd, const void* buf, u_int len, int flags );
int DzRecv( int fd, void* buf, u_int len, int flags );
int DzSendToEx(
    int                     fd,
    DzBuf*                  bufs,
    u_int                   bufCount,
    int                     flags,
    const struct sockaddr*  to,
    int                     tolen
    );
int DzRecvFromEx(
    int                     fd,
    DzBuf*                  bufs,
    u_int                   bufCount,
    int                     flags,
    struct sockaddr*        from,
    int*                    fromlen
    );
int DzSendTo(
    int                     fd,
    const void*             buf,
    u_int                   len,
    int                     flags,
    const struct sockaddr*  to,
    int                     tolen
    );
int DzRecvFrom(
    int                     fd,
    void*                   buf,
    u_int                   len,
    int                     flags,
    struct sockaddr*        from,
    int*                    fromlen
    );

DzParamNode* DzAllocParamNode();
void DzFreeParamNode( DzParamNode* node );

void* DzMalloc( size_t size );
void* DzCalloc( size_t num, size_t size );
void* DzReAlloc( void* mem, size_t size );
void DzFree( void* p );
void* DzMallocEx( size_t size );
void DzFreeEx( void* mem );

unsigned long long DzUnixTime();
unsigned long long DzMilUnixTime();

int __DzDbgLastErr();

#ifdef __cplusplus
};
#endif

#endif // __Inc_Dzcot_h__
