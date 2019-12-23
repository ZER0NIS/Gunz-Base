#pragma once

#include "SafeString.h"
#include "GlobalTypes.h"


class MCriticalSection {
	CRITICAL_SECTION	m_cs;

public:
	MCriticalSection() { InitializeCriticalSection(&m_cs);	}
	~MCriticalSection() { DeleteCriticalSection(&m_cs); }
	void Lock() { EnterCriticalSection(&m_cs); }
	void Unlock() { LeaveCriticalSection(&m_cs); }
};


class MSignalEvent {
	HANDLE		m_hEvent;

public:
	MSignalEvent() { m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL); }
	~MSignalEvent() { if (m_hEvent) { CloseHandle(m_hEvent); m_hEvent = NULL; } }
	HANDLE GetEvent() { return m_hEvent; }
	BOOL SetEvent() { return ::SetEvent(m_hEvent); }
	BOOL ResetEvent() { return ::ResetEvent(m_hEvent); }
};


class MSingleRunController {
protected:
	char	m_szAppName[_MAX_DIR];
	HANDLE	m_Mutex;
public:
	MSingleRunController(const char* pszAppName)	{ 
		m_Mutex = NULL;
		strcpy_safe(m_szAppName, pszAppName); 
	}
	virtual ~MSingleRunController()	{ 
		Destroy();
	}	
	bool Create(bool bWaitMode=false) {
		if (bWaitMode == true) {
			int tmTotalWait = 0;
			while(true) {
				m_Mutex = CreateMutex(NULL, TRUE, m_szAppName);
				if (GetLastError() == ERROR_ALREADY_EXISTS)	{
					CloseHandle(m_Mutex);

					Sleep(100);
					tmTotalWait += 100;
					if (tmTotalWait >= 5*60*1000) // 5분이상은 강제종료
						return false;
				} else {
					return true;
				}
			}
		} else {
			m_Mutex = CreateMutex(NULL, TRUE, m_szAppName);
			if (GetLastError() == ERROR_ALREADY_EXISTS)
				return false;
			else
				return true;
		}
	}
	void Destroy() {
		if (m_Mutex) {
			ReleaseMutex(m_Mutex);	
			m_Mutex = NULL;
		}
	}
};