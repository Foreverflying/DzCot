
#include "CotTestUtil.h"
#include "DRandom.h"

#define DZMAKEIPADDRESS( a1, a2, a3, a4 ) (\
    ((a1) << 24) | ((a2) << 16) | ((a3) << 8) | (a4)\
    )

#define hton16( n ) (\
    ( ( (n) & 0xff ) << 8 ) | ( ( (n) & 0xff00 ) >> 8 )\
    )

#define hton32( n ) (\
    ( ( (n) & 0xff ) << 24 ) | ( ( (n) & 0xff00 ) << 8 ) |\
    ( ( (n) & 0xff0000 ) >> 8 ) | ( ( (n) & 0xff000000 ) >> 24 )\
    )

#define hton64( n ) (\
    ( ( (n) & 0xff ) << 56 )             | ( ( (n) & 0xff00 ) << 40 ) |\
    ( ( (n) & 0xff0000 ) << 24 )         | ( ( (n) & 0xff000000 ) << 8 ) |\
    ( ( (n) & 0xff00000000 ) >> 8 )      | ( ( (n) & 0xff0000000000 ) >> 24 ) |\
    ( ( (n) & 0xff000000000000 ) >> 40 ) | ( ( (n) & 0xff00000000000000 ) >> 56 )\
    )

#define DZ_TEST_SOCKET_SVR
#define DZ_TEST_SOCKET_CLT

#define DZ_READ_BUFF_SIZE       512
#define DZ_WRITE_BUFF_SIZE      512

//static unsigned long gIp = DZMAKEIPADDRESS( 192, 168, 137, 132 );
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
    sockaddr_in* addr = (sockaddr_in*)DzMalloc( sizeof(sockaddr_in) );
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = hton32( gIp );
    addr->sin_port = hton16( gPort );
    gAddr = (sockaddr*)addr;

    addr = (sockaddr_in*)DzMalloc( sizeof(sockaddr_in) );
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
    DzDelSynObj( gHelloEvt );
    DzDelSynObj( gEndEvt );
    DzFree( gAddr );
    gAddr = NULL;
    DzFree( gHelloAddr );
    gHelloAddr = NULL;
    gAddrLen = 0;
}

void InitBuffArray( int seed, int count, int minBuffSize, int randRange )
{
    gMaxBuffLen = minBuffSize + randRange;
    gRand = new DRandom( seed );
    gBufArr = (char**)DzMalloc( count * sizeof(char*) );

    for( int i = 0; i < count; i++ ){
        int len = minBuffSize + gRand->rand( 0, randRange );
        gBufArr[i] = (char*)DzMalloc( len );
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
        DzFree( gBufArr[i] );
    }
    DzFree( gBufArr );
    delete gRand;
    gBufArr = NULL;
    gRand = NULL;
    gBuffArrCount = 0;
}

void CotStart(
    DzEntry     entry,
    intptr_t    context,
    int         priority = CP_DEFAULT,
    int         sType = ST_DEFAULT
    )
{
    gMaxCotCount++;
    gCotCount++;
    DzStartCot( entry, context, priority, sType );
}

void CotStop()
{
    gCotCount--;
    if( gCotCount == 0 ){
        DzSetEvt( gEndEvt );
    }
}

CotEntry SayHello( intptr_t context )
{
    int fd = -1;
    try{
        fd = DzSocket( gHelloAddr->sa_family, SOCK_STREAM, 0 );
        if( fd == -1 ){
            throw (int)__LINE__;
        }
        if( DzConnect( fd, gHelloAddr, gAddrLen ) != 0 ){
            throw (int)__LINE__;
        }

        char sendBuff[] = "hello";
        char recvBuff[ 16 ];
        DzSend( fd, sendBuff, sizeof( sendBuff ), 0 );
        DzRecv( fd, recvBuff, sizeof( recvBuff ), 0 );
    }catch( int line ){
        DZ_ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzClose( fd );
    }
}

