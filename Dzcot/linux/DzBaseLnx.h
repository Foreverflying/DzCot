/********************************************************************
    created:    2010/11/22 17:27
    file:       DzBaseLnx.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzBaseLnx_h__
#define __DzBaseLnx_h__

#include "../DzStructs.h"
#include "../../DzcotData/DzcotData.h"
#include <pthread.h>

//#define STORE_HOST_IN_SPECIFIC_POINTER

#define PAGE_SIZE                   4096
#define DZ_STACK_UNIT_SIZE          65536

#ifdef __cplusplus
extern "C"{
#endif

inline void* BaseAlloc( size_t size )
{
	return malloc( size );
}

#ifdef CONFIG_SMP
#define LOCK "lock ; "
#else
#define LOCK ""
#endif

inline int InterlockedExchange( volatile int* addr, int value )
{
	//TODO: rewrite with atom asm
	//	int ret = *addr;
	//	*addr = value;
	//	return ret;

	int ret;
	__asm__ __volatile__(
		LOCK "movl %3, %1; movl %2, %0"
		: "=m" (addr), "=ir" (ret)
		: "ir" (value), "m" (addr)
	);
	return ret;
}

inline void InitTlsIndex()
{
#ifdef STORE_HOST_IN_SPECIFIC_POINTER
#else
    if( tlsIndex == -1 ){
        while( InterlockedExchange( &tlsLock, 1 ) == 1 );
        if( *(volatile int*)&tlsIndex == -1 ){
            pthread_key_create( (pthread_key_t*)&tlsIndex, NULL );
        }
        InterlockedExchange( &tlsLock, 0 );
    }
#endif
}

inline DzHost* GetHost()
{
#ifdef STORE_HOST_IN_SPECIFIC_POINTER
#if defined( __i386 )
    return (DzHost*)__readfsdword( 20 );
#elif defined( __amd64 )
    return *(DzHost**)( __readgsqword( 0x30 ) + 40 );
#endif
#else
    return (DzHost*)pthread_getspecific( tlsIndex );
#endif
}

inline void SetHost( DzHost *host )
{
#ifdef STORE_HOST_IN_SPECIFIC_POINTER
#if defined( __i386 )
    __writefsdword( 20, (DWORD)host );
#elif defined( __amd64 )
    *(DzHost**)( __readgsqword( 0x30 ) + 40 ) = host;
#endif
#else
    pthread_setspecific( tlsIndex, host );
#endif
}

#ifdef __cplusplus
};
#endif

#endif // __DzBaseLnx_h__
