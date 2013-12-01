/**
 *  @file       CotTryUtil.cpp
 *  @brief      
 *  @author	    Foreverflying <foreverflying@live.cn>
 *  @date       2013/11/08
 *
 */

#include "CotTryUtil.h"

void MainEntry();

CotEntry CleanEntry( intptr_t context )
{
    __DzTceEnableScopePrint( 1 );
    __DzTce1( "//==============================" );
    for( int i = 0; i <= ST_UL; i++ ){
        __DzTce1( "stack %d used size: %d", i, __DzDbgMaxStackUse( i ) );
    }
    __DzTce1( "SynObjLeak:  %d", __DzDbgSynObjLeak() );
    __DzTce1( "FdLeak:      %d", __DzDbgFdLeak() );
    __DzTce1( "HeapLeak:    %d", __DzDbgHeapLeak() );
    __DzTce1( "ParamLeak:   %d", __DzDbgParamNodeLeak() );
    __DzTce1( "\\\\==============================" );
}

void StartHosts( DzEntry firstEntry, int hostCount, intptr_t context )
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
