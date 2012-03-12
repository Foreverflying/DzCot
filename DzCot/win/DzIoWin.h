/********************************************************************
    created:    2010/02/11 22:03
    file:       DzIoWin.h
    author:     Foreverflying
    purpose:    
********************************************************************/

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

inline DzFd* Socket( DzHost* host, int domain, int type, int protocol )
{
    SOCKET fd;
    DzFd* dzFd;

    fd = socket( domain, type, protocol );
    if( fd == INVALID_SOCKET ){
        __DbgSetLastErr( host, WSAGetLastError() );
        return NULL;
    }
    CreateIoCompletionPort( (HANDLE)fd, host->os.iocp, (ULONG_PTR)NULL, 0 );
    dzFd = CreateDzFd( host );
    dzFd->s = fd;
    return dzFd;
}

inline int CloseSocket( DzHost* host, DzFd* dzFd )
{
    int ret;

    ret = closesocket( dzFd->s );
    dzFd->err = WSAECONNRESET;
    CloseDzFd( host, dzFd );
    return ret;
}

inline int GetSockOpt( DzFd* dzFd, int level, int name, void* option, int* len )
{
    return getsockopt( dzFd->s, level, name, (char*)option, len );
}

inline int SetSockOpt( DzFd* dzFd, int level, int name, const void* option, int len )
{
    return setsockopt( dzFd->s, level, name, (const char*)option, len );
}

inline int GetSockName( DzFd* dzFd, struct sockaddr* addr, int* addrLen )
{
    return getsockname( dzFd->s, addr, addrLen );
}

inline int Bind( DzFd* dzFd, struct sockaddr* addr, int addrLen )
{
    return bind( dzFd->s, addr, addrLen );
}

inline int Listen( DzFd* dzFd, int backlog )
{
    return listen( dzFd->s, backlog );
}

inline int Shutdown( DzFd* dzFd, int how )
{
    return shutdown( dzFd->s, how );
}

inline int TryConnectDatagram( SOCKET fd, struct sockaddr* addr, int addrLen )
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

inline int Connect( DzHost* host, DzFd* dzFd, struct sockaddr* addr, int addrLen )
{
    DWORD bytes;
    DzIoHelper helper;
    struct sockaddr tmpAddr;
    DWORD flag;
    int err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzIoHelper* helperPtr;

    ZeroMemory( &helper.overlapped, sizeof( helper.overlapped ) );
    ZeroMemory( &tmpAddr, sizeof( struct sockaddr ) );
    tmpAddr.sa_family = addr->sa_family;
    bind( dzFd->s, &tmpAddr, (int)sizeof(struct sockaddr) );
    if( !host->os._ConnectEx( dzFd->s, addr, addrLen, NULL, 0, &bytes, &helper.overlapped ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            if( err == WSAEINVAL ){
                if( TryConnectDatagram( dzFd->s, addr, addrLen ) == 0 ){
                    return 0;
                }
            }
            __DbgSetLastErr( host, err );
            return -1;
        }
        CloneDzFd( dzFd );
        WaitEasyEvt( host, &helper.easyEvt );
        if( dzFd->err ){
            __DbgSetLastErr( host, err );
            CloseDzFd( host, dzFd );
            return -1;
        }
        CloseDzFd( host, dzFd );
    }else{
        GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &helper.overlapped ){
            host->currPri = CP_FIRST;
            do{
                if( !key ){
                    helperPtr = MEMBER_BASE( overlapped, DzIoHelper, overlapped );
                    NotifyEasyEvt( host, &helperPtr->easyEvt );
                }
                GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &helper.overlapped );
        }
    }
    ret = WSAGetOverlappedResult(
        dzFd->s,
        &helper.overlapped,
        &bytes,
        FALSE,
        &flag
        );
    if( !ret ){
        __DbgSetLastErr( host, WSAGetLastError() );
        return -1;
    }
    setsockopt( dzFd->s, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0 );
    return 0;
}

