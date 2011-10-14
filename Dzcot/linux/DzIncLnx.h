/********************************************************************
    created:    2010/02/11 22:02
    file:       DzIncLnx.h
    author:     Foreverflying
    purpose:
*********************************************************************/

#ifndef __DzIncLnx_h__
#define __DzIncLnx_h__

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <sys/timeb.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
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

//linux specific constants
#define EPOLL_EVT_LIST_SIZE     4096

//switchers

//debug switchers

#endif // __DzIncLnx_h__
