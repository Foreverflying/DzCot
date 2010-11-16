/********************************************************************
    created:    2010/02/11 22:03
    file:       DzQueue.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzQueue_h__
#define __DzQueue_h__

#include "DzStructsQueue.h"

#ifdef __cplusplus
extern "C"{
#endif

#define MEMBER_BASE( memAddr, type, mem )\
    ( (type*)( (char*)(memAddr) - (int)(&((type*)0)->mem ) ) )

inline void InitDeque( DzDeque *queue )
{
    queue->entry.prev = &queue->entry;
    queue->entry.next = &queue->entry;
}

inline void AddDqItrToHead( DzDeque *queue, DzDqItr *dqItr )
{
    queue->entry.next->prev = dqItr;
    dqItr->next = queue->entry.next;
    dqItr->prev = &queue->entry;
    queue->entry.next = dqItr;
}

inline void AddDqItrToTail( DzDeque *queue, DzDqItr *dqItr )
{
    queue->entry.prev->next = dqItr;
    dqItr->prev = queue->entry.prev;
    dqItr->next = &queue->entry;
    queue->entry.prev = dqItr;
}

inline void DequeueDqItr( DzDqItr *dqItr )
{
    dqItr->prev->next = dqItr->next;
    dqItr->next->prev = dqItr->prev;
}

inline void LinkDqItr( DzDqItr *left, DzDqItr *right )
{
    left->next = right;
    right->prev = left;
}

inline void InitQueue( DzQueue *queue )
{
    queue->head = NULL;
    queue->tail = NULL;
}

inline void AddQItrToHead( DzQueue *queue, DzQItr *qItr )
{
    if( !queue->head ){
        queue->head = queue->tail = qItr;
    }else{
        qItr->next = queue->head;
        queue->head = qItr;
    }
}

inline void AddQItrToTail( DzQueue *queue, DzQItr *qItr )
{
    if( !queue->tail ){
        queue->head = queue->tail = qItr;
    }else{
        queue->tail->next = qItr;
        queue->tail = qItr;
    }
}

inline void DelHeadQItr( DzQueue *queue )
{
    if( queue->head == queue->tail ){
        queue->head = queue->tail = NULL;
    }else{
        queue->head = queue->head->next;
    }
}

inline void PushQItr( DzQItr *head, DzQItr *qItr )
{
    qItr->next = head->next;
    head->next = qItr;
}

inline void PopQItr( DzQItr *head )
{
    head->next = head->next->next;
}

#ifdef __cplusplus
};
#endif

#endif // __DzQueue_h__
