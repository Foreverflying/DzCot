/********************************************************************
    created:    2010/02/11 22:02
    file:       DzIncWin.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzIncWin_h__
#define __DzIncWin_h__

// 以下宏定义要求的最低平台。要求的最低平台
// 是具有运行应用程序所需功能的 Windows、Internet Explorer 等产品的
// 最早版本。通过在指定版本及更低版本的平台上启用所有可用的功能，宏可以
// 正常工作。

// 如果必须要针对低于以下指定版本的平台，请修改下列定义。
// 有关不同平台对应值的最新信息，请参考 MSDN。
#ifndef WINVER                          // 指定要求的最低平台是 Windows xp。
#define WINVER 0x0501           // 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif

#ifndef _WIN32_WINNT            // 指定要求的最低平台是 Windows xp。
#define _WIN32_WINNT 0x0501     // 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <sys/timeb.h>
#pragma comment(lib, "Ws2_32.lib")

// Windows 头文件:
#include <windows.h>

typedef long long int64;

#ifndef __cplusplus
#define inline __inline
#endif

#endif // __DzIncWin_h__
