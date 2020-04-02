/**
 *  @file       DzBaseArch.h
 *  @brief      for windows x86
 *  @author	    Foreverflying <foreverflying@live.cn>
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
    return (DzHost*)__readfsdword(20);
}

static inline
void SetHost(DzHost* host)
{
    __writefsdword(20, (DWORD)host);
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
    return (void*)__readfsdword(0);
}

static inline
char* GetStackPtr()
{
    return (char*)__readfsdword(4);
}

static inline
size_t ArchSeek(HANDLE handle, ssize_t offset, int whence)
{
    return (size_t)SetFilePointer(handle, (long)offset, NULL, whence);
}

static inline
size_t ArchFileSize(HANDLE handle)
{
    return GetFileSize(handle, NULL);
}

#endif // __DzBaseArch_h__