/********************************************************************
    created:    2011/10/13 14:42
    file:       DzRmtCore.c
    author:     Foreverflying
    purpose:    
*********************************************************************/

#include "DzRmtCore.h"
#include "DzCore.h"

void __stdcall PauseCotHelpEntry( intptr_t context )
{
    DzHost* host = GetHost();
    DzLNode* node = (DzLNode*)context;
    int rmtId = (int)node->d2;
    DzCot* dzCot = (DzCot*)node->d1;

    if( node->d3 ){
        SendRmtCot( host, rmtId, node->d3 < 0, dzCot );
    }else{
        AddLItrToTail( &host->lazyRmtCot[ rmtId ], &dzCot->lItr );
        if( !host->lazyTimer ){
            StartLazyTimer( host );
        }
    }
}

inline void MoveCurCotToRmt( DzHost* host, int rmtId, int type )
{
    DzCot* helpCot;
    DzLNode node;

    node.d1 = (intptr_t)host->currCot;
    node.d2 = (intptr_t)rmtId;
    node.d3 = (intptr_t)type;
    helpCot = AllocDzCot( host, SS_FIRST );
    SetCotEntry( helpCot, PauseCotHelpEntry, (intptr_t)&node );
    SwitchToCot( host, helpCot );
}

void __stdcall RemoteCotEntry( intptr_t context )
{
    DzHost* host = GetHost();
    DzCotParam* param = (DzCotParam*)context;

    param->entry( param->context );
    host->currCot->priority = CP_DEFAULT;
    MoveCurCotToRmt( host, param->hostId, param->type );
    host = GetHost();
    if( param->type > 0 ){
        if( param->evtType == 0 ){
            SetEvt( host, param->evt );
            CloseSynObj( host, param->evt );
        }else{
            NotifyEasyEvt( host, param->easyEvt );
        }
    }
    FreeLNode( host, (DzLNode*)param );
}

void __stdcall LazyFreeMemEntry( intptr_t context )
{
    int rmtId;
    DzLNode* node;
    DzLItr* tail;
    DzHost* host;

    host = GetHost();
    node = MEMBER_BASE( (DzLItr*)context, DzLNode, lItr );
    tail = (DzLItr*)node->d2;
    rmtId = (int)node->d3;
    do{
        Free( host, (void*)node->d1 );
        node = MEMBER_BASE( node->lItr.next, DzLNode, lItr );
    }while( node );
    host->currCot->priority = CP_DEFAULT;
    MoveCurCotToRmt( host, rmtId, 1 );
    host = GetHost();
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
            SendRmtCot( host, i, FALSE, dzCot );
            if( !IsSListEmpty( &host->pendRmtCot[i] ) ){
                host->pendRmtCot[i].tail = tail;
            }
        }
        if( !IsSListEmpty( host->lazyFreeMem + i ) ){
            tail = host->lazyFreeMem[i].tail;
            lItr = GetChainAndResetSList( host->lazyFreeMem + i );
            node = MEMBER_BASE( lItr, DzLNode, lItr );
            node->d2 = (intptr_t)tail;
            node->d3 = (intptr_t)host->hostId;
            dzCot = AllocDzCot( host, SS_FIRST );
            dzCot->priority = CP_DEFAULT;
            SetCotEntry( dzCot, LazyFreeMemEntry, (intptr_t)node );
            SendRmtCot( host, i, FALSE, dzCot );
        }
    }
    StopLazyTimer( host );
}

void __stdcall RemoteHostEntry( intptr_t context )
{
    int i;
    DzHost* host;
    DzRmtCotFifo* fifo;
    DzSysParam* param;
    
    host = GetHost();
    for( i = 0; i < host->hostCount; i++ ){
        if( host->servMask & ( 1 << i ) ){
            if( !host->checkFifo ){
                host->checkFifo = host->rmtFifoArr + i;
                fifo = host->checkFifo;
            }else{
                fifo->next = host->rmtFifoArr + i;
                fifo = fifo->next;
            }
            fifo->rmtCotArr = (DzCot**)
                AllocChunk( host, sizeof( DzCot* ) * RMT_CALL_FIFO_SIZE );
        }
    }
    if( host->checkFifo ){
        fifo->next = host->checkFifo;
    }

    param = (DzSysParam*)context;
    param->result = DS_OK;
    fifo = host->mgr->hostArr[0]->rmtFifoArr + host->hostId;
    fifo->rmtCotArr[0] = param->hs.returnCot;
    NotifyRmtFifo( host->mgr->hostArr[0], fifo->writePos );
}

