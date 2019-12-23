/////////////////////////////////////////////////////////////////////
// MBMatchSystemInfo.cpp
/////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Psapi.h"
#include "MBMatchSystemInfo.h"

MBMatchSystemInfo::MBMatchSystemInfo()
{
	m_dwStartTick = GetTickCount();
}

MBMatchSystemInfo::~MBMatchSystemInfo()
{
}

/////////////////////////////////////////////////////////////////////
// GetProcessRunningTime => 프로세스 가동시간에 대한 정보를 얻어온다.
/////////////////////////////////////////////////////////////////////
void MBMatchSystemInfo::GetProcessRunningTime(SYSTEMTIME* stRunningTime)
{
#ifdef _MONITORING
	memset(stRunningTime, 0, sizeof(SYSTEMTIME));
	// 경과 시간(현재 시간의 절대초 - 시작 시간의 절대초)
	DWORD dwNowTick = GetTickCount();
	DWORD dwRunningTime = dwNowTick - m_dwStartTick;

	DWORD dwTotalSecond = dwRunningTime / 1000;
	DWORD dwDay = dwTotalSecond / 86400;
	DWORD dwRemanderSecond = dwTotalSecond % 86400;
	stRunningTime->wDay = (WORD)dwDay;
	DWORD dwHour = dwRemanderSecond / 3600;
	dwRemanderSecond = dwRemanderSecond % 3600;
	stRunningTime->wHour = (WORD)dwHour;
	DWORD dwMinute = dwRemanderSecond / 60;
	stRunningTime->wMinute = (WORD)dwMinute;
	DWORD dwSecond = dwRemanderSecond % 60;
	stRunningTime->wSecond = (WORD)dwSecond;
#endif
}

/////////////////////////////////////////////////////////////////////
// GetMemoryInfo => 메모리에 대한 정보를 얻어온다. (단위 MB)
// dwTotalMemMB : 시스템 전체 메모리 용량
// dwAvailMemMB : 사용중인 메모리 용량
// dwVirtualMemMB : 가상메모리 전체 용량
/////////////////////////////////////////////////////////////////////
void MBMatchSystemInfo::GetMemoryInfo(DWORD* dwTotalMemMB, DWORD* dwAvailMemMB, DWORD* dwVirtualMemMB)
{
#ifdef _MONITORING
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);

	DWORDLONG lMemTotalMB = (statex.ullTotalPhys / (1024 * 1024));
	*dwTotalMemMB = (DWORD)lMemTotalMB;

	DWORDLONG lAvailMemMB = (statex.ullAvailPhys / (1024 * 1024));
	*dwAvailMemMB = (DWORD)lAvailMemMB;

	DWORDLONG lVirtualMemMB = (statex.ullTotalVirtual / (1024 * 1024));
	*dwVirtualMemMB = (DWORD)lVirtualMemMB;
#endif
}

/////////////////////////////////////////////////////////////////////
// GetDiskInfo => 드라이브에 대한 정보를 얻어온다. (단위 MB)
// szDriveName : 드라이브 명
// dwTotalDriveMB : 해당 드라이브 전체 용량
// dwFreeDriveMB : 해당 드라이브 남은 용량(사용가능)
// return value : 성공하면 true 리턴, false => szDriveName이 디스크 드라이브명이 아닐 경우
/////////////////////////////////////////////////////////////////////
bool MBMatchSystemInfo::GetDiskInfo(char szDriveName, DWORD* dwTotalDriveMB, DWORD* dwFreeDriveMB)
{
#ifdef _MONITORING
	char strDirveName[MAX_PATH] = {0};
	sprintf(strDirveName, "%c:", szDriveName);
	unsigned int uDrvType = GetDriveType(strDirveName);
	if (uDrvType != DRIVE_FIXED)						// 디스크 드라이버인 경우만
	{
		return false;
	}

	typedef BOOL (WINAPI *P_GDFSE)(LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);
    P_GDFSE pGetDiskFreeSpaceEx = (P_GDFSE)GetProcAddress(GetModuleHandle("kernel32.dll"), "GetDiskFreeSpaceExA");

	DWORD dwVolumeSerialNumber = 0;
	DWORD dwMaxNameLength = 0;
	DWORD dwFileSystemFlags = 0;
	char szFileSysName[MAX_PATH] = {0};
	char szLabel[MAX_PATH] = {0};

	// GetVolumeInformation은 해당 디스크의 일반정보를 가지고 오는 함수이다.
	GetVolumeInformation(strDirveName, szLabel, 256, &dwVolumeSerialNumber, &dwMaxNameLength, &dwFileSystemFlags, szFileSysName, 256);

	BOOL fResult;
	unsigned __int64 i64FreeBytesToCaller, i64TotalBytes, i64FreeBytes;
	DWORD dwSectPerClust, dwBytesPerSect, dwFreeClusters, dwTotalClusters;
	DWORD dwTotalMBytes, dwFreeMBytes;

	// 만약 95이상의 파일 시스템 일경우
	if (pGetDiskFreeSpaceEx)
	{
		// Win98 or NT 나 그 이상이면 True 값을 반환한다.
		fResult = pGetDiskFreeSpaceEx(strDirveName, (PULARGE_INTEGER)&i64FreeBytesToCaller, (PULARGE_INTEGER)&i64TotalBytes, (PULARGE_INTEGER)&i64FreeBytes);
		if (fResult)
		{
			dwTotalMBytes = (DWORD)(i64TotalBytes / (1024 * 1024));
			dwFreeMBytes = (DWORD)(i64FreeBytes / (1024 * 1024));
		}
	}
	// 만약 95이하의 시스템 일경우
	else
	{
		fResult = GetDiskFreeSpace(strDirveName, &dwSectPerClust, &dwBytesPerSect, &dwFreeClusters, &dwTotalClusters);
		if (fResult)
		{
			i64TotalBytes = (__int64)dwTotalClusters * dwSectPerClust *	dwBytesPerSect;
			i64FreeBytes = (__int64)dwFreeClusters * dwSectPerClust * dwBytesPerSect;
			dwTotalMBytes = (DWORD)(i64TotalBytes / (1024 * 1024));
			dwFreeMBytes = (DWORD)(i64FreeBytes / (1024 * 1024));
		}
	}

	*dwTotalDriveMB = dwTotalMBytes;
	*dwFreeDriveMB = dwFreeMBytes;
#endif
	return true;
}

/////////////////////////////////////////////////////////////////////
// GetProcessName => 자기자신 실행파일명에 대한 정보를 얻어온다.
// szProcessName : 실행파일명
////////////////////////////////////////////////////


void MBMatchSystemInfo::GetExeFileName(char* szExeFileName)
{
	char szName[MAX_PATH] = {0};
	DWORD cb;
	BOOL bResult;
	HMODULE hModule;
	HANDLE hProcess = GetCurrentProcess();
	if (hProcess)
	{
		bResult = EnumProcessModules(hProcess, &hModule, sizeof(hModule), &cb);
		if (bResult != 0)
		{
			GetModuleBaseNameA(hProcess, hModule, szName, sizeof(szName));
		}
	}
	memcpy(szExeFileName, szName, MAX_PATH);
}
