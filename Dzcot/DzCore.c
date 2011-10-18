/********************************************************************
    created:    2010/02/11 22:11
    file:       DzCore.c
    author:     Foreverflying
    purpose:    
*********************************************************************/

#include "DzIncOs.h"
#include "DzCore.h"

void __stdcall DelayFreeCotHelper( intptr_t context )
{
    DzHost* host = GetHost();
    DzCot* dzCot = (DzCot*)context;

    FreeCotStack( dzCot );
    dzCot->lItr.next = host->cotPool;
    host->cotPool = &dzCot->lItr;
}

void __stdcall EventNotifyCotEntry( intptr_t context )
{
    DzHost* host = GetHost();
    DzLNode* node = (DzLNode*)context;
    DzRoutine entry = (DzRoutine)node->content;
    DzSynObj* evt = (DzSynObj*)node->context2;

    entry( node->context1 );
    SetEvt( host, evt );
    CloseSynObj( host, evt );
    FreeLNode( host, node );
}

void __stdcall CallbackTimerEntry( intptr_t context )
{
    DzSynObj* timer = (DzSynObj*)context;

    if( timer->routine ){
        timer->routine( timer->context );
    }
    timer->ref--;
    if( timer->ref == 0 ){
        InitDList( &timer->waitQ[ CP_HIGH ] );
        InitDList( &timer->waitQ[ CP_NORMAL ] );
        FreeSynObj( GetHost(), timer );
    }
}
