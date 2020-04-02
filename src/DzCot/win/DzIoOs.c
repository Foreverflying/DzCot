/**
 *  @file       DzIoOs.c
 *  @brief      for windows
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#include "../DzInc.h"
#include "../DzCore.h"
#include "DzIoOs.h"

void __stdcall GetNameInfoEntry(intptr_t context)
{
    DzLNode* node = (DzLNode*)context;

    *(int*)(node->d8) = getnameinfo(
        (const SOCKADDR*)node->d1,
        (socklen_t)node->d2,
        (PCHAR)node->d3,
        (DWORD)node->d4,
        (PCHAR)node->d5,
        (DWORD)node->d6,
        (INT)node->d7
        );
}

void __stdcall GetNameInfoEntryW(intptr_t context)
{
    DzLNode* node = (DzLNode*)context;

    *(int*)(node->d8) = GetNameInfoW(
        (const SOCKADDR*)node->d1,
        (socklen_t)node->d2,
        (PWCHAR)node->d3,
        (DWORD)node->d4,
        (PWCHAR)node->d5,
        (DWORD)node->d6,
        (INT)node->d7
        );
}

void __stdcall GetAddrInfoEntry(intptr_t context)
{
    DzLNode* node = (DzLNode*)context;

    *(int*)(node->d8) = getaddrinfo(
        (PCSTR)node->d1,
        (PCSTR)node->d2,
        (const ADDRINFOA*)node->d3,
        (PADDRINFOA*)node->d4
        );
}

void __stdcall GetAddrInfoEntryW(intptr_t context)
{
    DzLNode* node = (DzLNode*)context;

    *(int*)(node->d8) = GetAddrInfoW(
        (PCWSTR)node->d1,
        (PCWSTR)node->d2,
        (const ADDRINFOW*)node->d3,
        (PADDRINFOW*)node->d4
        );
}

int DzGetNameInfoW(
    const struct sockaddr*  sa,
    socklen_t               salen,
    wchar_t*                host,
    size_t                  hostlen,
    wchar_t*                serv,
    size_t                  servlen,
    int                     flags
    )
{
    DzHost* dzHost = GetHost();
    assert(dzHost);

    return DGetNameInfoW(dzHost, sa, salen, host, (int)hostlen, serv, (int)servlen, flags);
}

int DzGetAddrInfoW(
    const wchar_t*          node,
    const wchar_t*          service,
    const struct addrinfoW* hints,
    struct addrinfoW**      res
    )
{
    DzHost* host = GetHost();
    assert(host);

    return DGetAddrInfoW(host, node, service, hints, res);
}

void DzFreeAddrInfoW(struct addrinfoW *res)
{
    DFreeAddrInfoW(res);
}

int DzInetPtonW(int af, const wchar_t* src, void* dst)
{
    return DInetPtonW(af, src, dst);
}

const wchar_t* DzInetNtopW(int af, const void* src, wchar_t* dst, socklen_t size)
{
    return DInetNtopW(af, (PVOID)src, dst, size);
}

int DzOpenW(const wchar_t* fileName, int flags)
{
    DzHost* host = GetHost();
    assert(host);

    return OpenW(host, fileName, flags);
}

void PopImmediatelySucceedOverlappedIo(DzHost* host, DzIoHelper* pHelper)
{
    DWORD bytes;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    DzIoHelper* helper;

    while (1) {
        GetQueuedCompletionStatus(host->os.iocp, &bytes, &key, &overlapped, 0);
        if (overlapped == &pHelper->overlapped) {
            host->scheduleCd--;
            if (!host->scheduleCd) {
                DispatchCurrCot(host);
            }
            return;
        }
        if (!key) {
            host->currPri = CP_FIRST;
            helper = MEMBER_BASE(overlapped, DzIoHelper, overlapped);
            NotifyEasyEvt(host, &helper->easyEvt);
        }
    }
}
