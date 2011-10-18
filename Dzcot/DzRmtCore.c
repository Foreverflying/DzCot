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
    DzRmtCotParam* param = (DzRmtCotParam*)context;

    DispatchRmtCot( host, (int)param->hostId, param->type < 0, param->cot );
}

inline void MoveCurCotNative( DzHost* host, DzRmtCotParam* param )
{
    DzCot* helpCot;

    param->cot = host->currCot;
    param->cot->priority = CP_DEFAULT;
    helpCot = AllocDzCot( host, SS_FIRST );
    SetCotEntry( helpCot, PauseCotHelpEntry, (intptr_t)param );
    SwitchToCot( host, helpCot );
}

void __stdcall RemoteCotEntry( intptr_t context )
{
    DzHost* host = GetHost();
    DzRmtCotParam* param = (DzRmtCotParam*)context;

    param->entry( param->context );
    MoveCurCotNative( host, param );
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

void __stdcall RemoteHostEntry( intptr_t context )
{
    int i;
    DzHost* host;
    DzRmtCotFifo* fifo;
    DzSysParam* param;
    
    host = GetHost();
    for( i = 0; i < host->hostMgr->hostCount; i++ ){
        if( host->hostMgr->servMask[ host->hostId ] & ( 1 << i ) ){
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
    fifo = host->hostMgr->hostArr[0]->rmtFifoArr + host->hostId;
    fifo->rmtCotArr[0] = param->hostStart.returnCot;
    NotifyRmtFifo( host->hostMgr->hostArr[0], &fifo->writePos );
}

void __stdcall RunRemoteHost( intptr_t context )
{
    int ret;
    DzSysParam* param;
    DzRmtCotFifo* fifo;

    param = (DzSysParam*)context;
    ret = RunHost(
        param->hostStart.hostMgr, param->hostStart.hostId,
        param->hostStart.lowestPri, param->hostStart.dftPri,
        param->hostStart.dftSSize, RemoteHostEntry, context
        );
    if( ret != DS_OK ){
        param->result = ret;
        fifo = param->hostStart.hostMgr->hostArr[0]->rmtFifoArr + param->hostStart.hostId;
        fifo->rmtCotArr[0] = param->hostStart.returnCot;
        NotifyRmtFifo( param->hostStart.hostMgr->hostArr[0], &fifo->writePos );
    }
}

void __stdcall SysThreadReturnCot( intptr_t context )
{
    DzHost* host = GetHost();
    DzSysParam* param = (DzSysParam*)context;

    SetEvt( host, param->hostStart.evt );
}

void __stdcall MainHostEntry( intptr_t context )
{
    int i;
    DzHost* host;
    DzSysParam* cotParam;
    DzSysParam param[ DZ_MAX_HOST ];
    DzCot* tmpCallPkgArr[ DZ_MAX_HOST ];
    DzSynObj* evt;
    DzRmtCotFifo* fifo;
    DzCot* dzCot;

    host = GetHost();
    host->checkFifo = host->rmtFifoArr;
    for( i = 0; i < host->hostMgr->hostCount; i++ ){
        host->rmtFifoArr[i].rmtCotArr = tmpCallPkgArr + i;
        host->rmtFifoArr[i].next = host->rmtFifoArr + i + 1;
    }
    host->rmtFifoArr[ i - 1 ].next = host->rmtFifoArr;
    evt = CreateCdEvt( host, host->hostMgr->hostCount - 1 );
    cotParam = (DzSysParam*)context;
    for( i = 1; i < host->hostMgr->hostCount; i++ ){
        dzCot = AllocDzCot( host, SS_FIRST );
        dzCot->priority = CP_DEFAULT;
        SetCotEntry( dzCot, SysThreadReturnCot, (intptr_t)&param[i] );
        param[i].threadEntry = RunRemoteHost;
        param[i].hostStart.evt = evt;
        param[i].hostStart.hostMgr = host->hostMgr;
        param[i].hostStart.returnCot = dzCot;
        param[i].hostStart.hostId = i;
        param[i].hostStart.lowestPri = host->lowestPri;
        param[i].hostStart.dftPri = host->dftPri;
        param[i].hostStart.dftSSize = host->dftSSize;
        StartSystemThread( param + i );
    }

    WaitSynObj( host, evt, -1 );
    CloseSynObj( host, evt );
    for( i = 1; i < host->hostMgr->hostCount; i++ ){
        if( param[i].result != DS_OK ){
            cotParam->result = param[i].result;
            return;
        }
    }
    host->checkFifo = NULL;
    for( i = 0; i < host->hostMgr->hostCount; i++ ){
        host->rmtFifoArr[i].readPos = 0;
        host->rmtFifoArr[i].writePos = 0;
        host->rmtFifoArr[i].rmtCotArr = NULL;
        if( host->hostMgr->servMask[ host->hostId ] & ( 1 << i ) ){
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
        host, cotParam->cotStart.entry, cotParam->cotStart.context,
        host->dftPri, host->dftSSize
        );
}

void __stdcall WaitFifoWritableEntry( intptr_t context )
{
    int rmtId;
    DzCot* dzCot;
    DzHost* host = GetHost();
    DzRmtCotParam* param = (DzRmtCotParam*)context;

    rmtId = host->hostId;
    MoveCurCotNative( host, param );
    host = GetHost();
    host->pendRmtCot[ rmtId ].tail->next = NULL;
    dzCot = MEMBER_BASE( host->pendRmtCot[ rmtId ].head, DzCot, lItr );
    dzCot->priority -= CP_DEFAULT + 1;
    InitSList( &host->pendRmtCot[ rmtId ] );
    DispatchRmtCot( host, rmtId, FALSE, dzCot );
    FreeLNode( host, (DzLNode*)param );
}

DzCot* CreateWaitFifoCot( DzHost* host )
{
    DzCot* ret;
    DzRmtCotParam* param;

    param = (DzRmtCotParam*)AllocLNode( host );
    param->type = -1;
    ret = AllocDzCot( host, SS_FIRST );
    ret->priority = CP_DEFAULT;
    SetCotEntry( ret, WaitFifoWritableEntry, (intptr_t)param );
    return ret;
}
