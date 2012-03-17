/********************************************************************
    created:    2010/02/11 22:03
    file:       DzIoLnx.h
    author:     Foreverflying
    purpose:
********************************************************************/

#ifndef __DzIoLnx_h__
#define __DzIoLnx_h__

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
    int fd;
    DzFd* dzFd;
    int flag;
    struct epoll_event evt;

    fd = socket( domain, type, protocol );
    if( fd >= 0 ){
        dzFd = CreateDzFd( host );
        dzFd->fd = fd;
        evt.data.ptr = dzFd;
        evt.events = EPOLLIN | EPOLLOUT | EPOLLET;
        flag = fcntl( fd, F_GETFL, 0 );
        fcntl( fd, F_SETFL, flag | O_NONBLOCK );
        epoll_ctl( host->os.epollFd, EPOLL_CTL_ADD, fd, &evt );
        return (int)( (intptr_t)dzFd - host->handleBase );
    }else{
        __DbgSetLastErr( host, errno );
        return -1;
    }
}

inline int CloseSocket( DzHost* host, int hFd )
{
    DzFd* dzFd;
    int ret;

    dzFd = (DzFd*)( host->handleBase + hFd );
    ret = close( dzFd->fd );
    if( ret == 0 ){
        dzFd->err = ECONNABORTED;
        if( IsEasyEvtWaiting( &dzFd->inEvt ) ){
            NotifyEasyEvt( host, &dzFd->inEvt );
            CleanEasyEvt( &dzFd->inEvt );
        }
        if( IsEasyEvtWaiting( &dzFd->outEvt ) ){
            NotifyEasyEvt( host, &dzFd->outEvt );
            CleanEasyEvt( &dzFd->outEvt );
        }
        CloseDzFd( host, dzFd );
    }
    return ret;
}

inline int GetSockOpt( DzHost* host, int hFd, int level, int name, void* option, int* len )
{
    DzFd* dzFd = (DzFd*)( host->handleBase + hFd );
    return getsockopt( dzFd->fd, level, name, (char*)option, (socklen_t*)len );
}

inline int SetSockOpt( DzHost* host, int hFd, int level, int name, const void* option, int len )
{
    DzFd* dzFd = (DzFd*)( host->handleBase + hFd );
    return setsockopt( dzFd->fd, level, name, (const char*)option, (socklen_t)len );
}

inline int GetSockName( DzHost* host, int hFd, struct sockaddr* addr, int* addrLen )
{
    DzFd* dzFd = (DzFd*)( host->handleBase + hFd );
    return getsockname( dzFd->fd, addr, (socklen_t*)addrLen );
}

inline int Bind( DzHost* host, int hFd, struct sockaddr* addr, int addrLen )
{
    DzFd* dzFd = (DzFd*)( host->handleBase + hFd );
    return bind( dzFd->fd, addr, (socklen_t)addrLen );
}

inline int Listen( DzHost* host, int hFd, int backlog )
{
    DzFd* dzFd = (DzFd*)( host->handleBase + hFd );
    return listen( dzFd->fd, backlog );
}

inline int Shutdown( DzHost* host, int hFd, int how )
{
    DzFd* dzFd = (DzFd*)( host->handleBase + hFd );
    return shutdown( dzFd->fd, how );
}

inline int Connect( DzHost* host, int hFd, struct sockaddr* addr, int addrLen )
{
    DzFd* dzFd;
    int err;
    socklen_t errLen;

    dzFd = (DzFd*)( host->handleBase + hFd );
    if( connect( dzFd->fd, addr, (socklen_t)addrLen ) != 0 ){
        if( errno == EINPROGRESS ){
            CloneDzFd( dzFd );
            WaitEasyEvt( host, &dzFd->outEvt );
            if( dzFd->err ){
                __DbgSetLastErr( host, dzFd->err );
                CloseDzFd( host, dzFd );
                return -1;
            }
            CloseDzFd( host, dzFd );
            errLen = sizeof( err );
            getsockopt( dzFd->fd, SOL_SOCKET, SO_ERROR, &err, &errLen );
            if( err ){
                __DbgSetLastErr( host, err );
                return -1;
            }
        }else{
            __DbgSetLastErr( host, errno );
            return -1;
        }
    }
    return 0;
}

