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

void __stdcall Wait1Evt1_Sem2( void *context )
{
    DzHandle obj[] = { synEvt1, synSem2 };
    int n = DzWaitMultiSynObj( 2, obj, TRUE );
    ret[ retCount++ ] = 1 * 100 + n + 1;
}

void __stdcall Wait2Evt1_Evt2( void *context )
{
    DzHandle obj[] = { synEvt1, synEvt2 };
    int n = DzWaitMultiSynObj( 2, obj, TRUE );
    ret[ retCount++ ] = 2 * 100 + n + 1;
}

void __stdcall Wait3Evt1_Timer( void *context )
{
    DzHandle obj[] = { synEvt1, synTimer };
    int n = DzWaitMultiSynObj( 2, obj, FALSE );
    ret[ retCount++ ] = 3 * 100 + n + 1;
}

void __stdcall Wait4Sem1_Evt2( void *context )
{
    DzHandle obj[] = { synSem1, synEvt2 };
    int n = DzWaitMultiSynObj( 2, obj, FALSE );
    ret[ retCount++ ] = 4 * 100 + n + 1;
}

void __stdcall Wait5Sem2_Timer( void *context )
{
    DzHandle obj[] = { synSem2, synTimer };
    int n = DzWaitMultiSynObj( 2, obj, TRUE );
    ret[ retCount++ ] = 5 * 100 + n + 1;
}

void __stdcall Wait6Evt1( void *context )
{
    int n = DzWaitSynObj( synEvt1 );
    ret[ retCount++ ] = 6 * 100 + n + 1;
}

void __stdcall Wait7Evt1_Sem1_Timer( void *context )
{
    DzHandle obj[] = { synEvt1, synSem1, synTimer };
    int n = DzWaitMultiSynObj( 3, obj, TRUE );
    ret[ retCount++ ] = 7 * 100 + n + 1;
}

void __stdcall Wait8Evt2( void *context )
{
    int n = DzWaitSynObj( synEvt2 );
    ret[ retCount++ ] = 8 * 100 + n + 1;
}

void __stdcall Wait9Sem2TimeOut( void *context )
{
    int n = DzWaitSynObj( synSem2, 2500 );
    ret[ retCount++ ] = 9 * 100 + n + 1;
}

void __stdcall Wait10Sem1Sem2TimeOut( void *context )
{
    DzHandle obj[] = { synSem1, synSem2 };
    int n = DzWaitMultiSynObj( 2, obj, TRUE, 2500 );
    ret[ retCount++ ] = 10 * 100 + n + 1;
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
    int rightRet[] = { 601, 201, 201, 601 };
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
    DzStartCot( Wait4Sem1_Evt2 );
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
    int rightRet[] = { 201, 301, 601, 402, 201, 701, 501, 501, 501, 701 };
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
    DzStartCot( Wait4Sem1_Evt2 );
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
    int rightRet[] = { 201, 301, 601, 801, 401, 501, 501, 701, 501, 701 };
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

BOOL TestSynObj4()
{
    retCount = 0;
    int i = 0;

    DzStartCot( Wait9Sem2TimeOut );
    DzStartCot( Wait2Evt1_Evt2, NULL, CP_HIGH );
    DzStartCot( Wait4Sem1_Evt2 );
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
    DzSleep( 3000 );
    DzReleaseSem( synSem2, 2 );

    DzSleep0();
    int rightRet[] = { 201, 301, 601, 402, 900, 501, 501, 701, 701, 501, 901, 1000, 901, 1001, 1000 };
    if( retCount != 7 ){
        return FALSE;
    }
    for( ; i < retCount; i++ ){
        if( ret[i] != rightRet[i] ){
            return FALSE;
        }
    }

    DzStartCot( Wait9Sem2TimeOut );
    DzStartCot( Wait10Sem1Sem2TimeOut );
    DzSleep( 1500 );
    DzSetEvt( synEvt1 );
    DzReleaseSem( synSem2, 2 );

    DzStartCot( Wait9Sem2TimeOut );
    DzStartCot( Wait10Sem1Sem2TimeOut );

    DzSleep( 1500 );
    if( retCount != 12 ){
        return FALSE;
    }
    for( ; i < retCount; i++ ){
        if( ret[i] != rightRet[i] ){
            return FALSE;
        }
    }

    DzReleaseSem( synSem2, 2 );
    DzReleaseSem( synSem1, 1 );
    DzStartCot( Wait10Sem1Sem2TimeOut );

    DzSleep( 3000 );
    if( retCount != 15 ){
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
    synEvt1 = DzCreateEvt( TRUE, FALSE );
    synEvt2 = DzCreateEvt( FALSE, TRUE );
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

void __stdcall StartTestSynobj( void *context )
{
    BOOL ret;

    DzGrowCotPoolDepth( SS_64K, 2 );

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

    InitSynObj();
    ret = TestSynObj4();
    printf( "TestSynObj4 result: %d\r\n", ret );
    ReleaseSynObj();
}
