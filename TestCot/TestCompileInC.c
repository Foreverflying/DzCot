
#include "../Dzcot/Inc_Dzcot.h"
#include "TestCompileInC.h"


void __stdcall TestCompileInCRoutine( intptr_t context )
{
    DzSleep( 50 );
}

void TestCompileInC()
{
    DzRunHost( CP_LOW, CP_LOW, SS_64K, TestCompileInCRoutine, 0, CP_DEFAULT, CP_DEFAULT );
}