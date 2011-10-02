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

#define DZ_TLS_IDX                  ( 16 )
#define PAGE_SIZE                   ( 4096 )
#define DZ_STACK_UNIT_SIZE          ( 4096 )
#define DZ_STACK_SIZE_STEP          ( 2 )
#define DZ_MAX_PERSIST_STACK_SIZE   ( SS_16K )
#define DZ_MAX_COT_POOL_DEPTH       ( 0x3fffffff )
#define MEMERY_POOL_GROW_SIZE       ( 16 * 1024 * 1024 )
#define OBJ_POOL_GROW_COUNT         ( PAGE_SIZE / sizeof( int ) )
#define TIME_HEAP_SIZE              ( 1024 * 1024 )
#define MIN_TIME_INTERVAL           ( 5 )
#define DZ_IOV_MAX                  ( 64 )

#endif // __DzIncOs_h__
