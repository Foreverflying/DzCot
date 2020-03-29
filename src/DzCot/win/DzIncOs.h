/**
 *  @file       DzIncOs.h
 *  @brief      for windows
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzIncOs_h__
#define __DzIncOs_h__

#ifndef WINVER
#define WINVER 0x0600
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <Ws2tcpip.h>
#include <sys/timeb.h>
#include <MSWSock.h>
#include <windows.h>
#include <process.h>
#include <intrin.h>
#include <malloc.h>
#include <assert.h>

// predefine
typedef struct _DzIov
{
    unsigned long   len;
    void*           buf;
}DzIov;

#define inline __inline

// global constants on windows
#define THREAD_STACK_MIN            (64 * 1024)
#define DZ_PAGE_STACK_BOUNDARY      (65536)

#ifdef _DEBUG
#define DZ_MIN_STACK_SIZE           (2048)
#else
#define DZ_MIN_STACK_SIZE           (1024)
#endif

// windows specific constants
#define OVERLAPPED_ENTRY_LIST_SIZE  (1024)

// switchers
#define STORE_HOST_IN_ARBITRARY_USER_POINTER
// #define GENERATE_MINIDUMP_FOR_UNHANDLED_EXP

#include "DzIncArch.h"

#endif // __DzIncOs_h__
