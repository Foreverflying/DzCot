
#include "stdafx.h"
#include <MSWSock.h>
#include <commctrl.h>

static int globalCount = 0;
static int connErrCount = 0;
static int acptErrCount = 0;
static int sendErrCount = 0;
static int recvErrCount = 0;
static int lisErrCount = 0;

BOOL isSocketStarted = FALSE;
LPFN_CONNECTEX ConnectEx = NULL;
//LPFN_ACCEPTEX AcceptEx = NULL;
LPFN_GETACCEPTEXSOCKADDRS GetAcceptExSockAddrs = NULL;

inline void GetWinSockFunc( SOCKET tmpSock, GUID *guid, void *funcAddr )
{
    DWORD bytes;
    int a = WSAIoctl(
        tmpSock,
        SIO_GET_EXTENSION_FUNCTION_POINTER,
        guid,
        sizeof( GUID ),
        funcAddr,
        sizeof( void* ),
        &bytes,
        NULL,
        NULL
        );
}

BOOL SockStartup()
{
    if( isSocketStarted ){
        return TRUE;
    }
    WSADATA data;
    WORD wVer = MAKEWORD( 2, 2 );
    int ret = WSAStartup( wVer, &data );
    if( ret != 0 ){
        return FALSE;
    }
    SOCKET tmpSock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    //GUID guidAcceptEx = WSAID_ACCEPTEX;
    //GetWinSockFunc( tmpSock, &guidAcceptEx, &AcceptEx );
    GUID guidConnectEx = WSAID_CONNECTEX;
    GetWinSockFunc( tmpSock, &guidConnectEx, &ConnectEx );
    GUID guidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
    GetWinSockFunc( tmpSock, &guidGetAcceptExSockAddrs, &GetAcceptExSockAddrs );
    closesocket( tmpSock );
    isSocketStarted = TRUE;
    return TRUE;
}

BOOL SockCleanup()
{
    if( isSocketStarted ){
        WSACleanup();
        //AcceptEx = NULL;
        ConnectEx = NULL;
        GetAcceptExSockAddrs = NULL;
        isSocketStarted = FALSE;
        return TRUE;
    }
    return FALSE;
}

int __stdcall TestFastConnect( void *context )
{
    struct FastAsynConn 
    {
        OVERLAPPED      overlapped;
        char            buf[16];
        int             op;
        SOCKET          s;
        WSABUF          wsaBuf;
    };

    int count = (int)context;
    SockStartup();

    HANDLE iocp = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, NULL, 1 );

    printf( "start here:\t %d\r\n", GetTickCount() );

    for( int i=0; i<count; i++ ){
        SOCKET s = socket( AF_INET, SOCK_STREAM, 0 );
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl( MAKEIPADDRESS( 192, 168, 0, 1 ) );
        addr.sin_port = htons( 59999 );
        sockaddr_in temp;
        temp.sin_family = AF_INET;
        temp.sin_port = htons( (u_short)i + 5000 );
        //temp.sin_port = htons( 0 );
        temp.sin_addr.s_addr = htonl( MAKEIPADDRESS( 0, 0, 0, 0 ) );;
        bind( s, (sockaddr*)&temp, sizeof(sockaddr_in) );
        CreateIoCompletionPort( (HANDLE)s, iocp, NULL, 0 );
        DWORD bytes;
        FastAsynConn *fac = new FastAsynConn;
        ZeroMemory( &fac->overlapped, sizeof( OVERLAPPED ) );
        fac->s = s;
        fac->op = 0;
        BOOL ret = ConnectEx(
            s,
            (sockaddr*)&addr,
            sizeof(sockaddr_in),
            NULL,
            0,
            &bytes,
            &fac->overlapped
            );
        if( !ret ){
            DWORD err = WSAGetLastError();
            if( err != ERROR_IO_PENDING ){
                connErrCount++;
            }
        }
    }

    count -= connErrCount;
    if( count ){
        DWORD bytes;
        ULONG_PTR key;
        int nResult = 0;
        int sendCount = 0;
        int recvCount = 0;
        DWORD flag = 0;
        FastAsynConn *pFac;
        BOOL result = GetQueuedCompletionStatus( iocp, &bytes, &key, (LPOVERLAPPED*)&pFac, INFINITE );
        while( pFac ){
            switch( pFac->op ){
            case 0:
                result = WSAGetOverlappedResult( pFac->s, &pFac->overlapped, &bytes, false, &flag );
                if( !result ){
                    connErrCount++;
                    closesocket( pFac->s );
                    delete pFac;
                    count--;
                }else{
                    pFac->op = 1;
                    pFac->wsaBuf.buf = "hello";
                    pFac->wsaBuf.len = 5;
                    ZeroMemory( &pFac->overlapped, sizeof(OVERLAPPED) );
                    nResult = WSASend( pFac->s, &pFac->wsaBuf, 1, &bytes, 0, &pFac->overlapped, NULL );
                    if( nResult != 0 && WSAGetLastError() != ERROR_IO_PENDING ){
                        sendErrCount++;
                        closesocket( pFac->s );
                        delete pFac;
                        count--;
                    }
                }
                break;
            case 1:
                result = WSAGetOverlappedResult( pFac->s, &pFac->overlapped, &bytes, false, &flag );
                if( !result ){
                    sendErrCount++;
                    closesocket( pFac->s );
                    delete pFac;
                    count--;
                }else{
                    pFac->op = 2;
                    pFac->wsaBuf.buf = pFac->buf;
                    pFac->wsaBuf.len = 16;
                    ZeroMemory( &pFac->overlapped, sizeof(OVERLAPPED) );
                    nResult = WSARecv( pFac->s, &pFac->wsaBuf, 1, &bytes, &flag, &pFac->overlapped, NULL );
                    if( nResult != 0 && WSAGetLastError() != ERROR_IO_PENDING ){
                        recvErrCount++;
                        closesocket( pFac->s );
                        delete pFac;
                        count--;
                    }
                }
                break;
            case 2:
                result = WSAGetOverlappedResult( pFac->s, &pFac->overlapped, &bytes, false, &flag );
                if( !result ){
                    recvErrCount++;
                }
                closesocket( pFac->s );
                delete pFac;
                count--;
                break;
            }
            if( count ){
                GetQueuedCompletionStatus( iocp, &bytes, &key, (LPOVERLAPPED*)&pFac, INFINITE );
            }else{
                break;
            }
        }
    }

    printf( "end here:\t %d\r\n", GetTickCount() );
    printf( "acptErrCount:\t %d\r\n", acptErrCount );
    printf( "connErrCount:\t %d\r\n", connErrCount );
    printf( "sendErrCount:\t %d\r\n", sendErrCount );
    printf( "recvErrCount:\t %d\r\n", recvErrCount );
    acptErrCount = 0;
    connErrCount = 0;
    sendErrCount = 0;
    recvErrCount = 0;

    CloseHandle( iocp );
    SockCleanup();
    return 1;
}