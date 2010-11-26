/********************************************************************
    created:    2010/02/11 21:50
    file:       DzCoreLnx.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzCoreLnx_h__
#define __DzCoreLnx_h__

#include "../DzBaseOs.h"
#include "../DzResourceMgr.h"
#include "../../DzcotData/DzcotData.h"
#include <sys/resource.h>
#include <sys/mman.h>

#define __fastcall __attribute__((fastcall))

#ifdef __cplusplus
extern "C"{
#endif

void InitAsynIo( DzAsynIo* asynIo );
void InitDzThread( DzThread* dzThread, int sSize );

#ifdef SWITCH_COT_FLOAT_SAFE
#define DzSwitch DzSwitchFloatSafe
#else
void __fastcall DzSwitchFast( DzHost* host, DzThread* dzThread );
#define DzSwitch DzSwitchFast
#endif

#if defined( __i386 )

struct DzStackBottom
{
    void*       unusedEdi;
    void*       unusedEsi;
    void*       unusedEbx;
    void*       unusedEbp;
    void*       routineEntry;
    void*       unusedEip;
    DzRoutine   entry;
    void*       context;
};

void __stdcall DzcotRoutine(
    DzRoutine entry,
    void* context
    );

#elif defined( __amd64 )

struct DzStackBottom
{
    void*       unusedR15;
    void*       unusedR14;
    void*       unusedR13;
    void*       unusedR12;
    void*       unusedRbx;
    void*       unusedRbp;
    void*       routineEntry;
    void*       unusedEip;
    DzRoutine   entry;
    void*       context;
};

void DzcotRoutine(
    void* unused1,
    void* unused2,
    void* unused3,
    void* unused4,
    void* unused5,
    void* unused6,
    DzRoutine entry,
    void* context
    );

#endif

inline void InitOsStruct( DzHost* host )
{
    struct rlimit fdLimit;

    getrlimit( RLIMIT_NOFILE, &fdLimit );
    host->osStruct.maxFd = fdLimit.rlim_cur;
    host->osStruct.fdTable = BaseAlloc( sizeof(int) * host->osStruct.maxFd );
    host->osStruct.epollFd = epoll_create( host->osStruct.maxFd );
}

inline void SetThreadEntry( DzThread* dzThread, DzRoutine entry, void* context )
{
    struct DzStackBottom* bottom;

    bottom = ( (struct DzStackBottom*)dzThread->stack ) - 1;
    bottom->entry = entry;
    bottom->context = context;

    /*
    __asm{
        mov ecx, dzThread
            mov eax, [ecx] DzThread.stack
            mov edx, context;
        mov [eax-4], edx
            mov edx, entry
            mov [eax-8], edx
    }
    */
}

inline char* AllocStack( int sSize )
{
    size_t size;
    char* base;

    size = DZ_STACK_UNIT_SIZE << sSize;
    base = (char*)mmap(
        NULL,
        size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
        );

    if( base == MAP_FAILED ){
        return NULL;
    }
    return base + size;
}

inline void FreeStack( char* stack, int sSize )
{
    size_t size;
    char* base;

    size = DZ_STACK_UNIT_SIZE << sSize;
    base = stack - size;
    munmap( base, size );
}

inline void InitCotStack( DzHost* host, DzThread* dzThread )
{
    struct DzStackBottom* bottom;

    bottom = ( (struct DzStackBottom*)dzThread->stack ) - 1;
    bottom->routineEntry = DzcotRoutine;
    dzThread->sp = bottom;
}

inline DzThread* InitCot( DzHost* host, DzThread* dzThread, int sSize )
{
    if( !dzThread->stack ){
        dzThread->stack = AllocStack( sSize );
        if( !dzThread->stack ){
            return NULL;
        }
        host->poolCotCounts[ sSize ]++;
        InitCotStack( host, dzThread );
    }
    return dzThread;
}

#ifdef __cplusplus
};
#endif

#endif // __DzCoreLnx_h__
