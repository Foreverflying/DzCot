/********************************************************************
    created:    2010/02/11 22:03
    file:       DzIoWin.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzIoWin_h__
#define __DzIoWin_h__

#include "../DzStructs.h"
#include "../DzStructsOs.h"
#include "../DzBaseOs.h"
#include "../DzBase.h"
#include "../DzResourceMgr.h"
#include "../DzCoreOs.h"
#include "../DzSynObj.h"

#ifdef __cplusplus
extern "C"{
#endif

inline int Socket( DzHost* host, int domain, int type, int protocol )
{
    SOCKET fd;

    fd = socket( domain, type, protocol );
    CreateIoCompletionPort( (HANDLE)fd, host->osStruct.iocp, (ULONG_PTR)NULL, 0 );
    return (int)fd;
}

inline int Shutdown( int fd, int how )
{
    return shutdown( (SOCKET)fd, how );
}

inline int CloseSocket( DzHost* host, int fd )
{
    return closesocket( (SOCKET)fd );
}

inline int Bind( int fd, struct sockaddr* addr, int addrLen )
{
    return bind( (SOCKET)fd, addr, addrLen );
}

inline int Listen( int fd, int backlog )
{
    return listen( (SOCKET)fd, backlog );
}

inline int Connect( DzHost* host, int fd, struct sockaddr* addr, int addrLen )
{
    DWORD bytes;
    DzAsynIo asynIo;
    struct sockaddr tmpAddr;
    DWORD flag;
    int err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzAsynIo* asynIoPtr;

    ZeroMemory( &asynIo.overlapped, sizeof( asynIo.overlapped ) );
    ZeroMemory( &tmpAddr, sizeof( struct sockaddr ) );
    tmpAddr.sa_family = addr->sa_family;
    bind( (SOCKET)fd, &tmpAddr, (int)sizeof(struct sockaddr) );
    if( !host->osStruct._ConnectEx( (SOCKET)fd, addr, addrLen, NULL, 0, &bytes, &asynIo.overlapped ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            SetLastErr( host, err );
            return -1;
        }
        InitFastEvt( &asynIo.fastEvt );
        WaitFastEvt( host, &asynIo.fastEvt, -1 );
    }else{
        GetQueuedCompletionStatus( host->osStruct.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &asynIo.overlapped ){
            host->currPriority = CP_FIRST;
            do{
                asynIoPtr = MEMBER_BASE( overlapped, DzAsynIo, overlapped );
                NotifyFastEvt( host, &asynIoPtr->fastEvt, 0 );
                GetQueuedCompletionStatus( host->osStruct.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &asynIo.overlapped );
        }
    }
    ret = WSAGetOverlappedResult(
        (SOCKET)fd,
        &asynIo.overlapped,
        &bytes,
        FALSE,
        &flag
        );
    if( !ret ){
        SetLastErr( host, (int)WSAGetLastError() );
        return -1;
    }
    SetLastErr( host, 0 );
    return 0;
}

inline int Accept( DzHost* host, int fd, struct sockaddr* addr, int* addrLen )
{
    SOCKET s;
    char buf[64];
    DWORD bytes;
    DzAsynIo asynIo;
    DWORD flag;
    DWORD err;
    BOOL ret;
    struct sockaddr* lAddr;
    struct sockaddr* rAddr;
    int lAddrLen;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzAsynIo* asynIoPtr;

    s = socket( AF_INET, SOCK_STREAM, 0 );
    if( s == INVALID_SOCKET ){
        return -1;
    }
    ZeroMemory( &asynIo.overlapped, sizeof( asynIo.overlapped ) );
    if( !host->osStruct._AcceptEx( (SOCKET)fd, s, buf, 0, 32, 32, &bytes, &asynIo.overlapped ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            closesocket( s );
            SetLastErr( host, err );
            return -1;
        }
        InitFastEvt( &asynIo.fastEvt );
        WaitFastEvt( host, &asynIo.fastEvt, -1 );
    }else{
        GetQueuedCompletionStatus( host->osStruct.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &asynIo.overlapped ){
            host->currPriority = CP_FIRST;
            do{
                asynIoPtr = MEMBER_BASE( overlapped, DzAsynIo, overlapped );
                NotifyFastEvt( host, &asynIoPtr->fastEvt, 0 );
                GetQueuedCompletionStatus( host->osStruct.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &asynIo.overlapped );
        }
    }
    ret = WSAGetOverlappedResult(
        (SOCKET)fd,
        &asynIo.overlapped,
        &bytes,
        FALSE,
        &flag
        );
    if( !ret ){
        closesocket( s );
        SetLastErr( host, (int)WSAGetLastError() );
        return -1;
    }
    setsockopt( s, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&fd, sizeof( fd ) );
    CreateIoCompletionPort( (HANDLE)s, host->osStruct.iocp, (ULONG_PTR)NULL, 0 );
    if( addr ){
        host->osStruct._GetAcceptExSockAddrs( buf, bytes, 32, 32, &lAddr, &lAddrLen, &rAddr, addrLen );
        memcpy( addr, rAddr, *addrLen );
    }
    SetLastErr( host, 0 );
    return (int)s;
}

inline int SendEx( DzHost* host, int fd, DzBuf* bufs, u_int bufCount, int flags )
{
    DWORD bytes;
    DzAsynIo asynIo;
    DWORD tmpFlag;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzAsynIo* asynIoPtr;

    ZeroMemory( &asynIo.overlapped, sizeof( asynIo.overlapped ) );
    if( WSASend( (SOCKET)fd, (WSABUF*)bufs, bufCount, &bytes, flags, &asynIo.overlapped, NULL ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            SetLastErr( host, (int)err );
            return -1;
        }
        InitFastEvt( &asynIo.fastEvt );
        WaitFastEvt( host, &asynIo.fastEvt, -1 );
    }else{
        GetQueuedCompletionStatus( host->osStruct.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &asynIo.overlapped ){
            host->currPriority = CP_FIRST;
            do{
                asynIoPtr = MEMBER_BASE( overlapped, DzAsynIo, overlapped );
                NotifyFastEvt( host, &asynIoPtr->fastEvt, 0 );
                GetQueuedCompletionStatus( host->osStruct.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &asynIo.overlapped );
        }
    }
    ret = WSAGetOverlappedResult(
        (SOCKET)fd,
        &asynIo.overlapped,
        &bytes,
        FALSE,
        &tmpFlag
        );
    if( !ret ){
        SetLastErr( host, (int)WSAGetLastError() );
        return -1;
    }
    SetLastErr( host, 0 );
    return bytes;
}

inline int RecvEx( DzHost* host, int fd, DzBuf* bufs, u_int bufCount, int flags )
{
    DWORD bytes;
    DzAsynIo asynIo;
    DWORD tmpFlag;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzAsynIo* asynIoPtr;

    ZeroMemory( &asynIo.overlapped, sizeof( asynIo.overlapped ) );
    tmpFlag = (DWORD)flags;
    if( WSARecv( (SOCKET)fd, (WSABUF*)bufs, bufCount, &bytes, &tmpFlag, &asynIo.overlapped, NULL ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            SetLastErr( host, (int)err );
            return -1;
        }
        InitFastEvt( &asynIo.fastEvt );
        WaitFastEvt( host, &asynIo.fastEvt, -1 );
    }else{
        GetQueuedCompletionStatus( host->osStruct.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &asynIo.overlapped ){
            host->currPriority = CP_FIRST;
            do{
                asynIoPtr = MEMBER_BASE( overlapped, DzAsynIo, overlapped );
                NotifyFastEvt( host, &asynIoPtr->fastEvt, 0 );
                GetQueuedCompletionStatus( host->osStruct.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &asynIo.overlapped );
        }
    }
    ret = WSAGetOverlappedResult(
        (SOCKET)fd,
        &asynIo.overlapped,
        &bytes,
        FALSE,
        &tmpFlag
        );
    if( !ret ){
        SetLastErr( host, (int)WSAGetLastError() );
        return -1;
    }
    SetLastErr( host, 0 );
    return bytes;
}

inline int Send( DzHost* host, int fd, const void* buf, u_int len, int flags )
{
    DzBuf tmpBuf;
    tmpBuf.len = (unsigned long)len;
    tmpBuf.buf = (void*)buf;

    return SendEx( host, fd, &tmpBuf, 1, flags );
}

inline int Recv( DzHost* host, int fd, void* buf, u_int len, int flags )
{
    DzBuf tmpBuf;
    tmpBuf.len = (unsigned long)len;
    tmpBuf.buf = buf;

    return RecvEx( host, fd, &tmpBuf, 1, flags );
}

inline int SendToEx(
    DzHost*                 host,
    int                     fd,
    DzBuf*                  bufs,
    u_int                   bufCount,
    int                     flags,
    const struct sockaddr*  to,
    int                     tolen
    )
{
    DWORD bytes;
    DzAsynIo asynIo;
    DWORD tmpFlag;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzAsynIo* asynIoPtr;

    ZeroMemory( &asynIo.overlapped, sizeof( asynIo.overlapped ) );
    if( WSASendTo( (SOCKET)fd, (WSABUF*)bufs, bufCount, &bytes, flags, to, tolen, &asynIo.overlapped, NULL ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            SetLastErr( host, (int)err );
            return -1;
        }
        InitFastEvt( &asynIo.fastEvt );
        WaitFastEvt( host, &asynIo.fastEvt, -1 );
    }else{
        GetQueuedCompletionStatus( host->osStruct.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &asynIo.overlapped ){
            host->currPriority = CP_FIRST;
            do{
                asynIoPtr = MEMBER_BASE( overlapped, DzAsynIo, overlapped );
                NotifyFastEvt( host, &asynIoPtr->fastEvt, 0 );
                GetQueuedCompletionStatus( host->osStruct.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &asynIo.overlapped );
        }
    }
    ret = WSAGetOverlappedResult(
        (SOCKET)fd,
        &asynIo.overlapped,
        &bytes,
        FALSE,
        &tmpFlag
        );
    if( !ret ){
        SetLastErr( host, (int)WSAGetLastError() );
        return -1;
    }
    SetLastErr( host, 0 );
    return bytes;
}

inline int RecvFromEx(
    DzHost*                 host,
    int                     fd,
    DzBuf*                  bufs,
    u_int                   bufCount,
    int                     flags,
    struct sockaddr*        from,
    int*                    fromlen
    )
{
    DWORD bytes;
    DzAsynIo asynIo;
    DWORD tmpFlag;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzAsynIo* asynIoPtr;

    ZeroMemory( &asynIo.overlapped, sizeof( asynIo.overlapped ) );
    tmpFlag = (DWORD)flags;
    if( WSARecvFrom( (SOCKET)fd, (WSABUF*)bufs, bufCount, &bytes, &tmpFlag, from, fromlen, &asynIo.overlapped, NULL ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            SetLastErr( host, (int)err );
            return -1;
        }
        InitFastEvt( &asynIo.fastEvt );
        WaitFastEvt( host, &asynIo.fastEvt, -1 );
    }else{
        GetQueuedCompletionStatus( host->osStruct.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &asynIo.overlapped ){
            host->currPriority = CP_FIRST;
            do{
                asynIoPtr = MEMBER_BASE( overlapped, DzAsynIo, overlapped );
                NotifyFastEvt( host, &asynIoPtr->fastEvt, 0 );
                GetQueuedCompletionStatus( host->osStruct.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &asynIo.overlapped );
        }
    }
    ret = WSAGetOverlappedResult(
        (SOCKET)fd,
        &asynIo.overlapped,
        &bytes,
        FALSE,
        &tmpFlag
        );
    if( !ret ){
        SetLastErr( host, (int)WSAGetLastError() );
        return -1;
    }
    SetLastErr( host, 0 );
    return bytes;
}

inline int SendTo(
    DzHost*                 host,
    int                     fd,
    const void*             buf,
    u_int                   len,
    int                     flags,
    const struct sockaddr*  to,
    int                     tolen
    )
{
    DzBuf tmpBuf;
    tmpBuf.len = (unsigned long)len;
    tmpBuf.buf = (void*)buf;

    return SendToEx( host, fd, &tmpBuf, 1, flags, to, tolen );
}

inline int RecvFrom(
    DzHost*                 host,
    int                     fd,
    void*                   buf,
    u_int                   len,
    int                     flags,
    struct sockaddr*        from,
    int*                    fromlen
    )
{
    DzBuf tmpBuf;
    tmpBuf.len = (unsigned long)len;
    tmpBuf.buf = buf;

    return RecvFromEx( host, fd, &tmpBuf, 1, flags, from, fromlen );
}

inline DWORD GetFileFlag( int flags, DWORD* accessFlag )
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

inline int GetFd( DzHost* host, HANDLE file, int flags )
{
    if( file == INVALID_HANDLE_VALUE ){
        return -1;
    }
    CreateIoCompletionPort( file, host->osStruct.iocp, (ULONG_PTR)NULL, 0 );
    if( flags & DZ_O_APPEND && GetFileType( file ) == FILE_TYPE_DISK ){
        SetFilePointer( file, 0, 0, FILE_END );
    }
    return (int)file;
}

inline int OpenA( DzHost* host, const char* fileName, int flags )
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

inline int OpenW( DzHost* host, const wchar_t* fileName, int flags )
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

inline int Close( DzHost* host, int fd )
{
    BOOL ret;
        
    ret = CloseHandle( (HANDLE)fd );
    return ret ? 0 : -1;
}

inline ssize_t Read( DzHost* host, int fd, void* buf, size_t count )
{
    DWORD bytes;
    BOOL isFile;
    DzAsynIo asynIo;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzAsynIo* asynIoPtr;

    isFile = GetFileType( (HANDLE)fd ) == FILE_TYPE_DISK;
    ZeroMemory( &asynIo.overlapped, sizeof( asynIo.overlapped ) );
    if( isFile ){
        asynIo.overlapped.OffsetHigh = 0;
        asynIo.overlapped.Offset = SetFilePointer(
            (HANDLE)fd,
            0,
            (PLONG)&asynIo.overlapped.OffsetHigh,
            FILE_CURRENT
            );
    }
    if( !ReadFile( (HANDLE)fd, buf, (DWORD)count, &bytes, &asynIo.overlapped ) ){
        err = GetLastError();
        if( err == ERROR_HANDLE_EOF ){
            SetLastErr( host, err );
            return 0;
        }else if( err != ERROR_IO_PENDING ){
            SetLastErr( host, err );
            return -1;
        }
        InitFastEvt( &asynIo.fastEvt );
        WaitFastEvt( host, &asynIo.fastEvt, -1 );
    }else{
        GetQueuedCompletionStatus( host->osStruct.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &asynIo.overlapped ){
            host->currPriority = CP_FIRST;
            do{
                asynIoPtr = MEMBER_BASE( overlapped, DzAsynIo, overlapped );
                NotifyFastEvt( host, &asynIoPtr->fastEvt, 0 );
                GetQueuedCompletionStatus( host->osStruct.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &asynIo.overlapped );
        }
    }
    ret = GetOverlappedResult(
        (HANDLE)fd,
        &asynIo.overlapped,
        &bytes,
        FALSE
        );
    if( !ret ){
        err = GetLastError();
        SetLastErr( host, err );
        if( err == ERROR_HANDLE_EOF ){
            return 0;
        }else{
            return -1;
        }
    }
    if( isFile ){
        SetFilePointer( (HANDLE)fd, (LONG)bytes, 0, FILE_CURRENT );
    }
    SetLastErr( host, 0 );
    return bytes;
}

inline ssize_t Write( DzHost* host, int fd, const void* buf, size_t count )
{
    DWORD bytes;
    BOOL isFile;
    DzAsynIo asynIo;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzAsynIo* asynIoPtr;

    isFile = GetFileType( (HANDLE)fd ) == FILE_TYPE_DISK;
    ZeroMemory( &asynIo.overlapped, sizeof( asynIo.overlapped ) );
    if( isFile ){
        asynIo.overlapped.OffsetHigh = 0;
        asynIo.overlapped.Offset = SetFilePointer(
            (HANDLE)fd,
            0,
            (PLONG)&asynIo.overlapped.OffsetHigh,
            FILE_CURRENT
            );
    }
    if( !WriteFile( (HANDLE)fd, buf, (DWORD)count, &bytes, &asynIo.overlapped )  ){
        err = GetLastError();
        if( err == ERROR_HANDLE_EOF ){
            SetLastErr( host, err );
            return 0;
        }else if( err != ERROR_IO_PENDING ){
            SetLastErr( host, err );
            return -1;
        }
        InitFastEvt( &asynIo.fastEvt );
        WaitFastEvt( host, &asynIo.fastEvt, -1 );
    }else{
        GetQueuedCompletionStatus( host->osStruct.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &asynIo.overlapped ){
            host->currPriority = CP_FIRST;
            do{
                asynIoPtr = MEMBER_BASE( overlapped, DzAsynIo, overlapped );
                NotifyFastEvt( host, &asynIoPtr->fastEvt, 0 );
                GetQueuedCompletionStatus( host->osStruct.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &asynIo.overlapped );
        }
    }
    ret = GetOverlappedResult(
        (HANDLE)fd,
        &asynIo.overlapped,
        &bytes,
        FALSE
        );
    if( !ret ){
        err = GetLastError();
        SetLastErr( host, err );
        if( err == ERROR_HANDLE_EOF ){
            return 0;
        }else{
            return -1;
        }
    }
    if( isFile ){
        SetFilePointer( (HANDLE)fd, (LONG)bytes, 0, FILE_CURRENT );
    }
    SetLastErr( host, 0 );
    return bytes;
}

inline size_t Seek( DzHost* host, int fd, ssize_t offset, int whence )
{
    size_t ret;

#if defined( _X86_ )
    ret = (size_t)SetFilePointer( (HANDLE)fd, (long)offset, NULL, whence );
#elif defined( _M_AMD64 )
    SetFilePointerEx( (HANDLE)fd, *(LARGE_INTEGER*)&offset, (LARGE_INTEGER*)&ret, whence );
#endif

    return ret;
}

inline size_t FileSize( DzHost* host, int fd )
{
    size_t ret;

#if defined( _X86_ )
    ret = GetFileSize( (HANDLE)fd, NULL );
#elif defined( _M_AMD64 )
    GetFileSizeEx( (HANDLE)fd, (LARGE_INTEGER*)&ret );
#endif
    return ret;
}

// IoMgrRoutine:
// the IO mgr thread uses the host's origin thread's stack
// manager all kernel objects that may cause real block
inline void IoMgrRoutine( DzHost* host )
{
    ULONG_PTR key;
    DWORD bytes;
    OVERLAPPED* overlapped;
    DzAsynIo* asynIo;
    DWORD timeout;

    timeout = (DWORD)NotifyMinTimers( host );
    while( host->threadCount ){
        GetQueuedCompletionStatus( host->osStruct.iocp, &bytes, &key, &overlapped, timeout );
        if( overlapped != NULL ){
            do{
                asynIo = MEMBER_BASE( overlapped, DzAsynIo, overlapped );
                NotifyFastEvt( host, &asynIo->fastEvt, 0 );
                GetQueuedCompletionStatus( host->osStruct.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != NULL );
            host->currPriority = CP_FIRST;
            Schedule( host );
        }
        timeout = (DWORD)NotifyMinTimers( host );
    }
}

#ifdef __cplusplus
};
#endif

#endif // __DzIoWin_h__
