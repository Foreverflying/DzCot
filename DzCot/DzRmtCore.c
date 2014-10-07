/**
 *  @file       DzRmtCore.c
 *  @brief      
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2011/10/13
 *
 */

#include "DzRmtCore.h"
#include "DzCore.h"

void __stdcall PauseCotHelpEntry( intptr_t context )
{
    DzHost* host = GetHost();
    DzCot* cot = host->currCot;
    DzCot* dzCot = (DzCot*)context;

    if( cot->feedType ){
        SendRmtCot( host, cot->hostId, cot->feedType < 0, dzCot );
    }else{
        AddLItrToTail( &host->lazyRmtCot[ cot->hostId ], &dzCot->lItr );
        if( !host->lazyTimer ){
            StartLazyTimer( host );
        }
    }
}

inline void MoveCurCotToRmt( DzHost* host, int rmtId, int feedType )
{
    DzCot* helpCot;

    helpCot = AllocDzCot( host, ST_FIRST );
    SetCotEntry( helpCot, PauseCotHelpEntry, (intptr_t)host->currCot );
    helpCot->hostId = rmtId;
    helpCot->feedType = feedType;
    SwitchToCot( host, helpCot );
}

void __stdcall RemoteCotEntry( intptr_t context )
{
    DzHost* host = GetHost();
    DzCot* dzCot = host->currCot;

    dzCot->entry( context );
    MoveCurCotToRmt( host, dzCot->hostId, dzCot->feedType );
    host = GetHost();
    host->scheduleCd++;
    if( dzCot->feedType > 0 ){
        if( dzCot->evtType == 0 ){
            SetEvt( host, dzCot->evt );
            DelSynObj( host, dzCot->evt );
        }else{
            NotifyEasyEvt( host, dzCot->easyEvt );
        }
    }
}

void __stdcall LazyFreeMemEntry( intptr_t context )
{
    int rmtId;
    DzLNode* node;
    DzLItr* tail;
    DzHost* host;

    host = GetHost();
    host->scheduleCd++;
    node = MEMBER_BASE( (DzLItr*)context, DzLNode, lItr );
    tail = (DzLItr*)node->d2;
    rmtId = (int)node->d3;
    do{
        Free( host, (void*)node->d1 );
        node = MEMBER_BASE( node->lItr.next, DzLNode, lItr );
    }while( node );
    MoveCurCotToRmt( host, rmtId, 1 );
    host = GetHost();
    host->scheduleCd++;
    FreeChainLNode( host, (DzLItr*)context, tail );
}

void __stdcall DealLazyResEntry( intptr_t context )
{
    int i;
    DzLItr* lItr;
    DzLItr* tail;
    DzCot* dzCot;
    DzHost* host;
    DzLNode* node;

    host = GetHost();
    for( i = 0; i < host->hostCount; i++ ){
        if( !IsSListEmpty( host->lazyRmtCot + i ) ){
            tail = host->lazyRmtCot[i].tail;
            lItr = GetChainAndResetSList( host->lazyRmtCot + i );
            dzCot = MEMBER_BASE( lItr, DzCot, lItr );
            dzCot->priority -= CP_DEFAULT + 1;
            if( SendRmtCot( host, i, FALSE, dzCot ) == FALSE ){
                dzCot->priority += CP_DEFAULT + 1;
                host->pendRmtCot[i].tail = tail;
            }
        }
        if( !IsSListEmpty( host->lazyFreeMem + i ) ){
            tail = host->lazyFreeMem[i].tail;
            lItr = GetChainAndResetSList( host->lazyFreeMem + i );
            node = MEMBER_BASE( lItr, DzLNode, lItr );
            node->d2 = (intptr_t)tail;
            node->d3 = (intptr_t)host->hostId;
            dzCot = AllocDzCot( host, ST_FIRST );
            dzCot->priority = CP_FIRST;
            SetCotEntry( dzCot, LazyFreeMemEntry, (intptr_t)node );
            SendRmtCot( host, i, FALSE, dzCot );
        }
    }
    StopLazyTimer( host );
}

void __stdcall RmtHostFirstEntry( intptr_t context )
{
    int i;
    DzHost* host;
    DzSysParam* param;
    DzRmtCotFifo* fifo;
    DzCot** cotResBase;
    
    host = GetHost();
    cotResBase = host->mgr->rmtFifoCotArrRes;
    cotResBase += host->hostCount * host->hostId * RMT_CALL_FIFO_SIZE;
    for( i = 0; i < host->hostCount; i++ ){
        host->rmtFifoArr[i].rmtCotArr = cotResBase + i * RMT_CALL_FIFO_SIZE;
    }
    param = (DzSysParam*)context;
    param->result = DS_OK;
    fifo = host->mgr->hostArr[0]->rmtFifoArr + host->hostId;
    fifo->rmtCotArr[0] = param->hs.returnCot;
    NotifyRmtFifo(
        host->mgr,
        host->mgr->hostArr[0],
        host->hostMask,
        fifo->writePos,
        0
        );
}

void __stdcall RunRmtHostMain( intptr_t context )
{
    int ret;
    DzSysParam* param;
    DzRmtCotFifo* fifo;

    param = (DzSysParam*)context;
    ret = RunHost(
        param->hs.hostMgr, param->hs.hostId, RmtHostFirstEntry, context, NULL
        );
    if( ret != DS_OK ){
        param->result = ret;
        fifo = param->hs.hostMgr->hostArr[0]->rmtFifoArr + param->hs.hostId;
        fifo->rmtCotArr[0] = param->hs.returnCot;
        NotifyRmtFifo(
            param->hs.hostMgr,
            param->hs.hostMgr->hostArr[0],
            1 << param->hs.hostId,
            fifo->writePos,
            0
            );
    }
}

