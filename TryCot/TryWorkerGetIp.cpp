
#include "CotTryUtil.h"

CotEntry GetHostByNameEntry( intptr_t context )
{
    int* ip = (int*)context;
    struct hostent* ret;

    ret = gethostbyname( "www.baidu.com" );
    *ip = *(int*)ret->h_addr_list;
}

CotEntry GetIpEntry( intptr_t context )
{
    DzSetWorkerPoolDepth( 2 );

    int ip;
    DzRunWorker( GetHostByNameEntry, (intptr_t)&ip );
    printf(
        "ip is %d %d %d %d\r\n",
        ( ip >> 24 ) & 0xff,
        ( ip >> 16 ) & 0xff,
        ( ip >> 8 ) & 0xff,
        ip & 0xff
        );

    DzSleep( 5000 );

    DzRunWorker( GetHostByNameEntry, (intptr_t)&ip );
    printf(
        "ip is %d %d %d %d\r\n",
        ( ip >> 24 ) & 0xff,
        ( ip >> 16 ) & 0xff,
        ( ip >> 8 ) & 0xff,
        ip & 0xff
        );
}
 
CotEntry TryGetAddrInfoEntry()
{
#ifdef _WIN32
    addrinfoW* infow;
    int ret = DzGetAddrInfoW( L"www.163.com", L"http", NULL, &infow );
    DzFreeAddrInfoW( infow );
#endif

    addrinfo* info;
    ret = DzGetAddrInfoA( "www.163.com", "http", NULL, &info );
    DzFreeAddrInfoA( info );

    char buff[32];
    ret = DzGetNameInfoA( NULL, 0, NULL, 0, buff, 32, 0 );
}

void TryWorkerGetIp()
{
    StartHosts( GetIpEntry, 1 );
}