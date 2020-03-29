/**
 *  @file       DzBaseArch.h
 *  @brief      for windows x64
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

#endif // __DzBaseArch_h__