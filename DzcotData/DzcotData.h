
#ifndef _DZCOT_DATA_H_
#define _DZCOT_DATA_H_

typedef __int64 int64;

#include <WinSock2.h>
#include <windows.h>
#include <MSWSock.h>
#include "../Dzcot/DzType.h"
#include "../Dzcot/DzStructs.h"

#ifdef DZCOTDATA_EXPORTS
#define DZCOTDATA_API __declspec(dllexport)
#else
#define DZCOTDATA_API __declspec(dllimport)
#pragma comment( lib, "DzcotData.lib" )
#endif

#ifdef __cplusplus
extern "C"{
#endif

extern DZCOTDATA_API DWORD tlsIndex;
extern DZCOTDATA_API LONG tlsLock;
extern DZCOTDATA_API LONG sockInitCount;
extern DZCOTDATA_API LONG sockInitLock;
extern DZCOTDATA_API BOOL isSocketStarted;
extern DZCOTDATA_API LPFN_ACCEPTEX _AcceptEx;
extern DZCOTDATA_API LPFN_CONNECTEX _ConnectEx;
extern DZCOTDATA_API LPFN_GETACCEPTEXSOCKADDRS _GetAcceptExSockAddrs;

//void __stdcall DzInitCot( DzHost *host, DzThread *dzThread );
//void __stdcall DzSwitch( DzHost *host, DzThread *dzThread );

#ifdef __cplusplus
};
#endif

#endif
