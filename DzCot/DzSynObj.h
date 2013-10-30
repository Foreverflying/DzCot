/********************************************************************
    created:    2010/02/11 22:07
    file:       DzSynObj.h
    author:     Foreverflying
    purpose:    
********************************************************************/

#ifndef __DzSynObj_h__
#define __DzSynObj_h__

#include "DzStructs.h"
#include "DzResourceMgr.h"
#include "DzTimer.h"
#include "DzSchedule.h"

#ifdef __cplusplus
extern "C"{
#endif

inline int StartCot(
    DzHost*     host,
    DzEntry     entry,
    intptr_t    context,
    int         priority,
    int         sType
    );

inline DzSynObj* AllocSynObj( DzHost* host )
{
    DzSynObj* obj;

    if( !host->synObjPool ){
        if( !AllocSynObjPool( host ) ){
            return NULL;
        }
    }
    obj = MEMBER_BASE( host->synObjPool, DzSynObj, lItr );
    host->synObjPool = host->synObjPool->next;
    return obj;
}

inline void FreeSynObj( DzHost* host, DzSynObj* obj )
{
    obj->lItr.next = host->synObjPool;
    host->synObjPool = &obj->lItr;
}

inline BOOL IsNotified( DzSynObj* obj )
{
    return obj->notifyCount > 0;
}

inline void WaitNotified( DzSynObj* obj )
{
    if( obj->type > TYPE_MAX_WAIT_AFFECT ){
        return;
    }
    if( obj->type == TYPE_SEM ){
        obj->notifyCount--;
    }else{
        obj->notifyCount = 0;
    }
}

inline void AppendToWaitQ( DzDList* queue, DzWaitNode* node )
{
    AddDLItrToTail( queue, &node->dlItr );
}

inline void ClearWait( DzHost* host, DzWaitHelper* helper )
{
    int i;

    for( i = 0; i < helper->waitCount; i++ ){
        EraseDLItr( &helper->nodeArray[i].dlItr );
    }
    if( IsTimeNodeInHeap( &helper->timeout.timerNode ) ){
        RemoveTimer( host, &helper->timeout.timerNode );
    }
}

inline BOOL NotifyWaitQueue( DzHost* host, DzSynObj* obj )
{
    DzDList* queue;
    DzDLItr* dlItr;
    DzWaitNode* node;
    DzWaitNode* head;
    int priority;
    int i;
    BOOL ret;
    
    ret = FALSE;
    for( priority = CP_FIRST; priority <= host->lowestPri; priority++ ){
        queue = &obj->waitQ[ priority ];
        dlItr = queue->entry.next;
        while( dlItr != &queue->entry ){
            node = MEMBER_BASE( dlItr, DzWaitNode, dlItr );
            dlItr = dlItr->next;
            if( node->helper->checkIdx < 0 ){
                WaitNotified( obj );
                ClearWait( host, node->helper );
                DispatchCot( host, node->helper->dzCot );
                node->helper->checkIdx = (int)( node - node->helper->nodeArray );
                ret = TRUE;
                if( !IsNotified( obj ) ){
                    return ret;
                }
            }else{
                head = node->helper->nodeArray;
                if( head + node->helper->checkIdx == node ){
                    i = node->helper->checkIdx + 1;
                    while(
                        i < node->helper->waitCount &&
                        IsNotified( head[i].synObj )
                        ){
                        i++;
                    }
                    if( i == node->helper->waitCount ){
                        i = node->helper->checkIdx - 1;
                        while(
                            i >= 0 &&
                            IsNotified( head[i].synObj )
                            ){
                            i--;
                        }
                    }
                    if( i >= 0 ){
                        node->helper->checkIdx = i;
                    }else{
                        for( i = 0; i < node->helper->waitCount; i++ ){
                            WaitNotified( head[i].synObj );
                        }
                        ClearWait( host, node->helper );
                        DispatchCot( host, node->helper->dzCot );
                        node->helper->checkIdx = 0;
                        ret = TRUE;
                        if( !IsNotified( obj ) ){
                            return ret;
                        }
                    }
                }
            }
        }
    }
    return ret;
}

inline DzSynObj* CreateManualEvt( DzHost* host, BOOL notified )
{
    DzSynObj* obj;

    obj = AllocSynObj( host );
    if( !obj ){
        return NULL;
    }
    obj->type = TYPE_EVT_MANUAL;
    obj->notifyCount = notified ? 1 : 0;
    obj->ref++;
    return obj;
}

inline DzSynObj* CreateAutoEvt( DzHost* host, BOOL notified )
{
    DzSynObj* obj;

    obj = AllocSynObj( host );
    if( !obj ){
        return NULL;
    }
    obj->type = TYPE_EVT_AUTO;
    obj->notifyCount = notified ? 1 : 0;
    obj->ref++;
    return obj;
}

inline DzSynObj* CreateCdEvt( DzHost* host, int count )
{
    DzSynObj* obj;

    obj = AllocSynObj( host );
    if( !obj ){
        return NULL;
    }
    obj->type = TYPE_EVT_COUNT;
    obj->notifyCount = 1 - count;
    obj->ref++;
    return obj;
}

inline void SetEvt( DzHost* host, DzSynObj* obj )
{
    if( obj->notifyCount > 0 ){
        return;
    }
    obj->notifyCount++;
    if( obj->notifyCount <= 0 ){
        return;
    }
    if( NotifyWaitQueue( host, obj ) ){
        host->currPri = CP_FIRST;
    }
}

inline void ResetEvt( DzSynObj* evt )
{
    if( evt->type == TYPE_EVT_COUNT ){
        evt->notifyCount--;
    }else{
        evt->notifyCount = 0;
    }
}

inline DzSynObj* CreateSem( DzHost* host, int count )
{
    DzSynObj* obj;

    obj = AllocSynObj( host );
    if( !obj ){
        return NULL;
    }
    obj->type = TYPE_SEM;
    obj->notifyCount = count;
    obj->ref++;
    return obj;
}

inline int ReleaseSem( DzHost* host, DzSynObj* sem, int count )
{
    if( sem->notifyCount > 0 ){
        sem->notifyCount += count;
        return sem->notifyCount;
    }
    sem->notifyCount += count;
    if( NotifyWaitQueue( host, sem ) ){
        host->currPri = CP_FIRST;
    }
    return sem->notifyCount;
}

inline DzSynObj* CreateTimer( DzHost* host, int milSec, BOOL repeat )
{
    DzSynObj* obj;

    obj = AllocSynObj( host );
    if( !obj ){
        return NULL;
    }
    obj->type = TYPE_TIMER;
    obj->timerNode.timestamp = MilUnixTime( host ) + milSec;
    obj->timerNode.interval = - milSec;
    obj->timerNode.repeat = repeat;
    obj->ref++;
    AddTimer( host, &obj->timerNode );
    return obj;
}

inline DzSynObj* CreateCallbackTimer(
    DzHost*         host,
    int             milSec,
    BOOL            repeat,
    DzEntry         callback,
    intptr_t        context,
    int             priority,
    int             sType
    )
{
    DzSynObj* obj;

    obj = AllocSynObj( host );
    if( !obj ){
        return NULL;
    }
    obj->type = TYPE_CALLBACK_TIMER;
    obj->timerNode.index = -1;
    obj->timerNode.timestamp = MilUnixTime( host ) + milSec;
    obj->timerNode.interval = - milSec;
    obj->timerNode.repeat = repeat;
    obj->routine = callback;
    obj->context = context;
    obj->priority = priority;
    obj->sType = sType;
    obj->ref++;
    host->cotCount++;

    AddTimer( host, &obj->timerNode );
    return obj;
}

inline void CloseCallbackTimer( DzHost* host, DzSynObj* timer )
{
    if( IsTimeNodeInHeap( &timer->timerNode ) ){
        RemoveTimer( host, &timer->timerNode );
    }
    host->cotCount--;
    timer->routine = NULL;
    timer->ref--;
    if( timer->ref == 0 ){
        InitDList( &timer->waitQ[0] );
        InitDList( &timer->waitQ[1] );
        FreeSynObj( host, timer );
    }
}

inline BOOL NotifyTimerNode( DzHost* host, DzTimerNode* timerNode )
{
    BOOL ret;
    DzFastEvt* fastEvt;
    DzSynObj* timer;
    DzCot* dzCot;

    switch( timerNode->type ){
    case TYPE_TIMER:
        timer = MEMBER_BASE( timerNode, DzSynObj, timerNode );
        //since DzTimerNode.interval and DzTimer.notifyCount is union,
        //DzTimer.notifyCount > 0 means not notified.
        //For DzTimerNode.interval is always a negative number,
        //when notifying Timer, DzTimerNode.notifyCount should keep positive
        timer->notifyCount = - timer->notifyCount;
        ret = NotifyWaitQueue( host, timer );
        //if a timer can be notified repeatedly, it is still in timer heap.
        //so we set it to NOT notified, or else, we keep it notified.
        if( IsTimeNodeInHeap( timerNode ) ){
            timer->notifyCount = - timer->notifyCount;
        }
        return ret;

    case TYPE_CALLBACK_TIMER:
        timer = MEMBER_BASE( timerNode, DzSynObj, timerNode );
        timer->ref++;
        StartCot( host, CallbackTimerEntry, (intptr_t)timer, timer->priority, timer->sType );
        return TRUE;

    case TYPE_TIMEOUT:
        fastEvt = MEMBER_BASE( timerNode, DzFastEvt, timerNode );
        fastEvt->helper->checkIdx = -1;
        ClearWait( host, fastEvt->helper );
        DispatchCot( host, fastEvt->helper->dzCot );
        return TRUE;

    case TYPE_FAST_EVT:
        fastEvt = MEMBER_BASE( timerNode, DzFastEvt, timerNode );
        fastEvt->status = -1;
        dzCot = fastEvt->dzCot;
        fastEvt->dzCot = NULL;
        DispatchCot( host, dzCot );
        return TRUE;
    }
    return TRUE;
}

inline DzSynObj* CloneSynObj( DzSynObj* obj )
{
    obj->ref++;
    return obj;
}

inline void DelSynObj( DzHost* host, DzSynObj* obj )
{
    obj->ref--;
    if( obj->ref == 0 ){
        if( obj->type == TYPE_TIMER && IsTimeNodeInHeap( &obj->timerNode ) ){
            RemoveTimer( host, &obj->timerNode );
        }
        FreeSynObj( host, obj );
    }
}

inline void InitTimeOut( DzFastEvt* timeout, int64 timestamp, DzWaitHelper* helper )
{
    timeout->type = TYPE_TIMEOUT;
    timeout->timerNode.repeat = FALSE;
    timeout->timerNode.timestamp = timestamp;
    timeout->helper = helper;
    timeout->timerNode.index = -1;
}

inline int WaitSynObj( DzHost* host, DzSynObj* obj, int timeout )
{
    DzWaitHelper helper;
    DzWaitNode waitNode;

    if( IsNotified( obj ) ){
        WaitNotified( obj );
        return 0;
    }
    if( timeout == 0 ){
        return -1;
    }
    helper.nodeArray = &waitNode;
    helper.waitCount = 1;
    helper.checkIdx = -1;
    helper.nodeArray[0].helper = &helper;
    helper.dzCot = host->currCot;
    if( timeout > 0 ){
        InitTimeOut( &helper.timeout, MilUnixTime( host ) + timeout, &helper );
        AddTimer( host, &helper.timeout.timerNode );
    }else{
        helper.timeout.timerNode.index = -1;
    }
    AppendToWaitQ( &obj->waitQ[ host->currCot->priority ], &helper.nodeArray[0] );
    Schedule( host );
    return helper.checkIdx;
}

inline int WaitMultiSynObj( DzHost* host, int count, DzSynObj** objs, BOOL waitAll, int timeout )
{
    DzWaitHelper helper;
    DzWaitNode* node;
    int i;

    if( waitAll ){
        for( i = 0; i < count; i++ ){
            if( !IsNotified( objs[i] ) ){
                break;
            }
        }
        if( i == count ){
            for( i = 0; i < count; i++ ){
                WaitNotified( objs[i] );
            }
            return 0;
        }
    }else{
        for( i = 0; i < count; i++ ){
            if( IsNotified( objs[i] ) ){
                WaitNotified( objs[i] );
                return i;
            }
        }
    }
    if( timeout == 0 ){
        return -1;
    }
    helper.waitCount = count;
    helper.dzCot = host->currCot;
    helper.nodeArray = (DzWaitNode*)alloca( sizeof(DzWaitNode) * count );
    helper.checkIdx = i == count ? -1 : i;
    for( node = helper.nodeArray; node != helper.nodeArray + count; node++ ){
        node->helper = &helper;
        node->synObj = *objs;
        AppendToWaitQ( &node->synObj->waitQ[ host->currCot->priority ], node );
        objs++;
    }
    if( timeout > 0 ){
        InitTimeOut( &helper.timeout, MilUnixTime( host ) + timeout, &helper );
        AddTimer( host, &helper.timeout.timerNode );
    }else{
        helper.timeout.timerNode.index = -1;
    }
    Schedule( host );
    return helper.checkIdx;
}

inline void InitFastEvt( DzFastEvt* fastEvt )
{
    fastEvt->type = TYPE_FAST_EVT;
    fastEvt->notified = FALSE;
    fastEvt->timerNode.index = -1;
}

inline void NotifyFastEvt( DzHost* host, DzFastEvt* fastEvt, int status )
{
    if( !fastEvt->dzCot ){
        fastEvt->notified = TRUE;
        return;
    }
    DispatchCot( host, fastEvt->dzCot );
    fastEvt->dzCot = NULL;
    if( IsTimeNodeInHeap( &fastEvt->timerNode ) ){
        RemoveTimer( host, &fastEvt->timerNode );
    }
    fastEvt->status = status;
}

inline int WaitFastEvt( DzHost* host, DzFastEvt* fastEvt, int timeout )
{
    if( fastEvt->notified ){
        fastEvt->notified = FALSE;
        return 0;
    }
    fastEvt->dzCot = host->currCot;
    if( timeout >= 0 ){
        fastEvt->timerNode.repeat = FALSE;
        fastEvt->timerNode.timestamp = MilUnixTime( host ) + timeout;
        AddTimer( host, &fastEvt->timerNode );
    }
    Schedule( host );
    return fastEvt->status;
}

inline void CleanEasyEvt( DzEasyEvt* easyEvt )
{
    easyEvt->dzCot = NULL;
}

inline BOOL IsEasyEvtWaiting( DzEasyEvt* easyEvt )
{
    return easyEvt->dzCot != NULL;
}

inline void NotifyEasyEvt( DzHost* host, DzEasyEvt* easyEvt )
{
    DispatchCot( host, easyEvt->dzCot );
}

inline void WaitEasyEvt( DzHost* host, DzEasyEvt* easyEvt )
{
    easyEvt->dzCot = host->currCot;
    Schedule( host );
}

inline void DelayCurrCot( DzHost* host, int milSec )
{
    DzFastEvt evt;

    InitFastEvt( &evt );
    WaitFastEvt( host, &evt, milSec );
}

#ifdef __cplusplus
};
#endif

#endif // __DzSynObj_h__
