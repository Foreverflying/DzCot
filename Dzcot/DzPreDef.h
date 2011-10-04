/********************************************************************
    created:    2010/02/11 22:07
    file:       DzType.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzType_h__
#define __DzType_h__

#undef FALSE
#undef TRUE
#undef NULL

#define FALSE   0
#define TRUE    1
#define NULL    0

typedef int BOOL;

enum
{
    DS_OK,
    DS_TIMEOUT,
    DS_INVALID_ARG,
    DS_ALREADY_EXIST,
    DS_NOT_INIT,
    DS_NO_MEMORY
};

enum
{
    CP_FIRST,
    CP_HIGH = CP_FIRST,
    CP_NORMAL,
    CP_LOW,
    COT_PRIORITY_COUNT,     //should not use this;
    CP_DEFAULT = COT_PRIORITY_COUNT
};

enum
{
    SS_FIRST,
    SS_4K = SS_FIRST,
    SS_16K,
    SS_64K,
    SS_256K,
    SS_1M,
    SS_4M,
    SS_16M,
    SS_64M,
    STACK_SIZE_COUNT,       //should not use this
    SS_DEFAULT = STACK_SIZE_COUNT
};

typedef struct _DzParamNode
{
    void*           context3;
    void*           context2;
    void*           context1;
    void*           content;
}DzParamNode;

#ifdef _WIN32

#if defined( _X86_ )
typedef int ssize_t;
#elif defined( _AMD64_ )
typedef long long ssize_t;
#endif

typedef struct _DzBuf
{
    unsigned long   len;
    void*           buf;
}DzBuf;

#elif defined __linux__

typedef struct _DzBuf
{
    void*           buf;
    size_t          len;
}DzBuf;

#endif

struct _DzSynObj;
typedef struct _DzSynObj* DzHandle;

#ifdef __linux__
#define __stdcall __attribute__((stdcall))
#endif

typedef void (__stdcall *DzRoutine)( void* context );

#define DZ_O_RD         0x01
#define DZ_O_WR         0x02
#define DZ_O_RDWR       0x03
#define DZ_O_CREATE     0x08
#define DZ_O_EXCL       0x10
#define DZ_O_TRUNC      0x20
#define DZ_O_APPEND     0x40

#endif // __DzType_h__
