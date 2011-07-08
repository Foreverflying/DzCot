/********************************************************************
    created:    2010/02/11 22:04
    file:       DzSchedule.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzSchedule_h__
#define __DzSchedule_h__

#include "DzStructs.h"
#include "DzList.h"
#include "DzBaseOs.h"

#ifdef __cplusplus
extern "C"{
#endif

inline BOOL UpdateCurrPriority( DzHost* host, int currPriority )
{
    if( currPriority > CP_FIRST  ){
        if( host->taskLs[ CP_FIRST ].head ){
            host->currPriority = CP_FIRST;
            return TRUE;
        }
        host->currPriority = CP_HIGH;
    }
    return FALSE;
}

// DispatchThread:
// the dzThread is in the block queue or new created
// put the thread to the active queue
inline void DispatchThread( DzHost* host, DzThread* dzThread )
{
    AddLItrToTail( &host->taskLs[ dzThread->priority ], &dzThread->lItr );
}

inline void TemporaryPushThread( DzHost* host, DzThread* dzThread )
{
    host->currPriority = CP_FIRST;
    AddLItrToHead( &host->taskLs[ CP_FIRST ], &dzThread->lItr );
}

inline void SwitchToCot( DzHost* host, DzThread* dzThread )
{
    CheckCotStackOverflow( host->currThread->sSize );
    DzSwitch( host, dzThread );
}

// Schedule:
// schedule the thread according to the priority
// if no thread is active, schedule the IO manager thread
inline void Schedule( DzHost* host )
{
    do{
        if( host->taskLs[ host->currPriority ].head ){
            DzThread* dzThread =
                MEMBER_BASE( host->taskLs[ host->currPriority ].head, DzThread, lItr );
            EraseListHead( &host->taskLs[ host->currPriority ] );
            SwitchToCot( host, dzThread );
            return;
        }else{
            host->currPriority++;
        }
    }while( host->currPriority <= host->lowestPriority );
    if( host->currThread != &host->originThread ){
        SwitchToCot( host, &host->originThread );
    }
}

inline void DispatchCurrThread( DzHost* host )
{
    DispatchThread( host, host->currThread );
    Schedule( host );
}

#ifdef __cplusplus
};
#endif

#endif // __DzSchedule_h__
