/********************************************************************
    created:    2010/02/11 22:02
    file:       DzIncWin.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzIncWin_h__
#define __DzIncWin_h__

// ���º궨��Ҫ������ƽ̨��Ҫ������ƽ̨
// �Ǿ�������Ӧ�ó������蹦�ܵ� Windows��Internet Explorer �Ȳ�Ʒ��
// ����汾��ͨ����ָ���汾�����Ͱ汾��ƽ̨���������п��õĹ��ܣ������
// ����������

// �������Ҫ��Ե�������ָ���汾��ƽ̨�����޸����ж��塣
// �йز�ͬƽ̨��Ӧֵ��������Ϣ����ο� MSDN��
#ifndef WINVER                          // ָ��Ҫ������ƽ̨�� Windows xp��
#define WINVER 0x0501           // ����ֵ����Ϊ��Ӧ��ֵ���������� Windows �������汾��
#endif

#ifndef _WIN32_WINNT            // ָ��Ҫ������ƽ̨�� Windows xp��
#define _WIN32_WINNT 0x0501     // ����ֵ����Ϊ��Ӧ��ֵ���������� Windows �������汾��
#endif

#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ���ų�����ʹ�õ�����

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <sys/timeb.h>
#pragma comment(lib, "Ws2_32.lib")

// Windows ͷ�ļ�:
#include <windows.h>

typedef long long int64;

#ifndef __cplusplus
#define inline __inline
#endif

#endif // __DzIncWin_h__
