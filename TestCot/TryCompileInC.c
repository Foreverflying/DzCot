
#include "../DzCot/Inc_DzCot.h"
#include "TryCompileInC.h"


CotEntry TryCompileInCRoutine( intptr_t context )
{
    DzSleep( 50 );
}

void TryCompileInC()
{
    DzRunHosts( 1, CP_LOW, CP_LOW, SS_64K, TryCompileInCRoutine, 0, NULL );
}