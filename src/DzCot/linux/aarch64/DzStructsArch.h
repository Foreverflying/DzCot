/**
 *  @file       DzStructsArch.h
 *  @brief      for linux arm64
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2022/01/09
 *
 */

#ifndef __DzStructsArch_h__
#define __DzStructsArch_h__

#include "../../DzInc.h"
#include "../../DzDeclareStructs.h"

struct _DzStackBottom
{
    void *_unusedX19;
    void *_unusedX20;
    void *_unusedX21;
    void *_unusedX22;
    void *_unusedX23;
    void *_unusedX24;
    void *_unusedX25;
    void *_unusedX26;
    void *_unusedX27;
    void *dzCotEntry; // use x28 register
    void *host;       // use x29 register
    void *ipEntry;    // use lr register
    DzEntry entry;
    intptr_t context;
};

#endif // __DzStructsArch_h__