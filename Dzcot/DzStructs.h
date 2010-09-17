
#ifndef _DZ_STRUCTS_H_
#define _DZ_STRUCTS_H_

#include "DzType.h"
#include "DzStructsQueue.h"
#include "DzStructsOs.h"

#define MAX_DZ_HOST     8

struct _DzHost;
struct _DzThread;
struct _DzWaitNode;
struct _DzWaitHelper;

typedef enum _SynObjType
{
    TYPE_SEM,
    TYPE_EVT_AUTO,
    TYPE_EVT_MANUAL,
    TYPE_TIMER,
    TYPE_CALLBACK_TIMER,
    TYPE_TIMEOUT,
    TYPE_FAST_EVT
}SynObjType;

typedef struct _DzTimerNode
{
    short           type;
    unsigned short  repeat;
    int             index;
    int64           timestamp;
    int             interval;       //for repeat timer, should set minus
}DzTimerNode;

typedef struct _DzFastEvt
{
    union{
        DzTimerNode     timerNode;
        struct{
            short       type;
            short       unused1;
            int         unused2;
            int64       unusedTimestamp;
            int         status;
        };
    };
    BOOL                notified;
    union{
        struct _DzWaitHelper*   helper;     //for timeout
        struct _DzThread*       dzThread;   //for fast event
    };
}DzFastEvt;

typedef struct _DzSynObj
{
    union{
        DzQItr          qItr;
        DzTimerNode     timerNode;
        struct{
            short       type;
            short       unused1;
            int         status;
            int64       unusedTimestamp;
            union{
                int     unusedInterval;     //for repeat timer, should set minus
                int     count;              //for semaphore
                BOOL    notified;           //for event
            };
        };
    };
    int                 ref;
    union{
        struct{
            DzRoutine   routine;            //for CallbackTimer
            void*       context;            //should reset the waitQ[ CP_HIGH ] and waitQ[ CP_NORMAL ]
            int         priority;           //when release CallbackTimer
            int         sSize;
        };
        struct{
            DzDeque     waitQ[ COT_PRIORITY_COUNT ];
        };
    };
}DzSynObj;

typedef struct _DzWaitNode
{
    union{
        DzQItr              qItr;
        DzDqItr             dqItr;
    };
    struct _DzWaitHelper*   helper;
    DzSynObj*               synObj;
}DzWaitNode;

typedef struct _DzWaitHelper
{
    int                     nowCount;
    int                     waitCount;
    struct _DzThread*       dzThread;
    DzWaitNode*             notifyNode;
    DzFastEvt               timeOut;
    DzWaitNode*             nodeArray;
}DzWaitHelper;

typedef struct _DzThread
{
    DzQItr          qItr;
    void*           sp;
    char*           stack;
    char*           stackLimit;
    DzSynObj*       finishEvent;
    int             stackSize;
    int             priority;
}DzThread;

typedef struct _DzHost
{
    //running co thread
    DzThread*       currThread;

    //CP_INSTANT task called instantly, so needn't be queued
    //if needed, while switch a CP_INSTANT to another CP_INSTANT
    //push it to the head of CP_HIGH queue
    int             lowestPriority;
    int             currPriority;
    DzQueue         taskQs[ COT_PRIORITY_COUNT ];

    //the host thread's original info
    DzThread        originThread;

    //DzDqNode struct pool
    DzQItr          qNodePool;

    //used for checking additional task after cot switching
    int             taskCheckTag;

    //DzThread struct pool
    int             poolDepth;
    DzQItr          threadPools[ STACK_SIZE_COUNT ];

    //statistic for different stack size thread
    //poolCotCount only accumulate the thread reserved virtual address space
    int             poolCotCounts[ STACK_SIZE_COUNT ];
    int             threadCount;
    int             maxThreadCount;

    //used for timer
    int             timerCount;
    int             timerHeapSize;
    DzTimerNode**   timerHeap;

    //Io Manager
    DzIoMgr         ioMgr;

    //Os Append info
    DzOsAppend      osAppend;

    //record malloc history
    int             mallocCount;
    DzQItr          mallocList;

    //DzSynObj struct pool
    DzQItr          synObjPool;

    //DzAsynIo struct pool
    DzQItr          asynIoPool;

    //default co thread value
    void*           originExceptPtr;
    int             defaultPri;
    int             defaultSSize;

    //host's status
    BOOL            isExiting;
    BOOL            isBlocking;
}DzHost;

#endif  //#ifndef _DZ_STRUCTS_H_