void __stdcall RunRemoteHost( intptr_t context )
{
    int ret;
    DzSysParam* param;
    DzRmtCotFifo* fifo;

    param = (DzSysParam*)context;
    ret = RunHost(
        param->hs.hostMgr, param->hs.hostId, param->hs.lowestPri,
        param->hs.dftPri, param->hs.dftSSize, RemoteHostEntry, context
        );
    if( ret != DS_OK ){
        param->result = ret;
        fifo = param->hs.hostMgr->hostArr[0]->rmtFifoArr + param->hs.hostId;
        fifo->rmtCotArr[0] = param->hs.returnCot;
        NotifyRmtFifo( param->hs.hostMgr->hostArr[0], fifo->writePos );
    }
}

void __stdcall SysThreadReturnCot( intptr_t context )
{
    DzHost* host = GetHost();
    DzSysParam* param = (DzSysParam*)context;

    SetEvt( host, param->hs.evt );
}

void __stdcall MainHostEntry( intptr_t context )
{
    int i;
    DzHost* host;
    DzSysParam* cotParam;
    DzSysParam param[ DZ_MAX_HOST ];
    DzCot* tmpCotArr[ DZ_MAX_HOST ];
    DzSynObj* evt;
    DzRmtCotFifo* fifo;
    DzCot* dzCot;

    host = GetHost();
    host->checkFifo = host->rmtFifoArr;
    for( i = 0; i < host->hostCount; i++ ){
        host->rmtFifoArr[i].rmtCotArr = tmpCotArr + i;
        host->rmtFifoArr[i].next = host->rmtFifoArr + i + 1;
    }
    host->rmtFifoArr[ i - 1 ].next = host->rmtFifoArr;
    evt = CreateCdEvt( host, host->hostCount - 1 );
    cotParam = (DzSysParam*)context;
    for( i = 1; i < host->hostCount; i++ ){
        dzCot = AllocDzCot( host, SS_FIRST );
        dzCot->priority = CP_DEFAULT;
        SetCotEntry( dzCot, SysThreadReturnCot, (intptr_t)&param[i] );
        param[i].threadEntry = RunRemoteHost;
        param[i].hs.evt = evt;
        param[i].hs.hostMgr = host->mgr;
        param[i].hs.returnCot = dzCot;
        param[i].hs.hostId = i;
        param[i].hs.lowestPri = host->lowestPri;
        param[i].hs.dftPri = host->dftPri;
        param[i].hs.dftSSize = host->dftSSize;
        StartSystemThread( param + i );
    }

    WaitSynObj( host, evt, -1 );
    CloseSynObj( host, evt );
    for( i = 1; i < host->hostCount; i++ ){
        if( param[i].result != DS_OK ){
            cotParam->result = param[i].result;
            return;
        }
    }
    host->checkFifo = NULL;
    for( i = 0; i < host->hostCount; i++ ){
        *host->rmtFifoArr[i].readPos = 0;
        *host->rmtFifoArr[i].writePos = 0;
        host->rmtFifoArr[i].rmtCotArr = NULL;
        if( host->servMask & ( 1 << i ) ){
            if( !host->checkFifo ){
                host->checkFifo = host->rmtFifoArr + i;
                fifo = host->checkFifo;
            }else{
                fifo->next = host->rmtFifoArr + i;
                fifo = fifo->next;
            }
            fifo->rmtCotArr = (DzCot**)
                AllocChunk( host, sizeof( DzCot* ) * RMT_CALL_FIFO_SIZE );
        }
    }
    if( host->checkFifo ){
        fifo->next = host->checkFifo;
    }
    cotParam->result = StartCot(
        host, cotParam->cs.entry, cotParam->cs.context,
        host->dftPri, host->dftSSize
        );
}

void __stdcall WaitFifoWritableEntry( intptr_t context )
{
    int rmtId;
    DzCot* dzCot;
    DzLItr* lItr;
    DzHost* host;

    host = GetHost();
    rmtId = (int)context;
    MoveCurCotToRmt( host, rmtId, -1 );
    host = GetHost();
    lItr = GetChainAndResetSList( &host->pendRmtCot[ rmtId ] );
    dzCot = MEMBER_BASE( lItr, DzCot, lItr );
    dzCot->priority -= CP_DEFAULT + 1;
    SendRmtCot( host, rmtId, FALSE, dzCot );
}

DzCot* CreateWaitFifoCot( DzHost* host )
{
    DzCot* ret;

    ret = AllocDzCot( host, SS_FIRST );
    ret->priority = CP_DEFAULT;
    SetCotEntry( ret, WaitFifoWritableEntry, (intptr_t)host->hostId );
    return ret;
}
