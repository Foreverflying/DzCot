#include "DzIncOs.h"
//#include "DzcotExports.h"

#define BOOST_PYTHON_STATIC_LIB
#include <boost/python.hpp>
using namespace boost::python;

#include "DzType.h"
#include "DzSynObj.h"
#include "DzCore.h"
#include <string>
using namespace std;

namespace DzcotPython{

    PyThreadState *tsState = NULL;

    int __stdcall PythonFunc( void *context )
    {
        PyEval_RestoreThread( tsState );

        DzQNode *node = (DzQNode*)context;
        PyObject *func = (PyObject*)node->content;
        PyObject *arg = (PyObject*)node->context1;
        DzHost *host = (DzHost*)node->context3;
        FreeQNode( host, node );
        object entry( (detail::new_non_null_reference)func );
        object argObj( (detail::new_reference)arg );
        object ret = entry( argObj );
        int intRet = extract<int>(ret);

        tsState = PyEval_SaveThread();
        return intRet;
    }

    int DzPrint( object str )
    {
        string a = extract<string>(str);
        printf( a.c_str() );
        return 0;
    }

    // InitHost:
    // create structs needed
    int DzInitHost(
        int lowestPriority,
        int defaultPri,
        int defaultSSize
        )
    {
        assert(
            lowestPriority >= CP_HIGH &&
            lowestPriority < COT_PRIORITY_COUNT
            );
        assert(
            defaultPri >= CP_HIGH &&
            defaultPri < COT_PRIORITY_COUNT
            );
        assert(
            defaultSSize >= SS_64K &&
            defaultSSize < STACK_SIZE_COUNT
            );
        assert( !GetHost() );

        return InitHost( lowestPriority, defaultPri, defaultSSize );
    }

    // StartHost:
    // create the Io mgr co thread, so the host can serve requests
    int DzStartHost(
        object      firstEntry,
        object      context,
        int         priority,
        int         sSize
        )
    {
        DzHost *host = GetHost();
        assert( host );
        assert( host->threadCount == 0 );

        tsState = PyEval_SaveThread();

        DzQNode *node = NULL;
        PyObject *func = firstEntry.ptr();
        if( func ){
            Py_INCREF( func );
            PyObject *arg = context.ptr();
            Py_XINCREF( arg );

            node = AllocQNode( host );
            node->content = func;
            node->context1 = arg;
            node->context3 = host;
        }

        int ret = StartHost(
            host,
            func ? PythonFunc : NULL,
            node,
            priority,
            sSize
            );

        PyEval_RestoreThread( tsState );
        return ret;
    }

    // DzStartCot:
    // create a new co thread
    int DzStartCot(
        object      entry,
        object      context,
        int         priority,
        int         sSize
        )
    {
        DzHost *host = GetHost();
        assert( host );
        assert( entry );
        assert(
            priority >= CP_INSTANT &&
            priority <= CP_DEFAULT &&
            priority != COT_PRIORITY_COUNT
            );
        assert(
            sSize >= SS_64K &&
            sSize <= SS_DEFAULT &&
            sSize != STACK_SIZE_COUNT
            );
        assert( host->threadCount > 0 );

        tsState = PyEval_SaveThread();

        DzQNode *node = NULL;
        PyObject *func = entry.ptr();
        assert( func );

        Py_INCREF( func );
        PyObject *arg = context.ptr();
        Py_XINCREF( arg );

        node = AllocQNode( host );
        node->content = func;
        node->context1 = arg;
        node->context3 = host;

        int ret = StartCot(
            host,
            PythonFunc,
            node,
            priority,
            sSize
            );

        PyEval_RestoreThread( tsState );
        return ret;
    }

    void DzInitCotPool( u_int count, u_int depth, int sSize )
    {
        DzHost *host = GetHost();
        assert( host );
        assert(
            sSize >= SS_64K &&
            sSize <= SS_DEFAULT &&
            sSize != STACK_SIZE_COUNT
            );

        return InitCotPool( host, count, depth, sSize );
    }

    int DzSleep( int milSec )
    {
        DzHost *host = GetHost();
        assert( host );

        tsState = PyEval_SaveThread();

        if( !milSec ){
            DispatchCurrThread( host );
        }else{
            //BlockCurrThread( host );    //TODO: need a timer to block on
        }

        PyEval_RestoreThread( tsState );
        return DS_OK;
    }

    int DzSockStartup()
    {
        return (int)SockStartup();
    }

    int DzSockCleanup()
    {
        return (int)SockCleanup();
    }

    int DoNothing(
        object      entry,
        object      context,
        int         priority,
        int         sSize
        )
    {
        //int ret = StartCot( entry, context, priority, sSize );
        return 1;
    }

    BOOST_PYTHON_MODULE( dzcot )
    {
        def( "DzPrint", DzPrint );
        def( "DzInitHost", DzInitHost );
        def( "DzStartHost", DzStartHost );
        def( "DzStartCot", DzStartCot );
        def( "DzInitCotPool", DzInitCotPool );
        def( "DzSleep", DzSleep );
        def( "DzSockStartup", DzSockStartup );
        def( "DzSockCleanup", DzSockCleanup );
        def( "DoNothing", DoNothing );
        //         def( "CreateDzEvt", CreateDzEvt );
        //         def( "SetDzEvt", SetDzEvt );
        //         def( "ResetDzEvt", ResetDzEvt );
        //         def( "CreateDzSem", CreateDzSem );
        //         def( "ReleaseDzSem", ReleaseDzSem );
        //         def( "CloneSynObj", CloneSynObj );
        //         def( "CloseSynObj", CloseSynObj );
        //         def( "WaitSynObj", WaitSynObj );
        //         def( "WaitMultiSynObj", WaitMultiSynObj );
        //         def( "DzOpenFileA", DzOpenFileA );
        //         def( "DzOpenFileW", DzOpenFileW );
        //         def( "DzReadFile", DzReadFile );
        //         def( "DzWriteFile", DzWriteFile );
        //         def( "DzSeekFile", DzSeekFile );
        //         def( "DzGetFileSize", DzGetFileSize );
        //         def( "DzSockStartup", DzSockStartup );
        //         def( "DzSockCleanup", DzSockCleanup );
        //         def( "DzSocket", DzSocket );
        //         def( "DzShutdown", DzShutdown );
        //         def( "DzCloseSocket", DzCloseSocket );
        //         def( "DzBind", DzBind );
        //         def( "DzListen", DzListen );
        //         def( "DzConnect", DzConnect );
        //         def( "DzAccept", DzAccept );
        //         def( "DzSend", DzSend );
        //         def( "DzRecv", DzRecv );
    }

}
