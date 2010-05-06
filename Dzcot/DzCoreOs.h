
#ifndef _DZ_CORE_OS_H_
#define _DZ_CORE_OS_H_

#ifdef _WIN32
    #include "win/DzCoreWin.h"
#elif defined LINUX
    #include "linux/DzCoreLinux.h"
#endif

#endif  //#ifdef _DZ_CORE_OS_H_