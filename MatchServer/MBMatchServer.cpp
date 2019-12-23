#include "stdafx.h"
#include "MBMatchServer.h"
#include "MatchServerDoc.h"
#include "OutputView.h"
#include <atltime.h>
#include "MatchServer.h"
#include "MMap.h"
#include "MErrorTable.h"
#include "CommandLogView.h"
#include "MDebug.h"
#include "MMatchRule.h"
#include "MBMatchAuth.h"
#include "MDebug.h"
#include "MMatchStatus.h"
#include "MMatchSchedule.h"
#include "MSharedCommandTable.h"
#include "MMatchConfig.h"
#include "MMatchEventFactory.h"
#include "HShield/AntiCPSvrfunc.h"
#include "MMatchLocale.h"
#include "MBlobArray.h"
#include "MMatchCheckLoopTime.h"


void __cdecl MBRCPLog(const char *pFormat,...)
{
	char szBuf[256];

	va_list args;

	va_start(args,pFormat);
	vsprintf(szBuf, pFormat, args);
	va_end(args);

	int nEnd = (int)(strlen(szBuf)-1);
	if ((nEnd >= 0) && (szBuf[nEnd] == '\n')) {
		szBuf[nEnd] = NULL;
		strcat(szBuf, "\n");
	}
	mlog(szBuf);
}

MBMatchServer::~MBMatchServer() 
{
	OnDestroy();
}

bool MBMatchServer::OnCreate(void)
{
	SetupRCPLog(MBRCPLog);

	CMatchServerApp* pApp = (CMatchServerApp*)AfxGetApp();

	// UDP초기화보다 먼저 실행이 되어야 함.
	// 모니터링 관련 패킷이 모니터링 초기화 보다 먼저 도착하면 NULL포인터 참조가 일어남.
#ifdef _MONITORING
	if( !m_Monitor.Init() )
	{
		mlog( "Init monitor failn" );
		return false;
	}
#endif

	if( !MMatchServer::OnCreate() )
		return false;

	if( !m_ConfigReloader.Create() )
		return false;

	if( !InitSecrity() )
	{
		mlog( "init security fail.\n" );
		return false;
	}
	
	if( !LoadFileMD5HashValue() )
		return false; // - by SungE 2007-05-28
	

	if( m_KillTracker.Init(&(MGetServerConfig()->GetKillTrackerConfig())) )
		return false;


	WriteServerInfoLog();

	MGetCheckLoopTimeInstance()->SetInit();

#ifdef _DEBUG
	//m_Tester.Init( this );
	//m_Tester.DoTest( GetGlobalClockCount() );
#endif
	return true;
}


bool MBMatchServer::LoadFileMD5HashValue()
{
#ifndef _DEBUG
	char curDirectory[MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH, curDirectory);

	char filePath[MAX_PATH] = {0};
	sprintf(filePath, "%s\\MD5HashValue.txt", curDirectory);

	FILE *ReadFp = fopen(filePath, "rb");
	if( NULL == ReadFp )
	{
		mlog( "Load MD5 fail.\n" );
		return false;
	}

	int readNum = 0;
	for (int i = 0; i < 16; i++)
	{
		fscanf(ReadFp, "%x", &readNum);
		m_szMD5Value[i] = (unsigned char)readNum;
	}
	
	fclose(ReadFp);


	mlog("FILE PATH : %s\nMD5 Hash Value => ", filePath);
	for( int i=0; i<16; i++)
	{
		mlog("%02x ", m_szMD5Value[i]);
	}

	mlog("\n");
#endif

	return true;
}


void MBMatchServer::ReleaseSecurity()
{
	m_Security.Release();

#ifdef _XTRAP
	ReleaseMemPool(MBMatchXTrapCC);
	UninitMemPool(MBMatchXTrapCC);
#endif
}


void MBMatchServer::OnDestroy(void)
{
#ifdef _HSHIELD
	if( MGetServerConfig()->IsUseHShield() )
		_AntiCpSvr_Finalize();
#endif

	ReleaseSecurity();

#ifdef _GAMEGUARD
		CleanupMBMatchGameguardAuth();
#endif

#ifdef _MONITORING
		m_Monitor.Release();
#endif

#ifdef _DEBUG
	m_Tester.Release();
#endif
}

