#include "stdafx.h"
#include "../Dzcot/Dzcot.h"

void GenerateException()
{
    int *p = 0;
    *p = 0;
}

void __stdcall TestPrintf( void *context )
{
    int ret = 1;
    DzSleep(1000);
    printf( "wo qu, I can print in COT!! number: %d", ret );
    //printf( "%d\n", ret );
    DzStartCot( TestPrintf, 0 );
}

void __stdcall TestMiniDump( void *context )
{
    GenerateException();
}

void __stdcall TestSehException( void *context )
{
    int b = 10;

#ifdef _WIN32
    int a = ( (int)context ) > 100 ? 0 : 1;
    __try{
        b = 10 /a;
    }__except(EXCEPTION_EXECUTE_HANDLER){
        printf( "SEH exception catched, thread:\t %d\r\n", (int)context );
    }
#endif
}

void __stdcall TestException( void *context )
{
    int b = 10;
    int a = ( (int)context ) > 100 ? 0 : 1;

    try{
        b = 10 /a;
    }catch( int e ){
        printf( "c++ exception catched, thread:\t %d\r\n", (int)e );
    }
}

void __stdcall StartTestException( void *context )
{
    int count = (int)context;
    for( int i=0; i<count; i++ ){
        DzStartCot( TestException, (void*)(i+95) );
    }
}

#define _MULT_SWITCH_TEST_
static int countTime = 0;
static int currCount = 0;

void __stdcall TestCotSwitch( void *context )
{
    int id = (int)context;
    int a[400];
    a[0] = 1;
    while(1){
#ifdef _MULT_SWITCH_TEST_
        currCount++;
        if( currCount == 10000000 ){
            currCount = 0;
            countTime++;
            printf( "%d\t times called\r\n", countTime );
        }
        DzSleep0();
#else
        printf( "thread %d\t scheduled\r\n", id );
        //cout << "thread " << id << endl;
        DzSleep( 1000 );
#endif
    }
}

void __stdcall StartTestSwitch( void *context )
{
    int count = (int)context;
    for( int i=0; i<count; i++ ){
        DzStartCot( TestCotSwitch, (void*)i );
    }
}

void __stdcall StartTestStackAlloc( void *context )
{
    if( !context ){
        return;
    }
    int mem[ 512 ];
    for( int i = 0; i < 512; i++ ){
        mem[ i ] = 0;
    }
    StartTestStackAlloc( (void*)((size_t)context-1) );
}
