
#ifndef _DZ_INC_OS_H_
#define _DZ_INC_OS_H_

#ifdef _WIN32
    #include "win/DzIncWin.h"
#elif defined LINUX
    #include "linux/DzIncLinux.h"
#endif

#endif  //#ifdef _DZ_INC_OS_H_