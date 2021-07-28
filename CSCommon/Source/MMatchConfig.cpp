#include "stdafx.h"
#include "MMatchConfig.h"
#include <windows.h>
#include "MMatchMap.h"
#include "MLex.h"
#include "MZFileSystem.h"
#include "MErrorTable.h"
#include "MMatchServer.h"
#include "MInetUtil.h"
#include "MMatchCheckLoopTime.h"

bool MMatchConfig::GetPrivateProfileBool(const char* szAppName, const char* szKeyName,
	bool bDefault, const char* szFileName)
{
	int nDefault = bDefault ? 1 : 0;

	int n;
	n = GetPrivateProfileInt(szAppName, szKeyName, nDefault, szFileName);
	if (n == 0) return false;
	return true;
}

MMatchConfig::MMatchConfig()
{
	m_nMaxUser = 0;
	m_szDB_DNS[0] = '\0';
	m_szDB_UserName[0] = '\0';
	m_szDB_Password[0] = '\0';
	m_nServerID = 0;
	m_szServerName[0] = '\0';
	m_nServerPort = 0;
	m_nServerUDPPort = 0;
	m_nServerMode = MSM_NORMALS;
	m_bRestrictionMap = false;
	m_bCheckPremiumIP = false;
	m_bUseFilter = false;
	m_bAcceptInvalidIP = false;
	m_bIsDebugServer = false;
	m_bEnabledCreateLadderGame = true;
	m_bEnabledSurvivalMode = true;
	m_bEnabledDuelTournament = true;
	m_bIsComplete = false;
	m_bIsUseGameguard = false;
	m_bIsUseNHNUSAAuth = false;
	m_dwMonitorUDPIP = 0;
	m_nUDPPort = 0;
	m_bIsUseResourceCRC32CacheCheck = false;
	m_bUseLoopLog = false;
	m_dwMaxLoopTimeGap = 0;

	m_dwSurvivalRankingDailyRequestHour = 5;
	m_dwSurvivalRankingDailyRequestMin = 0;

	m_dwDuelTournamentMatchMakingInterval = 1000;
	m_dwDuelTournamentMatchMaingAcceptableTpGap = 10;
	m_dwDuelTournamentMatchMaingWaitLimit = 10000;
	m_dwDuelTournamentServiceStartTime = 0;
	m_dwDuelTournamentServiceEndTime = 23;
	m_bSendLoginUserToDuelTournamentChannel = true;

	m_dwBRDescriptionRefreshInterval = 5 * 60 * 1000;
}

MMatchConfig::~MMatchConfig()
{
}

MMatchConfig* MMatchConfig::GetInstance()
{
	static MMatchConfig m_MatchConfig;
	return &m_MatchConfig;
}

bool MMatchConfig::InitPowerLevelingConfig()
{
	char szUsePowerLevelingDBBlock[2] = { 0, };;
	char szPowerLevelingDBBlockTime[8] = { 0, };

	GetPrivateProfileString("POWERLEVELING", "USE_POWERLEVELING_DB_BLOCK", "0", szUsePowerLevelingDBBlock, 2, SERVER_CONFIG_FILENAME);
	if (0 == _stricmp("1", szUsePowerLevelingDBBlock))
		m_PowerLevelingConfig.UsePowerLevelingDBBlock();

	GetPrivateProfileString("POWERLEVELING", "POWERLEVELING_DB_BLOCKTIME", "10", szPowerLevelingDBBlockTime, 8, SERVER_CONFIG_FILENAME);
	m_PowerLevelingConfig.SetPowerLevelingDBBlockTime(static_cast<DWORD>(atol(szPowerLevelingDBBlockTime)));

	return true;
}

