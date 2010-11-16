/********************************************************************
    created:    2010/02/11 22:06
    file:       DzStructsQueue.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzStructsQueue_h__
#define __DzStructsQueue_h__

typedef struct _DzQItr
{
    struct _DzQItr*     next;
}DzQItr;

typedef struct _DzQueue
{
    struct _DzQItr*     head;
    struct _DzQItr*     tail;
}DzQueue;

typedef struct _DzDqItr
{
    struct _DzDqItr*    next;
    struct _DzDqItr*    prev;
}DzDqItr;

typedef struct _DzDeque
{
    struct _DzDqItr     entry;
}DzDeque;

typedef struct _DzQNode
{
    union{
        struct{
            union{
                DzQItr  qItr;
                void*   context3;
            };
            void*       context2;
        };
        DzDqItr         dqItr;
    };
    void*               context1;
    void*               content;
}DzQNode;

#endif // __DzStructsQueue_h__
