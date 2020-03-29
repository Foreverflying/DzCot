/**
 *  @file       DzDebug.h
 *  @brief      
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2011/10/06
 *
 */

#ifndef __DzDebug_h__
#define __DzDebug_h__

#include "DzInc.h"
#include "DzDeclareStructs.h"

#ifdef __DBG_DEBUG_CHECK_MODE
#define __Dbg(func)       __Dbg##func
#else
#define __Dbg(func)       __EmptyCall
#endif

static inline
int __EmptyCall(DzHost* host, ...)
{
    return 0;
}

void __DbgInitDzHost(DzHost* host);
void __DbgInitDzCot(DzHost* host, DzCot* dzCot);

int __DbgGetLastErr(DzHost* host);
void __DbgSetLastErr(DzHost* host, int err);

void __DbgMarkCurrStackForCheck(DzHost* host);
void __DbgCheckCotStackOverflow(DzHost* host, DzCot* dzCot);
int __DbgGetMaxStackUse(DzHost* host, int sType);

void __DbgAllocSynObj(DzHost* host, DzSynObj* obj);
void __DbgFreeSynObj(DzHost* host, DzSynObj* obj);
int __DbgGetSynObjLeak(DzHost* host);

void __DbgAllocFd(DzHost* host, DzFd* fd);
void __DbgFreeFd(DzHost* host, DzFd* fd);
int __DbgGetFdLeak(DzHost* host);

void __DbgAllocHeap(DzHost* host, void* p, size_t size);
void __DbgFreeHeap(DzHost* host, void* p);
int __DbgGetHeapLeak(DzHost* host);

void __DbgAllocParamNode(DzHost* host, DzParamNode* node);
void __DbgFreeParamNode(DzHost* host, DzParamNode* node);
int __DbgGetParamNodeLeak(DzHost* host);

#endif // __DzDebug_h__
