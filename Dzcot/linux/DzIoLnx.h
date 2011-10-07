/********************************************************************
    created:    2010/02/11 22:03
    file:       DzIoLnx.h
    author:     Foreverflying
    purpose:
*********************************************************************/

#ifndef __DzIoLnx_h__
#define __DzIoLnx_h__

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

inline DzAsynIo* CreateAsynIo( DzHost* host )
{
    DzAsynIo* asynIo;

    if( !host->osStruct.asynIoPool ){
        if( !AllocAsynIoPool( host ) ){
            return NULL;
        }
    }
    asynIo = MEMBER_BASE( host->osStruct.asynIoPool, DzAsynIo, lItr );
    host->osStruct.asynIoPool = host->osStruct.asynIoPool->next;
    asynIo->sign = 0;
    asynIo->ref = 1;
    return asynIo;
}

inline void CloneAsynIo( DzAsynIo* asynIo )
{
    asynIo->ref++;
}

inline void CloseAsynIo( DzHost* host, DzAsynIo* asynIo )
{
    asynIo->ref--;
    if( !asynIo->ref ){
        asynIo->lItr.next = host->osStruct.asynIoPool;
        host->osStruct.asynIoPool = &asynIo->lItr;
    }
}

inline int Socket( DzHost* host, int domain, int type, int protocol )
{
    int fd;
    int flag;
    struct epoll_event evt;

    fd = socket( domain, type, protocol );
    if( fd >= 0 ){
        host->osStruct.fdTable[ fd ] = CreateAsynIo( host );
        evt.data.ptr = host->osStruct.fdTable[ fd ];
        evt.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLET;
        flag = fcntl( fd, F_GETFL, 0 );
        fcntl( fd, F_SETFL, flag | O_NONBLOCK );
        epoll_ctl( host->osStruct.epollFd, EPOLL_CTL_ADD, fd, &evt );
    }
    return fd;
}

inline int CloseSocket( DzHost* host, int fd )
{
    assert( fd >= 0 && fd < host->osStruct.maxFd );
    assert( host->osStruct.fdTable[ fd ] );
    assert( host->osStruct.fdTable[ fd ]->outEvt.dzThread == NULL );

    epoll_ctl( host->osStruct.epollFd, EPOLL_CTL_DEL, fd, (struct epoll_event*)1 );
    CloseAsynIo( host, host->osStruct.fdTable[ fd ] );
    host->osStruct.fdTable[ fd ] = NULL;
    return close( fd );
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
    return bind( fd, addr, addrLen );
}

inline int Listen( int fd, int backlog )
{
    return listen( fd, backlog );
}

inline int Shutdown( int fd, int how )
{
    return shutdown( fd, how );
}

inline int Connect( DzHost* host, int fd, struct sockaddr* addr, int addrLen )
{
    DzAsynIo* asynIo;
    assert( fd >= 0 && fd < host->osStruct.maxFd );
    assert( host->osStruct.fdTable[ fd ] );
    assert( host->osStruct.fdTable[ fd ]->outEvt.dzThread == NULL );

    if( connect( fd, addr, addrLen ) != 0 && errno != EINPROGRESS ){
        return -1;
    }
    asynIo = host->osStruct.fdTable[ fd ];
    CloneAsynIo( asynIo );
    WaitFastEvt( host, &asynIo->outEvt, -1 );
    CloseAsynIo( host, asynIo );
    if( asynIo->sign ){
        return -1;
    }
    NotifyFastEvt( host, &asynIo->inEvt, 0 );
    NotifyFastEvt( host, &asynIo->outEvt, 0 );
    return 0;
}

inline int Accept( DzHost* host, int fd, struct sockaddr* addr, int* addrLen )
{
    DzAsynIo* asynIo;
    int ret;
    int flag;
    struct epoll_event evt;
    assert( fd >= 0 && fd < host->osStruct.maxFd );
    assert( host->osStruct.fdTable[ fd ] );
    assert( host->osStruct.fdTable[ fd ]->outEvt.dzThread == NULL );

    asynIo = host->osStruct.fdTable[ fd ];
    CloneAsynIo( asynIo );
    while( 1 ){
        WaitFastEvt( host, &asynIo->inEvt, -1 );
        if( asynIo->sign ){
            CloseAsynIo( host, asynIo );
            return -1;
        }
        ret = accept( fd, addr, (socklen_t*)addrLen );
        if( ret >= 0 ){
            host->osStruct.fdTable[ ret ] = CreateAsynIo( host );
            NotifyFastEvt( host, &host->osStruct.fdTable[ ret ]->inEvt, 0 );
            NotifyFastEvt( host, &host->osStruct.fdTable[ ret ]->outEvt, 0 );
            evt.data.ptr = host->osStruct.fdTable[ ret ];
            evt.events = EPOLLIN | EPOLLOUT | EPOLLET;
            flag = fcntl( ret, F_GETFL, 0 );
            fcntl( ret, F_SETFL, flag | O_NONBLOCK );
            epoll_ctl( host->osStruct.epollFd, EPOLL_CTL_ADD, ret, &evt );
            NotifyFastEvt( host, &asynIo->inEvt, 0 );
            CloseAsynIo( host, asynIo );
            return ret;
        }else if( errno != EAGAIN ){
            CloseAsynIo( host, asynIo );
            return -1;
        }
    }
}
inline int SendEx( DzHost* host, int fd, DzBuf* bufs, int bufCount, int flags )
{
    return 0;
}

