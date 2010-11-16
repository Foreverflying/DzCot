/********************************************************************
    created:    2010/02/11 22:14
    file:       DzcotData.c
    author:     Foreverflying
    purpose:    
*********************************************************************/

#include "DzcotData.h"

#ifdef _WIN32

DWORD tlsIndex = TLS_OUT_OF_INDEXES;
LONG tlsLock = 0;
LONG sockInitCount = 0;
LONG sockInitLock = 0;
BOOL isSocketStarted = FALSE;
LPFN_ACCEPTEX _AcceptEx = NULL;
LPFN_CONNECTEX _ConnectEx = NULL;
LPFN_GETACCEPTEXSOCKADDRS _GetAcceptExSockAddrs = NULL;

#elif defined LINUX

int tlsIndex = -1;
int tlsLock = 0;

#endif
