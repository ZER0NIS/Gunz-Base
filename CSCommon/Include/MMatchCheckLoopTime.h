#pragma once

#include <vector>
using namespace std;

struct ST_COMMAND_TIMEGAP
{
	int nCmdID;
	DWORD dwStartTick;
	DWORD dwEndTick;
};

typedef vector<ST_COMMAND_TIMEGAP> VEC_COMMAND;

class MMatchCheckLoopTime
{
private:
	DWORD m_dwStartLoop;
	DWORD m_dwEndLoop;
	DWORD m_dwStartWindowMsg;
	DWORD m_dwEndWindowMsg;
	DWORD m_dwStartPrepareRun;
	DWORD m_dwEndPrepareRun;
	DWORD m_dwStartCommand;
	DWORD m_dwEndCommand;
	VEC_COMMAND m_vecCommand;
	DWORD m_nVecCount;
	DWORD m_dwMaxCommandCount;
	DWORD m_dwStartScheduler;
	DWORD m_dwEndScheduler;
	DWORD m_dwStartPremiumIPCache;
	DWORD m_dwEndPremiumIPCache;
	SYSTEMTIME m_stTimePremiumIPCache;
	DWORD m_dwStartObject;
	DWORD m_dwEndObject;
	DWORD m_dwStartStage;
	DWORD m_dwEndStage;
	DWORD m_dwStartChannel;
	DWORD m_dwEndChannel;
	DWORD m_dwStartClan;
	DWORD m_dwEndClan;
	SYSTEMTIME m_stTimeClan;
	DWORD m_dwStartLadder;
	DWORD m_dwEndLadder;
	SYSTEMTIME m_stTimeLadder;
	DWORD m_dwStartPing;
	DWORD m_dwEndPing;
	SYSTEMTIME m_stTimePing;
	DWORD m_dwStartSessionClean;
	DWORD m_dwEndSessionClean;
	DWORD m_dwStartAsyncJob;
	DWORD m_dwEndAsyncJob;
	DWORD m_dwStartServerLog;
	DWORD m_dwEndServerLog;
	SYSTEMTIME m_stTimeServerLog;
	DWORD m_dwStartServerDBLog;
	DWORD m_dwEndServerDBLog;
	SYSTEMTIME m_stTimeServerDBLog;
	DWORD m_dwStartCustomIPList;
	DWORD m_dwEndCustomIPList;
	SYSTEMTIME m_stTimeCustomIPList;
	DWORD m_dwStartShutdown;
	DWORD m_dwEndShutdown;
	SYSTEMTIME m_stTimeShutdown;
	DWORD m_dwStartNHNUSAAuth;
    DWORD m_dwEndNHNUSAAuth;
	SYSTEMTIME m_stTimeNHNUSAAuth;
	DWORD m_dwStartGameGuard;
	DWORD m_dwEndGameGuard;
	SYSTEMTIME m_stTimeGameGuard;
	DWORD m_dwStartXTrap;
	DWORD m_dwEndXTrap;
	DWORD m_dwStartMonitor;
	DWORD m_dwEndMonitor;
	DWORD m_dwStartKillTracker;
	DWORD m_dwEndKillTracker;
	SYSTEMTIME m_stTimeKillTracker;

	SYSTEMTIME m_stStartLogTime;
	DWORD m_dwMaxLoopGap;
	bool m_bInit;
	bool m_bIsRun;
	MMatchCheckLoopTime();

public:
	virtual ~MMatchCheckLoopTime();

	static MMatchCheckLoopTime* GetInstance();

	void SetInit()					{ m_bInit = true; }
	void SetLoopCheckRun()			{ m_bIsRun = true; }
	DWORD GetLoopTimeGap()			{ return (m_dwEndLoop - m_dwStartLoop); }
	void SaveLoopLogFile();

	int AddCommandTimeGap(int nCmdID);
	void ClearCommandTimeGap();
	void SetCommandEndTick(int nIndex);

