/********************************************************************
    created:    2010/02/11 22:07
    file:       DzSynObj.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzSynObj_h__
#define __DzSynObj_h__

#include <malloc.h>
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
    void*       context,
    int         priority,
    int         sSize
    );

inline DzSynObj* AllocSynObj( DzHost* host )
{
    DzLItr* head;
    DzSynObj* obj;

    head = &host->synObjPool;
    if( !head->next ){
        if( !AllocSynObjPool( host ) ){
            return NULL;
        }
    }
    obj = MEMBER_BASE( head->next, DzSynObj, lItr );
    PopSList( head );
    return obj;
}

inline void FreeSynObj( DzHost* host, DzSynObj* obj )
{
    PushSList( &host->synObjPool, &obj->lItr );
}

inline BOOL IsNotified( DzSynObj* obj )
{
    return obj->count > 0;
}

inline void WaitNotified( DzSynObj* obj )
{
    if( obj->type > TYPE_EVT_AUTO ){
        return;
    }
    if( obj->type == TYPE_SEM ){
        obj->count--;
    }else{
        obj->notified = FALSE;
    }
}

inline void AppendToWaitQ( DzDList* queue, DzWaitNode* node )
{
    AddDLItrToTail( queue, &node->dlItr );
}

inline void ClearWait( DzHost* host, DzWaitHelper* helper )
{
    int i;

    for( i=0; i < helper->waitCount; i++ ){
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

    for( priority = CP_FIRST; priority <= host->lowestPriority; priority++ ){
        queue = &obj->waitQ[ priority ];
        dlItr = queue->entry.next;
        while( dlItr != &queue->entry ){
            node = MEMBER_BASE( dlItr, DzWaitNode, dlItr );
            dlItr = dlItr->next;
            if( node->helper->nowCount ){
                node->helper->nowCount--;
                if( node->helper->nowCount == 0 ){
                    head = node->helper->nodeArray;
                    for( i=0; i < node->helper->waitCount; i++ ){
                        if( !IsNotified( head[i].synObj ) ){
                            node->helper->nowCount++;
                        }
                    }
                    if( node->helper->nowCount == 0 ){
                        for( i=0; i < node->helper->waitCount; i++ ){
                            WaitNotified( head[i].synObj );
                        }
                        ClearWait( host, node->helper );
                        DispatchThread( host, node->helper->dzThread );
                        node->helper->notifyNode = node;
                        if( !IsNotified( obj ) ){
                            return FALSE;
                        }
                    }
                }
            }else{
                WaitNotified( obj );
                ClearWait( host, node->helper );
                DispatchThread( host, node->helper->dzThread );
                node->helper->notifyNode = node;

                if( !IsNotified( obj ) ){
                    return FALSE;
                }
            }
        }
    }
    return TRUE;
}

inline DzSynObj* CreateEvt( DzHost* host, BOOL notified, BOOL autoReset )
{
    DzSynObj* obj;

    obj = AllocSynObj( host );
    if( !obj ){
        return NULL;
    }
    obj->type = autoReset ? TYPE_EVT_AUTO : TYPE_EVT_MANUAL;
    obj->notified = notified;
    obj->status = DS_NONE;
    obj->ref = 1;
    return obj;
}

inline void SetEvt( DzHost* host, DzSynObj* evt )
{
    if( evt->notified ){
        return;
    }
    evt->notified = TRUE;
    NotifyWaitQueue( host, evt );
}

inline void ResetEvt( DzSynObj* evt )
{
    evt->notified = FALSE;
}

inline DzSynObj* CreateSem( DzHost* host, uint  count )
{
    DzSynObj* obj;

    obj = AllocSynObj( host );
    if( !obj ){
        return NULL;
    }
    obj->type = TYPE_SEM;
    obj->count = count;
    obj->status = DS_NONE;
    obj->ref = 1;
    return obj;
}

inline uint ReleaseSem( DzHost* host, DzSynObj* sem, int count )
{
    if( sem->count > 0 ){
        sem->count += count;
        return sem->count;
    }
    sem->count += count;
    NotifyWaitQueue( host, sem );
    return sem->count;
}

inline DzSynObj* CreateTimer( DzHost* host, int milSec, unsigned short repeat )
{
    DzSynObj* obj;

    obj = AllocSynObj( host );
    if( !obj ){
        return NULL;
    }
    obj->type = TYPE_TIMER;
    obj->timerNode.timestamp = MilUnixTime() + milSec;
    obj->timerNode.repeat = repeat;
    obj->timerNode.interval = - milSec;
    obj->ref = 1;
    AddTimer( host, &obj->timerNode );
    return obj;
}

inline void CloseTimer( DzHost* host, DzSynObj* timer )
{
    timer->ref--;
    if( timer->ref == 0 ){
        if( IsTimeNodeInHeap( &timer->timerNode ) ){
            RemoveTimer( host, &timer->timerNode );
        }
        FreeSynObj( host, timer );
    }
}

inline DzSynObj* CreateCallbackTimer(
    DzHost*     host,
    DzRoutine   callback,
    int         priority,
    int         sSize
    )
{
    DzSynObj* obj;

    obj = AllocSynObj( host );
    if( !obj ){
        return NULL;
    }
    obj->type = TYPE_CALLBACK_TIMER;
    obj->routine = callback;
    obj->priority = priority;
    obj->sSize = sSize;
    obj->timerNode.index = -1;
    obj->ref = 1;
    return obj;
}

inline BOOL StartCallbackTimer(
    DzHost*         host,
    DzSynObj*       timer,
    int             milSec,
    unsigned short  repeat,
    void*           context
    )
{
    if( IsTimeNodeInHeap( &timer->timerNode ) ){
        return FALSE;
    }
    timer->context = context;
    timer->timerNode.timestamp = MilUnixTime() + milSec;
    timer->timerNode.repeat = repeat;
    timer->timerNode.interval = - milSec;
    AddTimer( host, &timer->timerNode );
    return TRUE;
}

inline BOOL StopCallbackTimer( DzHost* host, DzSynObj* timer )
{
    if( IsTimeNodeInHeap( &timer->timerNode ) ){
        RemoveTimer( host, &timer->timerNode );
        return TRUE;
    }
    return FALSE;
}

inline void CloseCallbackTimer( DzHost* host, DzSynObj* timer )
{
    timer->ref--;
    if( timer->ref == 0 ){
        if( IsTimeNodeInHeap( &timer->timerNode ) ){
            RemoveTimer( host, &timer->timerNode );
        }
        InitDList( &timer->waitQ[ CP_HIGH ] );
        InitDList( &timer->waitQ[ CP_NORMAL ] );
        FreeSynObj( host, timer );
    }
}

inline void NotifyTimerNode( DzHost* host, DzTimerNode* timerNode )
{
    DzFastEvt* fastEvt;
    DzSynObj* timer;
    DzThread* dzThread;

    switch( timerNode->type ){
    case TYPE_TIMER:
        timer = MEMBER_BASE( timerNode, DzSynObj, timerNode );
        //since DzTimerNode.interval and DzTimerNode.notified is union,
        //negative DzTimerNode.notified means not notified.
        //so interval is a negative number
        //when notifying Timer, DzTimerNode.notified should keep positive,
        timer->notified = - timer->notified;
        NotifyWaitQueue( host, timer );
        //well, if a timer can be notified more than one time, it is still in timer heap.
        //so we set it to NOT notified, or else, we keep it notified.
        if( IsTimeNodeInHeap( timerNode ) ){
            timer->notified = - timer->notified;
        }
        break;
    case TYPE_CALLBACK_TIMER:
        timer = MEMBER_BASE( timerNode, DzSynObj, timerNode );
        StartCot( host, timer->routine, timer->context, timer->priority, timer->sSize );
        break;
    case TYPE_TIMEOUT:
        fastEvt = MEMBER_BASE( timerNode, DzFastEvt, timerNode );
        fastEvt->helper->notifyNode = NULL;
        ClearWait( host, fastEvt->helper );
        DispatchThread( host, fastEvt->helper->dzThread );
        break;
    case TYPE_FAST_EVT:
        fastEvt = MEMBER_BASE( timerNode, DzFastEvt, timerNode );
        fastEvt->status = DS_TIMEOUT;
        dzThread = fastEvt->dzThread;
        fastEvt->dzThread = NULL;
        DispatchThread( host, dzThread );
        break;
    }
}

inline DzSynObj* CloneSynObj( DzSynObj* obj )
{
    obj->ref++;
    return obj;
}

inline void CloseSynObj( DzHost* host, DzSynObj* obj )
{
    obj->ref--;
    if( obj->ref == 0 ){
        FreeSynObj( host, obj );
    }
}

inline void InitTimeOut( DzFastEvt* timeout, int milSec, DzWaitHelper* helper )
{
    timeout->type = TYPE_TIMEOUT;
    timeout->timerNode.repeat = 1;
    timeout->timerNode.timestamp = MilUnixTime() + milSec;
    timeout->helper = helper;
    timeout->timerNode.index = -1;
}

inline void InitFastEvt( DzFastEvt* fastEvt )
{
    fastEvt->type = TYPE_FAST_EVT;
    fastEvt->notified = FALSE;
    fastEvt->timerNode.index = -1;
}

inline void NotifyFastEvt( DzHost* host, DzFastEvt* fastEvt, int status )
{
    if( !fastEvt->dzThread ){
        fastEvt->notified = TRUE;
        return;
    }
    DispatchThread( host, fastEvt->dzThread );
    fastEvt->dzThread = NULL;
    if( IsTimeNodeInHeap( &fastEvt->timerNode ) ){
        RemoveTimer( host, &fastEvt->timerNode );
    }
    fastEvt->status = status;
}

inline int WaitFastEvt( DzHost* host, DzFastEvt* obj, int timeout )
{
    if( obj->notified ){
        obj->notified = FALSE;
        return 0;
    }
    if( timeout == 0 ){
        return -1;
    }
    obj->dzThread = host->currThread;
    if( timeout > 0 ){
        obj->timerNode.repeat = 1;
        obj->timerNode.timestamp = MilUnixTime() + timeout;
        AddTimer( host, &obj->timerNode );
    }
    Schedule( host );
    return obj->status  == DS_TIMEOUT ? -1 : 0;
}

inline void DelayCurrThread( DzHost* host, int milSec )
{
    DzFastEvt evt;

    InitFastEvt( &evt );
    WaitFastEvt( host, &evt, milSec );
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
    helper.nowCount = 0;
    helper.nodeArray = &waitNode;
    helper.waitCount = 1;
    helper.nodeArray[0].helper = &helper;
    helper.dzThread = host->currThread;
    if( timeout > 0 ){
        InitTimeOut( &helper.timeout, timeout, &helper );
        AddTimer( host, &helper.timeout.timerNode );
    }else{
        helper.timeout.timerNode.index = -1;
    }
    AppendToWaitQ( &obj->waitQ[ host->currThread->priority ], &helper.nodeArray[0] );
    Schedule( host );
    return helper.notifyNode ? 0 : -1;
}

inline int WaitMultiSynObj( DzHost* host, int count, DzSynObj** obj, BOOL waitAll, int timeout )
{
    DzWaitHelper helper;
    int nowCount;
    int i;

    nowCount = 0;
    if( waitAll ){
        for( i=0; i<count; i++ ){
            if( !IsNotified( obj[i] ) ){
                nowCount++;
            }
        }
        if( nowCount == 0 ){
            for( i=0; i<count; i++ ){
                WaitNotified( obj[i] );
            }
            return 0;
        }
    }else{
        for( i=0; i<count; i++ ){
            if( IsNotified( obj[i] ) ){
                WaitNotified( obj[i] );
                return i;
            }
        }
    }
    if( timeout == 0 ){
        return -1;
    }
    helper.waitCount = count;
    helper.dzThread = host->currThread;
    helper.nodeArray = (DzWaitNode*)alloca( sizeof(DzWaitNode) * count );
    for( i=0; i<count; i++ ){
        helper.nodeArray[i].helper = &helper;
        helper.nodeArray[i].synObj = obj[i];
    }
    if( timeout > 0 ){
        InitTimeOut( &helper.timeout, timeout, &helper );
        AddTimer( host, &helper.timeout.timerNode );
    }else{
        helper.timeout.timerNode.index = -1;
    }
    helper.nowCount = nowCount;
    for( i=0; i<count; i++ ){
        AppendToWaitQ( &obj[i]->waitQ[ host->currThread->priority ], &helper.nodeArray[i] );
    }
    Schedule( host );
    if( !helper.notifyNode ){
        return -1;
    }
    if( waitAll ){
        return 0;
    }else{
        return (int)(helper.notifyNode - &helper.nodeArray[0]);
    }
}

#ifdef __cplusplus
};
#endif

#endif // __DzSynObj_h__
