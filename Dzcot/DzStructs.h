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
    int             type;
    int             index;
    int64           timestamp;
    int             interval;
    int             repeat;       //for repeat timer, should set minus
};

struct _DzEasyEvt
{
    DzCot*          dzCot;
};

struct _DzFastEvt
{
    union{
        DzTimerNode     timerNode;
        struct{
            int         type;
            int         unused1;
            int64       unusedTimestamp;
            BOOL        notified;
            int         status;
        };
    };
    union{
        DzWaitHelper*   helper;     //for timeout
        DzCot*          dzCot;   //for fast event
    };
};

struct _DzSynObj
{
    union{
        DzLItr          lItr;
        DzTimerNode     timerNode;
        struct{
            int         type;
            int         unused1;
            int64       unusedTimestamp;
            int         notifyCount;        //for semaphore and event
            int         unused2;
        };
    };
    int                 ref;
    union{
        struct{
            DzRoutine   routine;    //for CallbackTimer, must reset
            intptr_t    context;    //the waitQ[ CP_HIGH ] and waitQ[ CP_NORMAL ]
            int         priority;   //when release CallbackTimer
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
    DzCot*          dzCot;
    DzWaitNode*     nodeArray;
    DzFastEvt       timeout;
};

struct _DzRmtCotFifo
{
    DzRmtCotFifo*   next;
    int             readPos;
    int             writePos;
    DzCot**         rmtCotArr;
};

struct _DzHostsMgr
{
    DzHost**        hostArr;
    DzRmtCotFifo*   rmtFifoRes;
    DzSList*        pendRmtCotRes;
    DzSList*        lazyRmtCotRes;
    DzSList*        lazyFreeMemRes;
    int*            servMask;
    int             hostCount;
    volatile int    exitSign;
};

struct _DzHost
{
    //running cot
    DzCot*          currCot;

    //CP_INSTANT task called instantly, so needn't be queued
    //if needed, while switch a CP_INSTANT to another CP_INSTANT
    //push it to the head of CP_HIGH queue
    int             lowestPri;
    int             currPri;
    DzSList         taskLs[ COT_PRIORITY_COUNT ];

    union{
        //the host thread's original stack info
        DzCot               centerCot;

        struct{
            //centerCot use sp only
            DzLItr          unused_lItr;
            void*           unused_sp;

            //host's id
            int             hostId;

            //remote call FIFO check sign
            volatile int    checkRmtSign;

            //host mask
            int             hostMask;
        };
    };

    //current cot count
    int             cotCount;

    //cot schedule countdown
    int             scheduleCd;

    //DzCot struct pool
    DzLItr*         cotPool;
    DzLItr*         cotPools[ STACK_SIZE_COUNT ];
    int             cotPoolNowDepth[ STACK_SIZE_COUNT ];

    //Os struct
    DzOsStruct      osStruct;

    //used for timer
    DzTimerNode**   timerHeap;
    int             timerCount;
    int             timerHeapSize;

    //default cot value
    int             dftPri;
    int             dftSSize;

    //dlmalloc heap
    void*           mSpace;

    //DzSynObj struct pool
    DzLItr*         synObjPool;

    //DzDqNode struct pool
    DzLItr*         lNodePool;

    //multi hosts manager
    DzHostsMgr*     hostMgr;

    //checking FIFO chain
    DzRmtCotFifo*   checkFifo;

    //remote call FIFOs
    DzRmtCotFifo*   rmtFifoArr;

    //pending remote cots
    DzSList*        pendRmtCot;

    //lazy dispatch remote cots
    DzSList*        lazyRmtCot;

    //lazy free memory list
    DzSList*        lazyFreeMem;

    //lazy checking timer
    DzSynObj*       lazyTimer;

    //memory chunk pool
    char*           memPoolPos;
    char*           memPoolEnd;

    //record pool alloc history
    DzLItr*         poolGrowList;

    //configure data
    int             cotPoolSetDepth[ STACK_SIZE_COUNT ];
};

struct _DzSysParam
{
    DzRoutine           threadEntry;
    int                 result;
    union{
        struct{
            DzHandle    evt;
            DzHostsMgr* hostMgr;
            DzCot*      returnCot;
            int         hostId;
            int         lowestPri;
            int         dftPri;
            int         dftSSize;
        } hs;   //used for host start
        struct{
            DzRoutine   entry;
            intptr_t    context;
        } cs;   //used for cot start
    };
};

struct _DzCotParam
{
    char            hostId;
    char            type;       //0 normal call; 1 feedback call; -1 emergency
    char            evtType;    //0 SynObj Events; 1 EasyEvt
    char            sign;
    union{
        DzSynObj*   evt;
        DzEasyEvt*  easyEvt;
    };
    DzRoutine       entry;
    intptr_t        context;
};

struct _DzMemExTag
{
    int             hostId;
    int             reserve;
};

#endif // __DzStructs_h__
