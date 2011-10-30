/********************************************************************
    created:    2010/02/11 22:05
    file:       DzStructsIoOs.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzStructsIoOs_h__
#define __DzStructsIoOs_h__

#ifdef _WIN32
    #include "win/DzStructsIoWin.h"
#elif defined __linux__
    #include "linux/DzStructsIoLnx.h"
#endif

#endif // __DzStructsIoOs_h__
