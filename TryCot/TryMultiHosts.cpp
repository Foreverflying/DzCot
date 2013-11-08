
#include "CotTryUtil.h"
#include <vector>

using namespace std;

static int gHostCount = 8;

CotEntry TestRemoteCot1( intptr_t context )
{
    int hostId = DzGetHostId();
    char buff[32];
    sprintf( buff, "                               " );
    buff[ 1 + 2 * hostId ] = 0;
    printf( "%s--%d--print from host %d\r\n", buff, hostId, (int)context );
}

CotEntry TestRemoteCot( intptr_t context )
{
    vector<int>* tmp = (vector<int>*)context;
    delete tmp;

    int hostId = DzGetHostId();
    char buff[32];
    sprintf( buff, "                               " );
    buff[ 1 + 2 * hostId ] = 0;
    printf( "%s--%d--PRINT\r\n", buff, hostId );
    for( int i = 0; i < hostId; i++ ){
        DzStartRemoteCot( i, TestRemoteCot1, hostId );
        DzStartRemoteCot( i, TestRemoteCot1, hostId );
        DzStartRemoteCot( i, TestRemoteCot1, hostId );
    }
    DzSleep( 1000 );
}

CotEntry MultiHostsEntry( intptr_t context )
{
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
    DzDelSynObj( evt );
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

void TryMultiHosts()
{
    StartHosts( MultiHostsEntry, 8 );
}

