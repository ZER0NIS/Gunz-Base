#include "stdafx.h"
#include "MMatchCheckLoopTime.h"

MMatchCheckLoopTime::MMatchCheckLoopTime()
{
	m_dwStartLoop = 0;
	m_dwEndLoop = 0;
	m_dwStartWindowMsg = 0;
	m_dwEndWindowMsg = 0;
	m_dwStartPrepareRun = 0;
	m_dwEndPrepareRun = 0;
	m_dwStartCommand = 0;
	m_dwEndCommand = 0;
	m_dwStartScheduler = 0;
	m_dwEndScheduler = 0;
	m_dwStartPremiumIPCache = 0;
	m_dwEndPremiumIPCache = 0;
	memset(&m_stTimePremiumIPCache, 0, sizeof(SYSTEMTIME));
	m_dwStartObject = 0;
	m_dwEndObject = 0;
	m_dwStartStage = 0;
	m_dwEndStage = 0;
	m_dwStartChannel = 0;
	m_dwEndChannel = 0;
	m_dwStartClan = 0;
	m_dwEndClan = 0;
	memset(&m_stTimeClan, 0, sizeof(SYSTEMTIME));
	m_dwStartLadder = 0;
	m_dwEndLadder = 0;
	memset(&m_stTimeLadder, 0, sizeof(SYSTEMTIME));
	m_dwStartPing = 0;
	m_dwEndPing = 0;
	memset(&m_stTimePing, 0, sizeof(SYSTEMTIME));
	m_dwStartSessionClean = 0;
	m_dwEndSessionClean = 0;
	m_dwStartAsyncJob = 0;
	m_dwEndAsyncJob = 0;
	m_dwStartServerLog = 0;
	m_dwEndServerLog = 0;
	memset(&m_stTimeServerLog, 0, sizeof(SYSTEMTIME));
	m_dwStartServerDBLog = 0;
	m_dwEndServerDBLog = 0;
	memset(&m_stTimeServerDBLog, 0, sizeof(SYSTEMTIME));
	m_dwStartCustomIPList = 0;
	m_dwEndCustomIPList = 0;
	memset(&m_stTimeCustomIPList, 0, sizeof(SYSTEMTIME));
	m_dwStartShutdown = 0;
	m_dwEndShutdown = 0;
	memset(&m_stTimeShutdown, 0, sizeof(SYSTEMTIME));
	m_dwStartNHNUSAAuth = 0;
	m_dwEndNHNUSAAuth = 0;
	memset(&m_stTimeNHNUSAAuth, 0, sizeof(SYSTEMTIME));
	m_dwStartGameGuard = 0;
	m_dwEndGameGuard = 0;
	memset(&m_stTimeGameGuard, 0, sizeof(SYSTEMTIME));
	m_dwStartXTrap = 0;
	m_dwEndXTrap = 0;
	m_dwStartMonitor = 0;
	m_dwEndMonitor = 0;
	m_dwStartKillTracker = 0;
	m_dwEndKillTracker = 0;
	memset(&m_stTimeKillTracker, 0, sizeof(SYSTEMTIME));

	m_dwMaxLoopGap = 0;
	m_nVecCount = 0;
	m_dwMaxCommandCount = 0;
	memset(&m_stStartLogTime, 0, sizeof(SYSTEMTIME));
	m_bInit = false;
	m_bIsRun = false;
}

MMatchCheckLoopTime::~MMatchCheckLoopTime()
{
	ClearCommandTimeGap();
}

MMatchCheckLoopTime* MMatchCheckLoopTime::GetInstance()
{
	static MMatchCheckLoopTime hInst;
	return &hInst;
}

int MMatchCheckLoopTime::AddCommandTimeGap(int nCmdID)
{
	if (m_bIsRun == false)
	{
		return 0;
	}

	ST_COMMAND_TIMEGAP stCmdGap;
	stCmdGap.nCmdID = nCmdID;
	stCmdGap.dwEndTick = 0;
	if (m_nVecCount == 0)
	{
		stCmdGap.dwStartTick = m_dwEndPrepareRun;
	}
	else
	{
		stCmdGap.dwStartTick = m_vecCommand[m_nVecCount-1].dwEndTick;
	}

	m_vecCommand.push_back(stCmdGap);
	m_nVecCount++;
	return m_nVecCount - 1;
}

void MMatchCheckLoopTime::ClearCommandTimeGap()
{
	if (m_nVecCount != 0)
	{
		m_vecCommand.clear();
		m_nVecCount = 0;
	}
}

void MMatchCheckLoopTime::SetCommandEndTick(int nIndex)
{
	if ((m_bIsRun == false) || (m_nVecCount == 0) || ((int)m_vecCommand.size() == 0))
	{
		return;
	}

	ST_COMMAND_TIMEGAP* pCmdGap = &m_vecCommand[nIndex];
	if (pCmdGap == NULL)
	{
		return;
	}

	pCmdGap->dwEndTick = GetTickCount();
}

