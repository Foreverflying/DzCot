/********************************************************************
    created:    2010/02/11 22:05
    file:       DzStructs.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzStructs_h__
#define __DzStructs_h__

#include "DzIncOs.h"
#include "DzDeclareStructs.h"
#include "DzStructsList.h"
#include "DzStructsOs.h"

enum
{
    TYPE_SEM,
    TYPE_EVT_AUTO,
    TYPE_MAX_WAIT_AFFECT = TYPE_EVT_AUTO,
    TYPE_EVT_MANUAL,
    TYPE_EVT_COUNT,
    TYPE_TIMER,
    TYPE_MAX_USER_CAN_WAIT = TYPE_TIMER,
    TYPE_CALLBACK_TIMER,
    TYPE_TIMEOUT,
    TYPE_FAST_EVT
};

struct _DzTimerNode
{
    short           type;
    unsigned short  repeat;
    int             index;
    int64           timestamp;
    int             interval;       //for repeat timer, should set minus
};

struct _DzFastEvt
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
        DzWaitHelper*   helper;     //for timeout
        DzThread*       dzThread;   //for fast event
    };
};

struct _DzSynObj
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
                int     notifyCount;        //for semaphore and event
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
};

struct _DzWaitNode
{
    union{
        DzLItr      lItr;
        DzDLItr     dlItr;
    };
    DzWaitHelper*   helper;
    DzSynObj*       synObj;
};

struct _DzWaitHelper
{
    int             waitCount;
    int             checkIdx;
    DzThread*       dzThread;
    DzWaitNode*     nodeArray;
    DzFastEvt       timeout;
};

struct _DzHost
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

    //DzThread struct pool
    DzLItr*         threadPool;
    DzLItr*         cotPools[ STACK_SIZE_COUNT ];
    int             cotPoolDepth[ STACK_SIZE_COUNT ];

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

    //dlmalloc heap
    void*           mallocSpace;

    //DzSynObj struct pool
    DzLItr*         synObjPool;

    //DzDqNode struct pool
    DzLItr*         lNodePool;

    //record pool alloc history
    DzLItr*         poolGrowList;

    char*           memPoolPos;
    char*           memPoolEnd;

    //default co thread value
    int             defaultPri;
    int             defaultSSize;
};

#endif // __DzStructs_h__
