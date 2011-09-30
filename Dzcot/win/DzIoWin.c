/********************************************************************
    created:    2010/02/11 22:12
    file:       DzIoWin.c
    author:     Foreverflying
    purpose:    
*********************************************************************/

#include "../DzIncOs.h"
#include "../DzIoOs.h"
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

BOOL SockStartup()
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
        tmpSock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
        GetWinSockFunc( tmpSock, &guidAcceptEx, &_AcceptEx );
        GetWinSockFunc( tmpSock, &guidConnectEx, &_ConnectEx );
        GetWinSockFunc( tmpSock, &guidGetAcceptExSockAddrs, &_GetAcceptExSockAddrs );
        closesocket( tmpSock );
    }
    return ret;
}

BOOL SockCleanup()
{
    BOOL ret;

    ret = WSACleanup() == 0;
    _AcceptEx = NULL;
    _ConnectEx = NULL;
    _GetAcceptExSockAddrs = NULL;
    return ret;
}