inline DzFd* Accept( DzHost* host, DzFd* dzFd, struct sockaddr* addr, int* addrLen )
{
    SOCKET s;
    char buf[64];
    DWORD bytes;
    DzIoHelper helper;
    DWORD flag;
    DWORD err;
    BOOL ret;
    struct sockaddr* lAddr;
    struct sockaddr* rAddr;
    int lAddrLen;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzIoHelper* helperPtr;

    s = socket( AF_INET, SOCK_STREAM, 0 );
    if( s == INVALID_SOCKET ){
        return NULL;
    }
    ZeroMemory( &helper.overlapped, sizeof( helper.overlapped ) );
    if( !host->os._AcceptEx( dzFd->s, s, buf, 0, 32, 32, &bytes, &helper.overlapped ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            closesocket( s );
            __DbgSetLastErr( host, err );
            return NULL;
        }
        CloneDzFd( dzFd );
        WaitEasyEvt( host, &helper.easyEvt );
        if( dzFd->err ){
            __DbgSetLastErr( host, err );
            CloseDzFd( host, dzFd );
            return NULL;
        }
        CloseDzFd( host, dzFd );
    }else{
        GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &helper.overlapped ){
            host->currPri = CP_FIRST;
            do{
                if( !key ){
                    helperPtr = MEMBER_BASE( overlapped, DzIoHelper, overlapped );
                    NotifyEasyEvt( host, &helperPtr->easyEvt );
                }
                GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &helper.overlapped );
        }
    }
    ret = WSAGetOverlappedResult(
        dzFd->s,
        &helper.overlapped,
        &bytes,
        FALSE,
        &flag
        );
    if( !ret ){
        closesocket( s );
        __DbgSetLastErr( host, WSAGetLastError() );
        return NULL;
    }
    key = (ULONG_PTR)dzFd->s;
    setsockopt( s, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&key, sizeof( key ) );
    CreateIoCompletionPort( (HANDLE)s, host->os.iocp, (ULONG_PTR)NULL, 0 );
    if( addr ){
        host->os._GetAcceptExSockAddrs( buf, bytes, 32, 32, &lAddr, &lAddrLen, &rAddr, addrLen );
        memcpy( addr, rAddr, *addrLen );
    }
    dzFd = CreateDzFd( host );
    dzFd->s = s;
    return dzFd;
}

