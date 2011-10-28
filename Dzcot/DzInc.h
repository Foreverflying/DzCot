/********************************************************************
    created:    2011/10/24 21:14
    file:       DzInc.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

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
#define DZ_STACK_UNIT_SIZE          ( 4096 )
#define DZ_STACK_SIZE_STEP          ( 2 )
#define DZ_MAX_COT_POOL_DEPTH       ( 1024 * 1024 * 1024 )
#define MEMERY_POOL_GROW_SIZE       ( 16 * 1024 * 1024 )
#define WORKER_STACK_SIZE           ( 65536 )
#define OBJ_POOL_GROW_COUNT         ( 1024 )
#define TIME_HEAP_SIZE              ( 1024 * 1024 )
#define DFT_SSIZE_POOL_DEPTH        ( 64 )
#define DZ_MAX_IOV                  ( 64 )
#define DZ_MAX_HOST                 ( 16 )
#define RMT_CHECK_SLEEP_SIGN        ( 1 << 31 )
#define RMT_CALL_FIFO_SIZE          ( 1024 )
#define MIN_TIME_INTERVAL           ( 5 )
#define SCHEDULE_COUNTDOWN          ( 64 )
#define LAZY_TIMER_INTERVAL         ( 8192 )

//predefine
typedef void (__stdcall *DzRoutine)( intptr_t context );

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
    COT_PRIORITY_COUNT,     //should not use this;
    CP_DEFAULT = COT_PRIORITY_COUNT
};

enum
{
    SS_FIRST,
    SS_4K = SS_FIRST,
    SS_16K,
    SS_64K,
    SS_256K,
    SS_1M,
    SS_4M,
    SS_16M,
    SS_64M,
    STACK_SIZE_COUNT,       //should not use this
    SS_DEFAULT = STACK_SIZE_COUNT
};

#define DZ_O_RD         0x0000
#define DZ_O_WR         0x0001
#define DZ_O_RDWR       0x0002
#define DZ_O_CREATE     0x0100
#define DZ_O_EXCL       0x0200
#define DZ_O_TRUNC      0x1000
#define DZ_O_APPEND     0x2000

#endif // __DzInc_h__
