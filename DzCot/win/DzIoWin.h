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
#include "../DzBase.h"
#include "../DzResourceMgr.h"
#include "../DzSynObj.h"
#include "../DzDebug.h"

#ifdef __cplusplus
extern "C"{
#endif

inline int Socket( DzHost* host, int domain, int type, int protocol )
{
    SOCKET fd;

    fd = socket( domain, type, protocol );
    if( fd == INVALID_SOCKET ){
        __DbgSetLastErr( host, WSAGetLastError() );
        return -1;
    }
    CreateIoCompletionPort( (HANDLE)fd, host->os.iocp, (ULONG_PTR)NULL, 0 );
    return (int)fd;
}

inline int CloseSocket( int fd )
{
    return closesocket( (SOCKET)fd );
}

inline int GetSockOpt( int fd, int level, int name, void* option, int* len )
{
    return getsockopt( (SOCKET)fd, level, name, (char*)option, len );
}

inline int SetSockOpt( int fd, int level, int name, const void* option, int len )
{
    return setsockopt( (SOCKET)fd, level, name, (const char*)option, len );
}

inline int GetSockName( int fd, struct sockaddr* addr, int* addrLen )
{
    return getsockname( (SOCKET)fd, addr, addrLen );
}

inline int Bind( int fd, struct sockaddr* addr, int addrLen )
{
    return bind( (SOCKET)fd, addr, addrLen );
}

inline int Listen( int fd, int backlog )
{
    return listen( (SOCKET)fd, backlog );
}

inline int Shutdown( int fd, int how )
{
    return shutdown( (SOCKET)fd, how );
}

inline int TryConnectDatagram( int fd, struct sockaddr* addr, int addrLen )
{
    int sockType;
    int sockTypeLen = sizeof( sockType );

    if( getsockopt( fd, SOL_SOCKET, SO_TYPE, (char*)&sockType, &sockTypeLen ) ){
        return -1;
    }
    if( sockType == SOCK_DGRAM ){
        return connect( fd, addr, addrLen );
    }
    return -1;
}

inline int Connect( DzHost* host, int fd, struct sockaddr* addr, int addrLen )
{
    DWORD bytes;
    DzAsyncIo asyncIo;
    struct sockaddr tmpAddr;
    DWORD flag;
    int err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzAsyncIo* asyncIoPtr;

    ZeroMemory( &asyncIo.overlapped, sizeof( asyncIo.overlapped ) );
    ZeroMemory( &tmpAddr, sizeof( struct sockaddr ) );
    tmpAddr.sa_family = addr->sa_family;
    bind( (SOCKET)fd, &tmpAddr, (int)sizeof(struct sockaddr) );
    if( !host->os._ConnectEx( (SOCKET)fd, addr, addrLen, NULL, 0, &bytes, &asyncIo.overlapped ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            if( err == WSAEINVAL ){
                if( TryConnectDatagram( fd, addr, addrLen ) == 0 ){
                    return 0;
                }
            }
            __DbgSetLastErr( host, err );
            return -1;
        }
        WaitEasyEvt( host, &asyncIo.easyEvt );
    }else{
        GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &asyncIo.overlapped ){
            host->currPri = CP_FIRST;
            do{
                if( !key ){
                    asyncIoPtr = MEMBER_BASE( overlapped, DzAsyncIo, overlapped );
                    NotifyEasyEvt( host, &asyncIoPtr->easyEvt );
                }
                GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &asyncIo.overlapped );
        }
    }
    ret = WSAGetOverlappedResult(
        (SOCKET)fd,
        &asyncIo.overlapped,
        &bytes,
        FALSE,
        &flag
        );
    if( !ret ){
        __DbgSetLastErr( host, WSAGetLastError() );
        return -1;
    }
    setsockopt( fd, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0 );
    return 0;
}