inline int SendEx( DzHost* host, DzFd* dzFd, DzBuf* bufs, u_int bufCount, int flags )
{
    DWORD bytes;
    DzIoHelper helper;
    DWORD tmpFlag;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzIoHelper* helperPtr;

    ZeroMemory( &helper.overlapped, sizeof( helper.overlapped ) );
    if( WSASend( dzFd->s, (WSABUF*)bufs, bufCount, &bytes, flags, &helper.overlapped, NULL ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            __DbgSetLastErr( host, err );
            return -1;
        }
        CloneDzFd( dzFd );
        WaitEasyEvt( host, &helper.easyEvt );
        if( dzFd->err ){
            __DbgSetLastErr( host, err );
            CloseDzFd( host, dzFd );
            return -1;
        }
        CloseDzFd( host, dzFd );
    }else{
        GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &helper.overlapped ){
            host->currPri = CP_FIRST;
            do{
                if( !key ){
                    helperPtr = MEMBER_BASE( overlapped, DzIoHelper, overlapped );
                    NotifyEasyEvt( host, &helperPtr->easyEvt );
                }
                GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &helper.overlapped );
        }
    }
    ret = WSAGetOverlappedResult(
        dzFd->s,
        &helper.overlapped,
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

inline int RecvEx( DzHost* host, DzFd* dzFd, DzBuf* bufs, u_int bufCount, int flags )
{
    DWORD bytes;
    DzIoHelper helper;
    DWORD tmpFlag;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzIoHelper* helperPtr;

    ZeroMemory( &helper.overlapped, sizeof( helper.overlapped ) );
    tmpFlag = (DWORD)flags;
    if( WSARecv( dzFd->s, (WSABUF*)bufs, bufCount, &bytes, &tmpFlag, &helper.overlapped, NULL ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            __DbgSetLastErr( host, err );
            return -1;
        }
        CloneDzFd( dzFd );
        WaitEasyEvt( host, &helper.easyEvt );
        if( dzFd->err ){
            __DbgSetLastErr( host, err );
            CloseDzFd( host, dzFd );
            return -1;
        }
        CloseDzFd( host, dzFd );
    }else{
        GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &helper.overlapped ){
            host->currPri = CP_FIRST;
            do{
                if( !key ){
                    helperPtr = MEMBER_BASE( overlapped, DzIoHelper, overlapped );
                    NotifyEasyEvt( host, &helperPtr->easyEvt );
                }
                GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &helper.overlapped );
        }
    }
    ret = WSAGetOverlappedResult(
        dzFd->s,
        &helper.overlapped,
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

inline int Send( DzHost* host, DzFd* dzFd, const void* buf, u_int len, int flags )
{
    DzBuf tmpBuf;
    tmpBuf.len = (unsigned long)len;
    tmpBuf.buf = (void*)buf;

    return SendEx( host, dzFd, &tmpBuf, 1, flags );
}

inline int Recv( DzHost* host, DzFd* dzFd, void* buf, u_int len, int flags )
{
    DzBuf tmpBuf;
    tmpBuf.len = (unsigned long)len;
    tmpBuf.buf = buf;

    return RecvEx( host, dzFd, &tmpBuf, 1, flags );
}

inline int SendToEx(
    DzHost*                 host,
    DzFd*                   dzFd,
    DzBuf*                  bufs,
    u_int                   bufCount,
    int                     flags,
    const struct sockaddr*  to,
    int                     tolen
    )
{
    DWORD bytes;
    DzIoHelper helper;
    DWORD tmpFlag;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzIoHelper* helperPtr;

    ZeroMemory( &helper.overlapped, sizeof( helper.overlapped ) );
    if( WSASendTo( dzFd->s, (WSABUF*)bufs, bufCount, &bytes, flags, to, tolen, &helper.overlapped, NULL ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            __DbgSetLastErr( host, err );
            return -1;
        }
        CloneDzFd( dzFd );
        WaitEasyEvt( host, &helper.easyEvt );
        if( dzFd->err ){
            __DbgSetLastErr( host, err );
            CloseDzFd( host, dzFd );
            return -1;
        }
        CloseDzFd( host, dzFd );
    }else{
        GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &helper.overlapped ){
            host->currPri = CP_FIRST;
            do{
                if( !key ){
                    helperPtr = MEMBER_BASE( overlapped, DzIoHelper, overlapped );
                    NotifyEasyEvt( host, &helperPtr->easyEvt );
                }
                GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &helper.overlapped );
        }
    }
    ret = WSAGetOverlappedResult(
        dzFd->s,
        &helper.overlapped,
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
    DzFd*                   dzFd,
    DzBuf*                  bufs,
    u_int                   bufCount,
    int                     flags,
    struct sockaddr*        from,
    int*                    fromlen
    )
{
    DWORD bytes;
    DzIoHelper helper;
    DWORD tmpFlag;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzIoHelper* helperPtr;

    ZeroMemory( &helper.overlapped, sizeof( helper.overlapped ) );
    tmpFlag = (DWORD)flags;
    if( WSARecvFrom( dzFd->s, (WSABUF*)bufs, bufCount, &bytes, &tmpFlag, from, fromlen, &helper.overlapped, NULL ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            __DbgSetLastErr( host, err );
            return -1;
        }
        CloneDzFd( dzFd );
        WaitEasyEvt( host, &helper.easyEvt );
        if( dzFd->err ){
            __DbgSetLastErr( host, err );
            CloseDzFd( host, dzFd );
            return -1;
        }
        CloseDzFd( host, dzFd );
    }else{
        GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &helper.overlapped ){
            host->currPri = CP_FIRST;
            do{
                if( !key ){
                    helperPtr = MEMBER_BASE( overlapped, DzIoHelper, overlapped );
                    NotifyEasyEvt( host, &helperPtr->easyEvt );
                }
                GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &helper.overlapped );
        }
    }
    ret = WSAGetOverlappedResult(
        dzFd->s,
        &helper.overlapped,
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
    DzFd*                   dzFd,
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

    return SendToEx( host, dzFd, &tmpBuf, 1, flags, to, tolen );
}

inline int RecvFrom(
    DzHost*                 host,
    DzFd*                   dzFd,
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

    return RecvFromEx( host, dzFd, &tmpBuf, 1, flags, from, fromlen );
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

inline DzFd* GetFd( DzHost* host, HANDLE file, int flags )
{
    DzFd* dzFd;

    if( file == INVALID_HANDLE_VALUE ){
        return NULL;
    }
    CreateIoCompletionPort( file, host->os.iocp, (ULONG_PTR)NULL, 0 );
    if( flags & DZ_O_APPEND && GetFileType( file ) == FILE_TYPE_DISK ){
        SetFilePointer( file, 0, 0, FILE_END );
    }
    dzFd = CreateDzFd( host );
    dzFd->fd = file;
    return dzFd;
}

inline DzFd* OpenA( DzHost* host, const char* fileName, int flags )
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

inline DzFd* OpenW( DzHost* host, const wchar_t* fileName, int flags )
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

inline int Close( DzHost* host, DzFd* dzFd )
{
    BOOL ret;

    ret = CloseHandle( dzFd->fd );
    dzFd->err = WSAECONNRESET;
    CloseDzFd( host, dzFd );
    return ret;
    return ret ? 0 : -1;
}

inline ssize_t Read( DzHost* host, DzFd* dzFd, void* buf, size_t count )
{
    DWORD bytes;
    BOOL isFile;
    DzIoHelper helper;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzIoHelper* helperPtr;

    isFile = GetFileType( dzFd->fd ) == FILE_TYPE_DISK;
    ZeroMemory( &helper.overlapped, sizeof( helper.overlapped ) );
    if( isFile ){
        helper.overlapped.OffsetHigh = 0;
        helper.overlapped.Offset = SetFilePointer(
            dzFd->fd,
            0,
            (PLONG)&helper.overlapped.OffsetHigh,
            FILE_CURRENT
            );
    }
    if( !ReadFile( dzFd->fd, buf, (DWORD)count, &bytes, &helper.overlapped ) ){
        err = GetLastError();
        if( err != ERROR_IO_PENDING ){
            if( err == ERROR_HANDLE_EOF ){
                return 0;
            }else{
                __DbgSetLastErr( host, (int)err );
                return -1;
            }
        }
        CloneDzFd( dzFd );
        WaitEasyEvt( host, &helper.easyEvt );
        if( dzFd->err ){
            __DbgSetLastErr( host, err );
            CloseDzFd( host, dzFd );
            return -1;
        }
        CloseDzFd( host, dzFd );
    }else{
        GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &helper.overlapped ){
            host->currPri = CP_FIRST;
            do{
                if( !key ){
                    helperPtr = MEMBER_BASE( overlapped, DzIoHelper, overlapped );
                    NotifyEasyEvt( host, &helperPtr->easyEvt );
                }
                GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &helper.overlapped );
        }
    }
    ret = GetOverlappedResult(
        dzFd->fd,
        &helper.overlapped,
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
        SetFilePointer( dzFd->fd, (LONG)bytes, 0, FILE_CURRENT );
    }
    return bytes;
}

inline ssize_t Write( DzHost* host, DzFd* dzFd, const void* buf, size_t count )
{
    DWORD bytes;
    BOOL isFile;
    DzIoHelper helper;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzIoHelper* helperPtr;

    isFile = GetFileType( dzFd->fd ) == FILE_TYPE_DISK;
    ZeroMemory( &helper.overlapped, sizeof( helper.overlapped ) );
    if( isFile ){
        helper.overlapped.OffsetHigh = 0;
        helper.overlapped.Offset = SetFilePointer(
            dzFd->fd,
            0,
            (PLONG)&helper.overlapped.OffsetHigh,
            FILE_CURRENT
            );
    }
    if( !WriteFile( dzFd->fd, buf, (DWORD)count, &bytes, &helper.overlapped )  ){
        err = GetLastError();
        if( err != ERROR_IO_PENDING ){
            if( err == ERROR_HANDLE_EOF ){
                return 0;
            }else{
                __DbgSetLastErr( host, (int)err );
                return -1;
            }
        }
        CloneDzFd( dzFd );
        WaitEasyEvt( host, &helper.easyEvt );
        if( dzFd->err ){
            __DbgSetLastErr( host, err );
            CloseDzFd( host, dzFd );
            return -1;
        }
        CloseDzFd( host, dzFd );
    }else{
        GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
        if( overlapped != &helper.overlapped ){
            host->currPri = CP_FIRST;
            do{
                if( !key ){
                    helperPtr = MEMBER_BASE( overlapped, DzIoHelper, overlapped );
                    NotifyEasyEvt( host, &helperPtr->easyEvt );
                }
                GetQueuedCompletionStatus( host->os.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != &helper.overlapped );
        }
    }
    ret = GetOverlappedResult(
        dzFd->fd,
        &helper.overlapped,
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
        SetFilePointer( dzFd->fd, (LONG)bytes, 0, FILE_CURRENT );
    }
    return bytes;
}

inline size_t Seek( DzFd* dzFd, ssize_t offset, int whence )
{
    size_t ret;

#if defined( _X86_ )
    ret = (size_t)SetFilePointer( dzFd->fd, (long)offset, NULL, whence );
#elif defined( _M_AMD64 )
    if( !SetFilePointerEx( dzFd->fd, *(LARGE_INTEGER*)&offset, (LARGE_INTEGER*)&ret, whence ) ){
        return -1;
    }
#endif

    return ret;
}

inline size_t FileSize( DzFd* dzFd )
{
    size_t ret;

#if defined( _X86_ )
    ret = GetFileSize( dzFd->fd, NULL );
#elif defined( _M_AMD64 )
    if( !GetFileSizeEx( dzFd->fd, (LARGE_INTEGER*)&ret ) ){
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
    DzIoHelper* helper;

    GetQueuedCompletionStatus( host->os.iocp, &n, &key, &overlapped, (DWORD)timeout );
    AtomAndInt( host->rmtCheckSignPtr, ~RMT_CHECK_SLEEP_SIGN );
    if( overlapped != NULL ){
        do{
            if( !key ){
                helper = MEMBER_BASE( overlapped, DzIoHelper, overlapped );
                NotifyEasyEvt( host, &helper->easyEvt );
            }
            GetQueuedCompletionStatus( host->os.iocp, &n, &key, &overlapped, 0 );
        }while( overlapped != NULL );
    }
}

#ifdef __cplusplus
};
#endif

#endif // __DzIoWin_h__
