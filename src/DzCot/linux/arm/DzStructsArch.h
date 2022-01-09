/**
 *  @file       DzStructsArch.h
 *  @brief      for linux arm32
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2016/04/07
 *
 */

#ifndef __DzStructsArch_h__
#define __DzStructsArch_h__

#include "../../DzInc.h"
#include "../../DzDeclareStructs.h"

struct _DzStackBottom
{
    void*       _unusedR4;
    void*       _unusedR5;
    void*       _unusedR6;
    void*       _unusedR7;
    void*       _unusedR8;
    void*       _unusedR9;
    void*       dzCotEntry;     // use r10 register
    void*       host;           // use r11 register
    void*       ipEntry;        // use lr register
    DzEntry     entry;
    intptr_t    context;
};

#endif // __DzStructsArch_h__