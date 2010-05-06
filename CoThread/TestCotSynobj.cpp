#include "StdAfx.h"
#include "TestCotSynobj.h"
#include "../Dzcot/Dzcot.h"

static int currCount = 0;
DzHandle synObj1 = NULL;
DzHandle synObj2 = NULL;
DzHandle synObj3 = NULL;

#define TEST_SYN_OBJ
#define TEST_SYN_EVT

int __stdcall TestSynObj1( void *context );
int __stdcall TestSysObj2( void *context );
int __stdcall TestSysObj3( void *context );

int __stdcall TestSynObj1( void *context )
{
    while(1){
        printf( "%d\tTestSynObj1A: %d\r\n", currCount, (int)context );
        currCount++;
        //Sleep( 500 );
        DzSleep( 1000 );
        printf( "%d\tTestSynObj1B: %d\r\n", currCount, (int)context );
        if( currCount - 15 == 0 ){
#ifdef TEST_SYN_EVT
            DzSetEvt( synObj1 );
            DzResetEvt( synObj1 );
#else
            DzReleaseSem( synObj1, 3 );
#endif
            //StartCot( TestSysObj3, (void*)(10000 + (int)context), CP_INSTANT );
        }
        if( currCount - 25 == 0 ){
            DzReleaseSem( synObj2, 3 );
            //ResetDzEvt( synObj1 );
            //StartCot( TestSysObj2, (void*)(10000 + (int)context), CP_INSTANT );
        }
        if( currCount -35 == 0 ){
            DzSetEvt( synObj1 );
            DzResetEvt( synObj1 );
        }
        if( currCount - 40 == 0 ){
            DzReleaseSem( synObj2, 3 );
            DzSetEvt( synObj1 );
        }
        if( currCount - 50 == 0 ){
            DzReleaseSem( synObj2, 10 );
            //SetDzEvt( synObj1 );
        }
    }
    return DS_OK;
}

int __stdcall TestSysObj2( void *context )
{
    while(1){
        //WaitSynObj( synObj1, -1 );
        DzHandle obj[] = { synObj1, synObj2, synObj3 };
        //DzWaitMultiSynObj( 2, obj, TRUE, 20000 );
        DzWaitMultiSynObj( 3, obj, TRUE, 200000 );
        printf( "%d\tTestSysObj2: %d\r\n", currCount, (int)context );
        //Sleep( 500 );
        //DzSleep( 0 );
    }
    return DS_OK;
}

int __stdcall TestSysObj3( void *context )
{
    printf( "%d\thello, i am a fool: %d\r\n", currCount, (int)context );
    return DS_OK;
}

int __stdcall StartTestSynobj( void *context )
{
    int count = (int)context;
#ifdef TEST_SYN_EVT
    synObj1 = DzCreateEvt( FALSE, FALSE );
    synObj2 = DzCreateSem( 10 );
    synObj3 = DzCreateTimer( 50000, FALSE );
#else
    synObj1 = DzCreateSem( 5 );
#endif
    for( int i=0; i<count; i++ ){
        DzStartCot( TestSysObj2, (void*)(i*2), CP_INSTANT );
    }
    for( int i=0; i<1; i++ ){
        DzStartCot( TestSynObj1, (void*)(i*2+1) );
    }
    return DS_OK;
}
