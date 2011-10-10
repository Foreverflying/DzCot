/********************************************************************
    created:    2010/02/11 22:12
    file:       DzCoreWin.c
    author:     Foreverflying
    purpose:    
*********************************************************************/

#include "../DzIncOs.h"
#include "../DzCoreOs.h"
#include "../DzCore.h"

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

#if defined( _X86_ )
    host->osStruct.originExceptPtr = (void*)__readfsdword( 0 );
    host->osStruct.originalStack = (char*)__readfsdword( 4 );
#elif defined( _M_AMD64 )
    host->osStruct.originExceptPtr = NULL;
    host->osStruct.originalStack = (char*)( __readgsqword( 0x30 ) + 8 );
#endif

    return host->osStruct.iocp != NULL;
}

void DeleteOsStruct( DzHost* host, DzHost* parentHost )
{
    CloseHandle( host->osStruct.iocp );
    if( !parentHost ){
        SockCleanup();
    }
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

// DzcotRoutine:
// the real entry the co thread starts, it call the user entry
// after that, the thread is finished, so put itself to the thread pool
// schedule next thread
void __stdcall DzcotRoutine( DzRoutine entry, intptr_t context )
{
    DzHost* host = GetHost();
    __try{
        while(1){
            //call the entry
            ( *(DzRoutine volatile *)(&entry) )( *(intptr_t volatile *)(&context) );

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
