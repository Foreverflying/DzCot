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
#elif defined __linux__
    #include "linux/DzCoreLnx.h"
#endif

#endif // __DzCoreOs_h__
