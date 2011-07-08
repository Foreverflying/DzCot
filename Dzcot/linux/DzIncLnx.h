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
#include <unistd.h>
#include <sys/timeb.h>
#include <sys/socket.h>
#include <sys/epoll.h>

typedef long long int64;

#ifndef __cplusplus
#define inline static __inline
#endif

#define __fastcall __attribute__((fastcall))

//#define STORE_HOST_IN_SPECIFIC_POINTER

#define EPOLL_EVT_LIST_SIZE     4096

#endif // __DzIncLnx_h__
