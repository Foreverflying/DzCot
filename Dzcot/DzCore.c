/********************************************************************
    created:    2010/02/11 22:11
    file:       DzCore.c
    author:     Foreverflying
    purpose:    
*********************************************************************/

#include "DzIncOs.h"
#include "DzCore.h"

void __stdcall DelayFreeTheadEntry( intptr_t context )
{
    DzHost* host = GetHost();
    DzThread* dzThread = (DzThread*)context;

    FreeCotStack( dzThread );
    dzThread->lItr.next = host->threadPool;
    host->threadPool = &dzThread->lItr;
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

inline int DispatchMinTimers( DzHost* host )
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
                return ret ? 0 : -1;
            }
        }
        if( ret ){
            return 0;
        }else{
            return (int)( GetMinTimerNode( host )->timestamp - currTime );
        }
    }
    return -1;
}

inline BOOL DispatchRmtCall( DzHost* host, BOOL sleep )
{
    int count;
    int nowCount;
    int writePos;
    DzRmtCallPkg* pkg;
    DzRmtCallPkg* newPkg;

    if( sleep ){
        nowCount = AtomCasInt( host->checkSignPtr, 0, RMT_CHECK_SLEEP_SIGN );
    }else{
        nowCount = *host->checkSignPtr;
    }
    if( nowCount == 0 ){
        return FALSE;
    }
    count = nowCount;
    while( 1 ){
        writePos = *(volatile int*)&host->checkFifo->writePos;
        while( host->checkFifo->readPos != writePos ){
            pkg = host->checkFifo->callPkgArr + host->checkFifo->readPos;
            if( pkg->entry ){
                if( pkg->evt ){
                    newPkg = (DzRmtCallPkg*)AllocLNode( host );
                    CopyRmtCallPkg( pkg, newPkg );
                    StartCot(
                        host, FeedRmtCallEntry, (intptr_t)newPkg,
                        (int)pkg->priority, (int)pkg->sSize
                        );
                }else{
                    StartCot(
                        host, pkg->entry, pkg->context,
                        (int)pkg->priority, (int)pkg->sSize );
                }
            }else{
                SetEvt( host, pkg->evt );
            }
            host->checkFifo->readPos++;
            if( host->checkFifo->readPos == RMT_CALL_FIFO_SIZE ){
                host->checkFifo->readPos = 0;
            }
            nowCount--;
            if( nowCount == 0 ){
                nowCount = AtomSubInt( host->checkSignPtr, count );
                nowCount -= count;
                if( nowCount == 0 ){
                    if( sleep ){
                        nowCount = AtomCasInt(
                            host->checkSignPtr, 0, RMT_CHECK_SLEEP_SIGN
                            );
                        if( nowCount == 0 ){
                            return TRUE;
                        }
                    }else{
                        return TRUE;
                    }
                }
                count = nowCount;
            }
        }
        host->checkFifo = host->checkFifo->next;
    }
}

int NotifyMinTimersAndRmtCalls( DzHost* host )
{
    BOOL rmtRet;
    int timerRet;

    while( 1 ){
        timerRet = DispatchMinTimers( host );
        rmtRet = DispatchRmtCall( host, timerRet != 0 );
        if( rmtRet || timerRet == 0 ){
            host->currPriority = CP_FIRST;
            Schedule( host );
        }else{
            return timerRet;
        }
    }
}