inline int RecvEx( DzHost* host, int fd, DzBuf* bufs, int bufCount, int flags )
{
    return 0;
}

inline int Send( DzHost* host, int fd, const void* buf, int len, int flag )
{
    DzAsynIo* asynIo;
    int ret;
    int sendLen = 0;
    int leftLen = len;
    assert( fd >= 0 && fd < host->osStruct.maxFd );
    assert( host->osStruct.fdTable[ fd ] );
    assert( host->osStruct.fdTable[ fd ]->outEvt.dzThread == NULL );

    asynIo = host->osStruct.fdTable[ fd ];
    CloneAsynIo( asynIo );
    while( 1 ){
        WaitFastEvt( host, &asynIo->outEvt, -1 );
        if( asynIo->sign ){
            CloseAsynIo( host, asynIo );
            return -1;
        }
        ret = send( fd, (char*)buf + sendLen, leftLen, flag | MSG_NOSIGNAL );
        if( ret < 0 ){
            if( errno != EAGAIN ){
                CloseAsynIo( host, asynIo );
                return -1;
            }
        }else if( ret == leftLen ){
            NotifyFastEvt( host, &asynIo->outEvt, 0 );
            CloseAsynIo( host, asynIo );
            return len;
        }else if( ret > 0 ){
            sendLen += ret;
            leftLen -= ret;
        }else{
            CloseAsynIo( host, asynIo );
            return 0;
        }
    }
}

inline int Recv( DzHost* host, int fd, void* buf, int len, int flag )
{
    DzAsynIo* asynIo;
    int ret;
    assert( fd >= 0 && fd < host->osStruct.maxFd );
    assert( host->osStruct.fdTable[ fd ] );
    assert( host->osStruct.fdTable[ fd ]->outEvt.dzThread == NULL );

    asynIo = host->osStruct.fdTable[ fd ];
    CloneAsynIo( asynIo );
    while( 1 ){
        WaitFastEvt( host, &asynIo->inEvt, -1 );
        if( asynIo->sign ){
            CloseAsynIo( host, asynIo );
            return -1;
        }
        ret = recv( fd, buf, len, flag );
        if( ret < 0 ){
            if( errno != EAGAIN ){
                CloseAsynIo( host, asynIo );
                return -1;
            }
        }else{
            if( ret == len ){
                NotifyFastEvt( host, &asynIo->inEvt, 0 );
            }
            CloseAsynIo( host, asynIo );
            return ret;
        }
    }
}

inline int SendToEx(
    DzHost*                 host,
    int                     fd,
    DzBuf*                  bufs,
    int                     bufCount,
    int                     flags,
    const struct sockaddr*  to,
    int                     tolen
    )
{
	return 0;
}

inline int RecvFromEx(
    DzHost*                 host,
    int                     fd,
    DzBuf*                  bufs,
    int                     bufCount,
    int                     flags,
    struct sockaddr*        from,
    int*                    fromlen
    )
{
    return 0;
}

inline int SendTo(
    DzHost*                 host,
    int                     fd,
    const void*             buf,
    int                     len,
    int                     flags,
    const struct sockaddr*  to,
    int                     tolen
    )
{
    return 0;
}

inline int RecvFrom(
    DzHost*                 host,
    int                     fd,
    void*                   buf,
    int                     len,
    int                     flags,
    struct sockaddr*        from,
    int*                    fromlen
    )
{
    return 0;
}

inline int OpenA( DzHost* host, const char* fileName, int flags )
{
    int fd;
    int flag;
    struct stat st;
    struct epoll_event evt;

    fd = open( fileName, flags );
    int err = errno;
    printf("errno %d\n" , err );
    if( fd >= 0 ){
        host->osStruct.fdTable[ fd ] = CreateAsynIo( host );
        evt.data.ptr = host->osStruct.fdTable[ fd ];
        evt.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLET;
        flag = fcntl( fd, F_GETFL, 0 );
        fcntl( fd, F_SETFL, flag | O_NONBLOCK );
        epoll_ctl( host->osStruct.epollFd, EPOLL_CTL_ADD, fd, &evt );
        fstat( fd, &st );
        if( st.st_mode & S_IFREG ){
            host->osStruct.fdTable[ fd ]->sign |= ASYNIO_SIGN_ISFILE;
        }
    }
    return fd;
}

inline int OpenW( DzHost* host, const wchar_t* fileName, int flags )
{
    return -1;
}

