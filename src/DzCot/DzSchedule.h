/**
 *  @file       DzSchedule.h
 *  @brief      
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzSchedule_h__
#define __DzSchedule_h__

#include "DzStructs.h"
#include "DzList.h"
#include "DzBase.h"
#include "DzDebug.h"

static inline
BOOL UpdateCurrPriority( DzHost* host, int currPriority )
{
    if( currPriority > CP_FIRST  ){
        if( host->taskLs[ CP_FIRST ].head ){
            host->currPri = CP_FIRST;
            return TRUE;
        }
        host->currPri = CP_HIGH;
    }
    return FALSE;
}

// DispatchCot:
// the dzCot is in the block queue or new created
// put the cot to the active queue
static inline
void DispatchCot( DzHost* host, DzCot* dzCot )
{
    AddLItrToTail( &host->taskLs[ dzCot->priority ], &dzCot->lItr );
}

static inline
void PushCotToTop( DzHost* host, DzCot* dzCot )
{
    AddLItrToHead( &host->taskLs[ CP_FIRST ], &dzCot->lItr );
    host->currPri = CP_FIRST;
}

static inline
void SwitchToCot( DzHost* host, DzCot* dzCot )
{
    __Dbg( CheckCotStackOverflow )( host, host->currCot );
    DzSwitch( host, dzCot );
}

// Schedule:
// schedule cot according to the priority
// if no cot is active, schedule the ScheduleCenter
static inline
void Schedule( DzHost* host )
{
    DzCot* dzCot;

    if( host->scheduleCd ){
        do{
            if( host->taskLs[ host->currPri ].head ){
                host->scheduleCd--;
                dzCot = MEMBER_BASE( host->taskLs[ host->currPri ].head, DzCot, lItr );
                EraseListHead( &host->taskLs[ host->currPri ] );
                SwitchToCot( host, dzCot );
                return;
            }else{
                host->currPri++;
            }
        }while( host->currPri <= host->lowestPri );
    }
    SwitchToCot( host, &host->centerCot );
}

static inline
void DispatchCurrCot( DzHost* host )
{
    AddLItrToTail( &host->taskLs[ host->lowestPri ], &host->currCot->lItr );
    Schedule( host );
}

#endif // __DzSchedule_h__
