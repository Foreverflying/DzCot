/**
 *  @file       DzStructs.h
 *  @brief      
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzStructs_h__
#define __DzStructs_h__

#include "DzInc.h"
#include "DzDeclareStructs.h"
#include "DzStructsList.h"
#include DZ_OS_FILE( DzStructsOs.h )

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
    int64_t         timestamp;
    int             interval;
    BOOL            repeat;
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
            int64_t     _unusedTimestamp;
            BOOL        notified;
            int         status;
        };
    };
    union{
        DzWaitHelper*   helper;     // for timeout
        DzCot*          dzCot;      // for fast event
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
            int64_t     _unusedTimestamp;
            int         notifyCount;        // for semaphore and event
            int         _unused2;
        };
    };
    int                 ref;
    union{
        struct{
            DzDList     waitQ[ COT_PRIORITY_COUNT ];
        };
        // used for CallbackTimer, must reset
        // the waitQ[ 0 ] and waitQ[ 1 ]
        // when release CallbackTimer
        struct{
            DzEntry     routine;
            intptr_t    context;
            int         priority;
            int         sType;
        };
    };
};

struct _DzWaitNode
{
    union{
        DzLItr      lItr;
        DzDlItr     dlItr;
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
    int volatile*   readPos;
    int volatile*   writePos;
    DzCot**         rmtCotArr;
    DzSList*        pendRmtCot;
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
    int volatile    liveSign;
    int             hostCount;
    int             workerNowDepth;
    int             workerSetDepth;
    DzLItr*         workerPool;
    DzRmtCotFifo*   rmtFifoRes;
    DzCot**         rmtFifoCotArrRes;
    int             lowestPri;
    int             dftPri;
    int             dftSType;
    int             smallStackSize;
    int             middleStackSize;
    int             largeStackSize;
};

struct _DzHost
{
    // the first cache will be used only by the host itself.
    union{
        struct{
            // running cot
            DzCot*          currCot;

            // union centerCot use sp only
            void*           _unused_sp;

            // cot schedule countdown
            int             scheduleCd;

            // host's io reaction rate,
            // check io state after ( ioReactionRate ) cots switches
            int             ioReactionRate;

            // iterator for task lists when scheduling
            int             lowestPri;
            int             currPri;

            // used for timer
            DzTimerNode**   timerHeap;
            int             timerCount;
            int             timerHeapSize;
            int64_t         latestMilUnixTime;

            // current cot count
            int             cotCount;
        };

        // the host thread's original stack info
        DzCot               centerCot;

        // CPU cache align
        DzCacheChunk        _pending1;
    };

    // the second cache align may be read by all hosts.
    // it is read only when hosts are running, so there's no false sharing
    union{
        struct{
            // host's id
            int             hostId;

            // host mask
            int             hostMask;

            // remote cot FIFOs
            DzRmtCotFifo*   rmtFifoArr;

            // remote check sign pointer
            int volatile*   rmtCheckSignPtr;

            // Os struct
            DzOsStruct      os;
        };

        // CPU cache align
        DzCacheChunk        _pending2;
    };

    // the third cache align
    // local access only, frequently

    // schedule tasks' list
    DzSList         taskLs[ COT_PRIORITY_COUNT ];

    // dlmalloc heap
    void*           mSpace;

    // address prefix of handle struct
    intptr_t        handleBase;

    // the fourth and fifth cache align on 64 bit platform
    // resource pools, local access only, frequently

    // DzCot struct pool
    DzLItr*         cotPools[ STACK_TYPE_COUNT ];
    int             cotPoolNowDepth[ STACK_TYPE_COUNT ];
    int             cotStackSize[ STACK_TYPE_COUNT ];
    DzLItr*         cotPool;

    // DzSynObj struct pool
    DzLItr*         synObjPool;

    // DzDqNode struct pool
    DzLItr*         lNodePool;

    // DzFd struct Pool
    DzLItr*         dzFdPool;

    // multi hosts manager
    DzHostsMgr*     mgr;

    // pending remote cots
    DzSList*        pendRmtCot;

    // lazy dispatch remote cots
    DzSList*        lazyRmtCot;

    // lazy free memory list
    DzSList*        lazyFreeMem;

    // the sixth cache align on 64 bit platform

    // lazy checking timer
    DzSynObj*       lazyTimer;

    // memory chunk pool
    char*           memPoolPos;
    char*           memPoolEnd;

    // record pool alloc history
    DzLItr*         poolGrowList;

    // handle struct chunk pool
    char*           handlePoolPos;
    char*           handlePoolEnd;

    // host count local copy,
    // avoid reading global hostCount leads false sharing
    int             hostCount;

    // default cot value
    int             dftPri;
    int             dftSType;

    // the seventh cache align on 64 bit platform begin

    // configure data
    int             cotPoolSetDepth[ STACK_TYPE_COUNT ];

    // debug struct
    __DBG_STRUCT( DzHost )
};

struct _DzSysParam
{
    DzEntry             threadEntry;
    int                 result;
    union{
        struct{
            DzSynObj*   evt;
            DzHostsMgr* hostMgr;
            DzCot*      returnCot;
            int         hostId;
        } hs;   // used for host start
        struct{
            DzEntry     entry;
            intptr_t    context;
        } cs;   // used for cot start
        struct{
            DzEntry     entry;
            intptr_t    context;
            DzCot*      dzCot;
            DzHostsMgr* hostMgr;
        } wk;   // used for worker thread start
    };
};

struct _DzWorker
{
    DzLItr          lItr;
    DzCot*          dzCot;
    DzEntry         entry;
    intptr_t        context;
    DzSysAutoEvt    sysEvt;
};

struct _DzMemExTag
{
    intptr_t        hostId;
};

#endif // __DzStructs_h__
