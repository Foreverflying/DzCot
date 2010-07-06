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
     * 如果用户在命令行上提供了参数，
     * 那么拿它当作 conninfo 字串；否则缺省设置是 dbname=template1
     * 并且对其它连接使用环境变量或者缺省值。
     * 
     */
    conninfo = "user=BombBabyMgr password=asdf hostaddr=192.168.1.109 port=5432 dbname=template1";
    conninfo = "user=BombBabyMgr password=asdf hostaddr=192.168.1.109 port=5432 dbname=BombBaby";

    /* 和数据库建立链接 */
    conn = PQconnectdb(conninfo);

	/*
	* 检查一下与服务器的连接是否成功建立
	*/
	if (PQstatus(conn) != CONNECTION_OK)
	{
        printf( "Connection to database failed: %s", PQerrorMessage(conn) );
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
        exit_nicely(conn);
	}

    /*
     * 我们这里的测试案例涉及使用游标，这种情况下我们必须在一个事务块里面。
     * 我们可以用一个简单的 PQexec()，执行
     * "select * from pg_database"，完成全部操作，不过那样的话对一个例子就太简单了。
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
	 * 如果不再需要 PGresult 了，我们应该 PQclear，以避免内存泄漏
	 */
	 PQclear(res);

	/*
	 * 从存储数据库信息的系统表 pg_database 中抓取数据行
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

    /* 首先，打印属性名 */
    nFields = PQnfields(res);
    for (i = 0; i < nFields; i++)
        printf("%-15s\r\n", PQfname(res, i));
    printf("\r\n");

    /* 然后，打印数据行 */
    for (i = 0; i < PQntuples(res); i++)
    {
        for (j = 0; j < nFields; j++)
            printf("%-15s\r\n", PQgetvalue(res, i, j));
        printf("\r\n");
    }

    PQclear(res);


	/* 关闭入口，我们不用关心错误检查... */
        res = PQexec(conn, "CLOSE myportal");
        PQclear(res);

	/* 提交事务 */
        res = PQexec(conn, "END");
        PQclear(res);

	/* 关闭与数据库的连接并且清理 */
        PQfinish(conn);

        cout << "cot end: \t" << (int)context << "\r\n";
	return 0;

}

#endif