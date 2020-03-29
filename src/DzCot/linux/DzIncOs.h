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
#include <stdint.h>
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

// global constants on linux
#define THREAD_STACK_MIN            (PTHREAD_STACK_MIN)
#define DZ_PAGE_STACK_BOUNDARY      (16384)
#define DZ_MIN_STACK_SIZE           (1024)

// linux specific constants
#define EPOLL_EVT_LIST_SIZE         (1024)

// switchers

#include "DzIncArch.h"

#endif // __DzIncOs_h__
