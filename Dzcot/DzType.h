
#ifndef _DZ_TYPE_H_
#define _DZ_TYPE_H_

#undef FALSE
#undef TRUE

#define FALSE   0
#define TRUE    1

typedef int BOOL;

enum
{
    DS_NONE,
    DS_OK,
    DS_TIMEOUT,
    DS_INVALID_ARG,
    DS_ALREADY_EXIST,
    DS_NOT_INIT,
    DS_NO_MEMORY
};

enum
{
    CP_INSTANT,
    CP_HIGH,
    CP_NORMAL,
    CP_LOW,
    COT_PRIORITY_COUNT,     //should not use this;
    CP_DEFAULT
};

enum
{
    SS_64K,
    SS_128K,
    SS_256K,
    SS_512K,
    SS_1M,
    SS_2M,
    SS_4M,
    SS_8M,
    STACK_SIZE_COUNT,       //should not use this
    SS_DEFAULT
};

typedef struct _DzParamNode
{
    void*           context3;
    void*           context2;
    void*           context1;
    void*           content;
}DzParamNode;

struct _DzSynObj;
typedef struct _DzSynObj* DzHandle;

typedef int (__stdcall *DzRoutine)( void *context );

#endif      //#ifndef _DZ_TYPE_H_