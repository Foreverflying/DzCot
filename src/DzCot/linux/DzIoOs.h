/**
 *  @file       DzIoOs.h
 *  @brief      for linux
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzIoOs_h__
#define __DzIoOs_h__

#include "../DzStructs.h"
#include "DzStructsOs.h"
#include "../DzBase.h"
#include "../DzResourceMgr.h"
#include "../DzSynObj.h"
#include "../DzDebug.h"

void __stdcall GetNameInfoEntry(intptr_t context);
void __stdcall GetAddrInfoEntry(intptr_t context);

static inline
int Socket(DzHost* host, int domain, int type, int protocol)
{
    int fd;
    DzFd* dzFd;
    int flag;
    struct epoll_event evt;

    fd = socket(domain, type, protocol);
    if (fd >= 0) {
        dzFd = CreateDzFd(host);
        dzFd->fd = fd;
        dzFd->notSock = FALSE;
        evt.data.ptr = dzFd;
        evt.events = EPOLLIN | EPOLLOUT | EPOLLET;
        flag = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flag | O_NONBLOCK);
        epoll_ctl(host->os.epollFd, EPOLL_CTL_ADD, fd, &evt);
        return (int)((intptr_t)dzFd - host->handleBase);
    } else {
        __Dbg(SetLastErr)(host, errno);
        return -1;
    }
}

static inline
intptr_t RawSocket(DzHost* host, int hFd)
{
    DzFd* dzFd = (DzFd*)(host->handleBase + hFd);
    return dzFd->fd;
}

static inline
int GetSockOpt(DzHost* host, int hFd, int level, int name, void* option, socklen_t* len)
{
    DzFd* dzFd = (DzFd*)(host->handleBase + hFd);
    return getsockopt(dzFd->fd, level, name, (char*)option, len);
}

static inline
int SetSockOpt(DzHost* host, int hFd, int level, int name, const void* option, socklen_t len)
{
    DzFd* dzFd = (DzFd*)(host->handleBase + hFd);
    return setsockopt(dzFd->fd, level, name, (const char*)option, len);
}

static inline
int GetSockName(DzHost* host, int hFd, struct sockaddr* addr, socklen_t* addrLen)
{
    DzFd* dzFd = (DzFd*)(host->handleBase + hFd);
    return getsockname(dzFd->fd, addr, addrLen);
}

static inline
int GetPeerName(DzHost* host, int hFd, struct sockaddr* addr, socklen_t* addrLen)
{
    DzFd* dzFd = (DzFd*)(host->handleBase + hFd);
    return getpeername(dzFd->fd, addr, addrLen);
}

static inline
int Bind(DzHost* host, int hFd, const struct sockaddr* addr, socklen_t addrLen)
{
    DzFd* dzFd = (DzFd*)(host->handleBase + hFd);
    return bind(dzFd->fd, addr, addrLen);
}

static inline
int Listen(DzHost* host, int hFd, int backlog)
{
    DzFd* dzFd = (DzFd*)(host->handleBase + hFd);
    return listen(dzFd->fd, backlog);
}

static inline
int Shutdown(DzHost* host, int hFd, int how)
{
    DzFd* dzFd = (DzFd*)(host->handleBase + hFd);
    return shutdown(dzFd->fd, how);
}

static inline
int Connect(DzHost* host, int hFd, const struct sockaddr* addr, socklen_t addrLen)
{
    DzFd* dzFd;
    int err;
    socklen_t errLen;

    dzFd = (DzFd*)(host->handleBase + hFd);
    if (connect(dzFd->fd, addr, addrLen) != 0) {
        if (errno == EINPROGRESS) {
            CloneDzFd(dzFd);
            WaitEasyEvt(host, &dzFd->outEvt);
            if (dzFd->err) {
                __Dbg(SetLastErr)(host, dzFd->err);
                CloseDzFd(host, dzFd);
                return -1;
            }
            CloseDzFd(host, dzFd);
            errLen = sizeof(err);
            getsockopt(dzFd->fd, SOL_SOCKET, SO_ERROR, &err, &errLen);
            if (err) {
                __Dbg(SetLastErr)(host, err);
                return -1;
            }
        } else {
            __Dbg(SetLastErr)(host, errno);
            return -1;
        }
    }
    return 0;
}

static inline
int Accept(DzHost* host, int hFd, struct sockaddr* addr, socklen_t* addrLen)
{
    DzFd* dzFd;
    int ret;
    int flag;
    struct epoll_event evt;

    dzFd = (DzFd*)(host->handleBase + hFd);
    ret = accept(dzFd->fd, addr, addrLen);
    if (ret < 0) {
        if (errno == EAGAIN) {
            CloneDzFd(dzFd);
            WaitEasyEvt(host, &dzFd->inEvt);
            if (dzFd->err) {
                __Dbg(SetLastErr)(host, dzFd->err);
                CloseDzFd(host, dzFd);
                return -1;
            }
            CloseDzFd(host, dzFd);
            ret = accept(dzFd->fd, addr, addrLen);
            if (ret < 0) {
                __Dbg(SetLastErr)(host, errno);
                return -1;
            }
        } else {
            __Dbg(SetLastErr)(host, errno);
            return -1;
        }
    }
    dzFd = CreateDzFd(host);
    dzFd->fd = ret;
    dzFd->notSock = FALSE;
    evt.data.ptr = dzFd;
    evt.events = EPOLLIN | EPOLLOUT | EPOLLET;
    flag = fcntl(ret, F_GETFL, 0);
    fcntl(ret, F_SETFL, flag | O_NONBLOCK);
    epoll_ctl(host->os.epollFd, EPOLL_CTL_ADD, ret, &evt);
    return (int)((intptr_t)dzFd - host->handleBase);
}

static inline
int SendMsg(DzHost* host, int hFd, struct msghdr* msg, int flags)
{
    DzFd* dzFd;
    int ret;

    dzFd = (DzFd*)(host->handleBase + hFd);
    ret = sendmsg(dzFd->fd, msg, flags | MSG_NOSIGNAL);
    if (ret < 0) {
        if (errno == EAGAIN) {
            CloneDzFd(dzFd);
            WaitEasyEvt(host, &dzFd->outEvt);
            if (dzFd->err) {
                __Dbg(SetLastErr)(host, dzFd->err);
                CloseDzFd(host, dzFd);
                return -1;
            }
            CloseDzFd(host, dzFd);
            ret = sendmsg(dzFd->fd, msg, flags | MSG_NOSIGNAL);
            if (ret < 0) {
                __Dbg(SetLastErr)(host, errno);
                return -1;
            }
        } else {
            __Dbg(SetLastErr)(host, errno);
            return -1;
        }
    }
    return ret;
}

static inline
int RecvMsg(DzHost* host, int hFd, struct msghdr* msg, int flags)
{
    DzFd* dzFd;
    int ret;

    dzFd = (DzFd*)(host->handleBase + hFd);
    ret = recvmsg(dzFd->fd, msg, flags);
    if (ret < 0) {
        if (errno == EAGAIN) {
            CloneDzFd(dzFd);
            WaitEasyEvt(host, &dzFd->inEvt);
            if (dzFd->err) {
                __Dbg(SetLastErr)(host, dzFd->err);
                CloseDzFd(host, dzFd);
                return -1;
            }
            CloseDzFd(host, dzFd);
            ret = recvmsg(dzFd->fd, msg, flags);
            if (ret < 0) {
                __Dbg(SetLastErr)(host, errno);
                return -1;
            }
        } else {
            __Dbg(SetLastErr)(host, errno);
            return -1;
        }
    }
    return ret;
}

static inline
int IovSend(DzHost* host, int hFd, DzIov* bufs, size_t bufCount, int flags)
{
    struct msghdr msg;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = (struct iovec*)bufs;
    msg.msg_iovlen = bufCount;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    return SendMsg(host, hFd, &msg, flags);
}

static inline
int IovRecv(DzHost* host, int hFd, DzIov* bufs, size_t bufCount, int flags)
{
    struct msghdr msg;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = (struct iovec*)bufs;
    msg.msg_iovlen = bufCount;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    return RecvMsg(host, hFd, &msg, flags);
}

static inline
int Send(DzHost* host, int hFd, const void* buf, size_t len, int flags)
{
    DzIov tmpBuf;

    tmpBuf.len = len;
    tmpBuf.buf = (void*)buf;
    return IovSend(host, hFd, &tmpBuf, 1, flags);
}

static inline
int Recv(DzHost* host, int hFd, void* buf, size_t len, int flags)
{
    DzIov tmpBuf;

    tmpBuf.len = len;
    tmpBuf.buf = buf;
    return IovRecv(host, hFd, &tmpBuf, 1, flags);
}

static inline
int IovSendTo(
    DzHost*                 host,
    int                     hFd,
    DzIov*                  bufs,
    size_t                  bufCount,
    int                     flags,
    const struct sockaddr*  to,
    socklen_t               tolen
    )
{
    struct msghdr msg;

    msg.msg_name = (void*)to;
    msg.msg_namelen = tolen;
    msg.msg_iov = (struct iovec*)bufs;
    msg.msg_iovlen = bufCount;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    return SendMsg(host, hFd, &msg, flags);
}

static inline
int IovRecvFrom(
    DzHost*                 host,
    int                     hFd,
    DzIov*                  bufs,
    size_t                  bufCount,
    int                     flags,
    struct sockaddr*        from,
    socklen_t*              fromlen
    )
{
    int ret;
    struct msghdr msg;

    msg.msg_name = from;
    msg.msg_namelen = fromlen ? *fromlen : 0;
    msg.msg_iov = (struct iovec*)bufs;
    msg.msg_iovlen = bufCount;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    ret = RecvMsg(host, hFd, &msg, flags);
    if (fromlen) {
        *fromlen = msg.msg_namelen;
    }
    return ret;
}

static inline
int SendTo(
    DzHost*                 host,
    int                     hFd,
    const void*             buf,
    size_t                  len,
    int                     flags,
    const struct sockaddr*  to,
    socklen_t               tolen
    )
{
    DzIov tmpBuf;

    tmpBuf.len = len;
    tmpBuf.buf = (void*)buf;
    return IovSendTo(host, hFd, &tmpBuf, 1, flags, to, tolen);
}

static inline
int RecvFrom(
    DzHost*                 host,
    int                     hFd,
    void*                   buf,
    size_t                  len,
    int                     flags,
    struct sockaddr*        from,
    socklen_t*              fromlen
    )
{
    DzIov tmpBuf;

    tmpBuf.len = len;
    tmpBuf.buf = buf;
    return IovRecvFrom(host, hFd, &tmpBuf, 1, flags, from, fromlen);
}

static inline
int DGetNameInfo(
    DzHost*                 dzHost,
    const struct sockaddr*  sa,
    socklen_t               salen,
    char*                   host,
    size_t                  hostlen,
    char*                   serv,
    size_t                  servlen,
    int                     flags
    )
{
    int ret;
    DzLNode* node = AllocLNode(dzHost);
    node->d1 = (intptr_t)sa;
    node->d2 = (intptr_t)salen;
    node->d3 = (intptr_t)host;
    node->d4 = (intptr_t)hostlen;
    node->d5 = (intptr_t)serv;
    node->d6 = (intptr_t)servlen;
    node->d7 = (intptr_t)flags;
    node->d8 = (intptr_t)&ret;
    RunWorker(dzHost, GetNameInfoEntry, (intptr_t)node);
    FreeLNode(dzHost, node);
    return ret;
}

static inline
int DGetAddrInfo(
    DzHost*                 host,
    const char*             node,
    const char*             service,
    const struct addrinfo*  hints,
    struct addrinfo**       res
    )
{
    int ret;
    DzLNode* param = AllocLNode(host);
    param->d1 = (intptr_t)node;
    param->d2 = (intptr_t)service;
    param->d3 = (intptr_t)hints;
    param->d4 = (intptr_t)res;
    param->d8 = (intptr_t)&ret;
    RunWorker(host, GetAddrInfoEntry, (intptr_t)param);
    FreeLNode(host, param);
    return ret;
}

static inline
void DFreeAddrInfo(struct addrinfo *res)
{
    freeaddrinfo(res);
}

static inline
int DInetPton(int af, const char* src, void* dst)
{
    return inet_pton(af, src, dst);
}

static inline
const char* DInetNtop(int af, const void* src, char* dst, socklen_t size)
{
    return inet_ntop(af, src, dst, size);
}

static inline
int Open(DzHost* host, const char* fileName, int flags)
{
    int fd;
    DzFd* dzFd;
    struct epoll_event evt;

    fd = open(fileName, flags | O_NONBLOCK, 0664);
    if (fd >= 0) {
        dzFd = CreateDzFd(host);
        dzFd->fd = fd;
        dzFd->notSock = isfdtype(fd, S_IFSOCK) <= 0;
        evt.data.ptr = dzFd;
        evt.events = EPOLLIN | EPOLLOUT | EPOLLET;
        epoll_ctl(host->os.epollFd, EPOLL_CTL_ADD, fd, &evt);
        return (int)((intptr_t)dzFd - host->handleBase);
    } else {
        __Dbg(SetLastErr)(host, errno);
        return -1;
    }
}

static inline
size_t Read(DzHost* host, int hFd, void* buf, size_t count)
{
    DzFd* dzFd;
    int ret;

    dzFd = (DzFd*)(host->handleBase + hFd);
    ret = read(dzFd->fd, buf, count);
    if (ret < 0) {
        if (errno == EAGAIN) {
            CloneDzFd(dzFd);
            WaitEasyEvt(host, &dzFd->inEvt);
            if (dzFd->err) {
                __Dbg(SetLastErr)(host, dzFd->err);
                CloseDzFd(host, dzFd);
                return -1;
            }
            CloseDzFd(host, dzFd);
            ret = read(dzFd->fd, buf, count);
            if (ret < 0) {
                __Dbg(SetLastErr)(host, errno);
                return -1;
            }
        } else {
            __Dbg(SetLastErr)(host, errno);
            return -1;
        }
    }
    return ret;
}

static inline
size_t Write(DzHost* host, int hFd, const void* buf, size_t count)
{
    DzFd* dzFd;
    int ret;

    dzFd = (DzFd*)(host->handleBase + hFd);
    if (!dzFd->notSock) {
        return (size_t)Send(host, hFd, buf, (u_int)count, 0);
    }
    ret = write(dzFd->fd, buf, count);
    if (ret < 0) {
        if (errno == EAGAIN) {
            CloneDzFd(dzFd);
            WaitEasyEvt(host, &dzFd->outEvt);
            if (dzFd->err) {
                __Dbg(SetLastErr)(host, dzFd->err);
                CloseDzFd(host, dzFd);
                return -1;
            }
            CloseDzFd(host, dzFd);
            ret = write(dzFd->fd, buf, count);
            if (ret < 0) {
                __Dbg(SetLastErr)(host, errno);
                return -1;
            }
        } else {
            __Dbg(SetLastErr)(host, errno);
            return -1;
        }
    }
    return ret;
}

static inline
size_t Seek(DzHost* host, int hFd, size_t offset, int whence)
{
    DzFd* dzFd = (DzFd*)(host->handleBase + hFd);
    return (size_t)lseek(dzFd->fd, (off_t)offset, whence);
}

static inline
size_t FileSize(DzHost* host, int hFd)
{
    DzFd* dzFd;
    struct stat st;

    dzFd = (DzFd*)(host->handleBase + hFd);
    if (fstat(dzFd->fd, &st) < 0) {
        return -1;
    }
    return (size_t)st.st_size;
}

static inline
intptr_t GetFdData(DzHost* host, int hFd)
{
    DzFd* dzFd = (DzFd*)(host->handleBase + hFd);
    return dzFd->fdData;
}

static inline
void SetFdData(DzHost* host, int hFd, intptr_t data)
{
    DzFd* dzFd = (DzFd*)(host->handleBase + hFd);
    dzFd->fdData = data;
}

static inline
int Close(DzHost* host, int hFd)
{
    DzFd* dzFd;
    int ret;

    dzFd = (DzFd*)(host->handleBase + hFd);
    ret = close(dzFd->fd);
    if (ret == 0) {
        dzFd->err = ECONNABORTED;
        if (IsEasyEvtWaiting(&dzFd->inEvt)) {
            NotifyEasyEvt(host, &dzFd->inEvt);
            CleanEasyEvt(&dzFd->inEvt);
        }
        if (IsEasyEvtWaiting(&dzFd->outEvt)) {
            NotifyEasyEvt(host, &dzFd->outEvt);
            CleanEasyEvt(&dzFd->outEvt);
        }
        CloseDzFd(host, dzFd);
    }
    return ret;
}

static inline
void BlockAndDispatchIo(DzHost* host, int timeout)
{
    int signAtWakeUp;
    int listCount;
    DzFd* dzFd;
    struct epoll_event* evtList;
    struct epoll_event* itr;

    evtList = host->os.evtList;
    listCount = epoll_wait(host->os.epollFd, evtList, EPOLL_EVT_LIST_SIZE, timeout);
    signAtWakeUp = AtomOrInt(host->rmtCheckSignPtr, RMT_CHECK_AWAKE_SIGN);
    if (listCount) {
        while (1) {
            for (itr = evtList; itr != evtList + listCount; itr++) {
                dzFd = (DzFd*)itr->data.ptr;
                if (IsEasyEvtWaiting(&dzFd->inEvt) && (itr->events & EPOLLIN)) {
                    NotifyEasyEvt(host, &dzFd->inEvt);
                    CleanEasyEvt(&dzFd->inEvt);
                }
                if (IsEasyEvtWaiting(&dzFd->outEvt) && (itr->events & EPOLLOUT)) {
                    NotifyEasyEvt(host, &dzFd->outEvt);
                    CleanEasyEvt(&dzFd->outEvt);
                }
            }
            if (listCount == EPOLL_EVT_LIST_SIZE) {
                listCount = epoll_wait(host->os.epollFd, evtList, EPOLL_EVT_LIST_SIZE, 0);
                continue;
            }
            break;
        }
        // if waked up by other host (by writing pipe), that means there must be content in the pipe, consume it
        if (signAtWakeUp != 0) {
            listCount = read(host->os.pipe[0], evtList, PAGE_SIZE);
        }
    }
}

static inline
void BlockAndDispatchIoNoRmtCheck(DzHost* host, int timeout)
{
    int listCount;
    DzFd* dzFd;
    struct epoll_event* evtList;
    struct epoll_event* itr;

    evtList = host->os.evtList;
    listCount = epoll_wait(host->os.epollFd, evtList, EPOLL_EVT_LIST_SIZE, timeout);
    if (listCount != 0) {
        while (1) {
            for (itr = evtList; itr != evtList + listCount; itr++) {
                dzFd = (DzFd*)itr->data.ptr;
                if (IsEasyEvtWaiting(&dzFd->inEvt) && (itr->events & EPOLLIN)) {
                    NotifyEasyEvt(host, &dzFd->inEvt);
                    CleanEasyEvt(&dzFd->inEvt);
                }
                if (IsEasyEvtWaiting(&dzFd->outEvt) && (itr->events & EPOLLOUT)) {
                    NotifyEasyEvt(host, &dzFd->outEvt);
                    CleanEasyEvt(&dzFd->outEvt);
                }
            }
            if (listCount == EPOLL_EVT_LIST_SIZE) {
                listCount = epoll_wait(host->os.epollFd, evtList, EPOLL_EVT_LIST_SIZE, 0);
                continue;
            }
            break;
        }
    }
}

#endif // __DzIoOs_h__