	void SetStartLoop()				{ GetLocalTime(&m_stStartLogTime); m_dwStartLoop = GetTickCount(); m_dwStartWindowMsg = m_dwStartLoop; }
	void SetPrepareRunTick()		{ m_dwEndWindowMsg = GetTickCount(); m_dwStartPrepareRun = m_dwEndWindowMsg; }
	void SetCommandTick()			{ m_dwEndPrepareRun = GetTickCount(); m_dwStartCommand = m_dwEndPrepareRun; }
	void SetSchedulerTick()			{ m_dwEndCommand = GetTickCount(); m_dwStartScheduler = m_dwEndCommand; }
	void SetPremiumIPCacheTick()	{ m_dwEndScheduler = GetTickCount(); m_dwStartPremiumIPCache = m_dwEndScheduler; GetLocalTime(&m_stTimePremiumIPCache); }
	void SetObjectTick()			{ m_dwEndPremiumIPCache = GetTickCount(); m_dwStartObject = m_dwEndPremiumIPCache; }
	void SetStageTick()				{ m_dwEndObject = GetTickCount(); m_dwStartStage = m_dwEndObject; }
	void SetChannelTick()			{ m_dwEndStage = GetTickCount(); m_dwStartChannel = m_dwEndStage; }
	void SetClanTick()				{ m_dwEndChannel = GetTickCount(); m_dwStartClan = m_dwEndChannel; GetLocalTime(&m_stTimeClan); }
	void SetLadderTick()			{ m_dwEndClan = GetTickCount(); m_dwStartLadder = m_dwEndClan; GetLocalTime(&m_stTimeLadder); }
	void SetPingTick()				{ m_dwEndLadder = GetTickCount(); m_dwStartPing = m_dwEndLadder; GetLocalTime(&m_stTimePing); }
	void SetSessionCleanTick()		{ m_dwEndPing = GetTickCount(); m_dwStartSessionClean = m_dwEndPing; }
	void SetAsyncJopTick()			{ m_dwEndSessionClean = GetTickCount(); m_dwStartAsyncJob = m_dwEndSessionClean; }
	void SetServerLogTick()			{ m_dwEndAsyncJob = GetTickCount(); m_dwStartServerLog = m_dwEndAsyncJob; GetLocalTime(&m_stTimeServerLog); }
	void SetServerDBLogTick()		{ m_dwEndServerLog = GetTickCount(); m_dwStartServerDBLog = m_dwEndServerLog; GetLocalTime(&m_stTimeServerDBLog); }
	void SetCustomIPListTick()		{ m_dwEndServerDBLog = GetTickCount(); m_dwStartCustomIPList = m_dwEndServerDBLog; GetLocalTime(&m_stTimeCustomIPList); }
	void SetShutdownTick()			{ m_dwEndCustomIPList = GetTickCount(); m_dwStartShutdown = m_dwEndCustomIPList; GetLocalTime(&m_stTimeShutdown); }
	void SetNHNUSAAuthTick()		{ m_dwEndShutdown = GetTickCount(); m_dwStartNHNUSAAuth = m_dwEndShutdown; GetLocalTime(&m_stTimeNHNUSAAuth); }
	void SetGameGuardTick()			{ m_dwEndNHNUSAAuth = GetTickCount(); m_dwStartGameGuard = m_dwEndNHNUSAAuth; GetLocalTime(&m_stTimeGameGuard); }
	void SetXTrapTick()				{ m_dwEndGameGuard = GetTickCount(); m_dwStartXTrap = m_dwEndGameGuard; }
	void SetMonitorTick()			{ m_dwEndXTrap = GetTickCount(); m_dwStartMonitor = m_dwEndXTrap; }
	void SetKillTrackerTick()		{ m_dwEndMonitor = GetTickCount(); m_dwStartKillTracker = m_dwEndMonitor; GetLocalTime(&m_stTimeKillTracker); }
	void SetEndLoop()				{ m_dwEndKillTracker = GetTickCount(); m_dwEndLoop = m_dwEndKillTracker; }
};

inline MMatchCheckLoopTime* MGetCheckLoopTimeInstance()
{
	return MMatchCheckLoopTime::GetInstance();
}
