/**
 *  @file       DzIncOs.h
 *  @brief      for linux
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzIncOs_h__
#define __DzIncOs_h__

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

#if defined( __i386 )
#define DZ_ARCH_FILE( file )            MAKE_STR( x86/file )
#elif defined( __x86_64 )
#define DZ_ARCH_FILE( file )            MAKE_STR( x64/file )
#endif

typedef struct _DzIov
{
    void*           buf;
    size_t          len;
}DzIov;

typedef int BOOL;

#undef  TRUE
#undef  FALSE
#define TRUE    1
#define FALSE   0

typedef long long int64;


//global constants on linux
#define THREAD_STACK_MIN            ( PTHREAD_STACK_MIN )
#define DZ_PAGE_STACK_BOUNDARY      ( 16384 )
#define DZ_MIN_STACK_SIZE           ( 1024 )

//linux specific constants
#define EPOLL_EVT_LIST_SIZE         ( 1024 )

//switchers

#include DZ_ARCH_FILE( DzIncArch.h )

#endif // __DzIncOs_h__
