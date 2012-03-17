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
    DzRoutine   entry,
    intptr_t    context,
    int         priority,
    int         sSize
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

inline int CreateManualEvt( DzHost* host, BOOL notified )
{
    DzSynObj* obj;

    obj = AllocSynObj( host );
    if( !obj ){
        return -1;
    }
    obj->type = TYPE_EVT_MANUAL;
    obj->notifyCount = notified ? 1 : 0;
    obj->ref++;
    return (int)( (intptr_t)obj - host->handleBase );
}

inline int CreateAutoEvt( DzHost* host, BOOL notified )
{
    DzSynObj* obj;

    obj = AllocSynObj( host );
    if( !obj ){
        return -1;
    }
    obj->type = TYPE_EVT_AUTO;
    obj->notifyCount = notified ? 1 : 0;
    obj->ref++;
    return (int)( (intptr_t)obj - host->handleBase );
}

inline int CreateCdEvt( DzHost* host, u_int count )
{
    DzSynObj* obj;

    obj = AllocSynObj( host );
    if( !obj ){
        return -1;
    }
    obj->type = TYPE_EVT_COUNT;
    obj->notifyCount = 1 - count;
    obj->ref++;
    return (int)( (intptr_t)obj - host->handleBase );
}

inline void SetEvtRaw( DzHost* host, DzSynObj* obj )
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

inline void SetEvt( DzHost* host, int evt )
{
    DzSynObj* obj = (DzSynObj*)( host->handleBase + evt );
    SetEvtRaw( host, obj );
}

inline void ResetEvt( DzHost* host, int evt )
{
    DzSynObj* obj = (DzSynObj*)( host->handleBase + evt );
    if( obj->type == TYPE_EVT_COUNT ){
        obj->notifyCount--;
    }else{
        obj->notifyCount = 0;
    }
}

inline int CreateSem( DzHost* host, int count )
{
    DzSynObj* obj;

    obj = AllocSynObj( host );
    if( !obj ){
        return -1;
    }
    obj->type = TYPE_SEM;
    obj->notifyCount = count;
    obj->ref++;
    return (int)( (intptr_t)obj - host->handleBase );
}

inline int ReleaseSem( DzHost* host, int sem, int count )
{
    DzSynObj* obj = (DzSynObj*)( host->handleBase + sem );
    if( obj->notifyCount > 0 ){
        obj->notifyCount += count;
        return obj->notifyCount;
    }
    obj->notifyCount += count;
    if( NotifyWaitQueue( host, obj ) ){
        host->currPri = CP_FIRST;
    }
    return obj->notifyCount;
}

inline int CreateTimer( DzHost* host, int milSec, int repeat )
{
    DzSynObj* obj;

    obj = AllocSynObj( host );
    if( !obj ){
        return -1;
    }
    obj->type = TYPE_TIMER;
    obj->timerNode.timestamp = MilUnixTime() + milSec;
    obj->timerNode.repeat = repeat;
    obj->timerNode.interval = - milSec;
    obj->ref++;
    AddTimer( host, &obj->timerNode );
    return (int)( (intptr_t)obj - host->handleBase );
}

inline void CloseTimer( DzHost* host, int timer )
{
    DzSynObj* obj = (DzSynObj*)( host->handleBase + timer );
    obj->ref--;
    if( obj->ref == 0 ){
        if( IsTimeNodeInHeap( &obj->timerNode ) ){
            RemoveTimer( host, &obj->timerNode );
        }
        FreeSynObj( host, obj );
    }
}

inline int CreateCallbackTimer(
    DzHost*         host,
    int             milSec,
    int             repeat,
    DzRoutine       callback,
    intptr_t        context,
    int             priority,
    int             sSize
    )
{
    DzSynObj* obj;

    obj = AllocSynObj( host );
    if( !obj ){
        return -1;
    }
    obj->type = TYPE_CALLBACK_TIMER;
    obj->timerNode.interval = - milSec;
    obj->timerNode.index = -1;
    obj->routine = callback;
    obj->context = context;
    obj->timerNode.timestamp = MilUnixTime() + milSec;
    obj->timerNode.repeat = repeat;
    obj->priority = priority;
    obj->sSize = sSize;
    obj->ref++;
    host->cotCount++;

    AddTimer( host, &obj->timerNode );
    return (int)( (intptr_t)obj - host->handleBase );
}

