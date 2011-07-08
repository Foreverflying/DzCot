/********************************************************************
    created:    2010/11/22 17:27
    file:       DzBase.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzBase_h__
#define __DzBase_h__

#include "DzStructs.h"

#ifdef __cplusplus
extern "C"{
#endif

inline void SetLastErr( DzHost* host, int err )
{
    host->currThread->lastErr = err;
}

inline int GetLastErr( DzHost* host )
{
    return host->currThread->lastErr;
}

#ifdef __cplusplus
};
#endif

#endif // __DzBase_h__