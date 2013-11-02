/**
 *  @file       DzIoOs.h
 *  @brief      
 *  @author	    Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzIoOs_h__
#define __DzIoOs_h__

#ifdef _WIN32
    #include "win/DzIoWin.h"
#elif defined __linux__
    #include "linux/DzIoLnx.h"
#endif

#endif // __DzIoOs_h__