CotEntry WaitHello( intptr_t context )
{
    int lisFd = -1;
    int fd = -1;
    DzHandle helloEvt = DzCloneSynObj( gHelloEvt );
    try{
        BOOL opt = TRUE;
        lisFd = DzSocket( gHelloAddr->sa_family, SOCK_STREAM, 0 );
        DzSetSockOpt( lisFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt) );
        if( lisFd == -1 ){
            throw (int)__LINE__;
        }
        if( DzBind( lisFd, gHelloAddr, gAddrLen ) != 0 ){
            throw (int)__LINE__;
        }
        if( DzListen( lisFd, SOMAXCONN ) != 0 ){
            throw (int)__LINE__;
        }
        fd = DzAccept( lisFd, NULL, NULL );
        if( fd == -1 ){
            throw (int)__LINE__;
        }

        char sendBuff[] = "hello";
        char recvBuff[ 16 ];
        DzRecv( fd, recvBuff, sizeof( recvBuff ), 0 );
        DzWaitSynObj( helloEvt );
        DzSend( fd, sendBuff, sizeof( sendBuff ), 0 );
    }catch( int line ){
        DZ_ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzClose( fd );
    }
    if( lisFd != -1 ){
        DzClose( lisFd );
    }
    DzDelSynObj( helloEvt );
}

void WaitAllCotEnd( int expectMaxCotCount )
{
    gCotCount--;
    if( gCotCount == 0 ){
        DzSetEvt( gEndEvt );
    }
    DzWaitSynObj( gEndEvt );

    DZ_EXPECT_EQ( expectMaxCotCount, gMaxCotCount );
#ifdef DZ_TEST_SOCKET_SVR
    DzSetEvt( gHelloEvt );
#endif

#ifdef DZ_TEST_SOCKET_CLT
    SayHello( 0 );
#endif
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
    DzIov buffs[ DZ_TEST_IOV_COUNT ];
    int buffLen = len / DZ_TEST_IOV_COUNT;
    for( int i = 0; i < DZ_TEST_IOV_COUNT; i++ ){
        buffs[i].buf = startPos;
        buffs[i].len = buffLen;
        startPos += buffLen;
    }
    buffs[ DZ_TEST_IOV_COUNT - 1 ].len += len % DZ_TEST_IOV_COUNT;
    return DzIovRecv( fd, buffs, DZ_TEST_IOV_COUNT, 0 );
}

int RecvFromExFunc( int fd, void* buff, int len, sockaddr* from, int* fromLen )
{
    char* startPos = (char*)buff;
    DzIov buffs[ DZ_TEST_IOV_COUNT ];
    int buffLen = len / DZ_TEST_IOV_COUNT;
    for( int i = 0; i < DZ_TEST_IOV_COUNT; i++ ){
        buffs[i].buf = startPos;
        buffs[i].len = buffLen;
        startPos += buffLen;
    }
    buffs[ DZ_TEST_IOV_COUNT - 1 ].len += len % DZ_TEST_IOV_COUNT;
    return DzIovRecvFrom( fd, buffs, DZ_TEST_IOV_COUNT, 0, from, fromLen );
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
    DzIov buffs[ DZ_TEST_IOV_COUNT ];
    int buffLen = len / DZ_TEST_IOV_COUNT;
    for( int i = 0; i < DZ_TEST_IOV_COUNT; i++ ){
        buffs[i].buf = startPos;
        buffs[i].len = buffLen;
        startPos += buffLen;
    }
    buffs[ DZ_TEST_IOV_COUNT - 1 ].len += len % DZ_TEST_IOV_COUNT;
    return DzIovSend( fd, buffs, DZ_TEST_IOV_COUNT, 0 );
}

int SendToExFunc( int fd, const void* buff, int len, const sockaddr* to, int toLen )
{
    char* startPos = (char*)buff;
    DzIov buffs[ DZ_TEST_IOV_COUNT ];
    int buffLen = len / DZ_TEST_IOV_COUNT;
    for( int i = 0; i < DZ_TEST_IOV_COUNT; i++ ){
        buffs[i].buf = startPos;
        buffs[i].len = buffLen;
        startPos += buffLen;
    }
    buffs[ DZ_TEST_IOV_COUNT - 1 ].len += len % DZ_TEST_IOV_COUNT;
    return DzIovSendTo( fd, buffs, DZ_TEST_IOV_COUNT, 0, to, toLen );
}

typedef int ( *FuncRead )( int fd, void* buff, int len, sockaddr* from, int* fromLen );
typedef int ( *FuncWrite )( int fd, const void* buff, int len, const sockaddr* to, int toLen );

static int gNextReadFuncIdx = 0;
static int gNextWriteFuncIdx = 0;

