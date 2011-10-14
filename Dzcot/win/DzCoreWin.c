/********************************************************************
    created:    2010/02/11 22:12
    file:       DzCoreWin.c
    author:     Foreverflying
    purpose:    
*********************************************************************/

#include "../DzIncOs.h"
#include "../DzCoreOs.h"
#include "../DzCore.h"

void __cdecl SysThreadEntry( void* context )
{
    DzSysParam* param = (DzSysParam*)context;
    param->threadEntry( (intptr_t)param );
}

#ifdef GENERATE_MINIDUMP_FOR_UNHANDLED_EXP

#include <tchar.h>
#include <DbgHelp.h>
#pragma comment( lib, "DbgHelp.lib" )

int MiniDumpExpFilter( LPEXCEPTION_POINTERS exception )
{
    int size;
    HANDLE dumpFile;
    TCHAR fileName[MAX_PATH];
    MINIDUMP_EXCEPTION_INFORMATION dumpExpInfo;

    dumpExpInfo.ThreadId = GetCurrentThreadId();
    dumpExpInfo.ExceptionPointers = exception;
    dumpExpInfo.ClientPointers = TRUE;
    size = GetModuleFileName( NULL, fileName, MAX_PATH );
    _stprintf( fileName + size, _T("%s"), _T(".dmp") );
    dumpFile = CreateFile( fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    MiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        dumpFile,
        MiniDumpWithFullMemory,
        &dumpExpInfo,
        NULL,
        NULL
        );
    CloseHandle( dumpFile );
    return EXCEPTION_EXECUTE_HANDLER;
}

#else

int MiniDumpExpFilter( LPEXCEPTION_POINTERS exception )
{
    return EXCEPTION_CONTINUE_SEARCH;
}

#endif // GENERATE_MINIDUMP_FOR_UNHANDLED_EXP

// DzcotEntry:
// the real function entry the cot starts, it call the user entry
// after that, when the cot is finished, put it into the thread pool
// schedule next cot
void __stdcall DzcotEntry(
    volatile DzRoutine* entryPtr,
    volatile intptr_t*  contextPtr
    )
{
    DzHost* host = GetHost();
    __try{
        while(1){
            //call the entry
            ( *entryPtr )( *contextPtr );

            //free the thread
            host->threadCount--;
            FreeDzThread( host, host->currThread );

            //then schedule another thread
            Schedule( host );
        }
    }__except( MiniDumpExpFilter( GetExceptionInformation() ) ){
        exit( 0 );
    }
}

inline void GetWinSockFunc( SOCKET tmpSock, GUID* guid, void* funcAddr )
{
    DWORD bytes;
    WSAIoctl(
        tmpSock,
        SIO_GET_EXTENSION_FUNCTION_POINTER,
        guid,
        (DWORD)sizeof( GUID ),
        funcAddr,
        (DWORD)sizeof( void* ),
        &bytes,
        NULL,
        NULL
        );
}

BOOL SockStartup( DzOsStruct* osStruct )
{
    BOOL ret;
    WSADATA data;
    WORD wVer;
    SOCKET tmpSock;
    GUID guidAcceptEx = WSAID_ACCEPTEX;
    GUID guidConnectEx = WSAID_CONNECTEX;
    GUID guidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;

    wVer = MAKEWORD( 2, 2 );
    ret = WSAStartup( wVer, &data ) == 0;
    if( ret ){
        tmpSock = socket( AF_INET, SOCK_STREAM, 0 );
        GetWinSockFunc( tmpSock, &guidAcceptEx, &osStruct->_AcceptEx );
        GetWinSockFunc( tmpSock, &guidConnectEx, &osStruct->_ConnectEx );
        GetWinSockFunc( tmpSock, &guidGetAcceptExSockAddrs, &osStruct->_GetAcceptExSockAddrs );
        closesocket( tmpSock );
    }
    return ret;
}

BOOL SockCleanup()
{
    return WSACleanup() == 0;
}

BOOL InitOsStruct( DzHost* host, DzHost* parentHost )
{
    if( !parentHost ){
        if( !SockStartup( &host->osStruct ) ){
            return FALSE;
        }
    }else{
        host->osStruct._AcceptEx = parentHost->osStruct._AcceptEx;
        host->osStruct._ConnectEx = parentHost->osStruct._ConnectEx;
        host->osStruct._GetAcceptExSockAddrs = parentHost->osStruct._GetAcceptExSockAddrs;
    }
    host->osStruct.iocp = CreateIoCompletionPort(
        INVALID_HANDLE_VALUE,
        NULL,
        (ULONG_PTR)NULL,
        1
        );
    host->osStruct.originExceptPtr = GetExceptPtr();
    host->osStruct.originalStack = GetStackPtr();

    return host->osStruct.iocp != NULL;
}

void DeleteOsStruct( DzHost* host, DzHost* parentHost )
{
    CloseHandle( host->osStruct.iocp );
    if( !parentHost ){
        SockCleanup();
    }
}

// CotScheduleCenter:
// the Cot Schedule Center thread uses the host's origin thread's stack
// manager all kernel objects that may cause real block
void CotScheduleCenter( DzHost* host )
{
    ULONG_PTR key;
    DWORD bytes;
    OVERLAPPED* overlapped;
    DzAsyncIo* asyncIo;
    DWORD timeout;

    timeout = (DWORD)NotifyMinTimersAndRmtCalls( host );
    while( host->threadCount ){
        GetQueuedCompletionStatus( host->osStruct.iocp, &bytes, &key, &overlapped, timeout );
        if( overlapped != NULL ){
            do{
                asyncIo = MEMBER_BASE( overlapped, DzAsyncIo, overlapped );
                NotifyEasyEvt( host, &asyncIo->easyEvt );
                GetQueuedCompletionStatus( host->osStruct.iocp, &bytes, &key, &overlapped, 0 );
            }while( overlapped != NULL );
            host->currPriority = CP_FIRST;
            Schedule( host );
        }
        timeout = (DWORD)NotifyMinTimersAndRmtCalls( host );
    }
}

