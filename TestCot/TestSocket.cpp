
#include "stdafx.h"
#include <gtest/gtest.h>
#include "Util.h"
#include "DRandom.h"
#include <WinSock2.h>
#include "../Dzcot/Inc_DzSockUtils.h"


#define DZ_TEST_SOCKET_SVR
//#define DZ_TEST_SOCKET_CLT

//#define DZ_SMALL_RECV_BUFF
#define DZ_SMALL_SEND_BUFF

static unsigned long gIp = DZMAKEIPADDRESS( 192, 168, 1, 10 );
//static unsigned long gIp = DZMAKEIPADDRESS( 127, 0, 0, 1 );
static unsigned short gPort = 9999;
static int gCltDelay = 500;
static int gSmallBuffSize = 64;

struct TestStream
{
    int        idx;
    int        len;
};

static int gTestCount = 0;
static DRandom* gRand = NULL;
static char** gBufArr = NULL;
static int gBuffArrCount = 0;
static sockaddr* gAddr = NULL;
static int gAddrLen = 0;
static int gCotCount = 0;
static int gMaxCotCount = 0;
static DzHandle gEndEvt = 0;

void InitParam()
{
    sockaddr_in* addr = new sockaddr_in;
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = hton32( gIp );
    addr->sin_port = hton16( gPort + gTestCount );
    gTestCount++;

    gAddr = (sockaddr*)addr;
    gAddrLen = sizeof( sockaddr_in );
    gCotCount = 1;
    gMaxCotCount = 0;
    gEndEvt = DzCreateEvt( TRUE, FALSE );
}

void FreeParam()
{
    DzCloseSynObj( gEndEvt );
    delete gAddr;
    gAddr = NULL;
    gAddrLen = 0;
}

void InitBuffArray( int seed, int count, int minBuffSize, int randRange )
{
    gRand = new DRandom( seed );
    gBufArr = new char*[ count ];

    for( int i = 0; i < count; i++ ){
        int len = minBuffSize + gRand->rand( 0, randRange );
        gBufArr[i] = new char[ len ];
        TestStream *stream = (TestStream*)gBufArr[i];
        len -= sizeof( TestStream );
        stream->idx = i;
        stream->len = len;
        char* p = gBufArr[i] + sizeof( TestStream );
        char* end = gBufArr[i] + len;
        int n = len / sizeof( unsigned long );
        for( int j = 0; j < n; j++ ){
            *(unsigned long*)p = gRand->rand();
            p += sizeof( unsigned long );
        }
        n = gRand->rand();
        int j = 0;
        while( p < end ){
            *p = ((char*)&n)[j];
        }
    }
    gBuffArrCount = count;
}

void DeleteBuffArray()
{
    for( int i = 0; i < gBuffArrCount; i++ ){
        delete[] gBufArr[i];
    }
    delete[] gBufArr;
    delete gRand;
    gBufArr = NULL;
    gRand = NULL;
    gBuffArrCount = 0;
}

void CotStart(
    DzRoutine   entry,
    void*       context,
    int         priority = CP_DEFAULT,
    int         sSize = SS_DEFAULT
    )
{
    gMaxCotCount++;
    gCotCount++;
    DzStartCot( entry, context, priority, sSize );
}

void CotStop()
{
    gCotCount--;
    if( gCotCount == 0 ){
        DzSetEvt( gEndEvt );
    }
}

void WaitAllCotEnd( int expectMaxCot )
{
    gCotCount--;
    if( gCotCount == 0 ){
        DzSetEvt( gEndEvt );
    }
    DzWaitSynObj( gEndEvt );
    EXPECT_EQ( expectMaxCot, gMaxCotCount );
}

int TcpRecvOneStream( int fd )
{
    DzBuf buffs[ DZ_IOV_MAX ];
    TestStream ts;

    int recvLen = 0;
    do{
        int tmp = DzRecv( fd, (char*)&ts + recvLen, sizeof( TestStream ) - recvLen, 0 );
        recvLen += tmp;
    }while( recvLen < sizeof( TestStream ) );

#ifdef DZ_SMALL_RECV_BUFF
    int tmp;
    int buffLen = 16384;
    char* buff = new char[ buffLen ];
    buffs[0].buf = buff;
    buffs[0].len = gSmallBuffSize;
    buffs[1].buf = buff + gSmallBuffSize;
    buffs[1].len = gSmallBuffSize;

    int i = 2;
    int recvTimeCount = 0;
    recvLen = 0;
    char* cmp = gBufArr[ ts.idx ] + sizeof( TestStream );
    do{
        printf( "recv start %d\r\n", recvTimeCount++ );
        tmp = DzRecvEx( fd, buffs, i, 0 );
        recvLen += tmp;
        printf( "recv : %d,\t total : %d\r\n", tmp, recvLen );
        int ret = memcmp( buff, cmp, tmp );
        EXPECT_EQ( 0, ret );
        cmp += tmp;
    }while( tmp > 0 && recvLen < ts.len );
    delete[] buff;
#else
    int buffLen = ts.len + 256;
    char* buff = new char[ buffLen ];

    int n = (int)gRand->rand( 1, DZ_IOV_MAX + 1 );
    int tmp = buffLen / n;
    int minStep = tmp * 2 / 3;
    int maxStep = minStep * 2;

    int recvTimeCount = 0;
    recvLen = 0;
    do{
        int i = 0;
        int nowPos = recvLen;
        while( 1 ){
            buffs[i].buf = buff + nowPos;
            int tmp = gRand->rand( minStep, maxStep );
            buffs[i].len = tmp;
            i++;
            if( i == n || nowPos + tmp >= buffLen ){
                buffs[ i - 1 ].len = buffLen - nowPos;
                break;
            }
            nowPos += tmp;
        }

        printf( "recv start %d\r\n", recvTimeCount++ );
        tmp = DzRecvEx( fd, buffs, i, 0 );
        recvLen += tmp;
        printf( "recv : %d,\t total : %d\r\n", tmp, recvLen );
    }while( tmp > 0 && recvLen < ts.len );

    EXPECT_EQ( ts.len, recvLen );
    int ret = memcmp( buff, gBufArr[ ts.idx ] + sizeof( TestStream ) , recvLen );
    EXPECT_EQ( 0, ret );
    delete[] buff;
#endif
    return ts.idx;
}

