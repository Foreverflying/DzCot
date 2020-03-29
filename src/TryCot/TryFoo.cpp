
#include "CotTryUtil.h"

CotEntry FooEntry(intptr_t context)
{
    printf("Try foo\r\n");
}

void TryFoo()
{
    StartHosts(FooEntry);
}