
#ifndef _DZ_STRUCTS_OS_H_
#define _DZ_STRUCTS_OS_H_

#ifdef _WIN32
    #include "win/DzStructsWin.h"
#elif defined LINUX
    #include "linux/DzStructsLinux.h"
#endif

#endif  //#ifdef _DZ_STRUCTS_OS_H_