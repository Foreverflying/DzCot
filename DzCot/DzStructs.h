/********************************************************************
    created:    2010/02/11 22:05
    file:       DzStructs.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzStructs_h__
#define __DzStructs_h__

#include "DzInc.h"
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
    int             interval;       //should set minus
    int             repeat;
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
            int         _unused1;
            int64       _unusedTimestamp;
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
            int         _unused1;
            int64       _unusedTimestamp;
            int         notifyCount;        //for semaphore and event
            int         _unused2;
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
    int volatile*   readPos;
    int volatile*   writePos;
    DzCot**         rmtCotArr;
};

struct _DzShareConstant
{
    DzRmtCotFifo    rmtFifo;
};

struct _DzHostsMgr
{
    DzHost*         hostArr[ DZ_MAX_HOST ];
    DzSysAutoEvt    sysAutoEvt[ DZ_MAX_HOST ];
    int volatile    rmtCheckSign[ DZ_MAX_HOST ];
    int volatile    rmtWritePos[ DZ_MAX_HOST ][ DZ_MAX_HOST ];
    int volatile    rmtReadPos[ DZ_MAX_HOST ][ DZ_MAX_HOST ];
    int volatile    exitSign;
    int             hostCount;
    int             workerNowDepth;
    int             workerSetDepth;
    DzLItr*         workerPool;
    DzRmtCotFifo*   rmtFifoRes;
    int*            servMask;
};

struct _DzHost
{
    //the first cache align is mutable
    //and will be used only by the host itself.
    //for currCot have to stay at the head
    union{
        struct{
            //running cot
            DzCot*          currCot;

            //union centerCot use sp only
            void*           _unused_sp;

            //current cot count
            int             cotCount;

            //cot schedule countdown
            int             scheduleCd;

            //default cot value
            int             dftPri;
            int             dftSSize;

            //used for timer
            DzTimerNode**   timerHeap;
            int             timerCount;
            int             timerHeapSize;

            //iterator for task lists when scheduling
            int             lowestPri;
            int             currPri;

            //dlmalloc heap
            void*           mSpace;
        };

        //the host thread's original stack info
        DzCot               centerCot;

        //CPU cache align
        DzCacheChunk        _pending1;
    };

    //the second cache align may be read by all hosts.
    //it is read only when hosts are running, so there's no false sharing
    union{
        struct{
            //host's id
            int             hostId;

            //host mask
            int             hostMask;

            //remote cot FIFOs
            DzRmtCotFifo*   rmtFifoArr;

            //remote check sign pointer
            int volatile*   rmtCheckSignPtr;

            //Os struct
            DzOsStruct      os;
        };

        //CPU cache align
        DzCacheChunk        _pending2;
    };

    //the third cache align
    //local access only, frequently

    //schedule tasks' list
    DzSList         taskLs[ COT_PRIORITY_COUNT ];

    //multi hosts manager
    DzHostsMgr*     mgr;

    //address prefix of handle struct
    intptr_t        handleBase;

    //the fourth and fifth cache align on 64 bit platform
    //resource pools, local access only, frequently

    //DzCot struct pool
    DzLItr*         cotPools[ STACK_SIZE_COUNT ];
    int             cotPoolNowDepth[ STACK_SIZE_COUNT ];
    DzLItr*         cotPool;

    //DzSynObj struct pool
    DzLItr*         synObjPool;

    //DzDqNode struct pool
    DzLItr*         lNodePool;

    //DzFd struct Pool
    DzLItr*         dzFdPool;

    //the sixth cache align on 64 bit platform

    //checking FIFO chain
    DzRmtCotFifo*   checkFifo;

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

    //the seventh cache align on 64 bit platform begin

    //handle like struct chunk pool
    char*           handlePoolPos;
    char*           handlePoolEnd;

    //host count and serve mask local copy,
    //avoid reading global hostCount leads false sharing
    int             hostCount;
    int             servMask;

    //configure data
    int             cotPoolSetDepth[ STACK_SIZE_COUNT ];
};

struct _DzSysParam
{
    DzRoutine           threadEntry;
    int                 result;
    union{
        struct{
            DzSynObj*   evt;
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
        struct{
            DzRoutine   entry;
            intptr_t    context;
            DzCot*      dzCot;
            DzHostsMgr* hostMgr;
        } wk;   //used for worker thread start
    };
};

struct _DzWorker
{
    DzLItr          lItr;
    DzCot*          dzCot;
    DzRoutine       entry;
    intptr_t        context;
    DzSysAutoEvt    sysEvt;
};

struct _DzMemExTag
{
    int             hostId;
    int             reserve;
};

#endif // __DzStructs_h__
