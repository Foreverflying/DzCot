/********************************************************************
    created:    2010/11/22 17:29
    file:       DzBaseOs.h
    author:     Foreverflying
    purpose:    
********************************************************************/

#ifndef __DzBaseOs_h__
#define __DzBaseOs_h__

#ifdef _WIN32
#include "win/DzBaseWin.h"
#elif defined __linux__
#include "linux/DzBaseLnx.h"
#endif

#endif // __DzBaseOs_h__