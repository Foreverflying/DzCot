/********************************************************************
    created:    2011/10/24 21:13
    file:       DzBase.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzBase_h__
#define __DzBase_h__

#include "DzInc.h"
#include "DzStructs.h"
#include "DzBaseOs.h"

inline DzLItr* AtomPopSList( DzLItr** head )
{
    DzLItr* now;
    DzLItr* origin;

    origin = NULL;
    now = (DzLItr*)AtomReadPtr( (void**)head );
    while( now && now != origin ){
        origin = now;
        now = (DzLItr*)AtomCasPtr( (void**)head, origin, origin->next );
    }
    return now;
}

inline void AtomPushSList( DzLItr** head, DzLItr* lItr )
{
    DzLItr* now;
    DzLItr* origin;

    origin = NULL;
    now = (DzLItr*)AtomReadPtr( (void**)head );
    do{
        lItr->next = now;
        origin = now;
        now = (DzLItr*)AtomCasPtr( (void**)head, origin, lItr );
    }while( now != origin );
}

#endif // __DzBase_h__
