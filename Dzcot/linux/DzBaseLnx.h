/********************************************************************
    created:    2010/11/22 17:27
    file:       DzBaseLnx.h
    author:     Foreverflying
    purpose:
*********************************************************************/

#ifndef __DzBaseLnx_h__
#define __DzBaseLnx_h__

#include "../DzStructs.h"

#ifdef __cplusplus
extern "C"{
#endif

void* SysThreadEntry( void* context );
void __fastcall DzSwitch( DzHost* host, DzCot* dzCot );

inline void* PageAlloc( size_t size )
{
    return mmap( NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 );
}

inline void* PageReserv( size_t size )
{
    return PageAlloc( size );
}

inline void* PageCommit( void* p, size_t size )
{
    return p;
}

inline void PageFree( void* p, size_t size )
{
    munmap( p, size );
}

inline void StartSystemThread( DzSysParam* param )
{
    pthread_t tid;
    pthread_attr_t attr;

    pthread_attr_init( &attr );
    pthread_attr_setscope( &attr, PTHREAD_SCOPE_SYSTEM );
    pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );
    pthread_attr_setstacksize( &attr, PTHREAD_STACK_MIN );
    pthread_create( &tid, &attr, SysThreadEntry, param );
    pthread_attr_destroy( &attr );
}

inline void AwakeRemoteHost( DzHost* dstHost )
{
    char n = 0;
    write( dstHost->osStruct.pipe[1], &n, sizeof( n ) );
}

inline int AtomReadInt( volatile int* val )
{
    return *val;
}

inline int AtomIncInt( volatile int* val )
{
    return __sync_fetch_and_add( val, 1 );
}

inline int AtomDecInt( volatile int* val )
{
    return __sync_fetch_and_sub( val, 1 );
}

inline int AtomAddInt( volatile int* val, int add )
{
    return __sync_fetch_and_add( val, add );
}

inline int AtomSubInt( volatile int* val, int sub )
{
    return __sync_fetch_and_sub( val, sub );
}

inline int AtomOrInt( volatile int* val, int mask )
{
    return __sync_fetch_and_or( val, mask );
}

inline int AtomAndInt( volatile int* val, int mask )
{
    return __sync_fetch_and_and( val, mask );
}

inline int AtomCasInt( volatile int* val, int cmp, int set )
{
    return __sync_val_compare_and_swap( val, cmp, set );
}

inline void* AtomCasPtr( volatile void** val, void* cmp, void* set )
{
    return (void*)__sync_val_compare_and_swap( val, cmp, set );
}

inline BOOL AllocTlsIndex()
{
    int i;
    int ret;
    pthread_key_t tlsKey;
    pthread_key_t tlsArr[ DZ_TLS_IDX * 2 ];

    i = 0;
    ret = pthread_key_create( &tlsKey, NULL );
    if( ret != 0 ){
        tlsKey = ( pthread_key_t ) -1;
    }
    while( tlsKey != DZ_TLS_IDX && i < DZ_TLS_IDX * 2 - 1 ){
        tlsArr[i] = tlsKey;
        ret = pthread_key_create( &tlsKey, NULL );
        if( ret != 0 ){
            tlsKey = ( pthread_key_t ) -1;
        }
        i++;
    }
    i--;
    while( i >= 0 ){
        if( tlsArr[i] > 0 ){
            pthread_key_delete( tlsArr[i] );
        }
        i--;
    }
    if( tlsKey != DZ_TLS_IDX ){
        if( tlsKey > 0 ){
            pthread_key_delete( tlsKey );
        }
        return FALSE;
    }else{
        return TRUE;
    }
}

inline void FreeTlsIndex()
{
    pthread_key_delete( DZ_TLS_IDX );
}

inline DzHost* GetHost()
{
    return (DzHost*)pthread_getspecific( DZ_TLS_IDX );
}

inline void SetHost( DzHost* host )
{
    pthread_setspecific( DZ_TLS_IDX, host );
}

#ifdef __cplusplus
};
#endif

#endif // __DzBaseLnx_h__
