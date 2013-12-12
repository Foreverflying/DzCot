/**
 *  @file       DzIncWin.h
 *  @brief      
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzIncWin_h__
#define __DzIncWin_h__

#ifndef WINVER
#define WINVER 0x0600
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
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

#ifndef __cplusplus
#define inline static __inline
#endif

//predefine
#if defined( _X86_ )
typedef int ssize_t;
#elif defined( _AMD64_ )
typedef long long ssize_t;
#endif

typedef struct _DzBuf
{
    unsigned long   len;
    void*           buf;
}DzBuf;

typedef long long int64;


//global constants on windows
#define THREAD_STACK_MIN            ( 64 * 1024 )
#define DZ_PAGE_STACK_BOUNDARY      ( 65536 )

#ifdef _DEBUG
#define DZ_MIN_STACK_SIZE           ( 2048 )
#else
#define DZ_MIN_STACK_SIZE           ( 1024 )
#endif

//windows specific constants

//switchers
#define STORE_HOST_IN_ARBITRARY_USER_POINTER
//#define GENERATE_MINIDUMP_FOR_UNHANDLED_EXP

//for debug check
#if defined( _X86_ )
#define CHECK_RESERV_SIZE           ( PAGE_SIZE )
#elif defined( _AMD64_ )
#define CHECK_RESERV_SIZE           ( PAGE_SIZE * 2 )
#endif


#endif // __DzIncWin_h__
