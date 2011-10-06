/********************************************************************
    created:    2011/10/06 19:53
    file:       DzDeclareStructs.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzDeclareStructs_h__
#define __DzDeclareStructs_h__

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
struct _DzFastEvt;
struct _DzSynObj;
struct _DzWaitNode;
struct _DzWaitHelper;
struct _DzHost;

typedef struct _DzTimerNode     DzTimerNode;
typedef struct _DzFastEvt       DzFastEvt;
typedef struct _DzSynObj        DzSynObj;
typedef struct _DzWaitNode      DzWaitNode;
typedef struct _DzWaitHelper    DzWaitHelper;
typedef struct _DzHost          DzHost;

//define in DzStructsOs.h
struct _DzOsStruct;
struct _DzThread;

typedef struct _DzOsStruct      DzOsStruct;
typedef struct _DzThread        DzThread;

//define in DzStructsIoOs.h
struct _DzAsynIo;

typedef struct _DzAsynIo        DzAsynIo;

//alias struct names when using in the interface
typedef struct _DzLNode         DzParamNode;
typedef struct _DzSynObj*       DzHandle;

#endif // __DzDeclareStructs_h__
