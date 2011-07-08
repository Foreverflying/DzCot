/********************************************************************
    created:    2010/02/11 22:06
    file:       DzStructsLnx.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzStructsLnx_h__
#define __DzStructsLnx_h__

#include "../DzStructsList.h"

struct _DzSynObj;
struct _DzAsynIo;

typedef struct _DzOsStruct
{
    int                 epollFd;
    int                 maxFd;
    struct _DzAsynIo**  fdTable;
}DzOsStruct;

typedef struct _DzThread
{
    DzLItr              lItr;
    void*               sp;
    char*               stack;
    int                 sSize;
    int                 lastErr;
}DzThread;

#endif // __DzStructsLnx_h__