void MBMatchServer::OnPrepareCommand(MCommand* pCommand)
{
	// 커맨드 로그 남기기
	if(m_pCmdLogView==NULL) return;

	CMatchServerApp* pApp = (CMatchServerApp*)AfxGetApp();
	if (pApp->CheckOutputLog() == false) return;


	CCommandLogView::CCommandType t;
	if(pCommand->m_pCommandDesc->IsFlag(MCDT_LOCAL)==true) t = CCommandLogView::CCT_LOCAL;
	else if(pCommand->m_Sender==m_This) t = CCommandLogView::CCT_SEND;
	else if(pCommand->m_Receiver==m_This) t = CCommandLogView::CCT_RECEIVE;
	else _ASSERT(FALSE);

	m_pCmdLogView->AddCommand(GetGlobalClockCount(), t, pCommand);
/*
#ifdef _DEBUG
#ifndef _DEBUG_PUBLISH
	// 커맨드 로그 남기기
	if(m_pCmdLogView==NULL) return;

	CMatchServerApp* pApp = (CMatchServerApp*)AfxGetApp();
	if (pApp->CheckOutputLog() == false) return;


	CCommandLogView::CCommandType t;
	if(pCommand->m_pCommandDesc->IsFlag(MCDT_LOCAL)==true) t = CCommandLogView::CCT_LOCAL;
	else if(pCommand->m_Sender==m_This) t = CCommandLogView::CCT_SEND;
	else if(pCommand->m_Receiver==m_This) t = CCommandLogView::CCT_RECEIVE;
	else _ASSERT(FALSE);
	
	m_pCmdLogView->AddCommand(GetGlobalClockCount(), t, pCommand);
#endif
#endif
*/
}



MBMatchServer::MBMatchServer(COutputView* pView)
{
	m_pView = pView;
	m_pCmdLogView = NULL;
	
	SetKeeperUID( MUID(0, 0) );
}

void MBMatchServer::Shutdown()
{
	MMatchServer::Shutdown();
	AfxGetMainWnd()->PostMessage(WM_DESTROY);
//	AfxGetMainWnd()->DestroyWindow();
}

void MBMatchServer::Log(unsigned int nLogLevel, const char* szLog)
{
	MMatchServer::Log(nLogLevel, szLog);	

	if ((nLogLevel & LOG_PROG) == LOG_PROG)
	{
		if(m_pView==NULL) return;

		CTime theTime = CTime::GetCurrentTime();
		CString szTime = theTime.Format( "[%c] " );

		m_pView->AddString(szTime, TIME_COLOR, false);
		m_pView->AddString(szLog, RGB(0,0,0));
	}
}

bool MBMatchServer::InitSubTaskSchedule()
{
	/*  등록방법을 결정해야 함.
		우선은 코드 속에다가. */

	// TODO: 버그가 있는 것 같아 주석처리 해놓았습니다. 클랜전신청 스위치가 꺼지지 않아야 할 때 꺼집니다. 확인 요망 -bird
	// 수정 했음 - SungE.

	// clan서버일 경우만.
	if( MSM_CLAN == MGetServerConfig()->GetServerMode() ){
		if( !AddClanServerSwitchDownSchedule() )
			return false;

		if( !AddClanServerAnnounceSchedule() )
		return false;
	}

	return true;
}

// 서버 시작시 클랜전 서버일경우 등록되는 스케쥴.
bool MBMatchServer::AddClanServerSwitchDownSchedule()
{
	int a = 0;

	MCommand* pCmd = CreateCommand( MC_MATCH_SCHEDULE_CLAN_SERVER_SWITCH_DOWN, MUID(0, 0) );
	if( 0 == pCmd )
		return false;

	tm* ptm = MMatchGetLocalTime();

	// 이것을 현제는 한번만 실행을 하지만, 매달 시작을 하는것으로 수정을 해야함.
	// 커맨드가 실행을 하는 시점에서 커맨드를 업데이트해서 다시 스케쥴러에 등록을 하는쪽으로 방향을 잡음.
	// OnCommnad계열에서 클랜 서버 다운 커맨드가 실행이 될시에 서버다운 커맨드를 다음달로 재 설정을 하고,
	//  다음달 1일에 클랜 서버가 다시 동작하는 커맨드를 생성하여 추가하는 방식으로 함.
	// 클랜서버 다운 커맨드 실행 -> 다음달 1일 클랜 서버 다시 실행 커맨드 생성, 등록 -> 다음달 클랜서버 다운 커맨드 생성, 등록.
	// 만약 클랜전다운 시간중에 여기까지 오면(서버 재시작등) 클랜전다운 시간이 이미 지났지만 이 시간으로 세팅하여
	// 곧바로 클랜전다운을 시켜주고 클랜전 시작시간을 생성, 등록해준다.

	MMatchScheduleData* pScheduleData = m_pScheduler->MakeOnceScheduleData( ptm->tm_year - 100, ptm->tm_mon + 1, GetMaxDay(), 23, 50, pCmd );
	if( 0 == pScheduleData ){
		delete pCmd;
		return false;
	}

	if( !m_pScheduler->AddDynamicSchedule(pScheduleData) ){
		delete pCmd;
		delete pScheduleData;
		return false;
	}

	mlog( "MBMatchServer::AddClanServerSwitchDownSchedule - make close clan mode schedule success. next close clan mode time is %d-%d-%d %d:%d\n",
		pScheduleData->GetYear(), pScheduleData->GetMonth(), pScheduleData->GetDay(),
		pScheduleData->GetHour(), pScheduleData->GetMin() );

	return true;
}


