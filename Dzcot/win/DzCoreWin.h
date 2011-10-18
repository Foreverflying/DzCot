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
#include "../DzSynObj.h"

#ifdef __cplusplus
extern "C"{
#endif

void __stdcall CallDzcotEntry( void );
void __stdcall DzcotEntry(
    DzHost*             host,
    volatile DzRoutine* entryPtr,
    volatile intptr_t*  contextPtr
    );
BOOL InitOsStruct( DzHost* host, DzHost* firstHost );
void DeleteOsStruct( DzHost* host, DzHost* firstHost );
void CotScheduleCenter( DzHost* host );

inline void InitDzCot( DzHost* host, DzCot* dzCot )
{
    __DBG_INIT_INFO( DzCot, NULL, dzCot );
}

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
    void*       ipEntry;
    DzRoutine   entry;
    intptr_t    context;
    DzHost*     host;
};

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
    void*       ipEntry;
    DzRoutine   entry;
    intptr_t    context;
    DzHost*     host;
};

#endif

inline void SetCotEntry( DzCot* dzCot, DzRoutine entry, intptr_t context )
{
    ( ( (struct DzStackBottom*)dzCot->stack ) - 1 )->entry = entry;
    ( ( (struct DzStackBottom*)dzCot->stack ) - 1 )->context = context;
}

inline char* AllocStack( DzHost* host, int size )
{
    char* base;
    
    base = (char*)VirtualAlloc(
        NULL,
        size,
        MEM_RESERVE | MEM_TOP_DOWN,
        PAGE_READWRITE
        );

    return base ? base + size : NULL;
}

inline void FreeStack( char* stack, int size )
{
    char* base;
    
    base = stack - size;
    VirtualFree( base, 0, MEM_RELEASE );
}

inline char* CommitStack( char* stack, int size )
{
    void* tmp;
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

inline void InitCotStack( DzHost* host, DzCot* dzCot )
{
    struct DzStackBottom* bottom;

    bottom = ( (struct DzStackBottom*)dzCot->stack ) - 1;
    bottom->host = host;
    bottom->ipEntry = CallDzcotEntry;
#ifdef _X86_
    bottom->exceptPtr = host->osStruct.originExceptPtr;
#endif
    bottom->stackPtr = dzCot->stack;
    bottom->stackLimit = dzCot->stackLimit;

    dzCot->sp = bottom;
}

inline DzCot* InitCot( DzHost* host, DzCot* dzCot, int sSize )
{
    int size;
    
    size = DZ_STACK_UNIT_SIZE << ( sSize * DZ_STACK_SIZE_STEP );
    if( sSize <= DZ_MAX_PERSIST_STACK_SIZE ){
        dzCot->stackLimit = (char*)AllocChunk( host, size );
        if( !dzCot->stackLimit ){
            return NULL;
        }
        dzCot->stack = dzCot->stackLimit + size;
    }else{
        dzCot->stack = AllocStack( host, size );
        if( !dzCot->stack ){
            return NULL;
        }
        dzCot->stackLimit = CommitStack( dzCot->stack, PAGE_SIZE * 3 );
        if( !dzCot->stackLimit )
        {
            return NULL;
        }
    }
    dzCot->sSize = sSize;
    InitCotStack( host, dzCot );
    return dzCot;
}

inline void FreeCotStack( DzCot* dzCot )
{
    FreeStack( dzCot->stack, DZ_STACK_UNIT_SIZE << ( dzCot->sSize * DZ_STACK_SIZE_STEP ) );
}

#ifdef __cplusplus
};
#endif

#endif // __DzCoreWin_h__
