/**
 *  @file       DzCot.h
 *  @brief      The interface header of DzCot.
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzCot_h__
#define __DzCot_h__

#if defined(_WIN32)
#include <Ws2tcpip.h>
#elif defined(__linux__)
#include <stddef.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <arpa/inet.h>
#endif

/** return type of Create cot and host functions */
enum DzState
{
    DS_OK,
    DS_NO_MEMORY
};

/** Cot priority levels */
enum DzCotPriority
{
    CP_HIGH,
    CP_NORMAL,
    CP_LOW,
    CP_DEFAULT
};

/** Cot stack types */
enum DzStackTypes
{
    ST_MIN,
    ST_US,
    ST_UM,
    ST_UL,
    ST_DEFAULT
};

/** open file flags */
enum DzOpenFlag
{
    DZ_O_RD         =   00000000,
    DZ_O_WR         =   00000001,
    DZ_O_RDWR       =   00000002,
    DZ_O_CREATE     =   00000100,
    DZ_O_EXCL       =   00000200,
    DZ_O_TRUNC      =   00001000,
    DZ_O_APPEND     =   00002000
};

/** seek file flags */
enum DzSeekFlag
{
    DZ_SEEK_SET     =   0,
    DZ_SEEK_CUR     =   1,
    DZ_SEEK_END     =   2
};

/** shutdown socket flags */
enum DzShutFlag
{
    DZ_SHUT_READ    =   0,
    DZ_SHUT_WRITE   =   1,
    DZ_SHUT_BOTH    =   2
};

/** max buffer count when using vectored i/o */
#define DZ_MAX_IOV      64

/** max cot host count */
#define DZ_MAX_HOST     16

/** 64 bytes struct used for passing common parameter data */
typedef struct
{
    union {
        struct {
            int     d8a;
            int     d8;
        };
        void*       p8;
    };
    union {
        struct {
            int     d7a;
            int     d7;
        };
        void*       p7;
    };
    union {
        struct {
            int     d6a;
            int     d6;
        };
        void*       p6;
    };
    union {
        struct {
            int     d5a;
            int     d5;
        };
        void*       p5;
    };
    union {
        struct {
            int     d4a;
            int     d4;
        };
        void*       p4;
    };
    union {
        struct {
            int     d3a;
            int     d3;
        };
        void*       p3;
    };
    union {
        struct {
            int     d2a;
            int     d2;
        };
        void*       p2;
    };
    union {
        struct {
            int     d1a;
            int     d1;
        };
        void*       p1;
    };
} DzParamNode;

/** handle for synchronize objects */
typedef void* DzHandle;

/** DzIov used by vectored i/o
 *  the data field order is different between Windows and Linux
 */
#ifdef _WIN32

typedef intptr_t ssize_t;

typedef struct
{
    unsigned long   len;
    void*           buf;
} DzIov;

#elif defined __linux__

typedef struct
{
    void*           buf;
    size_t          len;
} DzIov;

typedef int BOOL;

#undef  TRUE
#undef  FALSE
#define TRUE    1
#define FALSE   0

#endif

#if defined(__linux__)
#if defined(__i386)
#define __stdcall __attribute__((stdcall))
#else
#define __stdcall
#endif
#endif

/** Cot entry function type */
typedef void (__stdcall *DzEntry)(intptr_t context);

#define CotEntry    void __stdcall 

