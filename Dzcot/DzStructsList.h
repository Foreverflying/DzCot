/********************************************************************
    created:    2010/02/11 22:06
    file:       DzStructsList.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzStructsList_h__
#define __DzStructsList_h__

typedef struct _DzLItr
{
    struct _DzLItr*     next;
}DzLItr;

typedef struct _DzSList
{
    struct _DzLItr*     head;
    struct _DzLItr*     tail;
}DzSList;

typedef struct _DzDLItr
{
    struct _DzDLItr*    next;
    struct _DzDLItr*    prev;
}DzDLItr;

typedef struct _DzDList
{
    struct _DzDLItr     entry;
}DzDList;

typedef struct _DzLNode
{
    union{
        struct{
            union{
                DzLItr  lItr;
                void*   context3;
            };
            void*       context2;
        };
        DzDLItr         dlItr;
    };
    void*               context1;
    void*               content;
}DzLNode;

#endif // __DzStructsList_h__
