
#include "CotTryUtil.h"

CotEntry CotTryEntry( intptr_t context )
{
    int idx = (int)context;
    int a = 0;
    printf( "Idx %d a's address is %p\r\n", idx, &a );
    //DzSleep( 15 );
}

CotEntry TryCotPoolEntry( intptr_t context )
{
    printf( "Size of BOOL is %d\r\n", (int)sizeof( BOOL ) );
    DzSetCotPoolDepth( ST_UM, 5 );
    for( int i = 0; i < 10; i++ ){
        DzStartCotInstant( CotTryEntry );
    }
    for( int i = 0; i < 10; i++ ){
        DzStartCot( CotTryEntry );
    }
}

void TryCotPool()
{
    CotTryRunHosts( TryCotPoolEntry );
}