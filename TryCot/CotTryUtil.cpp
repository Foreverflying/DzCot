
#include "CotTryUtil.h"

void MainEntry();

CotEntry CleanEntry( intptr_t context )
{
    __DzTceEnableScopePrint( 1 );
    for( int i = 0; i < 4; i++ ){
        __DzTce1( "stack %d used size: %d", i, __DzDbgMaxStackUse( i ) );
    }
    __DzTce1( "Hosts exit" );
}

void CotTryRunHosts( DzEntry firstEntry, int hostCount, intptr_t context )
{
    DzRunHosts(
        hostCount,
        4000, 64000, 128000,
        CP_LOW, CP_LOW, ST_UM,
        firstEntry, context, CleanEntry
        );
}

int main(int argc, _TCHAR* argv[])
{
    MainEntry();
    getchar();
    return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
    MainEntry();
    getchar();
    return 0;
}
