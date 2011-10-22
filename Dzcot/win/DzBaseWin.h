/********************************************************************
    created:    2010/11/22 17:27
    file:       DzBaseWin.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzBaseWin_h__
#define __DzBaseWin_h__

#include "../DzStructs.h"

#ifdef __cplusplus
extern "C"{
#endif

void AwakeRemoteHost( DzHost* dstHost );
void __cdecl SysThreadEntry( void* context );
void __fastcall DzSwitch( DzHost* host, DzCot* dzCot );

inline void* PageAlloc( size_t size )
{
    return VirtualAlloc( NULL, size, MEM_COMMIT, PAGE_READWRITE );
}

inline void* PageReserv( size_t size )
{
    return VirtualAlloc( NULL, size, MEM_RESERVE, PAGE_READWRITE );
}

inline void* PageCommit( void* p, size_t size )
{
    return VirtualAlloc( p, size, MEM_COMMIT, PAGE_READWRITE );
}

inline void PageFree( void* p, size_t size )
{
    VirtualFree( p, 0, MEM_RELEASE );
}

inline void StartSystemThread( DzSysParam* param )
{
    _beginthread( SysThreadEntry, MIN_STACK_SIZE, param );
}

inline void AwakeRemoteHost( DzHost* dstHost )
{
    PostQueuedCompletionStatus( dstHost->osStruct.iocp, 0, 1, (LPOVERLAPPED)1 );
}

inline int AtomReadInt( volatile int* val )
{
    return *val;
}

inline int AtomIncInt( volatile int* val )
{
    return InterlockedIncrement( (volatile LONG*)val ) - 1;
}

inline int AtomDecInt( volatile int* val )
{
    return InterlockedDecrement( (volatile LONG*)val ) + 1;
}

inline int AtomAddInt( volatile int* val, int add )
{
    return InterlockedExchangeAdd( (volatile LONG*)val, (LONG)add );
}

inline int AtomSubInt( volatile int* val, int sub )
{
    return InterlockedExchangeAdd( (volatile LONG*)val, (LONG)( - sub ) );
}

inline int AtomOrInt( volatile int* val, int mask )
{
    return _InterlockedOr( (volatile LONG*)val, (LONG)mask );
}

inline int AtomAndInt( volatile int* val, int mask )
{
    return _InterlockedAnd( (volatile LONG*)val, (LONG)mask );
}

inline int AtomCasInt( volatile int* val, int cmp, int set )
{
    return InterlockedCompareExchange(
        (volatile LONG*)val, (LONG)set, (LONG)cmp
        );
}

inline void* AtomCasPtr( volatile void** val, void* cmp, void* set )
{
    return InterlockedCompareExchangePointer(
        (volatile PVOID*)val, (PVOID)set, (PVOID)cmp
        );
}

#ifdef STORE_HOST_IN_ARBITRARY_USER_POINTER

inline BOOL AllocTlsIndex()
{
    return TRUE;
}

inline void FreeTlsIndex()
{
}

#if defined( _X86_ )

inline DzHost* GetHost()
{
    return (DzHost*)__readfsdword( 20 );
}

inline void SetHost( DzHost* host )
{
    __writefsdword( 20, (DWORD)host );
}

#elif defined( _M_AMD64 )

inline DzHost* GetHost()
{
    return *(DzHost**)( __readgsqword( 0x30 ) + 40 );
}

inline void SetHost( DzHost* host )
{
    *(DzHost**)( __readgsqword( 0x30 ) + 40 ) = host;
}

#endif

#else

inline BOOL AllocTlsIndex()
{
    int i;
    DWORD tlsIndex;
    DWORD tlsArr[ DZ_TLS_IDX * 2 ];

    i = 0;
    tlsIndex = TlsAlloc();
    while( tlsIndex != DZ_TLS_IDX && i < DZ_TLS_IDX * 2 - 1 ){
        tlsArr[i] = tlsIndex;
        tlsIndex = TlsAlloc();
        i++;
    }
    i--;
    while( i >= 0 ){
        TlsFree( tlsArr[i] );
        i--;
    }
    if( tlsIndex != DZ_TLS_IDX ){
        TlsFree( tlsIndex );
        return FALSE;
    }else{
        return TRUE;
    }
}

inline void FreeTlsIndex()
{
    TlsFree( DZ_TLS_IDX );
}

inline DzHost* GetHost()
{
    return (DzHost*)TlsGetValue( DZ_TLS_IDX );
}

inline void SetHost( DzHost* host )
{
    TlsSetValue( DZ_TLS_IDX, host );
}

#endif  //STORE_HOST_IN_ARBITRARY_USER_POINTER

#if defined( _X86_ )

inline void* GetExceptPtr()
{
    return (void*)__readfsdword( 0 );
}

inline char* GetStackPtr()
{
    return (char*)__readfsdword( 4 );
}

#elif defined( _M_AMD64 )

inline void* GetExceptPtr()
{
    return NULL;
}

inline char* GetStackPtr()
{
    return (char*)( __readgsqword( 0x30 ) + 8 );
}

#endif


#ifdef __cplusplus
};
#endif

#endif // __DzBaseWin_h__