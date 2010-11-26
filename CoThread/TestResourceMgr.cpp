#include "stdafx.h"
#include "TestCotSynobj.h"
#include "../Dzcot/Dzcot.h"


int __stdcall StartTestResourceMgr( void *context )
{
    DzHandle *arr = new DzHandle[ (int)20000 ];

    for( int i=0; i<(int)10000; i++ ){
        arr[i] = DzCreateEvt( FALSE, FALSE );
    }
    for( int i=0; i<(int)10000; i++ ){
        DzCloseSynObj( arr[i] );
    }
    for( int i=0; i<(int)20000; i++ ){
        arr[i] = DzCreateEvt( FALSE, FALSE );
    }
    for( int i=0; i<(int)20000; i++ ){
        DzCloseSynObj( arr[i] );
    }
    delete []arr;
    return DS_OK;
}
