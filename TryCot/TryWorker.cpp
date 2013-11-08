
#include "CotTryUtil.h"

#ifdef __linux__
inline void Sleep( int milsec ){}
#endif

CotEntry SimplePrint( intptr_t context )
{
    int delay = (int)context;

    Sleep( delay );
    printf( "Hello,\r\n" );
    Sleep( delay );
    printf( "World!\r\n" );
}

CotEntry RunSimplePrintCot( intptr_t context )
{
    DzRunWorker( SimplePrint, context );
}

CotEntry TrySimplePrintEntry( intptr_t context )
{
    for( int i = 0; i < 5; i++ ){
        DzStartCot( RunSimplePrintCot, 1000 + i * 200 );
    }
    DzSleep( 100 );
    printf( "main thread fall sleep\r\n" );
    Sleep( 5000 );
    printf( "main thread wake up\r\n" );
    return;

}

void TryWorker()
{
    StartHosts( TrySimplePrintEntry, 1, 2000 );
}