bool MMatchConfig::InitKillTrackerConfig()
{
	char szUseKillTracker[2] = { 0, };
	char szMaxKillCountOnTraceTime[8] = { 0, };
	char szKillCountTraceTime[8] = { 0, };

	GetPrivateProfileString("KILLTRACKER", "USE_KILLTRACKER", "0", szUseKillTracker, 2, SERVER_CONFIG_FILENAME);
	if (0 == _stricmp("1", szUseKillTracker))
		m_KillTrackConfig.UseKillTracker();

	GetPrivateProfileString("KILLTRACKER", "MAXKILLCOUNT_ON_TRACETIME", "40", szMaxKillCountOnTraceTime, 8, SERVER_CONFIG_FILENAME);
	m_KillTrackConfig.SetMaxKillCountOnTraceTime(static_cast<DWORD>(atol(szMaxKillCountOnTraceTime)));

	GetPrivateProfileString("KILLTRACKER", "KILLCOUNT_TRACETIME", "10", szKillCountTraceTime, 8, SERVER_CONFIG_FILENAME);
	m_KillTrackConfig.SetKillCountTraceTime(static_cast<DWORD>(atol(szKillCountTraceTime)));

	return true;
}

bool MMatchConfig::Create()
{
	int nTemp = 0;

	GetPrivateProfileString("DB", "DNS", "gunzdb", m_szDB_DNS, 64, SERVER_CONFIG_FILENAME);
	GetPrivateProfileString("DB", "USERNAME", "gunzdb", m_szDB_UserName, 64, SERVER_CONFIG_FILENAME);
	GetPrivateProfileString("DB", "PASSWORD", "gunzdb", m_szDB_Password, 64, SERVER_CONFIG_FILENAME);

	m_nMaxUser = GetPrivateProfileInt("SERVER", "MAXUSER", 1500, SERVER_CONFIG_FILENAME);
	m_nServerID = GetPrivateProfileInt("SERVER", "SERVERID", 1500, SERVER_CONFIG_FILENAME);
	GetPrivateProfileString("SERVER", "SERVERNAME", "matchserver", m_szServerName, 256, SERVER_CONFIG_FILENAME);
	m_nServerPort = GetPrivateProfileInt("SERVER", "SERVERPORT", 6000, SERVER_CONFIG_FILENAME);
	m_nServerUDPPort = GetPrivateProfileInt("SERVER", "SERVERUDPPORT", 7777, SERVER_CONFIG_FILENAME);

	char szServerMode[128] = "";
	GetPrivateProfileString("SERVER", "MODE", SERVER_CONFIG_SERVERMODE_NORMAL, szServerMode, 128, SERVER_CONFIG_FILENAME);

	if (!_stricmp(szServerMode, SERVER_CONFIG_SERVERMODE_NORMAL)) m_nServerMode = MSM_NORMALS;
	else if (!_stricmp(szServerMode, SERVER_CONFIG_SERVERMODE_CLAN)) m_nServerMode = MSM_CLAN;
	else if (!_stricmp(szServerMode, SERVER_CONFIG_SERVERMODE_LADDER)) m_nServerMode = MSM_LADDER;
	else if (!_stricmp(szServerMode, SERVER_CONFIG_SERVERMODE_EVENT)) m_nServerMode = MSM_EVENT;
	else if (!_stricmp(szServerMode, SERVER_CONFIG_SERVERMODE_TEST)) m_nServerMode = MSM_QUEST;

	m_bEnabledSurvivalMode = (0 != GetPrivateProfileInt("SERVER", "SURVIVALENABLE", 1, SERVER_CONFIG_FILENAME));

	m_dwSurvivalRankingDailyRequestHour = GetPrivateProfileInt("SERVER", "SURVIVALRANKING_DAILY_REQUEST_HOUR", 5, SERVER_CONFIG_FILENAME);
	m_dwSurvivalRankingDailyRequestMin = GetPrivateProfileInt("SERVER", "SURVIVALRANKING_DAILY_REQUEST_MINUTE", 0, SERVER_CONFIG_FILENAME);
	if (m_dwSurvivalRankingDailyRequestHour < 0 || m_dwSurvivalRankingDailyRequestHour >= 24) {
		mlog("[ASSERTION FAILED!] %s : SURVIVALRANKING_DAILY_REQUEST_HOUR invalid!\n", SERVER_CONFIG_FILENAME);
	}
	if (m_dwSurvivalRankingDailyRequestMin < 0 || m_dwSurvivalRankingDailyRequestMin >= 60) {
		mlog("[ASSERTION FAILED!] %s : SURVIVALRANKING_DAILY_REQUEST_MINUTE invalid!\n", SERVER_CONFIG_FILENAME);
	}

	m_dwDuelTournamentMatchMakingInterval = GetPrivateProfileInt("SERVER", "DUELTOURNAMENT_MATCHMAKING_INTERVAL", 1000, SERVER_CONFIG_FILENAME);
	m_dwDuelTournamentMatchMaingAcceptableTpGap = GetPrivateProfileInt("SERVER", "DUELTOURNAMENT_MATCHMAKING_ACCEPTABLE_TP_GAP", 10, SERVER_CONFIG_FILENAME);
	m_dwDuelTournamentMatchMaingWaitLimit = GetPrivateProfileInt("SERVER", "DUELTOURNAMENT_MATCHMAKING_WAIT_LIMIT", 10000, SERVER_CONFIG_FILENAME);
	if (m_dwDuelTournamentMatchMakingInterval > 10000)
		mlog("[WARNING] %s : DUELTOURNAMENT_MATCHMAKING_INTERVAL is too big.\n", SERVER_CONFIG_FILENAME);
	if (m_dwDuelTournamentMatchMaingAcceptableTpGap > 1000)
		mlog("[WARNING] %s : DUELTOURNAMENT_MATCHMAKING_ACCEPTABLE_TP_GAP is too big.\n", SERVER_CONFIG_FILENAME);
	if (m_dwDuelTournamentMatchMaingWaitLimit > 60000)
		mlog("[WARNING] %s : DUELTOURNAMENT_MATCHMAKING_WAIT_LIMIT is too big.\n", SERVER_CONFIG_FILENAME);

	m_dwDuelTournamentServiceStartTime = GetPrivateProfileInt("SERVER", "DUELTOURNAMENT_SERVICE_START_TIME ", 0, SERVER_CONFIG_FILENAME);
	m_dwDuelTournamentServiceEndTime = GetPrivateProfileInt("SERVER", "DUELTOURNAMENT_SERVICE_END_TIME ", 23, SERVER_CONFIG_FILENAME);
	if (m_dwDuelTournamentServiceStartTime > 23) {
		m_dwDuelTournamentServiceStartTime = 23;
		mlog("[WARNING] %s : DUELTOURNAMENT_SERVICE_START_TIME is too big. max is 23.\n", SERVER_CONFIG_FILENAME);
	}
	if (m_dwDuelTournamentServiceEndTime > 23) {
		m_dwDuelTournamentServiceEndTime = 23;
		mlog("[WARNING] %s : DUELTOURNAMENT_SERVICE_END_TIME is too big. max is 23.\n", SERVER_CONFIG_FILENAME);
	}
	if (0 > m_dwDuelTournamentServiceStartTime) {
		m_dwDuelTournamentServiceStartTime = 0;
		mlog("[WARNING] %s : DUELTOURNAMENT_SERVICE_START_TIME must be a positive value.\n", SERVER_CONFIG_FILENAME);
	}
	if (0 > m_dwDuelTournamentServiceEndTime) {
		m_dwDuelTournamentServiceEndTime = 0;
		mlog("[WARNING] %s : DUELTOURNAMENT_SERVICE_END_TIME must be a positive value.\n", SERVER_CONFIG_FILENAME);
	}

	m_bSendLoginUserToDuelTournamentChannel = (0 != GetPrivateProfileInt("SERVER", "SEND_LOGINUSER_TO_DUELTOURNAMENT_CHANNEL", 1, SERVER_CONFIG_FILENAME));

	m_bEnabledDuelTournament = (0 != GetPrivateProfileInt("SERVER", "DUELTOURNAMENT_ENABLE", 1, SERVER_CONFIG_FILENAME));

	m_dwBRDescriptionRefreshInterval = GetPrivateProfileInt("BATTLETIMEREWARD", "BATTLETIMEREWARD_REFRESH_INTERVAL", 5, SERVER_CONFIG_FILENAME);
	m_dwBRDescriptionRefreshInterval = m_dwBRDescriptionRefreshInterval * 60 * 1000;

	char szAllowIP[1024] = "";
	char* pNextArg = szAllowIP;
	GetPrivateProfileString("SERVER", "FREELOGINIP", "", szAllowIP, 1024, SERVER_CONFIG_FILENAME);
	MLex lex;
	while (true) {
		char szIP[128] = "";
		pNextArg = lex.GetOneArg(pNextArg, szIP);
		if (*szIP == NULL)
			break;
		AddFreeLoginIP(szIP);
	}

	char szDebug[4] = { 0, };
	GetPrivateProfileString("SERVER", "DEBUG", SERVER_CONFIG_DEBUG_DEFAULT, szDebug, 4, SERVER_CONFIG_FILENAME);
	if (0 == _stricmp("0", szDebug))
		m_bIsDebugServer = false;
	else
		m_bIsDebugServer = true;

	char szDebugIP[1024] = { 0, };
	char* pNextDbgIP = szDebugIP;
	GetPrivateProfileString("SERVER", "DEBUGIP", "", szDebugIP, 1024, SERVER_CONFIG_FILENAME);
	while (true) {
		char szIP[128] = "";
		pNextDbgIP = lex.GetOneArg(pNextDbgIP, szIP);
		if (*szIP == NULL)
			break;
		AddDebugLoginIP(szIP);
	}

	if (!LoadMonitorIPnPort())
	{
		mlog("server.ini - monitor ip not setting\n");
		return false;
	}

	if (!LoadKeeperIP())
	{
		mlog("server.ini - Keeper ip not setting\n");
		return false;
	}

	int nCheckPremiumIP = GetPrivateProfileInt("SERVER", "CheckPremiumIP", 0, SERVER_CONFIG_FILENAME);
	if (nCheckPremiumIP != 0) m_bCheckPremiumIP = true;

	char szCountry[32] = "";
	GetPrivateProfileString("SERVER", "COUNTRY", "", szCountry, 31, SERVER_CONFIG_FILENAME);
	if (0 != strlen(szCountry))
		m_strCountry = szCountry;
	else
	{
		ASSERT(0);
		mlog("server.ini - Invalid country type.\n");
		return false;
	}

	char szLanguage[32] = "";
	GetPrivateProfileString("SERVER", "LANGUAGE", "", szLanguage, 31, SERVER_CONFIG_FILENAME);
	if (0 != strlen(szLanguage))
		m_strLanguage = szLanguage;
	else
	{
		ASSERT(0);
		mlog("server.ini - Invalid language type.\n");
		return false;
	}

	char szIsUseTicket[2] = "";
	GetPrivateProfileString("SERVER", "USETICKET", SERVER_CONFIG_USE_TICKET, szIsUseTicket, 2, SERVER_CONFIG_FILENAME);
	if (0 != strlen(szIsUseTicket))
		m_bIsUseTicket = static_cast<bool>(atoi(szIsUseTicket));
	else
	{
		ASSERT(0);
		mlog("server.ini - invalid ticket setting.\n");
		return false;
	}

	char szGameguard[2] = "";
	GetPrivateProfileString("SERVER", "GAMEGUARD", "1", szGameguard, 2, SERVER_CONFIG_FILENAME);
	if (1 == atoi(szGameguard))
		m_bIsUseGameguard = true;
	else
	{
		mlog("gameguard not running\n");
		m_bIsUseGameguard = false;
	}

	char szNHNUSAAuth[2] = "";
	GetPrivateProfileString("SERVER", "NHNUSA_AUTH", "1", szNHNUSAAuth, 2, SERVER_CONFIG_FILENAME);
	if (1 == atoi(szNHNUSAAuth))
		m_bIsUseNHNUSAAuth = true;
	else
	{
		m_bIsUseNHNUSAAuth = false;
		mlog("nhn usa auth not running.\n");
	}

	char szNHNServerMode[2] = "";
	GetPrivateProfileString("SERVER", "NHN_SERVERMODE", "r", szNHNServerMode, 2, SERVER_CONFIG_FILENAME);
	if (0 == _stricmp("r", szNHNServerMode))
	{
		m_NHNServerMode = NSM_REAL;
	}
	else if (0 == _stricmp("a", szNHNServerMode))
	{
		m_NHNServerMode = NSM_ALPHA;
	}

	GetPrivateProfileString("LOCALE", "DBAgentIP", SERVER_CONFIG_DEFAULT_NJ_DBAGENT_IP, m_NJ_szDBAgentIP, 64, SERVER_CONFIG_FILENAME);
	m_NJ_nDBAgentPort = GetPrivateProfileInt("LOCALE", "DBAgentPort", SERVER_CONFIG_DEFAULT_NJ_DBAGENT_PORT, SERVER_CONFIG_FILENAME);
	m_NJ_nGameCode = GetPrivateProfileInt("LOCALE", "GameCode", SERVER_CONFIG_DEFAULT_NJ_DBAGENT_GAMECODE, SERVER_CONFIG_FILENAME);

	ReadEnableMaps();

	char szUse[2] = { 0, };
	GetPrivateProfileString("FILTER", "USE", "0", szUse, 2, SERVER_CONFIG_FILENAME);
	SetUseFilterState(atoi(szUse));

	char szAccept[2] = { 0, };
	GetPrivateProfileString("FILTER", "ACCEPT_INVALID_IP", "1", szAccept, 2, SERVER_CONFIG_FILENAME);
	SetAcceptInvalidIPState(atoi(szAccept));

	char szUseHShield[2] = { 0, };
	GetPrivateProfileString("ENVIRONMENT", "USE_HSHIELD", SERVER_CONFIG_DEFAULT_USE_HSHIELD, szUseHShield, 2, SERVER_CONFIG_FILENAME);
	ASSERT(0 != strlen(szUseHShield));
	if (0 == _stricmp("1", szUseHShield))
		m_bIsUseHShield = true;
	else
		m_bIsUseHShield = false;

	char szUseXTrap[2] = { 0, };
	GetPrivateProfileString("ENVIRONMENT", "USE_XTRAP", SERVER_CONFIG_DEFAULT_USE_XTRAP, szUseXTrap, 2, SERVER_CONFIG_FILENAME);
	ASSERT(0 != strlen(szUseXTrap));
	if (0 == _stricmp("1", szUseXTrap))
		m_bIsUseXTrap = true;
	else
		m_bIsUseXTrap = false;

	char szUseEvent[2] = { 0, };
	GetPrivateProfileString("ENVIRONMENT", "USE_EVENT", SERVER_CONFIG_DEFAULT_USE_EVENT, szUseEvent, 2, SERVER_CONFIG_FILENAME);
	ASSERT(0 != strlen(szUseEvent));
	if (0 == _stricmp("1", szUseEvent))
		m_bIsUseEvent = true;
	else
		m_bIsUseEvent = false;

	char szUseFileCrc[2] = { 0, };
	GetPrivateProfileString("ENVIRONMENT", "USE_FILECRC", SERVER_CONFIG_DEFAULT_USE_FILECRC, szUseFileCrc, 2, SERVER_CONFIG_FILENAME);
	ASSERT(0 != strlen(szUseFileCrc));
	if (0 == _stricmp("1", szUseFileCrc))
		m_bIsUseFileCrc = true;
	else
		m_bIsUseFileCrc = false;

	char szUseMD5[2] = { 0, };
	GetPrivateProfileString("ENVIRONMENT", "USE_MD5", SERVER_CONFIG_DEFAULT_USE_FILECRC, szUseMD5, 2, SERVER_CONFIG_FILENAME);
	ASSERT(0 != strlen(szUseMD5));
	if (0 == _stricmp("1", szUseMD5))
		m_bIsUseMD5 = true;
	else
		m_bIsUseMD5 = false;

	char szBlockFlooding[2] = { 0, };
	GetPrivateProfileString("ENVIRONMENT", "BLOCK_FLOODING", SERVER_CONFIG_DEFAULT_BLOCK_FLOODING, szBlockFlooding, 2, SERVER_CONFIG_FILENAME);
	ASSERT(0 != strlen(szBlockFlooding));
	if (0 == _stricmp("1", szBlockFlooding))
		m_bBlockFlooding = true;
	else
		m_bBlockFlooding = false;

	if (m_bIsUseHShield && m_bIsUseXTrap)
	{
		ASSERT(0 && "hackshield와 x-trap은 같이 사용할 수 없다.");
		mlog("server.ini - HackShield and XTrap is duplicated\n");
		return false;
	}

	char szBlockHacking[2] = { 0, };
	GetPrivateProfileString("ENVIRONMENT", "USE_BLOCKHACKING", 0, szBlockHacking, 2, SERVER_CONFIG_FILENAME);
	ASSERT(0 != strlen(szBlockHacking));
	if (0 == _stricmp("1", szBlockHacking))
		m_bIsUseBlockHancking = true;
	else
		m_bIsUseBlockHancking = false;

	char szItemConsistency[2] = { 0, };
	GetPrivateProfileString("ENVIRONMENT", "USE_ITEM_CONSISTENCY", 0, szItemConsistency, 2, SERVER_CONFIG_FILENAME);
	ASSERT(0 != strlen(szItemConsistency));
	if (0 == _stricmp("1", szItemConsistency))	m_bIsUseItemConsistency = true;
	else										m_bIsUseItemConsistency = false;

	InitKillTrackerConfig();
	InitPowerLevelingConfig();

	char szUseResourceCRC32CacheCheck[2] = { 0, };
	GetPrivateProfileString("ENVIRONMENT"
		, "USE_RESOURCECRC32CACHECKECK"
		, SERVER_CONFIG_DEFAULT_USE_RESOURCECRC32CACHECHECK
		, szUseResourceCRC32CacheCheck, 2, SERVER_CONFIG_FILENAME);
	ASSERT(0 != strlen(szUseResourceCRC32CacheCheck));
	if (0 == _stricmp("1", szUseResourceCRC32CacheCheck))
		m_bIsUseResourceCRC32CacheCheck = true;
	else
		m_bIsUseResourceCRC32CacheCheck = false;

	InitLoopLogConfig();

	m_bIsComplete = true;
	return m_bIsComplete;
}

