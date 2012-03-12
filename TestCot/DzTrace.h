/********************************************************************
    created:    2011/10/31 23:26
    file:       DzTrace.h
    author:     Foreverflying
    purpose:    
********************************************************************/

#ifndef __DzTrace_h__
#define __DzTrace_h__

#include <stdio.h>
#include <stdarg.h>

#define TCE_EOL     "\r\n"

namespace __DzTrace
{

namespace __Inner
{

class DbgNopeTracer
{
public:
    static void Tce( const char* fmt, ... )
    {
    }
};

template< class TracerCls, int level, int n >
class DbgTceTml
{
public:
    enum{ BASE = level };
    typedef TracerCls Tracer;
};

template< int level, int n >
class DbgTceTml< DbgNopeTracer, level, n >
{
public:
    enum{ BASE = DbgTceTml< DbgNopeTracer, level, n - 1 >::BASE };
};

template< int level >
class DbgTceTml< DbgNopeTracer, level, 0 >
{
public:
    enum{ BASE = 0 };
    typedef DbgNopeTracer Tracer;
};

//print trace
class DbgPrintTracer
{
public:
    static void Tce( const char* fmt, ... )
    {
        va_list ap;
        va_start( ap, fmt );
        vprintf( fmt, ap );
        va_end( ap );
    }
};

}   //namespace __DzTraceInner

using namespace __Inner;

typedef DbgNopeTracer __DzDbgTce1;
typedef DbgNopeTracer __DzDbgTce2;
typedef DbgNopeTracer __DzDbgTce3;
typedef DbgNopeTracer __DzDbgTce4;
typedef DbgNopeTracer __DzDbgTce5;

}   //namespace __DzTrace

using namespace __DzTrace;

#define __z_DbgRptDef_1( m )  m( 1 )
#define __z_DbgRptDef_2( m )  m( 1 ); m( 2 )
#define __z_DbgRptDef_3( m )  m( 1 ); m( 2 ); m( 3 )
#define __z_DbgRptDef_4( m )  m( 1 ); m( 2 ); m( 3 ); m( 4 )
#define __z_DbgRptDef_5( m )  m( 1 ); m( 2 ); m( 3 ); m( 4 ); m( 5 )

#define __z_DbgTceFunc( lev )\
    __Inner::DbgTceTml<\
        __DzDbgTce##lev,\
        lev,\
        __Inner::DbgTceTml< __DzDbgTce##lev, lev, 2 >::BASE\
    >::Tracer::Tce

#define __DzTce1( fmt, ... )    __z_DbgTceFunc( 1 )( fmt TCE_EOL, ##__VA_ARGS__ )
#define __DzTce2( fmt, ... )    __z_DbgTceFunc( 2 )( fmt TCE_EOL, ##__VA_ARGS__ )
#define __DzTce3( fmt, ... )    __z_DbgTceFunc( 3 )( fmt TCE_EOL, ##__VA_ARGS__ )
#define __DzTce4( fmt, ... )    __z_DbgTceFunc( 4 )( fmt TCE_EOL, ##__VA_ARGS__ )
#define __DzTce5( fmt, ... )    __z_DbgTceFunc( 5 )( fmt TCE_EOL, ##__VA_ARGS__ )

#define __DzTceEnableGloblePrint( level )\
    namespace __DzTrace{ namespace __Inner{\
        template<> class DbgTceTml< DbgNopeTracer, level, 1 >;\
    } }\
    template<>\
    class __Inner::DbgTceTml< __Inner::DbgNopeTracer, level, 1 >\
    {\
    public:\
        enum{ BASE = 1 };\
        typedef DbgPrintTracer Tracer;\
    }

#define __DzTceEnableGloblePrintLe( level )\
    __z_DbgRptDef_##level( __DzTceEnableGloblePrint )

#define __DzTceEnableScopePrint( level )\
    typedef __Inner::DbgPrintTracer __DzDbgTce##level

#define __DzTceEnableScopePrintLe( level )\
    __z_DbgRptDef_##level( __DzTceEnableScopePrint )

#endif // __DzTrace_h__
