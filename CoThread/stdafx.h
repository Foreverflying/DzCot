// stdafx.h : ��׼ϵͳ���ļ��İ��ļ���
// ���Ǿ���ʹ�õ�������ĵ�
// �ض�����Ŀ�İ��ļ�
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef _WIN32

#include <tchar.h>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")
#include <Windows.h>
#include <conio.h>
#include <commctrl.h>

#else

#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>

#define __stdcall __attribute__((stdcall))
#define _TCHAR char
#define _tmain main
#define _ttoi atoi
#define MAKEIPADDRESS( a1, a2, a3, a4 ) ( (a1 << 24) | (a2 << 16) | (a3 << 8) | a4 )

typedef long long __int64;
typedef unsigned char byte;

inline int GetTickCount()
{
	struct timeval current;
	gettimeofday(&current, NULL);
	return current.tv_sec * 1000 + current.tv_usec/1000;
}

#endif



// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
