/**
 *  @file       DzIncOs.h
 *  @brief      
 *  @author	    Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzIncOs_h__
#define __DzIncOs_h__

#ifdef _WIN32
#include "win/DzIncWin.h"
#elif defined __linux__
#include "linux/DzIncLnx.h"
#endif

#endif // __DzIncOs_h__