static FuncRead gReadFuncArr[] = {
    RecvFunc,
    RecvExFunc,
    RecvFromFunc,
    RecvFromExFunc
};
static FuncWrite gWriteFuncArr[] = {
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

void CltMain( DzEntry cltRoutine, int count )
{
    for( int i = 0; i < count; i++ ){
        CotStart( cltRoutine, (intptr_t)i );
    }
    CotStop();
}

int TcpReadOneStream( FuncRead readFunc, int lisFd )
{
    int buffLen = DZ_TEST_READ_BUFF_LEN;
    if( buffLen > gMaxBuffLen ){
        buffLen = gMaxBuffLen;
    }
    char* buff = (char*)DzMalloc( buffLen );

    TestStream ts;
    ts.idx = 0;
    ts.len = 0;
    char* cmp = NULL;
    int tmp;
    int recvTimeCount = 0;
    int recvLen = 0;
    do{
        __DzTce5( "recv start %d", recvTimeCount++ );
        tmp = readFunc( lisFd, buff, buffLen, NULL, NULL );
        if( tmp < 0 ){
            DzFree( buff );
            throw (int)__LINE__;
        }
        recvLen += tmp;
        __DzTce5( "recv : %d,\t total : %d", tmp, recvLen );
        if( !cmp ){
            if( recvLen < (int)sizeof( TestStream ) ){
                int tmp1;
                do{
                    tmp1 = readFunc( lisFd, buff + recvLen, buffLen - recvLen, NULL, NULL );
                    if( tmp1 <= 0 ){
                        DzFree( buff );
                        throw (int)__LINE__;
                    }
                    recvLen += tmp1;
                }while( recvLen < (int)sizeof( TestStream ) );
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
        DZ_EXPECT_EQ( 0, ret );
        cmp += tmp;
    }while( tmp > 0 && recvLen < ts.len );
    DZ_EXPECT_EQ( ts.len, recvLen ) << "recv time : " << recvTimeCount;
    DzFree( buff );
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
        __DzTce5( "send start %d", sendTimeCount++ );
        tmp = writeFunc( fd, buff + sendLen, emptyLen, NULL, 0 );
        if( tmp < 0 ){
            throw (int)__LINE__;
        }
        sendLen += tmp;
        __DzTce5( "send : %d,\t total : %d", tmp, sendLen );
        if( gSendInterval ){
            DzSleep( gSendInterval );
        }
    }while( tmp > 0 && sendLen < buffLen );
    DZ_EXPECT_EQ( buffLen, sendLen ) << "send time : " << sendTimeCount;
}

void TcpSvrMain( DzEntry svrEntry, int count )
{
    DzStartCot( WaitHello );

    int lisFd = -1;
    try{
        BOOL opt = TRUE;
        lisFd = DzSocket( gAddr->sa_family, SOCK_STREAM, 0 );
        DzSetSockOpt( lisFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt) );
        if( lisFd == -1 ){
            throw (int)__LINE__;
        }
        int ret = DzBind( lisFd, gAddr, gAddrLen );
        if( ret != 0 ){
            throw (int)__LINE__;
        }

        ret = DzListen( lisFd, SOMAXCONN );
        if( ret != 0 ){
            throw (int)__LINE__;
        }

        int connCount = count;
        sockaddr acptAddr;
        while( connCount ){
            int acptAddrLen = sizeof( sockaddr );
            int fd = DzAccept( lisFd, &acptAddr, &acptAddrLen );
            if( fd == -1 ){
                throw (int)__LINE__;
            }
            CotStart( svrEntry, (intptr_t)fd );
            connCount--;
        }
    }catch( int line ){
        DZ_ADD_FAILURE() << "socket error, line : " << line;
    }
    if( lisFd != -1 ){
        DzClose( lisFd );
    }
    CotStop();
}

int UdpReadOneStream( FuncRead readFunc, int fd, sockaddr* from = NULL, int* fromLen = NULL )
{
    int buffLen = gMaxBuffLen;
    char* buff = (char*)DzMalloc( buffLen );
    int tmp = readFunc( fd, buff, buffLen, from, fromLen );
    if( tmp < 0 ){
        DzFree( buff );
        throw (int)__LINE__;
    }
    TestStream* ts = (TestStream*)buff;
    int ret = ts->idx;
    DZ_EXPECT_EQ( ts->len, tmp );
    DzFree( buff );
    return ret;
}

void UdpWriteOneStream( FuncWrite writeFunc, int fd, int idx, const sockaddr* to = NULL, int toLen = 0 )
{
    TestStream* ts = (TestStream*)gBufArr[ idx ];
    int tmp = writeFunc( fd, gBufArr[ idx ], ts->len, to, toLen );
    if( tmp < 0 ){
        throw (int)__LINE__;
    }
    DZ_EXPECT_EQ( ts->len, tmp );
}

void UdpSvrMain( DzEntry svrEntry, int count )
{
    DzSetPriority( CP_HIGH );
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
            CotStart( svrEntry, (intptr_t)idx );
            connCount--;
        }
    }catch( int line ){
        DZ_ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzClose( fd );
    }
    CotStop();
}

