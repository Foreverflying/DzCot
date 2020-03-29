/**
 *  @file       DzCoreOs.h
 *  @brief      for windows
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzCoreOs_h__
#define __DzCoreOs_h__

#include "../DzBase.h"
#include "../DzResourceMgr.h"
#include "../DzSynObj.h"

void __stdcall CallDzCotEntry(void);
BOOL InitOsStruct(DzHost* host);
void CleanOsStruct(DzHost* host);
DzCot* InitCot(DzHost* host, DzCot* dzCot, int sType);

void __stdcall DzCotEntry(
    DzHost*             host,
    DzEntry volatile*   entryPtr,
    intptr_t volatile*  contextPtr
    );

static inline
void InitDzCot(DzHost* host, DzCot* dzCot)
{
    __Dbg(InitDzCot)(host, dzCot);
}

static inline
void InitDzFd(DzFd* dzFd)
{
    dzFd->ref = 0;
    dzFd->notSock = FALSE;
    dzFd->isFile = FALSE;
    dzFd->addrLen = 0;
}

static inline
DzFd* CreateDzFd(DzHost* host)
{
    DzFd* dzFd;

    if (!host->dzFdPool) {
        if (!AllocDzFdPool(host)) {
            return NULL;
        }
    }
    dzFd = MEMBER_BASE(host->dzFdPool, DzFd, lItr);
    host->dzFdPool = host->dzFdPool->next;
    dzFd->err = 0;
    dzFd->ref++;
    __Dbg(AllocFd)(host, dzFd);
    return dzFd;
}

static inline
void CloneDzFd(DzFd* dzFd)
{
    dzFd->ref++;
}

static inline
void CloseDzFd(DzHost* host, DzFd* dzFd)
{
    dzFd->ref--;
    if (dzFd->ref == 0) {
        __Dbg(FreeFd)(host, dzFd);
        dzFd->lItr.next = host->dzFdPool;
        host->dzFdPool = &dzFd->lItr;
    }
}

static inline
void SetCotEntry(DzCot* dzCot, DzEntry entry, intptr_t context)
{
    (((DzStackBottom*)dzCot->stack) - 1)->entry = entry;
    (((DzStackBottom*)dzCot->stack) - 1)->context = context;
}

static inline
char* AllocStack(DzHost* host, int size)
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

static inline
void FreeStack(char* stack, int size)
{
    VirtualFree(stack - size, 0, MEM_RELEASE);
}

static inline
char* CommitStack(char* stack, int size)
{
    void* tmp;
    BOOL ret;

    tmp = VirtualAlloc(
        stack - size,
        size,
        MEM_COMMIT,
        PAGE_READWRITE
        );
    if (!tmp) {
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

static inline
void InitCotStack(DzHost* host, DzCot* dzCot)
{
    DzStackBottom* bottom;

    bottom = ((DzStackBottom*)dzCot->stack) - 1;
    bottom->dzCotEntry = DzCotEntry;
    bottom->host = host;
    bottom->ipEntry = CallDzCotEntry;

    InitExceptPtr(bottom, host->os.originExceptPtr);
    bottom->stackPtr = dzCot->stack;
    bottom->stackLimit = dzCot->stackLimit;

    dzCot->sp = bottom;
}

static inline
void FreeCotStack(DzHost* host, DzCot* dzCot)
{
    FreeStack(dzCot->stack, host->cotStackSize[dzCot->sType]);
}

#endif // __DzCoreOs_h__