bool MBMatchServer::AddClanServerSwitchUpSchedule()
{
	// 다음달 1일 아침 10시에 클랜서버 클랜전 활성화.
	MCommand* pCmd = CreateCommand( MC_MATCH_SCHEDULE_CLAN_SERVER_SWITCH_ON, MUID(0, 0) );
	if( 0 == pCmd )
	{
		mlog( "MBMatchServer::AddClanServerSwitchUpSchedule - make clan mode open command fail.\n" );
		return false;
	}

	tm* ptm = MMatchGetLocalTime();

	unsigned char cYear;
	unsigned char cMon;

	// 다음달이 내년으로 넘어갔는지 검사함.
	if( 12 >= (ptm->tm_mon + 2) )
	{
		cYear = ptm->tm_year - 100;
		cMon  = ptm->tm_mon + 2;
	}
	else
	{
		// 다음달이 내년 1월일 경우.
		cYear = ptm->tm_year - 99;
		cMon  = 1;
	}

	MMatchScheduleData* pScheduleData = m_pScheduler->MakeOnceScheduleData( cYear, cMon, 1, 9, 0, pCmd );
	if( 0 == pScheduleData )
	{
		delete pCmd;
		return false;
	}

	if( !m_pScheduler->AddDynamicSchedule(pScheduleData) )
	{
		delete pCmd;
		delete pScheduleData;
		return false;
	}

	mlog( "MBMatchServer::AddClanServerSwitchUpSchedule - make open clan mode schedule success. next open time is %d-%d-%d %d:%d\n",
		pScheduleData->GetYear(), pScheduleData->GetMonth(), pScheduleData->GetDay(),
		pScheduleData->GetHour(), pScheduleData->GetMin() );

	return true;
}


// 서버 시작시 등록되어야 하는 공지사항 스케쥴.
bool MBMatchServer::AddClanServerAnnounceSchedule()
{
	char szTest[] = "clan mode is closed on 23:50";

	MCommand* pCmd = CreateCommand( MC_MATCH_SCHEDULE_ANNOUNCE_MAKE, MUID(0, 0) );
	if( 0 == pCmd )
		return false;

	MCommandParameterString* pCmdPrmStr = new MCommandParameterString( szTest );
	if( 0 == pCmdPrmStr ){
		delete pCmd;
		return false;
	}
	
	if( !pCmd->AddParameter(pCmdPrmStr) ){
		delete pCmd;
		return false;
	}

	tm* ptm = MMatchGetLocalTime();

	MMatchScheduleData* pScheduleData = m_pScheduler->MakeOnceScheduleData( ptm->tm_year - 100, ptm->tm_mon + 1, GetMaxDay(), 23, 40, pCmd );
	if( 0 == pScheduleData ){
		delete pCmd;
		return false;
	}

	if( !m_pScheduler->AddDynamicSchedule(pScheduleData) ){
		pScheduleData->Release();
		delete pScheduleData;
		return false;
	}
	
	return true;
}



char log_buffer[65535];

void AddStr(const char* pFormat,...)
{
	va_list args;
	char temp[1024];

	va_start(args, pFormat);
	vsprintf(temp, pFormat, args);

	strcat(log_buffer, temp);
	va_end(args);
}

void MBMatchServer::OnViewServerStatus()
{
	MGetServerStatusSingleton()->SaveToLogFile();
}

ULONG MBMatchServer::HShield_MakeGuidReqMsg(unsigned char *pbyGuidReqMsg, unsigned char *pbyGuidReqInfo)
{
#ifdef _HSHIELD
	if( MGetServerConfig()->IsUseHShield() )
		return _AntiCpSvr_MakeGuidReqMsg(pbyGuidReqMsg, pbyGuidReqInfo);
#endif
	return 0L;
}

ULONG MBMatchServer::HShield_AnalyzeGuidAckMsg(unsigned char *pbyGuidAckMsg, unsigned char *pbyGuidReqInfo, unsigned long **ppCrcInfo)
{
#ifdef _HSHIELD
	if( MGetServerConfig()->IsUseHShield() )
		return _AntiCpSvr_AnalyzeGuidAckMsg(pbyGuidAckMsg, pbyGuidReqInfo, ppCrcInfo);
#endif
	return 0L;
}

ULONG MBMatchServer::HShield_MakeReqMsg(unsigned long *pCrcInfo, unsigned char *pbyReqMsg, unsigned char *pbyReqInfo, unsigned long ulOption)
{
#ifdef _HSHIELD
	if( MGetServerConfig()->IsUseHShield() )
		return _AntiCpSvr_MakeReqMsg(pCrcInfo, pbyReqMsg, pbyReqInfo, ulOption);
#endif
	return 0L;
}

