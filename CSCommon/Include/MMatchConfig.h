#ifndef _MMATCHCONFIG_H
#define _MMATCHCONFIG_H

#include <string>
#include <list>
#include <set>
using namespace std;

#include "MMatchMap.h"
#include "MMatchGlobal.h"
#include "MMatchKillTrackerConfig.h"
#include "MMatchPowerLevelingConfig.h"

enum NHN_SERVERMODE
{
	NSM_ALPHA = 1,
	NSM_REAL,

	NSM_END,
};

class MMatchConfig
{
private:
	char				m_szDB_DNS[64];
	char				m_szDB_UserName[64];
	char				m_szDB_Password[64];

	int					m_nMaxUser;
	int					m_nServerID;
	char				m_szServerName[256];
	int					m_nServerPort;
	int					m_nServerUDPPort;

	MMatchServerMode	m_nServerMode;
	bool				m_bRestrictionMap;
	set<int>			m_EnableMaps;
	list<string>		m_FreeLoginIPList;
	bool				m_bCheckPremiumIP;
	string				m_strCountry;
	string				m_strLanguage;

	bool				m_bEnabledCreateLadderGame;

	bool				m_bEnabledSurvivalMode;
	bool				m_bEnabledDuelTournament;

	char				m_NJ_szDBAgentIP[64];
	int					m_NJ_nDBAgentPort;
	int					m_NJ_nGameCode;

	bool				m_bUseFilter;
	bool				m_bAcceptInvalidIP;

	bool				m_bIsUseHShield;
	bool				m_bIsUseXTrap;
	bool				m_bIsUseEvent;
	bool				m_bIsUseFileCrc;
	bool				m_bIsUseMD5;
	bool				m_bBlockFlooding;

	bool				m_bIsUseBlockHancking;
	bool				m_bIsUseItemConsistency;

	DWORD				m_dwSurvivalRankingDailyRequestHour;
	DWORD				m_dwSurvivalRankingDailyRequestMin;

	DWORD				m_dwDuelTournamentMatchMakingInterval;
	DWORD				m_dwDuelTournamentMatchMaingAcceptableTpGap;
	DWORD				m_dwDuelTournamentMatchMaingWaitLimit;
	DWORD				m_dwDuelTournamentServiceStartTime;
	DWORD				m_dwDuelTournamentServiceEndTime;
	bool				m_bSendLoginUserToDuelTournamentChannel;

	DWORD			m_dwBRDescriptionRefreshInterval;

	list<string>		m_DebugLoginIPList;
	bool				m_bIsDebugServer;

	string				m_strKeeperIP;

	bool				m_bIsUseTicket;

	bool				m_bIsComplete;

	bool				m_bIsUseNHNUSAAuth;
	bool				m_bIsUseGameguard;

	NHN_SERVERMODE		m_NHNServerMode;

	DWORD				m_dwMonitorUDPIP;
	USHORT				m_nUDPPort;

	MMatchKillTrackerConfig		m_KillTrackConfig;
	MMatchPowerLevelingConfig	m_PowerLevelingConfig;

	bool				m_bIsUseResourceCRC32CacheCheck;

	bool m_bUseLoopLog;
	DWORD m_dwMaxLoopTimeGap;

private:
	bool GetPrivateProfileBool(const char* szAppName, const char* szKeyName,
		bool bDefault, const char* szFileName);
	void AddFreeLoginIP(const char* szIP);
	void AddDebugLoginIP(const char* szIP);
	void ReadEnableMaps();
	void TrimStr(const char* szSrcStr, char* outStr);

	bool InitKillTrackerConfig();
	bool InitPowerLevelingConfig();

	const bool LoadMonitorIPnPort();
	const bool LoadKeeperIP();

	void InitLoopLogConfig();

public:
	MMatchConfig();
	virtual ~MMatchConfig();
	static MMatchConfig* GetInstance();
	bool Create();
	void Destroy();
	void Clear();

	const char* GetDB_DNS() { return m_szDB_DNS; }
	const char* GetDB_UserName() { return m_szDB_UserName; }
	const char* GetDB_Password() { return m_szDB_Password; }
	const int GetMaxUser() { return m_nMaxUser; }
	const int GetServerID() { return m_nServerID; }
	const char* GetServerName() { return m_szServerName; }
	const int GetServerPort() { return m_nServerPort; }
	const int GetServerUDPPort() { return m_nServerUDPPort; }
	const MMatchServerMode		GetServerMode() { return m_nServerMode; }
	bool IsResMap() { return m_bRestrictionMap; }
	bool IsEnableMap(const MMATCH_MAP nMap)
	{
		if (!m_bRestrictionMap) return true;
		if (m_EnableMaps.find(nMap) != m_EnableMaps.end()) return true;
		return false;
	}
	const bool IsDebugServer() { return m_bIsDebugServer; }
	bool CheckFreeLoginIPList(const char* pszIP);
	bool IsDebugLoginIPList(const char* pszIP);
	bool CheckPremiumIP() { return m_bCheckPremiumIP; }

	bool IsEnabledCreateLadderGame() { return m_bEnabledCreateLadderGame; }
	void SetEnabledCreateLadderGame(bool bVal) { m_bEnabledCreateLadderGame = bVal; }

	bool IsEnabledSurvivalMode() { return m_bEnabledSurvivalMode; }
	bool IsEnabledDuelTournament() { return m_bEnabledDuelTournament; }