void SocketTestFrame(
    DzEntry     svrTstEntry,
    DzEntry     cltTstEntry,
    int         testCount
    )
{
    int n = 0;
    InitParam();

#ifdef DZ_TEST_SOCKET_SVR
    CotStart( svrTstEntry, (intptr_t)testCount );
    n++;
#endif

#ifdef DZ_TEST_SOCKET_CLT
    DzSleep( gCltDelay );
    CotStart( cltTstEntry, (intptr_t)testCount );
    n++;
#endif

    WaitAllCotEnd( n * ( 1 + testCount ) );
    FreeParam();
}

CotEntry HelpCloseSocket( intptr_t context )
{
    int* fd = (int*)context;
    if( *fd != -1 ){
        DzClose( *fd );
    }
    *fd = -1;
}

CotEntry TcpSvrRecvRoutine( intptr_t context )
{
    int fd = (int)context;
    try{
        TcpReadOneStream( GetReadFunc(), fd );
    }catch( int line ){
        DZ_ADD_FAILURE() << "socket error, line : " << line;
    }
    DzClose( fd );
    CotStop();
}

CotEntry TcpCltSendRoutine( intptr_t context )
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
        DZ_ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzClose( fd );
    }
    CotStop();
}

CotEntry TcpSvrSendRoutine( intptr_t context )
{
    int fd = (int)context;
    try{
        TcpWriteOneStream( GetWriteFunc(), fd, 0 );
    }catch( int line ){
        DZ_ADD_FAILURE() << "socket error, line : " << line;
    }
    DzClose( fd );
    CotStop();
}

CotEntry TcpCltRecvRoutine( intptr_t context )
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
        DZ_EXPECT_EQ( 0, idx );
    }catch( int line ){
        DZ_ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzClose( fd );
    }
    CotStop();
}

CotEntry TcpSvrRsrsRoutine( intptr_t context )
{
    int fd = (int)context;
    try{
        int idx = TcpReadOneStream( GetReadFunc(), fd );
        TcpWriteOneStream( GetWriteFunc(), fd, idx + 1 );
        int idx1 = TcpReadOneStream( GetReadFunc(), fd );
        DZ_EXPECT_EQ( idx + 2, idx1 );
        TcpWriteOneStream( GetWriteFunc(), fd, idx1 + 1 );
    }catch( int line ){
        DZ_ADD_FAILURE() << "socket error, line : " << line;
    }
    DzClose( fd );
    CotStop();
}

CotEntry TcpCltSrsrRoutine( intptr_t context )
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
        DZ_EXPECT_EQ( idx + 1, idx1 );
        TcpWriteOneStream( GetWriteFunc(), fd, idx1 + 1 );
        int idx2 = TcpReadOneStream( GetReadFunc(), fd );
        DZ_EXPECT_EQ( idx1 + 2, idx2 );
    }catch( int line ){
        DZ_ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzClose( fd );
    }
    CotStop();
}

CotEntry TcpSvrRecvOpCloseRoutine( intptr_t context )
{
    FuncRead readFunc = GetReadFunc();
    int fd = (int)context;
    char buff[ 32 ];

    unsigned long long start = DzMilUnixTime();
    int ret = readFunc( fd, buff, 32, NULL, NULL );
    unsigned long long stop = DzMilUnixTime();
    DZ_EXPECT_EQ( 0, ret );
    ret = readFunc( fd, buff, 32, NULL, NULL );
    DZ_EXPECT_EQ( 0, ret );
    DZ_EXPECT_GE( 200, stop - start );
    DzClose( fd );
    CotStop();
}

CotEntry TcpCltRecvOpCloseRoutine( intptr_t context )
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
        DzSleep( 100 );
        if( DzClose( fd ) != 0 ){
            throw (int)__LINE__;
        }
        fd = -1;
    }catch( int line ){
        DZ_ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzClose( fd );
    }
    CotStop();
}

