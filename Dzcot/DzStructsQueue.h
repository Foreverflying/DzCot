
#ifndef _DZ_STRUCT_QUEUE_H_
#define _DZ_STRUCT_QUEUE_H_

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

#endif  //#ifndef _DZ_STRUCTS_QUEUE_H_