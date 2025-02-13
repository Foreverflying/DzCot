/**
 *  @file       DzStructsArch.h
 *  @brief      for linux x64
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2015/09/14
 *
 */

#ifndef __DzStructsArch_h__
#define __DzStructsArch_h__

#include "../../DzInc.h"
#include "../../DzDeclareStructs.h"

struct _DzStackBottom
{
    void*       _unusedR15;
    void*       _unusedR14;
    void*       dzCotEntry;     // use r13 register
    void*       host;           // use r12 register
    void*       _unusedRbx;
    void*       _unusedRbp;
    void*       ipEntry;
    DzEntry     entry;
    intptr_t    context;
};

#endif // __DzStructsArch_h__