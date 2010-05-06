
#ifndef _DZ_STRUCTS_IO_OS_H_
#define _DZ_STRUCTS_IO_OS_H_

#ifdef _WIN32
    #include "win/DzStructsIoWin.h"
#elif defined LINUX
    #include "linux/DzStructsIoLinux.h"
#endif

#endif  //#ifdef _DZ_STRUCTS_IO_OS_H_