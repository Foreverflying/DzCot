/********************************************************************
    created:    2010/02/11 21:50
    file:       Inc_DzCot.h
    author:     Foreverflying
    purpose:    
********************************************************************/

#ifndef __Inc_DzCot_h__
#define __Inc_DzCot_h__

#if defined( _WIN32 )
#include <WinSock2.h>
#include <Ws2tcpip.h>
#elif defined( __linux__ )
#include <stddef.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
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

int DzRunHosts(
    int         hostCount,
    int*        servMask,
    int         lowestPri,
    int         dftPri,
    int         dftSSize,
    DzRoutine   firstEntry,
    intptr_t    context         __DZ_DFT_ARG( 0 ),
    DzRoutine   cleanEntry      __DZ_DFT_ARG( NULL )
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
    int         evt,
    DzRoutine   entry,
    intptr_t    context         __DZ_DFT_ARG( 0 ),
    int         priority        __DZ_DFT_ARG( CP_DEFAULT ),
    int         sSize           __DZ_DFT_ARG( SS_DEFAULT )
    );
int DzEvtStartCotInstant(
    int         evt,
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
    int         evt,
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
    int         obj,
    int         timeout         __DZ_DFT_ARG( -1 )
    );
int DzWaitMultiSynObj(
    int         count,
    int*        obj,
    BOOL        waitAll,
    int         timeout         __DZ_DFT_ARG( -1 )
    );
int DzCreateMtx( BOOL owner );
BOOL DzReleaseMtx( int mtx );
int DzCreateManualEvt( BOOL notified );
int DzCreateAutoEvt( BOOL notified );
int DzCreateCdEvt( u_int count );
BOOL DzSetEvt( int evt );
BOOL DzResetEvt( int evt );
int DzCreateSem( int count );
int DzReleaseSem( int sem, int count );
int DzCloneSynObj( int obj );
BOOL DzCloseSynObj( int obj );
int DzCreateTimer( int milSec, int repeat );
BOOL DzCloseTimer( int timer );
int DzCreateCallbackTimer(
    int         milSec,
    int         repeat,
    DzRoutine   callback,
    intptr_t    context         __DZ_DFT_ARG( 0 ),
    int         priority        __DZ_DFT_ARG( CP_DEFAULT ),
    int         sSize           __DZ_DFT_ARG( SS_DEFAULT )
    );
BOOL DzCloseCallbackTimer( int timer );
void DzSleep( int milSec );

int DzOpenFileA( const char* fileName, int flags );
int DzCloseFile( int fd );
ssize_t DzReadFile( int fd, void* buf, size_t count );
ssize_t DzWriteFile( int fd, const void* buf, size_t count );
size_t DzSeekFile( int fd, ssize_t offset, int whence );
size_t DzGetFileSize( int fd );

#ifdef _WIN32
#endif

#if defined( _WIN32 ) && defined( UNICODE )
#define DzOpenFile DzOpenFileW
#else
#define DzOpenFile DzOpenFileA
#endif

int DzSocket( int domain, int type, int protocol );
int DzCloseSocket( int fd );
int DzGetSockOpt( int fd, int level, int name, void* option, int* len );
int DzSetSockOpt( int fd, int level, int name, const void* option, int len );
int DzGetSockName( int fd, struct sockaddr* addr, int* addrLen );
int DzGetPeerName( int fd, struct sockaddr* addr, int* addrLen );
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
int DzGetNameInfoA(
    const struct sockaddr*  sa,
    int                     salen,
    char*                   host,
    size_t                  hostlen,
    char*                   serv,
    size_t                  servlen,
    int                     flags
    );
int DzGetAddrInfoA(
    const char*             node,
    const char*             service,
    const struct addrinfo*  hints,
    struct addrinfo**       res
    );
void DzFreeAddrInfoA( struct addrinfo *res );
int DzInetPtonA( int af, const char* src, void* dst );
const char* DzInetNtopA( int af, const void* src, char* dst, int size );

DzParamNode* DzAllocParamNode();
void DzFreeParamNode( DzParamNode* node );

void* DzAllocPermanentChunk( size_t size );
void* DzMalloc( size_t size );
void* DzCalloc( size_t num, size_t size );
void* DzReAlloc( void* mem, size_t size );
void DzFree( void* p );
void* DzMallocEx( size_t size );
void DzFreeEx( void* mem );

unsigned long long DzUnixTime();
unsigned long long DzMilUnixTime();

int __DzMakeServMask( BOOL notServ, ... );
int __DzDbgLastErr();

#define DzMakeServMask( notServ, ... )\
    __DzMakeServMask( notServ, ##__VA_ARGS__, -1 )

#ifdef _WIN32

int DzOpenFileW( const wchar_t* fileName, int flags );
int DzGetNameInfoW(
    const struct sockaddr*  sa,
    int                     salen,
    wchar_t*                host,
    size_t                  hostlen,
    wchar_t*                serv,
    size_t                  servlen,
    int                     flags
    );
int DzGetAddrInfoW(
    const wchar_t*          node,
    const wchar_t*          service,
    const struct addrinfoW* hints,
    struct addrinfoW**      res
    );
void DzFreeAddrInfoW( struct addrinfoW *res );
int DzInetPtonW( int af, const wchar_t* src, void* dst );
const wchar_t* DzInetNtopW( int af, const void* src, wchar_t* dst, int size );

#endif

#if defined( _WIN32 ) && defined( UNICODE )

#define DzOpenFile          DzOpenFileW
#define DzGetNameInfo       DzGetNameInfoW
#define DzGetAddrInfo       DzGetAddrInfoW
#define DzFreeAddrInfo      DzFreeAddrInfoW
#define DzInetPton          DzInetPtonW
#define DzInetNtop          DzInetNtopW

#else

#define DzOpenFile          DzOpenFileA
#define DzGetNameInfo       DzGetNameInfoA
#define DzGetAddrInfo       DzGetAddrInfoA
#define DzFreeAddrInfo      DzFreeAddrInfoA
#define DzInetPton          DzInetPtonA
#define DzInetNtop          DzInetNtopA

#endif

#ifdef __cplusplus
};
#endif

#endif // __Inc_DzCot_h__