inline int Accept( DzHost* host, int hFd, struct sockaddr* addr, int* addrLen )
{
    DzFd* dzFd;
    int ret;
    int flag;
    struct epoll_event evt;

    dzFd = (DzFd*)( host->handleBase + hFd );
    ret = accept( dzFd->fd, addr, (socklen_t*)addrLen );
    if( ret < 0 ){
        if( errno == EAGAIN ){
            CloneDzFd( dzFd );
            WaitEasyEvt( host, &dzFd->inEvt );
            if( dzFd->err ){
                __DbgSetLastErr( host, dzFd->err );
                CloseDzFd( host, dzFd );
                return -1;
            }
            CloseDzFd( host, dzFd );
            ret = accept( dzFd->fd, addr, (socklen_t*)addrLen );
            if( ret < 0 ){
                __DbgSetLastErr( host, errno );
                return -1;
            }
        }else{
            __DbgSetLastErr( host, errno );
            return -1;
        }
    }
    dzFd = CreateDzFd( host );
    dzFd->fd = ret;
    evt.data.ptr = dzFd;
    evt.events = EPOLLIN | EPOLLOUT | EPOLLET;
    flag = fcntl( ret, F_GETFL, 0 );
    fcntl( ret, F_SETFL, flag | O_NONBLOCK );
    epoll_ctl( host->os.epollFd, EPOLL_CTL_ADD, ret, &evt );
    return (int)( (intptr_t)dzFd - host->handleBase );
}

inline int SendMsg( DzHost* host, int hFd, struct msghdr* msg, int flags )
{
    DzFd* dzFd;
    int ret;

    dzFd = (DzFd*)( host->handleBase + hFd );
    ret = sendmsg( dzFd->fd, msg, flags | MSG_NOSIGNAL );
    if( ret < 0 ){
        if( errno == EAGAIN ){
            CloneDzFd( dzFd );
            WaitEasyEvt( host, &dzFd->outEvt );
            if( dzFd->err ){
                __DbgSetLastErr( host, dzFd->err );
                CloseDzFd( host, dzFd );
                return -1;
            }
            CloseDzFd( host, dzFd );
            ret = sendmsg( dzFd->fd, msg, flags | MSG_NOSIGNAL );
            if( ret < 0 ){
                __DbgSetLastErr( host, errno );
                return -1;
            }
        }else{
            __DbgSetLastErr( host, errno );
            return -1;
        }
    }
    return ret;
}

inline int RecvMsg( DzHost* host, int hFd, struct msghdr* msg, int flags )
{
    DzFd* dzFd;
    int ret;

    dzFd = (DzFd*)( host->handleBase + hFd );
    ret = recvmsg( dzFd->fd, msg, flags );
    if( ret < 0 ){
        if( errno == EAGAIN ){
            CloneDzFd( dzFd );
            WaitEasyEvt( host, &dzFd->inEvt );
            if( dzFd->err ){
                __DbgSetLastErr( host, dzFd->err );
                CloseDzFd( host, dzFd );
                return -1;
            }
            CloseDzFd( host, dzFd );
            ret = recvmsg( dzFd->fd, msg, flags );
            if( ret < 0 ){
                __DbgSetLastErr( host, errno );
                return -1;
            }
        }else{
            __DbgSetLastErr( host, errno );
            return -1;
        }
    }
    return ret;
}

inline int SendEx( DzHost* host, int hFd, DzBuf* bufs, u_int bufCount, int flags )
{
    struct msghdr msg;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = (struct iovec*)bufs;
    msg.msg_iovlen = bufCount;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    return SendMsg( host, hFd, &msg, flags );
}

