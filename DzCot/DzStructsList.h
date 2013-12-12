/**
 *  @file       DzStructsList.h
 *  @brief      
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzStructsList_h__
#define __DzStructsList_h__

#include "DzDeclareStructs.h"

struct _DzLItr
{
    DzLItr*     next;
};

struct _DzSList
{
    DzLItr*     head;
    DzLItr*     tail;
};

struct _DzDLItr
{
    DzDlItr*    next;
    DzDlItr*    prev;
};

struct _DzDList
{
    DzDlItr     entry;
};

struct _DzLNode
{
    union{
        struct{
            union{
                struct{
                    union{
                        DzLItr      lItr;
                        intptr_t    d8;
                    };
                    intptr_t        d7;
                };
                DzDlItr             dlItr;
            };
            intptr_t                d6;
            intptr_t                d5;
            intptr_t                d4;
            intptr_t                d3;
            intptr_t                d2;
            intptr_t                d1;
        };
        DzCacheChunk                _padding;
    };
};

#endif // __DzStructsList_h__
