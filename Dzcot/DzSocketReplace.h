
#ifndef _DZ_SOCKET_REPLACE_H_
#define _DZ_SOCKET_REPLACE_H_

#include "Dzcot.h"

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

#endif

