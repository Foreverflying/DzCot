/**
 *  @file       DzIoOs.c
 *  @brief      for linux
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#include "../DzInc.h"
#include "../DzCore.h"
#include "DzIoOs.h"

void __stdcall GetNameInfoEntry(intptr_t context)
{
    DzLNode* node = (DzLNode*)context;

    *(int*)(node->d8) = getnameinfo(
        (const struct sockaddr*)node->d1,
        (socklen_t)node->d2,
        (char*)node->d3,
        (size_t)node->d4,
        (char*)node->d5,
        (size_t)node->d6,
        (int)node->d7
        );
}

void __stdcall GetAddrInfoEntry(intptr_t context)
{
    DzLNode* node = (DzLNode*)context;

    *(int*)(node->d8) = getaddrinfo(
        (const char*)node->d1,
        (const char*)node->d2,
        (const struct addrinfo*)node->d3,
        (struct addrinfo**)node->d4
        );
}
