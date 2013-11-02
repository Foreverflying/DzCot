/**
 *  @file       DzIncLnx.h
 *  @brief      
 *  @author	    Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzIncLnx_h__
#define __DzIncLnx_h__

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <sys/timeb.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <assert.h>

#ifndef __cplusplus
#define inline static __inline
#endif

#if defined( __i386 )
#define __stdcall   __attribute__((stdcall))
#define __fastcall  __attribute__((fastcall))
#else
#define __stdcall
#define __fastcall
#endif

typedef struct _DzBuf
{
    void*           buf;
    size_t          len;
}DzBuf;

typedef enum{
    FALSE,
    TRUE
}BOOL;

typedef long long int64;


//global constants on linux
#define THREAD_STACK_MIN            ( PTHREAD_STACK_MIN )
#define DZ_PAGE_STACK_BOUNDARY      ( 16384 )
#define DZ_MIN_STACK_SIZE           ( 1024 )

//linux specific constants
#define EPOLL_EVT_LIST_SIZE         1024

//switchers

//for debug check
#define CHECK_RESERV_SIZE           ( 0 )

#endif // __DzIncLnx_h__