ULONG MBMatchServer::HShield_AnalyzeAckMsg(unsigned long *pCrcInfo, unsigned char *pbyAckMsg, unsigned char *pbyReqInfo)
{
#ifdef _HSHIELD
	if( MGetServerConfig()->IsUseHShield() )
		return _AntiCpSvr_AnalyzeAckMsg(pCrcInfo, pbyAckMsg, pbyReqInfo);
#endif
	return 0L;
}

bool MBMatchServer::IsKeeper( const MUID& uidKeeper )
{
	MMatchObject* pObj = GetObject( uidKeeper );
	if( 0 == pObj )
		return false;

	if( !MGetServerConfig()->IsKeeperIP(pObj->GetIPString()) )
	{
		mlog( "Keeper hacking. " );
		if( 0 != pObj->GetIPString() )
			mlog( "IP:%s, ", pObj->GetIPString() );

		if( (0 != pObj->GetCharInfo()) && (0 != pObj->GetCharInfo()->m_szName) )
			mlog( "Name:%s", pObj->GetCharInfo()->m_szName );

		mlog( "\n" );

		return false;
	}

	return true;
}


void MBMatchServer::WriteServerInfoLog()
{
	mlog( "\n" );
	mlog( "================================== Server configure info ==================================\n" );

	char szTemp[256];
	sprintf(szTemp, "Release Date : %s", __DATE__);
	Log(LOG_PROG, szTemp);
	
#ifdef _XTRAP
	if( MGetServerConfig()->IsUseXTrap() )
	{
		LOG( LOG_PROG, "X-Trap On" );
		LOG( LOG_PROG, "X-Trap usable state : (true)" );
	}
	else
		LOG( LOG_PROG, "X-Trap Off" );
#endif

#ifdef _HSHIELD
	if( MGetServerConfig()->IsUseHShield() )
	{
		LOG( LOG_PROG, "Hack Shield On" );
		LOG( LOG_PROG, "Hack Shield usable state : (true)" );
	}
	else
		LOG( LOG_PROG, "Hack Shield Off" );
#endif

#ifndef _DEBUG
	// MD5 확인 하는지 여부 로그로 남긴다.
	if (MGetServerConfig()->IsUseMD5())
	{
		LOG( LOG_PROG, "MD5 Check On" );
	}
	else
	{
		LOG( LOG_PROG, "MD5 Check Off" );
	}
#endif

	if (MGetServerConfig()->IsUseLoopLog())
	{
		LOG(LOG_PROG, "Loop Log Save On - Time Gap : (%u)", MGetServerConfig()->GetLoopTimeGap());
	}
	else
	{
		LOG(LOG_PROG, "Loop Log Save Off");
	}

	if( MC_KOREA == MGetLocale()->GetCountry() )
		LOG( LOG_PROG, "Server Country : KOREA" );
	else if( MC_US == MGetLocale()->GetCountry() )
		LOG( LOG_PROG, "Server Country : US" );
	else if( MC_JAPAN == MGetLocale()->GetCountry() )
		LOG( LOG_PROG, "Server Country : JAPAN" );
	else if( MC_BRAZIL == MGetLocale()->GetCountry() )
		LOG( LOG_PROG, "Server Country : BRAZIL" );
	else if( MC_INDIA == MGetLocale()->GetCountry() )
		LOG( LOG_PROG, "Server Country : INDIA" );
	else if( MC_NHNUSA == MGetLocale()->GetCountry() )
		LOG( LOG_PROG, "Server Country : NHNUSA" );
	else
	{
		ASSERT( 0 && "국가 설정을 해주세요." );
		LOG( LOG_PROG, "!!!!!!!!!Not setted country code!!!!!!!!!!!!!" );
	}
	
	LOG( LOG_PROG, "Command version : (%u)", MCOMMAND_VERSION );
	LOG( LOG_PROG, "Event usable state : (%s)", MGetServerConfig()->IsUseEvent() ? "true" : "false" );
	LOG( LOG_PROG, "Load event size : (%u)", MMatchEventFactoryManager::GetInstance().GetLoadEventSize() );
	LOG( LOG_PROG, "FileCRCCheckSum usable state : (%s)", MGetServerConfig()->IsUseFileCrc() ? "true" : "false" );
	LOG( LOG_PROG, "FileCRC size : (%u)", MMatchAntiHack::GetFielCRCSize() );
	LOG( LOG_PROG, "Country Code Filter usalbe state : (%s)", MGetServerConfig()->IsUseFilter() ? "true" : "false" );
	LOG( LOG_PROG, "Accept Invalied IP state : (%s)", MGetServerConfig()->IsAcceptInvalidIP() ? "true" : "false" );
	LOG( LOG_PROG, "Keeper IP : (%s)", MGetServerConfig()->GetKeeperIP().c_str() );
	in_addr ar;
	ar.S_un.S_addr = MGetServerConfig()->GetMonitorUDPIP();
	LOG( LOG_PROG, "Monitor IP : (%s)", inet_ntoa(ar) );
	LOG( LOG_PROG, "Ticket use : %d\n", MGetServerConfig()->IsUseTicket() );

	mlog( "===========================================================================================\n" );
	mlog( "\n" );
}