inline void CloseCallbackTimer( DzHost* host, int timer )
{
    DzSynObj* obj = (DzSynObj*)( host->handleBase + timer );
    if( IsTimeNodeInHeap( &obj->timerNode ) ){
        RemoveTimer( host, &obj->timerNode );
    }
    host->cotCount--;
    obj->routine = NULL;
    obj->ref--;
    if( obj->ref == 0 ){
        FreeSynObj( host, obj );
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
        //well, if a timer can be notified more than once, it is still in timer heap.
        //so we set it to NOT notified, or else, we keep it notified.
        if( IsTimeNodeInHeap( timerNode ) ){
            timer->notifyCount = - timer->notifyCount;
        }
        return ret;

    case TYPE_CALLBACK_TIMER:
        timer = MEMBER_BASE( timerNode, DzSynObj, timerNode );
        timer->ref++;
        StartCot( host, CallbackTimerEntry, (intptr_t)timer, timer->priority, timer->sSize );
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

inline int CloneSynObj( DzHost* host, int obj )
{
    DzSynObj* synObj = (DzSynObj*)( host->handleBase + obj );
    synObj->ref++;
    return obj;
}

inline void CloseSynObjRaw( DzHost* host, DzSynObj* obj )
{
    obj->ref--;
    if( obj->ref == 0 ){
        FreeSynObj( host, obj );
    }
}

inline void CloseSynObj( DzHost* host, int obj )
{
    DzSynObj* synObj = (DzSynObj*)( host->handleBase + obj );
    CloseSynObjRaw( host, synObj );
}

inline void InitTimeOut( DzFastEvt* timeout, int milSec, DzWaitHelper* helper )
{
    timeout->type = TYPE_TIMEOUT;
    timeout->timerNode.repeat = 1;
    timeout->timerNode.timestamp = MilUnixTime() + milSec;
    timeout->helper = helper;
    timeout->timerNode.index = -1;
}

inline int WaitSynObj( DzHost* host, int obj, int timeout )
{
    DzWaitHelper helper;
    DzWaitNode waitNode;
    DzSynObj* synObj;

    synObj = (DzSynObj*)( host->handleBase + obj );
    if( IsNotified( synObj ) ){
        WaitNotified( synObj );
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
        InitTimeOut( &helper.timeout, timeout, &helper );
        AddTimer( host, &helper.timeout.timerNode );
    }else{
        helper.timeout.timerNode.index = -1;
    }
    AppendToWaitQ( &synObj->waitQ[ host->currCot->priority ], &helper.nodeArray[0] );
    Schedule( host );
    return helper.checkIdx;
}

inline int WaitMultiSynObj( DzHost* host, int count, int* objs, BOOL waitAll, int timeout )
{
    DzWaitHelper helper;
    DzSynObj* synObj;
    DzWaitNode* node;
    int i;

    if( waitAll ){
        for( i = 0; i < count; i++ ){
            if( !IsNotified( (DzSynObj*)( host->handleBase + objs[i] ) ) ){
                break;
            }
        }
        if( i == count ){
            for( i = 0; i < count; i++ ){
                WaitNotified( (DzSynObj*)( host->handleBase + objs[i] ) );
            }
            return 0;
        }
    }else{
        for( i = 0; i < count; i++ ){
            synObj = (DzSynObj*)( host->handleBase + objs[i] );
            if( IsNotified( synObj ) ){
                WaitNotified( synObj );
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
        node->synObj = (DzSynObj*)( host->handleBase + *objs );
        AppendToWaitQ( &node->synObj->waitQ[ host->currCot->priority ], node );
        objs++;
    }
    if( timeout > 0 ){
        InitTimeOut( &helper.timeout, timeout, &helper );
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
        fastEvt->timerNode.repeat = 1;
        fastEvt->timerNode.timestamp = MilUnixTime() + timeout;
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
