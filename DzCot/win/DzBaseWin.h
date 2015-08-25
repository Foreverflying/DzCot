/**
 *  @file       DzBaseWin.h
 *  @brief      
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/11/22
 *
 */

#ifndef __DzBaseWin_h__
#define __DzBaseWin_h__

#include "../DzStructs.h"

void __cdecl SysThreadMain( void* context );
void __fastcall DzSwitch( DzHost* host, DzCot* dzCot );

static inline
void* PageAlloc( size_t size )
{
    return VirtualAlloc( NULL, size, MEM_COMMIT, PAGE_READWRITE );
}

static inline
void* PageReserv( size_t size )
{
    return VirtualAlloc( NULL, size, MEM_RESERVE, PAGE_READWRITE );
}

static inline
void* PageCommit( void* p, size_t size )
{
    return VirtualAlloc( p, size, MEM_COMMIT, PAGE_READWRITE ) ? p : NULL;
}

static inline
void PageFree( void* p, size_t size )
{
    VirtualFree( p, 0, MEM_RELEASE );
}

static inline
void StartSystemThread( DzSysParam* param, int stackSize )
{
    _beginthread( SysThreadMain, stackSize, param );
}

static inline
void AwakeRemoteHost( DzHost* dstHost )
{
    PostQueuedCompletionStatus( dstHost->os.iocp, 0, 1, (LPOVERLAPPED)1 );
}

static inline
void InitSysAutoEvt( DzSysAutoEvt* sysEvt )
{
    sysEvt->event = CreateEvent( NULL, FALSE, FALSE, NULL );
}

static inline
void FreeSysAutoEvt( DzSysAutoEvt* sysEvt )
{
    CloseHandle( sysEvt->event );
}

static inline
void WaitSysAutoEvt( DzSysAutoEvt* sysEvt )
{
    WaitForSingleObject( sysEvt->event, INFINITE );
}

static inline
void NotifySysAutoEvt( DzSysAutoEvt* sysEvt )
{
    SetEvent( sysEvt->event );
}

static inline
int AtomReadInt( int volatile* val )
{
    return *val;
}

static inline
void AtomSetInt( int volatile* val, int set )
{
    *val = set;
}

static inline
void* AtomReadPtr( void* volatile* val )
{
    return *val;
}

static inline
void AtomSetPtr( void* volatile* val, void* set )
{
    *val = set;
}

static inline
int AtomIncInt( int volatile* val )
{
    return InterlockedIncrement( (LONG volatile*)val ) - 1;
}

static inline
int AtomDecInt( int volatile* val )
{
    return InterlockedDecrement( (LONG volatile*)val ) + 1;
}

static inline
int AtomAddInt( int volatile* val, int add )
{
    return InterlockedExchangeAdd( (LONG volatile*)val, (LONG)add );
}

static inline
int AtomSubInt( int volatile* val, int sub )
{
    return InterlockedExchangeAdd( (LONG volatile*)val, (LONG)( - sub ) );
}

static inline
int AtomOrInt( int volatile* val, int mask )
{
    return _InterlockedOr( (LONG volatile*)val, (LONG)mask );
}

static inline
int AtomAndInt( int volatile* val, int mask )
{
    return _InterlockedAnd( (LONG volatile*)val, (LONG)mask );
}

static inline
int AtomCasInt( int volatile* val, int cmp, int set )
{
    return InterlockedCompareExchange(
        (LONG volatile*)val, (LONG)set, (LONG)cmp
        );
}

static inline
void* AtomCasPtr( void* volatile* val, void* cmp, void* set )
{
    return (void*)InterlockedCompareExchangePointer(
        ( PVOID volatile*)val, (PVOID)set, (PVOID)cmp
        );
}

#ifdef STORE_HOST_IN_ARBITRARY_USER_POINTER

static inline
BOOL AllocTlsIndex()
{
    return TRUE;
}

static inline
void FreeTlsIndex()
{
}

#if defined( _X86_ )

static inline
DzHost* GetHost()
{
    return (DzHost*)__readfsdword( 20 );
}

static inline
void SetHost( DzHost* host )
{
    __writefsdword( 20, (DWORD)host );
}

#elif defined( _M_AMD64 )

static inline
DzHost* GetHost()
{
    return *(DzHost**)( __readgsqword( 0x30 ) + 40 );
}

static inline
void SetHost( DzHost* host )
{
    *(DzHost**)( __readgsqword( 0x30 ) + 40 ) = host;
}

#endif

#else

static inline
BOOL AllocTlsIndex()
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

static inline
void FreeTlsIndex()
{
    TlsFree( DZ_TLS_IDX );
}

static inline
DzHost* GetHost()
{
    return (DzHost*)TlsGetValue( DZ_TLS_IDX );
}

static inline
void SetHost( DzHost* host )
{
    TlsSetValue( DZ_TLS_IDX, host );
}

#endif  //STORE_HOST_IN_ARBITRARY_USER_POINTER

#if defined( _X86_ )

static inline
void* GetExceptPtr()
{
    return (void*)__readfsdword( 0 );
}

static inline
char* GetStackPtr()
{
    return (char*)__readfsdword( 4 );
}

#elif defined( _M_AMD64 )

static inline
void* GetExceptPtr()
{
    return NULL;
}

static inline
char* GetStackPtr()
{
    return (char*)( __readgsqword( 0x30 ) + 8 );
}

#endif

#endif // __DzBaseWin_h__
