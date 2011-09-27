
#include "stdafx.h"
#include <gtest/gtest.h>
#include "Util.h"
#include "DRandom.h"
#include <WinSock2.h>
#include "../Dzcot/Inc_DzSockUtils.h"


#define DZ_TEST_SOCKET_SVR
#define DZ_TEST_SOCKET_CLT

#define DZ_SMALL_RECV_BUFF
//#define DZ_SMALL_SEND_BUFF

static unsigned long gIp = DZMAKEIPADDRESS( 192, 168, 1, 10 );
//static unsigned long gIp = DZMAKEIPADDRESS( 127, 0, 0, 1 );
static unsigned short gPort = 9999;
static int gCltDelay = 1500;
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
static sockaddr* gHelloAddr = NULL;
static int gAddrLen = 0;
static int gCotCount = 0;
static int gMaxCotCount = 0;
static DzHandle gEndEvt = 0;
static DzHandle gHelloEvt = 0;

void InitParam()
{
    gTestCount++;
    sockaddr_in* addr = new sockaddr_in;
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = hton32( gIp );
    addr->sin_port = hton16( gPort );
    gAddr = (sockaddr*)addr;

    addr = new sockaddr_in;
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = hton32( gIp );
    addr->sin_port = hton16( gPort + 1 );
    gHelloAddr = (sockaddr*)addr;

    gAddrLen = sizeof( sockaddr_in );
    gCotCount = 1;
    gMaxCotCount = 0;
    gEndEvt = DzCreateEvt( TRUE, FALSE );
    gHelloEvt = DzCreateEvt( TRUE, FALSE );
}

