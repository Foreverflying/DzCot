/********************************************************************
    created:    2010/02/11 22:04
    file:       Inc_DzSocketReplace.h
    author:     Foreverflying
    purpose:    
********************************************************************/

#ifndef __Inc_DzSocketReplace_h__
#define __Inc_DzSocketReplace_h__

#include "Inc_DzCot.h"

#undef socket
#undef close
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

#define socket          DzSocket
#define close           DzCloseSocket
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

#endif // __Inc_DzSocketReplace_h__
