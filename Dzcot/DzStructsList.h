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
                intptr_t    context3;
            };
            intptr_t        context2;
        };
        DzDLItr             dlItr;
    };
    intptr_t                context1;
    intptr_t                content;
};

#endif // __DzStructsList_h__
