
#ifndef _DZ_STRUCTS_IO_WIN_H_
#define _DZ_STRUCTS_IO_WIN_H_

#include "../DzStructs.h"

typedef struct _DzFd
{
    union{
        DzQItr      qItr;
        HANDLE      file;
        SOCKET      s;
    };
    BOOL        isFile;
    union{
        off_t64     offset;
        struct{
            DWORD   offsetLow;
            DWORD   offsetHigh;
        };
    };
}DzFd;

typedef struct _DzAsynIo
{
    union{
        DzQItr      qItr;
        DzRoutine   callback;
    };
    void*           context;
    int             fd;
    int             ref;
    DzFastEvt       fastEvt;
    OVERLAPPED      overlapped;
}DzAsynIo;

#endif  //#ifdef _DZ_STRUCTS_IO_WIN_H_