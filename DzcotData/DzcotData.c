// DzcotData.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "DzcotData.h"

DWORD tlsIndex = TLS_OUT_OF_INDEXES;
LONG tlsLock = 0;
LONG sockInitCount = 0;
LONG sockInitLock = 0;
BOOL isSocketStarted = FALSE;
LPFN_ACCEPTEX _AcceptEx = NULL;
LPFN_CONNECTEX _ConnectEx = NULL;
LPFN_GETACCEPTEXSOCKADDRS _GetAcceptExSockAddrs = NULL;
