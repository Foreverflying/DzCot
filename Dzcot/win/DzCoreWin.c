#include "DzIncWin.h"
#include "DzCoreWin.h"
#include "../DzCore.h"

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
    DzHost *host = GetHost();
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

char* AllocStack( int sSize )
{
    size_t size;
    char *base;

    size = DZ_STACK_UNIT_SIZE << sSize;
    base = (char*)VirtualAlloc(
        NULL,
        size,
        MEM_RESERVE,
        PAGE_READWRITE
        );

    if( !base ){
        return NULL;
    }
    return base + size;
}

void FreeStack( char *stack, int sSize )
{
    size_t size;
    char *base;

    size = DZ_STACK_UNIT_SIZE << sSize;
    base = stack - size;
    VirtualFree( base, 0, MEM_RELEASE );
}

char* CommitStack( char *stack, size_t size )
{
    void *tmp;
    BOOL ret;

    tmp = VirtualAlloc(
        stack - size,
        size,
        MEM_COMMIT,
        PAGE_READWRITE
        );
    if( !tmp ){
        return NULL;
    }
    ret = VirtualProtect(
        stack - size,
        PAGE_SIZE,
        PAGE_READWRITE | PAGE_GUARD,
        (LPDWORD)&tmp
        );
    return ret ? stack - size + PAGE_SIZE : NULL;
}

void DeCommitStack( char *stack, char *stackLimit )
{
    size_t size;

    size = stack - stackLimit;
    VirtualFree( stackLimit, size, MEM_DECOMMIT );
}
