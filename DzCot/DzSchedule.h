/********************************************************************
    created:    2010/02/11 22:04
    file:       DzSchedule.h
    author:     Foreverflying
    purpose:    
********************************************************************/

#ifndef __DzSchedule_h__
#define __DzSchedule_h__

#include "DzStructs.h"
#include "DzList.h"
#include "DzBase.h"
#include "DzDebug.h"

#ifdef __cplusplus
extern "C"{
#endif

inline BOOL UpdateCurrPriority( DzHost* host, int currPriority )
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
inline void DispatchCot( DzHost* host, DzCot* dzCot )
{
    AddLItrToTail( &host->taskLs[ dzCot->priority ], &dzCot->lItr );
}

inline void PushCotToTop( DzHost* host, DzCot* dzCot )
{
    AddLItrToHead( &host->taskLs[ CP_FIRST ], &dzCot->lItr );
    host->currPri = CP_FIRST;
}

inline void SwitchToCot( DzHost* host, DzCot* dzCot )
{
    __Dbg( CheckCotStackOverflow )( host, host->currCot );
    DzSwitch( host, dzCot );
}

// Schedule:
// schedule cot according to the priority
// if no cot is active, schedule the ScheduleCenter
inline void Schedule( DzHost* host )
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

inline void DispatchCurrCot( DzHost* host )
{
    AddLItrToTail( &host->taskLs[ host->lowestPri ], &host->currCot->lItr );
    Schedule( host );
}

#ifdef __cplusplus
};
#endif

#endif // __DzSchedule_h__
