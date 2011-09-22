/********************************************************************
    created:    2011/09/22 21:34
    file:       Inc_DzSockUtils.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __Inc_DzSockUtils_h__
#define __Inc_DzSockUtils_h__

#include <assert.h>

#define DZMAKEIPADDRESS( a1, a2, a3, a4 ) (\
    (a1 << 24) | (a2 << 16) | (a3 << 8) | a4\
)

#define hton16( n ) (\
    ( ( n & 0xff ) << 8 ) | ( ( n & 0xff00 ) >> 8 )\
)

#define hton32( n ) (\
    ( ( n & 0xff ) << 24 ) | ( ( n & 0xff00 ) << 8 ) |\
    ( ( n & 0xff0000 ) >> 8 ) | ( ( n & 0xff000000 ) >> 24 )\
)

#endif // __Inc_DzSockUtils_h__