CotEntry TcpSvrSendOpCloseRoutine( intptr_t context )
{
    FuncRead readFunc = GetReadFunc();
    int fd = (int)context;
    int idx;
    try{
        int ret = readFunc( fd, &idx, sizeof( idx ), NULL, NULL );
        DZ_EXPECT_EQ( sizeof( idx ), ret );
        DzSleep( 100 );
        if( DzClose( fd ) != 0 ){
            throw (int)__LINE__;
        }
        fd = -1;
    }catch( int line ){
        DZ_ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzClose( fd );
    }
    CotStop();
}

CotEntry TcpCltSendOpCloseRoutine( intptr_t context )
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
        DZ_EXPECT_EQ( -1, ret );
        DZ_EXPECT_GE( 200, stop - start );
    }catch( int line ){
        DZ_ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzClose( fd );
    }
    CotStop();
}

CotEntry TcpSvrRecvCloseRoutine( intptr_t context )
{
    FuncRead readFunc = GetReadFunc();
    FuncWrite writeFunc = GetWriteFunc();
    int fd = (int)context;
    int idx;
    char buff[ 32 ];
    try{
        int ret = readFunc( fd, &idx, sizeof( idx ), NULL, NULL );
        if( ret < 0 ){
            throw (int)__LINE__;
        }
        int type = idx % 3;
        if( type == 0 ){
            DzShutdown( fd, 0 );
            ret = readFunc( fd, buff, sizeof( buff ), NULL, NULL );
            //Warning: platform difference!
            //after shutdown for READ, subsequent read call return different
            //linux return 0 and windows return -1
            DZ_EXPECT_GE( 0, ret );
            ret = writeFunc( fd, &idx, sizeof( idx ), NULL, 0 );
            DZ_EXPECT_EQ( sizeof( idx ), ret );
            DzClose( fd );
        }else if( type == 1 ){
            DzHandle timer = DzCreateCallbackTimer( 200, FALSE, HelpCloseSocket, (intptr_t)&fd );
            unsigned long long start = DzMilUnixTime();
            ret = readFunc( fd, buff, sizeof( buff ), NULL, NULL );
            unsigned long long stop = DzMilUnixTime();
            DZ_EXPECT_EQ( -1, ret );
            DZ_EXPECT_LE( 190, stop - start );
            DzDelCallbackTimer( timer );
            if( fd != -1 ){
                DzClose( fd );
            }
        }else{
            DzSleep( 1000 );
            DzClose( fd );
        }
    }catch( int line ){
        DZ_ADD_FAILURE() << "socket error, line : " << line;
        if( fd != -1 ){
            DzClose( fd );
        }
    }
    CotStop();
}

CotEntry TcpCltSendCloseRoutine( intptr_t context )
{
    FuncRead readFunc = GetReadFunc();
    FuncWrite writeFunc = GetWriteFunc();
    int idx = (int)context;
    int type = idx % 3;
    int fd = -1;
    const char* buff = "1234567890123456789012345678901234567890";
    try{
        fd = DzSocket( gAddr->sa_family, SOCK_STREAM, 0 );
        if( fd == -1 ){
            throw (int)__LINE__;
        }
        if( DzConnect( fd, gAddr, gAddrLen ) != 0 ){
            throw (int)__LINE__;
        }
        int ret = writeFunc( fd, &idx, sizeof( idx ), NULL, 0 );
        if( ret < 0 ){
            throw (int)__LINE__;
        }
        if( type == 0 ){
            DzShutdown( fd, 1 );
            ret = writeFunc( fd, &idx, sizeof( idx ), NULL, 0 );
            DZ_EXPECT_EQ( -1, ret );
            int idx1;
            ret = readFunc( fd, &idx1, sizeof( idx1 ), NULL, NULL );
            DZ_EXPECT_EQ( sizeof( idx1 ), ret );
            DZ_EXPECT_EQ( idx, idx1 );
            DzClose( fd );
        }else if( type == 1 ){
            DzSleep( 2000 );
            DzClose( fd );
        }else{
            DzHandle timer = DzCreateCallbackTimer( 200, FALSE, HelpCloseSocket, (intptr_t)&fd );
            unsigned long long start = DzMilUnixTime();
            do{
                ret = writeFunc( fd, buff, sizeof( buff ), NULL, 0 );
            }while( ret > 0 );
            unsigned long long stop = DzMilUnixTime();
            DZ_EXPECT_EQ( -1, ret );
            DZ_EXPECT_LE( 190, stop - start );
            DzDelCallbackTimer( timer );
            if( fd != -1 ){
                DzClose( fd );
            }
        }
    }catch( int line ){
        DZ_ADD_FAILURE() << "socket error, line : " << line;
        if( fd != -1 ){
            DzClose( fd );
        }
    }
    CotStop();
}

