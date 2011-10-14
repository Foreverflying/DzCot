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
#include "../DzSynObj.h"

#ifdef __cplusplus
extern "C"{
#endif

BOOL AllocAsyncIoPool( DzHost* host );
void __stdcall CallDzcotEntry( void );
void __stdcall DzcotEntry(
    volatile DzRoutine* entryPtr,
    volatile intptr_t*  contextPtr
    );
BOOL InitOsStruct( DzHost* host, DzHost* parentHost );
void DeleteOsStruct( DzHost* host, DzHost* parentHost );
void CotScheduleCenter( DzHost* host );

inline void InitDzThread( DzThread* dzThread )
{
    __DBG_INIT_INFO( DzThread, NULL, dzThread );
}

#if defined( __i386 )

struct DzStackBottom
{
    void*       unusedEdi;
    void*       unusedEsi;
    void*       unusedEbx;
    void*       unusedEbp;
    void*       ipEntry;
    DzRoutine   entry;
    intptr_t    context;
};

#elif defined( __amd64 )

struct DzStackBottom
{
    void*       unusedR15;
    void*       unusedR14;
    void*       unusedR13;
    void*       unusedR12;
    void*       unusedRbx;
    void*       unusedRbp;
    void*       ipEntry;
    DzRoutine   entry;
    intptr_t    context;
};

#endif

inline void SetThreadEntry( DzThread* dzThread, DzRoutine entry, intptr_t context )
{
    ( ( (struct DzStackBottom*)dzThread->stack ) - 1 )->entry = entry;
    ( ( (struct DzStackBottom*)dzThread->stack ) - 1 )->context = context;
}

inline char* AllocStack( int size )
{
    char* base;

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

inline void FreeStack( char* stack, int size )
{
    munmap( stack - size, size );
}

inline void InitCotStack( DzHost* host, DzThread* dzThread )
{
    struct DzStackBottom* bottom;

    bottom = ( (struct DzStackBottom*)dzThread->stack ) - 1;
    bottom->ipEntry = CallDzcotEntry;
    dzThread->sp = bottom;
}

inline DzThread* InitCot( DzHost* host, DzThread* dzThread, int sSize )
{
    int size;

    size = DZ_STACK_UNIT_SIZE << ( sSize * DZ_STACK_SIZE_STEP );
    if( sSize <= DZ_MAX_PERSIST_STACK_SIZE ){
        dzThread->stack = (char*)AllocChunk( host, size );
        if( !dzThread->stack ){
            return NULL;
        }
        dzThread->stack += size;
    }else{
        dzThread->stack = AllocStack( size );
        if( !dzThread->stack ){
            return NULL;
        }
    }
    dzThread->sSize = sSize;
    InitCotStack( host, dzThread );
    return dzThread;
}

inline void FreeCotStack( DzThread* dzThread )
{
    FreeStack( dzThread->stack, DZ_STACK_UNIT_SIZE << ( dzThread->sSize * DZ_STACK_SIZE_STEP ) );
}

#ifdef __cplusplus
};
#endif

#endif // __DzCoreLnx_h__
