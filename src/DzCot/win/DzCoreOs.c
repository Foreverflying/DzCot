/**
 *  @file       DzCoreOs.c
 *  @brief      for windows
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#include "../DzInc.h"
#include "../DzCore.h"

void __cdecl SysThreadMain(void* context)
{
    DzSysParam* param = (DzSysParam*)context;
    param->threadEntry((intptr_t)param);
}

DzCot* InitCot(DzHost* host, DzCot* dzCot, int sType)
{
    int size;

    size = host->cotStackSize[sType];
    if (size < DZ_MIN_PAGE_STACK_SIZE) {
        dzCot->stackLimit = (char*)AllocChunk(host, size);
        if (!dzCot->stackLimit) {
            return NULL;
        }
        dzCot->stack = dzCot->stackLimit + size;
    } else {
        dzCot->stack = AllocStack(host, size);
        if (!dzCot->stack) {
            return NULL;
        }
        dzCot->stackLimit = CommitStack(dzCot->stack, PAGE_SIZE * 3);
        if (!dzCot->stackLimit)
        {
            return NULL;
        }
    }
    dzCot->sType = sType;
    InitCotStack(host, dzCot);
    return dzCot;
}

#ifdef GENERATE_MINIDUMP_FOR_UNHANDLED_EXP

#include <tchar.h>
#include <DbgHelp.h>
#pragma comment(lib, "DbgHelp.lib")

int MiniDumpExpFilter(LPEXCEPTION_POINTERS exception)
{
    int size;
    HANDLE dumpFile;
    TCHAR fileName[MAX_PATH];
    MINIDUMP_EXCEPTION_INFORMATION dumpExpInfo;

    dumpExpInfo.ThreadId = GetCurrentThreadId();
    dumpExpInfo.ExceptionPointers = exception;
    dumpExpInfo.ClientPointers = TRUE;
    size = GetModuleFileName(NULL, fileName, MAX_PATH);
    _stprintf(fileName + size, _T("%s"), _T(".dmp"));
    dumpFile = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    MiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        dumpFile,
        MiniDumpWithFullMemory,
        &dumpExpInfo,
        NULL,
        NULL
        );
    CloseHandle(dumpFile);
    return EXCEPTION_EXECUTE_HANDLER;
}

#else

int MiniDumpExpFilter(LPEXCEPTION_POINTERS exception)
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
    DzEntry volatile*   entryPtr,
    intptr_t volatile*  contextPtr
    )
{
    __Dbg(MarkCurrStackForCheck)(host);
    __try{
        while (1) {
            // call the entry
            (*entryPtr)(*contextPtr);

            // free the cot
            host->cotCount--;
            FreeDzCot(host, host->currCot);

            // then schedule another cot
            Schedule(host);
        }
    }__except(MiniDumpExpFilter(GetExceptionInformation())) {
        exit(0);
    }
}

static inline
void GetWinSockFunc(SOCKET tmpSock, GUID* guid, void* funcAddr)
{
    DWORD bytes;
    WSAIoctl(
        tmpSock,
        SIO_GET_EXTENSION_FUNCTION_POINTER,
        guid,
        (DWORD)sizeof(GUID),
        funcAddr,
        (DWORD)sizeof(void*),
        &bytes,
        NULL,
        NULL
        );
}

BOOL SockStartup(DzHost* host)
{
    BOOL ret;
    WSADATA data;
    WORD wVer;
    SOCKET tmpSock;
    GUID guidAcceptEx = WSAID_ACCEPTEX;
    GUID guidConnectEx = WSAID_CONNECTEX;
    GUID guidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;

    wVer = MAKEWORD(2, 2);
    ret = WSAStartup(wVer, &data) == 0;
    if (ret) {
        tmpSock = socket(AF_INET, SOCK_STREAM, 0);
        GetWinSockFunc(tmpSock, &guidAcceptEx, &host->readonly._AcceptEx);
        GetWinSockFunc(tmpSock, &guidConnectEx, &host->readonly._ConnectEx);
        GetWinSockFunc(tmpSock, &guidGetAcceptExSockAddrs, &host->readonly._GetAcceptExSockAddrs);
        closesocket(tmpSock);
    }
    return ret;
}

BOOL SockCleanup()
{
    return WSACleanup() == 0;
}

BOOL InitOsStruct(DzHost* host)
{
    DzHost* firstHost = host->hostId == 0 ? NULL : host->mgr->hostArr[0];

    if (!firstHost) {
        if (!SockStartup(host)) {
            return FALSE;
        }
    } else {
        host->readonly._AcceptEx = firstHost->readonly._AcceptEx;
        host->readonly._ConnectEx = firstHost->readonly._ConnectEx;
        host->readonly._GetAcceptExSockAddrs = firstHost->readonly._GetAcceptExSockAddrs;
    }
    host->os.iocp = CreateIoCompletionPort(
        INVALID_HANDLE_VALUE,
        NULL,
        (ULONG_PTR)NULL,
        1
        );
    host->os.originExceptPtr = GetExceptPtr();
    host->os.overlappedEntryList = (OVERLAPPED_ENTRY*)
        AllocChunk(host, sizeof(OVERLAPPED_ENTRY) * OVERLAPPED_ENTRY_LIST_SIZE);
    return host->os.iocp != NULL;
}

void CleanOsStruct(DzHost* host)
{
    CloseHandle(host->os.iocp);
    if (host->hostId == 0) {
        SockCleanup();
    }
}