const bool MMatchConfig::LoadMonitorIPnPort()
{
	char szMonitorIP[1024] = "";
	string strRealIP;
	GetPrivateProfileString("SERVER", "MONITORIP", "", szMonitorIP, 32, SERVER_CONFIG_FILENAME);
	if (0 == strlen(szMonitorIP))
	{
		return false;
	}

	if (isalpha(szMonitorIP[0]))
	{
		if (!MGetIPbyHostName(szMonitorIP, strRealIP))
		{
			return false;
		}
	}
	else
	{
		strRealIP = szMonitorIP;
	}

	m_dwMonitorUDPIP = inet_addr(strRealIP.c_str());

	m_nUDPPort = GetPrivateProfileInt("SERVER", "MONITORPORT", 9001, SERVER_CONFIG_FILENAME);

	return true;
}

const bool MMatchConfig::LoadKeeperIP()
{
	char szKeeperIP[1024] = "";
	string strRealIP;
	GetPrivateProfileString("SERVER", "KEEPERIP", "", szKeeperIP, 32, SERVER_CONFIG_FILENAME);
	if (0 == strlen(szKeeperIP))
	{
		return false;
	}

	if (isalpha(szKeeperIP[0]))
	{
		if (!MGetIPbyHostName(szKeeperIP, strRealIP))
		{
			return false;
		}
	}
	else
	{
		strRealIP = szKeeperIP;
	}

	m_strKeeperIP = strRealIP;

	return true;
}

