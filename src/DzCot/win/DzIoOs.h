/**
 *  @file       DzIoOs.h
 *  @brief      for windows
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzIoOs_h__
#define __DzIoOs_h__

#include "../DzStructs.h"
#include "../DzBase.h"
#include "../DzResourceMgr.h"
#include "../DzSynObj.h"
#include "../DzDebug.h"

void __stdcall GetNameInfoEntry(intptr_t context);
void __stdcall GetNameInfoEntryW(intptr_t context);
void __stdcall GetAddrInfoEntry(intptr_t context);
void __stdcall GetAddrInfoEntryW(intptr_t context);

static inline
int Socket(DzHost* host, int domain, int type, int protocol)
{
    SOCKET s;
    DzFd* dzFd;

    s = socket(domain, type, protocol);
    if (s == INVALID_SOCKET) {
        __Dbg(SetLastErr)(host, WSAGetLastError());
        return -1;
    }
    CreateIoCompletionPort((HANDLE)s, host->os.iocp, (ULONG_PTR)NULL, 0);
    dzFd = CreateDzFd(host);
    dzFd->s = s;
    return (int)((intptr_t)dzFd - host->handleBase);
}

static inline
intptr_t RawSocket(DzHost* host, int hFd)
{
    DzFd* dzFd = (DzFd*)(host->handleBase + hFd);
    return dzFd->s;
}

static inline
int GetSockOpt(DzHost* host, int hFd, int level, int name, void* option, socklen_t* len)
{
    DzFd* dzFd = (DzFd*)(host->handleBase + hFd);
    return getsockopt(dzFd->s, level, name, (char*)option, len);
}

static inline
int SetSockOpt(DzHost* host, int hFd, int level, int name, const void* option, socklen_t len)
{
    DzFd* dzFd = (DzFd*)(host->handleBase + hFd);
    return setsockopt(dzFd->s, level, name, (const char*)option, len);
}

static inline
int GetSockName(DzHost* host, int hFd, struct sockaddr* addr, socklen_t* addrLen)
{
    DzFd* dzFd = (DzFd*)(host->handleBase + hFd);
    return getsockname(dzFd->s, addr, addrLen);
}

static inline
int GetPeerName(DzHost* host, int hFd, struct sockaddr* addr, socklen_t* addrLen)
{
    DzFd* dzFd = (DzFd*)(host->handleBase + hFd);
    return getpeername(dzFd->s, addr, addrLen);
}

static inline
int Bind(DzHost* host, int hFd, const struct sockaddr* addr, socklen_t addrLen)
{
    DzFd* dzFd = (DzFd*)(host->handleBase + hFd);
    dzFd->addrLen = addrLen;
    return bind(dzFd->s, addr, addrLen);
}

static inline
int Listen(DzHost* host, int hFd, int backlog)
{
    DzFd* dzFd = (DzFd*)(host->handleBase + hFd);
    return listen(dzFd->s, backlog);
}

static inline
int Shutdown(DzHost* host, int hFd, int how)
{
    DzFd* dzFd = (DzFd*)(host->handleBase + hFd);
    return shutdown(dzFd->s, how);
}

static inline
int TryConnectDatagram(SOCKET fd, const struct sockaddr* addr, socklen_t addrLen)
{
    int sockType;
    int sockTypeLen = sizeof(sockType);

    if (getsockopt(fd, SOL_SOCKET, SO_TYPE, (char*)&sockType, &sockTypeLen)) {
        return -1;
    }
    if (sockType == SOCK_DGRAM) {
        return connect(fd, addr, addrLen);
    }
    return -1;
}

static inline
int Connect(DzHost* host, int hFd, const struct sockaddr* addr, socklen_t addrLen)
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

    dzFd = (DzFd*)(host->handleBase + hFd);
    ZeroMemory(&helper.overlapped, sizeof(helper.overlapped));
    ZeroMemory(&tmpAddr, addrLen);
    tmpAddr.ss_family = addr->sa_family;
    bind(dzFd->s, (struct sockaddr*)&tmpAddr, addrLen);
    if (!host->readonly._ConnectEx(dzFd->s, addr, addrLen, NULL, 0, &bytes, &helper.overlapped)) {
        err = WSAGetLastError();
        if (err != ERROR_IO_PENDING) {
            if (err == WSAEINVAL) {
                if (TryConnectDatagram(dzFd->s, addr, addrLen) == 0) {
                    return 0;
                }
            }
            __Dbg(SetLastErr)(host, err);
            return -1;
        }
    }
    CloneDzFd(dzFd);
    WaitEasyEvt(host, &helper.easyEvt);
    if (dzFd->err) {
        __Dbg(SetLastErr)(host, dzFd->err);
        CloseDzFd(host, dzFd);
        return -1;
    }
    CloseDzFd(host, dzFd);
    ret = WSAGetOverlappedResult(
        dzFd->s,
        &helper.overlapped,
        &bytes,
        FALSE,
        &flag
       );
    if (!ret) {
        __Dbg(SetLastErr)(host, WSAGetLastError());
        return -1;
    }
    setsockopt(dzFd->s, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
    return 0;
}

static inline
int Accept(DzHost* host, int hFd, struct sockaddr* addr, socklen_t* addrLen)
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

    dzFd = (DzFd*)(host->handleBase + hFd);
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        return -1;
    }
    lAddrLen = dzFd->addrLen + 16;
    ZeroMemory(&helper.overlapped, sizeof(helper.overlapped));
    if (!host->readonly._AcceptEx(dzFd->s, s, buf, 0, lAddrLen, lAddrLen, &bytes, &helper.overlapped)) {
        err = WSAGetLastError();
        if (err != ERROR_IO_PENDING) {
            closesocket(s);
            __Dbg(SetLastErr)(host, err);
            return -1;
        }
    }
    CloneDzFd(dzFd);
    WaitEasyEvt(host, &helper.easyEvt);
    if (dzFd->err) {
        __Dbg(SetLastErr)(host, dzFd->err);
        CloseDzFd(host, dzFd);
        return -1;
    }
    CloseDzFd(host, dzFd);
    ret = WSAGetOverlappedResult(
        dzFd->s,
        &helper.overlapped,
        &bytes,
        FALSE,
        &flag
       );
    if (!ret) {
        closesocket(s);
        __Dbg(SetLastErr)(host, WSAGetLastError());
        return -1;
    }
    key = (ULONG_PTR)dzFd->s;
    setsockopt(s, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&key, sizeof(key));
    CreateIoCompletionPort((HANDLE)s, host->os.iocp, (ULONG_PTR)NULL, 0);
    if (addr) {
        host->readonly._GetAcceptExSockAddrs(buf, bytes, lAddrLen, lAddrLen, &lAddr, &lAddrLen, &rAddr, addrLen);
        memcpy(addr, rAddr, *addrLen);
    }
    dzFd = CreateDzFd(host);
    dzFd->s = s;
    return (int)((intptr_t)dzFd - host->handleBase);
}

static inline
int IovSend(DzHost* host, int hFd, DzIov* bufs, size_t bufCount, int flags)
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

    dzFd = (DzFd*)(host->handleBase + hFd);
    ZeroMemory(&helper.overlapped, sizeof(helper.overlapped));
    if (WSASend(dzFd->s, (WSABUF*)bufs, (DWORD)bufCount, &bytes, flags, &helper.overlapped, NULL)) {
        err = WSAGetLastError();
        if (err != ERROR_IO_PENDING) {
            __Dbg(SetLastErr)(host, err);
            return -1;
        }
    }
    CloneDzFd(dzFd);
    WaitEasyEvt(host, &helper.easyEvt);
    if (dzFd->err) {
        __Dbg(SetLastErr)(host, dzFd->err);
        CloseDzFd(host, dzFd);
        return -1;
    }
    CloseDzFd(host, dzFd);
    ret = WSAGetOverlappedResult(
        dzFd->s,
        &helper.overlapped,
        &bytes,
        FALSE,
        &tmpFlag
       );
    if (!ret) {
        __Dbg(SetLastErr)(host, WSAGetLastError());
        return -1;
    }
    return bytes;
}

static inline
int IovRecv(DzHost* host, int hFd, DzIov* bufs, size_t bufCount, int flags)
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

    dzFd = (DzFd*)(host->handleBase + hFd);
    ZeroMemory(&helper.overlapped, sizeof(helper.overlapped));
    tmpFlag = (DWORD)flags;
    if (WSARecv(dzFd->s, (WSABUF*)bufs, (DWORD)bufCount, &bytes, &tmpFlag, &helper.overlapped, NULL)) {
        err = WSAGetLastError();
        if (err != ERROR_IO_PENDING) {
            __Dbg(SetLastErr)(host, err);
            return -1;
        }
    }
    CloneDzFd(dzFd);
    WaitEasyEvt(host, &helper.easyEvt);
    if (dzFd->err) {
        __Dbg(SetLastErr)(host, dzFd->err);
        CloseDzFd(host, dzFd);
        return -1;
    }
    CloseDzFd(host, dzFd);
    ret = WSAGetOverlappedResult(
        dzFd->s,
        &helper.overlapped,
        &bytes,
        FALSE,
        &tmpFlag
       );
    if (!ret) {
        __Dbg(SetLastErr)(host, WSAGetLastError());
        return -1;
    }
    return bytes;
}

static inline
int Send(DzHost* host, int hFd, const void* buf, size_t len, int flags)
{
    DzIov tmpBuf;
    tmpBuf.len = (unsigned long)len;
    tmpBuf.buf = (void*)buf;

    return IovSend(host, hFd, &tmpBuf, 1, flags);
}

static inline
int Recv(DzHost* host, int hFd, void* buf, size_t len, int flags)
{
    DzIov tmpBuf;
    tmpBuf.len = (unsigned long)len;
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
    DzFd* dzFd;
    DWORD bytes;
    DzIoHelper helper;
    DWORD tmpFlag;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzIoHelper* helperPtr;

    dzFd = (DzFd*)(host->handleBase + hFd);
    ZeroMemory(&helper.overlapped, sizeof(helper.overlapped));
    if (WSASendTo(dzFd->s, (WSABUF*)bufs, (DWORD)bufCount, &bytes, flags, to, tolen, &helper.overlapped, NULL)) {
        err = WSAGetLastError();
        if (err != ERROR_IO_PENDING) {
            __Dbg(SetLastErr)(host, err);
            return -1;
        }
    }
    CloneDzFd(dzFd);
    WaitEasyEvt(host, &helper.easyEvt);
    if (dzFd->err) {
        __Dbg(SetLastErr)(host, dzFd->err);
        CloseDzFd(host, dzFd);
        return -1;
    }
    CloseDzFd(host, dzFd);
    ret = WSAGetOverlappedResult(
        dzFd->s,
        &helper.overlapped,
        &bytes,
        FALSE,
        &tmpFlag
       );
    if (!ret) {
        __Dbg(SetLastErr)(host, WSAGetLastError());
        return -1;
    }
    return bytes;
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
    DzFd* dzFd;
    DWORD bytes;
    DzIoHelper helper;
    DWORD tmpFlag;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzIoHelper* helperPtr;

    dzFd = (DzFd*)(host->handleBase + hFd);
    ZeroMemory(&helper.overlapped, sizeof(helper.overlapped));
    tmpFlag = (DWORD)flags;
    if (WSARecvFrom(dzFd->s, (WSABUF*)bufs, (DWORD)bufCount, &bytes, &tmpFlag, from, fromlen, &helper.overlapped, NULL)) {
        err = WSAGetLastError();
        if (err != ERROR_IO_PENDING) {
            __Dbg(SetLastErr)(host, err);
            return -1;
        }
    }
    CloneDzFd(dzFd);
    WaitEasyEvt(host, &helper.easyEvt);
    if (dzFd->err) {
        __Dbg(SetLastErr)(host, dzFd->err);
        CloseDzFd(host, dzFd);
        return -1;
    }
    CloseDzFd(host, dzFd);
    ret = WSAGetOverlappedResult(
        dzFd->s,
        &helper.overlapped,
        &bytes,
        FALSE,
        &tmpFlag
       );
    if (!ret) {
        __Dbg(SetLastErr)(host, WSAGetLastError());
        return -1;
    }
    return bytes;
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
    tmpBuf.len = (unsigned long)len;
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
    tmpBuf.len = (unsigned long)len;
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
    return inet_ntop(af, (PVOID)src, dst, (size_t)size);
}

static inline
DWORD GetFileFlag(int flags, DWORD* accessFlag)
{
    DWORD access = 0;
    DWORD createFlag = 0;
    DWORD rwFlag = 0;

    rwFlag = flags & 3;
    rwFlag++;
    if (rwFlag & 1) {
        access |= GENERIC_READ;
    }
    if (rwFlag & 2) {
        access |= GENERIC_WRITE;
    }
    if (flags & DZ_O_CREATE) {
        if (flags & DZ_O_EXCL) {
            createFlag = CREATE_NEW;
        } else if (flags & DZ_O_TRUNC) {
            createFlag = CREATE_ALWAYS;
        } else {
            createFlag = OPEN_ALWAYS;
        }
    } else {
        if (flags & DZ_O_TRUNC) {
            createFlag = TRUNCATE_EXISTING;
        } else {
            createFlag = OPEN_EXISTING;
        }
    }
    access &= ~SYNCHRONIZE;
    *accessFlag = access;
    return createFlag;
}

static inline
int GetFd(DzHost* host, HANDLE file, int flags)
{
    DzFd* dzFd;

    if (file == INVALID_HANDLE_VALUE) {
        return -1;
    }
    CreateIoCompletionPort(file, host->os.iocp, (ULONG_PTR)NULL, 0);
    if (flags & DZ_O_APPEND && GetFileType(file) == FILE_TYPE_DISK) {
        SetFilePointer(file, 0, 0, FILE_END);
    }
    dzFd = CreateDzFd(host);
    dzFd->notSock = TRUE;
    dzFd->isFile = GetFileType(file) == FILE_TYPE_DISK;
    dzFd->fd = file;
    return (int)((intptr_t)dzFd - host->handleBase);
}

static inline
int Open(DzHost* host, const char* fileName, int flags)
{
    DWORD access = 0;
    DWORD createFlag;
    HANDLE file;
    
    createFlag = GetFileFlag(flags, &access);
    file = CreateFileA(
        fileName,
        access,
        0,
        0,
        createFlag,
        FILE_FLAG_OVERLAPPED,
        NULL
       );
    return GetFd(host, file, flags);
}

static inline
ssize_t Read(DzHost* host, int hFd, void* buf, size_t count)
{
    DzFd* dzFd;
    DWORD bytes;
    DzIoHelper helper;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzIoHelper* helperPtr;

    dzFd = (DzFd*)(host->handleBase + hFd);
    ZeroMemory(&helper.overlapped, sizeof(helper.overlapped));
    if (dzFd->isFile) {
        helper.overlapped.OffsetHigh = 0;
        helper.overlapped.Offset = SetFilePointer(
            dzFd->fd,
            0,
            (PLONG)&helper.overlapped.OffsetHigh,
            FILE_CURRENT
           );
    }
    if (!ReadFile(dzFd->fd, buf, (DWORD)count, &bytes, &helper.overlapped)) {
        err = GetLastError();
        if (err != ERROR_IO_PENDING) {
            if (err == ERROR_HANDLE_EOF) {
                return 0;
            } else {
                __Dbg(SetLastErr)(host, (int)err);
                return -1;
            }
        }
    }
    CloneDzFd(dzFd);
    WaitEasyEvt(host, &helper.easyEvt);
    if (dzFd->err) {
        __Dbg(SetLastErr)(host, dzFd->err);
        CloseDzFd(host, dzFd);
        return -1;
    }
    CloseDzFd(host, dzFd);
    ret = GetOverlappedResult(
        dzFd->fd,
        &helper.overlapped,
        &bytes,
        FALSE
       );
    if (!ret) {
        err = GetLastError();
        if (err == ERROR_HANDLE_EOF) {
            return 0;
        } else {
            __Dbg(SetLastErr)(host, (int)err);
            return -1;
        }
    }
    if (dzFd->isFile) {
        SetFilePointer(dzFd->fd, (LONG)bytes, 0, FILE_CURRENT);
    }
    return bytes;
}

static inline
ssize_t Write(DzHost* host, int hFd, const void* buf, size_t count)
{
    DzFd* dzFd;
    DWORD bytes;
    DzIoHelper helper;
    DWORD err;
    BOOL ret;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzIoHelper* helperPtr;

    dzFd = (DzFd*)(host->handleBase + hFd);
    ZeroMemory(&helper.overlapped, sizeof(helper.overlapped));
    if (dzFd->isFile) {
        helper.overlapped.OffsetHigh = 0;
        helper.overlapped.Offset = SetFilePointer(
            dzFd->fd,
            0,
            (PLONG)&helper.overlapped.OffsetHigh,
            FILE_CURRENT
           );
    }
    if (!WriteFile(dzFd->fd, buf, (DWORD)count, &bytes, &helper.overlapped)) {
        err = GetLastError();
        if (err != ERROR_IO_PENDING) {
            if (err == ERROR_HANDLE_EOF) {
                return 0;
            } else {
                __Dbg(SetLastErr)(host, (int)err);
                return -1;
            }
        }
    }
    CloneDzFd(dzFd);
    WaitEasyEvt(host, &helper.easyEvt);
    if (dzFd->err) {
        __Dbg(SetLastErr)(host, dzFd->err);
        CloseDzFd(host, dzFd);
        return -1;
    }
    CloseDzFd(host, dzFd);
    ret = GetOverlappedResult(
        dzFd->fd,
        &helper.overlapped,
        &bytes,
        FALSE
       );
    if (!ret) {
        err = GetLastError();
        __Dbg(SetLastErr)(host, (int)err);
        if (err == ERROR_HANDLE_EOF) {
            return 0;
        } else {
            return -1;
        }
    }
    if (dzFd->isFile) {
        SetFilePointer(dzFd->fd, (LONG)bytes, 0, FILE_CURRENT);
    }
    return bytes;
}

static inline
size_t Seek(DzHost* host, int hFd, ssize_t offset, int whence)
{
    DzFd* dzFd = (DzFd*)(host->handleBase + hFd);
    size_t ret;

#if defined(_X86_)
    ret = (size_t)SetFilePointer(dzFd->fd, (long)offset, NULL, whence);
#elif defined(_M_AMD64)
    if (!SetFilePointerEx(dzFd->fd, *(LARGE_INTEGER*)&offset, (LARGE_INTEGER*)&ret, whence)) {
        return -1;
    }
#endif

    return ret;
}

static inline
size_t FileSize(DzHost* host, int hFd)
{
    DzFd* dzFd = (DzFd*)(host->handleBase + hFd);
    size_t ret;

#if defined(_X86_)
    ret = GetFileSize(dzFd->fd, NULL);
#elif defined(_M_AMD64)
    if (!GetFileSizeEx(dzFd->fd, (LARGE_INTEGER*)&ret)) {
        return -1;
    }
#endif

    return ret;
}

static inline
int DGetNameInfoW(
    DzHost*                 dzHost,
    const struct sockaddr*  sa,
    socklen_t               salen,
    wchar_t*                host,
    size_t                  hostlen,
    wchar_t*                serv,
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
    RunWorker(dzHost, GetNameInfoEntryW, (intptr_t)node);
    FreeLNode(dzHost, node);
    return ret;
}

static inline
int DGetAddrInfoW(
    DzHost*                 host,
    const wchar_t*          node,
    const wchar_t*          service,
    const struct addrinfoW* hints,
    struct addrinfoW**      res
   )
{
    int ret;
    DzLNode* param = AllocLNode(host);
    param->d1 = (intptr_t)node;
    param->d2 = (intptr_t)service;
    param->d3 = (intptr_t)hints;
    param->d4 = (intptr_t)res;
    param->d8 = (intptr_t)&ret;
    RunWorker(host, GetAddrInfoEntryW, (intptr_t)param);
    FreeLNode(host, param);
    return ret;
}

static inline
void DFreeAddrInfoW(struct addrinfoW *res)
{
    FreeAddrInfoW(res);
}

static inline
int DInetPtonW(int af, const wchar_t* src, void* dst)
{
    return InetPtonW(af, src, dst);
}

static inline
const wchar_t* DInetNtopW(int af, const void* src, wchar_t* dst, socklen_t size)
{
    return InetNtopW(af, (PVOID)src, dst, size);
}

static inline
int OpenW(DzHost* host, const wchar_t* fileName, int flags)
{
    DWORD access = 0;
    DWORD createFlag;
    HANDLE file;

    createFlag = GetFileFlag(flags, &access);
    file = CreateFileW(
        fileName,
        access,
        0,
        0,
        createFlag,
        FILE_FLAG_OVERLAPPED,
        NULL
       );
    return GetFd(host, file, flags);
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
    DzFd* dzFd = (DzFd*)(host->handleBase + hFd);
    BOOL ret;

    if (dzFd->notSock) {
        ret = CloseHandle(dzFd->fd) == S_OK ? 0 : -1;
        dzFd->notSock = FALSE;
        dzFd->isFile = FALSE;
    } else {
        ret = closesocket(dzFd->s);
    }
    dzFd->err = WSAECONNRESET;
    CloseDzFd(host, dzFd);
    return ret;
}

static inline
void BlockAndDispatchIo(DzHost* host, int timeout)
{
    ULONG_PTR key;
    DWORD n;
    OVERLAPPED* overlapped;
    DzIoHelper* helper;

    GetQueuedCompletionStatus(host->os.iocp, &n, &key, &overlapped, (DWORD)timeout);
    AtomOrInt(host->rmtCheckSignPtr, RMT_CHECK_AWAKE_SIGN);
    while (overlapped != NULL) {
        if (!key) {
            helper = MEMBER_BASE(overlapped, DzIoHelper, overlapped);
            NotifyEasyEvt(host, &helper->easyEvt);
        }
        GetQueuedCompletionStatus(host->os.iocp, &n, &key, &overlapped, 0);
    }

    /*
    DzIoHelper* helper;
    ULONG listCount;
    OVERLAPPED_ENTRY* list;
    OVERLAPPED_ENTRY* itr;

    list = host->os.overlappedEntryList;
    GetQueuedCompletionStatusEx(host->os.iocp, list, OVERLAPPED_ENTRY_LIST_SIZE, &listCount, (DWORD)timeout, FALSE);
    AtomOrInt(host->rmtCheckSignPtr, RMT_CHECK_AWAKE_SIGN);
    if (!listCount) {
        while (1) {
            for (itr = list; itr != list + listCount; itr++) {
                if (itr->lpOverlapped && itr->lpCompletionKey == 0) {
                    helper = MEMBER_BASE(itr->lpOverlapped, DzIoHelper, overlapped);
                    NotifyEasyEvt(host, &helper->easyEvt);
                }
            }
            if (listCount == OVERLAPPED_ENTRY_LIST_SIZE) {
                GetQueuedCompletionStatusEx(host->os.iocp, list, OVERLAPPED_ENTRY_LIST_SIZE, &listCount, (DWORD)0, FALSE);
                continue;
            }
            break;
        }
    }
    */
}

