/**
 *  @file       DzTimer.h
 *  @brief      
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2010/02/11
 *
 */

#ifndef __DzTimer_h__
#define __DzTimer_h__

#include "DzStructs.h"
#include "DzBase.h"

#ifdef __cplusplus
extern "C"{
#endif

void __stdcall CallbackTimerEntry( intptr_t context );

inline int64 MilUnixTime( DzHost* host )
{
    struct timeb t;

    ftime( &t );
    host->latestMilUnixTime = t.time;
    host->latestMilUnixTime *= 1000;
    host->latestMilUnixTime += t.millitm;
    return host->latestMilUnixTime;
}

inline int64 UnixTime()
{
    struct timeb t;

    ftime( &t );
    return (int64)t.time;
}

inline BOOL LessThanNode( DzTimerNode* left, DzTimerNode* right )
{
    return left->timestamp < right->timestamp;
}

inline void SetTimerNode( DzTimerNode** timeHeap, int index, DzTimerNode* timerNode )
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

inline void ShiftDownNode( DzHost* host, int index, DzTimerNode* timerNode )
{
    DzTimerNode* node;
    DzTimerNode* right;
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

inline void AddTimer( DzHost* host, DzTimerNode* timerNode )
{
    int curr;

    curr = host->timerCount++;
    if( host->timerCount > host->timerHeapSize ){
        PageCommit( host->timerHeap + host->timerHeapSize, PAGE_SIZE );
        host->timerHeapSize += PAGE_SIZE / sizeof(DzTimerNode*);
    }
    while( curr != 0 && LessThanNode( timerNode, host->timerHeap[ ParentNodeIdx( curr ) ] ) ){
        SetTimerNode( host->timerHeap, curr, host->timerHeap[ ParentNodeIdx( curr ) ] );
        curr = ParentNodeIdx( curr );
    }
    SetTimerNode( host->timerHeap, curr, timerNode );
}

inline BOOL IsTimeNodeInHeap( DzTimerNode* timeNode )
{
    return timeNode->index >= 0;
}

inline void RemoveTimer( DzHost* host, DzTimerNode* timerNode )
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

inline void RemoveMinTimer( DzHost* host )
{
    DzTimerNode* timerNode;

    host->timerHeap[0]->index = -1;
    host->timerCount--;
    if( host->timerCount ){
        timerNode = host->timerHeap[ host->timerCount ];
        ShiftDownNode( host, 0, timerNode );
    }
}

inline void AdjustMinTimer( DzHost* host, DzTimerNode* timerNode )
{
    ShiftDownNode( host, 0, timerNode );
}

inline DzTimerNode* GetMinTimerNode( DzHost* host )
{
    return host->timerHeap[0];
}

#ifdef __cplusplus
};
#endif

#endif // __DzTimer_h__
