/////////////////////////////////////////////////////////////////////
// MBMatchSystemInfo.h
// 머신의 시스템 정보를 수집한다.
/////////////////////////////////////////////////////////////////////
#ifndef _MBMATCHSYSTEMINFO_H_
#define _MBMATCHSYSTEMINFO_H_

class MBMatchSystemInfo
{
private:
	DWORD m_dwStartTick;

public:
	MBMatchSystemInfo();
	virtual ~MBMatchSystemInfo();

	void GetProcessRunningTime(SYSTEMTIME* stRunningTime);
	void GetMemoryInfo(DWORD* dwTotalMemMB, DWORD* dwAvailMemMB, DWORD* dwVirtualMemMB);
	bool GetDiskInfo(char szDriveName, DWORD* dwTotalDriveMB, DWORD* dwFreeDriveMB);
	void GetExeFileName(char* szExeFileName);
};

#endif