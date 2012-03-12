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
#undef connect
#undef accept
#undef send
#undef recv

#define socket( domain, type, protocol ) DzSocket( (domain), (type), (protocol) )
#define connect( s, addr, addrLen ) DzConnect( (s), (addr), (addrLen) )
#define accept( s, addr, addrLen ) DzAccept( (s), (addr), (addrLen) )
#define send( s, buf, len, flag ) DzSend( (s), (buf), (len), (flag) )
#define recv( s, buf, len, flag ) DzRecv( (s), (buf), (len), (flag) )

#endif // __Inc_DzSocketReplace_h__
