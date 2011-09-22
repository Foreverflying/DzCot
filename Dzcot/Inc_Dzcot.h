/********************************************************************
    created:    2010/02/11 21:50
    file:       Inc_Dzcot.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __Inc_Dzcot_h__
#define __Inc_Dzcot_h__

#undef FALSE
#undef TRUE
#undef NULL

#define FALSE   0
#define TRUE    1
#define NULL    0

typedef int BOOL;
typedef unsigned int uint;

enum
{
    DS_OK,
    DS_TIMEOUT,
    DS_INVALID_ARG,
    DS_ALREADY_EXIST,
    DS_NOT_INIT,
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

#define DZ_O_RD         0x01
#define DZ_O_WR         0x02
#define DZ_O_RDWR       0x03
#define DZ_O_CREATE     0x08
#define DZ_O_EXCL       0x10
#define DZ_O_TRUNC      0x20
#define DZ_O_APPEND     0x40

#define DZ_SD_RD        0
#define DZ_SD_WR        1
#define DZ_SD_RDWR      2

#define DZ_IOV_MAX      64

typedef struct _DzParamNode
{
    void*           context3;
    void*           context2;
    void*           context1;
    void*           content;
}DzParamNode;

#ifdef _WIN32

typedef struct _DzBuf
{
    unsigned long   len;
    char*           buf;
}DzBuf;

#elif defined __linux__

typedef struct _DzBuf
{
    void*           buf;
    size_t          len;
}DzBuf;

#endif

struct _DzSynObj;
typedef struct _DzSynObj* DzHandle;

#ifdef __linux__
#define __stdcall __attribute__((stdcall))
#endif

typedef void (__stdcall *DzRoutine)( void* context );


#ifndef __cplusplus
#define __DZ_DFT_ARG( x )
#else
#define __DZ_DFT_ARG( x ) = x
extern "C"{
#endif

int DzRunHost(
    int         lowestPriority,
    int         defaultPri,
    int         defaultSSize,
    DzRoutine   firstEntry,
    void*       context         __DZ_DFT_ARG( NULL ),
    int         priority        __DZ_DFT_ARG( CP_DEFAULT ),
    int         sSize           __DZ_DFT_ARG( SS_DEFAULT )
    );
int DzStartCot(
    DzRoutine   entry,
    void*       context         __DZ_DFT_ARG( NULL ),
    int         priority        __DZ_DFT_ARG( CP_DEFAULT ),
    int         sSize           __DZ_DFT_ARG( SS_DEFAULT )
    );
int DzStartCotInstant(
    DzRoutine   entry,
    void*       context         __DZ_DFT_ARG( NULL ),
    int         priority        __DZ_DFT_ARG( CP_DEFAULT ),
    int         sSize           __DZ_DFT_ARG( SS_DEFAULT )
    );
int DzGetCotCount();
int DzGetMaxCotCount(
    BOOL reset                  __DZ_DFT_ARG( FALSE )
    );
int DzChangePriority( int priority );
BOOL DzGrowCotPoolDepth( int sSize, int deta );

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
DzHandle DzCreateEvt( BOOL manualReset, BOOL notified );
BOOL DzSetEvt( DzHandle evt );
BOOL DzResetEvt( DzHandle evt );
DzHandle DzCreateSem( int count );
int DzReleaseSem( DzHandle sem, int count );
DzHandle DzCloneSynObj( DzHandle obj );
BOOL DzCloseSynObj( DzHandle obj );
DzHandle DzCreateTimer( uint milSec, uint repeat );
BOOL DzCloseTimer( DzHandle timer );
DzHandle DzCreateCallbackTimer(
    uint        milSec,
    uint        repeat,
    DzRoutine   callback,
    void*       context         __DZ_DFT_ARG( NULL ),
    int         priority        __DZ_DFT_ARG( CP_DEFAULT ),
    int         sSize           __DZ_DFT_ARG( SS_DEFAULT )
    );
BOOL DzCloseCallbackTimer( DzHandle timer );
int DzSleep( uint milSec );
int DzSleep0();

int DzOpenFileA( const char* fileName, int flags );
int DzOpenFileW( const wchar_t* fileName, int flags );
int DzCloseFd( int fd );
size_t DzReadFile( int fd, void* buf, size_t count );
size_t DzWriteFile( int fd, const void* buf, size_t count );
size_t DzSeekFile( int fd, size_t offset, int whence );
size_t DzGetFileSize( int fd );

#ifdef UNICODE
#define DzOpenFile DzOpenFileW
#else
#define DzOpenFile DzOpenFileA
#endif

BOOL DzSockStartup();
BOOL DzSockCleanup();
int DzSocket( int domain, int type, int protocol );
int DzShutdown( int fd, int how );
int DzCloseSocket( int fd );
int DzBind( int fd, struct sockaddr* addr, int addrLen );
int DzListen( int fd, int backlog );
int DzConnect( int fd, struct sockaddr* addr, int addrLen );
int DzAccept( int fd, struct sockaddr* addr, int* addrLen );
int DzSendEx( int fd, DzBuf* bufs, int bufCount, int flags );
int DzRecvEx( int fd, DzBuf* bufs, int bufCount, int flags );
int DzSend( int fd, const void* buf, int len, int flags );
int DzRecv( int fd, void* buf, int len, int flags );
int DzSendToEx(
    int                     fd,
    DzBuf*                  bufs,
    int                     bufCount,
    int                     flags,
    const struct sockaddr*  to,
    int                     tolen
    );
int DzRecvFromEx(
    int                     fd,
    DzBuf*                  bufs,
    int                     bufCount,
    int                     flags,
    struct sockaddr*        from,
    int*                    fromlen
    );
int DzSendTo(
    int                     fd,
    const char*             buf,
    int                     len,
    int                     flags,
    const struct sockaddr*  to,
    int                     tolen
    );
int DzRecvFrom(
    int                     fd,
    char*                   buf,
    int                     len,
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

unsigned long long DzUnixTime();
unsigned long long DzMilUnixTime();

#ifdef __cplusplus
};
#endif

#endif // __Inc_Dzcot_h__
