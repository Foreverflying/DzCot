/********************************************************************
    created:    2010/02/11 22:12
    file:       DzCoreWin.c
    author:     Foreverflying
    purpose:    
*********************************************************************/

#include "../DzIncOs.h"
#include "../DzCoreOs.h"
#include "../DzCore.h"

void InitAsynIo( DzAsynIo* asynIo )
{

}

void InitDzThread( DzThread* dzThread, int sSize )
{
    dzThread->stack = NULL;
    dzThread->stackLimit = NULL;
    dzThread->stackSize = sSize;
    dzThread->finishEvent = NULL;
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
void __stdcall DzcotRoutine( DzRoutine entry, void* context )
{
    DzHost* host = GetHost();
    __try{
        while(1){
            //call the entry
            ( *(DzRoutine volatile *)(&entry) )( *(void* volatile *)(&context) );

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

/*
void StackGrow()
{
    void** tib;
    char* stackLimit;

#if defined( _X86_ )
    tib = (void**)__readfsdword( 24 );
#elif defined( _M_AMD64 )
    tib = (void**)( __readgsqword( 0x30 ) + 48 );
#endif
    stackLimit = (char*)*(tib+2) - PAGE_SIZE;
    VirtualAlloc(
        stackLimit - PAGE_SIZE,
        PAGE_SIZE,
        MEM_COMMIT,
        PAGE_READWRITE | PAGE_GUARD
        );
    *(tib+2) = stackLimit;
}

int GuardMiniDumpExpFilter( LPEXCEPTION_POINTERS exception )
{
    if( exception->ExceptionRecord->ExceptionCode == STATUS_GUARD_PAGE_VIOLATION ){
        StackGrow();
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

// DzcotRoutine:
// the real entry the co thread starts, it call the user entry
// after that, the thread is finished, so put itself to the thread pool
// schedule next thread
void __stdcall DzcotRoutine( DzRoutine entry, void* context )
{
    DzHost* host = GetHost();
    __try{
        __try{
            while(1){
                //call the entry
                ( *(DzRoutine volatile *)(&entry) )( *(void* volatile *)(&context) );

                //free the thread
                host->threadCount--;
                FreeDzThread( host, host->currThread );

                //then schedule another thread
                Schedule( host );
            }
        }__except( GuardMiniDumpExpFilter( GetExceptionInformation() ) ){
        }
    }__except( MiniDumpExpFilter( GetExceptionInformation() ) ){
        exit( 0 );
    }
}
*/