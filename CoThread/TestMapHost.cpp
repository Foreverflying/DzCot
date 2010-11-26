#include "stdafx.h"
#include "../Dzcot/Dzcot.h"

#include "../Dzcot/DzList.h"
#include "Global.h"

#include "DlMalloc.h"

#define PACKAGE_LEN     32
#define MAX_HOST_COUNT  4096

static int globalCount = 0;
static int connErrCount = 0;
static int acptErrCount = 0;
static int sendErrCount = 0;
static int recvErrCount = 0;
static int lisErrCount = 0;
static int serveCount = 0;

struct Conn
{
    int             fd;
    int             index;
    volatile int    recvCount;
    volatile int    sendTotal;
    volatile int    recvTotal;
};

static volatile BOOL working = TRUE;

int __stdcall SendRountine( void *context )
{
    Conn *conn = (Conn*)context;

    char buff[ PACKAGE_LEN ];
    while( working ){
        int len = sprintf( buff, "%04d\t move %06d", conn->index, conn->sendTotal );
        conn->sendTotal++;
        int bytes = DzSend( conn->fd, buff, len+1, 0 );
        if( bytes < 0 ){
            sendErrCount++;
            printf( "send error! count = %d\r\n", sendErrCount );
            DzShutdown( conn->fd, DZ_SD_RDWR );
            DzCloseSocket( conn->fd );
            break;
        }
        DzSleep( 300 );
        //printf( "TickRecv:\t%d,  TotalSend:\t%d,  TotalRecv:\t%d\r\n", conn->recvCount, conn->sendTotal, conn->recvTotal );
        conn->recvCount = 0;
    }
    return 0;
}

int __stdcall TestMapHost( void *context )
{
    int index = -1;
    byte buff[PACKAGE_LEN];
    Conn conn;
    conn.recvCount = 0;
    conn.recvTotal = 0;
    conn.sendTotal = 0;

    globalCount++;

    int fd = DzSocket( AF_INET, SOCK_STREAM, 0 );
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = gIp;
    addr.sin_port = gPort;
    //addr.sin_addr.s_addr = htonl( MAKEIPADDRESS( 192, 168, 0, 1 ) );
    //addr.sin_port = htons( 59999 );
    sockaddr_in temp;
    temp.sin_family = AF_INET;
    temp.sin_port = htons( (u_short)globalCount + 5000 );
    temp.sin_addr.s_addr = htonl( 0 );
    //DzBind( fd, (sockaddr*)&temp, sizeof(sockaddr_in) );
    int ret = DzConnect( fd, (sockaddr*)&addr, sizeof(sockaddr_in) );
    if( ret != -1 ){
        int bytes = DzRecv( fd, buff, PACKAGE_LEN, 0 );
        if( bytes == 4 ){
            index = *(int*)buff;
            printf( "START HOST, index is\t\t%d\r\n", index );
        }else{
            recvErrCount++;
            printf( "recv error! count = %d\r\n", recvErrCount );
            DzShutdown( fd, DZ_SD_RDWR );
            DzCloseSocket( fd );
            return -1;
        }

        DzSleep( 3000 + rand() % 300 );

        conn.fd = fd;
        conn.index = index;
        DzStartCot( SendRountine, &conn );

        while( working ){
            bytes = DzRecv( fd, buff, PACKAGE_LEN, 0 );
            if( bytes < 0 ){
                recvErrCount++;
                printf( "recv error! count = %d\r\n", recvErrCount );
                DzShutdown( fd, DZ_SD_RDWR );
                DzCloseSocket( fd );
                return -1;
            }else{
                conn.recvCount++;
                conn.recvTotal++;
            }
        }
    }else{
        //printf( "connect error code: %d\r\n", WSAGetLastError() );
        connErrCount++;
    }
    DzShutdown( fd, DZ_SD_RDWR );
    DzCloseSocket( fd );

    return 1;
}

int __stdcall TestMultiMapHost( void *context )
{
    int count = (int)context;
    //DzInitCotPool( count > 40000 ? 40000 : count, 0 );
    for( int i=0; i<count; i++ ){
        DzStartCot( TestMapHost, 0 );
    }
    return 0;
}
