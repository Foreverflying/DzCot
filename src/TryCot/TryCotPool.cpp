
#include "CotTryUtil.h"

CotEntry PrintSome(intptr_t context)
{
    int idx = (int)context;
    int a = 0;
    printf("Idx %d a's address is %p\r\n", idx, &a);
    // DzSleep(15);
}

CotEntry CotPoolEntry(intptr_t context)
{
    printf("Size of BOOL is %d\r\n", (int)sizeof(BOOL));
    DzSetCotPoolDepth(ST_UM, 5);
    for (int i = 0; i < 10; i++) {
        DzStartCotInstant(PrintSome);
    }
    for (int i = 0; i < 10; i++) {
        DzStartCot(PrintSome);
    }
}

void TryCotPool()
{
    StartHosts(CotPoolEntry);
}