CotEntry UdpSvrRecvSendRoutine( intptr_t context )
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
        DZ_ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzClose( fd );
    }
    CotStop();
}

CotEntry UdpCltSendRecvRoutine( intptr_t context )
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
        DZ_EXPECT_EQ( idx + 1, ret );
    }catch( int line ){
        DZ_ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzClose( fd );
    }
    if( recvFd != -1 ){
        DzClose( recvFd );
    }
    CotStop();
}

CotEntry UdpSvrRecvSendNoConnRoutine( intptr_t context )
{
    int idx = (int)context;
    int fd = -1;
    try{
        sockaddr_in addr;
        addr.sin_family = gAddr->sa_family;
        addr.sin_addr.s_addr = hton32( 0 );
        addr.sin_port = hton16( gPort - 1 - idx );

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
        DZ_ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzClose( fd );
    }
    CotStop();
}

CotEntry UdpCltSendRecvNoConnRoutine( intptr_t context )
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
        DZ_EXPECT_EQ( idx + 1, ret );
        DZ_EXPECT_EQ( hton16( gPort - 1 - idx ), addr1.sin_port );
        DZ_EXPECT_EQ( ( (sockaddr_in*)gAddr )->sin_addr.s_addr, addr1.sin_addr.s_addr );
    }catch( int line ){
        DZ_ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzClose( fd );
    }
    CotStop();
}

CotEntry TcpSvrRecvMain( intptr_t context )
{
    TcpSvrMain( TcpSvrRecvRoutine, (int)context );
}

CotEntry TcpCltSendMain( intptr_t context )
{
    CltMain( TcpCltSendRoutine, (int)context );
}

CotEntry TcpSvrSendMain( intptr_t context )
{
    TcpSvrMain( TcpSvrSendRoutine, (int)context );
}

CotEntry TcpCltRecvMain( intptr_t context )
{
    CltMain( TcpCltRecvRoutine, (int)context );
}

CotEntry TcpSvrRsrsMain( intptr_t context )
{
    TcpSvrMain( TcpSvrRsrsRoutine, (int)context );
}

CotEntry TcpCltSrsrMain( intptr_t context )
{
    CltMain( TcpCltSrsrRoutine, (int)context );
}

CotEntry TcpSvrRecvOpCloseMain( intptr_t context )
{
    TcpSvrMain( TcpSvrRecvOpCloseRoutine, (int)context );
}

CotEntry TcpCltRecvOpCloseMain( intptr_t context )
{
    CltMain( TcpCltRecvOpCloseRoutine, (int)context );
}

CotEntry TcpSvrSendOpCloseMain( intptr_t context )
{
    TcpSvrMain( TcpSvrSendOpCloseRoutine, (int)context );
}

CotEntry TcpCltSendOpCloseMain( intptr_t context )
{
    CltMain( TcpCltSendOpCloseRoutine, (int)context );
}

CotEntry TcpSvrRecvCloseMain( intptr_t context )
{
    TcpSvrMain( TcpSvrRecvCloseRoutine, (int)context );
}

CotEntry TcpCltSendCloseMain( intptr_t context )
{
    CltMain( TcpCltSendCloseRoutine, (int)context );
}

