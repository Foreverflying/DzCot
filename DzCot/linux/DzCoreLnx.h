/**
 *  @file       DzCoreLnx.h
 *  @brief      
 *  @author	    Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzCoreLnx_h__
#define __DzCoreLnx_h__

#include "../DzBase.h"
#include "../DzResourceMgr.h"
#include "../DzSynObj.h"

#ifdef __cplusplus
extern "C"{
#endif

void __stdcall CallDzCotEntry( void );
BOOL InitOsStruct( DzHost* host );
void CleanOsStruct( DzHost* host );
DzCot* InitCot( DzHost* host, DzCot* dzCot, int sType );
void __stdcall DzCotEntry(
    DzHost*             host,
    DzEntry volatile*   entryPtr,
    intptr_t volatile*  contextPtr
    );

inline void InitDzCot( DzHost* host, DzCot* dzCot )
{
    __Dbg( InitDzCot )( host, dzCot );
}

inline void InitDzFd( DzFd* dzFd )
{
    CleanEasyEvt( &dzFd->inEvt );
    CleanEasyEvt( &dzFd->outEvt );
    dzFd->ref = 0;
    dzFd->isSock = TRUE;
}

inline DzFd* CreateDzFd( DzHost* host )
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

inline void CloneDzFd( DzFd* dzFd )
{
    dzFd->ref++;
}

inline void CloseDzFd( DzHost* host, DzFd* dzFd )
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

inline void SetCotEntry( DzCot* dzCot, DzEntry entry, intptr_t context )
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
    bottom->ipEntry = CallDzCotEntry;
    dzCot->sp = bottom;
}

inline void FreeCotStack( DzHost* host, DzCot* dzCot )
{
    FreeStack( dzCot->stack, host->cotStackSize[ dzCot->sType ] );
}

#ifdef __cplusplus
};
#endif

#endif // __DzCoreLnx_h__