inline int Accept( DzHost* host, int fd, struct sockaddr* addr, int* addrLen )
{
    SOCKET s;
    char buf[64];
    DWORD bytes;
    DzAsyncIo asyncIo;
    DWORD flag;
    DWORD err;
    BOOL ret;
    struct sockaddr* lAddr;
    struct sockaddr* rAddr;
    int lAddrLen;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzAsyncIo* asyncIoPtr;

    s = socket( AF_INET, SOCK_STREAM, 0 );
    if( s == INVALID_SOCKET ){
        return -1;
    }
    ZeroMemory( &asyncIo.overlapped, sizeof( asyncIo.overlapped ) );
    if( !host->os._AcceptEx( (SOCKET)fd, s, buf, 0, 32, 32, &bytes, &asyncIo.overlapped ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            closesocket( s );
            __DbgSetLastErr( host, err );
            return -1;
        }
        WaitEasyEvt( host, &asyncIo.easyEvt );
    }else{
        GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &asyncIo.overlapped ){
            host->currPri = CP_FIRST;
            do{
                if( !key ){
                    asyncIoPtr = MEMBER_BASE( overlapped, DzAsyncIo, overlapped );
                    NotifyEasyEvt( host, &asyncIoPtr->easyEvt );
                }
                GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &asyncIo.overlapped );
        }
    }
    ret = WSAGetOverlappedResult(
        (SOCKET)fd,
        &asyncIo.overlapped,
        &bytes,
        FALSE,
        &flag
        );
    if( !ret ){
        closesocket( s );
        __DbgSetLastErr( host, WSAGetLastError() );
        return -1;
    }
    key = (ULONG_PTR)fd;
    setsockopt( s, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&key, sizeof( key ) );
    CreateIoCompletionPort( (HANDLE)s, host->os.iocp, (ULONG_PTR)NULL, 0 );
    if( addr ){
        host->os._GetAcceptExSockAddrs( buf, bytes, 32, 32, &lAddr, &lAddrLen, &rAddr, addrLen );
        memcpy( addr, rAddr, *addrLen );
    }
    return (int)s;
}

inline int SendEx( DzHost* host, int fd, DzBuf* bufs, u_int bufCount, int flags )
{
    DWORD bytes;
    DzAsyncIo asyncIo;
    DWORD tmpFlag;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzAsyncIo* asyncIoPtr;

    ZeroMemory( &asyncIo.overlapped, sizeof( asyncIo.overlapped ) );
    if( WSASend( (SOCKET)fd, (WSABUF*)bufs, bufCount, &bytes, flags, &asyncIo.overlapped, NULL ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            __DbgSetLastErr( host, err );
            return -1;
        }
        WaitEasyEvt( host, &asyncIo.easyEvt );
    }else{
        GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &asyncIo.overlapped ){
            host->currPri = CP_FIRST;
            do{
                if( !key ){
                    asyncIoPtr = MEMBER_BASE( overlapped, DzAsyncIo, overlapped );
                    NotifyEasyEvt( host, &asyncIoPtr->easyEvt );
                }
                GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &asyncIo.overlapped );
        }
    }
    ret = WSAGetOverlappedResult(
        (SOCKET)fd,
        &asyncIo.overlapped,
        &bytes,
        FALSE,
        &tmpFlag
        );
    if( !ret ){
        __DbgSetLastErr( host, WSAGetLastError() );
        return -1;
    }
    return bytes;
}

