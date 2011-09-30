/********************************************************************
    created:    2010/02/11 22:13
    file:       DzcotData.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzcotData_h__
#define __DzcotData_h__

#if defined(_WIN32)
#   ifdef DZCOTDATA_EXPORTS
#       define DZCOTDATA_API __declspec(dllexport)
#   else
#       define DZCOTDATA_API __declspec(dllimport)
#       pragma comment( lib, "DzcotData.lib" )
#   endif
#endif

//#define DZCOTDATA_API

#ifdef __cplusplus
extern "C"{
#endif

#ifdef _WIN32
#include <WinSock2.h>
#include <windows.h>
#include <MSWSock.h>

extern DZCOTDATA_API DWORD tlsIndex;
extern DZCOTDATA_API LPFN_ACCEPTEX _AcceptEx;
extern DZCOTDATA_API LPFN_CONNECTEX _ConnectEx;
extern DZCOTDATA_API LPFN_GETACCEPTEXSOCKADDRS _GetAcceptExSockAddrs;

// extern DWORD tlsIndex;
// extern LPFN_ACCEPTEX _AcceptEx;
// extern LPFN_CONNECTEX _ConnectEx;
// extern LPFN_GETACCEPTEXSOCKADDRS _GetAcceptExSockAddrs;

#elif defined __linux__
#include <pthread.h>

extern pthread_key_t tlsIndex;

#endif


#ifdef __cplusplus
};
#endif

#endif // __DzcotData_h__
