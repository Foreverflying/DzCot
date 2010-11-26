/********************************************************************
    created:    2010/02/11 22:05
    file:       DzStructs.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzStructs_h__
#define __DzStructs_h__

#include "DzType.h"
#include "DzStructsList.h"
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
        DzLItr          lItr;
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
            DzDList     waitQ[ COT_PRIORITY_COUNT ];
        };
    };
}DzSynObj;

typedef struct _DzWaitNode
{
    union{
        DzLItr              lItr;
        DzDLItr             dlItr;
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
    DzFastEvt               timeout;
    DzWaitNode*             nodeArray;
}DzWaitHelper;

typedef struct _DzHost
{
    //running co thread
    DzThread*       currThread;

    //CP_INSTANT task called instantly, so needn't be queued
    //if needed, while switch a CP_INSTANT to another CP_INSTANT
    //push it to the head of CP_HIGH queue
    int             lowestPriority;
    int             currPriority;
    DzSList         taskLs[ COT_PRIORITY_COUNT ];

    //the host thread's original info
    DzThread        originThread;

    //DzDqNode struct pool
    DzLItr          lNodePool;

    //DzThread struct pool
    DzLItr          threadPools[ STACK_SIZE_COUNT ];
    int             poolDepth;

    //statistic for different stack size thread
    //poolCotCount only accumulate the thread reserved virtual address space
    int             threadCount;
    int             maxThreadCount;

    //Os struct
    DzOsStruct      osStruct;

    //used for timer
    DzTimerNode**   timerHeap;
    int             timerCount;
    int             timerHeapSize;

    //record pool alloc history
    DzLItr          poolGrowList;

    char*           memPoolPos;
    char*           memPoolEnd;

    //DzSynObj struct pool
    DzLItr          synObjPool;

    //DzAsynIo struct pool
    DzLItr          asynIoPool;

    //default co thread value
    int             defaultPri;
    int             defaultSSize;
}DzHost;

#endif // __DzStructs_h__
