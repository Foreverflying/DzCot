/********************************************************************
    created:    2010/02/11 22:04
    file:       DzIoWin.c
    author:     Foreverflying
    purpose:    
********************************************************************/

#include "../DzInc.h"
#include "../DzCore.h"
#include "../DzIoOs.h"

#ifdef __cplusplus
extern "C"{
#endif

void __stdcall GetNameInfoEntryA( intptr_t context )
{
    DzLNode* node = (DzLNode*)context;

    *(int*)(node->d8) = GetNameInfoA(
        (const SOCKADDR*)node->d1,
        (socklen_t)node->d2,
        (PCHAR)node->d3,
        (DWORD)node->d4,
        (PCHAR)node->d5,
        (DWORD)node->d6,
        (INT)node->d7
        );
}

void __stdcall GetNameInfoEntryW( intptr_t context )
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

void __stdcall GetAddrInfoEntryA( intptr_t context )
{
    DzLNode* node = (DzLNode*)context;

    *(int*)(node->d8) = GetAddrInfoA(
        (PCSTR)node->d1,
        (PCSTR)node->d2,
        (const ADDRINFOA*)node->d3,
        (PADDRINFOA*)node->d4
        );
}

void __stdcall GetAddrInfoEntryW( intptr_t context )
{
    DzLNode* node = (DzLNode*)context;

    *(int*)(node->d8) = GetAddrInfoW(
        (PCWSTR)node->d1,
        (PCWSTR)node->d2,
        (const ADDRINFOW*)node->d3,
        (PADDRINFOW*)node->d4
        );
}

int DzOpenFileW( const wchar_t* fileName, int flags )
{
    DzHost* host = GetHost();
    assert( host );

    return OpenW( host, fileName, flags );
}

int DzGetNameInfoW(
    const struct sockaddr*  sa,
    int                     salen,
    wchar_t*                host,
    size_t                  hostlen,
    wchar_t*                serv,
    size_t                  servlen,
    int                     flags
    )
{
    DzHost* dzHost = GetHost();
    assert( dzHost );
    assert( dzHost->servMask & ( 1 << dzHost->hostId ) );

    return DGetNameInfoW( dzHost, sa, salen, host, (int)hostlen, serv, (int)servlen, flags );
}

int DzGetAddrInfoW(
    const wchar_t*          node,
    const wchar_t*          service,
    const struct addrinfoW* hints,
    struct addrinfoW**      res
    )
{
    DzHost* host = GetHost();
    assert( host );
    assert( host->servMask & ( 1 << host->hostId ) );

    return DGetAddrInfoW( host, node, service, hints, res );
}

void DzFreeAddrInfoW( struct addrinfoW *res )
{
    DFreeAddrInfoW( res );
}

int DzInetPtonW( int af, const wchar_t* src, void* dst )
{
    return DInetPtonW( af, src, dst );
}

const wchar_t* DzInetNtopW( int af, const void* src, wchar_t* dst, int size )
{
    return DInetNtopW( af, (PVOID)src, dst, size );
}

#ifdef __cplusplus
};
#endif
