/**
 *  @file       DzSocketReplace.h
 *  @brief      
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzSocketReplace_h__
#define __DzSocketReplace_h__

#include "DzCot.h"

#undef socket
#undef closesocket
#undef getsockopt
#undef setsockopt
#undef getsockname
#undef getpeername
#undef bind
#undef listen
#undef shutdown
#undef connect
#undef accept
#undef send
#undef recv
#undef sendto
#undef recvfrom
#undef getnameinfo
#undef getaddrinfo
#undef freeaddrinfo
#undef inet_pton
#undef inet_ntop

#define socket          DzSocket
#define closesocket     DzClose
#define getsockopt      DzGetSockOpt
#define setsockopt      DzSetSockOpt
#define getsockname     DzGetSockName
#define getpeername     DzGetPeerName
#define bind            DzBind
#define listen          DzListen
#define shutdown        DzShutdown
#define connect         DzConnect
#define accept          DzAccept
#define send            DzSend
#define recv            DzRecv
#define sendto          DzSendTo
#define recvfrom        DzRecvFrom
#define getnameinfo     DzGetNameInfo
#define getaddrinfo     DzGetAddrInfo
#define freeaddrinfo    DzFreeAddrInfo
#define inet_pton       DzInetPton
#define inet_ntop       DzInetNtop

#endif // __DzSocketReplace_h__
