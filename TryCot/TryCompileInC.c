
#include "../DzCot/Inc_DzCot.h"
#include "TryCompileInC.h"


CotEntry TryCompileInCEntry( intptr_t context )
{
    DzSleep( 50 );
}

void TryCompileInC()
{
    DzRunHosts( 1, 4096, 65536, 1024 * 1024, CP_LOW, CP_LOW, ST_UM, TryCompileInCEntry, 0, NULL );
}