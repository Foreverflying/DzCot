/********************************************************************
    created:    2010/02/11 22:11
    file:       DzCore.c
    author:     Foreverflying
    purpose:    
********************************************************************/

#include "DzInc.h"
#include "DzCore.h"
#include "DzIoOs.h"

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

    host->currCot->entry( context );
    SetEvt( host, host->currCot->evt );
    CloseSynObj( host, host->currCot->evt );
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

// CotScheduleCenter:
// the Cot Schedule Center uses the host's origin thread's stack
// manager all kernel objects that may cause real block
void CotScheduleCenter( DzHost* host )
{
    int n;

    while( 1 ){
        while( 1 ){
            if( host->scheduleCd ){
                n = DispatchMinTimers( host );
                n = DispatchRmtCots( host, n );
                if( n == 0 ){
                    host->currPri = CP_FIRST;
                    Schedule( host );
                    continue;
                }else if( !host->cotCount ){
                    break;
                }
            }else{
                n = 0;
            }
            BlockAndDispatchIo( host, n );
            DispatchMinTimers( host );
            DispatchRmtCots( host, 0 );
            host->currPri = CP_FIRST;
            host->scheduleCd = host->ioReactionRate;
            Schedule( host );
        }
        if( host->lazyTimer ){
            DealLazyResEntry( 0 );
        }
        if( AtomAndInt( &host->mgr->liveSign, ~host->hostMask ) != host->hostMask ){
            BlockAndDispatchIo( host, -1 );
            if( AtomReadInt( &host->mgr->liveSign ) ){
                AtomAndInt( host->rmtCheckSignPtr, ~RMT_CHECK_SLEEP_SIGN );
                continue;
            }
        }else{
            //be sure quit id 0 host at last, for hostMgr is in id 0 host's stack
            for( n = host->hostCount - 1; n >= 0; n-- ){
                if( n != host->hostId ){
                    AwakeRemoteHost( host->mgr->hostArr[ n ] );
                }
            }
        }
        break;
    }
}

void CotScheduleCenterNoRmtCheck( DzHost* host )
{
    int n;

    while( 1 ){
        if( host->scheduleCd ){
            n = DispatchMinTimers( host );
            if( n == 0 ){
                host->currPri = CP_FIRST;
                Schedule( host );
                continue;
            }else if( !host->cotCount ){
                break;
            }
        }else{
            n = 0;
        }
        BlockAndDispatchIoNoRmtCheck( host, n );
        DispatchMinTimers( host );
        host->currPri = CP_FIRST;
        host->scheduleCd = host->ioReactionRate;
        Schedule( host );
    }
}
