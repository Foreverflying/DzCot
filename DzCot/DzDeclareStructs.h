/**
 *  @file       DzDeclareStructs.h
 *  @brief      
 *  @author	    Foreverflying <foreverflying@live.cn>
 *  @date       2011/10/06
 *
 */

#ifndef __DzDeclareStructs_h__
#define __DzDeclareStructs_h__

#include "DzInc.h"

//used for cache align to avoid false sharing
struct _DzCacheChunk
{
    char        _padding[ CPU_CACHE_ALIGN ];
};

typedef struct _DzCacheChunk    DzCacheChunk;

//define in DzStructsList.h
struct _DzLItr;
struct _DzSList;
struct _DzDLItr;
struct _DzDList;
struct _DzLNode;

typedef struct _DzLItr          DzLItr;
typedef struct _DzSList         DzSList;
typedef struct _DzDLItr         DzDLItr;
typedef struct _DzDList         DzDList;
typedef struct _DzLNode         DzLNode;

//define in DzStructs.h
struct _DzTimerNode;
struct _DzEasyEvt;
struct _DzFastEvt;
struct _DzSynObj;
struct _DzWaitNode;
struct _DzWaitHelper;
struct _DzRmtCotFifo;
struct _DzHost;
struct _DzHostsMgr;
struct _DzSysParam;
struct _DzWorker;
struct _DzMemExTag;

typedef struct _DzTimerNode     DzTimerNode;
typedef struct _DzEasyEvt       DzEasyEvt;
typedef struct _DzFastEvt       DzFastEvt;
typedef struct _DzSynObj        DzSynObj;
typedef struct _DzWaitNode      DzWaitNode;
typedef struct _DzWaitHelper    DzWaitHelper;
typedef struct _DzRmtCotFifo    DzRmtCotFifo;
typedef struct _DzHost          DzHost;
typedef struct _DzHostsMgr      DzHostsMgr;
typedef struct _DzSysParam      DzSysParam;
typedef struct _DzWorker        DzWorker;
typedef struct _DzMemExTag      DzMemExTag;

//define in DzStructsOs.h
struct _DzOsStruct;
struct _DzCot;
struct _DzSysAutoEvt;

typedef struct _DzOsStruct      DzOsStruct;
typedef struct _DzCot           DzCot;
typedef struct _DzSysAutoEvt    DzSysAutoEvt;

//define in DzStructsIoOs.h
struct _DzFd;

typedef struct _DzFd            DzFd;

//alias struct names when using in the interface
typedef struct _DzLNode         DzParamNode;
typedef struct _DzSynObj*       DzHandle;

#endif // __DzDeclareStructs_h__
