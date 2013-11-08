/**
 *  @file       Config.cpp
 *  @brief      
 *  @author	    Foreverflying <foreverflying@live.cn>
 *  @date       2013/11/08
 *
 */

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
