
#ifndef _DZ_SCHEDULE_H_
#define _DZ_SCHEDULE_H_

#include "DzQueue.h"
#include "DzCoreOs.h"

#ifdef __cplusplus
extern "C"{
#endif

inline BOOL UpdateCurrPriority( DzHost *host, int currPriority )
{
    if( currPriority > CP_INSTANT  ){
        if( host->taskQs[ CP_INSTANT ].head ){
            host->currPriority = CP_INSTANT;
            return TRUE;
        }
        host->currPriority = CP_HIGH;
    }
    return FALSE;
}

inline BOOL TrySetCurrPriority( DzHost *host, int priority )
{
    if( priority < host->currPriority ){
        host->currPriority = priority;
        return priority == CP_INSTANT;
    }
    return FALSE;
}

// DispatchThread:
// the dzThread is in the block queue or new created
// put the thread to the active queue
inline void DispatchThread( DzHost *host, DzThread *dzThread )
{
    DzQueue *queue = &host->taskQs[ dzThread->priority ];
    AddQItrToTail( queue, &dzThread->qItr );
}

inline void SwitchToCot( DzHost *host, DzThread *dzThread )
{
    DzSwitch( host, dzThread );
}

// Schedule:
// schedule the thread according to the priority
// if no thread is active, schedule the IO manager thread
inline void Schedule( DzHost *host )
{
    do{
        if( host->taskQs[ host->currPriority ].head ){
            DzThread *dzThread =
                MEMBER_BASE( host->taskQs[ host->currPriority ].head, DzThread, qItr );
            DelHeadQItr( &host->taskQs[ host->currPriority ] );
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

inline void DispatchCurrThread( DzHost *host )
{
    DispatchThread( host, host->currThread );
    Schedule( host );
}

#ifdef __cplusplus
};
#endif

#endif      //#ifndef _DZ_SCHEDULE_H_