void MMatchConfig::InitLoopLogConfig()
{
	char szUseLoopLog[2] = { 0, };
	GetPrivateProfileString("LOOPLOG", "USE_LOOPLOG", "0", szUseLoopLog, 2, SERVER_CONFIG_FILENAME);
	if (_stricmp("1", szUseLoopLog) == 0)
	{
		m_bUseLoopLog = true;
		MGetCheckLoopTimeInstance()->SetLoopCheckRun();
	}
	else
	{
		m_bUseLoopLog = false;
	}

	m_dwMaxLoopTimeGap = GetPrivateProfileInt("LOOPLOG", "MAX_LOOP_TIME_GAP", 0, SERVER_CONFIG_FILENAME);
}

void MMatchConfig::Destroy()
{
}

void MMatchConfig::AddFreeLoginIP(const char* pszIP)
{
	m_FreeLoginIPList.push_back(pszIP);
}

void MMatchConfig::AddDebugLoginIP(const char* szIP)
{
	m_DebugLoginIPList.push_back(szIP);
}

bool MMatchConfig::CheckFreeLoginIPList(const char* pszIP)
{
	list<string>::iterator end = m_FreeLoginIPList.end();
	for (list<string>::iterator i = m_FreeLoginIPList.begin(); i != end; i++) {
		const char* pszFreeIP = (*i).c_str();
		if (strncmp(pszIP, pszFreeIP, strlen(pszFreeIP)) == 0) {
			return true;
		}
	}
	return false;
}

