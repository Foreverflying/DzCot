/********************************************************************
    created:    2011/10/06 18:51
    file:       DzDebug.h
    author:     Foreverflying
    purpose:    
********************************************************************/

#ifndef __DzDebug_h__
#define __DzDebug_h__

#include "DzInc.h"
#include "DzDeclareStructs.h"

#ifdef __cplusplus
extern "C"{
#endif

#ifdef __DBG_DEBUG_CHECK_MODE

int __DbgGetLastErr( DzHost* host );
void __DbgSetLastErr( DzHost* host, int err );
void __DbgCheckCotStackOverflow( DzCot* dzCot );

#else

#define __DbgGetLastErr( host ) (0)
#define __DbgSetLastErr( host, err )
#define __DbgCheckCotStackOverflow( dzCot )

#endif

#ifdef __cplusplus
};
#endif


#endif // __DzDebug_h__
