/********************************************************************
    created:    2010/02/11 22:02
    file:       DzIncWin.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzIncWin_h__
#define __DzIncWin_h__

#ifndef WINVER
#define WINVER 0x0501
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <sys/timeb.h>
#include <MSWSock.h>
#include <windows.h>
#include <malloc.h>
#include <assert.h>

#ifndef __cplusplus
#define inline static __inline
#endif

typedef long long int64;


//global constants on windows

//windows specific constants

//switchers
#define STORE_HOST_IN_ARBITRARY_USER_POINTER
//#define SWITCH_COT_FLOAT_SAFE
//#define GENERATE_MINIDUMP_FOR_UNHANDLED_EXP

//debug switchers
//#define __DBG_CHECK_COT_STACK_OVERFLOW

#endif // __DzIncWin_h__