bool MMatchConfig::IsDebugLoginIPList(const char* pszIP)
{
	list< string >::iterator it, end;
	end = m_DebugLoginIPList.end();
	for (it = m_DebugLoginIPList.begin(); it != end; ++it)
	{
		const char* pszFreeIP = (*it).c_str();
		if (strncmp(pszIP, pszFreeIP, strlen(pszFreeIP)) == 0) {
			return true;
		}
	}
	return false;
}

void MMatchConfig::ReadEnableMaps()
{
	char szEnableMap[512];
	GetPrivateProfileString("SERVER", "EnableMap", "", szEnableMap, 512, SERVER_CONFIG_FILENAME);

	char seps[] = ";";
	char* token;

	int nMapCount = 0;

	token = strtok(szEnableMap, seps);
	while (token != NULL)
	{
		char szInputMapName[256] = "";
		TrimStr(token, szInputMapName);

		int nMapIndex = -1;
		for (int i = 0; i < MMATCH_MAP_MAX; i++)
		{
			if (!_stricmp(szInputMapName, MGetMapDescMgr()->GetMapName(i)))
			{
				nMapIndex = i;
				break;
			}
		}
		if (nMapIndex != -1)
		{
			nMapCount++;
			m_EnableMaps.insert(set<int>::value_type(nMapIndex));
		}

		token = strtok(NULL, seps);
	}

	if (nMapCount <= 0)
	{
		for (int i = 0; i < MMATCH_MAP_MAX; i++) m_EnableMaps.insert(set<int>::value_type(i));
		m_bRestrictionMap = false;
	}
	else
	{
		m_bRestrictionMap = true;
	}
}
void MMatchConfig::TrimStr(const char* szSrcStr, char* outStr)
{
	char szInputMapName[256] = "";

	int nSrcStrLen = (int)strlen(szSrcStr);
	for (int i = 0; i < nSrcStrLen; i++)
	{
		if (!isspace(szSrcStr[i]))
		{
			strcpy(szInputMapName, &szSrcStr[i]);
			break;
		}
	}
	int nLen = (int)strlen(szInputMapName);
	for (int i = nLen - 1; i >= 0; i--)
	{
		if (isspace(szInputMapName[i]))
		{
			szInputMapName[i] = '\0';
		}
		else
		{
			break;
		}
	}

	strcpy(outStr, szInputMapName);
}

