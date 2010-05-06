
#ifndef _DZCOT_H_
#define _DZCOT_H_

#if defined(_WIN32) && defined(USE_DZCOT_DLL)
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

DZCOT_API int DzInitHost(
    int         lowestPriority,
    int         defaultPri      __DZ_DFT_ARG( CP_HIGH ),
    int         defaultSSize    __DZ_DFT_ARG( SS_64K )
    );

DZCOT_API int DzStartHost(
    DzRoutine   firstEntry      __DZ_DFT_ARG( NULL ),
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

DZCOT_API int DzGetCotCount();

DZCOT_API int DzGetMaxCotCount(
    BOOL reset                  __DZ_DFT_ARG( FALSE )
    );

DZCOT_API void DzInitCotPool(
    u_int       count,
    u_int       depth,
    int         sSize           __DZ_DFT_ARG( SS_DEFAULT )
    );

DZCOT_API int DzSleep(
    int         milSec          __DZ_DFT_ARG( 0 )
    );

DZCOT_API int DzWaitSynObj(
    DzHandle    obj,
    int         timeOut         __DZ_DFT_ARG( -1 )
    );

DZCOT_API int DzWaitMultiSynObj(
    int         count,
    DzHandle*   obj,
    BOOL        waitAll,
    int         timeOut         __DZ_DFT_ARG( -1 )
    );

DZCOT_API DzHandle DzCreateEvt( BOOL notified, BOOL autoReset );
DZCOT_API BOOL DzSetEvt( DzHandle evt );
DZCOT_API BOOL DzResetEvt( DzHandle evt );
DZCOT_API DzHandle DzCreateSem( u_int count );
DZCOT_API u_int DzReleaseSem( DzHandle sem, int count );
DZCOT_API DzHandle DzCloneSynObj( DzHandle obj );
DZCOT_API BOOL DzCloseSynObj( DzHandle obj );
DZCOT_API DzHandle DzCreateTimer( int milSec, BOOL repeat );
DZCOT_API BOOL DzCloseTimer( DzHandle timer );

DZCOT_API int DzOpenFileA( char *fileName, int flags );
DZCOT_API int DzOpenFileW( wchar_t *fileName, int flags );
DZCOT_API int DzCloseFd( int fd );
DZCOT_API size_t DzReadFile( int fd, void *buff, size_t count );
DZCOT_API size_t DzWriteFile( int fd, void *buff, size_t count );
DZCOT_API off_t DzSeekFile( int fd, off_t offset, int whence );
DZCOT_API size_t DzGetFileSize( int fd );

DZCOT_API BOOL DzSockStartup();
DZCOT_API BOOL DzSockCleanup();
DZCOT_API int DzSocket( int domain, int type, int protocol );
DZCOT_API int DzShutDown( int fd, int how );
DZCOT_API int DzCloseSocket( int fd );
DZCOT_API int DzBind( int fd, struct sockaddr *addr, int addrLen );
DZCOT_API int DzListen( int fd, int backlog );
DZCOT_API int DzConnect( int fd, struct sockaddr *addr, int addrLen );
DZCOT_API int DzAccept( int fd, struct sockaddr *addr, int *addrLen );
DZCOT_API int DzSend( int fd, void *msg, int len, int flag );
DZCOT_API int DzRecv( int fd, void *buf, int len, int flag );

DZCOT_API DzParamNode* DzAllocParamNode();
DZCOT_API void DzFreeParamNode( DzParamNode *node );

#ifdef __cplusplus
};
#endif

#endif  //#ifndef _DZCOT_H_