static inline
void BlockAndDispatchIoNoRmtCheck(DzHost* host, int timeout)
{
    /*
    DzIoHelper* helper;
    ULONG listCount;
    OVERLAPPED_ENTRY* list;
    OVERLAPPED_ENTRY* itr;

    list = host->os.overlappedEntryList;
    GetQueuedCompletionStatusEx(host->os.iocp, list, OVERLAPPED_ENTRY_LIST_SIZE, &listCount, (DWORD)timeout, TRUE);
    if (listCount) {
        // printf("=====NOTIFY: List Count is %d\n", listCount);
        while (1) {
            for (itr = list; itr != list + listCount; itr++) {
                if (itr->lpOverlapped && itr->lpCompletionKey == 0) {
                    helper = MEMBER_BASE(itr->lpOverlapped, DzIoHelper, overlapped);
                    // printf("=====NOTIFY: helper addr is %x\n", helper);
                    NotifyEasyEvt(host, &helper->easyEvt);
                }
            }
            if (listCount == OVERLAPPED_ENTRY_LIST_SIZE) {
                GetQueuedCompletionStatusEx(host->os.iocp, list, OVERLAPPED_ENTRY_LIST_SIZE, &listCount, (DWORD)0, TRUE);
                continue;
            }
            break;
        }
    }
    */
    ULONG_PTR key;
    DWORD n;
    OVERLAPPED* overlapped;
    DzIoHelper* helper;

    GetQueuedCompletionStatus(host->os.iocp, &n, &key, &overlapped, (DWORD)timeout);
    while (overlapped != NULL) {
        if (!key) {
            helper = MEMBER_BASE(overlapped, DzIoHelper, overlapped);
            NotifyEasyEvt(host, &helper->easyEvt);
        }
        GetQueuedCompletionStatus(host->os.iocp, &n, &key, &overlapped, 0);
    }
}

#endif // __DzIoOs_h__
