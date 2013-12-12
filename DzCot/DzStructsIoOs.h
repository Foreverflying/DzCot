/**
 *  @file       DzStructsIoOs.h
 *  @brief      
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzStructsIoOs_h__
#define __DzStructsIoOs_h__

#ifdef _WIN32
    #include "win/DzStructsIoWin.h"
#elif defined __linux__
    #include "linux/DzStructsIoLnx.h"
#endif

#endif // __DzStructsIoOs_h__
