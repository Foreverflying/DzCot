// CoThread.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <conio.h>
#include <commctrl.h>
#include "../Dzcot/Dzcot.h"
#include "TestCotSome.h"
#include "TestCotSynobj.h"
#include "TestDzcotIo.h"
#include "TestIocpConnect.h"
#include "TestLibpq.h"
#include "TestMap.h"
#include "TestCallbackTimer.h"
#include "Global.h"

#pragma comment( lib, "Dzcot.lib" )

void MainTest( DzRoutine firstEntry, void *context )
{
    DzSockStartup();
    DzInitHost( CP_HIGH );
    DzStartHost( firstEntry, context );
    DzSockCleanup();
}

int _tmain(int argc, _TCHAR* argv[])
{
//     char *ip = "127.0.0.1";
//     char *port = "59999";
//     gIp = inet_addr( ip ) ;
//     gPort = (u_short)_ttoi( port );
//     gPort = htons( gPort );

    int numParam = 0;
    if( argc > 1 ){
        numParam = _ttoi( argv[1] );
    }
    if( argc > 2 ){
        gIp = inet_addr( argv[2] ) ;
        if( gIp == INADDR_NONE ){
            return -2;
        }
    }
    if( argc > 3 ){
        gPort = (u_short)_ttoi( argv[3] );
        if( gPort == 0 ){
            return -3;
        }
        gPort = htons( gPort );
    }
    gIp = gIp ? gIp : htonl( MAKEIPADDRESS( 127, 0, 0, 1 ) );
    gPort = gPort ? gPort : htons( 59999 );

    //MainTest( StartTestCallbackTimer, (void*)10 ) ;
    //MainTest( TestMultiMapHost, (void*)( numParam ? numParam : 200 ) );
    //MainTest( TestMapHost, (void*)0 );
    //MainTest( TestMapServer, (void*)4096 );
    //MainTest( StartTestException, (void*)10 );
    //MainTest( TestException, (void*)0 );
    //MainTest( TestServer, (void*)SOMAXCONN );
    //MainTest( TestServer, (void*)12000 );
    //MainTest( TestMultiClient, (void*)3000 );
    //MainTest( TestClient, (void*)0 );
    //MainTest( TestReadFile, (void*)0 );
    //MainTest( StartTestSynobj, (void*)10 );
    //MainTest( StartTestSwitch, (void*)10000 );
    MainTest( StartTestLibpq, (void*)150 );
    //MainTest( TestLibpq, "user=user password=postgreUser hostaddr=192.168.0.64 port=5432 dbname=template1" );
    //TestFastConnect( (void*)1000 );
    //TestLibpq( "user=user password=postgreUser hostaddr=192.168.0.64 port=5432 dbname=OgameX" );
    printf( "finish, yes!!!!!\r\n" );
    _getch();
	return 0;
}

