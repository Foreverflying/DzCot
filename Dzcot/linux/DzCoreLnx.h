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

inline DzAsyncIo* CreateAsyncIo( DzHost* host )
{
    DzAsyncIo* asyncIo;

    if( !host->osStruct.asyncIoPool ){
        if( !AllocAsyncIoPool( host ) ){
            return NULL;
        }
    }
    asyncIo = MEMBER_BASE( host->osStruct.asyncIoPool, DzAsyncIo, lItr );
    host->osStruct.asyncIoPool = host->osStruct.asyncIoPool->next;
    asyncIo->err = 0;
    asyncIo->ref++;
    return asyncIo;
}

inline void CloneAsyncIo( DzAsyncIo* asyncIo )
{
    asyncIo->ref++;
}

inline void CloseAsyncIo( DzHost* host, DzAsyncIo* asyncIo )
{
    asyncIo->ref--;
    if( asyncIo->ref == 0 ){
        asyncIo->lItr.next = host->osStruct.asyncIoPool;
        host->osStruct.asyncIoPool = &asyncIo->lItr;
    }
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
    DzHost*     host;
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
    DzHost*     host;
};

#endif

inline void SetCotEntry( DzCot* dzCot, DzRoutine entry, intptr_t context )
{
    ( ( (struct DzStackBottom*)dzCot->stack ) - 1 )->entry = entry;
    ( ( (struct DzStackBottom*)dzCot->stack ) - 1 )->context = context;
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

inline void InitCotStack( DzHost* host, DzCot* dzCot )
{
    struct DzStackBottom* bottom;

    bottom = ( (struct DzStackBottom*)dzCot->stack ) - 1;
    bottom->host = host;
    bottom->ipEntry = CallDzcotEntry;
    dzCot->sp = bottom;
}

inline DzCot* InitCot( DzHost* host, DzCot* dzCot, int sSize )
{
    int size;

    size = DZ_STACK_UNIT_SIZE << ( sSize * DZ_STACK_SIZE_STEP );
    if( sSize <= DZ_MAX_PERSIST_STACK_SIZE ){
        dzCot->stack = (char*)AllocChunk( host, size );
        if( !dzCot->stack ){
            return NULL;
        }
        dzCot->stack += size;
    }else{
        dzCot->stack = AllocStack( size );
        if( !dzCot->stack ){
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

#endif // __DzCoreLnx_h__
