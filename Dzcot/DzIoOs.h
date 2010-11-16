/********************************************************************
    created:    2010/02/11 22:02
    file:       DzIoOs.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzIoOs_h__
#define __DzIoOs_h__

#ifdef _WIN32
    #include "win/DzIoWin.h"
#elif defined LINUX
    #include "linux/DzIoLinux.h"
#endif

#endif // __DzIoOs_h__
