
#ifndef _DZ_TIMER_H_
#define _DZ_TIMER_H_

#include "DzStructs.h"

#define INIT_TIME_HEAP_SIZE     512
#define MIN_TIME_INTERVAL       5

void NotifyTimerNode( DzHost *host, DzTimerNode *timerNode );

inline int64 MilUnixTime()
{
    struct timeb t;
    int64 ret;

    ftime( &t );
    ret = t.time;
    ret *= 1000;
    ret += t.millitm;
    return ret;
}

inline int64 UnixTime()
{
    struct timeb t;

    ftime( &t );
    return (int64)t.time;
}

inline BOOL LessThanNode( DzTimerNode *left, DzTimerNode *right )
{
    return left->timestamp < right->timestamp;
}

inline void SetTimerNode( DzTimerNode **timeHeap, int index, DzTimerNode *timerNode )
{
    timeHeap[ index ] = timerNode;
    timerNode->index = index;
}

inline int LeftNodeIdx( int index )
{
    return index * 2 + 1;
}

inline int RightNodeIdx( int index )
{
    return index * 2 + 2;
}

inline int ParentNodeIdx( int index )
{
    return ( index - 1 ) / 2;
}

inline void ShiftDownNode( DzHost *host, int index, DzTimerNode *timerNode )
{
    DzTimerNode *node;
    DzTimerNode *right;
    int tmpIndex;

    while( index < host->timerCount / 2 ){
        node = host->timerHeap[ LeftNodeIdx( index ) ];
        if( RightNodeIdx( index ) < host->timerCount ){
            right = host->timerHeap[ RightNodeIdx( index ) ];
            if( LessThanNode( right, node ) ){
                node = right;
            }
        }
        if( LessThanNode( node, timerNode ) ){
            tmpIndex = node->index;
            SetTimerNode( host->timerHeap, index, node );
            index = tmpIndex;
        }else{
            break;
        }
    }
    SetTimerNode( host->timerHeap, index, timerNode );
}

inline void AddTimer( DzHost *host, DzTimerNode *timerNode )
{
    int curr;

    curr = host->timerCount++;
    if( host->timerCount > host->timerHeapSize ){
        if( host->timerHeapSize ){
            host->timerHeapSize *= 2;
        }else{
            host->timerHeapSize = INIT_TIME_HEAP_SIZE;
        }
        host->timerHeap = (DzTimerNode**)realloc(
            host->timerHeap,
            sizeof(DzTimerNode*) * host->timerHeapSize
            );
    }
    while( curr != 0 && LessThanNode(  timerNode, host->timerHeap[ ParentNodeIdx( curr ) ] ) ){
        SetTimerNode( host->timerHeap, curr, host->timerHeap[ ParentNodeIdx( curr ) ] );
        curr = ParentNodeIdx( curr );
    }
    SetTimerNode( host->timerHeap, curr, timerNode );
}

inline BOOL IsTimeNodeInHeap( DzTimerNode *timeNode )
{
    return timeNode->index >= 0;
}

inline void RemoveTimer( DzHost *host, DzTimerNode *timerNode )
{
    int curr;

    host->timerCount--;
    curr = timerNode->index;
    timerNode->index = -1;
    if( curr == host->timerCount ){
        return;
    }
    timerNode = host->timerHeap[ host->timerCount ];
    if( curr != 0 && LessThanNode( timerNode, host->timerHeap[ ParentNodeIdx( curr ) ] ) ){
        do{
            SetTimerNode( host->timerHeap, curr, host->timerHeap[ ParentNodeIdx( curr ) ] );
            curr = ParentNodeIdx( curr );
        }while( curr != 0 && LessThanNode( timerNode, host->timerHeap[ ParentNodeIdx( curr ) ] ) );
        SetTimerNode( host->timerHeap, curr, timerNode );
    }else{
        ShiftDownNode( host, curr, timerNode );
    }
}

inline void RemoveMinTimer( DzHost *host )
{
    DzTimerNode *timerNode;

    host->timerHeap[0]->index = -1;
    host->timerCount--;
    if( !host->timerCount ){
        return;
    }
    timerNode = host->timerHeap[ host->timerCount ];
    ShiftDownNode( host, 0, timerNode );
}

inline void AdjustMinTimer( DzHost *host, DzTimerNode *timerNode )
{
    ShiftDownNode( host, 0, timerNode );
}

inline DzTimerNode* GetMinTimerNode( DzHost *host )
{
    return host->timerHeap[0];
}

inline BOOL NotifyMinTimers( DzHost *host, int *timeOut )
{
    DzTimerNode *timerNode;
    int64 currTime;
    int64 cmpTime;
    BOOL ret = FALSE;

    currTime = MilUnixTime();
    cmpTime = currTime + MIN_TIME_INTERVAL;
    while( host->timerCount > 0 && GetMinTimerNode( host )->timestamp <= cmpTime ){
        ret = TRUE;
        timerNode = GetMinTimerNode( host );
        if( timerNode->repeat != 1 ){
            if( timerNode->repeat ){
                timerNode->repeat--;
            }
            timerNode->timestamp -= timerNode->interval;   //interval is negative
            AdjustMinTimer( host, timerNode );
            NotifyTimerNode( host, timerNode );
        }else{
            RemoveMinTimer( host );
            NotifyTimerNode( host, timerNode );
        }
    }
    if( ret ){
        return TRUE;
    }
    if( timeOut ){
        *timeOut = host->timerCount > 0 ? (int)( GetMinTimerNode( host )->timestamp - currTime ) : INFINITE;
    }
    return FALSE;
}

#endif      //#ifndef _DZ_TIMER_H_
