
#include "CotTestUtil.h"

DzHandle synEvt1 = NULL;
DzHandle synEvt2 = NULL;
DzHandle synSem1 = NULL;
DzHandle synSem2 = NULL;
DzHandle synTimer1 = NULL;
DzHandle synTimer2 = NULL;

int ret[ 64 ];
int retCount = 0;

void InitSynObj()
{
    synEvt1 = DzCreateManualEvt( FALSE );
    synEvt2 = DzCreateAutoEvt( TRUE );
    synSem1 = DzCreateSem( 0 );
    synSem2 = DzCreateSem( 0 );
    synTimer1 = DzCreateTimer( 1000, TRUE );
    synTimer2 = DzCreateTimer( 3000, FALSE );
}

void ReleaseSynObj()
{
    DzDelSynObj( synEvt1 );
    DzDelSynObj( synEvt2 );
    DzDelSynObj( synSem1 );
    DzDelSynObj( synSem2 );
    DzDelSynObj( synTimer1 );
    DzDelSynObj( synTimer2 );
}

void __stdcall Wait1Evt1_Sem2( intptr_t context )
{
    DzHandle obj[] = { synEvt1, synSem2 };
    int n = DzWaitMultiSynObj( 2, obj, TRUE );
    ret[ retCount++ ] = 1 * 100 + n + 1;
}

void __stdcall Wait2Evt1_Evt2( intptr_t context )
{
    DzHandle obj[] = { synEvt1, synEvt2 };
    int n = DzWaitMultiSynObj( 2, obj, TRUE );
    ret[ retCount++ ] = 2 * 100 + n + 1;
}

void __stdcall Wait3Evt1_Timer1( intptr_t context )
{
    DzHandle obj[] = { synEvt1, synTimer1 };
    int n = DzWaitMultiSynObj( 2, obj, FALSE );
    ret[ retCount++ ] = 3 * 100 + n + 1;
}

void __stdcall Wait4Sem1_Evt2( intptr_t context )
{
    DzHandle obj[] = { synSem1, synEvt2 };
    int n = DzWaitMultiSynObj( 2, obj, FALSE );
    ret[ retCount++ ] = 4 * 100 + n + 1;
}

void __stdcall Wait5Sem2_Timer1( intptr_t context )
{
    DzHandle obj[] = { synSem2, synTimer1 };
    int n = DzWaitMultiSynObj( 2, obj, TRUE );
    ret[ retCount++ ] = 5 * 100 + n + 1;
}

void __stdcall Wait6Evt1( intptr_t context )
{
    int n = DzWaitSynObj( synEvt1 );
    ret[ retCount++ ] = 6 * 100 + n + 1;
}

void __stdcall Wait7Evt1_Sem1_Timer1( intptr_t context )
{
    DzHandle obj[] = { synEvt1, synSem1, synTimer1 };
    int n = DzWaitMultiSynObj( 3, obj, TRUE );
    ret[ retCount++ ] = 7 * 100 + n + 1;
}

void __stdcall Wait8Evt2( intptr_t context )
{
    int n = DzWaitSynObj( synEvt2 );
    ret[ retCount++ ] = 8 * 100 + n + 1;
}

void __stdcall Wait9Sem2TimeOut( intptr_t context )
{
    int n = DzWaitSynObj( synSem2, 2500 );
    ret[ retCount++ ] = 9 * 100 + n + 1;
}

void __stdcall Wait10Sem1Sem2TimeOut( intptr_t context )
{
    DzHandle obj[] = { synSem1, synSem2 };
    int n = DzWaitMultiSynObj( 2, obj, TRUE, 2500 );
    ret[ retCount++ ] = 10 * 100 + n + 1;
}

void __stdcall Wait11Sem2_Timer2( intptr_t context )
{
    DzHandle obj[] = { synSem2, synTimer2 };
    int n = DzWaitMultiSynObj( 2, obj, TRUE );
    ret[ retCount++ ] = 11 * 100 + n + 1;
}

void __stdcall Wait12Evt1_Sem1_Timer2( intptr_t context )
{
    DzHandle obj[] = { synEvt1, synSem1, synTimer2 };
    int n = DzWaitMultiSynObj( 3, obj, TRUE );
    ret[ retCount++ ] = 12 * 100 + n + 1;
}

