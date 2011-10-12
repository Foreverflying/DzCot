
#include "stdafx.h"
//#include "../Dzcot/Inc_Dzcot.h"
#include "TryCot.h"
/*
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
*/
#include "../Dzcot/DzStructs.h"

void __stdcall TestCotTryEntry( intptr_t context )
{
    //TryCotPool();
    printf( "Host size is %d\r\n", (int)sizeof( DzHost ) );

}
