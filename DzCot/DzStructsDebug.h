/**
 *  @file       DzStructsDebug.h
 *  @brief      
 *  @author	    Foreverflying <foreverflying@live.cn>
 *  @date       2011/10/07
 *
 */

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
    int         maxStkUse[ STACK_TYPE_COUNT ];
    int         synObjLeak;
    int         fdLeak;
    int         heapLeak;
    int         paramNodeLeak;
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
