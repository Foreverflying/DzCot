
#include "../Dzcot/Inc_Dzcot.h"
#include "TryCompileInC.h"


void __stdcall TryCompileInCRoutine( intptr_t context )
{
    DzSleep( 50 );
}

void TryCompileInC()
{
    DzRunHosts( 1, CP_LOW, CP_LOW, SS_64K, TryCompileInCRoutine, 0 );
}