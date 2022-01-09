/**
 *  @file       DzBaseArch.h
 *  @brief      for windows x64
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2015/09/13
 *
 */

#ifndef __DzBaseArch_h__
#define __DzBaseArch_h__

#include "../../DzStructs.h"

#ifdef STORE_HOST_IN_ARBITRARY_USER_POINTER

static inline
DzHost* GetHost()
{
    return *(DzHost**)(__readgsqword(0x30) + 40);
}

static inline
void SetHost(DzHost* host)
{
    *(DzHost**)(__readgsqword(0x30) + 40) = host;
}

static inline
BOOL AllocTlsIndex()
{
    return TRUE;
}

static inline
void FreeTlsIndex()
{
}

#endif  // STORE_HOST_IN_ARBITRARY_USER_POINTER

static inline
void* GetExceptPtr()
{
    return NULL;
}

static inline
char* GetStackPtr()
{
    return (char*)(__readgsqword(0x30) + 8);
}

static inline
size_t ArchSeek(HANDLE handle, ssize_t offset, int whence)
{
    size_t ret;

    return SetFilePointerEx(handle, *(LARGE_INTEGER*)&offset, (LARGE_INTEGER*)&ret, whence) ? ret : -1;
}

static inline
size_t ArchFileSize(HANDLE handle)
{
    size_t ret;

    return GetFileSizeEx(handle, (LARGE_INTEGER*)&ret) ? ret : -1;
}

#endif // __DzBaseArch_h__