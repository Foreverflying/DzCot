/********************************************************************
    created:    2010/02/11 22:12
    file:       DzCoreWin.c
    author:     Foreverflying
    purpose:    
********************************************************************/

#include "../DzInc.h"
#include "../DzCore.h"

void __cdecl SysThreadMain( void* context )
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

// DzCotEntry:
// the real function entry the cot starts, it call the user entry
// after that, when the cot is finished, put it into the cot pool
// schedule next cot
void __stdcall DzCotEntry(
    DzHost*             host,
    DzRoutine volatile* entryPtr,
    intptr_t volatile*  contextPtr
    )
{
    __try{
        while(1){
            //call the entry
            ( *entryPtr )( *contextPtr );

            //free the cot
            host->cotCount--;
            FreeDzCot( host, host->currCot );

            //then schedule another cot
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

BOOL InitOsStruct( DzHost* host )
{
    DzHost* firstHost = host->hostId == 0 ? NULL : host->mgr->hostArr[0];

    if( !firstHost ){
        if( !SockStartup( &host->os ) ){
            return FALSE;
        }
    }else{
        host->os._AcceptEx = firstHost->os._AcceptEx;
        host->os._ConnectEx = firstHost->os._ConnectEx;
        host->os._GetAcceptExSockAddrs = firstHost->os._GetAcceptExSockAddrs;
    }
    host->os.iocp = CreateIoCompletionPort(
        INVALID_HANDLE_VALUE,
        NULL,
        (ULONG_PTR)NULL,
        1
        );
    host->os.originExceptPtr = GetExceptPtr();

    return host->os.iocp != NULL;
}

void DeleteOsStruct( DzHost* host )
{
    CloseHandle( host->os.iocp );
    if( host->hostId == 0 ){
        SockCleanup();
    }
}