bool MBMatchServer::InitHShiled()
{
#ifdef _HSHIELD
	// HSHIELD Init
	if( MGetServerConfig()->IsUseHShield() )
	{
		// HShield Init 지금은 사용하지 않음.
		// if(MPacketHShieldCrypter::Init() != ERROR_SUCCESS)
		// 	bResult = false;
	
		GetCurrentDirectory( sizeof( m_strFileCrcDataPath), m_strFileCrcDataPath);
		strcat( m_strFileCrcDataPath, "\\HackShield.crc");
		ULONG dwRet = _AntiCpSvr_Initialize(m_strFileCrcDataPath);

		if( dwRet != ERROR_SUCCESS )
		{
			AfxMessageBox("_AntiCpSvr_Initialize Failed!");
			return false;
		}

		LOG( LOG_PROG, "HackShield Init OK." );
	}
#endif

	return true;
}

#include "MMatchBuff.h"

void MBMatchServer::OnRun(void)
{
	MMatchServer::OnRun();

	const DWORD dwGlobalClock = GetGlobalClockCount();

	MGetCheckLoopTimeInstance()->SetNHNUSAAuthTick();
#ifdef LOCALE_NHNUSAA
	if( MGetServerConfig()->IsUseNHNUSAAuth() )
	{
		if( GetNHNRTA().IsElapsed(dwGlobalClock) )
		{
			GetNHNRTA().RTA( GetClientCount(), dwGlobalClock );
		}
	}
#endif

	MGetServerStatusSingleton()->SetRunStatus(113);

	MGetCheckLoopTimeInstance()->SetGameGuardTick();
#ifdef _GAMEGUARD
	OnRun_GameGuard(dwGlobalClock);
#endif

	MGetCheckLoopTimeInstance()->SetXTrapTick();
#ifdef _XTRAP
	OnRun_XTrap(dwGlobalClock);
#endif

	MGetServerStatusSingleton()->SetRunStatus(114);

	MGetCheckLoopTimeInstance()->SetMonitorTick();
#ifdef _MONITORING
	m_Monitor.Run( dwGlobalClock );
#endif

	MGetCheckLoopTimeInstance()->SetKillTrackerTick();
	GetKillTracker().Update( dwGlobalClock );
	
	MGetServerStatusSingleton()->SetRunStatus(115);
}

void MBMatchServer::OnRun_GameGuard(const DWORD dwGlobalClock)
{
#ifdef _GAMEGUARD
	if( MGetServerConfig()->IsUseGamegaurd() )
	{
		static DWORD dwLastGameguardCheckTime = dwGlobalClock;
		if( GAMEGUARD_CHECKTIME	 < (dwGlobalClock - dwLastGameguardCheckTime) )
		{
			MMatchObjectList::iterator itGameguard, endGameguard;
			itGameguard = m_Objects.begin();
			endGameguard = m_Objects.end();
			while( itGameguard != endGameguard )
			{
				CheckGameguard( itGameguard->second, dwGlobalClock );
				++itGameguard;
			}
			dwLastGameguardCheckTime = dwGlobalClock;
		}
	}
#endif
}

void MBMatchServer::OnRun_XTrap(const DWORD dwGlobalClock)
{
#ifdef _XTRAP
	DWORD retVal = 0;
	if (MGetServerConfig()->IsUseXTrap())
	{
		MMatchObjectList::iterator iter;
		for (iter = m_Objects.begin(); iter != m_Objects.end(); iter++)
		{
			MMatchObject *pObj = (MMatchObject*)(iter->second);
			if (pObj == NULL)
			{
				continue;
			}

			MBMatchXTrapCC *pXTrapCC = GetXTrapCC(pObj->GetUID());
			if (pXTrapCC == NULL)
			{
				continue;
			}

			// 20초마다 한번씩 호출한다.(로그인이 완료된 유저만, 연결중인 유저만(종료 처리중인 유저 제외))
			DWORD dwTimeGap = dwGlobalClock - pXTrapCC->GetXTrapLastUpdateTime();
			if (pObj->GetDisconnStatusInfo().GetStatus() == MMDS_CONNECTED && pObj->IsLoginCompleted())
			{
				if (dwGlobalClock > pXTrapCC->GetXTrapLastUpdateTime() && dwTimeGap >= MAX_XTRAP_ELAPSEDTIME)
				{
					retVal = pXTrapCC->XTrapGetHashData();
					OnRequestXTrapSeedKey(pObj->GetUID(), pXTrapCC->GetXTrapComBuf());
					pXTrapCC->SetXTrapLastUpdateTime(dwGlobalClock);

					// 오류 처리 (해당 유저의 연결을 종료한다.)
					if (retVal != XTRAP_API_RETURN_OK)
					{
//						pObj->SetXTrapHackerDisconnectWaitInfo();
						pObj->DisconnectHacker( MMHT_XTRAP_HACKER );
						mlog("pXTrapCC->XTrapGetHashData() return value = %d\n", retVal);
						continue;
					}
				}
			}
		}
	}
#endif
}

