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

#ifdef SWITCH_COT_FLOAT_SAFE
void __fastcall DzSwitchFloatSafe( DzHost* host, DzThread* dzThread );
#define DzSwitch DzSwitchFloatSafe
#else
void __fastcall DzSwitchFast( DzHost* host, DzThread* dzThread );
#define DzSwitch DzSwitchFast
#endif

#ifdef __cplusplus
};
#endif

#endif // __DzBaseLnx_h__
