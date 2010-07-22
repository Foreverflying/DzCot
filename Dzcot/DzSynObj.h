
#ifndef _DZ_SYN_OBJ_H_
#define _DZ_SYN_OBJ_H_

#include <malloc.h>
#include "DzStructs.h"
#include "DzTimer.h"
#include "DzSchedule.h"
#include "DzResourceMgr.h"

#ifdef __cplusplus
extern "C"{
#endif

int StartCot(
    DzHost*     host,
    DzRoutine   entry,
    void        *context,
    int         priority,
    int         sSize
    );

inline BOOL IsNotified( DzSynObj *obj )
{
    return obj->count > 0;
}

inline void WaitNotified( DzSynObj *obj )
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

inline void AppendToWaitQ( DzDeque *queue, DzWaitNode *node )
{
    AddDqItrToTail( queue, &node->dqItr );
}

inline void ClearWait( DzHost *host, DzWaitHelper *helper )
{
    int i;

    for( i=0; i < helper->waitCount; i++ ){
        DequeueDqItr( &helper->nodeArray[i].dqItr );
    }
    if( IsTimeNodeInHeap( &helper->timeOut.timerNode ) ){
        RemoveTimer( host, &helper->timeOut.timerNode );
    }
}

inline BOOL NotifyWaitAllQueue( DzHost *host, DzSynObj *obj )
{
    DzDeque *queue;
    DzDqItr *dqItr;
    DzWaitNode *node;
    DzWaitHelper *helper;
    DzWaitNode *head;
    int i;

    queue = &obj->waitAllQ;
    dqItr = queue->entry.next;
    while( dqItr != &queue->entry ){
        node = MEMBER_BASE( dqItr, DzWaitNode, dqItr );
        dqItr = dqItr->next;
        helper = node->helper;
        helper->nowCount--;
        if( helper->nowCount == 0 ){
            head = helper->nodeArray;
            for( i=0; i < helper->waitCount; i++ ){
                if( !IsNotified( head[i].synObj ) ){
                    helper->nowCount++;
                }
            }
            if( helper->nowCount == 0 ){
                for( i=0; i < helper->waitCount; i++ ){
                    WaitNotified( head[i].synObj );
                }
                ClearWait( host, helper );
                DispatchThread( host, helper->dzThread );
                helper->notifyNode = node;
                if( !IsNotified( obj ) ){
                    return FALSE;
                }
            }
        }
    }
    return TRUE;
}

inline BOOL NotifyWaitQueue( DzHost *host, DzSynObj *obj )
{
    DzDeque *queue;
    DzDqItr *dqItr;
    DzWaitNode *node;

    queue = &obj->waitQ;
    dqItr = queue->entry.next;
    while( dqItr != &queue->entry ){
        node = MEMBER_BASE( dqItr, DzWaitNode, dqItr );
        dqItr = dqItr->next;
        WaitNotified( obj );
        ClearWait( host, node->helper );
        DispatchThread( host, node->helper->dzThread );
        node->helper->notifyNode = node;

        if( !IsNotified( obj ) ){
            return FALSE;
        }
    }
    return TRUE;
}

inline DzSynObj* CreateEvt( DzHost *host, BOOL notified, BOOL autoReset )
{
    DzSynObj *obj;

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

inline void SetEvt( DzHost *host, DzSynObj *evt )
{
    if( evt->notified ){
        return;
    }
    evt->notified = TRUE;
    if( NotifyWaitAllQueue( host, evt ) ){
        NotifyWaitQueue( host, evt );
    }
    if( UpdateCurrPriority( host, host->currPriority ) ){
        DispatchCurrThread( host );
    }
}

inline void ResetEvt( DzSynObj *evt )
{
    evt->notified = FALSE;
}

inline DzSynObj* CreateSem( DzHost *host, u_int count )
{
    DzSynObj *obj;

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

inline u_int ReleaseSem( DzHost *host, DzSynObj *sem, int count )
{
    if( sem->count > 0 ){
        sem->count += count;
        return sem->count;
    }
    sem->count += count;
    if( NotifyWaitAllQueue( host, sem ) ){
        NotifyWaitQueue( host, sem );
    }
    if( UpdateCurrPriority( host, host->currPriority ) ){
        DispatchCurrThread( host );
    }
    return sem->count;
}

inline DzSynObj* CreateTimer( DzHost *host, int milSec, unsigned short repeat )
{
    DzSynObj *obj;

    obj = AllocSynObj( host );
    if( !obj ){
        return NULL;
    }
    obj->type = TYPE_TIMER;
    obj->timerNode.timestamp = DzCurrentTime() + milSec;
    obj->timerNode.repeat = repeat;
    obj->timerNode.interval = - milSec;
    obj->ref = 1;
    AddTimer( host, &obj->timerNode );
    return obj;
}

inline void CloseTimer( DzHost *host, DzSynObj *timer )
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
    DzHost      *host,
    DzRoutine   callback,
    int         priority,
    int         sSize
    )
{
    DzSynObj *obj;

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
    DzHost          *host,
    DzSynObj        *timer,
    int             milSec,
    unsigned short  repeat,
    void            *context
    )
{
    if( IsTimeNodeInHeap( &timer->timerNode ) ){
        return FALSE;
    }
    timer->context = context;
    timer->timerNode.timestamp = DzCurrentTime() + milSec;
    timer->timerNode.repeat = repeat;
    timer->timerNode.interval = - milSec;
    AddTimer( host, &timer->timerNode );
    return TRUE;
}

inline BOOL StopCallbackTimer( DzHost *host, DzSynObj *timer )
{
    if( IsTimeNodeInHeap( &timer->timerNode ) ){
        RemoveTimer( host, &timer->timerNode );
        return TRUE;
    }
    return FALSE;
}

inline void CloseCallbackTimer( DzHost *host, DzSynObj *timer )
{
    timer->ref--;
    if( timer->ref == 0 ){
        if( IsTimeNodeInHeap( &timer->timerNode ) ){
            RemoveTimer( host, &timer->timerNode );
        }
        InitDeque( &timer->waitQ );
        InitDeque( &timer->waitAllQ );
        FreeSynObj( host, timer );
    }
}

inline void NotifyTimerNode( DzHost *host, DzTimerNode *timerNode, BOOL lastTime )
{
    DzFastEvt *fastEvt;
    DzSynObj *timer;
    DzThread *dzThread;

    switch( timerNode->type ){
    case TYPE_TIMER:
        timer = MEMBER_BASE( timerNode, DzSynObj, timerNode );
        timer->notified = lastTime;
        if( NotifyWaitAllQueue( host, timer ) ){
            NotifyWaitQueue( host, timer );
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

inline DzSynObj* CloneSynObj( DzSynObj *obj )
{
    obj->ref++;
    return obj;
}

inline void CloseSynObj( DzHost *host, DzSynObj *obj )
{
    obj->ref--;
    if( obj->ref == 0 ){
        FreeSynObj( host, obj );
    }
}

inline void InitTimeOut( DzFastEvt *timeOut, int milSec, DzWaitHelper *helper )
{
    timeOut->type = TYPE_TIMEOUT;
    timeOut->timerNode.repeat = 1;
    timeOut->timerNode.timestamp = DzCurrentTime() + milSec;
    timeOut->helper = helper;
    timeOut->timerNode.index = -1;
}

inline void InitFastEvt( DzFastEvt *fastEvt )
{
    fastEvt->type = TYPE_FAST_EVT;
    fastEvt->notified = FALSE;
    fastEvt->timerNode.index = -1;
}

inline void NotifyFastEvt( DzHost *host, DzFastEvt *fastEvt, int status )
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

inline int WaitFastEvt( DzHost *host, DzFastEvt *obj, int timeOut )
{
    if( obj->notified ){
        obj->notified = FALSE;
        return 0;
    }
    if( timeOut == 0 ){
        return -1;
    }
    obj->dzThread = host->currThread;
    if( timeOut > 0 ){
        obj->timerNode.repeat = 1;
        obj->timerNode.timestamp = DzCurrentTime() + timeOut;
        AddTimer( host, &obj->timerNode );
    }
    Schedule( host );
    return obj->status  == DS_TIMEOUT ? -1 : 0;
}

inline void DelayCurrThread( DzHost *host, int milSec )
{
    DzFastEvt evt;

    InitFastEvt( &evt );
    WaitFastEvt( host, &evt, milSec );
}

inline int WaitSynObj( DzHost *host, DzSynObj *obj, int timeOut )
{
    DzWaitHelper helper;
    DzWaitNode waitNode;

    if( IsNotified( obj ) ){
        WaitNotified( obj );
        return 0;
    }
    if( timeOut == 0 ){
        return -1;
    }
    helper.nodeArray = &waitNode;
    helper.waitCount = 1;
    helper.nodeArray[0].helper = &helper;
    helper.dzThread = host->currThread;
    if( timeOut > 0 ){
        InitTimeOut( &helper.timeOut, timeOut, &helper );
        AddTimer( host, &helper.timeOut.timerNode );
    }else{
        helper.timeOut.timerNode.index = -1;
    }
    AppendToWaitQ( &obj->waitQ, &helper.nodeArray[0] );
    Schedule( host );
    return 0;
}

inline int WaitMultiSynObj( DzHost *host, int count, DzSynObj **obj, BOOL waitAll, int timeOut )
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
    if( timeOut == 0 ){
        return -1;
    }
    helper.waitCount = count;
    helper.dzThread = host->currThread;
    helper.nodeArray = (DzWaitNode*)alloca( sizeof(DzWaitNode) * count );
    for( i=0; i<count; i++ ){
        helper.nodeArray[i].helper = &helper;
        helper.nodeArray[i].synObj = obj[i];
    }
    if( timeOut > 0 ){
        InitTimeOut( &helper.timeOut, timeOut, &helper );
        AddTimer( host, &helper.timeOut.timerNode );
    }else{
        helper.timeOut.timerNode.index = -1;
    }
    if( waitAll ){
        helper.nowCount = nowCount;
        for( i=0; i<count; i++ ){
            AppendToWaitQ( &obj[i]->waitAllQ, &helper.nodeArray[i] );
        }
    }else{
        for( i=0; i<count; i++ ){
            AppendToWaitQ( &obj[i]->waitQ, &helper.nodeArray[i] );
        }
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

#endif  // #ifndef _DZ_SYN_OBJ_H_