void __stdcall TestAllSynObj1( intptr_t context )
{
    InitSynObj();

    int i = 0;
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

    EXPECT_EQ( 4, retCount );
    for( ; i < retCount; i++ ){
        EXPECT_EQ( ret[i], rightRet[i] );
    }

    ReleaseSynObj();
}

void __stdcall TestAllSynObj2( intptr_t context )
{
    InitSynObj();

    int i = 0;
    retCount = 0;

    DzStartCot( Wait7Evt1_Sem1_Timer1, 0, CP_DEFAULT, SS_1K );
    DzStartCot( Wait2Evt1_Evt2, 0, CP_DEFAULT, SS_4K );
    DzStartCot( Wait4Sem1_Evt2, 0, CP_DEFAULT, SS_16K );
    DzStartCot( Wait2Evt1_Evt2, 0, CP_DEFAULT, SS_256K );
    DzStartCot( Wait3Evt1_Timer1, 0, CP_DEFAULT, SS_1M );
    DzStartCot( Wait5Sem2_Timer1, 0, CP_DEFAULT, SS_4M );
    DzStartCot( Wait5Sem2_Timer1, 0, CP_DEFAULT, SS_16M );
    DzStartCot( Wait5Sem2_Timer1, 0, CP_DEFAULT, SS_64K );
    DzStartCot( Wait7Evt1_Sem1_Timer1 );
    DzStartCot( Wait6Evt1 );

    DzSetEvt( synEvt1 );
    DzSleep( 0 );
    DzSetEvt( synEvt2 );
    DzSleep( 0 );
    DzResetEvt( synEvt1 );
    DzSleep( 0 );
    DzReleaseSem( synSem1, 1 );
    DzSleep( 0 );
    DzSetEvt( synEvt2 );
    DzSleep( 0 );
    DzReleaseSem( synSem2, 2 );

    DzSleep( 500 );
    int rightRet[] = { 201, 301, 601, 402, 201, 701, 501, 501, 501, 701 };

    EXPECT_EQ( 4, retCount );
    for( ; i < retCount; i++ ){
        EXPECT_EQ( ret[i], rightRet[i] );
    }

    DzSetEvt( synEvt1 );

    DzSleep( 3000 );

    EXPECT_EQ( 8, retCount );
    for( ; i < retCount; i++ ){
        EXPECT_EQ( ret[i], rightRet[i] );
    }

    DzReleaseSem( synSem2, 1 );
    DzReleaseSem( synSem1, 1 );
    DzSleep( 1000 );

    EXPECT_EQ( 10, retCount );
    for( ; i < retCount; i++ ){
        EXPECT_EQ( ret[i], rightRet[i] );
    }

    ReleaseSynObj();
}

void __stdcall TestAllSynObj3( intptr_t context )
{
    InitSynObj();

    int i = 0;
    retCount = 0;

    DzStartCot( Wait9Sem2TimeOut );
    DzStartCot( Wait2Evt1_Evt2, NULL, CP_HIGH );
    DzStartCot( Wait4Sem1_Evt2 );
    DzStartCot( Wait12Evt1_Sem1_Timer2 );
    DzStartCot( Wait3Evt1_Timer1 );
    DzStartCot( Wait11Sem2_Timer2 );
    DzStartCot( Wait11Sem2_Timer2 );
    DzStartCot( Wait11Sem2_Timer2 );
    DzStartCot( Wait12Evt1_Sem1_Timer2 );
    DzStartCot( Wait6Evt1 );

    DzSetEvt( synEvt1 );
    DzSleep( 0 );
    DzSetEvt( synEvt2 );
    DzSleep( 0 );
    DzResetEvt( synEvt1 );
    DzSleep( 0 );
    DzReleaseSem( synSem1, 2 );
    DzSleep( 0 );
    DzSetEvt( synEvt2 );
    DzSleep( 3000 );
    DzReleaseSem( synSem2, 2 );

    DzSleep( 0 );
    int rightRet[] = { 201, 301, 601, 402, 900, 1101, 1101, 1201, 1201, 1101, 901, 1000, 901, 1001, 1000 };

    EXPECT_EQ( 7, retCount );
    for( ; i < retCount; i++ ){
        EXPECT_EQ( ret[i], rightRet[i] );
    }

    DzStartCot( Wait9Sem2TimeOut );
    DzStartCot( Wait10Sem1Sem2TimeOut );
    DzSleep( 1500 );
    DzSetEvt( synEvt1 );
    DzReleaseSem( synSem2, 2 );

    DzStartCot( Wait9Sem2TimeOut );
    DzStartCot( Wait10Sem1Sem2TimeOut );

    DzSleep( 1500 );

    EXPECT_EQ( 12, retCount );
    for( ; i < retCount; i++ ){
        EXPECT_EQ( ret[i], rightRet[i] );
    }

    DzReleaseSem( synSem2, 2 );
    DzReleaseSem( synSem1, 1 );
    DzStartCot( Wait10Sem1Sem2TimeOut );

    DzSleep( 3000 );

    EXPECT_EQ( 15, retCount );
    for( ; i < retCount; i++ ){
        EXPECT_EQ( ret[i], rightRet[i] );
    }

    ReleaseSynObj();
}

