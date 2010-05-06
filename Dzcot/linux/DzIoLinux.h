
#ifndef _DZ_IO_WIN_H_
#define _DZ_IO_WIN_H_

#include "../DzConstant.h"
#include "../DzStructs.h"
#include "../DzStructsOs.h"
#include "../DzCoreOs.h"
#include "../DzResourceMgr.h"
#include "../DzSynObj.h"
#include "../../DzcotData/DzcotData.h"
#include <MSWSock.h>

#ifdef __cplusplus
extern "C"{
#endif

BOOL SockStartup();
BOOL SockCleanup();

inline DzAsynIo* CreateAsynIo( DzHost *host )
{
    DzAsynIo *asynIo;

    asynIo = AllocAsynIo( host );
    asynIo->ref = 1;
    return asynIo;
}

inline DzAsynIo* CloneAsynIo( DzAsynIo *asynIo )
{
    asynIo->ref++;
    return asynIo;
}

inline void CloseAsynIo( DzHost *host, DzAsynIo *asynIo )
{
    asynIo->ref--;
    if( asynIo->ref == 0 ){
        FreeAsynIo( host, asynIo );
    }
}

inline int Socket( DzHost *host, int domain, int type, int protocol )
{
    SOCKET fd;

    fd = socket( domain, type, protocol );
    CreateIoCompletionPort( (HANDLE)fd, host->ioMgr.iocp, (ULONG_PTR)NULL, 0 );
    return (int)fd;
}

inline int ShutDown( int fd, int how )
{
    return shutdown( (SOCKET)fd, how );
}

inline int CloseSocket( DzHost *host, int fd )
{
    return closesocket( (SOCKET)fd );
}

inline int Bind( int fd, struct sockaddr *addr, int addrLen )
{
    return bind( (SOCKET)fd, addr, addrLen );
}

inline int Listen( int fd, int backlog )
{
    return listen( (SOCKET)fd, backlog );
}

inline int Connect( DzHost *host, int fd, struct sockaddr *addr, int addrLen )
{
    BOOL result = TRUE;
    DWORD bytes;
    DzAsynIo *asynIo;
    struct sockaddr temp;
    DWORD flag;
    int err;

    asynIo = CreateAsynIo( host );
    asynIo->fd = fd;
    asynIo->callback = NULL;
    InitFastEvt( &asynIo->fastEvt );
    ZeroMemory( &asynIo->overlapped, sizeof( asynIo->overlapped ) );
    ZeroMemory( &temp, sizeof( struct sockaddr ) );
    temp.sa_family = addr->sa_family;
    bind( (SOCKET)fd, &temp, (int)sizeof(struct sockaddr) );
    if( !_ConnectEx( (SOCKET)fd, addr, addrLen, NULL, 0, &bytes, &asynIo->overlapped ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            result = FALSE;
            goto endproc;
        }
        CloneAsynIo( asynIo );
        WaitFastEvt( host, &asynIo->fastEvt, -1 );
        result = WSAGetOverlappedResult(
            (SOCKET)fd,
            &asynIo->overlapped,
            &bytes,
            FALSE,
            &flag
            );
    }else{
        //complete directly
        asynIo->fd = -1;
        return 0;
    }
endproc:
    CloseAsynIo( host, asynIo );
    return result ? 0 : -1;
}

inline int Accept( DzHost *host, int fd, struct sockaddr *addr, int *addrLen )
{
    SOCKET s;
    char buff[64];
    DWORD bytes;
    DzAsynIo *asynIo;
    DWORD err;
    DWORD flag;
    BOOL result;
    struct sockaddr *lAddr;
    struct sockaddr *rAddr;
    int lAddrLen;

    s = socket( AF_INET, SOCK_STREAM, 0 );
    if( !s ){
        return -1;
    }
    asynIo = CreateAsynIo( host );
    asynIo->fd = fd;
    asynIo->callback = NULL;
    InitFastEvt( &asynIo->fastEvt );
    ZeroMemory( &asynIo->overlapped, sizeof( asynIo->overlapped ) );
    if( !_AcceptEx( (SOCKET)fd, s, buff, 0, 32, 32, &bytes, &asynIo->overlapped ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            closesocket( s );
            bytes = -1;
            goto endproc;
        }
        CloneAsynIo( asynIo );
        WaitFastEvt( host, &asynIo->fastEvt, -1 );
        result = WSAGetOverlappedResult(
            (SOCKET)fd,
            &asynIo->overlapped,
            &bytes,
            FALSE,
            &flag
            );
        if( !result ){
            closesocket( s );
            bytes = -1;
            goto endproc;
        }
    }else{
        //complete directly
        asynIo->fd = -1;
    }
    CreateIoCompletionPort( (HANDLE)s, host->ioMgr.iocp, (ULONG_PTR)NULL, 0 );
    if( addr ){
        _GetAcceptExSockAddrs( buff, bytes, 32, 32, &lAddr, &lAddrLen, &rAddr, addrLen );
        memcpy( addr, rAddr, *addrLen );
    }
    if( asynIo->fd == -1 ){
        return (int)s;
    }
endproc:
    CloseAsynIo( host, asynIo );
    return (int)s;
}

inline int Send( DzHost *host, int fd, void *msg, int len, int flag )
{
    DWORD bytes;
    DzAsynIo *asynIo;
    WSABUF tmpBuf;
    DWORD err;
    DWORD tmpFlag;
    BOOL result;

    //printf("/---------------send begin\t%d\r\n", fd );

    asynIo = CreateAsynIo( host );
    asynIo->fd = fd;
    asynIo->callback = NULL;
    InitFastEvt( &asynIo->fastEvt );
    ZeroMemory( &asynIo->overlapped, sizeof( asynIo->overlapped ) );
    tmpBuf.len = (ULONG)len;
    tmpBuf.buf = (char*)msg;
    if( WSASend( (SOCKET)fd, &tmpBuf, 1, &bytes, flag, &asynIo->overlapped, NULL ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            bytes = -1;
            goto endproc;
        }
        CloneAsynIo( asynIo );
        WaitFastEvt( host, &asynIo->fastEvt, -1 );
        result = WSAGetOverlappedResult(
            (SOCKET)fd,
            &asynIo->overlapped,
            &bytes,
            FALSE,
            &tmpFlag
            );
        if( !result ){
            bytes = -1;
            goto endproc;
        }
    }else{
        //complete directly
        asynIo->fd = -1;
        return bytes;
    }
    //printf("\\---------------send end\t%d\r\n", fd );
endproc:
    CloseAsynIo( host, asynIo );
    return bytes;
}

inline int Recv( DzHost *host, int fd, void *buf, int len, int flag )
{
    DWORD bytes;
    DzAsynIo *asynIo;
    WSABUF tmpBuf;
    DWORD tmpFlag;
    DWORD err;
    BOOL result;

    //printf("/---------------recv begin\t%d\r\n", fd );

    asynIo = CreateAsynIo( host );
    asynIo->fd = fd;
    asynIo->callback = NULL;
    InitFastEvt( &asynIo->fastEvt );
    ZeroMemory( &asynIo->overlapped, sizeof( asynIo->overlapped ) );
    tmpBuf.len = (ULONG)len;
    tmpBuf.buf = (char*)buf;
    tmpFlag = (DWORD)flag;
    if( WSARecv( (SOCKET)fd, &tmpBuf, 1, &bytes, &tmpFlag, &asynIo->overlapped, NULL ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            bytes = -1;
            goto endproc;
        }
        //printf( "Bingo, switched!\t%d\r\n", fd );
        CloneAsynIo( asynIo );
        WaitFastEvt( host, &asynIo->fastEvt, -1 );
        result = WSAGetOverlappedResult(
            (SOCKET)fd,
            &asynIo->overlapped,
            &bytes,
            FALSE,
            &tmpFlag
            );
        if( !result ){
            bytes = -1;
            goto endproc;
        }
    }else{
        //complete directly
        asynIo->fd = -1;
        return bytes;
    }
    //printf("\\---------------recv end\t%d\r\n", fd );
endproc:
    CloseAsynIo( host, asynIo );
    return bytes;
}

inline DWORD GetFileFlag( int flags, DWORD *accessFlag )
{
    DWORD access = 0;
    DWORD createFlag = 0;

    if( flags & DZ_O_RD ){
        access |= GENERIC_READ;
    }
    if( flags & DZ_O_WR ){
        access |= GENERIC_WRITE;
    }
    if( flags & DZ_O_CREATE ){
        if( flags & DZ_O_EXCL ){
            createFlag = CREATE_NEW;
        }else if( flags & DZ_O_TRUNC ){
            createFlag = CREATE_ALWAYS;
        }else{
            createFlag = OPEN_ALWAYS;
        }
    }else{
        if( flags & DZ_O_TRUNC ){
            createFlag = TRUNCATE_EXISTING;
        }else{
            createFlag = OPEN_EXISTING;
        }
    }
    access &= ~SYNCHRONIZE;
    *accessFlag = access;
    return createFlag;
}

inline int GetFd( DzHost *host, HANDLE file, int flags )
{
    if( file == INVALID_HANDLE_VALUE ){
        return -1;
    }
    CreateIoCompletionPort( file, host->ioMgr.iocp, (ULONG_PTR)NULL, 0 );
    if( flags & DZ_O_APPEND && GetFileType( file ) == FILE_TYPE_DISK ){
        SetFilePointer( file, 0, 0, FILE_END );
    }
    return (int)file;
}

inline int OpenA( DzHost *host, char *fileName, int flags )
{
    DWORD access = 0;
    DWORD createFlag;
    HANDLE file;
    
    createFlag = GetFileFlag( flags, &access );
    file = CreateFileA(
        fileName,
        access,
        0,
        0,
        createFlag,
        FILE_FLAG_OVERLAPPED,
        NULL
        );
    return GetFd( host, file, flags );
}

inline int OpenW( DzHost *host, wchar_t *fileName, int flags )
{
    DWORD access = 0;
    DWORD createFlag;
    HANDLE file;
    
    createFlag = GetFileFlag( flags, &access );
    file = CreateFileW(
        fileName,
        access,
        0,
        0,
        createFlag,
        FILE_FLAG_OVERLAPPED,
        NULL
        );
    return GetFd( host, file, flags );
}

inline int Close( DzHost *host, int fd )
{
    BOOL ret;
        
    ret = CloseHandle( (HANDLE)fd );
    return ret ? 0 : -1;
}

inline size_t Read( DzHost *host, int fd, void *buff, size_t count )
{
    DWORD bytes;
    BOOL isFile;
    DzAsynIo *asynIo;
    DWORD err;
    BOOL result;

    asynIo = AllocAsynIo( host );
    isFile = GetFileType( (HANDLE)fd ) == FILE_TYPE_DISK;
    asynIo->fd = fd;
    asynIo->callback = NULL;
    InitFastEvt( &asynIo->fastEvt );
    ZeroMemory( &asynIo->overlapped, sizeof( asynIo->overlapped ) );
    if( isFile ){
        asynIo->overlapped.OffsetHigh = 0;
        asynIo->overlapped.Offset = SetFilePointer(
            (HANDLE)fd,
            0,
            (PLONG)&asynIo->overlapped.OffsetHigh,
            FILE_CURRENT
            );
    }
    if( !ReadFile( (HANDLE)fd, buff, (DWORD)count, &bytes, &asynIo->overlapped ) ){
        err = GetLastError();
        if( err == ERROR_HANDLE_EOF ){
            bytes = 0;
            goto endproc;
        }else if( err != ERROR_IO_PENDING ){
            bytes = -1;
            goto endproc;
        }
        CloneAsynIo( asynIo );
        WaitFastEvt( host, &asynIo->fastEvt, -1 );
        result = GetOverlappedResult(
            (HANDLE)fd,
            &asynIo->overlapped,
            &bytes,
            FALSE
            );
        if( !result ){
            err = GetLastError();
            if( err == ERROR_HANDLE_EOF ){
                bytes = 0;
                goto endproc;
            }else{
                bytes = -1;
                goto endproc;
            }
        }
    }else{
        //complete directly
        asynIo->fd = -1;
    }
    if( isFile ){
        SetFilePointer( (HANDLE)fd, (LONG)bytes, 0, FILE_CURRENT );
    }
    if( asynIo->fd = -1 ){
        return bytes;
    }
endproc:
    CloseAsynIo( host, asynIo );
    return bytes;
}

inline size_t Write( DzHost *host, int fd, void *buff, size_t count )
{
    DWORD bytes;
    BOOL isFile;
    DzAsynIo *asynIo;
    DWORD err;
    BOOL result;

    isFile = GetFileType( (HANDLE)fd ) == FILE_TYPE_DISK;
    asynIo = CreateAsynIo( host );
    asynIo->fd = fd;
    asynIo->callback = NULL;
    InitFastEvt( &asynIo->fastEvt );
    ZeroMemory( &asynIo->overlapped, sizeof( asynIo->overlapped ) );
    if( isFile ){
        asynIo->overlapped.OffsetHigh = 0;
        asynIo->overlapped.Offset = SetFilePointer(
            (HANDLE)fd,
            0,
            (PLONG)&asynIo->overlapped.OffsetHigh,
            FILE_CURRENT
            );
    }
    if( !WriteFile( (HANDLE)fd, buff, (DWORD)count, &bytes, &asynIo->overlapped )  ){
        err = GetLastError();
        if( err == ERROR_HANDLE_EOF ){
            bytes = 0;
            goto endproc;
        }else if( err != ERROR_IO_PENDING ){
            bytes = -1;
            goto endproc;
        }
        CloneAsynIo( asynIo );
        WaitFastEvt( host, &asynIo->fastEvt, -1 );
        result = GetOverlappedResult(
            (HANDLE)fd,
            &asynIo->overlapped,
            &bytes,
            FALSE
            );
        if( !result ){
            err = GetLastError();
            if( err == ERROR_HANDLE_EOF ){
                bytes = 0;
                goto endproc;
            }else{
                bytes = -1;
                goto endproc;
            }
        }
    }else{
        //complete directly
        CloneAsynIo( asynIo );
        asynIo->fd = -1;
    }
    if( isFile ){
        SetFilePointer( (HANDLE)fd, (LONG)bytes, 0, FILE_CURRENT );
    }
    if( asynIo->fd = -1 ){
        return bytes;
    }
endproc:
    CloseAsynIo( host, asynIo );
    return bytes;
}

union DZQWORD{
    struct{
        DWORD   low;
        DWORD   high;
    };
    long long   qWord;
};

inline off_t64 Seek( DzHost *host, int fd, off_t64 offset, int whence )
{
    union DZQWORD ret;

    ret.qWord = offset;
    ret.low = SetFilePointer( (HANDLE)fd, (LONG)ret.low, (PLONG)&ret.high, whence );
    return (off_t64)ret.qWord;
}

inline size_t64 FileSize( int fd )
{
    union DZQWORD ret;

    ret.low = GetFileSize( (HANDLE)fd, &ret.high );
    return (size_t64)ret.qWord;
}

inline void InitIoMgr( DzHost *host )
{
    host->ioMgr.iocp = CreateIoCompletionPort(
        INVALID_HANDLE_VALUE,
        NULL,
        (ULONG_PTR)NULL,
        1
        );
}

// DzIoMgrRoutine:
// the IO mgr thread uses the host's origin thread's stack
// manager all kernel objects that may cause real block
inline void IoMgrRoutine( DzHost *host, BOOL block )
{
    ULONG_PTR key;
    DWORD bytes;
    OVERLAPPED *overlapped;
    DzAsynIo *asynIo;
    DWORD timeOut;
    static int DEBUGcallTime = 0;

    while( !host->isExiting || host->threadCount > 1 ){
        timeOut = (DWORD)GetMinWaitTime( host );
        GetQueuedCompletionStatus( host->ioMgr.iocp, &bytes, &key, &overlapped, timeOut );
        host->currPriority = CP_INSTANT;
        if( overlapped != NULL ){
            do{
                asynIo = MEMBER_BASE( overlapped, DzAsynIo, overlapped );
                if( asynIo->fd != -1 ){
                    if( asynIo->callback ){
                        asynIo->callback( asynIo->context );
                    }else{
                        NotifyFastEvt( host, &asynIo->fastEvt, 0 );
                    }
                }
                CloseAsynIo( host, asynIo );
                GetQueuedCompletionStatus( host->ioMgr.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != NULL );
        }else if( GetLastError() == WAIT_TIMEOUT ){
            NotifyMinTimers( host );
        }
        Schedule( host );
        //printf( "IoComplete called again! time:\t%d\r\n", DEBUGcallTime++ );
    }
}

#ifdef __cplusplus
};
#endif

#endif  //#ifndef _DZ_IO_WIN_H_