CotEntry TcpSvrAcceptCloseMain( intptr_t context )
{
    DzStartCot( WaitHello );

    int lisFd = -1;
    try{
        BOOL opt = TRUE;
        lisFd = DzSocket( gAddr->sa_family, SOCK_STREAM, 0 );
        DzSetSockOpt( lisFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt) );
        if( lisFd == -1 ){
            throw (int)__LINE__;
        }
        int ret = DzBind( lisFd, gAddr, gAddrLen );
        if( ret != 0 ){
            throw (int)__LINE__;
        }

        DzSleep( gCltDelay + 1000 );
        ret = DzListen( lisFd, SOMAXCONN );
        if( ret != 0 ){
            throw (int)__LINE__;
        }

        DzHandle timer = DzCreateCallbackTimer( 200, FALSE, HelpCloseSocket, (intptr_t)&lisFd );
        sockaddr acptAddr;
        int acptAddrLen = sizeof( sockaddr );
        int fd = DzAccept( lisFd, &acptAddr, &acptAddrLen );
        DZ_EXPECT_EQ( -1, fd );
        lisFd = -1;
        DzDelCallbackTimer( timer );
        if( lisFd != -1 ){
            DzClose( fd );
        }
    }catch( int line ){
        DZ_ADD_FAILURE() << "socket error, line : " << line;
        if( lisFd != -1 ){
            DzClose( lisFd );
        }
    }
    CotStop();
}

CotEntry TcpCltConnectCloseMain( intptr_t context )
{
    int fd = -1;
    try{
        fd = DzSocket( gAddr->sa_family, SOCK_STREAM, 0 );
        if( fd == -1 ){
            throw (int)__LINE__;
        }
        DzHandle timer = DzCreateCallbackTimer( 200, FALSE, HelpCloseSocket, (intptr_t)&fd );
        int ret = DzConnect( fd, gAddr, gAddrLen );
        DZ_EXPECT_EQ( -1, ret );
        DzDelCallbackTimer( timer );
        if( fd != -1 ){
            DzClose( fd );
        }
    }catch( int line ){
        DZ_ADD_FAILURE() << "socket error, line : " << line;
    }
    CotStop();
}

CotEntry UdpSvrRecvSendMain( intptr_t context )
{
    UdpSvrMain( UdpSvrRecvSendRoutine, (int)context );
}

CotEntry UdpCltSendRecvMain( intptr_t context )
{
    CltMain( UdpCltSendRecvRoutine, (int)context );
}

CotEntry UdpSvrRecvSendNoConnMain( intptr_t context )
{
    UdpSvrMain( UdpSvrRecvSendNoConnRoutine, (int)context );
}

CotEntry UdpCltSendRecvNoConnMain( intptr_t context )
{
    CltMain( UdpCltSendRecvNoConnRoutine, (int)context );
}

CotEntry TcpTestSimpleSend( intptr_t context )
{
    int cotCount = 1;
    InitBuffArray( 2345135, cotCount, 1024, 0 );
    SocketTestFrame( TcpSvrRecvMain, TcpCltSendMain, cotCount );
    DeleteBuffArray();
}

CotEntry TcpTestSimpleRecv( intptr_t context )
{
    int cotCount = 1;
    InitBuffArray( 82534, cotCount, 1024, 0 );
    SocketTestFrame( TcpSvrSendMain, TcpCltRecvMain, cotCount );
    DeleteBuffArray();
}

CotEntry TcpTestLargeBuffer( intptr_t context )
{
    int cotCount = 1;
    InitBuffArray( 9672122, cotCount, 64 * 1024 * 1024, 0 );
    SocketTestFrame( TcpSvrRecvMain, TcpCltSendMain, cotCount );
    DeleteBuffArray();
}

CotEntry TcpTestSendRecvSendRecv( intptr_t context )
{
    int cotCount = 1;
    InitBuffArray( 654752, cotCount + 3, 4* 1024 * 1024, 512 * 1024 );
    SocketTestFrame( TcpSvrRsrsMain, TcpCltSrsrMain, cotCount );
    DeleteBuffArray();
}

CotEntry TcpTestMultiSendRecvSendRecv( intptr_t context )
{
    int cotCount = 100;
    InitBuffArray( 654752, cotCount + 3, 1024, 512 );
    SocketTestFrame( TcpSvrRsrsMain, TcpCltSrsrMain, cotCount );
    DeleteBuffArray();
}

CotEntry TcpTestHugeSendRecvSendRecv( intptr_t context )
{
    int cotCount = 500;
    InitBuffArray( 654752, cotCount + 3, 64, 32 );
    SocketTestFrame( TcpSvrRsrsMain, TcpCltSrsrMain, cotCount );
    DeleteBuffArray();
}

CotEntry TcpTestRecvOpCloseValue( intptr_t context )
{
    SocketTestFrame( TcpSvrRecvOpCloseMain, TcpCltRecvOpCloseMain, 5 );
}

