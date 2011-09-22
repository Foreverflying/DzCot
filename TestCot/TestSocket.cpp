
#include "stdafx.h"
#include <gtest/gtest.h>
#include "Util.h"
#include "DRandom.h"
#include <WinSock2.h>
#include "../Dzcot/Inc_DzSockUtils.h"

struct TestStream
{
    int        idx;
    int        len;
};

static DRandom* gRand = NULL;
static char** gBufArr = NULL;
static int gBuffArrCount = 0;
static sockaddr* gAddr = NULL;
static int gAddrLen = 0;
static int gCotCount = 0;
static int gMaxCotCount = 0;
static DzHandle gEndEvt = 0;

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
    DzWaitSynObj( gEndEvt );
    EXPECT_EQ( expectMaxCot, gMaxCotCount );
}

void InitParam()
{
    sockaddr_in* addr = new sockaddr_in;
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = hton32( DZMAKEIPADDRESS( 127, 0, 0, 1 ) );
    addr->sin_port = hton16( 9999 );

    gAddr = (sockaddr*)addr;
    gAddrLen = sizeof( sockaddr_in );
    gCotCount = 0;
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

int TcpRecvOneStream( int fd )
{
    DzBuf buffs[ DZ_IOV_MAX ];
    TestStream ts;

    int recvLen = 0;
    do{
        int tmp = DzRecv( fd, (char*)&ts + recvLen, sizeof( TestStream ) - recvLen, 0 );
        recvLen += tmp;
    }while( recvLen < sizeof( TestStream ) );

    int buffLen = ts.len + 256;
    char* buff = new char[ buffLen ];

    int n = (int)gRand->rand( 1, DZ_IOV_MAX + 1 );
    int tmp = buffLen / n;
    int minStep = tmp * 2 / 3;
    int maxStep = minStep * 2;

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

        int tmp = DzRecvEx( fd, buffs, i, 0 );
        EXPECT_NE( 0, tmp );
        recvLen += tmp;
    }while( recvLen < ts.len );

    EXPECT_EQ( ts.len, recvLen );
    int ret = memcmp( buff, gBufArr[ ts.idx ] + sizeof( TestStream ) , recvLen );
    EXPECT_EQ( 0, ret );
    delete[] buff;
    return ts.idx;
}

void TcpSendOneStream( int fd, int idx )
{
    DzBuf buffs[ DZ_IOV_MAX ];

    char* buff = gBufArr[ idx ];
    TestStream& ts = *(TestStream*)buff;
    int buffLen = ts.len + sizeof( TestStream );

    int n = (int)gRand->rand( 1, DZ_IOV_MAX + 1 );
    int tmp = buffLen / n;
    int minStep = tmp * 2 / 3;
    int maxStep = minStep * 2;

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

        int tmp = DzSendEx( fd, buffs, i, 0 );
        sendLen += tmp;
    }while( sendLen < buffLen );
    EXPECT_EQ( buffLen, sendLen );
}

void TcpSvrMain( DzRoutine svrRoutine, int count )
{
    int lisSock = DzSocket( gAddr->sa_family, SOCK_STREAM, 0 );
    EXPECT_NE( -1, lisSock );

    int ret = DzBind( lisSock, gAddr, gAddrLen );
    EXPECT_EQ( 0, ret );

    ret = DzListen( lisSock, SOMAXCONN );
    EXPECT_EQ( 0, ret );

    int connCount = count;
    sockaddr acptAddr;
    while( connCount ){
        int acptAddrLen = sizeof( sockaddr );
        int fd = DzAccept( lisSock, &acptAddr, &acptAddrLen );
        EXPECT_NE( -1, fd );
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

void __stdcall TcpSvrRecvRoutine( void* context )
{
    int fd = (int)context;
    TcpRecvOneStream( fd );
    DzCloseSocket( fd );
    CotStop();
}

void __stdcall TcpCltSendRoutine( void* context )
{
    int fd = DzSocket( gAddr->sa_family, SOCK_STREAM, 0 );
    int ret = DzConnect( fd, gAddr, gAddrLen );
    EXPECT_EQ( 0, ret );
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
    int fd = DzSocket( gAddr->sa_family, SOCK_STREAM, 0 );
    int ret = DzConnect( fd, gAddr, gAddrLen );
    EXPECT_EQ( 0, ret );
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
    InitBuffArray( 2345135, 1, 1024, 0 );
    InitParam();
    CotStart( TcpSvrRecvMain, (void*)1 );
    DzSleep( 100 );
    CotStart( TcpCltSendMain, (void*)1 );
    WaitAllCotEnd( 4 );
    FreeParam();
    DeleteBuffArray();
}

void __stdcall TestLargeBuffer( void* context )
{
    InitBuffArray( 9672122, 1, 64 * 1024 * 1024, 0 );
    InitParam();
    CotStart( TcpSvrRecvMain, (void*)1 );
    DzSleep( 100 );
    CotStart( TcpCltSendMain, (void*)1 );
    WaitAllCotEnd( 4 );
    FreeParam();
    DeleteBuffArray();
}

void __stdcall TestSendRecvSendRecv( void* context )
{
    InitBuffArray( 654752, 4, 4* 1024 * 1024, 512 * 1024 );
    InitParam();
    CotStart( TcpSvrRsrsMain, (void*)1 );
    DzSleep( 100 );
    CotStart( TcpCltSrsrMain, (void*)1 );
    WaitAllCotEnd( 4 );
    FreeParam();
    DeleteBuffArray();
}

void __stdcall TestMultiSendRecvSendRecv( void* context )
{
    InitBuffArray( 654752, 103, 1024, 512 );
    InitParam();
    CotStart( TcpSvrRsrsMain, (void*)100 );
    DzSleep( 100 );
    CotStart( TcpCltSrsrMain, (void*)100 );
    WaitAllCotEnd( 202 );
    FreeParam();
    DeleteBuffArray();
}

void __stdcall TestHugeSendRecvSendRecv( void* context )
{
    InitBuffArray( 654752, 1003, 64, 32 );
    InitParam();
    CotStart( TcpSvrRsrsMain, (void*)1000 );
    DzSleep( 100 );
    CotStart( TcpCltSrsrMain, (void*)1000 );
    WaitAllCotEnd( 2002 );
    FreeParam();
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