inline int RecvEx( DzHost* host, int fd, DzBuf* bufs, u_int bufCount, int flags )
{
    DWORD bytes;
    DzAsyncIo asyncIo;
    DWORD tmpFlag;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzAsyncIo* asyncIoPtr;

    ZeroMemory( &asyncIo.overlapped, sizeof( asyncIo.overlapped ) );
    tmpFlag = (DWORD)flags;
    if( WSARecv( (SOCKET)fd, (WSABUF*)bufs, bufCount, &bytes, &tmpFlag, &asyncIo.overlapped, NULL ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            __DbgSetLastErr( host, err );
            return -1;
        }
        WaitEasyEvt( host, &asyncIo.easyEvt );
    }else{
        GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &asyncIo.overlapped ){
            host->currPri = CP_FIRST;
            do{
                if( !key ){
                    asyncIoPtr = MEMBER_BASE( overlapped, DzAsyncIo, overlapped );
                    NotifyEasyEvt( host, &asyncIoPtr->easyEvt );
                }
                GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &asyncIo.overlapped );
        }
    }
    ret = WSAGetOverlappedResult(
        (SOCKET)fd,
        &asyncIo.overlapped,
        &bytes,
        FALSE,
        &tmpFlag
        );
    if( !ret ){
        __DbgSetLastErr( host, WSAGetLastError() );
        return -1;
    }
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
    DzAsyncIo asyncIo;
    DWORD tmpFlag;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzAsyncIo* asyncIoPtr;

    ZeroMemory( &asyncIo.overlapped, sizeof( asyncIo.overlapped ) );
    if( WSASendTo( (SOCKET)fd, (WSABUF*)bufs, bufCount, &bytes, flags, to, tolen, &asyncIo.overlapped, NULL ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            __DbgSetLastErr( host, err );
            return -1;
        }
        WaitEasyEvt( host, &asyncIo.easyEvt );
    }else{
        GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &asyncIo.overlapped ){
            host->currPri = CP_FIRST;
            do{
                if( !key ){
                    asyncIoPtr = MEMBER_BASE( overlapped, DzAsyncIo, overlapped );
                    NotifyEasyEvt( host, &asyncIoPtr->easyEvt );
                }
                GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &asyncIo.overlapped );
        }
    }
    ret = WSAGetOverlappedResult(
        (SOCKET)fd,
        &asyncIo.overlapped,
        &bytes,
        FALSE,
        &tmpFlag
        );
    if( !ret ){
        __DbgSetLastErr( host, WSAGetLastError() );
        return -1;
    }
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
    DzAsyncIo asyncIo;
    DWORD tmpFlag;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzAsyncIo* asyncIoPtr;

    ZeroMemory( &asyncIo.overlapped, sizeof( asyncIo.overlapped ) );
    tmpFlag = (DWORD)flags;
    if( WSARecvFrom( (SOCKET)fd, (WSABUF*)bufs, bufCount, &bytes, &tmpFlag, from, fromlen, &asyncIo.overlapped, NULL ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            __DbgSetLastErr( host, err );
            return -1;
        }
        WaitEasyEvt( host, &asyncIo.easyEvt );
    }else{
        GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &asyncIo.overlapped ){
            host->currPri = CP_FIRST;
            do{
                if( !key ){
                    asyncIoPtr = MEMBER_BASE( overlapped, DzAsyncIo, overlapped );
                    NotifyEasyEvt( host, &asyncIoPtr->easyEvt );
                }
                GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &asyncIo.overlapped );
        }
    }
    ret = WSAGetOverlappedResult(
        (SOCKET)fd,
        &asyncIo.overlapped,
        &bytes,
        FALSE,
        &tmpFlag
        );
    if( !ret ){
        __DbgSetLastErr( host, WSAGetLastError() );
        return -1;
    }
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
    DWORD rwFlag = 0;

    rwFlag = flags & 3;
    rwFlag++;
    if( rwFlag & 1 ){
        access |= GENERIC_READ;
    }
    if( rwFlag & 2 ){
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
    CreateIoCompletionPort( file, host->os.iocp, (ULONG_PTR)NULL, 0 );
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

inline int Close( int fd )
{
    return CloseHandle( (HANDLE)fd ) ? 0 : -1;
}

inline ssize_t Read( DzHost* host, int fd, void* buf, size_t count )
{
    DWORD bytes;
    BOOL isFile;
    DzAsyncIo asyncIo;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzAsyncIo* asyncIoPtr;

    isFile = GetFileType( (HANDLE)fd ) == FILE_TYPE_DISK;
    ZeroMemory( &asyncIo.overlapped, sizeof( asyncIo.overlapped ) );
    if( isFile ){
        asyncIo.overlapped.OffsetHigh = 0;
        asyncIo.overlapped.Offset = SetFilePointer(
            (HANDLE)fd,
            0,
            (PLONG)&asyncIo.overlapped.OffsetHigh,
            FILE_CURRENT
            );
    }
    if( !ReadFile( (HANDLE)fd, buf, (DWORD)count, &bytes, &asyncIo.overlapped ) ){
        err = GetLastError();
        if( err != ERROR_IO_PENDING ){
            if( err == ERROR_HANDLE_EOF ){
                return 0;
            }else{
                __DbgSetLastErr( host, (int)err );
                return -1;
            }
        }
        WaitEasyEvt( host, &asyncIo.easyEvt );
    }else{
        GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &asyncIo.overlapped ){
            host->currPri = CP_FIRST;
            do{
                if( !key ){
                    asyncIoPtr = MEMBER_BASE( overlapped, DzAsyncIo, overlapped );
                    NotifyEasyEvt( host, &asyncIoPtr->easyEvt );
                }
                GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &asyncIo.overlapped );
        }
    }
    ret = GetOverlappedResult(
        (HANDLE)fd,
        &asyncIo.overlapped,
        &bytes,
        FALSE
        );
    if( !ret ){
        err = GetLastError();
        if( err == ERROR_HANDLE_EOF ){
            return 0;
        }else{
            __DbgSetLastErr( host, (int)err );
            return -1;
        }
    }
    if( isFile ){
        SetFilePointer( (HANDLE)fd, (LONG)bytes, 0, FILE_CURRENT );
    }
    return bytes;
}