void MBMatchServer::CheckGameguard( MMatchObject* pObj, const DWORD dwTime )
{
#ifdef _GAMEGUARD
	if( 0 == pObj ) return;

	if( (MMDS_DISCONN_WAIT == pObj->GetDisconnStatusInfo().GetStatus()) || 
		(MMDS_DISCONNECT == pObj->GetDisconnStatusInfo().GetStatus()) )
		return;

	// 접속직후 수행하는 첫인증 과정이 완료되지 않았으면 아직 정기인증을 해선 안된다
	if (!pObj->IsRecvFirstGameguardResponse())
		return;

	MBMatchGameguard* pGameguard = GetGameguard( pObj->GetUID() );
	if( 0 == pGameguard )
	{
		if( !pObj->IsLoginCompleted() )
		{
			// 아직 로긴 작업이 완료되지 않았으므로 그냥 무시함.
			return;
		}

		mlog( "gameguard obj is null point(1). AID(%u)\n", pObj->GetAccountInfo()->m_nAID );

		// 여기까지 오면 로긴 작업이 완료된 후에도 인증 객체가 없으므로 
		// 비정상 유저로 판단.
//		pObj->SetGameguardHackerDisconnectWaitInfo();
		pObj->DisconnectHacker(MMHT_GAMEGUARD_HACKER);
		return;
	}

	if( MAX_ELAPSEDTIME < (dwTime - pGameguard->GetLastCheckTime()) )
	{
		LOG(LOG_PROG,  "GAMEGUARD ERR :elapsed time(%u). AID(%u), CreateCount(%u)\n"
			, dwTime - pGameguard->GetLastCheckTime()
			, pObj->GetAccountInfo()->m_nAID
			, pGameguard->GetCreateAuthCount() );

//		pObj->SetGameguardHackerDisconnectWaitInfo();
		pObj->DisconnectHacker(MMHT_GAMEGUARD_HACKER);
		return;
	}

	if( !pGameguard->CreateAuthQuery() )
	{
		// 아직 클라이언트에서 응답을 기다리고 있음.
		if( ERROR_GGAUTH_NO_REPLY == pGameguard->GetLastError() )
			return;

		// 새로운 인증값을 만드는데 실패하면 접속 종료 시킴.
		// ggtest
		LOG(LOG_PROG,  "GAMEGUARD ERR :err make gameguard auth. AID(%u), ErrorCode(%u), CreateAuthCount(%u)\n"
			, pObj->GetAccountInfo()->m_nAID
			, pGameguard->GetLastError()
			, pGameguard->GetCreateAuthCount() );

//		pObj->SetGameguardHackerDisconnectWaitInfo();
		pObj->DisconnectHacker(MMHT_GAMEGUARD_HACKER);

		return;
	}

	MMatchServer* pServer = MMatchServer::GetInstance();
	if( 0 == pServer )
	{
		ASSERT( 0 );
		mlog( "MMatchObject::CheckGameguard - game server address is null...\n" );
		return;
	}

	const GG_AUTH_DATA& AuthData = pGameguard->GetServerAuth()->GetAuthQuery();
	pServer->RequestGameguardAuth( pObj->GetUID()
		, AuthData.dwIndex
		, AuthData.dwValue1
		, AuthData.dwValue2
		, AuthData.dwValue3 );
#endif
}

bool MBMatchServer::InitSecrity()
{
	// not use. - by SungE 2007-05-03
	//if( !InitHShiled() ) 
	//	return false;

	if( !InitNHNAuth() )
		return false;

	if( !InitGameguard() )
		return false;

	if( !InitXtrap() )
		return false;

	if( !InitGameOn() )
		return false;

#ifdef NEW_AUTH_MODULE
	if( !InitNetmarble() )
		return false;
#endif

	mlog( "success init security.\n" );

	return true;
}

MBMatchXTrapCC *MBMatchServer::GetXTrapCC(const MUID &uidUser)
{
	MBMatchUserSecurityMgr* pSecurityMgr = m_Security.GetManager(MSI_XTRAP);
	if (pSecurityMgr == 0)
	{
		mlog( "XTrapCC ERR :security mgr null point.\n" );
		return 0;
	}

	MBMatchUserSecurityInfo* pSecurityInfo = pSecurityMgr->GetSecurityInfo(uidUser);
	if (pSecurityInfo == 0)
	{
		mlog( "XTrap security info null point\n" );
	}
	return static_cast<MBMatchXTrapCC *>(pSecurityInfo);
}

