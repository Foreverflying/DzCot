/**
 *  @file       DzBaseOs.h
 *  @brief      
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/11/22
 *
 */

#ifndef __DzBaseOs_h__
#define __DzBaseOs_h__

#ifdef _WIN32
#include "win/DzBaseWin.h"
#elif defined __linux__
#include "linux/DzBaseLnx.h"
#endif

#endif // __DzBaseOs_h__