inline ssize_t Write( DzHost* host, int fd, const void* buf, size_t count )
{
    DWORD bytes;
    BOOL isFile;
    DzAsyncIo asyncIo;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzAsyncIo* asyncIoPtr;

    isFile = GetFileType( (HANDLE)fd ) == FILE_TYPE_DISK;
    ZeroMemory( &asyncIo.overlapped, sizeof( asyncIo.overlapped ) );
    if( isFile ){
        asyncIo.overlapped.OffsetHigh = 0;
        asyncIo.overlapped.Offset = SetFilePointer(
            (HANDLE)fd,
            0,
            (PLONG)&asyncIo.overlapped.OffsetHigh,
            FILE_CURRENT
            );
    }
    if( !WriteFile( (HANDLE)fd, buf, (DWORD)count, &bytes, &asyncIo.overlapped )  ){
        err = GetLastError();
        if( err != ERROR_IO_PENDING ){
            if( err == ERROR_HANDLE_EOF ){
                return 0;
            }else{
                __DbgSetLastErr( host, (int)err );
                return -1;
            }
        }
        WaitEasyEvt( host, &asyncIo.easyEvt );
    }else{
        GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &asyncIo.overlapped ){
            host->currPri = CP_FIRST;
            do{
                if( !key ){
                    asyncIoPtr = MEMBER_BASE( overlapped, DzAsyncIo, overlapped );
                    NotifyEasyEvt( host, &asyncIoPtr->easyEvt );
                }
                GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &asyncIo.overlapped );
        }
    }
    ret = GetOverlappedResult(
        (HANDLE)fd,
        &asyncIo.overlapped,
        &bytes,
        FALSE
        );
    if( !ret ){
        err = GetLastError();
        __DbgSetLastErr( host, (int)err );
        if( err == ERROR_HANDLE_EOF ){
            return 0;
        }else{
            return -1;
        }
    }
    if( isFile ){
        SetFilePointer( (HANDLE)fd, (LONG)bytes, 0, FILE_CURRENT );
    }
    return bytes;
}

inline size_t Seek( int fd, ssize_t offset, int whence )
{
    size_t ret;

#if defined( _X86_ )
    ret = (size_t)SetFilePointer( (HANDLE)fd, (long)offset, NULL, whence );
#elif defined( _M_AMD64 )
    if( !SetFilePointerEx( (HANDLE)fd, *(LARGE_INTEGER*)&offset, (LARGE_INTEGER*)&ret, whence ) ){
        return -1;
    }
#endif

    return ret;
}

inline size_t FileSize( int fd )
{
    size_t ret;

#if defined( _X86_ )
    ret = GetFileSize( (HANDLE)fd, NULL );
#elif defined( _M_AMD64 )
    if( !GetFileSizeEx( (HANDLE)fd, (LARGE_INTEGER*)&ret ) ){
        return -1;
    }
#endif

    return ret;
}

inline void BlockAndDispatchIo( DzHost* host, int timeout )
{
    ULONG_PTR key;
    DWORD n;
    OVERLAPPED* overlapped;
    DzAsyncIo* asyncIo;

    GetQueuedCompletionStatus( host->os.iocp, &n, &key, &overlapped, (DWORD)timeout );
    AtomAndInt( host->rmtCheckSignPtr, ~RMT_CHECK_SLEEP_SIGN );
    if( overlapped != NULL ){
        do{
            if( !key ){
                asyncIo = MEMBER_BASE( overlapped, DzAsyncIo, overlapped );
                NotifyEasyEvt( host, &asyncIo->easyEvt );
            }
            GetQueuedCompletionStatus( host->os.iocp, &n, &key, &overlapped, 0 );
        }while( overlapped != NULL );
    }
}

#ifdef __cplusplus
};
#endif

#endif // __DzIoWin_h__
