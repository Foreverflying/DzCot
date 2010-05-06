#include "StdAfx.h"
#include "../Dzcot/Dzcot.h"

int __stdcall TestException( void *context )
{
    int b = 10;
    int a = ( (int)context ) > 100 ? 0 : 1;

    __try{
        b = 10 /a;
    }__except(EXCEPTION_EXECUTE_HANDLER){
        printf( "exception catched, thread:\t %d\r\n", (int)context );
    }
    return b;
}

int __stdcall StartTestException( void *context )
{
    int count = (int)context;
    for( int i=0; i<count; i++ ){
        DzStartCot( TestException, (void*)(i+95) );
    }
    return 0;
}

//#define _MULT_SWITCH_TEST_
static int countTime = 0;
static int currCount = 0;

int __stdcall TestCotSwitch( void *context )
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
        DzSleep( 0 );
#else
        printf( "thread %d\t scheduled\r\n", id );
        //cout << "thread " << id << endl;
        DzSleep( 1000 );
#endif
    }
    return DS_OK;
}

int __stdcall StartTestSwitch( void *context )
{
    int count = (int)context;
    for( int i=0; i<count; i++ ){
        DzStartCot( TestCotSwitch, (void*)i );
    }
    return 0;
}