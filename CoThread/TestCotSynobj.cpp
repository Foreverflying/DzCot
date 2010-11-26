#include "stdafx.h"
#include "TestCotSynobj.h"
#include "../Dzcot/Dzcot.h"

static int currCount = 0;

DzHandle synEvt1 = NULL;
DzHandle synEvt2 = NULL;
DzHandle synSem1 = NULL;
DzHandle synSem2 = NULL;
DzHandle synTimer = NULL;

int ret[ 64 ];
int retCount = 0;

int __stdcall Wait1Evt1_Sem2( void *context )
{
    DzHandle obj[] = { synEvt1, synSem2 };
    DzWaitMultiSynObj( 2, obj, TRUE );
    ret[ retCount++ ] = 1;
    return DS_OK;
}

int __stdcall Wait2Evt1_Evt2( void *context )
{
    DzHandle obj[] = { synEvt1, synEvt2 };
    DzWaitMultiSynObj( 2, obj, TRUE );
    ret[ retCount++ ] = 2;
    return DS_OK;
}

int __stdcall Wait3Evt1_Timer( void *context )
{
    DzHandle obj[] = { synEvt1, synTimer };
    DzWaitMultiSynObj( 2, obj, FALSE );
    ret[ retCount++ ] = 3;
    return DS_OK;
}

int __stdcall Wait4Evt2_Sem1( void *context )
{
    DzHandle obj[] = { synEvt2, synSem1 };
    DzWaitMultiSynObj( 2, obj, FALSE );
    ret[ retCount++ ] = 4;
    return DS_OK;
}

int __stdcall Wait5Sem2_Timer( void *context )
{
    DzHandle obj[] = { synSem2, synTimer };
    DzWaitMultiSynObj( 2, obj, TRUE );
    ret[ retCount++ ] = 5;
    return DS_OK;
}

int __stdcall Wait6Evt1( void *context )
{
    DzWaitSynObj( synEvt1 );
    ret[ retCount++ ] = 6;
    return DS_OK;
}

int __stdcall Wait7Evt1_Sem1_Timer( void *context )
{
    DzHandle obj[] = { synEvt1, synSem1, synTimer };
    DzWaitMultiSynObj( 3, obj, TRUE );
    ret[ retCount++ ] = 7;
    return DS_OK;
}

int __stdcall Wait8Evt2( void *context )
{
    DzWaitSynObj( synEvt2 );
    ret[ retCount++ ] = 8;
    return DS_OK;
}


BOOL TestSynObj1()
{
    retCount = 0;

    DzStartCot( Wait6Evt1 );
    DzStartCot( Wait2Evt1_Evt2 );
    DzStartCot( Wait2Evt1_Evt2 );

    DzSetEvt( synEvt1 );
    DzSleep(1);
    DzResetEvt( synEvt1 );
    DzStartCot( Wait6Evt1 );
    DzSleep( 1 );
    DzSetEvt( synEvt2 );
    DzSetEvt( synEvt1 );
    DzSleep(1);
    DzSetEvt( synEvt2 );

    DzSleep( 1000 );
    int rightRet[] = { 6, 2, 2, 6 };
    if( retCount != 4 ){
        return FALSE;
    }
    for( int i = 0; i < retCount; i++ ){
        if( ret[i] != rightRet[i] ){
            return FALSE;
        }
    }
    return TRUE;
}

BOOL TestSynObj2()
{
    retCount = 0;
    int i = 0;

    DzStartCot( Wait7Evt1_Sem1_Timer );
    DzStartCot( Wait2Evt1_Evt2 );
    DzStartCot( Wait4Evt2_Sem1 );
    DzStartCot( Wait2Evt1_Evt2 );
    DzStartCot( Wait3Evt1_Timer );
    DzStartCot( Wait5Sem2_Timer );
    DzStartCot( Wait5Sem2_Timer );
    DzStartCot( Wait5Sem2_Timer );
    DzStartCot( Wait7Evt1_Sem1_Timer );
    DzStartCot( Wait6Evt1 );

    DzSetEvt( synEvt1 );
    DzSleep0();
    DzSetEvt( synEvt2 );
    DzSleep0();
    DzResetEvt( synEvt1 );
    DzSleep0();
    DzReleaseSem( synSem1, 1 );
    DzSleep0();
    DzSetEvt( synEvt2 );
    DzSleep0();
    DzReleaseSem( synSem2, 2 );

    DzSleep( 500 );
    int rightRet[] = { 2, 3, 6, 4, 2, 7, 5, 5, 5, 7 };
    if( retCount != 4 ){
        return FALSE;
    }
    for( ; i < retCount; i++ ){
        if( ret[i] != rightRet[i] ){
            return FALSE;
        }
    }

    DzSetEvt( synEvt1 );

    DzSleep( 3000 );
    if( retCount != 8 ){
        return FALSE;
    }
    for( ; i < retCount; i++ ){
        if( ret[i] != rightRet[i] ){
            return FALSE;
        }
    }

    DzReleaseSem( synSem2, 1 );
    DzReleaseSem( synSem1, 1 );
    DzSleep( 1 );
    if( retCount != 10 ){
        return FALSE;
    }
    for( ; i < retCount; i++ ){
        if( ret[i] != rightRet[i] ){
            return FALSE;
        }
    }

    return TRUE;
}

