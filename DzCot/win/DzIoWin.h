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

void __stdcall GetNameInfoEntryA( intptr_t context );
void __stdcall GetNameInfoEntryW( intptr_t context );
void __stdcall GetAddrInfoEntryA( intptr_t context );
void __stdcall GetAddrInfoEntryW( intptr_t context );

inline int Socket( DzHost* host, int domain, int type, int protocol )
{
    SOCKET fd;
    DzFd* dzFd;

    fd = socket( domain, type, protocol );
    if( fd == INVALID_SOCKET ){
        __DbgSetLastErr( host, WSAGetLastError() );
        return -1;
    }
    CreateIoCompletionPort( (HANDLE)fd, host->os.iocp, (ULONG_PTR)NULL, 0 );
    dzFd = CreateDzFd( host );
    dzFd->s = fd;
    return (int)( (intptr_t)dzFd - host->handleBase );
}

inline int CloseSocket( DzHost* host, int hFd )
{
    DzFd* dzFd = (DzFd*)( host->handleBase + hFd );
    int ret;

    ret = closesocket( dzFd->s );
    dzFd->err = WSAECONNRESET;
    CloseDzFd( host, dzFd );
    return ret;
}

inline int GetSockOpt( DzHost* host, int hFd, int level, int name, void* option, int* len )
{
    DzFd* dzFd = (DzFd*)( host->handleBase + hFd );
    return getsockopt( dzFd->s, level, name, (char*)option, len );
}

inline int SetSockOpt( DzHost* host, int hFd, int level, int name, const void* option, int len )
{
    DzFd* dzFd = (DzFd*)( host->handleBase + hFd );
    return setsockopt( dzFd->s, level, name, (const char*)option, len );
}

inline int GetSockName( DzHost* host, int hFd, struct sockaddr* addr, int* addrLen )
{
    DzFd* dzFd = (DzFd*)( host->handleBase + hFd );
    return getsockname( dzFd->s, addr, addrLen );
}

inline int GetPeerName( DzHost* host, int hFd, struct sockaddr* addr, int* addrLen )
{
    DzFd* dzFd = (DzFd*)( host->handleBase + hFd );
    return getpeername( dzFd->s, addr, addrLen );
}

inline int Bind( DzHost* host, int hFd, const struct sockaddr* addr, int addrLen )
{
    DzFd* dzFd = (DzFd*)( host->handleBase + hFd );
    return bind( dzFd->s, addr, addrLen );
}

inline int Listen( DzHost* host, int hFd, int backlog )
{
    DzFd* dzFd = (DzFd*)( host->handleBase + hFd );
    return listen( dzFd->s, backlog );
}

inline int Shutdown( DzHost* host, int hFd, int how )
{
    DzFd* dzFd = (DzFd*)( host->handleBase + hFd );
    return shutdown( dzFd->s, how );
}

inline int TryConnectDatagram( SOCKET fd, const struct sockaddr* addr, int addrLen )
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

