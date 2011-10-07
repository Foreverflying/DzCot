/********************************************************************
    created:    2011/10/07 18:25
    file:       DzStructsDebug.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzStructsDebug_h__
#define __DzStructsDebug_h__

#include "DzIncOs.h"

#ifndef __DBG_DEBUG_CHECK_MODE

#define __DBG_INFO_DEFINE( structName )

#else

#define __DBG_INFO_DEFINE( structName )\
    struct __DbgInfo##structName   __dbgInfoData;

#define __DBG_INFO_TYPE( structName )\
    struct __DbgInfo##structName

#define __DBG_INFO( structAddr )\
    ( structAddr->__dbgInfoData )

__DBG_INFO_TYPE( DzThread ){
    int     lastErr;
};

#endif


#endif // __DzStructsDebug_h__
