#include "stdafx.h"
#include "../Dzcot/Dzcot.h"
#include "TestLibpq.h"
#include <libpq-fe.h>
#include <iostream>
using namespace std;

#define  TEST_LIBPQ
#ifdef TEST_LIBPQ

#pragma comment( lib, "libpqdll.lib" )

//#define printf(...)

static void
exit_nicely(PGconn *conn)
{
    PQfinish(conn);
    exit(1);
}

int __stdcall StartTestLibpq( void *context )
{
    int count = (int)context;
    for( int i=0; i<count; i++ ){
        DzStartCot( TestLibpq, (void*)i );
    }
    return 0;
}

int __stdcall TestLibpq( void *context )
{
    const char *conninfo;
    PGconn     *conn;
    PGresult   *res;
    int         nFields;
    int         i;
    int         j;


    cout << "cot start: \t" << (int)context << "\r\n";

    /*
     * ����û������������ṩ�˲�����
     * ��ô�������� conninfo �ִ�������ȱʡ������ dbname=template1
     * ���Ҷ���������ʹ�û�����������ȱʡֵ��
     * 
     */
    conninfo = "user=BombBabyMgr password=asdf hostaddr=192.168.1.109 port=5432 dbname=template1";
    conninfo = "user=BombBabyMgr password=asdf hostaddr=192.168.1.109 port=5432 dbname=BombBaby";

    /* �����ݿ⽨������ */
    conn = PQconnectdb(conninfo);

	/*
	* ���һ����������������Ƿ�ɹ�����
	*/
	if (PQstatus(conn) != CONNECTION_OK)
	{
        printf( "Connection to database failed: %s", PQerrorMessage(conn) );
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
        exit_nicely(conn);
	}

    /*
     * ��������Ĳ��԰����漰ʹ���α꣬������������Ǳ�����һ����������档
     * ���ǿ�����һ���򵥵� PQexec()��ִ��
     * "select * from pg_database"�����ȫ�����������������Ļ���һ�����Ӿ�̫���ˡ�
     */
	res = PQexec(conn, "BEGIN");
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        printf( "BEGIN command failed: %s\r\n", PQerrorMessage(conn) );
        fprintf(stderr, "BEGIN command failed: %s", PQerrorMessage(conn));
        PQclear(res);
        exit_nicely(conn);
    }

	/*
	 * ���������Ҫ PGresult �ˣ�����Ӧ�� PQclear���Ա����ڴ�й©
	 */
	 PQclear(res);

	/*
	 * �Ӵ洢���ݿ���Ϣ��ϵͳ�� pg_database ��ץȡ������
	 */
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

    /* ���ȣ���ӡ������ */
    nFields = PQnfields(res);
    for (i = 0; i < nFields; i++)
        printf("%-15s\r\n", PQfname(res, i));
    printf("\r\n");

    /* Ȼ�󣬴�ӡ������ */
    for (i = 0; i < PQntuples(res); i++)
    {
        for (j = 0; j < nFields; j++)
            printf("%-15s\r\n", PQgetvalue(res, i, j));
        printf("\r\n");
    }

    PQclear(res);


	/* �ر���ڣ����ǲ��ù��Ĵ�����... */
        res = PQexec(conn, "CLOSE myportal");
        PQclear(res);

	/* �ύ���� */
        res = PQexec(conn, "END");
        PQclear(res);

	/* �ر������ݿ�����Ӳ������� */
        PQfinish(conn);

        cout << "cot end: \t" << (int)context << "\r\n";
	return 0;

}

#endif