void __stdcall TestAllSynObj4( intptr_t context )
{
    InitSynObj();

    int i = 0;
    retCount = 0;

    DzStartCot( Wait9Sem2TimeOut );
    DzStartCot( Wait2Evt1_Evt2, NULL, CP_HIGH );
    DzStartCot( Wait4Sem1_Evt2 );
    DzStartCot( Wait12Evt1_Sem1_Timer2 );
    DzStartCot( Wait3Evt1_Timer1 );
    DzStartCot( Wait11Sem2_Timer2 );
    DzStartCot( Wait11Sem2_Timer2 );
    DzStartCot( Wait11Sem2_Timer2 );
    DzStartCot( Wait12Evt1_Sem1_Timer2 );
    DzStartCot( Wait6Evt1 );

    DzSetEvt( synEvt1 );
    DzSleep( 0 );
    DzSetEvt( synEvt2 );
    DzSleep( 0 );
    DzResetEvt( synEvt1 );
    DzSleep( 0 );
    DzReleaseSem( synSem1, 2 );
    DzSleep( 0 );
    DzSetEvt( synEvt2 );
    DzSleep( 3000 );
    DzReleaseSem( synSem2, 2 );

    DzSleep( 0 );
    int rightRet[] = { 201, 301, 601, 402, 900, 1101, 1101, 1201, 1201, 1101, 901, 1000, 901, 1001, 1000 };

    EXPECT_EQ( 7, retCount );
    for( ; i < retCount; i++ ){
        EXPECT_EQ( ret[i], rightRet[i] );
    }

    DzStartCot( Wait9Sem2TimeOut );
    DzStartCot( Wait10Sem1Sem2TimeOut );
    DzSleep( 1500 );
    DzSetEvt( synEvt1 );
    DzReleaseSem( synSem2, 2 );

    DzStartCot( Wait9Sem2TimeOut );
    DzStartCot( Wait10Sem1Sem2TimeOut );

    DzSleep( 1500 );

    EXPECT_EQ( 12, retCount );
    for( ; i < retCount; i++ ){
        EXPECT_EQ( ret[i], rightRet[i] );
    }

    DzReleaseSem( synSem2, 2 );
    DzReleaseSem( synSem1, 1 );
    DzStartCot( Wait10Sem1Sem2TimeOut );

    DzSleep( 3000 );

    EXPECT_EQ( 15, retCount );
    for( ; i < retCount; i++ ){
        EXPECT_EQ( ret[i], rightRet[i] );
    }

    ReleaseSynObj();
}

TEST( TestSynObj, AllSynObj1 )
{
    TestCot( TestAllSynObj1 );
}

TEST( TestSynObj, AllSynObj2 )
{
    TestCot( TestAllSynObj2 );
}

TEST( TestSynObj, AllSynObj3 )
{
    TestCot( TestAllSynObj3 );
}

TEST( TestSynObj, AllSynObj4 )
{
    TestCot( TestAllSynObj4 );
}
