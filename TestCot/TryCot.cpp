
#include "stdafx.h"
#include "../Dzcot/Inc_Dzcot.h"
#include "TryCot.h"

namespace Inner{
#undef DZ_MAX_IOV
#undef DZ_MAX_HOST
#include "../Dzcot/DzStructs.h"
#include "../Dzcot/DzBaseOs.h"
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
    int hostId = Inner::GetHost()->hostId;
    char buff[32];
    sprintf( buff, "                               " );
    buff[ 1 + 2 * hostId ] = 0;
    printf( "%s--%d--PRINT\r\n", buff, hostId );
    for( int i = 0; i < context; i++ ){
        DzStartRemoteCot( i, TestRemoteCot1, hostId );
    }
}

void __stdcall TestCotTryEntry( intptr_t context )
{
    //printf( "Fuck this!\r\n" );
    //TryCotPool();

    //*
    printf( "Host size is %d\r\n", (int)sizeof( Inner::DzHost ) );
    for( int i = 1; i < gHostCount; i++ ){
        DzStartRemoteCot( i, TestRemoteCot, i );
    }
    printf( "hahhha, i am sleep now1!\r\n" );
    DzSleep( 1500 );
    printf( "well, awake1\r\n\r\n" );
    //*/

    //*
    printf( "hahhha, i am sleep now2!\r\n" );
    DzHandle evt = DzCreateCdEvt( gHostCount - 1 );
    for( int i = 1; i < gHostCount; i++ ){
        DzEvtStartRemoteCot( evt, i, TestRemoteCot, i );
    }
    DzWaitSynObj( evt );
    printf( "well, awake2\r\n" );
    DzCloseSynObj( evt );
    //*/

    //*
    printf( "hahhha, i am sleep now3!\r\n" );
    for( int i = 1; i < gHostCount; i++ ){
        DzRunRemoteCot( i, TestRemoteCot, i );
    }
    printf( "well, awake3\r\n\r\n" );
    //*/
}
