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
    DzRmtCallPkg* pkg;
    DzRmtCallFifo* fifo;
    DzSysParam* param;
    
    param = (DzSysParam*)context;
    param->result = DS_OK;
    fifo = param->hostStart.hostMgr->hostArr[0]->rmtFifoArr + param->hostStart.hostId;
    pkg = fifo->callPkgArr;
    pkg->priority = CP_FIRST;
    pkg->sSize = SS_FIRST;
    pkg->entry = NULL;
    pkg->evt = param->hostStart.evt;
    NotifyRmtCall( param->hostStart.hostMgr->hostArr[0], &fifo->writePos, 1 );
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
    DzSynObj* evt;

    host = GetHost();
    evt = CreateCountDownEvt( host, host->hostsMgr->hostCount - 1 );
    cotParam = (DzSysParam*)context;
    for( i = 1; i < host->hostsMgr->hostCount; i++ ){
        param[i].threadEntry = RunRemoteHost;
        param[i].hostStart.evt = evt;
        param[i].hostStart.hostMgr = host->hostsMgr;
        param[i].hostStart.hostId = i;
        param[i].hostStart.lowestPriority = host->lowestPriority;
        param[i].hostStart.defaultPri = host->defaultPri;
        param[i].hostStart.defaultSSize = host->defaultSSize;
        StartSystemThread( param + i );
    }

    WaitSynObj( host, evt, -1 );
    CloseSynObj( host, evt );
    for( i = 1; i < host->hostsMgr->hostCount; i++ ){
        if( param[i].result != DS_OK ){
            cotParam->result = param[i].result;
            return;
        }
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
    rmtHost = host->hostsMgr->hostArr[ context ];
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
    DzHost* rmtHost;
    DzRmtCallPkg pkg;

    host = GetHost();
    rmtHost = host->hostsMgr->hostArr[ context ];
    pkg.evt = 0;
    pkg.priority = CP_FIRST;
    pkg.sSize = SS_FIRST;
    pkg.entry = RecvFifoWritableEntry;
    pkg.context = host->hostId;
    SendRmtCall( host, rmtHost, TRUE, &pkg );
}

void __stdcall FeedRmtCallEntry( intptr_t context )
{

}
