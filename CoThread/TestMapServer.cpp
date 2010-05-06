#include "stdafx.h"
#include <commctrl.h>
#include "../Dzcot/Dzcot.h"

#include "../Dzcot/DzQueue.h"
#include "DlMalloc.h"
#include "Global.h"

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
    int         fd;
    int         index;
    DzQueue     sendQueue;
};

struct Buff
{
    char        buff[PACKAGE_LEN];
    int         len;
    int         ref;
};

struct Msg
{
    DzQItr      qItr;
    Buff*       buff;
};

static volatile BOOL working = FALSE;
static Conn ** connArr = NULL;
static int hostCount = 0;

void AddBuffRef( Buff *buff )
{
    buff->ref++;
}

void ReleaseBuffRef( Buff *buff )
{
    buff->ref--;
    if( buff->ref == 0 ){
        Free( buff );
    }
}

Buff* CreateBuff()
{
    Buff *ret = (Buff*)Malloc( sizeof( Buff ) );
    ret->len = 0;
    ret->ref = 1;
    return ret;
}

int __stdcall SendRoutine( void *context )
{
    Conn *conn = (Conn*)context;

    DzQItr *qItr = conn->sendQueue.head;
    while( qItr ){
        Msg *msg = MEMBER_BASE( qItr, Msg, qItr );
        int bytes = DzSend( conn->fd, msg->buff->buff, msg->buff->len, 0 );
        if( bytes < 0 ){
            DelHeadQItr( &conn->sendQueue );
            ReleaseBuffRef( msg->buff );
            sendErrCount++;
            printf( "send error! count = %d\r\n", sendErrCount );
            break;
        }
        DelHeadQItr( &conn->sendQueue );
        ReleaseBuffRef( msg->buff );
        Free( msg );
        qItr = conn->sendQueue.head;
    }
    return 0;
}

int __stdcall TestMapServerRoutine( void *context )
{
    Conn *conn = (Conn*)context;

    int bytes = DzSend( conn->fd, &conn->index, sizeof( int ), 0 );
    if( bytes < 0 ){
        sendErrCount++;
        printf( "send error! count = %d\r\n", sendErrCount );
        DzShutDown( conn->fd, SD_BOTH );
        DzCloseSocket( conn->fd );
        return -1;
    }

    //char tmpbuf[PACKAGE_LEN];
    //DzRecv( conn->fd, tmpbuf, PACKAGE_LEN, 0 );
    //printf( " take a test \r\n" );

    while( working ){
        Buff *buff = CreateBuff();
        bytes = DzRecv( conn->fd, buff->buff, PACKAGE_LEN, 0 );
        if( bytes > 0 ){
            buff->len = bytes;
            for( int i=0; i<hostCount; i++ ){
                Msg *msg = (Msg*)Malloc( sizeof( Msg ) );
                msg->buff = buff;
                AddBuffRef( buff );
                if( connArr[i]->sendQueue.head == NULL ){
                    //start send cot
                    DzStartCot( SendRoutine, connArr[i] );
                }
                AddQItrToTail( &connArr[i]->sendQueue, &msg->qItr );
            }
            ReleaseBuffRef( buff );
        }else{
            ReleaseBuffRef( buff );
            recvErrCount++;
            printf( "recv error! count = %d\r\n", recvErrCount );
            break;
        }
    }
    DzShutDown( conn->fd, SD_BOTH );
    DzCloseSocket( conn->fd );
    return 1;
}

int __stdcall TestMapServer( void *context )
{
    DzStartCot( TraceMalloc, NULL );
    connArr = (Conn**)Malloc( sizeof( Conn** ) * MAX_HOST_COUNT );
    for( int i=0; i<MAX_HOST_COUNT; i++ ){
        connArr[i] = NULL;
    }

    working = TRUE;
    int count = (int)context;
    //DzInitCotPool( count > 40000 ? 40000 : count, 0 );
    int fd = DzSocket( AF_INET, SOCK_STREAM, 0 );
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = gIp;
    addr.sin_port = gPort;
    //addr.sin_addr.s_addr = htonl( MAKEIPADDRESS( 192, 168, 0, 1 ) );
    //addr.sin_port = htons( 59999 );
    DzBind( fd, (sockaddr*)&addr, sizeof(sockaddr_in) );
    if( DzListen( fd, count ) != 0 ){
        lisErrCount++;
        return 0;
    }
    printf( "Server Start...\r\n");
    while(1){
        int addrLen;
        int newFd = DzAccept( fd, (sockaddr*)&addr, &addrLen );
        if( newFd != -1 ){
            Conn *conn = (Conn*)Malloc( sizeof( Conn ) );
            conn->fd = newFd;
            conn->index = hostCount;
            connArr[ conn->index ] = conn;
            //printf( "Accept Connection:\t %d\r\n", hostCount );
            
            hostCount++;
            DzStartCot( TestMapServerRoutine, conn );
        }else{
            acptErrCount++;
            printf( "accept error! count = %d\r\n", acptErrCount );
        }
    }
    return 1;
}