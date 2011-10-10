/********************************************************************
    created:    2010/02/11 22:11
    file:       DzCore.c
    author:     Foreverflying
    purpose:    
*********************************************************************/

#include "DzIncOs.h"
#include "DzCore.h"

void __stdcall DelayFreeTheadRoutine( intptr_t context )
{
    DzHost* host = GetHost();
    DzThread* dzThread = (DzThread*)context;

    FreeCotStack( dzThread );
    dzThread->lItr.next = host->threadPool;
    host->threadPool = &dzThread->lItr;
}

void __stdcall EventNotifyCotRoutine( intptr_t context )
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