MBMatchGameguard* MBMatchServer::GetGameguard( const MUID& uidUser )
{
	MBMatchUserSecurityMgr* pSecurityMgr = m_Security.GetManager( MSI_GAMEGUARD );
	if( 0 == pSecurityMgr )
	{
		// ggtest
		LOG(LOG_PROG,  "GAMEGUARD ERR :security mgr null point.\n" );
		return 0;
	}

	MBMatchUserSecurityInfo* pSecurityInfo = pSecurityMgr->GetSecurityInfo( uidUser );

	if( 0 == pSecurityInfo )
		mlog( "GameGuard security info null point\n" );
	
	return static_cast<MBMatchGameguard*>(pSecurityInfo);
}


bool MBMatchServer::DeleteGameguardUserSecurityInfo( const MUID& uidUser )
{
	if( MGetServerConfig()->IsUseGamegaurd() )
	{
		MBMatchUserSecurityMgr* pGameguardMgr = m_Security.GetManager( MSI_GAMEGUARD );
		if( 0 != pGameguardMgr )
			pGameguardMgr->Delete( uidUser );
		else
		{
			mlog( "gameguard security info manager pointer is null...\n" );
			ASSERT( 0 );
			return false;
		}
	}

	return true;
}

void MBMatchServer::OnNetClear( const MUID& CommUID )
{
	// MMatchServer::OnNetClear( CommUID );

    MMatchObject* pObj = GetObject(CommUID);
	if (pObj)
		OnCharClear(pObj->GetUID());

	MAgentObject* pAgent = GetAgent(CommUID);
	if (pAgent)
		AgentRemove(pAgent->GetUID(), NULL);
	
#ifdef _GAMEGUARD
	if( MGetServerConfig()->IsUseGamegaurd() )
		DeleteGameguardUserSecurityInfo( CommUID );
#endif

#ifdef _XTRAP
	if (MGetServerConfig()->IsUseXTrap())
	{
		DeleteXTrapUserSecurityInfo(CommUID);
	}
#endif
	MServer::OnNetClear(CommUID);
}

bool MBMatchServer::PreCheckAddObj(const MUID& uidObj)
{
#ifdef _XTRAP
	if (uidObj == MUID(0, 0))
	{
		LOG(LOG_PROG, "XTRAP ERR : uidObj == MUID(0, 0).\n");
		return false;
	}

	if (MGetServerConfig()->IsUseXTrap())
	{
		MBMatchXTrapCC* pXTrapCC = NULL;
		try
		{
			DWORD dwTick = GetTickCount();
			pXTrapCC = new MBMatchXTrapCC(uidObj, dwTick);
			if (pXTrapCC == NULL)
			{
				LOG(LOG_PROG, "XTRAP ERR : pXTrapCC pointer is NULL.\n");
				return false;
			}
		}
		catch (CMemoryException* e)
		{
			char strError[512] = {0, };
			e->GetErrorMessage(strError, 512);
			LOG(LOG_PROG, "XTRAP ERR : CMemoryException = %s\n", strError);
			return false;
		}

		if (pXTrapCC->XTrapSessionInit() != 0)
		{
			LOG(LOG_PROG, "XTRAP ERR : XTrapSessionInot() false.\n");
			delete pXTrapCC;
			return false;
		}

		MBMatchUserSecurityMgr *pXTrapMgr = m_Security.GetManager(MSI_XTRAP);
		if (pXTrapMgr == NULL)
		{
			LOG(LOG_PROG, "XTRAP ERR : pXTrapMgrsecurity manager is null...\n" );
			delete pXTrapCC;
			return false;
		}

		if (!pXTrapMgr->AddUserSecurityInfo(uidObj, pXTrapCC))
		{
			LOG(LOG_PROG, "XTRAP ERR :add security info fail.\n");
			delete pXTrapCC;
			return false;
		}
		pXTrapCC->SetXTrapLastUpdateTime(GetTickCount());
	}
#endif
	return true;
}

bool MBMatchServer::DeleteXTrapUserSecurityInfo(const MUID& uidUser)
{
	if (MGetServerConfig()->IsUseXTrap())
	{
		MBMatchUserSecurityMgr* pXTrapMgr = m_Security.GetManager(MSI_XTRAP);
		if (pXTrapMgr != 0)
		{
			pXTrapMgr->Delete(uidUser);
		}
		else
		{
			mlog("XTrap security info manager pointer is null...\n");
            ASSERT(0);
            return false;
		}
	}
	return true;
}

void MBMatchServer::OnRequestXTrapSeedKey(const MUID &uidChar, unsigned char *pComBuf)
{
}

