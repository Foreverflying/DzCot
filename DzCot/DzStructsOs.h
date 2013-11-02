/**
 *  @file       DzStructsOs.h
 *  @brief      
 *  @author	    Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzStructsOs_h__
#define __DzStructsOs_h__

#ifdef _WIN32
    #include "win/DzStructsWin.h"
#elif defined __linux__
    #include "linux/DzStructsLnx.h"
#endif

#endif // __DzStructsOs_h__
