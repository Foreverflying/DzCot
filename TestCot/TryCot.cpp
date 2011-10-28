
#include "stdafx.h"
#include "../Dzcot/Inc_Dzcot.h"
#include "TryCot.h"
#include <vector>

using namespace std;

namespace Inner{
#undef DZ_MAX_IOV
#undef DZ_MAX_HOST
#include "../Dzcot/DzStructs.h"
#include "../Dzcot/DzBase.h"
}

int gHostCount = 8;
int gServMask[] = {
    DzMakeServMask( TRUE ),
    DzMakeServMask( TRUE ),
    DzMakeServMask( TRUE ),
    DzMakeServMask( TRUE ),
    DzMakeServMask( TRUE ),
    DzMakeServMask( TRUE ),
    DzMakeServMask( TRUE ),
    DzMakeServMask( TRUE ),
    DzMakeServMask( TRUE ),
    DzMakeServMask( TRUE ),
    DzMakeServMask( TRUE ),
    DzMakeServMask( TRUE ),
    DzMakeServMask( TRUE ),
    DzMakeServMask( TRUE ),
    DzMakeServMask( TRUE ),
    DzMakeServMask( TRUE ),
    DzMakeServMask( TRUE ),
    DzMakeServMask( TRUE ),
    DzMakeServMask( TRUE ),
    DzMakeServMask( TRUE )
};

//*
void __stdcall CotTryEntry( intptr_t context )
{
    int idx = (int)context;
    int a = 0;
    printf( "Idx %d a's address is %p\r\n", idx, &a );
    //DzSleep( 15 );
}

void TryCotPool()
{
    printf( "Size of BOOL is %d\r\n", (int)sizeof( BOOL ) );
    DzSetCotPoolDepth( SS_64K, 5 );
    for( int i = 0; i < 10; i++ ){
        DzStartCotInstant( CotTryEntry );
    }
    for( int i = 0; i < 10; i++ ){
        DzStartCot( CotTryEntry );
    }
}
//*/

void __stdcall TestRemoteCot1( intptr_t context )
{
    int hostId = Inner::GetHost()->hostId;
    char buff[32];
    sprintf( buff, "                               " );
    buff[ 1 + 2 * hostId ] = 0;
    printf( "%s--%d--print from host %d\r\n", buff, hostId, (int)context );
}

void __stdcall TestRemoteCot( intptr_t context )
{
    vector<int>* tmp = (vector<int>*)context;
    delete tmp;

    int hostId = Inner::GetHost()->hostId;
    char buff[32];
    sprintf( buff, "                               " );
    buff[ 1 + 2 * hostId ] = 0;
    printf( "%s--%d--PRINT\r\n", buff, hostId );
    for( int i = 0; i < hostId; i++ ){
        DzStartRemoteCot( i, TestRemoteCot1, hostId );
//         DzStartRemoteCot( i, TestRemoteCot1, hostId );
//         DzStartRemoteCot( i, TestRemoteCot1, hostId );
    }
    DzSleep( 1000 );
}

#pragma comment(lib,"Ws2_32.lib")

void __stdcall GetHostByNameEntry( intptr_t context )
{
    int* ip = (int*)context;
    struct hostent* ret;

    ret = gethostbyname( "www.baidu.com" );
    *ip = *(int*)ret->h_addr_list;
}

void __stdcall TestCotTryEntry( intptr_t context )
{
    /*
    printf( "Fuck this!\r\n" );
    Inner::DzHost host;
    Inner::DzCot cot;
    //*/

    /*
    printf( "Host size is %d\r\n", (int)sizeof( host ) );
    printf( "DzCot size is %d\r\n", (int)sizeof( cot ) );
    TryCotPool();
    //*/

    /*
    DzSetWorkerPoolDepth( 2 );

    int ip;
    DzRunWorker( GetHostByNameEntry, (intptr_t)&ip );
    printf(
        "ip is %d %d %d %d\r\n",
        ( ip >> 24 ) & 0xff,
        ( ip >> 16 ) & 0xff,
        ( ip >> 8 ) & 0xff,
        ip & 0xff
        );

    DzSleep( 5000 );

    DzRunWorker( GetHostByNameEntry, (intptr_t)&ip );
    printf(
        "ip is %d %d %d %d\r\n",
        ( ip >> 24 ) & 0xff,
        ( ip >> 16 ) & 0xff,
        ( ip >> 8 ) & 0xff,
        ip & 0xff
        );

    return;
    //*/

    //*
    for( int i = 1; i < gHostCount; i++ ){
        vector<int>* n = new vector<int>();
        DzStartRemoteCot( i, TestRemoteCot, (intptr_t)n );
    }
    printf( "hahhha, i am sleep now1!\r\n" );
    DzSleep( 1500 );
    printf( "well, awake1\r\n\r\n" );
    //*/

    //*
    printf( "hahhha, i am sleep now2!\r\n" );
    DzHandle evt = DzCreateCdEvt( gHostCount - 1 );
    for( int i = 1; i < gHostCount; i++ ){
        vector<int>* n = new vector<int>();
        DzEvtStartRemoteCot( evt, i, TestRemoteCot, (intptr_t)n );
    }
    DzWaitSynObj( evt );
    printf( "well, awake2\r\n" );
    DzCloseSynObj( evt );
    //*/

    //*
    printf( "hahhha, i am sleep now3!\r\n" );
    for( int i = 1; i < gHostCount; i++ ){
        vector<int>* n = new vector<int>();
        DzRunRemoteCot( i, TestRemoteCot, (intptr_t)n );
    }
    printf( "well, awake3\r\n\r\n" );
    //*/
}
