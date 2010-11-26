/********************************************************************
    created:    2010/02/11 21:50
    file:       DzCoreWin.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzCoreWin_h__
#define __DzCoreWin_h__

#include "../DzBaseOs.h"
#include "../DzResourceMgr.h"
#include "../../DzcotData/DzcotData.h"

#ifdef __cplusplus
extern "C"{
#endif

void InitAsynIo( DzAsynIo *asynIo );
void InitDzThread( DzThread *dzThread, int sSize );
void __stdcall DzcotRoutine( DzRoutine entry, void* context );

#ifdef SWITCH_COT_FLOAT_SAFE
#define DzSwitch DzSwitchFloatSafe
#else
void __fastcall DzSwitchFast( DzHost *host, DzThread *dzThread );
#define DzSwitch DzSwitchFast
#endif

#if defined( _X86_ )

struct DzStackBottom
{
    void*       stackLimit;
    void*       stackPtr;
    void*       exceptPtr;
    void*       unusedEdi;
    void*       unusedEsi;
    void*       unusedEbx;
    void*       unusedEbp;
    void*       routineEntry;
    void*       unusedEip;
    DzRoutine   entry;
    void*       context;
};

#define DzcotRoutineEntry DzcotRoutine

#elif defined( _M_AMD64 )

struct DzStackBottom
{
    void*       stackLimit;
    void*       stackPtr;
    void*       unusedR15;
    void*       unusedR14;
    void*       unusedR13;
    void*       unusedR12;
    void*       unusedRdi;
    void*       unusedRsi;
    void*       unusedRbx;
    void*       unusedRbp;
    void*       routineEntry;
    void*       unusedEip;
    DzRoutine   entry;
    void*       context;
    void*       unusedR8;
    void*       unusedR9;
};

void CallDzcotRoutine();

#define DzcotRoutineEntry CallDzcotRoutine

#endif

inline void InitOsStruct( DzHost *host )
{
    host->osStruct.iocp = CreateIoCompletionPort(
        INVALID_HANDLE_VALUE,
        NULL,
        (ULONG_PTR)NULL,
        1
        );

#if defined( _X86_ )
    host->osStruct.originExceptPtr = (void*)__readfsdword( 0 );
    host->osStruct.originalStack = (char*)__readfsdword( 4 );
#elif defined( _M_AMD64 )
    host->osStruct.originExceptPtr = NULL;
    host->osStruct.originalStack = (char*)( __readgsqword( 0x30 ) + 8 );
#endif
    host->osStruct.reservedStack = NULL;
}

inline void SetThreadEntry( DzThread *dzThread, DzRoutine entry, void *context )
{
    struct DzStackBottom *bottom;

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
    char *base;
    DzQNode *node;
    DzHost *host = GetHost();

    while( 1 ){
        size = DZ_STACK_UNIT_SIZE << sSize;
        base = (char*)VirtualAlloc(
            NULL,
            size,
            MEM_RESERVE,
            PAGE_READWRITE
            );

        if( !base ){
            return NULL;
        }
        if( base < host->osStruct.originalStack ){
            node = AllocQNode( host );
            node->content = base;
            node->qItr.next = host->osStruct.reservedStack;
            host->osStruct.reservedStack = &node->qItr;
        }else{
            return base + size;
        }
    }
}

inline void FreeStack( char *stack, int sSize )
{
    size_t size;
    char *base;

    size = DZ_STACK_UNIT_SIZE << sSize;
    base = stack - size;
    VirtualFree( base, 0, MEM_RELEASE );
}

inline char* CommitStack( char *stack, size_t size )
{
    void *tmp;
    BOOL ret;

    tmp = VirtualAlloc(
        stack - size,
        size,
        MEM_COMMIT,
        PAGE_READWRITE
        );
    if( !tmp ){
        return NULL;
    }
    ret = VirtualProtect(
        stack - size,
        PAGE_SIZE,
        PAGE_READWRITE | PAGE_GUARD,
        (LPDWORD)&tmp
        );
    return ret ? stack - size + PAGE_SIZE : NULL;
}

inline void DeCommitStack( char *stack, char *stackLimit )
{
    size_t size;

    size = stack - stackLimit;
    VirtualFree( stackLimit, size, MEM_DECOMMIT );
}

inline void InitCotStack( DzHost *host, DzThread *dzThread )
{
    struct DzStackBottom *bottom;

    bottom = ( (struct DzStackBottom*)dzThread->stack ) - 1;
    bottom->routineEntry = DzcotRoutineEntry;
#ifdef _X86_
    bottom->exceptPtr = host->osStruct.originExceptPtr;
#endif
    bottom->stackPtr = dzThread->stack;
    bottom->stackLimit = dzThread->stackLimit;

    dzThread->sp = bottom;
}

inline DzThread* InitCot( DzHost *host, DzThread *dzThread, int sSize )
{
    if( !dzThread->stack ){
        dzThread->stack = AllocStack( sSize );
        if( !dzThread->stack ){
            return NULL;
        }
        host->poolCotCounts[ sSize ]++;
    }
    if( !dzThread->stackLimit ){
        dzThread->stackLimit = CommitStack( dzThread->stack, PAGE_SIZE * 3 );
        if( !dzThread->stackLimit )
        {
            return NULL;
        }
        InitCotStack( host, dzThread );
    }
    return dzThread;
}

#ifdef __cplusplus
};
#endif

#endif // __DzCoreWin_h__