	const char* GetNJDBAgentIP() { return m_NJ_szDBAgentIP; }
	int GetNJDBAgentPort() { return m_NJ_nDBAgentPort; }
	int GetNJDBAgentGameCode() { return m_NJ_nGameCode; }

	const bool IsUseFilter() const { return m_bUseFilter; }
	const bool IsAcceptInvalidIP() const { return m_bAcceptInvalidIP; }
	void SetUseFilterState(const bool bUse) { m_bUseFilter = bUse; }
	void SetAcceptInvalidIPState(const bool bAccept) { m_bAcceptInvalidIP = bAccept; }

	const bool IsUseHShield() const { return m_bIsUseHShield; }
	const bool IsUseXTrap() const { return m_bIsUseXTrap; }
	const bool IsUseEvent() const { return m_bIsUseEvent; }
	const bool IsUseFileCrc() const { return m_bIsUseFileCrc; }
	const bool IsUseMD5() const { return m_bIsUseMD5; }
	const bool IsUseBlockFlooding() const { return m_bBlockFlooding; }

	bool IsKeeperIP(const string& strIP) { return m_strKeeperIP == strIP; }

	const string& GetKeeperIP() { return m_strKeeperIP; }

	const string& GetCountry() { return m_strCountry; }
	const string& GetLanguage() { return m_strLanguage; }

	const bool IsUseTicket() { return m_bIsUseTicket; }

	const bool IsUseGamegaurd() { return m_bIsUseGameguard; }
	const bool IsUseNHNUSAAuth() { return m_bIsUseNHNUSAAuth; }

	const bool IsComplete() { return m_bIsComplete; }

	const NHN_SERVERMODE GetNHNServerMode() { return m_NHNServerMode; }

	const bool IsClanServer() { return (MSM_CLAN == m_nServerMode); }

	const DWORD GetMonitorUDPIP() { return m_dwMonitorUDPIP; }
	const USHORT GetMonitorUDPPORT() { return m_nUDPPort; }

	MMatchKillTrackerConfig& GetKillTrackerConfig() { return m_KillTrackConfig; }
	MMatchPowerLevelingConfig& GetPowerLevelingConfig() { return m_PowerLevelingConfig; }

	const bool IsUseResourceCRC32CacheCheck() { return m_bIsUseResourceCRC32CacheCheck; }

	const bool IsUseLoopLog() { return m_bUseLoopLog; }
	const DWORD GetLoopTimeGap() { return m_dwMaxLoopTimeGap; }

	bool IsBlockHacking() { return m_bIsUseBlockHancking; }
	bool IsItemConsistency() { return m_bIsUseItemConsistency; }

	DWORD GetSurvivalRankingDalyRequestTimeHour() const { return m_dwSurvivalRankingDailyRequestHour; }
	DWORD GetSurvivalRankingDalyRequestTimeMinute() const { return m_dwSurvivalRankingDailyRequestMin; }

	DWORD GetDuelTournamentMatchMakingInterval() const { return m_dwDuelTournamentMatchMakingInterval; }
	DWORD GetDuelTournamentMatchMakingAcceptableTpGap() const { return m_dwDuelTournamentMatchMaingAcceptableTpGap; }
	DWORD GetDuelTournamentMatchMakingWaitLimit() const { return m_dwDuelTournamentMatchMaingWaitLimit; }
	DWORD GetDuelTournamentServiceStartTime() const { return m_dwDuelTournamentServiceStartTime; }
	DWORD GetDuelTournamentServiceEndTime() const { return m_dwDuelTournamentServiceEndTime; }

	DWORD GetBRDescriptionRefreshInterval() const { return m_dwBRDescriptionRefreshInterval; }

	bool IsSendLoginUserToDuelTournamentChannel() const { return m_bSendLoginUserToDuelTournamentChannel; }
};

inline MMatchConfig* MGetServerConfig() { return MMatchConfig::GetInstance(); }

inline bool QuestTestServer() { return true; }

#define SERVER_CONFIG_FILENAME			"./config/server.ini"

#define SERVER_CONFIG_SERVERMODE_NORMAL			"normal"
#define SERVER_CONFIG_SERVERMODE_CLAN			"clan"
#define SERVER_CONFIG_SERVERMODE_LADDER			"ladder"
#define SERVER_CONFIG_SERVERMODE_EVENT			"event"
#define SERVER_CONFIG_SERVERMODE_TEST			"quest"

#define SERVER_CONFIG_DEFAULT_NJ_DBAGENT_IP			"192.168.0.15"
#define SERVER_CONFIG_DEFAULT_NJ_DBAGENT_PORT		7500
#define SERVER_CONFIG_DEFAULT_NJ_DBAGENT_GAMECODE	1013

#define SERVER_CONFIG_DEFAULT_USE_HSHIELD	"0"
#define SERVER_CONFIG_DEFAULT_USE_XTRAP		"0"
#define SERVER_CONFIG_DEFAULT_USE_EVENT		"1"
#define SERVER_CONFIG_DEFAULT_USE_FILECRC	"0"
#define SERVER_CONFIG_DEFAULT_BLOCK_FLOODING "0"
#define SERVER_CONFIG_DEFAULT_USE_KILTRACKER	"0"
#define SERVER_CONFIG_DEFAULT_USE_RESOURCECRC32CACHECHECK "0"

#define SERVER_CONFIG_DEBUG_DEFAULT			"0"

#define SERVER_CONFIG_USE_TICKET			"0"

#endif
