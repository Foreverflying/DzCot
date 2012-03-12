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

#define __DBG_DATA_DEFINE( structName )
#define __DBG_INIT_INFO( structName, host, obj )

#else

#define __DBG_DATA_DEFINE( structName )\
    struct __DbgInfo##structName   __dbgData;

#define __DBG_INIT_INFO( structName, host, obj )\
    __DbgInitInfo##structName( host, obj, &(obj)->__dbgData )

#define __DBG_DATA_TYPE( structName )\
    struct __DbgInfo##structName

#define __DBG_DATA_TYPE_INIT( structName )\
    inline void __DbgInitInfo##structName(\
        const DzHost* host,\
        const structName* obj,\
        struct __DbgInfo##structName* data\
        )

#define __DBG_DATA( structAddr )\
    ( structAddr->__dbgData )


__DBG_DATA_TYPE( DzCot ){
    union{
        struct{
            int         lastErr;
        };
        DzCacheChunk    _padding;
    };
};

__DBG_DATA_TYPE_INIT( DzCot )
{
    data->lastErr = 0;
}

#endif


#endif // __DzStructsDebug_h__
