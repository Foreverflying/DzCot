
#include "Config.h"

#ifdef TRY_FUNC
void TRY_FUNC();
#else
#define TRY_FUNC()
#endif

void MainEntry()
{
    TRY_FUNC();
}