
#include "stdafx.h"
#include "../Dzcot/Inc_Dzcot.h"
#include "TryCot.h"

void __stdcall DzTryEntry( intptr_t context )
{
    int idx = (int)context;
    int a = 0;
    printf( "Idx %d a's address is %x\r\n", idx, &a );
    //DzSleep( 15 );
}

void __stdcall DzcotEntry( intptr_t context )
{
    printf( "wo qu !!\r\n" );
    int a = DzSetCotPoolDepth( SS_64K, 5 );
    for( int i = 0; i < 10; i++ ){
        DzStartCotInstant( DzTryEntry );
    }
    for( int i = 0; i < 10; i++ ){
        DzStartCot( DzTryEntry );
    }

}