void TcpSendOneStream( int fd, int idx )
{
    DzBuf buffs[ DZ_IOV_MAX ];

    char* buff = gBufArr[ idx ];
    TestStream& ts = *(TestStream*)buff;
    int buffLen = ts.len + sizeof( TestStream );

#ifdef DZ_SMALL_SEND_BUFF
    int tmp;
    int sendTimeCount = 0;
    int sendLen = 0;
    do{
        int i = 0;
        int nowPos = sendLen;
        while( i < 2 ){
            buffs[i].buf = buff + nowPos;
            buffs[i].len = gSmallBuffSize;
            i++;
            if( nowPos + gSmallBuffSize >= buffLen ){
                buffs[ i - 1 ].len = buffLen - nowPos;
                break;
            }
            nowPos += gSmallBuffSize;
        }

        printf( "send start %d\r\n", sendTimeCount++ );
        tmp = DzSendEx( fd, buffs, i, 0 );
        sendLen += tmp;
        printf( "send : %d,\t total : %d\r\n", tmp, sendLen );
        DzSleep( 10 );
    }while( tmp > 0 && sendLen < buffLen );
#else
    int n = (int)gRand->rand( 1, DZ_IOV_MAX + 1 );
    int tmp = buffLen / n;
    int minStep = tmp * 2 / 3;
    int maxStep = minStep * 2;

    int sendTimeCount = 0;
    int sendLen = 0;
    do{
        int i = 0;
        int nowPos = sendLen;
        while( 1 ){
            buffs[i].buf = buff + nowPos;
            int tmp = gRand->rand( minStep, maxStep );
            buffs[i].len = tmp;
            i++;
            if( i == n || nowPos + tmp >= buffLen ){
                buffs[ i - 1 ].len = buffLen - nowPos;
                break;
            }
            nowPos += tmp;
        }

        printf( "send start %d\r\n", sendTimeCount++ );
        tmp = DzSendEx( fd, buffs, i, 0 );
        sendLen += tmp;
        printf( "send : %d,\t total : %d\r\n", tmp, sendLen );
    }while( tmp > 0 && sendLen < buffLen );
#endif
    EXPECT_EQ( buffLen, sendLen );
}

void TcpSvrMain( DzRoutine svrRoutine, int count )
{
    int lisSock = DzSocket( gAddr->sa_family, SOCK_STREAM, 0 );
    if( lisSock == -1 ){
        ADD_FAILURE();
        CotStop();
        return;
    }

    int ret = DzBind( lisSock, gAddr, gAddrLen );
    if( ret != 0 ){
        DzCloseSocket( lisSock );
        ADD_FAILURE();
        CotStop();
        return;
    }

    ret = DzListen( lisSock, SOMAXCONN );
    if( ret != 0 ){
        DzCloseSocket( lisSock );
        ADD_FAILURE();
        CotStop();
        return;
    }

    int connCount = count;
    sockaddr acptAddr;
    while( connCount ){
        int acptAddrLen = sizeof( sockaddr );
        int fd = DzAccept( lisSock, &acptAddr, &acptAddrLen );
        if( fd == -1 ){
            ADD_FAILURE();
            break;
        }
        CotStart( svrRoutine, (void*)fd );
        connCount--;
    }
    DzCloseSocket( lisSock );
    CotStop();
}

void TcpCltMain( DzRoutine cltRoutine, int count )
{
    for( int i = 0; i < count; i++ ){
        CotStart( cltRoutine, (void*)i );
    }
    CotStop();
}

int TcpCltConnect()
{
    int fd = DzSocket( gAddr->sa_family, SOCK_STREAM, 0 );
    if( fd == -1 ){
        ADD_FAILURE();
        return -1;
    }
    int ret = DzConnect( fd, gAddr, gAddrLen );
    if( ret != 0 ){
        DzCloseSocket( fd );
        ADD_FAILURE();
        return -1;
    }
    return fd;
}