void MBMatchServer::XTrap_OnAdminReloadFileHash(const MUID& uidAdmin)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if (!IsEnabledObject(pObj)) return;
	if (!IsAdminGrade(pObj)) return;

#ifdef _XTRAP
	if (MGetServerConfig()->IsUseXTrap())
	{
		ReloadXTrapMapFile();
	}
#endif	
}

bool MBMatchServer::InitXtrap()
{
#ifdef _XTRAP
	InitMemPool(MBMatchXTrapCC);

	if (MGetServerConfig()->IsUseXTrap())
	{
		if (!LoadXTrapFile())
		{
			return false;
		}

		MBMatchUserSecurityMgr* pXTrapMgr = new MBMatchUserSecurityMgr(MSI_XTRAP, "XTrapCS4", 0);
		if (pXTrapMgr == 0)
		{
			mlog("Failed! new MBMatchUserSecurityMgr()\n");
			return false;
		}
		m_Security.AddManager(MSI_XTRAP, pXTrapMgr);
		mlog("XTrap Files Load and Init success!\n");
	}
#endif
	return true;
}


bool MBMatchServer::InitNHNAuth()
{

	// move - by SungE 2007-05-04
#ifdef LOCALE_NHNUSAA
	if( MGetServerConfig()->IsUseNHNUSAAuth() )
	{
		bool bInitAuth = false;
		if( NSM_REAL == MGetServerConfig()->GetNHNServerMode() )
		{
			if( !GetNHNModule().InitAuthReal() )
			{
				mlog( "init nhn usa auth is fail(real).\n" );
				return false;
			}
		}
		else if( NSM_ALPHA == MGetServerConfig()->GetNHNServerMode() )
		{
			if( !GetNHNModule().InitAuthAlpha() )
			{
				mlog( "init nhn usa auth is fail(alpha).\n" );
				return false;
			}
		}

		mlog( "success init nhn auth.\n" );

		if( NSM_REAL == MGetServerConfig()->GetNHNServerMode() )
		{
			if( !GetNHNRTA().InitRTAReal(MGetServerConfig()->GetServerName()) )
			{
				mlog( "init nhn usa report is fail(real).\n" );
				return false;
			}
		}
		else if( NSM_ALPHA == MGetServerConfig()->GetNHNServerMode() )
		{
			if( !GetNHNRTA().InitRTAAlpha(MGetServerConfig()->GetServerName()) )
			{
				mlog( "init nhn usa report is fail(alpha).\n" );
				return false;
			}
		}

		mlog( "success init nhn usa report.\n" );
	}
#endif

	return true;
}


bool MBMatchServer::InitGameguard()
{
#ifdef _GAMEGUARD
	if( MGetServerConfig()->IsUseGamegaurd() )
	{
		const DWORD dwRet = InitMBMatchGameguardAuth(); 
		if( ERROR_SUCCESS != dwRet )
		{
			mlog( "init gameguard fail. err(%u)\n", dwRet );
			return false;
		}
	
		MBMatchUserSecurityMgr* pGameguardMgr = new MBMatchUserSecurityMgr( MSI_GAMEGUARD, "game guard", MAX_ELAPSEDTIME * 2 );
		if( NULL == pGameguardMgr )
		{
			mlog( "Init gameguard security manager fail!\n" );
			return false;
		}

		m_Security.AddManager( MSI_GAMEGUARD, pGameguardMgr );
	}

	mlog( "init gameguard success.\n" );
#endif

	return true;
}


bool MBMatchServer::InitGameOn()
{
#ifdef LOCALE_JAPAN
	if ( GetGameOnModule().InitModule() == false)
	{
		mlog( "init GameOn fail.\n" );
		return false;
	}
#endif

	mlog( "init GameOn success.\n" );
	return true;
}

#ifdef NEW_AUTH_MODULE
bool MBMatchServer::InitNetmarble()
{
#ifdef LOCALE_KOREA
	if ( MGetNetmarbleModule().InitModule() == false)
	{
		mlog( "init Netmarble failed.\n");
		return false;
	}

	mlog( "init Netmarble success.\n");
#endif

	return true;
}
#endif


void MBMatchServer::SafePushMonitorUDP( const DWORD dwIP, const WORD wPort, const char* pData, const DWORD dwDataSize )
{
#ifdef _MONITORING
	if( NULL == pData )
		return;

	m_Monitor.SafePushRecvUDP( dwIP, wPort, pData, dwDataSize );
#endif
}

bool MBMatchServer::SendMonitorUDP(const DWORD dwIP, const USHORT nPort, const string& strMonitorCommand)
{
	char* szMonitorCommand;
	szMonitorCommand = new char[ strMonitorCommand.length() ];
	strncpy( szMonitorCommand, strMonitorCommand.c_str(), strMonitorCommand.length() );
	return m_SafeUDP.Send(dwIP, nPort, szMonitorCommand, static_cast<DWORD>(strMonitorCommand.length()));
}