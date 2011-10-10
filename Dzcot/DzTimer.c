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

int NotifyMinTimers( DzHost* host )
{
    DzTimerNode* timerNode;
    int64 currTime;
    int64 cmpTime;
    BOOL ret;

    while( host->timerCount > 0 ){
        ret = FALSE;
        currTime = MilUnixTime();
        cmpTime = currTime + MIN_TIME_INTERVAL;
        while( GetMinTimerNode( host )->timestamp <= cmpTime ){
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
            if( host->timerCount == 0 ){
                if( !ret ){
                    return -1;
                }
                break;
            }
        }
        if( ret ){
            host->currPriority = CP_FIRST;
            Schedule( host );
        }else{
            return (int)( GetMinTimerNode( host )->timestamp - currTime );
        }
    }
    return -1;
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