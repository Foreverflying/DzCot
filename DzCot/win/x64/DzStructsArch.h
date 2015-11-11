/**
 *  @file       DzStructsArch.h
 *  @brief      for windows x64
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
    void*       _unusedR15;
    void*       _unusedR14;
    void*       _unusedR13;
    void*       _unusedR12;
    void*       dzCotEntry;     //use RDI register
    DzHost*     host;           //use RSI register
    void*       _unusedRbx;
    void*       _unusedRbp;
    void*       ipEntry;
    DzEntry     entry;
    intptr_t    context;
};

static inline
void InitExceptPtr( DzStackBottom* bottom, void* ptr )
{
}

#endif // __DzStructsArch_h__