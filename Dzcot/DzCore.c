/********************************************************************
    created:    2010/02/11 22:11
    file:       DzCore.c
    author:     Foreverflying
    purpose:    
*********************************************************************/

#include "DzIncOs.h"
#include "DzCore.h"

void __stdcall DelayFreeTheadRoutine( void* context )
{
    DzHost* host = GetHost();
    DzThread* dzThread = (DzThread*)context;

    FreeCotStack( dzThread );
    dzThread->lItr.next = host->threadPool;
    host->threadPool = &dzThread->lItr;
}
