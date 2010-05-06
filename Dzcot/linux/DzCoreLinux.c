#include "DzIncWin.h"
#include "DzCoreWin.h"

char* AllocStack( int sSize )
{
    size_t size;
    char *base;

    size = DZ_STACK_UNIT_SIZE << sSize;
    base = (char*)mmap()
    base = (char*)VirtualAlloc(
        NULL,
        size,
        MEM_RESERVE,
        PAGE_READWRITE
        );

    if( !base ){
        return NULL;
    }
    return base + size;
}

void FreeStack( char *stack, int sSize )
{
    size_t size;
    char *base;

    size = DZ_STACK_UNIT_SIZE << sSize;
    base = stack - size;
    VirtualFree( base, 0, MEM_RELEASE );
}

char* CommitStack( char *stack, size_t size )
{
    void *tmp;
    BOOL ret;

    tmp = VirtualAlloc(
        stack - size,
        size,
        MEM_COMMIT,
        PAGE_READWRITE
        );
    if( !tmp ){
        return NULL;
    }
    ret = VirtualProtect(
        stack - size,
        PAGE_SIZE,
        PAGE_READWRITE | PAGE_GUARD,
        (LPDWORD)&tmp
        );
    return ret ? stack - size + PAGE_SIZE : NULL;
}

void DeCommitStack( char *stack, char *stackLimit )
{
    size_t size;

    size = stack - stackLimit;
    VirtualFree( stackLimit, size, MEM_DECOMMIT );
}
