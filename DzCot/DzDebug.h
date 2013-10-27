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
#define __Dbg( func )       __Dbg##func
#else
#define __Dbg( func )       __EmptyCall
#endif

inline int __EmptyCall( DzHost* host, ... )
{
    return 0;
}

int __DbgGetLastErr( DzHost* host );
int __DbgGetMaxStackUse( DzHost* host, int sSize );
void __DbgSetLastErr( DzHost* host, int err );
void __DbgMarkCurrStackForCheck( DzHost* host );
void __DbgCheckCotStackOverflow( DzHost* host, DzCot* dzCot );
void __DbgInitDzHost( DzHost* host );
void __DbgInitDzCot( DzHost* host, DzCot* dzCot );

#ifdef __cplusplus
};
#endif


#endif // __DzDebug_h__
