
#include "stdafx.h"
#include <gtest/gtest.h>
#include "Util.h"
#include "DRandom.h"


#define DZ_TEST_SOCKET_SVR
#define DZ_TEST_SOCKET_CLT

#define DZ_READ_BUFF_SIZE       512
#define DZ_WRITE_BUFF_SIZE      512

//static unsigned long gIp = DZMAKEIPADDRESS( 192, 168, 137, 32 );
//static unsigned long gIp = DZMAKEIPADDRESS( 192, 168, 137, 1 );
//static unsigned long gIp = DZMAKEIPADDRESS( 192, 168, 1, 10 );
static unsigned long gIp = DZMAKEIPADDRESS( 127, 0, 0, 1 );
static unsigned short gPort = 9999;
static int gCltDelay = 1500;
static int gSendInterval = 0;

#define DZ_TEST_IOV_COUNT       ( 4 )
#define DZ_TEST_READ_BUFF_LEN   ( 64 * DZ_READ_BUFF_SIZE )
#define DZ_TEST_WRITE_BUFF_LEN  ( 64 * DZ_WRITE_BUFF_SIZE )

struct TestStream
{
    int        idx;
    int        len;
};

static int gTestCount = 0;
static DRandom* gRand = NULL;
static char** gBufArr = NULL;
static int gBuffArrCount = 0;
static int gMaxBuffLen = 0;
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
    ZeroMemory( addr, sizeof( sockaddr_in ) );
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
    gEndEvt = DzCreateManualEvt( FALSE );
    gHelloEvt = DzCreateManualEvt( FALSE );
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
    gMaxBuffLen = minBuffSize + randRange;
    gRand = new DRandom( seed );
    gBufArr = new char*[ count ];

    for( int i = 0; i < count; i++ ){
        int len = minBuffSize + gRand->rand( 0, randRange );
        gBufArr[i] = new char[ len ];
        TestStream *stream = (TestStream*)gBufArr[i];
        stream->idx = i;
        stream->len = len;
        len -= sizeof( TestStream );
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

void WaitAllCotEnd( int expectMaxCotCount )
{
    gCotCount--;
    if( gCotCount == 0 ){
        DzSetEvt( gEndEvt );
    }
    DzWaitSynObj( gEndEvt );

    EXPECT_EQ( expectMaxCotCount, gMaxCotCount );
#ifdef DZ_TEST_SOCKET_SVR
    DzSetEvt( gHelloEvt );
#endif

#ifdef DZ_TEST_SOCKET_CLT
    SayHello( NULL );
#endif
}

int ReadFunc( int fd, void* buff, int len, sockaddr* from, int* fromLen )
{
    return (int)DzReadFile( fd, buff, len );
}

int RecvFunc( int fd, void* buff, int len, sockaddr* from, int* fromLen )
{
    return DzRecv( fd, buff, len, 0 );
}

int RecvFromFunc( int fd, void* buff, int len, sockaddr* from, int* fromLen )
{
    return DzRecvFrom( fd, buff, len, 0, from, fromLen );
}

int RecvExFunc( int fd, void* buff, int len, sockaddr* from, int* fromLen )
{
    char* startPos = (char*)buff;
    DzBuf buffs[ DZ_TEST_IOV_COUNT ];
    int buffLen = len / DZ_TEST_IOV_COUNT;
    for( int i = 0; i < DZ_TEST_IOV_COUNT; i++ ){
        buffs[i].buf = startPos;
        buffs[i].len = buffLen;
        startPos += buffLen;
    }
    buffs[ DZ_TEST_IOV_COUNT - 1 ].len += len % DZ_TEST_IOV_COUNT;
    return DzRecvEx( fd, buffs, DZ_TEST_IOV_COUNT, 0 );
}

int RecvFromExFunc( int fd, void* buff, int len, sockaddr* from, int* fromLen )
{
    char* startPos = (char*)buff;
    DzBuf buffs[ DZ_TEST_IOV_COUNT ];
    int buffLen = len / DZ_TEST_IOV_COUNT;
    for( int i = 0; i < DZ_TEST_IOV_COUNT; i++ ){
        buffs[i].buf = startPos;
        buffs[i].len = buffLen;
        startPos += buffLen;
    }
    buffs[ DZ_TEST_IOV_COUNT - 1 ].len += len % DZ_TEST_IOV_COUNT;
    return DzRecvFromEx( fd, buffs, DZ_TEST_IOV_COUNT, 0, from, fromLen );
}

int WriteFunc( int fd, const void* buff, int len, const sockaddr* to, int toLen )
{
    return (int)DzWriteFile( fd, buff, len );
}

int SendFunc( int fd, const void* buff, int len, const sockaddr* to, int toLen )
{
    return DzSend( fd, buff, len, 0 );
}

int SendToFunc( int fd, const void* buff, int len, const sockaddr* to, int toLen )
{
    return DzSendTo( fd, buff, len, 0, to, toLen );
}

int SendExFunc( int fd, const void* buff, int len, const sockaddr* to, int toLen )
{
    char* startPos = (char*)buff;
    DzBuf buffs[ DZ_TEST_IOV_COUNT ];
    int buffLen = len / DZ_TEST_IOV_COUNT;
    for( int i = 0; i < DZ_TEST_IOV_COUNT; i++ ){
        buffs[i].buf = startPos;
        buffs[i].len = buffLen;
        startPos += buffLen;
    }
    buffs[ DZ_TEST_IOV_COUNT - 1 ].len += len % DZ_TEST_IOV_COUNT;
    return DzSendEx( fd, buffs, DZ_TEST_IOV_COUNT, 0 );
}

int SendToExFunc( int fd, const void* buff, int len, const sockaddr* to, int toLen )
{
    char* startPos = (char*)buff;
    DzBuf buffs[ DZ_TEST_IOV_COUNT ];
    int buffLen = len / DZ_TEST_IOV_COUNT;
    for( int i = 0; i < DZ_TEST_IOV_COUNT; i++ ){
        buffs[i].buf = startPos;
        buffs[i].len = buffLen;
        startPos += buffLen;
    }
    buffs[ DZ_TEST_IOV_COUNT - 1 ].len += len % DZ_TEST_IOV_COUNT;
    return DzSendToEx( fd, buffs, DZ_TEST_IOV_COUNT, 0, to, toLen );
}

typedef int ( *FuncRead )( int fd, void* buff, int len, sockaddr* from, int* fromLen );
typedef int ( *FuncWrite )( int fd, const void* buff, int len, const sockaddr* to, int toLen );

static int gNextReadFuncIdx = 0;
static int gNextWriteFuncIdx = 0;

static FuncRead gReadFuncArr[] = {
    ReadFunc,
    RecvFunc,
    RecvExFunc,
    RecvFromFunc,
    RecvFromExFunc
};
static FuncWrite gWriteFuncArr[] = {
    WriteFunc,
    SendFunc,
    SendExFunc,
    SendToFunc,
    SendToExFunc
};

void* GetRwFunc( void** arr, int size, int* idx )
{
    int count = size / sizeof( void* );
    if( *idx < 0 ){
        ( *idx )++;
        void* ret = arr[ - ( *idx ) ];
        if( *idx == 0 ){
            *idx = - count + 1;
        }
        return ret;
    }else if( *idx > 0 ){
        return arr[ ( *idx ) - 1 ];
    }else{
        return arr[ gRand->rand( 0, count ) ];
    }
}

void SetRwFunc( void** arr, int size, int* nextIdx, void* func, BOOL loop )
{
    int count = size / sizeof( void* );
    if( !func ){
        if( loop ){
            *nextIdx = - count;
        }else{
            *nextIdx = 0;
        }
    }else{
        for( int i = 0; i < count; i++ ){
            if( arr[i] == func ){
                *nextIdx = i + 1;
                if( loop ){
                    *nextIdx = - *nextIdx;
                }
                return;
            }
        }
    }
}

FuncRead GetReadFunc()
{
    return (FuncRead)GetRwFunc(
        (void**)gReadFuncArr, sizeof( gReadFuncArr ), &gNextReadFuncIdx
        );
}

FuncWrite GetWriteFunc()
{
    return (FuncWrite)GetRwFunc(
        (void**)gWriteFuncArr, sizeof( gWriteFuncArr ), &gNextWriteFuncIdx
        );
}

void SetReadFunc( FuncRead readFunc, BOOL loop = FALSE )
{
    SetRwFunc(
        (void**)gReadFuncArr, sizeof( gReadFuncArr ), &gNextReadFuncIdx,
        (void*)readFunc, loop
        );
}

void SetWriteFunc( FuncWrite writeFunc, BOOL loop = FALSE )
{
    SetRwFunc(
        (void**)gWriteFuncArr, sizeof( gWriteFuncArr ), &gNextWriteFuncIdx,
        (void*)writeFunc, loop
        );
}

void CltMain( DzRoutine cltRoutine, int count )
{
    for( int i = 0; i < count; i++ ){
        CotStart( cltRoutine, (void*)i );
    }
    CotStop();
}

int TcpReadOneStream( FuncRead readFunc, int lisFd )
{
    int buffLen = DZ_TEST_READ_BUFF_LEN;
    if( buffLen > gMaxBuffLen ){
        buffLen = gMaxBuffLen;
    }
    char* buff = new char[ buffLen ];

    TestStream ts;
    char* cmp = NULL;
    int tmp;
    int recvTimeCount = 0;
    int recvLen = 0;
    do{
        __DbgTce5( "recv start %d\r\n", recvTimeCount++ );
        tmp = readFunc( lisFd, buff, buffLen, NULL, NULL );
        if( tmp < 0 ){
            delete[] buff;
            throw (int)__LINE__;
        }
        recvLen += tmp;
        __DbgTce5( "recv : %d,\t total : %d\r\n", tmp, recvLen );
        if( !cmp ){
            if( recvLen < sizeof( TestStream ) ){
                int tmp1;
                do{
                    tmp1 = readFunc( lisFd, buff + recvLen, buffLen - recvLen, NULL, NULL );
                    if( tmp1 <= 0 ){
                        throw (int)__LINE__;
                    }
                    recvLen += tmp1;
                }while( recvLen < sizeof( TestStream ) );
            }
            ts.idx = ( (TestStream*)buff )->idx;
            ts.len = ( (TestStream*)buff )->len;
            cmp = gBufArr[ ts.idx ];
            tmp = recvLen;
        }
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
    }while( tmp > 0 && recvLen < ts.len );
    EXPECT_EQ( ts.len, recvLen ) << "recv time : " << recvTimeCount;
    delete[] buff;
    return ts.idx;
}

void TcpWriteOneStream( FuncWrite writeFunc, int fd, int idx )
{
    char* buff = gBufArr[ idx ];
    TestStream& ts = *(TestStream*)buff;
    int buffLen = ts.len;
    int emptyLen = DZ_TEST_WRITE_BUFF_LEN;

    int tmp;
    int sendTimeCount = 0;
    int sendLen = 0;
    do{
        if( emptyLen > buffLen - sendLen ){
            emptyLen = buffLen - sendLen;
        }
        __DbgTce5( "send start %d\r\n", sendTimeCount++ );
        tmp = writeFunc( fd, buff + sendLen, emptyLen, NULL, NULL );
        if( tmp < 0 ){
            throw (int)__LINE__;
        }
        sendLen += tmp;
        __DbgTce5( "send : %d,\t total : %d\r\n", tmp, sendLen );
        if( gSendInterval ){
            DzSleep( gSendInterval );
        }
    }while( tmp > 0 && sendLen < buffLen );
    EXPECT_EQ( buffLen, sendLen ) << "send time : " << sendTimeCount;
}

void TcpSvrMain( DzRoutine svrRoutine, int count )
{
    DzStartCot( WaitHello );

    int lisSock = -1;
    try{
        lisSock = DzSocket( gAddr->sa_family, SOCK_STREAM, 0 );
        if( lisSock == -1 ){
            throw (int)__LINE__;
        }
        int ret = DzBind( lisSock, gAddr, gAddrLen );
        if( ret != 0 ){
            throw (int)__LINE__;
        }

        ret = DzListen( lisSock, SOMAXCONN );
        if( ret != 0 ){
            throw (int)__LINE__;
        }

        int connCount = count;
        sockaddr acptAddr;
        while( connCount ){
            int acptAddrLen = sizeof( sockaddr );
            int fd = DzAccept( lisSock, &acptAddr, &acptAddrLen );
            if( fd == -1 ){
                throw (int)__LINE__;
            }
            CotStart( svrRoutine, (void*)fd );
            connCount--;
        }
    }catch( int line ){
        ADD_FAILURE() << "socket error, line : " << line;
    }
    if( lisSock != -1 ){
        DzCloseSocket( lisSock );
    }
    CotStop();
}

int UdpReadOneStream( FuncRead readFunc, int fd, sockaddr* from = NULL, int* fromLen = NULL )
{
    int buffLen = gMaxBuffLen;
    char* buff = new char[ buffLen ];
    int tmp = readFunc( fd, buff, buffLen, from, fromLen );
    if( tmp < 0 ){
        throw (int)__LINE__;
    }
    TestStream* ts = (TestStream*)buff;
    EXPECT_EQ( ts->len, tmp );
    return ts->idx;
}

void UdpWriteOneStream( FuncWrite writeFunc, int fd, int idx, const sockaddr* to = NULL, int toLen = 0 )
{
    TestStream* ts = (TestStream*)gBufArr[ idx ];
    int tmp = writeFunc( fd, gBufArr[ idx ], ts->len, to, toLen );
    if( tmp < 0 ){
        throw (int)__LINE__;
    }
    EXPECT_EQ( ts->len, tmp );
}

void UdpSvrMain( DzRoutine svrRoutine, int count )
{
    DzChangePriority( CP_HIGH );
    DzStartCot( WaitHello );

    int fd = -1;
    try{
        fd = DzSocket( gAddr->sa_family, SOCK_DGRAM, 0 );
        if( fd == -1 ){
            throw (int)__LINE__;
        }
        if( DzBind( fd, gAddr, gAddrLen ) != 0 ){
            throw (int)__LINE__;
        }

        int connCount = count;
        while( connCount ){
            int idx = UdpReadOneStream( GetReadFunc(), fd );
            CotStart( svrRoutine, (void*)idx );
            connCount--;
        }
    }catch( int line ){
        ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzCloseSocket( fd );
    }
    CotStop();
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

    WaitAllCotEnd( n * ( 1 + testCount ) );
    FreeParam();
}

void __stdcall TcpSvrRecvRoutine( void* context )
{
    int fd = (int)context;
    try{
        TcpReadOneStream( GetReadFunc(), fd );
    }catch( int line ){
        ADD_FAILURE() << "socket error, line : " << line;
    }
    DzCloseSocket( fd );
    CotStop();
}

void __stdcall TcpCltSendRoutine( void* context )
{
    int fd = -1;
    try{
        fd = DzSocket( gAddr->sa_family, SOCK_STREAM, 0 );
        if( fd == -1 ){
            throw (int)__LINE__;
        }
        if( DzConnect( fd, gAddr, gAddrLen ) != 0 ){
            throw (int)__LINE__;
        }
        int idx = (int)context;
        TcpWriteOneStream( GetWriteFunc(), fd, idx );
    }catch( int line ){
        ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzCloseSocket( fd );
    }
    CotStop();
}

void __stdcall TcpSvrSendRoutine( void* context )
{
    int fd = (int)context;
    try{
        TcpWriteOneStream( GetWriteFunc(), fd, 0 );
    }catch( int line ){
        ADD_FAILURE() << "socket error, line : " << line;
    }
    DzCloseSocket( fd );
    CotStop();
}

void __stdcall TcpCltRecvRoutine( void* context )
{
    int fd = -1;
    try{
        fd = DzSocket( gAddr->sa_family, SOCK_STREAM, 0 );
        if( fd == -1 ){
            throw (int)__LINE__;
        }
        if( DzConnect( fd, gAddr, gAddrLen ) != 0 ){
            throw (int)__LINE__;
        }
        int idx = TcpReadOneStream( GetReadFunc(), fd );
        EXPECT_EQ( 0, idx );
    }catch( int line ){
        ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzCloseSocket( fd );
    }
    CotStop();
}

void __stdcall TcpSvrRsrsRoutine( void* context )
{
    int fd = (int)context;
    try{
        int idx = TcpReadOneStream( GetReadFunc(), fd );
        TcpWriteOneStream( GetWriteFunc(), fd, idx + 1 );
        int idx1 = TcpReadOneStream( GetReadFunc(), fd );
        EXPECT_EQ( idx + 2, idx1 );
        TcpWriteOneStream( GetWriteFunc(), fd, idx1 + 1 );
    }catch( int line ){
        ADD_FAILURE() << "socket error, line : " << line;
    }
    DzCloseSocket( fd );
    CotStop();
}

void __stdcall TcpCltSrsrRoutine( void* context )
{
    int fd = -1;
    try{
        fd = DzSocket( gAddr->sa_family, SOCK_STREAM, 0 );
        if( fd == -1 ){
            throw (int)__LINE__;
        }
        if( DzConnect( fd, gAddr, gAddrLen ) != 0 ){
            throw (int)__LINE__;
        }
        int idx = (int)context;
        TcpWriteOneStream( GetWriteFunc(), fd, idx );
        int idx1 = TcpReadOneStream( GetReadFunc(), fd );
        EXPECT_EQ( idx + 1, idx1 );
        TcpWriteOneStream( GetWriteFunc(), fd, idx1 + 1 );
        int idx2 = TcpReadOneStream( GetReadFunc(), fd );
        EXPECT_EQ( idx1 + 2, idx2 );
    }catch( int line ){
        ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzCloseSocket( fd );
    }
    CotStop();
}

void __stdcall TcpSvrRecvCloseRoutine( void* context )
{
    FuncRead readFunc = GetReadFunc();
    int fd = (int)context;
    char buff[ 32 ];

    unsigned long long start = DzMilUnixTime();
    int ret = readFunc( fd, buff, 32, NULL, NULL );
    unsigned long long stop = DzMilUnixTime();
    EXPECT_EQ( 0, ret );
    EXPECT_GE( 200, stop - start );
    DzCloseSocket( fd );
    CotStop();
}

void __stdcall TcpCltRecvCloseRoutine( void* context )
{
    FuncWrite writeFunc = GetWriteFunc();
    int idx = (int)context;
    int fd = -1;
    try{
        fd = DzSocket( gAddr->sa_family, SOCK_STREAM, 0 );
        if( fd == -1 ){
            throw (int)__LINE__;
        }
        if( DzConnect( fd, gAddr, gAddrLen ) != 0 ){
            throw (int)__LINE__;
        }
        DzSleep( 100 );
        if( idx % 2 ){
            if( DzShutdown( fd, 2 ) != 0 ){
                throw (int)__LINE__;
            }
            DzSleep( 300 );
        }
        if( DzCloseSocket( fd ) != 0 ){
            throw (int)__LINE__;
        }
        fd = -1;
    }catch( int line ){
        ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzCloseSocket( fd );
    }
    CotStop();
}

void __stdcall TcpSvrSendCloseRoutine( void* context )
{
    FuncRead readFunc = GetReadFunc();
    int fd = (int)context;
    int idx;
    try{
        int ret = readFunc( fd, &idx, sizeof( idx ), NULL, NULL );
        EXPECT_EQ( sizeof( idx ), ret );
        DzSleep( 100 );
        if( idx % 2 ){
            if( DzShutdown( fd, 2 ) != 0 ){
                throw (int)__LINE__;
            }
            DzSleep( 300 );
        }
        if( DzCloseSocket( fd ) != 0 ){
            throw (int)__LINE__;
        }
        fd = -1;
    }catch( int line ){
        ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzCloseSocket( fd );
    }
    CotStop();
}

void __stdcall TcpCltSendCloseRoutine( void* context )
{
    FuncWrite writeFunc = GetWriteFunc();
    int idx = (int)context;
    int fd = -1;
    try{
        fd = DzSocket( gAddr->sa_family, SOCK_STREAM, 0 );
        if( fd == -1 ){
            throw (int)__LINE__;
        }
        if( DzConnect( fd, gAddr, gAddrLen ) != 0 ){
            throw (int)__LINE__;
        }
        int ret;
        unsigned long long start = DzMilUnixTime();
        do{
            ret = writeFunc( fd, &idx, sizeof( idx ), NULL, 0 );
            DzSleep( 10 );
        }while( ret > 0 );
        unsigned long long stop = DzMilUnixTime();
        EXPECT_EQ( -1, ret );
        EXPECT_GE( 200, stop - start );
    }catch( int line ){
        ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzCloseSocket( fd );
    }
    CotStop();
}

void __stdcall UdpSvrRecvSendRoutine( void* context )
{
    int idx = (int)context;
    int fd = -1;
    try{
        sockaddr_in addr;
        addr.sin_family = gAddr->sa_family;
        addr.sin_addr.s_addr = hton32( gIp );
        addr.sin_port = hton16( gPort + 2 + idx );

        fd = DzSocket( gAddr->sa_family, SOCK_DGRAM, 0 );
        if( fd == -1 ){
            throw (int)__LINE__;
        }
        if( DzConnect( fd, (sockaddr*)&addr, sizeof( addr ) ) != 0 ){
            throw (int)__LINE__;
        }
        UdpWriteOneStream( GetWriteFunc(), fd, idx + 1 );
    }catch( int line ){
        ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzCloseSocket( fd );
    }
    CotStop();
}

void __stdcall UdpCltSendRecvRoutine( void* context )
{
    int idx = (int)context;
    int fd = -1;
    int recvFd = -1;

    DzSleep( gRand->rand( 0, 1000 ) );
    try{
        fd = DzSocket( gAddr->sa_family, SOCK_DGRAM, 0 );
        if( fd == -1 ){
            throw (int)__LINE__;
        }
        if( DzConnect( fd, gAddr, gAddrLen ) != 0 ){
            throw (int)__LINE__;
        }

        sockaddr_in addr;
        addr.sin_family = gAddr->sa_family;
        addr.sin_addr.s_addr = hton32( gIp );
        addr.sin_port = hton16( gPort + 2 + idx );

        recvFd = DzSocket( gAddr->sa_family, SOCK_DGRAM, 0 );
        if( recvFd == -1 ){
            throw (int)__LINE__;
        }
        if( DzBind( recvFd, (sockaddr*)&addr, sizeof( addr ) ) != 0 ){
            throw (int)__LINE__;
        }

        UdpWriteOneStream( GetWriteFunc(), fd, idx );

        int ret = UdpReadOneStream( GetReadFunc(), recvFd );
        EXPECT_EQ( idx + 1, ret );
    }catch( int line ){
        ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzCloseSocket( fd );
    }
    if( recvFd != -1 ){
        DzCloseSocket( recvFd );
    }
    CotStop();
}

void __stdcall UdpSvrRecvSendNoConnRoutine( void* context )
{
    int idx = (int)context;
    int fd = -1;
    try{
        sockaddr_in addr;
        addr.sin_family = gAddr->sa_family;
        addr.sin_addr.s_addr = hton32( 0 );
        addr.sin_port = hton16( gPort - idx );

        fd = DzSocket( gAddr->sa_family, SOCK_DGRAM, 0 );
        if( fd == -1 ){
            throw (int)__LINE__;
        }
        if( DzBind( fd, (sockaddr*)&addr, sizeof( addr ) ) != 0 ){
            throw (int)__LINE__;
        }

        sockaddr_in addr1;
        addr1.sin_family = gAddr->sa_family;
        addr1.sin_addr.s_addr = hton32( gIp );
        addr1.sin_port = hton16( gPort + 2 + idx );

        UdpWriteOneStream( GetWriteFunc(), fd, idx + 1, (sockaddr*)&addr1, sizeof( addr1 ) );
    }catch( int line ){
        ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd == -1 ){
        DzCloseSocket( fd );
    }
    CotStop();
}

void __stdcall UdpCltSendRecvNoConnRoutine( void* context )
{
    int idx = (int)context;
    int fd = -1;

    DzSleep( gRand->rand( 0, 1000 ) );
    try{
        sockaddr_in addr;
        addr.sin_family = gAddr->sa_family;
        addr.sin_addr.s_addr = hton32( 0 );
        addr.sin_port = hton16( gPort + 2 + idx );

        fd = DzSocket( gAddr->sa_family, SOCK_DGRAM, 0 );
        if( fd == -1 ){
            throw (int)__LINE__;
        }
        if( DzBind( fd, (sockaddr*)&addr, sizeof( addr ) ) != 0 ){
            throw (int)__LINE__;
        }

        UdpWriteOneStream( GetWriteFunc(), fd, idx, gAddr, gAddrLen );

        sockaddr_in addr1;
        int addr1Len = sizeof( addr1 );
        int ret = UdpReadOneStream( GetReadFunc(), fd, (sockaddr*)&addr1, &addr1Len );
        EXPECT_EQ( idx + 1, ret );
        EXPECT_EQ( hton16( gPort - idx ), addr1.sin_port );
        EXPECT_EQ( ( (sockaddr_in*)gAddr )->sin_addr.s_addr, addr1.sin_addr.s_addr );
    }catch( int line ){
        ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzCloseSocket( fd );
    }
    CotStop();
}

void __stdcall TcpSvrRecvMain( void* context )
{
    TcpSvrMain( TcpSvrRecvRoutine, (int)context );
}

void __stdcall TcpCltSendMain( void* context )
{
    CltMain( TcpCltSendRoutine, (int)context );
}

void __stdcall TcpSvrSendMain( void* context )
{
    TcpSvrMain( TcpSvrSendRoutine, (int)context );
}

void __stdcall TcpCltRecvMain( void* context )
{
    CltMain( TcpCltRecvRoutine, (int)context );
}

void __stdcall TcpSvrRsrsMain( void* context )
{
    TcpSvrMain( TcpSvrRsrsRoutine, (int)context );
}

void __stdcall TcpCltSrsrMain( void* context )
{
    CltMain( TcpCltSrsrRoutine, (int)context );
}

void __stdcall TcpSvrRecvCloseMain( void* context )
{
    TcpSvrMain( TcpSvrRecvCloseRoutine, (int)context );
}

void __stdcall TcpCltRecvCloseMain( void* context )
{
    CltMain( TcpCltRecvCloseRoutine, (int)context );
}

void __stdcall TcpSvrSendCloseMain( void* context )
{
    TcpSvrMain( TcpSvrSendCloseRoutine, (int)context );
}

void __stdcall TcpCltSendCloseMain( void* context )
{
    CltMain( TcpCltSendCloseRoutine, (int)context );
}

void __stdcall HelpCloseSocket( void* context )
{
    int fd = (int)context;
    DzCloseSocket( fd );
}

void __stdcall TcpSvrAcceptCloseMain( void* context )
{
    DzStartCot( WaitHello );

    int lisSock = -1;
    try{
        lisSock = DzSocket( gAddr->sa_family, SOCK_STREAM, 0 );
        if( lisSock == -1 ){
            throw (int)__LINE__;
        }
        int ret = DzBind( lisSock, gAddr, gAddrLen );
        if( ret != 0 ){
            throw (int)__LINE__;
        }

        DzSleep( gCltDelay + 1000 );
        ret = DzListen( lisSock, SOMAXCONN );
        if( ret != 0 ){
            throw (int)__LINE__;
        }

        DzHandle timer = DzCreateCallbackTimer( 200, 1, HelpCloseSocket, (void*)lisSock );
        sockaddr acptAddr;
        int acptAddrLen = sizeof( sockaddr );
        int fd = DzAccept( lisSock, &acptAddr, &acptAddrLen );
        EXPECT_EQ( -1, fd );
        lisSock = -1;
        DzCloseCallbackTimer( timer );
    }catch( int line ){
        ADD_FAILURE() << "socket error, line : " << line;
    }
    if( lisSock != -1 ){
        DzCloseSocket( lisSock );
    }
    CotStop();
}

void __stdcall TcpCltConnectCloseMain( void* context )
{
    int fd = -1;
    try{
        fd = DzSocket( gAddr->sa_family, SOCK_STREAM, 0 );
        if( fd == -1 ){
            throw (int)__LINE__;
        }
        DzHandle timer = DzCreateCallbackTimer( 200, 1, HelpCloseSocket, (void*)fd );
        int ret = DzConnect( fd, gAddr, gAddrLen );
        EXPECT_EQ( -1, ret );
        DzCloseCallbackTimer( timer );
    }catch( int line ){
        ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzCloseSocket( fd );
    }
    CotStop();
}

void __stdcall UdpSvrRecvSendMain( void* context )
{
    UdpSvrMain( UdpSvrRecvSendRoutine, (int)context );
}

void __stdcall UdpCltSendRecvMain( void* context )
{
    CltMain( UdpCltSendRecvRoutine, (int)context );
}

void __stdcall UdpSvrRecvSendNoConnMain( void* context )
{
    UdpSvrMain( UdpSvrRecvSendNoConnRoutine, (int)context );
}

void __stdcall UdpCltSendRecvNoConnMain( void* context )
{
    CltMain( UdpCltSendRecvNoConnRoutine, (int)context );
}

void __stdcall TcpTestSimpleSend( void* context )
{
    int cotCount = 1;
    InitBuffArray( 2345135, cotCount, 1024, 0 );
    SocketTestFrame( TcpSvrRecvMain, TcpCltSendMain, cotCount );
    DeleteBuffArray();
}

void __stdcall TcpTestSimpleRecv( void* context )
{
    int cotCount = 1;
    InitBuffArray( 82534, cotCount, 1024, 0 );
    SocketTestFrame( TcpSvrSendMain, TcpCltRecvMain, cotCount );
    DeleteBuffArray();
}

void __stdcall TcpTestLargeBuffer( void* context )
{
    int cotCount = 1;
    InitBuffArray( 9672122, cotCount, 64 * 1024 * 1024, 0 );
    SocketTestFrame( TcpSvrRecvMain, TcpCltSendMain, cotCount );
    DeleteBuffArray();
}

void __stdcall TcpTestSendRecvSendRecv( void* context )
{
    int cotCount = 1;
    InitBuffArray( 654752, cotCount + 3, 4* 1024 * 1024, 512 * 1024 );
    SocketTestFrame( TcpSvrRsrsMain, TcpCltSrsrMain, cotCount );
    DeleteBuffArray();
}

void __stdcall TcpTestMultiSendRecvSendRecv( void* context )
{
    int cotCount = 100;
    InitBuffArray( 654752, cotCount + 3, 1024, 512 );
    SocketTestFrame( TcpSvrRsrsMain, TcpCltSrsrMain, cotCount );
    DeleteBuffArray();
}

void __stdcall TcpTestHugeSendRecvSendRecv( void* context )
{
    int cotCount = 500;
    InitBuffArray( 654752, cotCount + 3, 64, 32 );
    SocketTestFrame( TcpSvrRsrsMain, TcpCltSrsrMain, cotCount );
    DeleteBuffArray();
}

void __stdcall TcpTestRecvCloseValue( void* context )
{
    SocketTestFrame( TcpSvrRecvCloseMain, TcpCltRecvCloseMain, 5 );
}

void __stdcall TcpTestSendCloseValue( void* context )
{
    SocketTestFrame( TcpSvrSendCloseMain, TcpCltSendCloseMain, 5 );
}

void __stdcall TcpTestConnectAcceptClose( void* context )
{
    SocketTestFrame( TcpSvrAcceptCloseMain, TcpCltConnectCloseMain, 0 );
}

void __stdcall UdpTestSendRecv( void* context )
{
    int cotCount = 1;
    InitBuffArray( 2546257, cotCount + 1, 1024, 256 );
    SocketTestFrame( UdpSvrRecvSendMain, UdpCltSendRecvMain, cotCount );
    DeleteBuffArray();
}

void __stdcall UdpTestMultiSendRecv( void* context )
{
    int cotCount = 100;
    InitBuffArray( 342553, cotCount + 1, 1024, 256 );
    SocketTestFrame( UdpSvrRecvSendMain, UdpCltSendRecvMain, cotCount );
    DeleteBuffArray();
}

void __stdcall UdpTestSendRecvNoConn( void* context )
{
    int cotCount = 1;
    InitBuffArray( 2546257, cotCount + 1, 1024, 256 );
    SocketTestFrame( UdpSvrRecvSendNoConnMain, UdpCltSendRecvNoConnMain, cotCount );
    DeleteBuffArray();
}

void __stdcall UdpTestMultiSendRecvNoConn( void* context )
{
    int cotCount = 100;
    InitBuffArray( 342553, cotCount + 1, 1024, 256 );
    SocketTestFrame( UdpSvrRecvSendNoConnMain, UdpCltSendRecvNoConnMain, cotCount );
    DeleteBuffArray();
}

TEST( TestTcpSocket, SimpleSend )
{
    SetReadFunc( RecvFunc );
    SetWriteFunc( SendFunc );
    TestCot( TcpTestSimpleSend );
}

TEST( TestTcpSocket, SimpleRecv )
{
    SetReadFunc( ReadFunc );
    SetWriteFunc( WriteFunc );
    TestCot( TcpTestSimpleRecv );
}

TEST( TestTcpSocket, LargeBuffer )
{
    SetReadFunc( RecvFromFunc );
    SetWriteFunc( SendToFunc );
    TestCot( TcpTestLargeBuffer );
}

TEST( TestTcpSocket, SendRecvSendRecv )
{
    SetReadFunc( NULL, TRUE );
    SetWriteFunc( NULL, TRUE );
    TestCot( TcpTestSendRecvSendRecv );
}

TEST( TestTcpSocket, MultiSendRecvSendRecv )
{
    SetReadFunc( NULL, FALSE );
    SetWriteFunc( NULL, FALSE );
    TestCot( TcpTestMultiSendRecvSendRecv );
}

TEST( TestTcpSocket, HugeSendRecvSendRecv )
{
    SetReadFunc( NULL, FALSE );
    SetWriteFunc( NULL, FALSE );
    TestCot( TcpTestHugeSendRecvSendRecv );
}

TEST( TestTcpSocket, RecvCloseValue )
{
    SetReadFunc( RecvFunc, TRUE );
    SetWriteFunc( WriteFunc, TRUE );
    TestCot( TcpTestRecvCloseValue );
}

TEST( TestTcpSocket, SendCloseValue )
{
    SetReadFunc( RecvFunc, TRUE );
    SetWriteFunc( WriteFunc, TRUE );
    TestCot( TcpTestSendCloseValue );
}

TEST( TestTcpSocket, ConnectAcceptClose )
{
    TestCot( TcpTestConnectAcceptClose );
}

TEST( TestUdpSocket, SendRecv )
{
    SetReadFunc( RecvFromExFunc, TRUE );
    SetWriteFunc( SendToFunc, TRUE );
    TestCot( UdpTestSendRecv );
}

TEST( TestUdpSocket, MultiSendRecv )
{
    SetReadFunc( RecvFromFunc, TRUE );
    SetWriteFunc( SendToExFunc, TRUE );
    TestCot( UdpTestMultiSendRecv );
}

TEST( TestUdpSocket, SendRecvNoConn )
{
    SetReadFunc( RecvFromExFunc );
    SetWriteFunc( SendToFunc );
    TestCot( UdpTestSendRecvNoConn );
}

TEST( TestUdpSocket, MultiSendRecvNoConn )
{
    SetReadFunc( RecvFromFunc );
    SetWriteFunc( SendToExFunc );
    TestCot( UdpTestMultiSendRecvNoConn );
}
