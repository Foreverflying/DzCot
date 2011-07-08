#include "stdafx.h"

//#define  TEST_LIBPQ
#ifdef TEST_LIBPQ

#include "../Dzcot/Dzcot.h"
#include "TestLibpq.h"
#include <libpq-fe.h>
#include <iostream>
using namespace std;


#pragma comment( lib, "libpqdll.lib" )

//#define printf(...)

static void
exit_nicely(PGconn *conn)
{
    PQfinish(conn);
    exit(1);
}

void __stdcall StartTestLibpq( void *context )
{
    int count = (int)context;
    for( int i=0; i<count; i++ ){
        DzStartCot( TestLibpq, (void*)i );
    }
}

void __stdcall TestLibpq( void *context )
{
    const char *conninfo;
    PGconn     *conn;
    PGresult   *res;
    int         nFields;
    int         i;
    int         j;


    cout << "cot start: \t" << (int)context << "\r\n";

    conninfo = "user=BombBabyMgr password=asdf hostaddr=192.168.1.109 port=5432 dbname=template1";
    conninfo = "user=BombBabyMgr password=asdf hostaddr=192.168.1.109 port=5432 dbname=BombBaby";

    conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK)
    {
        printf( "Connection to database failed: %s", PQerrorMessage(conn) );
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
        exit_nicely(conn);
    }

    res = PQexec(conn, "BEGIN");
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        printf( "BEGIN command failed: %s\r\n", PQerrorMessage(conn) );
        fprintf(stderr, "BEGIN command failed: %s", PQerrorMessage(conn));
        PQclear(res);
        exit_nicely(conn);
    }

     PQclear(res);

    res = PQexec(conn, "DECLARE myportal CURSOR FOR select * from pg_database");
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        printf( "DECLARE CURSOR failed: %s\r\n", PQerrorMessage(conn) );
        fprintf(stderr, "DECLARE CURSOR failed: %s", PQerrorMessage(conn));
        PQclear(res);
        exit_nicely(conn);
    }
    PQclear(res);

    res = PQexec(conn, "FETCH ALL in myportal");
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        printf("FETCH ALL failed: %s\r\n", PQerrorMessage(conn));
        fprintf(stderr, "FETCH ALL failed: %s", PQerrorMessage(conn));
        PQclear(res);
        exit_nicely(conn);
    }

    nFields = PQnfields(res);
    for (i = 0; i < nFields; i++)
        printf("%-15s\r\n", PQfname(res, i));
    printf("\r\n");

    for (i = 0; i < PQntuples(res); i++)
    {
        for (j = 0; j < nFields; j++)
            printf("%-15s\r\n", PQgetvalue(res, i, j));
        printf("\r\n");
    }

    PQclear(res);


        res = PQexec(conn, "CLOSE myportal");
        PQclear(res);

        res = PQexec(conn, "END");
        PQclear(res);

        PQfinish(conn);

        cout << "cot end: \t" << (int)context << "\r\n";
}

#endif
