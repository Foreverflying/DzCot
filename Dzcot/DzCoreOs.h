/********************************************************************
    created:    2010/02/11 21:50
    file:       DzCoreOs.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzCoreOs_h__
#define __DzCoreOs_h__

#ifdef _WIN32
    #include "win/DzCoreWin.h"
#elif defined LINUX
    #include "linux/DzCoreLinux.h"
#endif

#endif // __DzCoreOs_h__
