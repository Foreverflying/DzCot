/********************************************************************
    created:    2010/02/11 22:06
    file:       DzStructsOs.h
    author:     Foreverflying
    purpose:    
********************************************************************/

#ifndef __DzStructsOs_h__
#define __DzStructsOs_h__

#ifdef _WIN32
    #include "win/DzStructsWin.h"
#elif defined __linux__
    #include "linux/DzStructsLnx.h"
#endif

#endif // __DzStructsOs_h__
