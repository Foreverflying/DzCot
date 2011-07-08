#include "StdAfx.h"
#include "Global.h"
#include "TestSocket.h"
#include "../Dzcot/Dzcot.h"
#include <assert.h>
#include <malloc.h>

#define TEST_BUFF_LEN       ( 16 * 1024 * 1024 )

static int* gBuff;
static int gSvrCount;
static int gClientCount;

struct ParamStruct
{
    sockaddr*       addr;
    int             addrLen;
    union{
        int         count;
        int         fd;
    };
    union{
        int         start;
        int         buffSize;
    };
    int             len;
    int             buffCount;
    DzHandle        evt;
};

void SvrCotStart()
{
    gSvrCount++;
}

void SvrCotStop( DzHandle evt )
{
    if( --gSvrCount == 0 ){
        printf( "here!\r\n" );
        DzSetEvt( evt );
    }
    printf( "SvrStop %d\r\n", gSvrCount );
    DzCloseSynObj( evt );
}

void __stdcall SvrRoutine( void* context )
{
    ParamStruct* node = (ParamStruct*)context;
    int s = node->fd;
    int* buf = new int[ node->buffSize * node->buffCount ];
    
    int* tmpBuf = buf;
    int step = node->buffSize;
    DzBuf* buffArr = (DzBuf*)alloca( sizeof(DzBuf) * node->buffCount );
    for( int i = 0; i < node->buffCount; i++ ){
        buffArr[i].buf = (char*)tmpBuf;
        buffArr[i].len = sizeof( int ) * step;
        tmpBuf += step;
    }

    int ret;
    int loopCount = 0;
    printf( "S: Server Recv %d\t len %d\r\n", *(gBuff + node->start), node->len );
    while( 1 ){
        loopCount++;
        int m = 0;
        ret = DzRecvEx( s, buffArr, node->buffCount, 0 );
        printf( "E: Server Recv %d\t len %d\t recvLen %d\r\n", *(gBuff + node->start), node->len, ret );
        if( ret > 0 ){
            int begin = buf[0];
            int n = ( ret + m ) / sizeof( int );
            for( int i = 0; i < n; i++ ){
                assert( buf[i] == gBuff[ begin + i ] );
            }
            m = ret % sizeof( int );
            if( m ){
                memmove( buf, buf + n, m );
                buffArr[0].buf = (char*)buf + m;
                buffArr[0].len = sizeof( int ) * step - m;
            }
        }else{
            break;
        }
        if( loopCount == 4 ){
            printf( "wo kao\r\n" );
            break;
        }
    }
    printf( "E: Server Recv %d\t len %d\t loop %d\r\n", *(gBuff + node->start), node->len, loopCount );
    delete buf;
    DzCloseSocket( s );
    SvrCotStop( node->evt );
    delete node;
}

void __stdcall HelpShutdownEntry( void* context )
{
    ParamStruct* node = (ParamStruct*)context;
    DzSleep( 3000 );
    DzWaitSynObj( node->evt );
    printf( "hello!\r\n" );
    //DzShutdown( node->fd, DZ_SD_RDWR );
    DzCloseSocket( node->fd );
    DzCloseSynObj( node->evt );
    delete node;
}

void __stdcall SvrEntry( void* context )
{
    ParamStruct* node = (ParamStruct*)context;

    int lisSock = DzSocket( node->addr->sa_family, SOCK_STREAM, 0 );
    DzBind( lisSock, node->addr, node->addrLen );
    DzListen( lisSock, 128 );

    ParamStruct* tmpNode = new ParamStruct;
    tmpNode->fd = lisSock;
    tmpNode->evt = DzCloneSynObj( node->evt );
    DzStartCot( HelpShutdownEntry, tmpNode );
    int start = 0;
    sockaddr* addr = new sockaddr;
    int addrLen;
    int sock = DzAccept( lisSock, addr, &addrLen );
    while( sock >= 0 ){
        printf( "Accept socket\r\n" );
        ParamStruct* ps = new ParamStruct;
        ps->addr = addr;
        ps->addrLen = addrLen;
        ps->fd = sock;
        ps->buffSize = node->buffSize;
        ps->len = node->len;
        ps->buffCount = node->buffCount;
        ps->evt = DzCloneSynObj( node->evt );
        SvrCotStart();
        DzStartCot( SvrRoutine, ps, CP_HIGH );
        addr = new sockaddr;
        sock = DzAccept( lisSock, addr, &addrLen );
    };
    printf( "accept routine quit\r\n" );
    DzCloseSocket( lisSock );
    //SvrCotStop( node->evt );
    delete addr;
    delete node;
}