inline int Connect( DzHost* host, int hFd, const struct sockaddr* addr, int addrLen )
{
    DzFd* dzFd;
    DWORD bytes;
    DzIoHelper helper;
    struct sockaddr_storage tmpAddr;
    DWORD flag;
    int err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzIoHelper* helperPtr;

    dzFd = (DzFd*)( host->handleBase + hFd );
    ZeroMemory( &helper.overlapped, sizeof( helper.overlapped ) );
    ZeroMemory( &tmpAddr, addrLen );
    tmpAddr.ss_family = addr->sa_family;
    bind( dzFd->s, (struct sockaddr*)&tmpAddr, addrLen );
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
            __DbgSetLastErr( host, dzFd->err );
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

inline int Accept( DzHost* host, int hFd, struct sockaddr* addr, int* addrLen )
{
    DzFd* dzFd;
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

    dzFd = (DzFd*)( host->handleBase + hFd );
    s = socket( AF_INET, SOCK_STREAM, 0 );
    if( s == INVALID_SOCKET ){
        return -1;
    }
    ZeroMemory( &helper.overlapped, sizeof( helper.overlapped ) );
    if( !host->os._AcceptEx( dzFd->s, s, buf, 0, 32, 32, &bytes, &helper.overlapped ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            closesocket( s );
            __DbgSetLastErr( host, err );
            return -1;
        }
        CloneDzFd( dzFd );
        WaitEasyEvt( host, &helper.easyEvt );
        if( dzFd->err ){
            __DbgSetLastErr( host, dzFd->err );
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
        closesocket( s );
        __DbgSetLastErr( host, WSAGetLastError() );
        return -1;
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
    return (int)( (intptr_t)dzFd - host->handleBase );
}

inline int SendEx( DzHost* host, int hFd, DzBuf* bufs, u_int bufCount, int flags )
{
    DzFd* dzFd;
    DWORD bytes;
    DzIoHelper helper;
    DWORD tmpFlag;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzIoHelper* helperPtr;

    dzFd = (DzFd*)( host->handleBase + hFd );
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
            __DbgSetLastErr( host, dzFd->err );
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

inline int RecvEx( DzHost* host, int hFd, DzBuf* bufs, u_int bufCount, int flags )
{
    DzFd* dzFd;
    DWORD bytes;
    DzIoHelper helper;
    DWORD tmpFlag;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzIoHelper* helperPtr;

    dzFd = (DzFd*)( host->handleBase + hFd );
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
            __DbgSetLastErr( host, dzFd->err );
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

inline int Send( DzHost* host, int hFd, const void* buf, u_int len, int flags )
{
    DzBuf tmpBuf;
    tmpBuf.len = (unsigned long)len;
    tmpBuf.buf = (void*)buf;

    return SendEx( host, hFd, &tmpBuf, 1, flags );
}

inline int Recv( DzHost* host, int hFd, void* buf, u_int len, int flags )
{
    DzBuf tmpBuf;
    tmpBuf.len = (unsigned long)len;
    tmpBuf.buf = buf;

    return RecvEx( host, hFd, &tmpBuf, 1, flags );
}

inline int SendToEx(
    DzHost*                 host,
    int                     hFd,
    DzBuf*                  bufs,
    u_int                   bufCount,
    int                     flags,
    const struct sockaddr*  to,
    int                     tolen
    )
{
    DzFd* dzFd;
    DWORD bytes;
    DzIoHelper helper;
    DWORD tmpFlag;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzIoHelper* helperPtr;

    dzFd = (DzFd*)( host->handleBase + hFd );
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
            __DbgSetLastErr( host, dzFd->err );
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
    int                     hFd,
    DzBuf*                  bufs,
    u_int                   bufCount,
    int                     flags,
    struct sockaddr*        from,
    int*                    fromlen
    )
{
    DzFd* dzFd;
    DWORD bytes;
    DzIoHelper helper;
    DWORD tmpFlag;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzIoHelper* helperPtr;

    dzFd = (DzFd*)( host->handleBase + hFd );
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
            __DbgSetLastErr( host, dzFd->err );
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
    int                     hFd,
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

    return SendToEx( host, hFd, &tmpBuf, 1, flags, to, tolen );
}

inline int RecvFrom(
    DzHost*                 host,
    int                     hFd,
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

    return RecvFromEx( host, hFd, &tmpBuf, 1, flags, from, fromlen );
}

inline int DGetNameInfoA(
    DzHost*                 dzHost,
    const struct sockaddr*  sa,
    int                     salen,
    char*                   host,
    size_t                  hostlen,
    char*                   serv,
    size_t                  servlen,
    int                     flags
    )
{
    int ret;
    DzLNode* node = AllocLNode( dzHost );
    node->d1 = (intptr_t)sa;
    node->d2 = (intptr_t)salen;
    node->d3 = (intptr_t)host;
    node->d4 = (intptr_t)hostlen;
    node->d5 = (intptr_t)serv;
    node->d6 = (intptr_t)servlen;
    node->d7 = (intptr_t)flags;
    node->d8 = (intptr_t)&ret;
    RunWorker( dzHost, GetNameInfoEntryA, (intptr_t)node );
    FreeLNode( dzHost, node );
    return ret;
}

inline int DGetAddrInfoA(
    DzHost*                 host,
    const char*             node,
    const char*             service,
    const struct addrinfo*  hints,
    struct addrinfo**       res
    )
{
    int ret;
    DzLNode* param = AllocLNode( host );
    param->d1 = (intptr_t)node;
    param->d2 = (intptr_t)service;
    param->d3 = (intptr_t)hints;
    param->d4 = (intptr_t)res;
    param->d8 = (intptr_t)&ret;
    RunWorker( host, GetAddrInfoEntryA, (intptr_t)param );
    FreeLNode( host, param );
    return ret;
}

inline void DFreeAddrInfoA( struct addrinfo *res )
{
    FreeAddrInfoA( res );
}

inline int DInetPtonA( int af, const char* src, void* dst )
{
    return InetPtonA( af, src, dst );
}

inline const char* DInetNtopA( int af, const void* src, char* dst, int size )
{
    return InetNtopA( af, (PVOID)src, dst, size );
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
    DzFd* dzFd;

    if( file == INVALID_HANDLE_VALUE ){
        return -1;
    }
    CreateIoCompletionPort( file, host->os.iocp, (ULONG_PTR)NULL, 0 );
    if( flags & DZ_O_APPEND && GetFileType( file ) == FILE_TYPE_DISK ){
        SetFilePointer( file, 0, 0, FILE_END );
    }
    dzFd = CreateDzFd( host );
    dzFd->fd = file;
    return (int)( (intptr_t)dzFd - host->handleBase );
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

inline int Close( DzHost* host, int hFd )
{
    DzFd* dzFd = (DzFd*)( host->handleBase + hFd );
    BOOL ret;

    ret = CloseHandle( dzFd->fd );
    dzFd->err = WSAECONNRESET;
    CloseDzFd( host, dzFd );
    return ret ? 0 : -1;
}

inline ssize_t Read( DzHost* host, int hFd, void* buf, size_t count )
{
    DzFd* dzFd;
    DWORD bytes;
    BOOL isFile;
    DzIoHelper helper;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzIoHelper* helperPtr;

    dzFd = (DzFd*)( host->handleBase + hFd );
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
            __DbgSetLastErr( host, dzFd->err );
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

inline ssize_t Write( DzHost* host, int hFd, const void* buf, size_t count )
{
    DzFd* dzFd;
    DWORD bytes;
    BOOL isFile;
    DzIoHelper helper;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzIoHelper* helperPtr;

    dzFd = (DzFd*)( host->handleBase + hFd );
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
            __DbgSetLastErr( host, dzFd->err );
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

inline size_t Seek( DzHost* host, int hFd, ssize_t offset, int whence )
{
    DzFd* dzFd = (DzFd*)( host->handleBase + hFd );
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

inline size_t FileSize( DzHost* host, int hFd )
{
    DzFd* dzFd = (DzFd*)( host->handleBase + hFd );
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

inline int DGetNameInfoW(
    DzHost*                 dzHost,
    const struct sockaddr*  sa,
    int                     salen,
    wchar_t*                host,
    size_t                  hostlen,
    wchar_t*                serv,
    size_t                  servlen,
    int                     flags
    )
{
    int ret;
    DzLNode* node = AllocLNode( dzHost );
    node->d1 = (intptr_t)sa;
    node->d2 = (intptr_t)salen;
    node->d3 = (intptr_t)host;
    node->d4 = (intptr_t)hostlen;
    node->d5 = (intptr_t)serv;
    node->d6 = (intptr_t)servlen;
    node->d7 = (intptr_t)flags;
    node->d8 = (intptr_t)&ret;
    RunWorker( dzHost, GetNameInfoEntryW, (intptr_t)node );
    FreeLNode( dzHost, node );
    return ret;
}

inline int DGetAddrInfoW(
    DzHost*                 host,
    const wchar_t*          node,
    const wchar_t*          service,
    const struct addrinfoW* hints,
    struct addrinfoW**      res
    )
{
    int ret;
    DzLNode* param = AllocLNode( host );
    param->d1 = (intptr_t)node;
    param->d2 = (intptr_t)service;
    param->d3 = (intptr_t)hints;
    param->d4 = (intptr_t)res;
    param->d8 = (intptr_t)&ret;
    RunWorker( host, GetAddrInfoEntryW, (intptr_t)param );
    FreeLNode( host, param );
    return ret;
}

inline void DFreeAddrInfoW( struct addrinfoW *res )
{
    FreeAddrInfoW( res );
}

inline int DInetPtonW( int af, const wchar_t* src, void* dst )
{
    return InetPtonW( af, src, dst );
}

inline const wchar_t* DInetNtopW( int af, const void* src, wchar_t* dst, int size )
{
    return InetNtopW( af, (PVOID)src, dst, size );
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

inline void BlockAndDispatchIoNoRmtCheck( DzHost* host, int timeout )
{
    ULONG_PTR key;
    DWORD n;
    OVERLAPPED* overlapped;
    DzIoHelper* helper;

    GetQueuedCompletionStatus( host->os.iocp, &n, &key, &overlapped, (DWORD)timeout );
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
