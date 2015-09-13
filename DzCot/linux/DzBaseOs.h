/**
 *  @file       DzBaseOs.h
 *  @brief      for linux
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/11/22
 *
 */

#ifndef __DzBaseOs_h__
#define __DzBaseOs_h__

#include "../DzStructs.h"

void* SysThreadMain( void* context );
void __fastcall DzSwitch( DzHost* host, DzCot* dzCot );

static inline
void* PageAlloc( size_t size )
{
    return mmap( NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 );
}

static inline
void* PageReserv( size_t size )
{
    return PageAlloc( size );
}

static inline
void* PageCommit( void* p, size_t size )
{
    return p;
}

static inline
void PageFree( void* p, size_t size )
{
    munmap( p, size );
}

static inline
void StartSystemThread( DzSysParam* param, int stackSize )
{
    pthread_t tid;
    pthread_attr_t attr;

    pthread_attr_init( &attr );
    pthread_attr_setscope( &attr, PTHREAD_SCOPE_SYSTEM );
    pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );
    pthread_attr_setstacksize( &attr, stackSize );
    pthread_create( &tid, &attr, SysThreadMain, param );
    pthread_attr_destroy( &attr );
}

static inline
void AwakeRemoteHost( DzHost* dstHost )
{
    int n;

    n = write( dstHost->os.pipe[1], &n, sizeof( n ) );
}

static inline
void InitSysAutoEvt( DzSysAutoEvt* sysEvt )
{
    sem_init( &sysEvt->sem, 0, 0 );
}

static inline
void FreeSysAutoEvt( DzSysAutoEvt* sysEvt )
{
    sem_destroy( &sysEvt->sem );
}

static inline
void WaitSysAutoEvt( DzSysAutoEvt* sysEvt )
{
    sem_wait( &sysEvt->sem );
}

static inline
void NotifySysAutoEvt( DzSysAutoEvt* sysEvt )
{
    sem_post( &sysEvt->sem );
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
    return __sync_fetch_and_add( val, 1 );
}

static inline
int AtomDecInt( int volatile* val )
{
    return __sync_fetch_and_sub( val, 1 );
}

static inline
int AtomAddInt( int volatile* val, int add )
{
    return __sync_fetch_and_add( val, add );
}

static inline
int AtomSubInt( int volatile* val, int sub )
{
    return __sync_fetch_and_sub( val, sub );
}

static inline
int AtomOrInt( int volatile* val, int mask )
{
    return __sync_fetch_and_or( val, mask );
}

static inline
int AtomAndInt( int volatile* val, int mask )
{
    return __sync_fetch_and_and( val, mask );
}

static inline
int AtomCasInt( int volatile* val, int cmp, int set )
{
    return __sync_val_compare_and_swap( val, cmp, set );
}

static inline
void* AtomCasPtr( void* volatile* val, void* cmp, void* set )
{
    return __sync_val_compare_and_swap( val, cmp, set );
}

static inline
BOOL AllocTlsIndex()
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

static inline
void FreeTlsIndex()
{
    pthread_key_delete( DZ_TLS_IDX );
}

static inline
DzHost* GetHost()
{
    return (DzHost*)pthread_getspecific( DZ_TLS_IDX );
}

static inline
void SetHost( DzHost* host )
{
    pthread_setspecific( DZ_TLS_IDX, host );
}

#endif // __DzBaseOs_h__
