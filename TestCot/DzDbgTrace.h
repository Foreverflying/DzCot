/********************************************************************
    created:    2011/09/26 3:29
    file:       DzDbgTrace.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzDbgTrace_h__
#define __DzDbgTrace_h__

#define __DBG_TRACE_ALL_FILE
#define __DBG_TRACE_LEVEL       4

#if defined( __DBG_TRACE_ALL_FILE ) || defined( __DBG_TRACE_THIS_FILE )

#if __DBG_TRACE_LEVEL >= 1
#define __DbgTce1    printf
#else
#define __DbgTce1( ... )
#endif

#if __DBG_TRACE_LEVEL >= 1
#define __DbgTce1    printf
#else
#define __DbgTce1( ... )
#endif

#if __DBG_TRACE_LEVEL >= 2
#define __DbgTce2    printf
#else
#define __DbgTce2( ... )
#endif

#if __DBG_TRACE_LEVEL >= 3
#define __DbgTce3    printf
#else
#define __DbgTce3( ... )
#endif

#if __DBG_TRACE_LEVEL >= 4
#define __DbgTce4    printf
#else
#define __DbgTce4( ... )
#endif

#if __DBG_TRACE_LEVEL >= 5
#define __DbgTce5    printf
#else
#define __DbgTce5( ... )
#endif

#else

#define __DbgTce1( ... )
#define __DbgTce2( ... )
#define __DbgTce3( ... )
#define __DbgTce4( ... )
#define __DbgTce5( ... )

#endif

#endif // __DzDbgTrace_h__
