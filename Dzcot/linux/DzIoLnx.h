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
#include "../DzDebug.h"

#ifdef __cplusplus
extern "C"{
#endif

inline DzAsyncIo* CreateAsyncIo( DzHost* host )
{
    DzAsyncIo* asyncIo;

    if( !host->osStruct.asyncIoPool ){
        if( !AllocAsyncIoPool( host ) ){
            return NULL;
        }
    }
    asyncIo = MEMBER_BASE( host->osStruct.asyncIoPool, DzAsyncIo, lItr );
    host->osStruct.asyncIoPool = host->osStruct.asyncIoPool->next;
    asyncIo->err = 0;
    asyncIo->ref++;
    return asyncIo;
}

inline void CloneAsyncIo( DzAsyncIo* asyncIo )
{
    asyncIo->ref++;
}

inline void CloseAsyncIo( DzHost* host, DzAsyncIo* asyncIo )
{
    asyncIo->ref--;
    if( asyncIo->ref == 0 ){
        asyncIo->lItr.next = host->osStruct.asyncIoPool;
        host->osStruct.asyncIoPool = &asyncIo->lItr;
    }
}

inline int Socket( DzHost* host, int domain, int type, int protocol )
{
    int fd;
    int flag;
    struct epoll_event evt;

    fd = socket( domain, type, protocol );
    __DbgSetLastErr( host, errno );
    if( fd >= 0 ){
        host->osStruct.fdTable[ fd ] = CreateAsyncIo( host );
        if( !host->osStruct.fdTable[ fd ] ){
            close( fd );
            return -1;
        }
        evt.data.ptr = host->osStruct.fdTable[ fd ];
        evt.events = EPOLLIN | EPOLLOUT | EPOLLET;
        flag = fcntl( fd, F_GETFL, 0 );
        fcntl( fd, F_SETFL, flag | O_NONBLOCK );
        epoll_ctl( host->osStruct.epollFd, EPOLL_CTL_ADD, fd, &evt );
    }
    return fd;
}

inline int CloseSocket( int fd )
{
    DzHost* host;
    DzAsyncIo* asyncIo;
    int ret;

    ret = close( fd );
    if( ret == 0 ){
        host = GetHost();
        asyncIo = host->osStruct.fdTable[ fd ];
        asyncIo->err = ECONNABORTED;
        if( IsEasyEvtWaiting( &asyncIo->inEvt ) ){
            NotifyEasyEvt( host, &asyncIo->inEvt );
            CleanEasyEvt( &asyncIo->inEvt );
        }
        if( IsEasyEvtWaiting( &asyncIo->outEvt ) ){
            NotifyEasyEvt( host, &asyncIo->outEvt );
            CleanEasyEvt( &asyncIo->outEvt );
        }
        CloseAsyncIo( host, asyncIo );
        host->osStruct.fdTable[ fd ] = NULL;
    }
    return ret;
}

inline int GetSockOpt( int fd, int level, int name, void* option, int* len )
{
    return getsockopt( fd, level, name, (char*)option, (socklen_t*)len );
}

inline int SetSockOpt( int fd, int level, int name, const void* option, int len )
{
    return setsockopt( fd, level, name, (const char*)option, (socklen_t)len );
}

inline int GetSockName( int fd, struct sockaddr* addr, int* addrLen )
{
    return getsockname( fd, addr, (socklen_t*)addrLen );
}

inline int Bind( int fd, struct sockaddr* addr, int addrLen )
{
    return bind( fd, addr, (socklen_t)addrLen );
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
    DzAsyncIo* asyncIo;
    int err;

    if( connect( fd, addr, (socklen_t)addrLen ) != 0 ){
        if( errno == EINPROGRESS ){
            asyncIo = host->osStruct.fdTable[ fd ];
            CloneAsyncIo( asyncIo );
            WaitEasyEvt( host, &asyncIo->outEvt );
            err = asyncIo->err;
            CloseAsyncIo( host, asyncIo );
            if( err ){
                __DbgSetLastErr( host, err );
                return -1;
            }
        }else{
            __DbgSetLastErr( host, errno );
            return -1;
        }
    }
    __DbgSetLastErr( host, 0 );
    return 0;
}

inline int Accept( DzHost* host, int fd, struct sockaddr* addr, int* addrLen )
{
    DzAsyncIo* asyncIo;
    int err;
    int ret;
    int flag;
    struct epoll_event evt;

    ret = accept( fd, addr, (socklen_t*)addrLen );
    if( ret < 0 ){
        if( errno == EAGAIN ){
            asyncIo = host->osStruct.fdTable[ fd ];
            CloneAsyncIo( asyncIo );
            WaitEasyEvt( host, &asyncIo->inEvt );
            err = asyncIo->err;
            CloseAsyncIo( host, asyncIo );
            if( err ){
                __DbgSetLastErr( host, err );
                return -1;
            }
            ret = accept( fd, addr, (socklen_t*)addrLen );
            if( ret < 0 ){
                __DbgSetLastErr( host, errno );
                return -1;
            }
        }else{
            __DbgSetLastErr( host, errno );
            return -1;
        }
    }
    host->osStruct.fdTable[ ret ] = CreateAsyncIo( host );
    if( !host->osStruct.fdTable[ ret ] ){
        __DbgSetLastErr( host, errno );
        close( ret );
        return -1;
    }
    evt.data.ptr = host->osStruct.fdTable[ ret ];
    evt.events = EPOLLIN | EPOLLOUT | EPOLLET;
    flag = fcntl( ret, F_GETFL, 0 );
    fcntl( ret, F_SETFL, flag | O_NONBLOCK );
    epoll_ctl( host->osStruct.epollFd, EPOLL_CTL_ADD, ret, &evt );
    __DbgSetLastErr( host, 0 );
    return ret;
}

inline int SendMsg( DzHost* host, int fd, struct msghdr* msg, int flags )
{
    int ret;
    int err;
    DzAsyncIo* asyncIo;

    ret = sendmsg( fd, msg, flags | MSG_NOSIGNAL );
    if( ret < 0 ){
        if( errno == EAGAIN ){
            asyncIo = host->osStruct.fdTable[ fd ];
            CloneAsyncIo( asyncIo );
            WaitEasyEvt( host, &asyncIo->outEvt );
            err = asyncIo->err;
            CloseAsyncIo( host, asyncIo );
            if( err ){
                __DbgSetLastErr( host, err );
                return -1;
            }
            ret = sendmsg( fd, msg, flags | MSG_NOSIGNAL );
            if( ret < 0 ){
                __DbgSetLastErr( host, errno );
                return -1;
            }
        }else{
            __DbgSetLastErr( host, errno );
            return -1;
        }
    }
    __DbgSetLastErr( host, 0 );
    return ret;
}

inline int RecvMsg( DzHost* host, int fd, struct msghdr* msg, int flags )
{
    int ret;
    int err;
    DzAsyncIo* asyncIo;

    ret = recvmsg( fd, msg, flags );
    if( ret < 0 ){
        if( errno == EAGAIN ){
            asyncIo = host->osStruct.fdTable[ fd ];
            CloneAsyncIo( asyncIo );
            WaitEasyEvt( host, &asyncIo->inEvt );
            err = asyncIo->err;
            CloseAsyncIo( host, asyncIo );
            if( err ){
                __DbgSetLastErr( host, err );
                return -1;
            }
            ret = recvmsg( fd, msg, flags );
            if( ret < 0 ){
                __DbgSetLastErr( host, errno );
                return -1;
            }
        }else{
            __DbgSetLastErr( host, errno );
            return -1;
        }
    }
    __DbgSetLastErr( host, 0 );
    return ret;
}

inline int SendEx( DzHost* host, int fd, DzBuf* bufs, u_int bufCount, int flags )
{
    struct msghdr msg;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = (struct iovec*)bufs;
    msg.msg_iovlen = bufCount;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    return SendMsg( host, fd, &msg, flags );
}

inline int RecvEx( DzHost* host, int fd, DzBuf* bufs, u_int bufCount, int flags )
{
    struct msghdr msg;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = (struct iovec*)bufs;
    msg.msg_iovlen = bufCount;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    return RecvMsg( host, fd, &msg, flags );
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
    struct msghdr msg;

    msg.msg_name = (void*)to;
    msg.msg_namelen = (socklen_t)tolen;
    msg.msg_iov = (struct iovec*)bufs;
    msg.msg_iovlen = bufCount;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    return SendMsg( host, fd, &msg, flags );
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
    int ret;
    struct msghdr msg;

    msg.msg_name = from;
    msg.msg_namelen = (socklen_t)( fromlen ? *fromlen : 0 );
    msg.msg_iov = (struct iovec*)bufs;
    msg.msg_iovlen = bufCount;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    ret = RecvMsg( host, fd, &msg, flags );
    if( fromlen ){
        *fromlen = (int)msg.msg_namelen;
    }
    return ret;
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

inline int OpenA( DzHost* host, const char* fileName, int flags )
{
    int fd;
    int flag;
    struct epoll_event evt;

    fd = open( fileName, flags );
    __DbgSetLastErr( host, errno );
    if( fd >= 0 ){
        host->osStruct.fdTable[ fd ] = CreateAsyncIo( host );
        if( !host->osStruct.fdTable[ fd ] ){
            close( fd );
            return -1;
        }
        evt.data.ptr = host->osStruct.fdTable[ fd ];
        evt.events = EPOLLIN | EPOLLOUT | EPOLLET;
        flag = fcntl( fd, F_GETFL, 0 );
        fcntl( fd, F_SETFL, flag | O_NONBLOCK );
        epoll_ctl( host->osStruct.epollFd, EPOLL_CTL_ADD, fd, &evt );
    }
    return fd;
}

inline int OpenW( DzHost* host, const wchar_t* fileName, int flags )
{
    return -1;
}

inline int Close( int fd )
{
    DzHost* host;
    DzAsyncIo* asyncIo;
    int ret;

    ret = close( fd );
    if( ret == 0 ){
        host = GetHost();
        asyncIo = host->osStruct.fdTable[ fd ];
        asyncIo->err = ECONNABORTED;
        if( IsEasyEvtWaiting( &asyncIo->inEvt ) ){
            NotifyEasyEvt( host, &asyncIo->inEvt );
            CleanEasyEvt( &asyncIo->inEvt );
        }
        if( IsEasyEvtWaiting( &asyncIo->outEvt ) ){
            NotifyEasyEvt( host, &asyncIo->outEvt );
            CleanEasyEvt( &asyncIo->outEvt );
        }
        CloseAsyncIo( host, asyncIo );
        host->osStruct.fdTable[ fd ] = NULL;
    }
    return ret;
}

inline size_t Read( DzHost* host, int fd, void* buf, size_t count )
{
    int ret;
    int err;
    DzAsyncIo* asyncIo;

    ret = read( fd, buf, count );
    if( ret < 0 ){
        if( errno == EAGAIN ){
            asyncIo = host->osStruct.fdTable[ fd ];
            CloneAsyncIo( asyncIo );
            WaitEasyEvt( host, &asyncIo->inEvt );
            err = asyncIo->err;
            CloseAsyncIo( host, asyncIo );
            if( err ){
                __DbgSetLastErr( host, err );
                return -1;
            }
            ret = read( fd, buf, count );
            if( ret < 0 ){
                __DbgSetLastErr( host, errno );
                return -1;
            }
        }else{
            __DbgSetLastErr( host, errno );
            return -1;
        }
    }
    __DbgSetLastErr( host, 0 );
    return ret;
}

inline size_t Write( DzHost* host, int fd, const void* buf, size_t count )
{
    int ret;
    int err;
    DzAsyncIo* asyncIo;

    if( isfdtype( fd, S_IFSOCK ) > 0 ){
        return (size_t)Send( host, fd, buf, (u_int)count, 0 );
    }
    ret = write( fd, buf, count );
    if( ret < 0 ){
        if( errno == EAGAIN ){
            asyncIo = host->osStruct.fdTable[ fd ];
            CloneAsyncIo( asyncIo );
            WaitEasyEvt( host, &asyncIo->outEvt );
            err = asyncIo->err;
            CloseAsyncIo( host, asyncIo );
            if( err ){
                __DbgSetLastErr( host, err );
                return -1;
            }
            ret = write( fd, buf, count );
            if( ret < 0 ){
                __DbgSetLastErr( host, errno );
                return -1;
            }
        }else{
            __DbgSetLastErr( host, errno );
            return -1;
        }
    }
    __DbgSetLastErr( host, 0 );
    return ret;
}

inline size_t Seek( int fd, size_t offset, int whence )
{
    return (size_t)lseek( fd, (off_t)offset, whence );
}

inline size_t FileSize( int fd )
{
    struct stat st;

    if( fstat( fd, &st ) < 0 ){
        return -1;
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
    int notifyCount;
    DzAsyncIo* asyncIo;
    struct epoll_event evtList[ EPOLL_EVT_LIST_SIZE ];

    timeout = NotifyMinTimers( host );
    while( host->threadCount ){
        listCount = epoll_wait( host->osStruct.epollFd, evtList, EPOLL_EVT_LIST_SIZE, timeout );
        if( listCount != 0 ){
            notifyCount = 0;
            for( i = 0; i < listCount; i++ ){
                asyncIo = (DzAsyncIo*)evtList[i].data.ptr;
                if( IsEasyEvtWaiting( &asyncIo->inEvt ) && ( evtList[i].events & EPOLLIN ) ){
                    NotifyEasyEvt( host, &asyncIo->inEvt );
                    CleanEasyEvt( &asyncIo->inEvt );
                    notifyCount++;
                }
                if( IsEasyEvtWaiting( &asyncIo->outEvt ) && ( evtList[i].events & EPOLLOUT ) ){
                    NotifyEasyEvt( host, &asyncIo->outEvt );
                    CleanEasyEvt( &asyncIo->outEvt );
                    notifyCount++;
                }
            }
            if( notifyCount ){
                host->currPriority = CP_FIRST;
                Schedule( host );
            }
        }
        timeout = NotifyMinTimers( host );
    }
}

#ifdef __cplusplus
};
#endif

#endif // __DzIoLnx_h__
