/********************************************************************
    created:    2010/02/11 22:06
    file:       DzStructsList.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

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
    DzDLItr*    next;
    DzDLItr*    prev;
};

struct _DzDList
{
    DzDLItr     entry;
};

struct _DzLNode
{
    union{
        struct{
            union{
                DzLItr      lItr;
                intptr_t    d4;
            };
            intptr_t        d3;
        };
        DzDLItr             dlItr;
    };
    intptr_t                d2;
    intptr_t                d1;
};

#endif // __DzStructsList_h__