void MMatchConfig::Clear()
{
	memset(m_szDB_DNS, 0, 64);
	memset(m_szDB_UserName, 0, 64);
	memset(m_szDB_Password, 0, 64);

	m_nMaxUser = 0;
	m_nServerID = 0;
	memset(m_szServerName, 0, 256);
	m_nServerPort = 0;
	m_nServerUDPPort = 0;

	m_bRestrictionMap = false;
	m_EnableMaps.clear();
	m_FreeLoginIPList.clear();
	m_bCheckPremiumIP = true;

	m_bEnabledCreateLadderGame = false;

	m_bEnabledSurvivalMode = true;
	m_bEnabledDuelTournament = true;

	memset(m_NJ_szDBAgentIP, 0, 64);
	m_NJ_nDBAgentPort = 0;
	m_NJ_nGameCode = 0;

	m_bUseFilter = false;
	m_bAcceptInvalidIP = true;

	m_dwSurvivalRankingDailyRequestHour = 5;
	m_dwSurvivalRankingDailyRequestMin = 0;

	m_dwDuelTournamentMatchMakingInterval = 1000;
	m_dwDuelTournamentMatchMaingAcceptableTpGap = 10;
	m_dwDuelTournamentMatchMaingWaitLimit = 10000;
	m_dwDuelTournamentServiceStartTime = 0;
	m_dwDuelTournamentServiceEndTime = 23;

	m_bSendLoginUserToDuelTournamentChannel = true;
}