void __stdcall StartRmtHostRetEntry( intptr_t context )
{
    DzHost* host = GetHost();
    DzSysParam* param = (DzSysParam*)context;

    SetEvt( host, param->hs.evt );
}

void __stdcall MainHostFirstEntry( intptr_t context )
{
    int i, j, hostCount;
    DzHost* host;
    DzHost** hostArr;
    DzSysParam* cotParam;
    DzSysParam param[ DZ_MAX_HOST ];
    DzSynObj* evt;
    DzCot* dzCot;
    DzCot** cotResBase;

    host = GetHost();
    hostCount = host->hostCount;
    i = sizeof( DzCot* ) * RMT_CALL_FIFO_SIZE * hostCount * hostCount;
    i = ( i + PAGE_SIZE - 1 ) & ~( PAGE_SIZE - 1 );
    cotResBase = (DzCot**)AllocChunk( host, i );
    for( i = 0; i < host->hostCount; i++ ){
        host->rmtFifoArr[i].rmtCotArr = cotResBase + i * RMT_CALL_FIFO_SIZE;
    }
    host->mgr->rmtFifoCotArrRes = cotResBase;
    evt = CreateCdEvt( host, host->hostCount - 1 );
    cotParam = (DzSysParam*)context;
    for( i = 1; i < host->hostCount; i++ ){
        dzCot = AllocDzCot( host, ST_FIRST );
        dzCot->priority = CP_FIRST;
        SetCotEntry( dzCot, StartRmtHostRetEntry, (intptr_t)&param[i] );
        param[i].threadEntry = RunRmtHostMain;
        param[i].hs.evt = evt;
        param[i].hs.hostMgr = host->mgr;
        param[i].hs.returnCot = dzCot;
        param[i].hs.hostId = i;
        StartSystemThread( param + i, THREAD_STACK_MIN );
    }
    WaitSynObj( host, evt, -1 );
    DelSynObj( host, evt );
    for( i = 1; i < host->hostCount; i++ ){
        if( param[i].result != DS_OK ){
            cotParam->result = param[i].result;
            return;
        }
    }
    hostArr = host->mgr->hostArr;
    for( i = 0; i < hostCount; i++ ){
        for( j = 0; j < hostCount; j++ ){
            hostArr[i]->rmtFifoArr[j].pendRmtCot = hostArr[j]->pendRmtCot + i;
        }
    }
    cotParam->result = StartCot(
        host, cotParam->cs.entry, cotParam->cs.context,
        host->dftPri, host->dftSType
        );
}

void __stdcall WaitFifoWritableEntry( intptr_t context )
{
    int rmtId;
    DzCot* dzCot;
    DzLItr* lItr;
    DzHost* host;

    host = GetHost();
    host->scheduleCd++;
    rmtId = (int)context;
    if( rmtId == host->hostId ){
        host->rmtFifoArr[ rmtId ].rmtCotArr[0] = NULL;
        InitSList( host->pendRmtCot + rmtId );
        NotifySysAutoEvt( host->mgr->sysAutoEvt + rmtId );
        return;
    }
    MoveCurCotToRmt( host, rmtId, -1 );
    host->rmtFifoArr[ rmtId ].rmtCotArr[0] = NULL;
    rmtId = host->hostId;
    host = GetHost();
    host->scheduleCd++;
    lItr = GetChainAndResetSList( &host->pendRmtCot[ rmtId ] );
    dzCot = MEMBER_BASE( lItr, DzCot, lItr );
    dzCot->priority -= CP_DEFAULT + 1;
    SendRmtCot( host, rmtId, TRUE, dzCot );
}

DzCot* CreateWaitFifoCot( DzHost* host )
{
    DzCot* ret;

    ret = AllocDzCot( host, ST_FIRST );
    ret->priority = CP_FIRST;
    SetCotEntry( ret, WaitFifoWritableEntry, (intptr_t)host->hostId );
    return ret;
}

void __stdcall WorkerMain( intptr_t context )
{
    DzHostsMgr* hostMgr;
    DzSysParam* param;
    DzWorker worker;
    DzWorker volatile* wk;
    DzHost* host;
    DzCot* dzCot;
    int rmtId;
    int nowDepth;
    BOOL eventInit;

    eventInit = FALSE;
    param = (DzSysParam*)context;
    hostMgr = param->wk.hostMgr;
    worker.entry = param->wk.entry;
    worker.context = param->wk.context;
    worker.dzCot = param->wk.dzCot;
    wk = &worker;
    while( 1 ){
        dzCot = wk->dzCot;
        if( !dzCot ){
            break;
        }
        rmtId = dzCot->hostId;
        wk->entry( wk->context );
        host = hostMgr->hostArr[ rmtId ];
        do{
            WaitSysAutoEvt( hostMgr->sysAutoEvt + rmtId );
        }while( SendRmtCot( host, rmtId, FALSE, dzCot ) == FALSE );
        NotifySysAutoEvt( hostMgr->sysAutoEvt + rmtId );

        nowDepth = AtomDecInt( &hostMgr->workerNowDepth );
        if( nowDepth <= 0 ){
            nowDepth = AtomIncInt( &hostMgr->workerNowDepth );
            break;
        }else{
            if( !eventInit ){
                eventInit = TRUE;
                InitSysAutoEvt( &worker.sysEvt );
            }
            AtomPushStack( &hostMgr->workerPool, &worker.lItr );
            WaitSysAutoEvt( &worker.sysEvt );
        }
    }
    if( eventInit ){
        FreeSysAutoEvt( &worker.sysEvt );
    }
}