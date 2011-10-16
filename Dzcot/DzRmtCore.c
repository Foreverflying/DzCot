/********************************************************************
    created:    2011/10/13 14:42
    file:       DzRmtCore.c
    author:     Foreverflying
    purpose:    
*********************************************************************/

#include "DzRmtCore.h"
#include "DzCore.h"

void __stdcall RemoteHostEntry( intptr_t context )
{
    int i;
    DzHost* host;
    DzRmtCallPkg* pkg;
    DzRmtCallFifo* fifo;
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
            fifo->callPkgArr = (DzRmtCallPkg*)
                AllocChunk( host, sizeof( DzRmtCallPkg ) * RMT_CALL_FIFO_SIZE );
        }
    }
    if( host->checkFifo ){
        fifo->next = host->checkFifo;
    }

    param = (DzSysParam*)context;
    param->result = DS_OK;
    fifo = host->hostMgr->hostArr[0]->rmtFifoArr + host->hostId;
    pkg = fifo->callPkgArr;
    pkg->priority = CP_FIRST;
    pkg->sSize = SS_FIRST;
    pkg->entry = NULL;
    pkg->evt = param->hostStart.evt;
    NotifyRmtCall( host->hostMgr->hostArr[0], &fifo->writePos, 1 );
}

void __stdcall RunRemoteHost( intptr_t context )
{
    int ret;
    DzSysParam* param;
    DzRmtCallPkg* pkg;
    DzRmtCallFifo* fifo;

    param = (DzSysParam*)context;
    ret = RunHost(
        param->hostStart.hostMgr, param->hostStart.hostId,
        param->hostStart.lowestPriority, param->hostStart.defaultPri,
        param->hostStart.defaultSSize, RemoteHostEntry, context
        );
    if( ret != DS_OK ){
        param->result = ret;
        fifo = param->hostStart.hostMgr->hostArr[0]->rmtFifoArr + param->hostStart.hostId;
        pkg = fifo->callPkgArr;
        pkg->priority = CP_FIRST;
        pkg->sSize = SS_FIRST;
        pkg->entry = NULL;
        pkg->evt = param->hostStart.evt;
        NotifyRmtCall( param->hostStart.hostMgr->hostArr[0], &fifo->writePos, 1 );
    }
}

void __stdcall MainHostEntry( intptr_t context )
{
    int i;
    DzHost* host;
    DzSysParam* cotParam;
    DzSysParam param[ DZ_MAX_HOST ];
    DzRmtCallPkg tmpCallPkgArr[ DZ_MAX_HOST ];
    DzSynObj* evt;
    DzRmtCallFifo* fifo;

    host = GetHost();
    host->checkFifo = host->rmtFifoArr;
    for( i = 0; i < host->hostMgr->hostCount; i++ ){
        host->rmtFifoArr[i].callPkgArr = tmpCallPkgArr + i;
        host->rmtFifoArr[i].next = host->rmtFifoArr + i + 1;
    }
    host->rmtFifoArr[ i - 1 ].next = host->rmtFifoArr;
    evt = CreateCountDownEvt( host, host->hostMgr->hostCount - 1 );
    cotParam = (DzSysParam*)context;
    for( i = 1; i < host->hostMgr->hostCount; i++ ){
        param[i].threadEntry = RunRemoteHost;
        param[i].hostStart.evt = evt;
        param[i].hostStart.hostMgr = host->hostMgr;
        param[i].hostStart.hostId = i;
        param[i].hostStart.lowestPriority = host->lowestPriority;
        param[i].hostStart.defaultPri = host->defaultPri;
        param[i].hostStart.defaultSSize = host->defaultSSize;
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
        host->rmtFifoArr[i].callPkgArr = NULL;
        if( host->hostMgr->servMask[ host->hostId ] & ( 1 << i ) ){
            if( !host->checkFifo ){
                host->checkFifo = host->rmtFifoArr + i;
                fifo = host->checkFifo;
            }else{
                fifo->next = host->rmtFifoArr + i;
                fifo = fifo->next;
            }
            fifo->callPkgArr = (DzRmtCallPkg*)
                AllocChunk( host, sizeof( DzRmtCallPkg ) * RMT_CALL_FIFO_SIZE );
        }
    }
    if( host->checkFifo ){
        fifo->next = host->checkFifo;
    }
    cotParam->result = StartCot(
        host, cotParam->cotStart.entry, cotParam->cotStart.context,
        CP_DEFAULT, SS_DEFAULT
        );
}

void __stdcall RecvFifoWritableEntry( intptr_t context )
{
    int empty;
    int addVal;
    int writePos;
    DzHost* host;
    DzHost* rmtHost;
    DzRmtCallFifo* fifo;
    DzRmtCallPkg* pkg;
    DzLNode* node;

    host = GetHost();
    rmtHost = host->hostMgr->hostArr[ context ];
    fifo = rmtHost->rmtFifoArr + host->hostId;
    writePos = fifo->writePos;
    empty = fifo->readPos - writePos;
    if( empty <= 0 ){
        empty += RMT_CALL_FIFO_SIZE;
    }
    empty -= 2;
    addVal = 0;
    do{
        node = MEMBER_BASE( host->pendingPkgs[ context ].head, DzLNode, lItr );
        EraseListHead( &host->pendingPkgs[ context ] );
        pkg = (DzRmtCallPkg*)node->content;
        CopyRmtCallPkg( pkg, fifo->callPkgArr + writePos );
        writePos++;
        if( writePos == RMT_CALL_FIFO_SIZE ){
            writePos = 0;
        }
        FreeLNode( host, (DzLNode*)pkg );
        FreeLNode( host, node );
        addVal++;
    }while( host->pendingPkgs[ context ].head && addVal < empty );
    if( addVal == empty && host->pendingPkgs[ context ].head ){
        pkg = fifo->callPkgArr + writePos;
        pkg->evt = 0;
        pkg->priority = CP_FIRST;
        pkg->sSize = SS_FIRST;
        pkg->entry = SendFifoWritableEntry;
        pkg->context = rmtHost->hostId;
        addVal++;
    }
    NotifyRmtCall( rmtHost, &fifo->writePos, addVal );
}

void __stdcall SendFifoWritableEntry( intptr_t context )
{
    DzHost* host;
    int rmtId = (int)context;
    DzRmtCallPkg pkg;

    host = GetHost();
    pkg.evt = 0;
    pkg.priority = CP_FIRST;
    pkg.sSize = SS_FIRST;
    pkg.entry = RecvFifoWritableEntry;
    pkg.context = host->hostId;
    SendRmtCall( host, rmtId, TRUE, &pkg );
}

void __stdcall FeedRmtCallEntry( intptr_t context )
{
    DzHost* host = GetHost();
    DzRmtCallPkg* pkg = (DzRmtCallPkg*)context;

    pkg->entry( pkg->context );
    pkg->entry = NULL;
    SendRmtCall( host, pkg->hostId, FALSE, pkg );
    FreeLNode( host, (DzLNode*)pkg );
}