void TestSvr( int len, int buffSize, int buffCount, sockaddr* addr, int addrLen, DzHandle evt )
{
    DzHandle svrEvt = DzCreateEvt( TRUE, FALSE );
    gSvrCount = 0;
    ParamStruct *node = new ParamStruct;
    node->addr = addr;
    node->addrLen = addrLen;
    node->buffSize = buffSize;
    node->len = len;
    node->buffCount = buffCount;
    node->evt = DzCloneSynObj( svrEvt );

    DzStartCot( SvrEntry, node, CP_HIGH );
    DzCloseSynObj( svrEvt );

}

void CltCotStart()
{
    gClientCount++;
}

void CltCotStop( DzHandle evt )
{
    if( --gClientCount == 0 ){
        DzSetEvt( evt );
    }
    printf( "CltStop %d\r\n", gClientCount );
    DzCloseSynObj( evt );
}

void __stdcall ClientRoutine( void* context )
{
    ParamStruct* node = (ParamStruct*)context;

    int* tmpBuf = gBuff + node->start;
    int step = node->len / node->buffCount;
    DzBuf* buffArr = (DzBuf*)alloca( sizeof(DzBuf) * node->buffCount );
    for( int i = 0; i < node->buffCount; i++ ){
        buffArr[i].buf = (char*)tmpBuf;
        buffArr[i].len = sizeof( int ) * step;
        tmpBuf += step;
    }
    buffArr[ node->buffCount - 1 ].len += node->len % node->buffCount;

    int s = DzSocket( node->addr->sa_family, SOCK_STREAM, 0 );
    int ret = DzConnect( s, node->addr, node->addrLen );
    assert( ret == 0 );
    printf( "S: Client Send %d\t len %d\r\n", *(gBuff + node->start), node->len );
    ret = DzSendEx( s, buffArr, node->buffCount, 0 );
    assert( ret == node->len * sizeof( int ) );
    printf( "E: Client Send %d\t len %d\t sendLen %d\r\n", *(gBuff + node->start), node->len, ret );
    DzCloseSocket( s );
    CltCotStop( node->evt );
    delete node;
}

void __stdcall ClientEntry( void* context )
{
    ParamStruct* node = (ParamStruct*)context;

    int start = node->start;
    int len = node->len / node->count;
    for( int i = 0; i < node->count; i++ ){
        ParamStruct *ps = new ParamStruct;
        ps->addr = node->addr;
        ps->addrLen = node->addrLen;
        ps->start = start;
        ps->len = len;
        ps->buffCount = node->buffCount;
        ps->evt = DzCloneSynObj( node->evt );
        CltCotStart();
        DzStartCot( ClientRoutine, ps, CP_HIGH );
        start += len;
    }
    CltCotStop( node->evt );
    delete node;
}

void TestClient( int count, int start, int len, int buffCount, sockaddr* addr, int addrLen, DzHandle evt )
{
    gClientCount = 0;
    ParamStruct *node = new ParamStruct;
    node->addr = addr;
    node->addrLen = addrLen;
    node->count = count;
    node->start = start;
    node->len = len;
    node->buffCount = buffCount;
    node->evt = DzCloneSynObj( evt );
    CltCotStart();
    DzStartCot( ClientEntry, node, CP_HIGH );
}

void TestSocket1()
{
    //test one server and one client, send large chunks
    sockaddr_in* addr = new sockaddr_in;
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = gIp;
    addr->sin_port = gPort;

    TestSvr( TEST_BUFF_LEN, 4096000, 3, (sockaddr*)addr, sizeof( sockaddr_in ) );

    DzHandle cltEvt = DzCreateEvt( TRUE, FALSE );
    TestClient( 1, 0, TEST_BUFF_LEN, 1, (sockaddr*)addr, sizeof( sockaddr_in ), cltEvt );
    DzWaitSynObj( cltEvt );
    DzCloseSynObj( cltEvt );

    printf( "End!\r\n" );
}

void __stdcall StartTestSocket( void* context )
{
    gBuff = new int[ TEST_BUFF_LEN ];
    for( int i = 0; i < TEST_BUFF_LEN; i++ ){
        *( gBuff + i ) = i;
    }
    TestSocket1();
    delete[] gBuff;
}
