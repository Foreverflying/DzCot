/********************************************************************
    created:    2011/10/24 21:14
    file:       DzInc.h
    author:     Foreverflying
    purpose:    
********************************************************************/

#ifndef __DzInc_h__
#define __DzInc_h__

#include "DzIncOs.h"

//debug switchers
#define __DBG_DEBUG_CHECK_MODE

//global constants
#define DZ_TLS_IDX                  ( 16 )
#define CPU_CACHE_ALIGN             ( 64 )
#define CPU_CACHE_ALIGN_MASK        ( CPU_CACHE_ALIGN - 1 )
#define PAGE_SIZE                   ( 4096 )
#define MIN_PERMANENT_CHUNK_SIZE    ( 1024 )
#define DZ_STACK_UNIT_SIZE          ( 1024 )
#define DZ_STACK_SIZE_STEP          ( 2 )
#define DZ_MAX_COT_POOL_DEPTH       ( 1024 * 1024 * 1024 )
#define DZ_MAX_WORKER_POOL_DEPTH    ( 1024 )
#define MEMERY_POOL_GROW_SIZE       ( 16 * 1024 * 1024 )
#define HANDLE_POOL_SIZE            ( 64 * 1024 * 1024 )
#define WORKER_STACK_SIZE           ( 65536 )
#define OBJ_POOL_GROW_COUNT         ( 1024 )
#define TIME_HEAP_SIZE              ( 1024 * 1024 )
#define DFT_SSIZE_POOL_DEPTH        ( 64 )
#define DZ_MAX_IOV                  ( 64 )
#define DZ_MAX_HOST                 ( 16 )
#define HANDLE_HOST_ID_MASK         ( DZ_MAX_HOST - 1 )
#define RMT_CHECK_AWAKE_SIGN        ( 1 << 31 )
#define RMT_CALL_FIFO_SIZE          ( 64 )
#define MIN_TIME_INTERVAL           ( 5 )
#define SCHEDULE_COUNTDOWN          ( 256 )
#define LAZY_TIMER_INTERVAL         ( 8192 )

//predefines
typedef void (__stdcall *DzEntry)( intptr_t context );

enum{
    RMT_PKG_SYNOBJ_EVT,
    RMT_PKG_EASY_EVT
};

enum
{
    DS_OK,
    DS_NO_MEMORY
};

enum
{
    CP_FIRST,
    CP_HIGH = CP_FIRST,
    CP_NORMAL,
    CP_LOW,
    COT_PRIORITY_COUNT,
    CP_DEFAULT = COT_PRIORITY_COUNT
};

enum
{
    SS_FIRST,
    SS_1K = SS_FIRST,
    SS_4K,
    SS_16K,
    SS_64K,
    SS_256K,
    SS_1M,
    SS_4M,
    SS_16M,
    STACK_SIZE_COUNT,
    SS_DEFAULT = STACK_SIZE_COUNT
};

enum
{
    DZ_O_RD         =   00000000,
    DZ_O_WR         =   00000001,
    DZ_O_RDWR       =   00000002,
    DZ_O_CREATE     =   00000100,
    DZ_O_EXCL       =   00000200,
    DZ_O_TRUNC      =   00001000,
    DZ_O_APPEND     =   00002000
};

#ifdef _DEBUG
#define SS_MIN      SS_4K
#else
#define SS_MIN      SS_1K
#endif

#endif // __DzInc_h__
