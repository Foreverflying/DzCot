/********************************************************************
    created:    2010/02/11 21:55
    file:       DzIncOs.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzIncOs_h__
#define __DzIncOs_h__

#ifdef _WIN32
    #include "win/DzIncWin.h"
#elif defined __linux__
    #include "linux/DzIncLnx.h"
#endif

//predefine
typedef void (__stdcall *DzRoutine)( intptr_t context );

#undef FALSE
#undef TRUE
#undef NULL

#define FALSE   0
#define TRUE    1
#define NULL    0

typedef int BOOL;

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


//global constants
#define DZ_TLS_IDX                  ( 16 )
#define PAGE_SIZE                   ( 4096 )
#define DZ_STACK_UNIT_SIZE          ( 4096 )
#define DZ_STACK_SIZE_STEP          ( 2 )
#define DZ_MAX_PERSIST_STACK_SIZE   ( SS_16K )
#define DZ_MAX_COT_POOL_DEPTH       ( 1024 * 1024 * 1024 )
#define MEMERY_POOL_GROW_SIZE       ( 16 * 1024 * 1024 )
#define OBJ_POOL_GROW_COUNT         ( PAGE_SIZE / sizeof( int ) )
#define TIME_HEAP_SIZE              ( 1024 * 1024 )
#define MIN_TIME_INTERVAL           ( 5 )
#define DZ_MAX_IOV                  ( 64 )


//debug switchers
#define __DBG_DEBUG_CHECK_MODE


#endif // __DzIncOs_h__
