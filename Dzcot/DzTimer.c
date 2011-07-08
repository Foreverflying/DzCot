/********************************************************************
    created:    2010/02/11 22:07
    file:       DzTimer.c
    author:     Foreverflying
    purpose:    
*********************************************************************/

#include "DzIncOs.h"
#include "DzCore.h"
#include "DzTimer.h"
#include "DzSynObj.h"

BOOL NotifyMinTimers( DzHost* host, int* timeout )
{
    DzTimerNode* timerNode;
    int64 currTime;
    int64 cmpTime;
    BOOL ret = FALSE;

    currTime = MilUnixTime();
    cmpTime = currTime + MIN_TIME_INTERVAL;
    while( host->timerCount > 0 && GetMinTimerNode( host )->timestamp <= cmpTime ){
        timerNode = GetMinTimerNode( host );
        if( timerNode->repeat != 1 ){
            if( timerNode->repeat ){
                timerNode->repeat--;
            }
            timerNode->timestamp -= timerNode->interval;    //timerNode->interval is negative
            AdjustMinTimer( host, timerNode );
        }else{
            RemoveMinTimer( host );
        }
        ret = NotifyTimerNode( host, timerNode ) || ret;
    }
    if( ret ){
        return TRUE;
    }
    if( timeout ){
        *timeout = host->timerCount > 0 ? (int)( GetMinTimerNode( host )->timestamp - currTime ) : -1;
    }
    return FALSE;
}

void __stdcall CallbackTimerEntry( void* context )
{
    DzSynObj* timer = (DzSynObj*)context;

    if( timer->routine ){
        timer->routine( timer->context );
    }
    CloseCallbackTimer( GetHost(), timer, FALSE );
}