/**
 *  @file       DzStructsArch.h
 *  @brief      for windows x86
 *  @author	    Foreverflying <foreverflying@live.cn>
 *  @date       2015/09/13
 *
 */

#ifndef __DzStructsArch_h__
#define __DzStructsArch_h__

#include "../../DzInc.h"
#include "../../DzDeclareStructs.h"

struct _DzStackBottom
{
    void*       stackLimit;
    void*       stackPtr;
    void*       exceptPtr;
    void*       dzCotEntry;     //use edi register
    DzHost*     host;           //use esi register
    void*       _unusedEbx;
    void*       _unusedEbp;
    void*       ipEntry;
    DzEntry     entry;
    intptr_t    context;
};

static inline
void InitExceptPtr( DzStackBottom* bottom, void* ptr )
{
    bottom->exceptPtr = ptr;
}

#endif // __DzStructsArch_h__