
#include "../Dzcot/Inc_Dzcot.h"
#include "TryCompileInC.h"


void __stdcall TryCompileInCRoutine( intptr_t context )
{
    DzSleep( 50 );
}

void TryCompileInC()
{
    DzRunHost( CP_LOW, CP_LOW, SS_64K, TryCompileInCRoutine, 0, CP_DEFAULT, CP_DEFAULT );
}