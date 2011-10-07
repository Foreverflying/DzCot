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

BOOL InitOsStruct( DzHost* host, DzHost* parentHost );
void DeleteOsStruct( DzHost* host, DzHost* parentHost );
BOOL AllocAsynIoPool( DzHost* host );

inline void InitDzThread( DzThread* dzThread )
{
}

inline void InitAsynIo( DzAsynIo* asynIo )
{
    InitFastEvt( &asynIo->inEvt );
    InitFastEvt( &asynIo->outEvt );
    asynIo->inEvt.dzThread = NULL;
    asynIo->outEvt.dzThread = NULL;
}

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
    DzRoutine   entry,
    void*       context
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
    void*       unused1,
    void*       unused2,
    void*       unused3,
    void*       unused4,
    void*       unused5,
    void*       unused6,
    DzRoutine   entry,
    void*       context
    );

#endif

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
    bottom->routineEntry = DzcotRoutine;
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
    dzThread->lastErr = 0;
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