/** default parameter used in c++ */
#ifndef __cplusplus
#define __DZ_DFT_ARG(x)
#else
#define __DZ_DFT_ARG(x) = (x)
extern "C"{
#endif

/** DzRunHosts
 *  starts cot hosts and blocks current thread.
 *  @param hostCount
 *      How many cot hosts you want to create.
 *      0 <= hostCount <= DZ_MAX_HOST, when hostCount == 0,
 *      a single cot host without worker support will be used,
 *      it will slightly improve performance, but you can not use
 *      worker thread, neither DzGetNameInfo nor DzGetAddrInfo.
 *  @param smallStackSize
 *      The cot stack size for cots that sType is ST_US.
 *      0 < smallStackSize <= 8192 * 1024 * 1024, in bytes.
 *  @param middleStackSize
 *      The cot stack size for cots that sType is ST_UM.
 *      0 < middleStackSize <= 8192 * 1024 * 1024, in bytes.
 *  @param largeStackSize
 *      The cot stack size for cots that sType is ST_UL.
 *      0 < largeStackSize <= 8192 * 1024 * 1024, in bytes.
 *  @param lowestPri
 *      Lowest cot priority you can use.
 *      CP_HIGH <= lowestPri <= CP_LOW, smaller value means higher priority.
 *      If you do not use all cot priority, a smaller lowestPriority leads
 *      slightly higher performance.
 *  @param dftPri
 *      Default cot priority for these hosts.
 *      CP_HIGH <= dftPri <= lowestPri.
 *      When creating cot with a priority CP_DEFAULT,
 *      the cot's priority will be set to this value.
 *  @param dftSType
 *      Default cot stack type for these hosts.
 *      ST_MIN <= dftSType <= ST_UL.
 *      when creating cot with a stack type ST_DEFAULT,
 *      the cot's stack type will be set to this value.
 *  @param firstEntry
 *      The first cot entry to be execute.
 *      The parameter context will be passed in.
 *  @param context
 *      The user specified data.
 *  @param cleanEntry
 *      The entry to do some clean job after all cot hosts quit,
 *      if nothing to do it can be NULL.
 *      The parameter context will be passed in.
 *  @return
 *      DS_OK if no error, DS_NO_MEMORY if not enough memory.
 *
 *  This function should be called before all other functions.
 *  It will block current thread, and create cot hosts.
 *  Cot hosts have their own host ids start from 0 like 0, 1, 2 ...
 *  Every host use a system thread,
 *  host 0 uses the "blocked" current thread, and others create their own.
 *  After all hosts created, firstEntry with parameter context will be
 *  executed as the first cot in the cot host 0, and other cot hosts pause.
 *  After all hosts' cot finished, all hosts quit,
 *  cleanEntry with parameter context will be called.
 *  Then in the cot host 0's system thread, DzRunHosts function returns,
 *  and other hosts' system threads are terminated.
 *
 *  @note
 *      Stack size will round up to a boundary, there are two kinds of cots:
 *      1. For stack size less than 16384, the stack memory is full allocated,
 *      when the cot exits, the stack will be pushed to it's stack pool so
 *      it's memory will be never freed. It's stack boundary is 1024.
 *      2. For stack size greater than or equal to 16384, the stack memory
 *      space is reserved, and only several pages on the stack top are
 *      committed, stack memory grows when needed, when the cot exits, if
 *      it's stack pool is full, the committed pages and the memory space will
 *      be freed. It's stack boundary is 65536 in windows, 16384 in linux.
 *  @remarks
 *      hostCount had better not set greater than CPU count,
 *      to avoid frequently system thread switching.
 */
int DzRunHosts(
    int         hostCount,
    int         smallStackSize,
    int         middleStackSize,
    int         largeStackSize,
    int         lowestPri,
    int         dftPri,
    int         dftSType,
    DzEntry     firstEntry,
    intptr_t    context         __DZ_DFT_ARG(0),
    DzEntry     cleanEntry      __DZ_DFT_ARG(NULL)
    );

/** DzStartCot
 *  starts a cot in current cot host.
 *  @param entry
 *      The cot entry to be execute.
 *  @param context
 *      The user specified data.
 *  @param priority
 *      The cot's priority.
 *      CP_HIGH <= priority <= lowestPri assigned in DzRunHosts,
 *      or priority == CP_DEFAULT, cot's priority will be set to
 *      current host's default cot priority.
 *  @param sType
 *      The cot's stack type.
 *      ST_MIN <= sType <= ST_UL,
 *      or sType == ST_DEFAULT, cot's stack type will be set to
 *      current host's default stack type.
 *  @return
 *      DS_OK if no error, DS_NO_MEMORY if not enough memory.
 *
 *  This function will create a cot, and append it to the last of
 *  current host's corresponding schedule queue.
 */
int DzStartCot(
    DzEntry     entry,
    intptr_t    context         __DZ_DFT_ARG(0),
    int         priority        __DZ_DFT_ARG(CP_DEFAULT),
    int         sType           __DZ_DFT_ARG(ST_DEFAULT)
    );

/** DzStartCotInstant
 *  starts a cot in current cot host and schedule to it immediately.
 *  @param entry
 *      The cot entry to be execute.
 *  @param context
 *      The user specified data.
 *  @param priority
 *      The cot's priority.
 *      CP_HIGH <= priority <= lowestPri assigned in DzRunHosts,
 *      or priority == CP_DEFAULT, cot's priority will be set to
 *      current host's default cot priority.
 *  @param sType
 *      The cot's stack type.
 *      ST_MIN <= sType <= ST_UL,
 *      or sType == ST_DEFAULT, cot's stack type will be set to
 *      current host's default stack type.
 *  @return
 *      DS_OK if no error, DS_NO_MEMORY if not enough memory.
 *
 *  This function will create a cot, and schedule it immediately in current
 *  host, despite of it's priority at this time.
 *  Current cot will be put to the top of current host's schedule queue,
 *  so, latter it will be scheduled before all other cots.
 */
int DzStartCotInstant(
    DzEntry     entry,
    intptr_t    context         __DZ_DFT_ARG(0),
    int         priority        __DZ_DFT_ARG(CP_DEFAULT),
    int         sType           __DZ_DFT_ARG(ST_DEFAULT)
    );


/** DzEvtStartCot
 *  starts a cot in current cot host, after it finished, notifies an event.
 *  @param evt
 *      An synchronization event object, when the created cot finished,
 *      the event will be notified once.
 *      the event can be a manual, auto or countdown event.
 *  @param entry
 *      The cot entry to be execute.
 *  @param context
 *      The user specified data.
 *  @param priority
 *      The cot's priority.
 *      CP_HIGH <= priority <= lowestPri assigned in DzRunHosts,
 *      or priority == CP_DEFAULT, cot's priority will be set to
 *      current host's default cot priority.
 *  @param sType
 *      The cot's stack type.
 *      ST_MIN <= sType <= ST_UL,
 *      or sType == ST_DEFAULT, cot's stack type will be set to
 *      current host's default stack type.
 *  @return
 *      DS_OK if no error, DS_NO_MEMORY if not enough memory.
 *
 *  This function will create a cot, and append it to the last of
 *  current host's corresponding schedule queue.
 */
int DzEvtStartCot(
    DzHandle    evt,
    DzEntry     entry,
    intptr_t    context         __DZ_DFT_ARG(0),
    int         priority        __DZ_DFT_ARG(CP_DEFAULT),
    int         sType           __DZ_DFT_ARG(ST_DEFAULT)
    );

/** DzEvtStartCotInstant
 *  starts a cot in current cot host and schedule to it immediately,
 *  after it finished, notifies an event.
 *  @param evt
 *      An synchronization event object, when the created cot finished,
 *      the event will be notified once.
 *      the event can be a manual, auto or countdown event.
 *  @param entry
 *      The cot entry to be execute.
 *  @param context
 *      The user specified data.
 *  @param priority
 *      The cot's priority.
 *      CP_HIGH <= priority <= lowestPri assigned in DzRunHosts,
 *      or priority == CP_DEFAULT, cot's priority will be set to
 *      current host's default cot priority.
 *  @param sType
 *      The cot's stack type.
 *      ST_MIN <= sType <= ST_UL,
 *      or sType == ST_DEFAULT, cot's stack type will be set to
 *      current host's default stack type.
 *  @return
 *      DS_OK if no error, DS_NO_MEMORY if not enough memory.
 *
 *  This function will create a cot, and schedule it immediately in current
 *  host, despite of it's priority at this time.
 *  Current cot will be put to the top of current host's schedule queue,
 *  so, latter it will be scheduled before all other cots.
 */
int DzEvtStartCotInstant(
    DzHandle    evt,
    DzEntry     entry,
    intptr_t    context         __DZ_DFT_ARG(0),
    int         priority        __DZ_DFT_ARG(CP_DEFAULT),
    int         sType           __DZ_DFT_ARG(ST_DEFAULT)
    );

/** DzStartRemoteCot
 *  starts a cot in a remote cot host.
 *  @param rmtId
 *      remote host's id.
 *      0 <= rmtId < hosts count
 *      and rmtId != current host's id
 *  @param entry
 *      The cot entry to be execute.
 *  @param context
 *      The user specified data.
 *  @param priority
 *      The cot's priority.
 *      CP_HIGH <= priority <= lowestPri assigned in DzRunHosts,
 *      or priority == CP_DEFAULT, cot's priority will be set to
 *      current host's default cot priority.
 *      Then, if the cot's priority is lower than remote host's
 *      lowest priority, this cot's priority will be set to it.
 *  @param sType
 *      The cot's stack type.
 *      ST_MIN <= sType <= ST_UL,
 *      or sType == ST_DEFAULT, cot's stack type will be set to
 *      current host's default stack type.
 *  @return
 *      DS_OK if no error, DS_NO_MEMORY if not enough memory.
 *
 *  This function will create a cot, and schedule it in a remote host.
 */
int DzStartRemoteCot(
    int         rmtId,
    DzEntry     entry,
    intptr_t    context         __DZ_DFT_ARG(0),
    int         priority        __DZ_DFT_ARG(CP_DEFAULT),
    int         sType           __DZ_DFT_ARG(ST_DEFAULT)
    );

/** DzEvtStartRemoteCot
 *  starts a cot in a remote cot host, after it finished, notifies an event.
 *  @param evt
 *      An synchronization event object, when the created cot finished,
 *      the event will be notified once.
 *      the event can be a manual, auto or countdown event.
 *  @param rmtId
 *      remote host's id.
 *      0 <= rmtId < hosts count
 *      and rmtId != current host's id
 *  @param entry
 *      The cot entry to be execute.
 *  @param context
 *      The user specified data.
 *  @param priority
 *      The cot's priority.
 *      CP_HIGH <= priority <= lowestPri assigned in DzRunHosts,
 *      or priority == CP_DEFAULT, cot's priority will be set to
 *      current host's default cot priority.
 *      Then, if the cot's priority is lower than remote host's
 *      lowest priority, this cot's priority will be set to it.
 *  @param sType
 *      The cot's stack type.
 *      ST_MIN <= sType <= ST_UL,
 *      or sType == ST_DEFAULT, cot's stack type will be set to
 *      current host's default stack type.
 *  @return
 *      DS_OK if no error, DS_NO_MEMORY if not enough memory.
 *
 *  This function will create a cot, and schedule it in a remote host.
 */
int DzEvtStartRemoteCot(
    DzHandle    evt,
    int         rmtId,
    DzEntry     entry,
    intptr_t    context         __DZ_DFT_ARG(0),
    int         priority        __DZ_DFT_ARG(CP_DEFAULT),
    int         sType           __DZ_DFT_ARG(ST_DEFAULT)
    );

/** DzRunRemoteCot
 *  starts a cot in a remote cot host, and block current cot,
 *  until the created remote cot finished.
 *  @param rmtId
 *      remote host's id.
 *      0 <= rmtId < hosts count
 *      and rmtId != current host's id
 *  @param entry
 *      The cot entry to be execute.
 *  @param context
 *      The user specified data.
 *  @param priority
 *      The cot's priority.
 *      CP_HIGH <= priority <= lowestPri assigned in DzRunHosts,
 *      or priority == CP_DEFAULT, cot's priority will be set to
 *      current host's default cot priority.
 *      Then, if the cot's priority is lower than remote host's
 *      lowest priority, this cot's priority will be set to it.
 *  @param sType
 *      The cot's stack type.
 *      ST_MIN <= sType <= ST_UL,
 *      or sType == ST_DEFAULT, cot's stack type will be set to
 *      current host's default stack type.
 *  @return
 *      DS_OK if no error, DS_NO_MEMORY if not enough memory.
 *
 *  This function will create a cot, and schedule it in a remote host,
 *  then block current cot, wait the remote cot finished.
 */
int DzRunRemoteCot(
    int         rmtId,
    DzEntry     entry,
    intptr_t    context         __DZ_DFT_ARG(0),
    int         priority        __DZ_DFT_ARG(CP_DEFAULT),
    int         sType           __DZ_DFT_ARG(ST_DEFAULT)
    );

/** DzRunWorker
 *  starts a worker system thread to do some long period work,
 *  and block current cot, until the worker system thread finished.
 *  @param entry
 *      The cot entry to be execute in the worker system thread.
 *  @param context
 *      The user specified data.
 *  @return
 *      DS_OK if no error, DS_NO_MEMORY if not enough memory.
 *
 *  This function will create a worker system thread,
 *  then block current cot, wait the worker system thread finished.
 *
 *  @remarks
 *      the worker system thread's stack size is hard coded to 64k.
 */
int DzRunWorker(
    DzEntry     entry,
    intptr_t    context         __DZ_DFT_ARG(0)
    );

/** DzGetCotData
 *  gets current cot's local storage data.
 *  @return
 *      The local storage data of current cot.
 */
intptr_t DzGetCotData();

/** DzSetCotData
 *  sets the current cot's local storage data.
 *  @param data
 *      The data to set.
 */
void DzSetCotData(intptr_t data);

/** DzGetCotCount
 *  gets current host's cot count.
 *  @return
 *      The cot count of current host.
 */
int DzGetCotCount();

/** DzGetHostId
 *  gets current host's id.
 *  @return
 *      The current host's id;
 */
int DzGetHostId();

/** DzSetPriority
 *  changes current cot's priority.
 *  @param priority
 *      The new priority value.
 *      priority <= current host's lowest cot priority.
 *  @return
 *      The old priority value.
 *
 *  @remarks
 *      if priority < 0, the cot's priority doesn't change,
 *      use this way to fetch the priority of current cot.
 */
int DzSetPriority(int priority);

/** DzSetCotPoolDepth
 *  sets current host's cot pool's depth.
 *  @param sType
 *      The stack type of cot which you want to set.
 *      ST_MIN <= sType <= ST_UL.
 *  @param depth
 *      The new cot pool depth value.
 *      depth < 1024 * 1024 * 1024.
 *  @return
 *      The old cot pool depth value.
 *
 *  By avoiding cot stack's allocation and release, cot pool make cot's
 *  creation and destruction faster.
 *  Different stack type cots use different cot pools.
 *  when host starting most pools' depth is 0, but the default stack type
 *  cot pool's depth is set to 64.
 *  Also, the cot stack whose size smaller than 16384 will never be released,
 *  so, for cots with these stacks their pool depth is always infinite and
 *  can't be set to other value.
 *
 *  @remarks
 *      if depth < 0, the cot's pool depth doesn't change,
 *      use this way to fetch the current depth of the cot pool.
 */
int DzSetCotPoolDepth(int sType, int depth);

/** DzSetWorkerPoolDepth
 *  sets worker thread pool depth.
 *  @param depth
 *      The new worker thread pool depth value.
 *      0 <= depth <= 1024.
 *  @return
 *      The old worker thread pool depth value.
 *
 *  By avoiding system thread's creation and destruction,
 *  the worker thread pool leads higher performance when using workers.
 *  All cot hosts share the worker pool, when hosts starting the worker
 *  pool depth is 0.
 */
int DzSetWorkerPoolDepth(int depth);

/** DzSetHostParam
 *  sets current cot host's lowest priority, default priority and
 *  default cot stack type.
 *  @param lowestPri
 *      The new lowest priority value of current cot host.
 *      CP_HIGH <= lowestPri <= CP_LOW, if the new value is smaller than
 *      the original value, the lowest cot priority of current host remains
 *      unchanged.
 *  @param dftPri
 *      The new default cot priority value.
 *      dftPri <= lowestPri.
 *  @param defSType
 *      The new default cot stack type value.
 *      defSType <= ST_UL.
 *      If new default stack type cot pool's depth is smaller than 64,
 *      it's depth is set to 64.
 *  @return
 *      The old lowest priority value of current cot host.
 *
 *  @remarks
 *      If dftPri < 0, the default priority of current host doesn't change.
 *      If defSType < 0, the default cot stack type of current host
 *      doesn't change.
 */
int DzSetHostParam(int lowestPri, int dftPri, int dftSType);

/** DzSetHostIoReaction
 *  sets the current host's i/o reaction rate.
 *  @param rate
 *      The new reaction rate value.
 *  @return
 *      The old reaction rate value.
 *
 *  When cot host is busy scheduling lots of cots, it is possible there's
 *  no chance to response i/o, to avoid this, i/o reaction rate is needed.
 *  The rate means, within how many times cot switching, the i/o check must
 *  be implemented. Smaller rate can avoid long i/o delay, but greater rate
 *  leads higher performance. When hosts starting, the rate is 256, in most
 *  real situations, there's no need to change this rate.
 *
 *  @remarks
 *      If rate <= 0, the current host's i/o reaction rate doesn't change,
 *      use this way to fetch the current host's i/o reaction rate.
 */
int DzSetHostIoReaction(int rate);

/** DzWaitSynObj
 *  waits until the synchronize object is signaled state
 *  or timeout interval elapses.
 *  @param obj
 *      The synchronize object that you want to wait for.
 *      A synchronize object can be a mutex, manual event, auto event,
 *      countdown event, semaphore, or wait-able timer.
 *  @param timeout
 *      The timeout interval, in milliseconds, timeout >= -1.
 *      If timeout > 0, the function waits until the object is signaled
 *      or the interval elapses. If timeout == 0, the function does not
 *      enter a wait state if the object is not signaled. If timeout == -1,
 *      the function will return only when the object is signaled.
 *  @return
 *      -1 if function returns due to timeout,
 *      0 if the object signaled.
 *
 *  @remarks
 *      Never wait synchronize objects created or cloned in a different
 *      cot host!
 */
int DzWaitSynObj(
    DzHandle    obj,
    int         timeout         __DZ_DFT_ARG(-1)
    );

/** DzWaitMultiSynObj
 *  waits until one or all specified synchronize objects is signaled state
 *  or timeout interval elapses.
 *  @param count
 *      The length of synchronize objects array, count > 0.
 *  @param objs
 *      The synchronize objects array that you want to wait for.
 *      A synchronize object can be a mutex, manual event, auto event,
 *      countdown event, semaphore, or wait-able timer
 *  @param waitAll
 *      TRUE if you want to wait all objects get signaled,
 *      FALSE if you want to wait any one of the object gets signaled.
 *  @param timeout
 *      The timeout interval, in milliseconds, timeout >= -1.
 *      If timeout > 0, the function waits until the signal condition is
 *      reached or the interval elapses. If timeout == 0, the function
 *      does not enter a wait state if the signal condition is not reached.
 *      If timeout == -1, the function will return only when the signal
 *      condition is reached.
 *  @return
 *      -1 if function returns due to timeout,
 *      0 if wait all objects and all objects get signaled,
 *      or when waitAll == FALSE, the nonnegative return value indicates
 *      the index of the signaled object in the objects array.
 *
 *  @remarks
 *      Never wait synchronize objects created or cloned in a different
 *      cot host!
 */
int DzWaitMultiSynObj(
    int         count,
    DzHandle*   objs,
    BOOL        waitAll,
    int         timeout         __DZ_DFT_ARG(-1)
    );

/** DzCreateMtx
 *  creates a mutex object.
 *  @param owner
 *      If owner == TRUE, the caller cot takes the ownership of the mutex,
 *      else the mutex object doesn't have an owner.
 *  @return
 *      The handle of the mutex.
 *
 *  The state of a mutex object is signaled only when it is not owned by
 *  any cot. Use wait function DzWaitSynObj and DzWaitMultiSynObj to request
 *  the ownership. A mutex object can be owned by only one cot, the owning
 *  cot can release the ownership by calling DzReleaseMtx.
 *
 *  @remarks
 *      Before passing a synchronize object to another cot for using,
 *      you should clone it by calling DzCloneSynObj, or else you need to
 *      carefully ensure it's deletion execute after all using finished.
 *      All the created or cloned synchronize objects need to delete after
 *      using, by calling DzDelSynObj.
 */
DzHandle DzCreateMtx(BOOL owner);

/** DzReleaseMtx
 *  releases the ownership of a mutex object.
 *  @param mtx
 *      The handle of the mutex.
 *  @return
 *      TRUE if succeed or else FALSE.
 *
 *  @remarks
 *      By now, the mutex object doesn't obtain the ownership yet, so this
 *      function always return TRUE even though the caller cot don't own
 *      the mutex. So you must make sure a cot never wait a mutex that
 *      already owned, nor release a mutex that doesn't owned.
 */
BOOL DzReleaseMtx(DzHandle mtx);

/** DzCreateManualEvt
 *  creates a manual event object.
 *  @param notified
 *      If notified == TRUE, the created manual event is initialized to
 *      signaled, or else non-signaled.
 *  @return
 *      The handle of the manual event.
 *
 *  The state of a manual event object keeps unchanged no matter how it be
 *  waited. It's state can be changed to signaled by calling DzSetEvt, and
 *  can be changed to non-signaled by calling DzResetEvt.
 *
 *  @remarks
 *      Before passing a synchronize object to another cot for using,
 *      you should clone it by calling DzCloneSynObj, or else you need to
 *      carefully ensure it's deletion execute after all using finished.
 *      All the created or cloned synchronize objects need to delete after
 *      using, by calling DzDelSynObj.
 */
DzHandle DzCreateManualEvt(BOOL notified);

/** DzCreateAutoEvt
 *  creates an auto event object.
 *  @param notified
 *      If notified == TRUE, the created auto event is initialized to
 *      signaled, or else non-signaled.
 *  @return
 *      The handle of the auto event.
 *
 *  An signaled auto event object will changed to non-signaled after
 *  a successfully waiting on it happened. Also, it's state can be changed
 *  to signaled by calling DzSetEvt, and can be changed to non-signaled
 *  by calling DzResetEvt.
 *
 *  @remarks
 *      Before passing a synchronize object to another cot for using,
 *      you should clone it by calling DzCloneSynObj, or else you need to
 *      carefully ensure it's deletion execute after all using finished.
 *      All the created or cloned synchronize objects need to delete after
 *      using, by calling DzDelSynObj.
 */
DzHandle DzCreateAutoEvt(BOOL notified);

/** DzCreateCdEvt
 *  creates a countdown event object.
 *  @param count
 *      The initial countdown of the event object, count >= 0.
 *  @return
 *      The handle of the countdown event.
 *
 *  A countdown event is signaled only when it's countdown is 0.
 *  Call function DzResetEvt on a countdown event will increase it's
 *  countdown by 1.
 *  Call function DzSetEvt on a countdown event will decrease it's
 *  countdown by 1, when it's countdown > 0.
 *
 *  @remarks
 *      Before passing a synchronize object to another cot for using,
 *      you should clone it by calling DzCloneSynObj, or else you need to
 *      carefully ensure it's deletion execute after all using finished.
 *      All the created or cloned synchronize objects need to delete after
 *      using, by calling DzDelSynObj.
 */
DzHandle DzCreateCdEvt(int count);

/** DzSetEvt
 *  sets the specified event object to signaled.
 *  @param evt
 *      The handle of an event object.
 *      The event can be manual event, auto event or countdown event.
 *  @return
 *      TRUE if succeed or else FALSE.
 *
 *  @see
 *      DzCreateManualEvt, DzCreateAutoEvt, DzCreateCdEvt
 */
BOOL DzSetEvt(DzHandle evt);

/** DzResetEvt
 *  sets the specified event object to non-signaled.
 *  @param evt
 *      The handle of an event object.
 *      The event can be manual event, auto event or countdown event.
 *  @return
 *      TRUE if succeed or else FALSE.
 *
 *  @see
 *      DzCreateManualEvt, DzCreateAutoEvt, DzCreateCdEvt
 */
BOOL DzResetEvt(DzHandle evt);

/** DzCreateSem
 *  creates a semaphore object.
 *  @param count
 *      The initial count of the semaphore, count >= 0.
 *  @return
 *      The handle of the semaphore.
 *
 *  A semaphore remains signaled when it's count > 0. It's count will
 *  decrease by 1 after a successfully waiting on it happened.
 *  Use function DzReleaseSem to increase it's count.
 *
 *  @remarks
 *      Before passing a synchronize object to another cot for using,
 *      you should clone it by calling DzCloneSynObj, or else you need to
 *      carefully ensure it's deletion execute after all using finished.
 *      All the created or cloned synchronize objects need to delete after
 *      using, by calling DzDelSynObj.
 */
DzHandle DzCreateSem(int count);

/** DzReleaseSem
 *  increases a semaphore's count by a specified amount.
 *  @param sem
 *      The handle of the semaphore.
 *  @param count
 *      The amount you want to increase, count > 0.
 *  @return
 *      The current count of the semaphore.
 */
int DzReleaseSem(DzHandle sem, int count);

/** DzCreateTimer
 *  creates a wait-able timer object.
 *  @param milSec
 *      The interval of the timer, in milliseconds, milSec > 0.
 *  @param repeat
 *      TRUE to create a repeated timer,
 *      FALSE to create a one-off timer.
 *  @return
 *      The handle of the wait-able timer.
 *
 *  A repeated timer gets notified periodically, the state of a repeated
 *  timer remains non-signaled, but all cots waiting on it will get active
 *  every time the interval elapses.
 *  A one-off timer gets notified for once, the state of the timer remains
 *  non-signaled until it's interval elapses, then the state keeps signaled.
 *
 *  @remarks
 *      Before passing a synchronize object to another cot for using,
 *      you should clone it by calling DzCloneSynObj, or else you need to
 *      carefully ensure it's deletion execute after all using finished.
 *      All the created or cloned synchronize objects need to delete after
 *      using, by calling DzDelSynObj.
 */
DzHandle DzCreateTimer(int milSec, BOOL repeat);

/** DzIsNotified
 *  check whether a synchronize object is notified.
 *  @param obj
 *      The handle of the synchronize object.
 *  @return
 *      TRUE if notified or else FALSE.
 */
BOOL DzIsNotified(DzHandle obj);

/** DzCloneSynObj
 *  clones a handle of a synchronize object.
 *  @param obj
 *      The handle of the synchronize object.
 *  @return
 *      The handle of the synchronize object.
 *
 *  Before passing a synchronize object to another cot for using, you
 *  should clone it, or else you need to carefully ensure it's deletion
 *  execute after all using finished.
 */
DzHandle DzCloneSynObj(DzHandle obj);

/** DzDelSynObj
 *  deletes a synchronize object.
 *  @param obj
 *      The handle of the synchronize object.
 *  @return
 *      TRUE if succeed or else FALSE.
 *
 *  All the created or cloned synchronize objects need to delete after using.
 */
BOOL DzDelSynObj(DzHandle obj);

/** DzCreateCallbackTimer
 *  creates a callback timer.
 *  @param milSec
 *      The interval of the callback timer, in milliseconds, milSec > 0.
 *  @param repeat
 *      If repeat == TRUE, the callback cot will be created periodically,
 *      if repeat == FALSE, the callback cot will be created once.
 *  @param callback
 *      The callback cot entry.
 *  @param context
 *      The user specified data.
 *  @param priority
 *      The callback cot's priority.
 *      CP_HIGH <= priority <= lowestPri assigned in DzRunHosts,
 *      or priority == CP_DEFAULT, cot's priority will be set to
 *      current host's default cot priority.
 *  @param sType
 *      The callback cot's stack type.
 *      ST_MIN <= sType <= ST_UL,
 *      or sType == ST_DEFAULT, cot's stack type will be set to
 *      current host's default stack type.
 *  @return
 *      The handle of the callback timer.
 *
 *  A callback timer will create a new cot in current cot host when
 *  specified interval elapses. According to parameter repeat, this
 *  will happen once or periodically. All created callback timer need
 *  to be deleted after using by calling function DzDelCallbackTimer.
 *
 *  @remarks
 *      A callback timer is not a synchronize object, be sure you never use
 *      wait functions on a callback timer, it can not be cloned neither.
 */
DzHandle DzCreateCallbackTimer(
    int         milSec,
    BOOL        repeat,
    DzEntry     callback,
    intptr_t    context         __DZ_DFT_ARG(0),
    int         priority        __DZ_DFT_ARG(CP_DEFAULT),
    int         sType           __DZ_DFT_ARG(ST_DEFAULT)
    );

/** DzDelCallbackTimer
 *  deletes a callback timer.
 *  @param timer
 *      The handle of the callback timer.
 *  @return
 *      TRUE if succeed or else FALSE.
 *
 *  After a callback timer's deletion, no more callback cot will be
 *  created or executed.
 */
BOOL DzDelCallbackTimer(DzHandle timer);

/** DzSleep
 *  suspends current cot for a specified period.
 *  @param milSec
 *      The interval you want to suspend the current cot, in milliseconds.
 *      milSec >= 0;
 *
 *  @remarks
 *      If milSec == 0, the current cot's executing will be dalayed
 *      instead of suspended, the cot will continue executing after
 *      other cots with same priority scheduled. Cots with lower priority
 *      will never scheduled until all higher priority cots suspended.
 */
void DzSleep(int milSec);

/** DzSocket
 *  creates a socket.
 *  @param domain
 *      The protocol family which will be used for communication.
 *      AF_INET for IPv4,
 *      AF_INET6 for IPv6.
 *      More options see Linux man page for socket.
 *  @param type
 *      The communication semantics.
 *      SOCK_STREAM for connection-based stream,
 *      SOCK_DGRAM for datagrams,
 *      SOCK_RAW for raw network protocol access.
 *      More options see Linux man page for socket.
 *  @param protocol
 *      A particular protocol to be used with the socket.
 *      Can be 0, it will automatically specified according to
 *      domain and type.
 *      .e.g IPPROTO_TCP, IPPROTO_UDP, IPPROTO_ICMP, IPPROTO_SCTP.
 *      More options see Linux man page for socket.
 *  @return
 *      If succeed, the return value is the socket's file descriptor,
 *      or else the return value is -1.
 *
 *  @remarks
 *      The returned file descriptor is not the same as the system's,
 *      so, only use it with DzCot socket operate functions.
 *      Also, it can be used only in the cot host which creates it,
 *      never pass it to a remote cot host.
 */
int DzSocket(int domain, int type, int protocol);

/** DzRawSocket
 *  gets the system socket from a file descriptor.
 *  @param
 *      The socket's file descriptor.
 *  @return
 *      The system socket.
 */
intptr_t DzRawSocket(int fd);

/** DzGetSockOpt
 *  gets options on sockets.
 *  @param fd
 *      The socket's file descriptor.
 *  @param level
 *      The level at which the option is defined,
 *      .e.g SOL_SOCKET, IPPROTO_TCP.
 *      More options see Linux man page for getsockopt.
 *  @param name
 *      The socket option for which the value is to be set.
 *      .e.g SO_KEEPALIVE on level SOL_SOCKET,
 *      TCP_NODELAY on level IPPROTO_TCP.
 *      More options see Linux man page for getsockopt.
 *  @param [out] option
 *      A pointer to the buffer in which the value for the
 *      requested option is to be returned.
 *  @param [in,out] len
 *      A pointer to the size, in bytes, of the option buffer.
 *  @return
 *      0 if succeed or else -1.
 */
int DzGetSockOpt(int fd, int level, int name, void* option, socklen_t* len);

/** DzSetSockOpt
 *  sets options on sockets.
 *  @param fd
 *      The socket's file descriptor.
 *  @param level
 *      The level at which the option is defined,
 *      .e.g SOL_SOCKET, IPPROTO_TCP.
 *      More options see Linux man page for getsockopt.
 *  @param name
 *      The socket option for which the value is to be set.
 *      .e.g SO_KEEPALIVE on level SOL_SOCKET,
 *      TCP_NODELAY on level IPPROTO_TCP.
 *      More options see Linux man page for getsockopt.
 *  @param option
 *      A pointer to the buffer in which the value for the
 *      requested option is specified.
 *  @param len
 *      The size, in bytes, of the buffer pointed to by the
 *      option parameter.
 *  @return
 *      0 if succeed or else -1.
 *
 *  @remarks
 *      The socket option SO_LINGER can not be assigned.
 */
int DzSetSockOpt(int fd, int level, int name, const void* option, socklen_t len);

/** DzGetSockName
 *  retrieves the local name for a socket.
 *  @param fd
 *      The socket's file descriptor.
 *  @param [out] addr
 *      Pointer to a SOCKADDR structure that receives the address (name)
 *      of the socket.
 *  @param [in,out] addrLen
 *      Size of the addr buffer, in bytes.
 *  @return
 *      0 if succeed or else -1.
 */
int DzGetSockName(int fd, struct sockaddr* addr, socklen_t* addrLen);

/** DzGetPeerName
 *  retrieves the address of the peer to which a socket is connected.
 *  @param fd
 *      The socket's file descriptor.
 *  @param [out] addr
 *      Pointer to a SOCKADDR structure that receives the address (name)
 *      of the peer.
 *  @param [in,out] addrLen
 *      Size of the addr buffer, in bytes.
 *  @return
 *      0 if succeed or else -1.
 */
int DzGetPeerName(int fd, struct sockaddr* addr, socklen_t* addrLen);

/** DzBind
 *  associates a local address with a socket.
 *  @param fd
 *      The socket's file descriptor.
 *  @param addr
 *      A pointer to a sockaddr structure of the local address to
 *      assign to the bound socket.
 *  @param addrLen
 *      The length, in bytes, of the value pointed to by the addr parameter.
 *  @return
 *      0 if succeed or else -1.
 */
int DzBind(int fd, const struct sockaddr* addr, socklen_t addrLen);

/** DzListen
 *  places a socket in a state in which it is listening for an
 *  incoming connection.
 *  @param fd
 *      The socket's file descriptor.
 *  @param backlog
 *      The maximum length of the queue of pending connections.
 *      If set to SOMAXCONN, the underlying service provider responsible
 *      will set the backlog to a maximum reasonable value.
 *  @return
 *      0 if succeed or else -1.
 */
int DzListen(int fd, int backlog);

/** DzShutdown
 *  disables sends or receives on a socket.
 *  @param fd
 *      The socket's file descriptor.
 *  @param how
 *      A flag that describes what types of operation will
 *      no longer be allowed.
 *      When how = DZ_SHUT_READ == 0, shutdown send operations.
 *      When how = DZ_SHUT_WRITE == 1, shutdown receive operations.
 *      When how = DZ_SHUT_BOTH == 2, shutdown send and receive operations.
 *  @return
 *      0 if succeed or else -1.
 *
 *  Shutdown writing before close will ensure all data in the socket's
 *  send buffer flush to the target, rather than dropping all data when
 *  call CloseSocket directly.
 *
 *  @note
 *      There's a bit of difference between Windows and Linux.
 *      Trying to receive data on a socket that receiving has been disabled
 *      will leads an error, the receive function will return -1 on
 *      Windows and return 0 on Linux.
 *  @remarks
 *      Never shutdown reading when a socket is reading, and never
 *      shutdown writing when a socket is writing.
 */
int DzShutdown(int fd, int how);

/** DzConnect
 *  establishes a connection to a specified socket.
 *  @param fd
 *      The socket's file descriptor.
 *  @param addr
 *      A pointer to the sockaddr structure to which the connection
 *      should be established.
 *  @param addrLen
 *      The length, in bytes, of the sockaddr structure pointed to
 *      by the addr parameter.
 *  @return
 *      0 if succeed or else -1.
 */
int DzConnect(int fd, const struct sockaddr* addr, socklen_t addrLen);

/** DzAccept
 *  permits an incoming connection attempt on a socket.
 *  @param fd
 *      The socket's file descriptor.
 *  @param [out] addr
 *      An optional pointer to a buffer that receives the address
 *      of the connecting entity, as known to the communications layer.
 *      The exact format of the addr parameter is determined by the
 *      address family that was established when the socket from the
 *      sockaddr structure was created.
 *  @param [in,out] addrLen
 *      An optional pointer to an integer that contains the length of
 *      structure pointed to by the addr parameter.
 *  @return
 *      If no error occurs, it returns a file descriptor for the new socket,
 *      on which the actual connection is made.
 *      Otherwise, a value -1 is returned.
 *
 *  @remarks
 *      The returned file descriptor is not the same as the system's,
 *      so, only use it with DzCot socket operate functions.
 *      Also, it can be used only in the cot host which creates it,
 *      never pass it to a remote cot host.
 */
int DzAccept(int fd, struct sockaddr* addr, socklen_t* addrLen);

/** DzIovSend
 *  sends data in vectored i/o mode on a socket.
 *  @param fd
 *      The socket's file descriptor.
 *  @param bufs
 *      A pointer to an array of DzIov structures. Each DzIov structure
 *      contains a pointer to a buffer and the length, in bytes,
 *      of the buffer.
 *  @param bufCount
 *      The number of DzIov structures in the bufs array,
 *      0 < bufCount <= DZ_MAX_IOV.
 *  @param flags
 *      The flags used to modify the behavior of the DzIovSend function call.
 *      For more information, see Linux man page for send.
 *  @return
 *      On success, the number of bytes sent is returned.
 *      It is not an error if this number is smaller than the total buffer
 *      size (when sending on a stream socket), On error, -1 is returned.
 *
 *  DzIovSend is mainly used for sending data on a connected stream socket,
 *  but it can also send data on a datagram socket.
 *  For more information see Linux man page for send.
 *
 *  @remarks
 *      Never execute more than one reading or writing (.e.g read in two
 *      active cots) on a socket.
 *      But you can read and write simultaneously (.e.g read and write in
 *      two active cots), for socket is a full duplex device.
 */
int DzIovSend(int fd, DzIov* bufs, size_t bufCount, int flags);

/** DzIovRecv
 *  receives data in vectored i/o mode on a socket.
 *  @param fd
 *      The socket's file descriptor.
 *  @param [in,out] bufs
 *      A pointer to an array of DzIov structures. Each DzIov structure
 *      contains a pointer to a buffer and the length, in bytes,
 *      of the buffer.
 *  @param bufCount
 *      The number of DzIov structures in the bufs array,
 *      0 < bufCount <= DZ_MAX_IOV.
 *  @param flags
 *      The flags used to modify the behavior of the DzIovRecv function call.
 *      For more information, see Linux man page for recv.
 *  @return
 *      On success, the number of bytes sent is returned.
 *      If remote host closes the connection, 0 is returned.
 *      On error, -1 is returned.
 *
 *  DzIovRecv is mainly used for receiving data on a connected stream socket,
 *  but it can also receive data on a datagram socket.
 *  For more information see Linux man page for recv.
 *
 *  @remarks
 *      Never execute more than one reading or writing (.e.g read in two
 *      active cots) on a socket.
 *      But you can read and write simultaneously (.e.g read and write in
 *      two active cots), for socket is a full duplex device.
 */
int DzIovRecv(int fd, DzIov* bufs, size_t bufCount, int flags);

int DzSend(int fd, const void* buf, size_t len, int flags);
int DzRecv(int fd, void* buf, size_t len, int flags);
int DzIovSendTo(
    int                     fd,
    DzIov*                  bufs,
    size_t                  bufCount,
    int                     flags,
    const struct sockaddr*  to,
    socklen_t               tolen
    );
int DzIovRecvFrom(
    int                     fd,
    DzIov*                  bufs,
    size_t                  bufCount,
    int                     flags,
    struct sockaddr*        from,
    socklen_t*              fromlen
    );
int DzSendTo(
    int                     fd,
    const void*             buf,
    size_t                  len,
    int                     flags,
    const struct sockaddr*  to,
    socklen_t               tolen
    );
int DzRecvFrom(
    int                     fd,
    void*                   buf,
    size_t                  len,
    int                     flags,
    struct sockaddr*        from,
    socklen_t*              fromlen
    );
int DzGetNameInfo(
    const struct sockaddr*  sa,
    socklen_t               salen,
    char*                   host,
    size_t                  hostlen,
    char*                   serv,
    size_t                  servlen,
    int                     flags
    );
int DzGetAddrInfo(
    const char*             node,
    const char*             service,
    const struct addrinfo*  hints,
    struct addrinfo**       res
    );
void DzFreeAddrInfo(struct addrinfo *res);
int DzInetPton(int af, const char* src, void* dst);
const char* DzInetNtop(int af, const void* src, char* dst, socklen_t size);

/** DzOpen
 *  creates or opens a file.
 *  @param fileName
 *      A 0 terminated string represents the file's full path or
 *      relative path.
 *  @param flags
 *      Bit masks specifying how to open the file.
 *  @return
 *      If succeed, the return value is the file's descriptor,
 *      or else the return value is -1.
 *
 *  The argument flags must include one of the following access modes:
 *  DZ_O_RD, DZ_O_WR, or DZ_O_RDWR. These request opening the file
 *  read-only, write-only, or read/write, respectively.
 *
 *  In addition, zero or more file creation flags and file status flags
 *  can be combined to flags with bitwise-or.
 *  DZ_O_CREATE:
 *      If the file does not exist it will be created.
 *  DZ_O_EXCL:
 *      Only valid when combined with DZ_O_CREATE.
 *      If the file exists, open will failed.
 *  DZ_O_TRUNC:
 *      If file exists and writable open succeed, the file will be
 *      truncated to length 0.
 *  DZ_O_APPEND:
 *      If file exists and writable open succeed, the file offset is
 *      positioned at the end of the file.
 *
 *  @remarks
 *      The returned file descriptor is not the same as the system's,
 *      so, only use it with DzCot file operate functions. Also, it can
 *      be used only in the cot host which creates it, never pass it to
 *      a remote cot host.
 */
int DzOpen(const char* fileName, int flags);

/** DzRead
 *  reads data from an opened file.
 *  @param fd
 *      The file's descriptor.
 *  @param [out] buf
 *      The buffer to retrieve data.
 *  @param count
 *      The byte size of the buffer.
 *  @return
 *      On success, the number of bytes read is returned
 *      (zero indicates end of file), and the file position
 *      is advanced by this number. It is not an error if this number
 *      is smaller than the number of bytes requested.
 *      On error, -1 is returned.
 *
 *  DzRead can also be used for receiving data on a socket, both
 *  connect stream and diagram. See Linux man page for read.
 *
 *  @remarks
 *      Never execute more than one reading or writing (.e.g read in two
 *      active cots) on any device descriptor.
 *      Never read and write simultaneously (.e.g read and write in two
 *      active cots) on a disk file (but you can do this on a full duplex
 *      device descriptor such as a socket).
 */
ssize_t DzRead(int fd, void* buf, size_t count);

/** DzWrite
 *  writes data to an opened file.
 *  @param fd
 *      The file's descriptor.
 *  @param buf
 *      The buffer containing the data.
 *  @param count
 *      The byte size of the data buffer.
 *  @return
 *      On success, the number of bytes written is returned, and the
 *      file position is advanced by this number. It is not an error
 *      if this number is smaller than the number of bytes requested.
 *      On error, -1 is returned.
 *
 *  DzWrite can also be used for sending data on a socket, both
 *  connected stream and datagram.
 *  For more information see Linux man page for write.
 *
 *  @remarks
 *      Never execute more than one reading or writing (.e.g read in two
 *      active cots) on a file descriptor.
 *      Never read and write simultaneously (.e.g read and write in two
 *      active cots) on a disk file (but you can do this on a full duplex
 *      device descriptor such as a socket).
 */
ssize_t DzWrite(int fd, const void* buf, size_t count);

/** DzSeek
 *  sets the file's read/write position.
 *  @param fd
 *      The file's descriptor.
 *  @param offset
 *      Number of bytes from whence.
 *  @param whence
 *      If SEEK_SET, the position is set to offset bytes,
 *      else if SEEK_CUR, the position is set to its current
 *      location plus offset bytes, else if SEEK_CUR, the position
 *      is set to its current location plus offset bytes.
 *  @return
 *      On succeed, The resulting offset location as measured in bytes
 *      from the beginning of the file.
 *      On error, -1 is returned.
 *
 *  @remarks
 *      Never set a file's position while reading or writing on it
 *      (.e.g reading in a active cot and seek it in another active cot).
 */
size_t DzSeek(int fd, ssize_t offset, int whence);

/** DzFileSize
 *  gets the file's size.
 *  @param fd
 *      The file's descriptor.
 *  @return
 *      The file's size in bytes.
 */
size_t DzFileSize(int fd);

intptr_t DzGetFdData(int fd);
void DzSetFdData(int fd, intptr_t data);

/** DzClose
 *  closes a socket or an opened file.
 *  @param fd
 *      The socket or file's descriptor.
 *  @return
 *      0 if succeed or else -1.
 */
int DzClose(int fd);

DzParamNode* DzAllocParamNode();
void DzFreeParamNode(DzParamNode* node);

void* DzAllocPermanentChunk(size_t size);
void* DzPageAlloc(size_t size);
void DzPageFree(void* p, size_t size);
void* DzMalloc(size_t size);
void DzFree(void* p);
void* DzMallocEx(size_t size);
void DzFreeEx(void* p);

unsigned long long DzUnixTime();
unsigned long long DzMilUnixTime();
unsigned long long DzLatestMilUnixTime();

int __DzDbgLastErr();
int __DzDbgMaxStackUse(int sType);
int __DzDbgSynObjLeak();
int __DzDbgFdLeak();
int __DzDbgHeapLeak();
int __DzDbgParamNodeLeak();

#ifdef _WIN32

int DzGetNameInfoW(
    const struct sockaddr*  sa,
    int                     salen,
    wchar_t*                host,
    size_t                  hostlen,
    wchar_t*                serv,
    size_t                  servlen,
    int                     flags
    );
int DzGetAddrInfoW(
    const wchar_t*          node,
    const wchar_t*          service,
    const struct addrinfoW* hints,
    struct addrinfoW**      res
    );
void DzFreeAddrInfoW(struct addrinfoW *res);
int DzInetPtonW(int af, const wchar_t* src, void* dst);
const wchar_t* DzInetNtopW(int af, const void* src, wchar_t* dst, socklen_t size);
int DzOpenW(const wchar_t* fileName, int flags);

#endif

#if defined(_WIN32) && defined(UNICODE)

#define DzOpenT             DzOpenW
#define DzGetNameInfoT      DzGetNameInfoW
#define DzGetAddrInfoT      DzGetAddrInfoW
#define DzFreeAddrInfoT     DzFreeAddrInfoW
#define DzInetPtonT         DzInetPtonW
#define DzInetNtopT         DzInetNtopW

#else

#define DzOpenT             DzOpen
#define DzGetNameInfoT      DzGetNameInfo
#define DzGetAddrInfoT      DzGetAddrInfo
#define DzFreeAddrInfoT     DzFreeAddrInfo
#define DzInetPtonT         DzInetPton
#define DzInetNtopT         DzInetNtop

#endif

#ifdef __cplusplus
};
#endif

#endif // __DzCot_h__