inline int Close( DzHost* host, int fd )
{
    assert( fd >= 0 && fd < host->osStruct.maxFd );
    assert( host->osStruct.fdTable[ fd ] );
    assert( host->osStruct.fdTable[ fd ]->outEvt.dzThread == NULL );

    epoll_ctl( host->osStruct.epollFd, EPOLL_CTL_DEL, fd, (struct epoll_event*)1 );
    CloseAsynIo( host, host->osStruct.fdTable[ fd ] );
    host->osStruct.fdTable[ fd ] = NULL;
    return close( fd );
}

inline size_t Read( DzHost* host, int fd, void* buf, size_t count )
{
    DzAsynIo* asynIo;
    int ret;
    int readLen = 0;
    int leftLen = count;
    BOOL isFile;
    assert( fd >= 0 && fd < host->osStruct.maxFd );
    assert( host->osStruct.fdTable[ fd ] );
    assert( host->osStruct.fdTable[ fd ]->outEvt.dzThread == NULL );

    asynIo = host->osStruct.fdTable[ fd ];
    CloneAsynIo( asynIo );
    isFile = asynIo->sign & ASYNIO_SIGN_ISFILE;
    do{
        WaitFastEvt( host, &asynIo->inEvt, -1 );
        if( asynIo->sign & ASYNIO_SIGN_ERR_MASK ){
            CloseAsynIo( host, asynIo );
            return -1;
        }
        ret = read( fd, (char*)buf + readLen, leftLen );
        if( ret < 0 ){
            if( errno != EAGAIN ){
                CloseAsynIo( host, asynIo );
                return -1;
            }
        }else{
            readLen += ret;
            leftLen -= ret;
            NotifyFastEvt( host, &asynIo->inEvt, 0 );
            if( ret == 0 || ret == leftLen ){
                CloseAsynIo( host, asynIo );
                return readLen;
            }
        }
    }while( isFile );
    CloseAsynIo( host, asynIo );
    return readLen;
}

inline size_t Write( DzHost* host, int fd, const void* buf, size_t count )
{
    DzAsynIo* asynIo;
    int ret;
    int writeLen = 0;
    int leftLen = count;
    assert( fd >= 0 && fd < host->osStruct.maxFd );
    assert( host->osStruct.fdTable[ fd ] );
    assert( host->osStruct.fdTable[ fd ]->outEvt.dzThread == NULL );

    asynIo = host->osStruct.fdTable[ fd ];
    while( 1 ){
        WaitFastEvt( host, &asynIo->outEvt, -1 );
        if( asynIo->sign & ASYNIO_SIGN_ERR_MASK ){
            CloseAsynIo( host, asynIo );
            return -1;
        }
        ret = write( fd, (char*)buf + writeLen, leftLen );
        if( ret < 0 ){
            if( errno != EAGAIN ){
                CloseAsynIo( host, asynIo );
                return -1;
            }
        }else if( ret == leftLen ){
            NotifyFastEvt( host, &asynIo->outEvt, 0 );
            CloseAsynIo( host, asynIo );
            return count;
        }else if( ret > 0 ){
            writeLen += ret;
            leftLen -= ret;
        }else{
            CloseAsynIo( host, asynIo );
            return 0;
        }
    }
}

inline size_t Seek( DzHost* host, int fd, size_t offset, int whence )
{
    return (size_t)lseek( fd, (off_t)offset, whence );
}

inline size_t FileSize( DzHost* host, int fd )
{
    struct stat st;

    if( fstat( fd, &st ) < 0 ){
        return 0;
    }
    return (size_t)st.st_size;
}

// DzIoMgrRoutine:
// the IO mgr thread uses the host's origin thread's stack
// manager all kernel objects that may cause real block
inline void IoMgrRoutine( DzHost* host )
{
    int i;
    int timeout;
    int listCount;
    DzAsynIo* asynIo;
    struct epoll_event evtList[ EPOLL_EVT_LIST_SIZE ];

    timeout = NotifyMinTimers( host );
    while( host->threadCount ){
        listCount = epoll_wait( host->osStruct.epollFd, evtList, EPOLL_EVT_LIST_SIZE, timeout );
        if( listCount != 0 ){
            for( i = 0; i < listCount; i++ ){
                asynIo = (DzAsynIo*)evtList[i].data.ptr;
                if( evtList[i].events & EPOLLERR ){
                    asynIo->sign |= 1;
                    NotifyFastEvt( host, &asynIo->inEvt, 0 );
                    NotifyFastEvt( host, &asynIo->outEvt, 0 );
                }else{
                    if( evtList[i].events & EPOLLIN ){
                        NotifyFastEvt( host, &asynIo->inEvt, 0 );
                    }
                    if( evtList[i].events & EPOLLOUT ){
                        NotifyFastEvt( host, &asynIo->outEvt, 0 );
                    }
                }
            }
            host->currPriority = CP_FIRST;
            Schedule( host );
        }
        timeout = NotifyMinTimers( host );
    }
}

#ifdef __cplusplus
};
#endif

#endif // __DzIoLnx_h__