BOOL TestSynObj3()
{
    retCount = 0;
    int i = 0;

    DzStartCot( Wait8Evt2 );
    DzStartCot( Wait2Evt1_Evt2, NULL, CP_HIGH );
    DzStartCot( Wait4Evt2_Sem1 );
    DzStartCot( Wait7Evt1_Sem1_Timer );
    DzStartCot( Wait3Evt1_Timer );
    DzStartCot( Wait5Sem2_Timer );
    DzStartCot( Wait5Sem2_Timer );
    DzStartCot( Wait5Sem2_Timer );
    DzStartCot( Wait7Evt1_Sem1_Timer );
    DzStartCot( Wait6Evt1 );

    DzSetEvt( synEvt1 );
    DzSleep0();
    DzSetEvt( synEvt2 );
    DzSleep0();
    DzResetEvt( synEvt1 );
    DzSleep0();
    DzReleaseSem( synSem1, 2 );
    DzSleep0();
    DzSetEvt( synEvt2 );
    DzSleep0();
    DzReleaseSem( synSem2, 2 );

    DzSleep0();
    int rightRet[] = { 2, 3, 6, 8, 4, 5, 5, 7, 5, 7 };
    if( retCount != 5 ){
        return FALSE;
    }
    for( ; i < retCount; i++ ){
        if( ret[i] != rightRet[i] ){
            return FALSE;
        }
    }

    DzSleep( 1500 );
    DzSetEvt( synEvt1 );

    DzSleep( 1500 );
    if( retCount != 8 ){
        return FALSE;
    }
    for( ; i < retCount; i++ ){
        if( ret[i] != rightRet[i] ){
            return FALSE;
        }
    }

    DzReleaseSem( synSem2, 1 );
    DzReleaseSem( synSem1, 1 );
    DzSleep( 1 );
    if( retCount != 10 ){
        return FALSE;
    }
    for( ; i < retCount; i++ ){
        if( ret[i] != rightRet[i] ){
            return FALSE;
        }
    }

    return TRUE;
}

void InitSynObj()
{
    synEvt1 = DzCreateEvt( FALSE, FALSE );
    synEvt2 = DzCreateEvt( TRUE, TRUE );
    synSem1 = DzCreateSem( 0 );
    synSem2 = DzCreateSem( 0 );
    synTimer = DzCreateTimer( 1000, 3 );
}

void ReleaseSynObj()
{
    DzCloseSynObj( synEvt1 );
    DzCloseSynObj( synEvt2 );
    DzCloseSynObj( synSem1 );
    DzCloseSynObj( synSem2 );
    DzCloseSynObj( synTimer );
}

int __stdcall StartTestSynobj( void *context )
{
    BOOL ret;

    InitSynObj();
    ret = TestSynObj1();
    printf( "TestSynObj1 result: %d\r\n", ret );
    ReleaseSynObj();

    InitSynObj();
    ret = TestSynObj2();
    printf( "TestSynObj2 result: %d\r\n", ret );
    ReleaseSynObj();

    InitSynObj();
    ret = TestSynObj3();
    printf( "TestSynObj3 result: %d\r\n", ret );
    ReleaseSynObj();

    return DS_OK;
}

/*
DzHandle synObj1 = NULL;
DzHandle synObj2 = NULL;
DzHandle synObj3 = NULL;

//#define TEST_SYN_EVT

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
        DzWaitSynObj( synObj1, -1 );
        DzHandle obj[] = { synObj1, synObj2, synObj3 };
        //DzWaitMultiSynObj( 2, obj, TRUE, 20000 );
        //DzWaitMultiSynObj( 3, obj, TRUE, 200000 );
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
        DzStartCot( TestSysObj2, (void*)(i*2), CP_HIGH );
    }
    for( int i=0; i<1; i++ ){
        DzStartCot( TestSynObj1, (void*)(i*2+1) );
    }
    return DS_OK;
}

*/
