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

void __fastcall DzSwitch( DzHost* host, DzThread* dzThread );

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

#elif defined( _M_AMD64 )

inline void* GetExceptPtr()
{
    return NULL;
}

#endif


#ifdef __cplusplus
};
#endif

#endif // __DzBaseWin_h__