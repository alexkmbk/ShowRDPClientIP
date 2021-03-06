// ShowRDPClientIP.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
//#define _WINSOCK_DEPRECATED_NO_WARNINGS = 1

#include "pch.h"
#include <iostream>
#include <ws2tcpip.h>
#include <windows.h>
#include <strsafe.h>

#pragma comment(lib, "Ws2_32.lib")

#define LOGONID_CURRENT     ((ULONG)-1)

typedef struct {
	unsigned short sin_family;
	union {
		struct {
			USHORT sin_port;
			ULONG in_addr;
			UCHAR sin_zero[8];
		} ipv4;
		struct {
			USHORT sin6_port;
			ULONG sin6_flowinfo;
			USHORT sin6_addr[8];
			ULONG sin6_scope_id;
		} ipv6;
	};
} WINSTATIONREMOTEADDRESS,
*PWINSTATIONREMOTEADDRESS;

typedef enum _WINSTATIONINFOCLASS
{
	WinStationCreateData,
	WinStationConfiguration,
	WinStationPdParams,
	WinStationWd,
	WinStationPd,
	WinStationPrinter,
	WinStationClient,
	WinStationModules,
	WinStationInformation,
	WinStationTrace,
	WinStationBeep,
	WinStationEncryptionOff,
	WinStationEncryptionPerm,
	WinStationNtSecurity,
	WinStationUserToken,
	WinStationUnused1,
	WinStationVideoData,
	WinStationInitialProgram,
	WinStationCd,
	WinStationSystemTrace,
	WinStationVirtualData,
	WinStationClientData,
	WinStationSecureDesktopEnter,
	WinStationSecureDesktopExit,
	WinStationLoadBalanceSessionTarget,
	WinStationLoadIndicator,
	WinStationShadowInfo,
	WinStationDigProductId,
	WinStationLockedState,
	WinStationRemoteAddress,
	WinStationIdleTime,
	WinStationLastReconnectType,
	WinStationDisallowAutoReconnect,
	WinStationUnused2,
	WinStationUnused3,
	WinStationUnused4,
	WinStationUnused5,
	WinStationReconnectedFromId,
	WinStationEffectsPolicy,
	WinStationType,
	WinStationInformationEx
} WINSTATIONINFOCLASS;

int main()
{
	ULONG cb;
	typedef BOOLEAN(WINAPI * PWINSTATIONQUERYINFORMATIONW)(HANDLE, ULONG, WINSTATIONINFOCLASS, PVOID, ULONG, PULONG);

	PWINSTATIONQUERYINFORMATIONW WinStationQueryInformationW;

	WINSTATIONREMOTEADDRESS ra;
	HINSTANCE hInstWinSta;
	ULONG ReturnLen;

	union {
		SOCKADDR sa;
		SOCKADDR_IN sa4;
		SOCKADDR_IN6_LH sa6;
	};

	hInstWinSta = LoadLibraryA("winsta.dll");
	if (hInstWinSta)
	{
		WinStationQueryInformationW = (PWINSTATIONQUERYINFORMATIONW)
			GetProcAddress(hInstWinSta, "WinStationQueryInformationW");
		if (WinStationQueryInformationW &&
			WinStationQueryInformationW(0,
				LOGONID_CURRENT,
				WinStationRemoteAddress,
				(PVOID)&ra,
				sizeof(ra), &cb))
		{
			char *some_addr;

			switch (sa.sa_family = ra.sin_family)
			{
			case AF_INET:
				sa4.sin_port = ra.ipv4.sin_port;
				sa4.sin_addr.S_un.S_addr = ra.ipv4.in_addr;
				RtlZeroMemory(sa4.sin_zero, sizeof(sa4.sin_zero));
				cb = sizeof(SOCKADDR_IN);

				some_addr = new char[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &(sa4.sin_addr), some_addr, INET_ADDRSTRLEN);
				printf("IP: %s\n", some_addr);
				delete[] some_addr;
				break;
			case AF_INET6:
				sa6.sin6_port = ra.ipv6.sin6_port;
				sa6.sin6_flowinfo = ra.ipv6.sin6_flowinfo;
				memcpy(&sa6.sin6_addr, &ra.ipv6.sin6_addr, sizeof(in6_addr));
				sa6.sin6_scope_id = ra.ipv6.sin6_scope_id;
				cb = sizeof(SOCKADDR_IN6);

				some_addr = new char[INET6_ADDRSTRLEN];
				inet_ntop(AF_INET6, &(sa6.sin6_addr), some_addr, INET6_ADDRSTRLEN);
				printf("IP6: %s\n", some_addr);
				delete[] some_addr;
				break;
			default:
				std::cout << "Error: unknown IP protocol" << std::endl;
			}

			FreeLibrary(hInstWinSta);
			return 0;
		}
		DWORD dw = WSAGetLastError();
		LPVOID lpMsgBuf;
		LPVOID lpDisplayBuf;


		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);

		// Display the error message and exit the process

		lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
			(lstrlen((LPCTSTR)lpMsgBuf) + 40) * sizeof(TCHAR));
		StringCchPrintf((LPTSTR)lpDisplayBuf,
			LocalSize(lpDisplayBuf) / sizeof(TCHAR),
			TEXT("failed with error %d: %s"),
			dw, lpMsgBuf);
		MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

		LocalFree(lpMsgBuf);
		LocalFree(lpDisplayBuf);

		FreeLibrary(hInstWinSta);
	}

	return 0;
}
