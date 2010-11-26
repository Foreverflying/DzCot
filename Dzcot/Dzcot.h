/********************************************************************
    created:    2010/02/11 21:50
    file:       Dzcot.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __Dzcot_h__
#define __Dzcot_h__

#if defined(WIN32) && defined(USE_DZCOT_DLL)
#   ifdef DZCOT_EXPORTS
#       define DZCOT_API __declspec(dllexport)
#   else
#       define DZCOT_API __declspec(dllimport)
#   endif
#else
#   define DZCOT_API
#endif

#include "DzConstant.h"
#include "DzType.h"

#ifndef __cplusplus
#define __DZ_DFT_ARG( x )
#else
#define __DZ_DFT_ARG( x ) = x
extern "C"{
#endif

DZCOT_API int DzRunHost(
    int         lowestPriority,
    int         defaultPri,
    int         defaultSSize,
    DzRoutine   firstEntry,
    void*       context         __DZ_DFT_ARG( NULL ),
    int         priority        __DZ_DFT_ARG( CP_DEFAULT ),
    int         sSize           __DZ_DFT_ARG( SS_DEFAULT )
    );
DZCOT_API int DzStartCot(
    DzRoutine   entry,
    void*       context         __DZ_DFT_ARG( NULL ),
    int         priority        __DZ_DFT_ARG( CP_DEFAULT ),
    int         sSize           __DZ_DFT_ARG( SS_DEFAULT )
    );
DZCOT_API int DzStartCotInstant(
    DzRoutine   entry,
    void*       context         __DZ_DFT_ARG( NULL ),
    int         priority        __DZ_DFT_ARG( CP_DEFAULT ),
    int         sSize           __DZ_DFT_ARG( SS_DEFAULT )
    );
DZCOT_API int DzGetCotCount();
DZCOT_API int DzGetMaxCotCount(
    BOOL reset                  __DZ_DFT_ARG( FALSE )
    );
DZCOT_API void DzInitCotPool(
    uint        count,
    uint        depth,
    int         sSize           __DZ_DFT_ARG( SS_DEFAULT )
    );
DZCOT_API int DzChangePriority( int priority );

DZCOT_API int DzWaitSynObj(
    DzHandle    obj,
    int         timeout         __DZ_DFT_ARG( -1 )
    );
DZCOT_API int DzWaitMultiSynObj(
    int         count,
    DzHandle*   obj,
    BOOL        waitAll,
    int         timeout         __DZ_DFT_ARG( -1 )
    );
DZCOT_API DzHandle DzCreateMutex( BOOL owner );
DZCOT_API BOOL DzReleaseMutex( DzHandle mtx );
DZCOT_API DzHandle DzCreateEvt( BOOL notified, BOOL autoReset );
DZCOT_API BOOL DzSetEvt( DzHandle evt );
DZCOT_API BOOL DzResetEvt( DzHandle evt );
DZCOT_API DzHandle DzCreateSem( uint count );
DZCOT_API uint DzReleaseSem( DzHandle sem, int count );
DZCOT_API DzHandle DzCloneSynObj( DzHandle obj );
DZCOT_API BOOL DzCloseSynObj( DzHandle obj );
DZCOT_API DzHandle DzCreateTimer( int milSec, int repeat );
DZCOT_API BOOL DzCloseTimer( DzHandle timer );
DZCOT_API DzHandle DzCreateCallbackTimer(
    DzRoutine   callback,
    int         priority        __DZ_DFT_ARG( CP_DEFAULT ),
    int         sSize           __DZ_DFT_ARG( SS_DEFAULT )
    );
DZCOT_API BOOL DzStartCallbackTimer( DzHandle timer, int milSec, int repeat, void* context );
DZCOT_API BOOL DzStopCallbackTimer( DzHandle timer );
DZCOT_API BOOL DzCloseCallbackTimer( DzHandle timer );
DZCOT_API int DzSleep( int milSec );
DZCOT_API int DzSleep0();

DZCOT_API int DzOpenFileA( const char* fileName, int flags );
DZCOT_API int DzOpenFileW( const wchar_t* fileName, int flags );
DZCOT_API int DzCloseFd( int fd );
DZCOT_API size_t DzReadFile( int fd, void* buf, size_t count );
DZCOT_API size_t DzWriteFile( int fd, const void* buf, size_t count );
DZCOT_API size_t DzSeekFile( int fd, size_t offset, int whence );
DZCOT_API size_t DzGetFileSize( int fd );

#ifdef UNICODE
#define DzOpenFile DzOpenFileW
#else
#define DzOpenFile DzOpenFileA
#endif

DZCOT_API BOOL DzSockStartup();
DZCOT_API BOOL DzSockCleanup();
DZCOT_API int DzSocket( int domain, int type, int protocol );
DZCOT_API int DzShutdown( int fd, int how );
DZCOT_API int DzCloseSocket( int fd );
DZCOT_API int DzBind( int fd, struct sockaddr* addr, int addrLen );
DZCOT_API int DzListen( int fd, int backlog );
DZCOT_API int DzConnect( int fd, struct sockaddr* addr, int addrLen );
DZCOT_API int DzAccept( int fd, struct sockaddr* addr, int* addrLen );
DZCOT_API int DzSend( int fd, const void* buf, int len, int flag );
DZCOT_API int DzRecv( int fd, void* buf, int len, int flag );

DZCOT_API DzParamNode* DzAllocParamNode();
DZCOT_API void DzFreeParamNode( DzParamNode* node );

DZCOT_API unsigned long long DzUnixTime();
DZCOT_API unsigned long long DzMilUnixTime();
#ifdef __cplusplus
};
#endif

#endif // __Dzcot_h__