void SocketTestFrame(
    DzRoutine   svrTstEntry,
    DzRoutine   cltTstEntry,
    int         testCount,
    int         cltWait = 0
    )
{
    int n = 0;
    InitParam();

#ifdef DZ_TEST_SOCKET_SVR
    CotStart( svrTstEntry, (void*)testCount );
    n++;
#endif

#ifdef DZ_TEST_SOCKET_CLT
    DzSleep( gCltDelay );
    CotStart( cltTstEntry, (void*)testCount );
    if( cltWait ){
        DzSleep( cltWait );
    }
    n++;
#endif

    WaitAllCotEnd( n * ( testCount + 1 ) );
    FreeParam();
}

void __stdcall TcpSvrRecvRoutine( void* context )
{
    int fd = (int)context;
    TcpRecvOneStream( fd );
    DzCloseSocket( fd );
    CotStop();
}

void __stdcall TcpCltSendRoutine( void* context )
{
    int fd = TcpCltConnect();
    if( fd == -1 ){
        ADD_FAILURE();
        CotStop();
        return;
    }
    int idx = (int)context;
    TcpSendOneStream( fd, idx );
    DzCloseSocket( fd );
    CotStop();
}

void __stdcall TcpSvrRsrsRoutine( void* context )
{
    int fd = (int)context;
    int idx = TcpRecvOneStream( fd );
    TcpSendOneStream( fd, idx + 1 );
    int idx1 = TcpRecvOneStream( fd );
    EXPECT_EQ( idx + 2, idx1 );
    TcpSendOneStream( fd, idx1 + 1 );
    DzCloseSocket( fd );
    CotStop();
}

void __stdcall TcpCltSrsrRoutine( void* context )
{
    int fd = TcpCltConnect();
    if( fd == -1 ){
        ADD_FAILURE();
        CotStop();
        return;
    }
    int idx = (int)context;
    TcpSendOneStream( fd, idx );
    int idx1 = TcpRecvOneStream( fd );
    EXPECT_EQ( idx + 1, idx1 );
    TcpSendOneStream( fd, idx1 + 1 );
    int idx2 = TcpRecvOneStream( fd );
    EXPECT_EQ( idx1 + 2, idx2 );
    DzCloseSocket( fd );
    CotStop();
}

void __stdcall TcpSvrRecvMain( void* context )
{
    TcpSvrMain( TcpSvrRecvRoutine, (int)context );
}

void __stdcall TcpCltSendMain( void* context )
{
    TcpCltMain( TcpCltSendRoutine, (int)context );
}

void __stdcall TcpSvrRsrsMain( void* context )
{
    TcpSvrMain( TcpSvrRsrsRoutine, (int)context );
}

void __stdcall TcpCltSrsrMain( void* context )
{
    TcpCltMain( TcpCltSrsrRoutine, (int)context );
}

void __stdcall TestSimpleSend( void* context )
{
    int cotCount = 1;
    InitBuffArray( 2345135, cotCount, 1024, 0 );
    SocketTestFrame( TcpSvrRecvMain, TcpCltSendMain, cotCount );
    DeleteBuffArray();
}

void __stdcall TestLargeBuffer( void* context )
{
    int cotCount = 1;
    InitBuffArray( 9672122, cotCount, 64 * 1024 * 1024, 0 );
    SocketTestFrame( TcpSvrRecvMain, TcpCltSendMain, cotCount, 2500 );
    DeleteBuffArray();
}

void __stdcall TestSendRecvSendRecv( void* context )
{
    int cotCount = 1;
    InitBuffArray( 654752, cotCount + 3, 4* 1024 * 1024, 512 * 1024 );
    SocketTestFrame( TcpSvrRsrsMain, TcpCltSrsrMain, cotCount );
    DeleteBuffArray();
}

void __stdcall TestMultiSendRecvSendRecv( void* context )
{
    int cotCount = 100;
    InitBuffArray( 654752, cotCount + 3, 1024, 512 );
    SocketTestFrame( TcpSvrRsrsMain, TcpCltSrsrMain, cotCount );
    DeleteBuffArray();
}

void __stdcall TestHugeSendRecvSendRecv( void* context )
{
    int cotCount = 500;
    InitBuffArray( 654752, cotCount + 3, 64, 32 );
    SocketTestFrame( TcpSvrRsrsMain, TcpCltSrsrMain, cotCount );
    DeleteBuffArray();
}

TEST( TestSocket, SimpleSend )
{
    TestCot( TestSimpleSend );
}

TEST( TestSocket, LargeBuffer )
{
    TestCot( TestLargeBuffer );
}

TEST( TestSocket, SendRecvSendRecv )
{
    TestCot( TestSendRecvSendRecv );
}

TEST( TestSocket, MultiSendRecvSendRecv )
{
    TestCot( TestMultiSendRecvSendRecv );
}

TEST( TestSocket, HugeSendRecvSendRecv )
{
    TestCot( TestHugeSendRecvSendRecv );
}