CotEntry TcpTestSendOpCloseValue( intptr_t context )
{
    SocketTestFrame( TcpSvrSendOpCloseMain, TcpCltSendOpCloseMain, 5 );
}

CotEntry TcpTestSendRecvClose( intptr_t context )
{
    SocketTestFrame( TcpSvrRecvCloseMain, TcpCltSendCloseMain, 15 );
}

CotEntry TcpTestConnectAcceptClose( intptr_t context )
{
    SocketTestFrame( TcpSvrAcceptCloseMain, TcpCltConnectCloseMain, 0 );
}

CotEntry UdpTestSendRecv( intptr_t context )
{
    int cotCount = 1;
    InitBuffArray( 2546257, cotCount + 1, 1024, 256 );
    SocketTestFrame( UdpSvrRecvSendMain, UdpCltSendRecvMain, cotCount );
    DeleteBuffArray();
}

CotEntry UdpTestMultiSendRecv( intptr_t context )
{
    int cotCount = 100;
    InitBuffArray( 342553, cotCount + 1, 1024, 256 );
    SocketTestFrame( UdpSvrRecvSendMain, UdpCltSendRecvMain, cotCount );
    DeleteBuffArray();
}

CotEntry UdpTestSendRecvNoConn( intptr_t context )
{
    int cotCount = 1;
    InitBuffArray( 2546257, cotCount + 1, 1024, 256 );
    SocketTestFrame( UdpSvrRecvSendNoConnMain, UdpCltSendRecvNoConnMain, cotCount );
    DeleteBuffArray();
}

CotEntry UdpTestMultiSendRecvNoConn( intptr_t context )
{
    int cotCount = 100;
    InitBuffArray( 342553, cotCount + 1, 1024, 256 );
    SocketTestFrame( UdpSvrRecvSendNoConnMain, UdpCltSendRecvNoConnMain, cotCount );
    DeleteBuffArray();
}

CotEntry UdpTestRecvClose( intptr_t context )
{
    InitParam();
    int count = (int)context;
    int fd = -1;
    try{
        while( count ){
            fd = DzSocket( gAddr->sa_family, SOCK_DGRAM, 0 );
            if( fd == -1 ){
                throw (int)__LINE__;
            }
            if( DzBind( fd, gAddr, gAddrLen ) != 0 ){
                throw (int)__LINE__;
            }

            FuncRead readFunc = GetReadFunc();
            char buff[32];
            DzHandle timer = DzCreateCallbackTimer( 200, FALSE, HelpCloseSocket, (intptr_t)&fd );
            unsigned long long start = DzMilUnixTime();
            int ret = readFunc( fd, buff, sizeof( buff ), NULL, NULL );
            unsigned long long stop = DzMilUnixTime();
            DZ_EXPECT_EQ( -1, ret );
            DZ_EXPECT_LT( 190, stop - start );
            DZ_EXPECT_GT( 300, stop - start );
            DzDelCallbackTimer( timer );
            if( fd != -1 ){
                DzClose( fd );
            }
            count--;
        }
    }catch( int line ){
        DZ_ADD_FAILURE() << "socket error, line : " << line;
    }
    if( fd != -1 ){
        DzClose( fd );
    }
    FreeParam();
}

TEST( TestTcpSocket, SimpleSend )
{
    SetReadFunc( RecvFunc );
    SetWriteFunc( SendFunc );
    TestCot( TcpTestSimpleSend );
}

TEST( TestTcpSocket, SimpleRecv )
{
    SetReadFunc( RecvFunc );
    SetWriteFunc( SendFunc );
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

TEST( TestTcpSocket, RecvOpCloseValue )
{
    SetReadFunc( RecvFunc, TRUE );
    SetWriteFunc( SendFunc, TRUE );
    TestCot( TcpTestRecvOpCloseValue );
}

TEST( TestTcpSocket, SendOpCloseValue )
{
    SetReadFunc( RecvFunc, TRUE );
    SetWriteFunc( SendFunc, TRUE );
    TestCot( TcpTestSendOpCloseValue );
}

TEST( TestTcpSocket, SendRecvClose )
{
    SetReadFunc( RecvFunc, TRUE );
    SetWriteFunc( SendFunc, TRUE );
    TestCot( TcpTestSendRecvClose );
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

TEST( TestUdpSocket, RecvClose )
{
    SetReadFunc( RecvFromFunc, TRUE );
    TestCot( UdpTestRecvClose, 5 );
}
