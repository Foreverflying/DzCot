/********************************************************************
    created:    2010/02/11 22:03
    file:       DzIoWin.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzIoWin_h__
#define __DzIoWin_h__

#include "../DzConstant.h"
#include "../DzStructs.h"
#include "../DzStructsOs.h"
#include "../DzBaseOs.h"
#include "../DzBase.h"
#include "../DzResourceMgr.h"
#include "../DzCoreOs.h"
#include "../DzSynObj.h"
#include "../../DzcotData/DzcotData.h"
#include <MSWSock.h>

#ifdef __cplusplus
extern "C"{
#endif

BOOL SockStartup();
BOOL SockCleanup();

inline DzAsynIo* CreateAsynIo( DzHost* host )
{
    DzAsynIo* asynIo;

    if( !host->asynIoPool ){
        if( !AllocAsynIoPool( host ) ){
            return NULL;
        }
    }
    asynIo = MEMBER_BASE( host->asynIoPool, DzAsynIo, lItr );
    host->asynIoPool = host->asynIoPool->next;
    asynIo->ref = 1;
    return asynIo;
}

inline DzAsynIo* CloneAsynIo( DzAsynIo* asynIo )
{
    asynIo->ref++;
    return asynIo;
}

inline void CloseAsynIo( DzHost* host, DzAsynIo* asynIo )
{
    asynIo->ref--;
    if( asynIo->ref == 0 ){
        asynIo->lItr.next = host->asynIoPool;
        host->asynIoPool = &asynIo->lItr;
    }
}

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
    BOOL result = TRUE;
    DWORD bytes;
    DzAsynIo* asynIo;
    struct sockaddr temp;
    DWORD flag;
    int err;

    asynIo = CreateAsynIo( host );
    asynIo->fd = fd;
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

inline int Accept( DzHost* host, int fd, struct sockaddr* addr, int* addrLen )
{
    SOCKET s;
    char buf[64];
    DWORD bytes;
    DzAsynIo* asynIo;
    DWORD err;
    DWORD flag;
    BOOL result;
    struct sockaddr* lAddr;
    struct sockaddr* rAddr;
    int lAddrLen;

    s = socket( AF_INET, SOCK_STREAM, 0 );
    if( !s ){
        return -1;
    }
    asynIo = CreateAsynIo( host );
    asynIo->fd = fd;
    InitFastEvt( &asynIo->fastEvt );
    ZeroMemory( &asynIo->overlapped, sizeof( asynIo->overlapped ) );
    if( !_AcceptEx( (SOCKET)fd, s, buf, 0, 32, 32, &bytes, &asynIo->overlapped ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            closesocket( s );
            s = -1;
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
            s = -1;
            goto endproc;
        }
    }else{
        //complete directly
        asynIo->fd = -1;
    }
    CreateIoCompletionPort( (HANDLE)s, host->osStruct.iocp, (ULONG_PTR)NULL, 0 );
    if( addr ){
        _GetAcceptExSockAddrs( buf, bytes, 32, 32, &lAddr, &lAddrLen, &rAddr, addrLen );
        memcpy( addr, rAddr, *addrLen );
    }
    if( asynIo->fd == -1 ){
        return (int)s;
    }
endproc:
    CloseAsynIo( host, asynIo );
    return (int)s;
}

inline int SendEx( DzHost* host, int fd, DzBuf* bufs, int bufCount, int flags )
{
    DWORD bytes;
    DzAsynIo* asynIo;
    DWORD err;
    DWORD tmpFlag;
    BOOL result;

    asynIo = CreateAsynIo( host );
    asynIo->fd = fd;
    InitFastEvt( &asynIo->fastEvt );
    ZeroMemory( &asynIo->overlapped, sizeof( asynIo->overlapped ) );
    if( WSASend( (SOCKET)fd, (WSABUF*)bufs, bufCount, &bytes, flags, &asynIo->overlapped, NULL ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            SetLastErr( host, (int)err );
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
    SetLastErr( host, 0 );
endproc:
    CloseAsynIo( host, asynIo );
    return bytes;
}

inline int RecvEx( DzHost* host, int fd, DzBuf* bufs, int bufCount, int flags )
{
    DWORD bytes;
    DzAsynIo* asynIo;
    DWORD tmpFlag; 
    DWORD err;
    BOOL result;

    asynIo = CreateAsynIo( host );
    asynIo->fd = fd;
    InitFastEvt( &asynIo->fastEvt );
    ZeroMemory( &asynIo->overlapped, sizeof( asynIo->overlapped ) );
    tmpFlag = (DWORD)flags;
    if( WSARecv( (SOCKET)fd, (WSABUF*)bufs, bufCount, &bytes, &tmpFlag, &asynIo->overlapped, NULL ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            SetLastErr( host, (int)err );
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
    SetLastErr( host, 0 );
endproc:
    CloseAsynIo( host, asynIo );
    return bytes;
}

inline int SendToEx( DzHost* host, int fd, DzBuf* bufs, int bufCount, int flags, const struct sockaddr *to, int tolen )
{
    DWORD bytes;
    DzAsynIo* asynIo;
    DWORD err;
    DWORD tmpFlag;
    BOOL result;

    asynIo = CreateAsynIo( host );
    asynIo->fd = fd;
    InitFastEvt( &asynIo->fastEvt );
    ZeroMemory( &asynIo->overlapped, sizeof( asynIo->overlapped ) );
    if( WSASendTo( (SOCKET)fd, (WSABUF*)bufs, bufCount, &bytes, flags, to, tolen, &asynIo->overlapped, NULL ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            SetLastErr( host, (int)err );
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
    SetLastErr( host, 0 );
endproc:
    CloseAsynIo( host, asynIo );
    return bytes;
}

inline int RecvFromEx( DzHost* host, int fd, DzBuf* bufs, int bufCount, int flags, struct sockaddr *from, int *fromlen )
{
    DWORD bytes;
    DzAsynIo* asynIo;
    DWORD tmpFlag; 
    DWORD err;
    BOOL result;

    asynIo = CreateAsynIo( host );
    asynIo->fd = fd;
    InitFastEvt( &asynIo->fastEvt );
    ZeroMemory( &asynIo->overlapped, sizeof( asynIo->overlapped ) );
    tmpFlag = (DWORD)flags;
    if( WSARecvFrom( (SOCKET)fd, (WSABUF*)bufs, bufCount, &bytes, &tmpFlag, from, fromlen, &asynIo->overlapped, NULL ) ){
        err = WSAGetLastError();
        if( err != ERROR_IO_PENDING ){
            SetLastErr( host, (int)err );
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
    SetLastErr( host, 0 );
endproc:
    CloseAsynIo( host, asynIo );
    return bytes;
    return 0;
}

inline int Send( DzHost* host, int fd, const void* buf, int len, int flags )
{
    DzBuf tmpBuf;
    tmpBuf.len = (unsigned long)len;
    tmpBuf.buf = (char*)buf;

    return SendEx( host, fd, &tmpBuf, 1, flags );
}

inline int Recv( DzHost* host, int fd, void* buf, int len, int flags )
{
    DzBuf tmpBuf;
    tmpBuf.len = (unsigned long)len;
    tmpBuf.buf = (char*)buf;

    return RecvEx( host, fd, &tmpBuf, 1, flags );
}

inline int SendTo( DzHost* host, int fd, const char *buf, int len, int flags, const struct sockaddr *to, int tolen )
{
    DzBuf tmpBuf;
    tmpBuf.len = (unsigned long)len;
    tmpBuf.buf = (char*)buf;

    return SendToEx( host, fd, &tmpBuf, 1, flags, to, tolen );
}

inline int RecvFrom( DzHost* host, int fd, char *buf, int len, int flags, struct sockaddr *from, int *fromlen )
{
    DzBuf tmpBuf;
    tmpBuf.len = (unsigned long)len;
    tmpBuf.buf = (char*)buf;

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

inline size_t Read( DzHost* host, int fd, void* buf, size_t count )
{
    DWORD bytes;
    BOOL isFile;
    DzAsynIo* asynIo;
    DWORD err;
    BOOL result;

    isFile = GetFileType( (HANDLE)fd ) == FILE_TYPE_DISK;
    asynIo = CreateAsynIo( host );
    asynIo->fd = fd;
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
    if( !ReadFile( (HANDLE)fd, buf, (DWORD)count, &bytes, &asynIo->overlapped ) ){
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

inline size_t Write( DzHost* host, int fd, const void* buf, size_t count )
{
    DWORD bytes;
    BOOL isFile;
    DzAsynIo* asynIo;
    DWORD err;
    BOOL result;

    isFile = GetFileType( (HANDLE)fd ) == FILE_TYPE_DISK;
    asynIo = CreateAsynIo( host );
    asynIo->fd = fd;
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
    if( !WriteFile( (HANDLE)fd, buf, (DWORD)count, &bytes, &asynIo->overlapped )  ){
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

inline size_t Seek( DzHost* host, int fd, size_t offset, int whence )
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

    while( host->threadCount ){
        while( NotifyMinTimers( host, (int*)&timeout ) ){
            host->currPriority = CP_FIRST;
            Schedule( host );
            if( !host->threadCount ){
                return;
            }
        }
        GetQueuedCompletionStatus( host->osStruct.iocp, &bytes, &key, &overlapped, timeout );
        if( overlapped != NULL ){
            do{
                asynIo = MEMBER_BASE( overlapped, DzAsynIo, overlapped );
                if( asynIo->fd != -1 ){
                    NotifyFastEvt( host, &asynIo->fastEvt, 0 );
                }
                CloseAsynIo( host, asynIo );
                GetQueuedCompletionStatus( host->osStruct.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != NULL );
            host->currPriority = CP_FIRST;
            Schedule( host );
        }
    }
}

#ifdef __cplusplus
};
#endif

#endif // __DzIoWin_h__
