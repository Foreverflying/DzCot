
#include "stdafx.h"
#include "../Dzcot/Dzcot.h"
#include "Global.h"

int __stdcall TestReadFile( void *context )
{
    int fd = DzOpenFileA( "d:\\temp\\emule.exe", DZ_O_RD );
    //off_t64 t = DzSeekFile( fd, 4000000, SEEK_END );
    char *buff = new char[ 10000000 ];
    int count = (int)DzReadFile( fd, buff, 10000000 );
    DzCloseFd( fd );
    printf( "count: %d\r\n", count );
    return count;
}

static int globalCount = 0;
static int connErrCount = 0;
static int acptErrCount = 0;
static int sendErrCount = 0;
static int recvErrCount = 0;
static int lisErrCount = 0;
static int serveCount = 0;

int __stdcall TestClient( void *context )
{
    if( globalCount == 0 ){
        printf( "start here:\t %d\r\n", GetTickCount() );
    }
    globalCount++;
    char buff[16];
    int fd = DzSocket( AF_INET, SOCK_STREAM, 0 );
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    //addr.sin_addr.S_addr = inet_addr( "127.0.0.1" );
    //addr.sin_addr.s_addr = htonl( MAKEIPADDRESS( 192, 168, 219, 134 ) );
    addr.sin_addr.s_addr = gIp;
    addr.sin_port = gPort;
    sockaddr_in temp;
    temp.sin_family = AF_INET;
    temp.sin_port = 0; //htons( (u_short)globalCount + 5000 );
    temp.sin_addr.s_addr = htonl( 0 );
    DzBind( fd, (sockaddr*)&temp, sizeof(sockaddr_in) );
    int ret = DzConnect( fd, (sockaddr*)&addr, sizeof(sockaddr_in) );
    if( ret != -1 ){
        int bytes = DzSend( fd, "hello", 5, 0 );
        if( bytes > 0 ){
            int recvBytes = DzRecv( fd, buff, 16, 0 );
            if( recvBytes > 0 ){
                buff[ recvBytes ] = '\0';
                //printf( "globalCount: %d\t recv bytes: %s\r\n", globalCount, buff );
            }else{
                recvErrCount++;
            }
        }else{
            sendErrCount++;
        }
    }else{
        //printf( "connect error code: %d\r\n", WSAGetLastError() );
        connErrCount++;
    }
    DzShutdown( fd, DZ_SD_RDWR );
    DzCloseSocket( fd );
    globalCount--;
    //printf( "globalCount: %d\r\n", globalCount );
    if( globalCount == 0 ){
        printf( "end here:\t %d\r\n", GetTickCount() );
        printf( "acptErrCount:\t %d\r\n", acptErrCount );
        printf( "connErrCount:\t %d\r\n", connErrCount );
        printf( "sendErrCount:\t %d\r\n", sendErrCount );
        printf( "recvErrCount:\t %d\r\n", recvErrCount );
        acptErrCount = 0;
        connErrCount = 0;
        sendErrCount = 0;
        recvErrCount = 0;
    }
    return 1;
}

int __stdcall TestMultiClient( void *context )
{
    int count = (int)context;
    DzInitCotPool( count > 40000 ? 40000 : count, 0 );
    for( int i=0; i<count; i++ ){
        DzStartCot( TestClient, 0, CP_HIGH );
    }
    return 0;
}

int __stdcall TestServerRoutine( void *context )
{
    char buff[16];
    int fd = (int)context;
    int bytes = DzRecv( fd, buff, 16, 0 );
    if( bytes > 0 ){
        bytes = DzSend( fd, "i love you", 10, 0 );
        if( bytes > 0 ){
            //
        }else{
            sendErrCount++;
        }
    }else{
        recvErrCount++;
    }
    DzShutdown( fd, DZ_SD_RDWR );
    DzCloseSocket( fd );
    globalCount--;
    //printf( "globalCount: %d\r\n", globalCount );
    if( globalCount == 0 ){
        printf( "end here:\t %d\r\n", GetTickCount() );
        printf( "acptErrCount:\t %d\r\n", acptErrCount );
        printf( "connErrCount:\t %d\r\n", connErrCount );
        printf( "sendErrCount:\t %d\r\n", sendErrCount );
        printf( "recvErrCount:\t %d\r\n", recvErrCount );
        acptErrCount = 0;
        connErrCount = 0;
        sendErrCount = 0;
        recvErrCount = 0;

        printf( "max cot count:\t %d\r\n", DzGetMaxCotCount( TRUE ) );
        printf( "served  count:\t %d\r\n", serveCount );
        serveCount = 0;
    }
    return 1;
}

int __stdcall TestServer( void *context )
{
    int count = (int)context;
    //DzInitCotPool( count > 40000 ? 40000 : count, 0 );
    int fd = DzSocket( AF_INET, SOCK_STREAM, 0 );
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl( MAKEIPADDRESS( 0, 0, 0, 0 ) );
    addr.sin_port = gPort;
    DzBind( fd, (sockaddr*)&addr, sizeof(sockaddr_in) );
    if( DzListen( fd, count ) != 0 ){
        lisErrCount++;
        return 0;
    }
    while(1){
        int addrLen;
        int newFd = DzAccept( fd, (sockaddr*)&addr, &addrLen );
        if( newFd != -1 ){
            if( globalCount == 0 ){
                printf( "start here:\t %d\r\n", GetTickCount() );
            }
            globalCount++;
            serveCount++;

            DzStartCot( TestServerRoutine, (void*)newFd );
        }else{
            acptErrCount++;
        }
    }
    return 1;
}
