/**
 *  @file       DzCoreOs.h
 *  @brief      
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzCoreOs_h__
#define __DzCoreOs_h__

#ifdef _WIN32
    #include "win/DzCoreWin.h"
#elif defined __linux__
    #include "linux/DzCoreLnx.h"
#endif

#endif // __DzCoreOs_h__
