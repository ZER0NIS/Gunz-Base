/////////////////////////////////////////////////
// MBMatchAsyncJob_GetCpuInfo.cpp
/////////////////////////////////////////////////
#include "stdafx.h"
#include "MBMatchAsyncJob_GetCpuInfo.h"

#define SystemBasicInformation			0
#define SystemPerformanceInformation	2
#define SystemTimeInformation			3
#define Li2Double(x)					((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))

typedef struct _stSYSTEM_BASIC_INFORMATION
{ 
	DWORD dwUnknown1;
	ULONG uKeMaximumIncrement;
	ULONG uPageSize;
	ULONG uMmNumberOfPhysicalPages;
	ULONG uMmLowestPhysicalPage;
	ULONG uMmHighestPhysicalPage;
	ULONG uAllocationGranularity;
	PVOID pLowestUserAddress;
	PVOID pMmHighestUserAddress;
	ULONG uKeActiveProcessors;
	BYTE bKeNumberProcessors;
	BYTE bUnknown2;
	WORD wUnknown3;
} SYSTEM_BASIC_INFORMATION;

typedef struct _stSYSTEM_PERFORMANCE_INFORMATION
{
	LARGE_INTEGER liIdleTime;
	DWORD dwSpare[76];
} SYSTEM_PERFORMANCE_INFORMATION;

typedef struct _stSYSTEM_TIME_INFORMATION
{
	LARGE_INTEGER liKeBootTime;
	LARGE_INTEGER liKeSystemTime;
	LARGE_INTEGER liExpTimeZoneBias;
	ULONG uCurrentTimeZoneId;
	DWORD dwReserved;
} SYSTEM_TIME_INFORMATION;

/////////////////////////////////////////////////////////////////////
// GetCpuUsage => 현재 사용중인 CPU의 사용량을 얻어온다. (단위 %)
/////////////////////////////////////////////////////////////////////
void MBMatchAsyncJob_GetCpuInfo::Run(void* pContext)
{
	SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo;
	SYSTEM_TIME_INFORMATION SysTimeInfo;
	SYSTEM_BASIC_INFORMATION SysBaseInfo;
	double dbIdleTime;
	double dbSystemTime;
	double CurrentCpuUsage = 0;
	LONG status;
	LARGE_INTEGER liOldIdleTime = {0, 0};
	LARGE_INTEGER liOldSystemTime = {0, 0};

	typedef LONG (WINAPI *PROCNTQSI)(UINT, PVOID, ULONG, PULONG);
	PROCNTQSI NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(GetModuleHandle("ntdll"), "NtQuerySystemInformation");
	if (!NtQuerySystemInformation)
	{
		m_iCpuUsage = 0;
		return;
	}

	// get number of processors in the system
	status = NtQuerySystemInformation(SystemBasicInformation, &SysBaseInfo, sizeof(SysBaseInfo), NULL);
	if (status != NO_ERROR)
	{
		m_iCpuUsage = 0;
		return;
	}

	// get first system time
	status = NtQuerySystemInformation(SystemTimeInformation, &SysTimeInfo, sizeof(SysTimeInfo), 0);
	if (status != NO_ERROR)
	{
		m_iCpuUsage = 0;
		return;
	}

	// get first CPU's idle time
	status = NtQuerySystemInformation(SystemPerformanceInformation, &SysPerfInfo, sizeof(SysPerfInfo), NULL);
	if (status != NO_ERROR)
	{
		m_iCpuUsage = 0;
		return;
	}

	// store first CPU's idle and system time
	liOldIdleTime = SysPerfInfo.liIdleTime;
	liOldSystemTime = SysTimeInfo.liKeSystemTime;

	Sleep(200);

	// get second system time
	status = NtQuerySystemInformation(SystemTimeInformation, &SysTimeInfo, sizeof(SysTimeInfo), 0);
	if (status != NO_ERROR)
	{
		m_iCpuUsage = 0;
		return;
	}

	// get second CPU's idle time
	status = NtQuerySystemInformation(SystemPerformanceInformation, &SysPerfInfo, sizeof(SysPerfInfo), NULL);
	if (status != NO_ERROR)
	{
		m_iCpuUsage = 0;
		return;
	}

	// CurrentValue = NewValue - OldValue
	dbIdleTime = Li2Double(SysPerfInfo.liIdleTime) - Li2Double(liOldIdleTime);
	dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(liOldSystemTime);

	// CurrentCpuIdle = IdleTime / SystemTime
	dbIdleTime = dbIdleTime / dbSystemTime;

	// CurrentCpuUsage% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors
	CurrentCpuUsage = 100.0 - dbIdleTime * 100.0 / (double)SysBaseInfo.bKeNumberProcessors + 0.5;
	m_iCpuUsage = (UINT)CurrentCpuUsage;
}
