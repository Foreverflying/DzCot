/**
 *  @file       DzStructsArch.h
 *  @brief      for linux x86
 *  @author	    Foreverflying <foreverflying@live.cn>
 *  @date       2015/09/14
 *
 */

#ifndef __DzStructsArch_h__
#define __DzStructsArch_h__

#include "../../DzInc.h"
#include "../../DzDeclareStructs.h"

struct _DzStackBottom
{
    void*       dzCotEntry;     // use edi register
    void*       host;           // use esi register
    void*       _unusedEbx;
    void*       _unusedEbp;
    void*       ipEntry;
    DzEntry     entry;
    intptr_t    context;
};

#endif // __DzStructsArch_h__