void MMatchCheckLoopTime::SaveLoopLogFile()
{
	if ((m_bInit == false) || (m_bIsRun == false))
	{
		return;
	}

	DWORD dwLoopGap = m_dwEndLoop - m_dwStartLoop;
	if (dwLoopGap > m_dwMaxLoopGap)
	{
		m_dwMaxLoopGap = dwLoopGap;
	}
	mlog("\nLOOP LOG START ===== [Loop Start Time : %2d/ %2d %2d:%2d:%2d] ===================\n",
		m_stStartLogTime.wMonth, m_stStartLogTime.wDay, m_stStartLogTime.wHour, m_stStartLogTime.wMinute, m_stStartLogTime.wSecond);
	mlog("LoopTimeGap = %5u                        [MAX : %5u]\n", dwLoopGap, m_dwMaxLoopGap);
	DWORD dwGap = 0;
	dwGap = m_dwEndWindowMsg - m_dwStartWindowMsg;
	if (dwGap > 0)
	{
		mlog("CMatchServerApp::Run() Window Message Peek [Gap : %5u]\n", dwGap);
	}
	dwGap = m_dwEndPrepareRun - m_dwStartPrepareRun;
	if (dwGap > 0)
	{
		mlog("MCommandCommunicator::Run() OnPrepareRun() [Gap : %5u]\n", dwGap);
	}
	dwGap = m_dwEndCommand - m_dwStartCommand;
	if (dwGap > 0)
	{
		mlog("MCommandCommunicator::Run() while (true)   [Gap : %5u]\n", dwGap);
	}
	if (m_nVecCount > m_dwMaxCommandCount)
	{
		m_dwMaxCommandCount = m_nVecCount;
	}
	mlog("COMMAND LOG START ==========================================================\n");
	mlog("Command Count = %2d                         [MAX : %5u]\n", m_nVecCount, m_dwMaxCommandCount);
	for (DWORD i = 0; i < m_nVecCount; i++)
	{
		dwGap = (m_vecCommand[i].dwEndTick - m_vecCommand[i].dwStartTick);
		if (dwGap > 0)
		{
			mlog("Command ID = %5d                         [Gap : %5u]\n", m_vecCommand[i].nCmdID, dwGap);
		}
	}
	mlog("COMMAND LOG END ============================================================\n");
	dwGap = m_dwEndScheduler - m_dwStartScheduler;
	if (dwGap > 0)
	{
		mlog("m_pScheduler->Update()                     [Gap : %5u]\n", dwGap);
	}
	dwGap = m_dwEndPremiumIPCache - m_dwStartPremiumIPCache;
	if (dwGap > 0)
	{
		mlog("MPremiumIPCache()->Update()                [Gap : %5u], [%2d/ %2d %2d:%2d:%2d]\n", dwGap,
			m_stTimePremiumIPCache.wMonth, m_stTimePremiumIPCache.wDay, m_stTimePremiumIPCache.wHour, m_stTimePremiumIPCache.wMinute, m_stTimePremiumIPCache.wSecond);
	}
	dwGap = m_dwEndObject - m_dwStartObject;
	if (dwGap > 0)
	{
		mlog("Update Objects                             [Gap : %5u]\n", dwGap);
	}
	dwGap = m_dwEndStage - m_dwStartStage;
	if (dwGap > 0)
	{
		mlog("Update Stages                              [Gap : %5u]\n", dwGap);
	}
	dwGap = m_dwEndChannel - m_dwStartChannel;
	if (dwGap > 0)
	{
		mlog("m_ChannelMap.Update()                      [Gap : %5u]\n", dwGap);
	}
	dwGap = m_dwEndClan - m_dwStartClan;
	if (dwGap > 0)
	{
		mlog("m_ClanMap.Tick()                           [Gap : %5u], [%2d/ %2d %2d:%2d:%2d]\n",
			dwGap, m_stTimeClan.wMonth, m_stTimeClan.wDay, m_stTimeClan.wHour, m_stTimeClan.wMinute, m_stTimeClan.wSecond);
	}
	dwGap = m_dwEndLadder - m_dwStartLadder;
	if (dwGap > 0)
	{
		mlog("GetLadderMgr()->Tick()                     [Gap : %5u], [%2d/ %2d %2d:%2d:%2d]\n",
			dwGap, m_stTimeLadder.wMonth, m_stTimeLadder.wDay, m_stTimeLadder.wHour, m_stTimeLadder.wMinute, m_stTimeLadder.wSecond);
	}
	dwGap = m_dwEndPing - m_dwStartPing;
	if (dwGap > 0)
	{
		mlog("Garbage Session Cleaning                   [Gap : %5u], [%2d/ %2d %2d:%2d:%2d]\n",
			dwGap, m_stTimePing.wMonth, m_stTimePing.wDay, m_stTimePing.wHour, m_stTimePing.wMinute, m_stTimePing.wSecond);
	}
	dwGap = m_dwEndSessionClean - m_dwStartSessionClean;
	if (dwGap > 0)
	{
		mlog("Garbage MatchObject Cleaning               [Gap : %5u]\n", dwGap);
	}
	dwGap = m_dwEndAsyncJob - m_dwStartAsyncJob;
	if (dwGap > 0)
	{
		mlog("ProcessAsyncJob()                          [Gap : %5u]\n", dwGap);
	}
	dwGap = m_dwEndServerLog - m_dwStartServerLog;
	if (dwGap > 0)
	{
		mlog("UpdateServerLog()                          [Gap : %5u], [%2d/ %2d %2d:%2d:%2d]\n",
			dwGap, m_stTimeServerLog.wMonth, m_stTimeServerLog.wDay, m_stTimeServerLog.wHour, m_stTimeServerLog.wMinute, m_stTimeServerLog.wSecond);
	}
	dwGap = m_dwEndServerDBLog - m_dwStartServerDBLog;
	if (dwGap > 0)
	{
		mlog("UpdateServerStatusDB()                     [Gap : %5u], [%2d/ %2d %2d:%2d:%2d]\n", dwGap,
			m_stTimeServerDBLog.wMonth, m_stTimeServerDBLog.wDay, m_stTimeServerDBLog.wHour, m_stTimeServerDBLog.wMinute, m_stTimeServerDBLog.wSecond);
	}
	dwGap = m_dwEndCustomIPList - m_dwStartCustomIPList;
	if (dwGap > 0)
	{
		mlog("UpdateCustomIPList()                       [Gap : %5u], [%2d/ %2d %2d:%2d:%2d]\n", dwGap,
			m_stTimeCustomIPList.wMonth, m_stTimeCustomIPList.wDay, m_stTimeCustomIPList.wHour, m_stTimeCustomIPList.wMinute, m_stTimeCustomIPList.wSecond);
	}
	dwGap = m_dwEndShutdown - m_dwStartShutdown;
	if (dwGap > 0)
	{
        mlog("m_MatchShutdown.OnRun()                    [Gap : %5u], [%2d/ %2d %2d:%2d:%2d]\n", dwGap,
			m_stTimeShutdown.wMonth, m_stTimeShutdown.wDay, m_stTimeShutdown.wHour, m_stTimeShutdown.wMinute, m_stTimeShutdown.wSecond);
	}
	dwGap = m_dwEndNHNUSAAuth - m_dwStartNHNUSAAuth;
	if (dwGap > 0)
	{
		mlog("GetNHNRTA().RTA()                          [Gap : %5u], [%2d/ %2d %2d:%2d:%2d]\n", dwGap,
			m_stTimeNHNUSAAuth.wMonth, m_stTimeNHNUSAAuth.wDay, m_stTimeNHNUSAAuth.wHour, m_stTimeNHNUSAAuth.wMinute, m_stTimeNHNUSAAuth.wSecond);
	}
	dwGap = m_dwEndGameGuard - m_dwStartGameGuard;
	if (dwGap > 0)
	{
		mlog("OnRun_GameGuard()                          [Gap : %5u], [%2d/ %2d %2d:%2d:%2d]\n", dwGap,
			m_stTimeGameGuard.wMonth, m_stTimeGameGuard.wDay, m_stTimeGameGuard.wHour, m_stTimeGameGuard.wMinute, m_stTimeGameGuard.wSecond);
	}
	dwGap = m_dwEndXTrap - m_dwStartXTrap;
	if (dwGap > 0)
	{
		mlog("OnRun_XTrap()                              [Gap : %5u]\n", dwGap);
	}
	dwGap = m_dwEndMonitor - m_dwStartMonitor;
	if (dwGap > 0)
	{
		mlog("m_Monitor.Run()                            [Gap : %5u]\n", dwGap);
	}
	dwGap = m_dwEndKillTracker - m_dwStartKillTracker;
	if (dwGap > 0)
	{
		mlog("GetKillTracker().Update()                  [Gap : %5u], [%2d/ %2d %2d:%2d:%2d]\n",dwGap,
			m_stTimeKillTracker.wMonth, m_stTimeKillTracker.wDay, m_stTimeKillTracker.wHour, m_stTimeKillTracker.wMinute, m_stTimeKillTracker.wSecond);
	}

	SYSTEMTIME st;
	memset(&st, 0, sizeof(SYSTEMTIME));
	GetLocalTime(&st);

	mlog("LOOP LOG END ======= [Loop End Time   : %2d/ %2d %2d:%2d:%2d] ===================\n", st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	ClearCommandTimeGap();
}
