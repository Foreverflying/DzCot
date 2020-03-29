/**
 *  @file       DzList.h
 *  @brief      
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzList_h__
#define __DzList_h__

#include "DzStructsList.h"

#define MEMBER_BASE(memAddr, type, mem)\
    ((type*)((char*)(memAddr) - (size_t)(&((type*)0)->mem)))

static inline
void InitDList(DzDList* list)
{
    list->entry.prev = &list->entry;
    list->entry.next = &list->entry;
}

static inline
void AddDlItrToHead(DzDList* list, DzDlItr* dlItr)
{
    list->entry.next->prev = dlItr;
    dlItr->next = list->entry.next;
    dlItr->prev = &list->entry;
    list->entry.next = dlItr;
}

static inline
void AddDlItrToTail(DzDList* list, DzDlItr* dlItr)
{
    list->entry.prev->next = dlItr;
    dlItr->prev = list->entry.prev;
    dlItr->next = &list->entry;
    list->entry.prev = dlItr;
}

static inline
void EraseDlItr(DzDlItr* dlItr)
{
    dlItr->prev->next = dlItr->next;
    dlItr->next->prev = dlItr->prev;
}

static inline
void LinkDList(DzDlItr* left, DzDlItr* right)
{
    left->next = right;
    right->prev = left;
}

static inline
void InitSList(DzSList* list)
{
    list->head = NULL;
    list->tail = NULL;
}

static inline
void EraseListHead(DzSList* list)
{
    if (list->head == list->tail) {
        list->head = list->tail = NULL;
    } else {
        list->head = list->head->next;
    }
}

static inline
void AddLItrToHead(DzSList* list, DzLItr* lItr)
{
    if (!list->head) {
        list->head = list->tail = lItr;
    } else {
        lItr->next = list->head;
        list->head = lItr;
    }
}

static inline
void AddLItrToTail(DzSList* list, DzLItr* lItr)
{
    if (!list->tail) {
        list->head = list->tail = lItr;
    } else {
        list->tail->next = lItr;
        list->tail = lItr;
    }
}

static inline
BOOL IsSListEmpty(DzSList* list)
{
    return list->tail == NULL;
}

static inline
void AddLItrToEptSList(DzSList* list, DzLItr* lItr)
{
    list->head = lItr;
    list->tail = lItr;
}

static inline
void AddLItrToNonEptTail(DzSList* list, DzLItr* lItr)
{
    list->tail->next = lItr;
    list->tail = lItr;
}

static inline
DzLItr* GetChainAndResetSList(DzSList* list)
{
    DzLItr* ret = list->head;
    list->tail->next = NULL;
    list->head = NULL;
    list->tail = NULL;
    return ret;
}

#endif // __DzList_h__
