/********************************************************************
    created:    2011/10/07 18:25
    file:       DzStructsDebug.h
    author:     Foreverflying
    purpose:    
********************************************************************/

#ifndef __DzStructsDebug_h__
#define __DzStructsDebug_h__

#include "DzInc.h"

#ifndef __DBG_DEBUG_CHECK_MODE

#define __DBG_STRUCT( structName )
#define __DBG_INIT(  host, structName,obj )

#else

#define __DBG_STRUCT( structName )\
    struct __DbgData##structName   __dbgData;

#define __DBG_DEF_STRUCT( structName )\
    struct __DbgData##structName

#define __DbgPtr( structAddr )\
    ( &structAddr->__dbgData )

#define __DbgPtrType( structName )\
    struct __DbgData##structName*

__DBG_DEF_STRUCT( DzHost )
{
    int         maxStkUse[ STACK_SIZE_COUNT ];
};

__DBG_DEF_STRUCT( DzCot ){
    union{
        struct{
            int         lastErr;
            int         maxStkUse;
        };
        DzCacheChunk    _padding;
    };
};

#endif


#endif // __DzStructsDebug_h__
