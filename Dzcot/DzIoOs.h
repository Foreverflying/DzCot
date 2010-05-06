
#ifndef _DZ_IO_OS_H_
#define _DZ_IO_OS_H_

#ifdef _WIN32
    #include "win/DzIoWin.h"
#elif defined LINUX
    #include "linux/DzIoLinux.h"
#endif

#endif  //#ifdef _DZ_IO_OS_H_