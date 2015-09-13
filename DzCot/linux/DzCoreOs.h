/**
 *  @file       DzCoreOs.h
 *  @brief      for linux
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzCoreOs_h__
#define __DzCoreOs_h__

#include "../DzBase.h"
#include "../DzResourceMgr.h"
#include "../DzSynObj.h"

void __stdcall CallDzCotEntry( void );
BOOL InitOsStruct( DzHost* host );
void CleanOsStruct( DzHost* host );
DzCot* InitCot( DzHost* host, DzCot* dzCot, int sType );

void __stdcall DzCotEntry(
    DzHost*             host,
    DzEntry volatile*   entryPtr,
    intptr_t volatile*  contextPtr
    );

static inline
void InitDzCot( DzHost* host, DzCot* dzCot )
{
    __Dbg( InitDzCot )( host, dzCot );
}

static inline
void InitDzFd( DzFd* dzFd )
{
    CleanEasyEvt( &dzFd->inEvt );
    CleanEasyEvt( &dzFd->outEvt );
    dzFd->ref = 0;
}

static inline
DzFd* CreateDzFd( DzHost* host )
{
    DzFd* dzFd;

    if( !host->dzFdPool ){
        if( !AllocDzFdPool( host ) ){
            return NULL;
        }
    }
    dzFd = MEMBER_BASE( host->dzFdPool, DzFd, lItr );
    host->dzFdPool = host->dzFdPool->next;
    dzFd->err = 0;
    dzFd->ref++;
    __Dbg( AllocFd )( host, dzFd );
    return dzFd;
}

static inline
void CloneDzFd( DzFd* dzFd )
{
    dzFd->ref++;
}

static inline
void CloseDzFd( DzHost* host, DzFd* dzFd )
{
    dzFd->ref--;
    if( dzFd->ref == 0 ){
        __Dbg( FreeFd )( host, dzFd );
        dzFd->lItr.next = host->dzFdPool;
        host->dzFdPool = &dzFd->lItr;
    }
}

#if defined( __i386 )

struct DzStackBottom
{
    void*       _unusedEdi;
    void*       _unusedEsi;
    void*       _unusedEbx;
    void*       _unusedEbp;
    void*       ipEntry;
    DzHost*     host;
    DzEntry     entry;
    intptr_t    context;
};

#elif defined( __amd64 )

struct DzStackBottom
{
    void*       _unusedR15;
    void*       _unusedR14;
    void*       _unusedR13;
    void*       _unusedR12;
    void*       _unusedRbx;
    void*       _unusedRbp;
    void*       ipEntry;
    DzHost*     host;
    DzEntry     entry;
    intptr_t    context;
};

#endif

static inline
void SetCotEntry( DzCot* dzCot, DzEntry entry, intptr_t context )
{
    ( ( (struct DzStackBottom*)dzCot->stack ) - 1 )->entry = entry;
    ( ( (struct DzStackBottom*)dzCot->stack ) - 1 )->context = context;
}

static inline
char* AllocStack( int size )
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

static inline
void FreeStack( char* stack, int size )
{
    munmap( stack - size, size );
}

static inline
void InitCotStack( DzHost* host, DzCot* dzCot )
{
    struct DzStackBottom* bottom;

    bottom = ( (struct DzStackBottom*)dzCot->stack ) - 1;
    bottom->host = host;
    bottom->ipEntry = CallDzCotEntry;
    dzCot->sp = bottom;
}

static inline
void FreeCotStack( DzHost* host, DzCot* dzCot )
{
    FreeStack( dzCot->stack, host->cotStackSize[ dzCot->sType ] );
}

#endif // __DzCoreOs_h__