void FreeParam()
{
    DzCloseSynObj( gHelloEvt );
    DzCloseSynObj( gEndEvt );
    delete gAddr;
    gAddr = NULL;
    delete gHelloAddr;
    gHelloAddr = NULL;
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
        char* end = p + len;
        int n = len / sizeof( unsigned long );
        for( int j = 0; j < n; j++ ){
            *(unsigned long*)p = gRand->rand();
            p += sizeof( unsigned long );
        }
        n = gRand->rand();
        int j = 0;
        while( p < end ){
            *p++ = ((char*)&n)[j];
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

void __stdcall SayHello( void* context )
{
    int fd = DzSocket( gHelloAddr->sa_family, SOCK_STREAM, 0 );
    if( fd == -1 ){
        ADD_FAILURE();
        return;
    }
    int ret = DzConnect( fd, gHelloAddr, gAddrLen );
    if( ret != 0 ){
        DzCloseSocket( fd );
        ADD_FAILURE();
        return;
    }

    char sendBuff[] = "hello";
    char recvBuff[ 16 ];
    DzSend( fd, sendBuff, sizeof( sendBuff ), 0 );
    DzRecv( fd, recvBuff, sizeof( recvBuff ), 0 );

    DzCloseSocket( fd );
}

void __stdcall WaitHello( void* context )
{
    int lisFd = DzSocket( gHelloAddr->sa_family, SOCK_STREAM, 0 );
    if( lisFd == -1 ){
        ADD_FAILURE();
        return;
    }
    int ret = DzBind( lisFd, gHelloAddr, gAddrLen );
    if( ret != 0 ){
        DzCloseSocket( lisFd );
        ADD_FAILURE();
        return;
    }
    ret = DzListen( lisFd, SOMAXCONN );
    if( ret != 0 ){
        DzCloseSocket( lisFd );
        ADD_FAILURE();
        return;
    }
    int fd = DzAccept( lisFd, NULL, NULL );
    if( fd < 0 ){
        DzCloseSocket( lisFd );
        DzCloseSocket( fd );
        ADD_FAILURE();
        return;
    }

    char sendBuff[] = "hello";
    char recvBuff[ 16 ];
    DzRecv( fd, recvBuff, sizeof( recvBuff ), 0 );
    DzWaitSynObj( gHelloEvt );
    DzSend( fd, sendBuff, sizeof( sendBuff ), 0 );

    DzCloseSocket( fd );
    DzCloseSocket( lisFd );
}

void WaitAllCotEnd( int expectMaxCot )
{
    gCotCount--;
    if( gCotCount == 0 ){
        DzSetEvt( gEndEvt );
    }
    DzWaitSynObj( gEndEvt );
    EXPECT_EQ( expectMaxCot, gMaxCotCount );

#ifdef DZ_TEST_SOCKET_SVR
    DzSetEvt( gHelloEvt );
#endif

#ifdef DZ_TEST_SOCKET_CLT
    SayHello( NULL );
#endif
}

int TcpReadOneStream( int lisFd, BOOL waitEnd )
{
    TestStream ts;

    int recvLen = 0;
    do{
        int tmp = DzReadFile( lisFd, (char*)&ts + recvLen, sizeof( TestStream ) - recvLen );
        recvLen += tmp;
    }while( recvLen < sizeof( TestStream ) );
    char* cmp = gBufArr[ ts.idx ] + sizeof( TestStream );

#ifdef DZ_SMALL_RECV_BUFF
    int buffLen = gSmallBuffSize;
    char* buff = new char[ buffLen ];
#else
    int buffLen = ts.len + 256;
    char* buff = new char[ buffLen ];
#endif

    int tmp;
    int recvTimeCount = 0;
    recvLen = 0;
    do{
        __DbgTce5( "recv start %d\r\n", recvTimeCount++ );
        tmp = DzReadFile( lisFd, buff, buffLen );
        recvLen += tmp;
        __DbgTce5( "recv : %d,\t total : %d\r\n", tmp, recvLen );
        if( tmp > 0 ){
            int ret = memcmp( buff, cmp, tmp );
            if( ret ){
                for( int i = 0; i < tmp; i++ ){
                    if( buff[i] != cmp[i] ){
                        ret = i;
                        break;
                    }
                }
            }
            EXPECT_EQ( 0, ret );
            cmp += tmp;
        }
    }while( tmp > 0 && ( waitEnd || recvLen < ts.len ) );

    if( waitEnd ){
        EXPECT_EQ( 0, tmp );
    }
    EXPECT_EQ( ts.len, recvLen );
    delete[] buff;
    return ts.idx;
}

void TcpWriteOneStream( int lisFd, int idx )
{
    char* buff = gBufArr[ idx ];
    TestStream& ts = *(TestStream*)buff;
    int buffLen = ts.len + sizeof( TestStream );

#ifdef DZ_SMALL_SEND_BUFF
    int emptyLen = gSmallBuffSize;
    int tmp;
    int sendTimeCount = 0;
    int sendLen = 0;
    do{
        __DbgTce5( "send start %d\r\n", sendTimeCount++ );
        tmp = DzWriteFile( lisFd, buff + sendLen, emptyLen );
        sendLen += tmp;
        __DbgTce5( "send : %d,\t total : %d\r\n", tmp, sendLen );
        if( emptyLen < buffLen - sendLen ){
            emptyLen = buffLen - sendLen;
        }
        DzSleep( 10 );
    }while( tmp > 0 && sendLen < buffLen );
#else
    int emptyLen = buffLen;
    int tmp;
    int sendTimeCount = 0;
    int sendLen = 0;
    do{
        __DbgTce5( "send start %d\r\n", sendTimeCount++ );
        tmp = DzWriteFile( lisFd, buff + sendLen, emptyLen );
        sendLen += tmp;
        __DbgTce5( "send : %d,\t total : %d\r\n", tmp, sendLen );
        emptyLen -= tmp;
    }while( tmp > 0 && sendLen < buffLen );
#endif
    EXPECT_EQ( buffLen, sendLen );
}

int TcpRecvOneStream( int lisFd, BOOL waitEnd )
{
    DzBuf buffs[ DZ_IOV_MAX ];
    TestStream ts;

    int recvLen = 0;
    do{
        int tmp = DzRecv( lisFd, (char*)&ts + recvLen, sizeof( TestStream ) - recvLen, 0 );
        recvLen += tmp;
    }while( recvLen < sizeof( TestStream ) );
    char* cmp = gBufArr[ ts.idx ] + sizeof( TestStream );

#ifdef DZ_SMALL_RECV_BUFF
    int tmp;
    int buffLen = gSmallBuffSize * 2;
    char* buff = new char[ buffLen ];
    buffs[0].buf = buff;
    buffs[0].len = gSmallBuffSize;
    buffs[1].buf = buff + gSmallBuffSize;
    buffs[1].len = gSmallBuffSize;

    int i = 2;
    int recvTimeCount = 0;
    recvLen = 0;
    do{
        __DbgTce5( "recv start %d\r\n", recvTimeCount++ );
        tmp = DzRecvEx( lisFd, buffs, i, 0 );
        recvLen += tmp;
        __DbgTce5( "recv : %d,\t total : %d\r\n", tmp, recvLen );
        if( tmp > 0 ){
            int ret = memcmp( buff, cmp, tmp );
            if( ret ){
                for( int i = 0; i < tmp; i++ ){
                    if( buff[i] != cmp[i] ){
                        ret = i;
                        break;
                    }
                }
            }
            EXPECT_EQ( 0, ret );
            EXPECT_EQ( 0, ret );
            cmp += tmp;
        }
    }while( tmp > 0 && ( waitEnd || recvLen < ts.len ) );
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

        __DbgTce5( "recv start %d\r\n", recvTimeCount++ );
        tmp = DzRecvEx( lisFd, buffs, i, 0 );
        recvLen += tmp;
        __DbgTce5( "recv : %d,\t total : %d\r\n", tmp, recvLen );
        if( tmp > 0 ){
            int ret = memcmp( buffs[0].buf, cmp, tmp );
            if( ret ){
                for( int i = 0; i < tmp; i++ ){
                    if( buffs[0].buf[i] != cmp[i] ){
                        ret = i;
                        break;
                    }
                }
            }
            EXPECT_EQ( 0, ret );
            cmp += tmp;
        }
    }while( tmp > 0 && ( waitEnd || recvLen < ts.len ) );
#endif
    if( waitEnd ){
        EXPECT_EQ( 0, tmp );
    }
    EXPECT_EQ( ts.len, recvLen );
    delete[] buff;
    return ts.idx;
}

void TcpSendOneStream( int lisFd, int idx )
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

        __DbgTce5( "send start %d\r\n", sendTimeCount++ );
        tmp = DzSendEx( lisFd, buffs, i, 0 );
        sendLen += tmp;
        __DbgTce5( "send : %d,\t total : %d\r\n", tmp, sendLen );
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

        __DbgTce5( "send start %d\r\n", sendTimeCount++ );
        tmp = DzSendEx( lisFd, buffs, i, 0 );
        sendLen += tmp;
        __DbgTce5( "send : %d,\t total : %d\r\n", tmp, sendLen );
    }while( tmp > 0 && sendLen < buffLen );
#endif
    EXPECT_EQ( buffLen, sendLen );
}

void TcpSvrMain( DzRoutine svrRoutine, int count )
{
    DzStartCot( WaitHello );

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
    int         testCount
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
    n++;
#endif

    WaitAllCotEnd( n * ( testCount + 1 ) );
    FreeParam();
}

void __stdcall TcpSvrRecvRoutine( void* context )
{
    int fd = (int)context;
    TcpRecvOneStream( fd, TRUE );
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
    int idx = TcpRecvOneStream( fd, FALSE );
    TcpWriteOneStream( fd, idx + 1 );
    int idx1 = TcpReadOneStream( fd, FALSE );
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
    int idx1 = TcpRecvOneStream( fd, FALSE );
    EXPECT_EQ( idx + 1, idx1 );
    TcpWriteOneStream( fd, idx1 + 1 );
    int idx2 = TcpReadOneStream( fd, TRUE );
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
    SocketTestFrame( TcpSvrRecvMain, TcpCltSendMain, cotCount );
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
