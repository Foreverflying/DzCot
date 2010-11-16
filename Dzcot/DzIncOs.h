/********************************************************************
    created:    2010/02/11 21:55
    file:       DzIncOs.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzIncOs_h__
#define __DzIncOs_h__

#ifdef _WIN32
    #include "win/DzIncWin.h"
#elif defined LINUX
    #include "linux/DzIncLinux.h"
#endif

#endif // __DzIncOs_h__