inline int RecvEx( DzHost* host, int hFd, DzBuf* bufs, u_int bufCount, int flags )
{
    struct msghdr msg;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = (struct iovec*)bufs;
    msg.msg_iovlen = bufCount;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    return RecvMsg( host, hFd, &msg, flags );
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
    struct msghdr msg;

    msg.msg_name = (void*)to;
    msg.msg_namelen = (socklen_t)tolen;
    msg.msg_iov = (struct iovec*)bufs;
    msg.msg_iovlen = bufCount;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    return SendMsg( host, hFd, &msg, flags );
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
    int ret;
    struct msghdr msg;

    msg.msg_name = from;
    msg.msg_namelen = (socklen_t)( fromlen ? *fromlen : 0 );
    msg.msg_iov = (struct iovec*)bufs;
    msg.msg_iovlen = bufCount;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    ret = RecvMsg( host, hFd, &msg, flags );
    if( fromlen ){
        *fromlen = (int)msg.msg_namelen;
    }
    return ret;
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

inline int OpenA( DzHost* host, const char* fileName, int flags )
{
    int fd;
    DzFd* dzFd;
    struct epoll_event evt;

    fd = open( fileName, flags | O_NONBLOCK );
    if( fd >= 0 ){
        dzFd = CreateDzFd( host );
        dzFd->fd = fd;
        dzFd->isSock = isfdtype( fd, S_IFSOCK ) > 0;
        evt.data.ptr = dzFd;
        evt.events = EPOLLIN | EPOLLOUT | EPOLLET;
        epoll_ctl( host->os.epollFd, EPOLL_CTL_ADD, fd, &evt );
        return (int)( (intptr_t)dzFd - host->handleBase );
    }else{
        __DbgSetLastErr( host, errno );
        return -1;
    }
}

inline int OpenW( DzHost* host, const wchar_t* fileName, int flags )
{
    return -1;
}

inline int Close( DzHost* host, int hFd )
{
    DzFd* dzFd;
    int ret;

    dzFd = (DzFd*)( host->handleBase + hFd );
    ret = close( dzFd->fd );
    if( ret == 0 ){
        dzFd->isSock = TRUE;
        dzFd->err = ECONNABORTED;
        if( IsEasyEvtWaiting( &dzFd->inEvt ) ){
            NotifyEasyEvt( host, &dzFd->inEvt );
            CleanEasyEvt( &dzFd->inEvt );
        }
        if( IsEasyEvtWaiting( &dzFd->outEvt ) ){
            NotifyEasyEvt( host, &dzFd->outEvt );
            CleanEasyEvt( &dzFd->outEvt );
        }
        CloseDzFd( host, dzFd );
    }
    return ret;
}

inline size_t Read( DzHost* host, int hFd, void* buf, size_t count )
{
    DzFd* dzFd;
    int ret;

    dzFd = (DzFd*)( host->handleBase + hFd );
    ret = read( dzFd->fd, buf, count );
    if( ret < 0 ){
        if( errno == EAGAIN ){
            CloneDzFd( dzFd );
            WaitEasyEvt( host, &dzFd->inEvt );
            if( dzFd->err ){
                __DbgSetLastErr( host, dzFd->err );
                CloseDzFd( host, dzFd );
                return -1;
            }
            CloseDzFd( host, dzFd );
            ret = read( dzFd->fd, buf, count );
            if( ret < 0 ){
                __DbgSetLastErr( host, errno );
                return -1;
            }
        }else{
            __DbgSetLastErr( host, errno );
            return -1;
        }
    }
    return ret;
}

inline size_t Write( DzHost* host, int hFd, const void* buf, size_t count )
{
    DzFd* dzFd;
    int ret;

    dzFd = (DzFd*)( host->handleBase + hFd );
    if( dzFd->isSock ){
        return (size_t)Send( host, hFd, buf, (u_int)count, 0 );
    }
    ret = write( dzFd->fd, buf, count );
    if( ret < 0 ){
        if( errno == EAGAIN ){
            CloneDzFd( dzFd );
            WaitEasyEvt( host, &dzFd->outEvt );
            if( dzFd->err ){
                __DbgSetLastErr( host, dzFd->err );
                CloseDzFd( host, dzFd );
                return -1;
            }
            CloseDzFd( host, dzFd );
            ret = write( dzFd->fd, buf, count );
            if( ret < 0 ){
                __DbgSetLastErr( host, errno );
                return -1;
            }
        }else{
            __DbgSetLastErr( host, errno );
            return -1;
        }
    }
    return ret;
}

inline size_t Seek( DzHost* host, int hFd, size_t offset, int whence )
{
    DzFd* dzFd = (DzFd*)( host->handleBase + hFd );
    return (size_t)lseek( dzFd->fd, (off_t)offset, whence );
}

inline size_t FileSize( DzHost* host, int hFd )
{
    DzFd* dzFd;
    struct stat st;

    dzFd = (DzFd*)( host->handleBase + hFd );
    if( fstat( dzFd->fd, &st ) < 0 ){
        return -1;
    }
    return (size_t)st.st_size;
}

inline void BlockAndDispatchIo( DzHost* host, int timeout )
{
    int i;
    int listCount;
    DzFd* dzFd;
    struct epoll_event* evtList;

    evtList = host->os.evtList;
    listCount = epoll_wait( host->os.epollFd, evtList, EPOLL_EVT_LIST_SIZE, timeout );
    AtomAndInt( host->rmtCheckSignPtr, ~RMT_CHECK_SLEEP_SIGN );
    if( listCount != 0 ){
        while( 1 ){
            for( i = 0; i < listCount; i++ ){
                dzFd = (DzFd*)evtList[i].data.ptr;
                if( IsEasyEvtWaiting( &dzFd->inEvt ) && ( evtList[i].events & EPOLLIN ) ){
                    NotifyEasyEvt( host, &dzFd->inEvt );
                    CleanEasyEvt( &dzFd->inEvt );
                }
                if( IsEasyEvtWaiting( &dzFd->outEvt ) && ( evtList[i].events & EPOLLOUT ) ){
                    NotifyEasyEvt( host, &dzFd->outEvt );
                    CleanEasyEvt( &dzFd->outEvt );
                }
            }
            if( listCount == EPOLL_EVT_LIST_SIZE ){
                listCount = epoll_wait( host->os.epollFd, evtList, EPOLL_EVT_LIST_SIZE, 0 );
                continue;
            }
            break;
        }
        read( host->os.pipe[0], evtList, 2048 );
    }
}

inline void BlockAndDispatchIoNoRmtCheck( DzHost* host, int timeout )
{
    int i;
    int listCount;
    DzFd* dzFd;
    struct epoll_event* evtList;

    evtList = host->os.evtList;
    listCount = epoll_wait( host->os.epollFd, evtList, EPOLL_EVT_LIST_SIZE, timeout );
    if( listCount != 0 ){
        while( 1 ){
            for( i = 0; i < listCount; i++ ){
                dzFd = (DzFd*)evtList[i].data.ptr;
                if( IsEasyEvtWaiting( &dzFd->inEvt ) && ( evtList[i].events & EPOLLIN ) ){
                    NotifyEasyEvt( host, &dzFd->inEvt );
                    CleanEasyEvt( &dzFd->inEvt );
                }
                if( IsEasyEvtWaiting( &dzFd->outEvt ) && ( evtList[i].events & EPOLLOUT ) ){
                    NotifyEasyEvt( host, &dzFd->outEvt );
                    CleanEasyEvt( &dzFd->outEvt );
                }
            }
            if( listCount == EPOLL_EVT_LIST_SIZE ){
                listCount = epoll_wait( host->os.epollFd, evtList, EPOLL_EVT_LIST_SIZE, 0 );
                continue;
            }
            break;
        }
    }
}

#ifdef __cplusplus
};
#endif

#endif // __DzIoLnx_h__
