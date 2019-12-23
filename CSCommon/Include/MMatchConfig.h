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
	char				m_szDB_DNS[64];				///< DB DNS
	char				m_szDB_UserName[64];		///< DB Username
	char				m_szDB_Password[64];		///< DB Password

	int					m_nMaxUser;					///< 최대 접속자
	int					m_nServerID;
	char				m_szServerName[256];		///< 서버이름
	int					m_nServerPort;				///< 서버포트
	int					m_nServerUDPPort;			///< 서버UDP포트

	MMatchServerMode	m_nServerMode;				///< 서버모드
	bool				m_bRestrictionMap;			///< 맵제한이 있는지 여부 - default : false
	set<int>			m_EnableMaps;				///< 맵제한이 있을경우 가능한 맵
	list<string>		m_FreeLoginIPList;			///< 접속인원 무시 IP
	bool				m_bCheckPremiumIP;			///< 프리미엄 IP 체크
	string				m_strCountry;				
	string				m_strLanguage;

	// enabled 씨리즈 - ini에서 관리하지 않는다.
	bool				m_bEnabledCreateLadderGame;	///< 클랜전 생성가능한지 여부

	// ini에서 관리하는 enabled
	bool				m_bEnabledSurvivalMode;		///< 서바이벌 모드 활성화
	bool				m_bEnabledDuelTournament;	///< 듀얼 토너먼트 활성화

	// -- 일본 넷마블 전용
	char				m_NJ_szDBAgentIP[64];
	int					m_NJ_nDBAgentPort;
	int					m_NJ_nGameCode;

	// filter 사용 설정.
	bool				m_bUseFilter;				/// 필터 사용을 설정함(0:사용않함. 1:사용.)
	bool				m_bAcceptInvalidIP;			/// 리스트에 없는 IP허용 여부.(0:허용하지 않음. 1:허용.)

	// evironment.
	bool				m_bIsUseHShield;
	bool				m_bIsUseXTrap;
	bool				m_bIsUseEvent;
	bool				m_bIsUseFileCrc;
	bool				m_bIsUseMD5;
	bool				m_bBlockFlooding;			///< 커맨드 flooding 이 일어나면 블럭한다


	bool				m_bIsUseBlockHancking;
	bool				m_bIsUseItemConsistency;

	// 서바이벌 랭킹 리스트 받아오는 시각
	DWORD				m_dwSurvivalRankingDailyRequestHour;		// 0~23
	DWORD				m_dwSurvivalRankingDailyRequestMin;			// 0~59

	// 듀얼토너먼트 설정값
	DWORD				m_dwDuelTournamentMatchMakingInterval;			// 몇초마다 매칭 작업을 수행할 것인가
	DWORD				m_dwDuelTournamentMatchMaingAcceptableTpGap;	// 기본적으로 TP 차이가 이 값 이하인 유저끼리 매칭이 가능하다
	DWORD				m_dwDuelTournamentMatchMaingWaitLimit;			// 이 시간보다 오래 대기한 유저는 TP차이를 무시하고 강제 매칭해준다
	DWORD				m_dwDuelTournamentServiceStartTime;				// 듀얼토너먼트 서비스 시작 시간(하루 기준) 설정 ex)0~23이면 24시간 모두 서비스한다.
	DWORD				m_dwDuelTournamentServiceEndTime;				// 듀얼토너먼트 서비스 끝나는 시간(하루 기준) 설정
	bool				m_bSendLoginUserToDuelTournamentChannel;		// 로그인한 유저를 듀얼토너먼트 채널로 보낼 것인가
	

	// BattleTime Reward 관련 설정값
	DWORD			m_dwBRDescriptionRefreshInterval;


	// debug.
	list<string>		m_DebugLoginIPList;			///< Debug용 IP.
	bool				m_bIsDebugServer;

	// keeper ip.
	string				m_strKeeperIP;				/// Keeper와 server와의 통신에서 키퍼의 요청인지 검사하기 위해서.

	bool				m_bIsUseTicket;				/// 입장권 유료아이템이 적용유무 설정.(0:일반서버, 1:유료 입장권 사용 서버)

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
	void AddDebugLoginIP( const char* szIP );
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

	// get
	const char* GetDB_DNS()							{ return m_szDB_DNS; }
	const char* GetDB_UserName()					{ return m_szDB_UserName; }
	const char* GetDB_Password()					{ return m_szDB_Password; }
	const int GetMaxUser()							{ return m_nMaxUser; }
	const int GetServerID()							{ return m_nServerID; }
	const char* GetServerName()						{ return m_szServerName; }
	const int GetServerPort()						{ return m_nServerPort; }
	const int GetServerUDPPort()					{ return m_nServerUDPPort; }
	const MMatchServerMode		GetServerMode()		{ return m_nServerMode; }
	bool IsResMap()									{ return m_bRestrictionMap; }	// 맵제한이 있는지 여부
	bool IsEnableMap(const MMATCH_MAP nMap)										// 플레이가능한 맵인지 여부
	{
		if (!m_bRestrictionMap) return true;
		if (m_EnableMaps.find(nMap) != m_EnableMaps.end()) return true;
		return false;
	}
	const bool IsDebugServer()							{ return m_bIsDebugServer; }
	bool CheckFreeLoginIPList(const char* pszIP);
	bool IsDebugLoginIPList( const char* pszIP );
	bool CheckPremiumIP()							{ return m_bCheckPremiumIP; }

	bool IsEnabledCreateLadderGame()				{ return m_bEnabledCreateLadderGame; }
	void SetEnabledCreateLadderGame(bool bVal)		{ m_bEnabledCreateLadderGame = bVal; }

	bool IsEnabledSurvivalMode()					{ return m_bEnabledSurvivalMode; }
	bool IsEnabledDuelTournament()					{ return m_bEnabledDuelTournament; }

	const char* GetNJDBAgentIP()					{ return m_NJ_szDBAgentIP; }
	int GetNJDBAgentPort()							{ return m_NJ_nDBAgentPort; }
	int GetNJDBAgentGameCode()						{ return m_NJ_nGameCode; }

	const bool IsUseFilter() const						{ return m_bUseFilter; }
	const bool IsAcceptInvalidIP() const				{ return m_bAcceptInvalidIP; }
	void SetUseFilterState( const bool bUse )			{ m_bUseFilter = bUse; }
	void SetAcceptInvalidIPState( const bool bAccept )	{ m_bAcceptInvalidIP = bAccept; }

	const bool IsUseHShield() const	{ return m_bIsUseHShield; }
	const bool IsUseXTrap() const	{ return m_bIsUseXTrap; }
	const bool IsUseEvent() const	{ return m_bIsUseEvent; }
	const bool IsUseFileCrc() const { return m_bIsUseFileCrc; }
	const bool IsUseMD5() const { return m_bIsUseMD5; }
	const bool IsUseBlockFlooding() const { return m_bBlockFlooding; }

	bool IsKeeperIP( const string& strIP )				{ return m_strKeeperIP == strIP; }

	const string& GetKeeperIP() { return m_strKeeperIP; }

	const string& GetCountry()	{ return m_strCountry; }
	const string& GetLanguage() { return m_strLanguage; }

	const bool IsUseTicket()	{ return m_bIsUseTicket; }

	const bool IsUseGamegaurd() { return m_bIsUseGameguard; }
	const bool IsUseNHNUSAAuth()	{ return m_bIsUseNHNUSAAuth; }

	const bool IsComplete() { return m_bIsComplete; }

	const NHN_SERVERMODE GetNHNServerMode() { return m_NHNServerMode; }

	const bool IsClanServer() { return (MSM_CLAN == m_nServerMode); }

	const DWORD GetMonitorUDPIP() { return m_dwMonitorUDPIP; }
	const USHORT GetMonitorUDPPORT()	{ return m_nUDPPort; }

	MMatchKillTrackerConfig& GetKillTrackerConfig() { return m_KillTrackConfig; }
	MMatchPowerLevelingConfig& GetPowerLevelingConfig() { return m_PowerLevelingConfig; }

	const bool IsUseResourceCRC32CacheCheck() { return m_bIsUseResourceCRC32CacheCheck; }

	const bool IsUseLoopLog()		{ return m_bUseLoopLog; }
	const DWORD GetLoopTimeGap()	{ return m_dwMaxLoopTimeGap; }

	bool IsBlockHacking()		{ return m_bIsUseBlockHancking;   }
	bool IsItemConsistency()	{ return m_bIsUseItemConsistency; }

	DWORD GetSurvivalRankingDalyRequestTimeHour() const		{ return m_dwSurvivalRankingDailyRequestHour; }
	DWORD GetSurvivalRankingDalyRequestTimeMinute() const	{ return m_dwSurvivalRankingDailyRequestMin; }

	DWORD GetDuelTournamentMatchMakingInterval() const			{ return m_dwDuelTournamentMatchMakingInterval; }
	DWORD GetDuelTournamentMatchMakingAcceptableTpGap() const	{ return m_dwDuelTournamentMatchMaingAcceptableTpGap; }
	DWORD GetDuelTournamentMatchMakingWaitLimit() const			{ return m_dwDuelTournamentMatchMaingWaitLimit; }
	DWORD GetDuelTournamentServiceStartTime() const				{ return m_dwDuelTournamentServiceStartTime; }
	DWORD GetDuelTournamentServiceEndTime() const					{ return m_dwDuelTournamentServiceEndTime; }

	DWORD GetBRDescriptionRefreshInterval() const			{ return m_dwBRDescriptionRefreshInterval; }

	bool IsSendLoginUserToDuelTournamentChannel() const  { return m_bSendLoginUserToDuelTournamentChannel; }
};

inline MMatchConfig* MGetServerConfig() { return MMatchConfig::GetInstance(); }

inline bool QuestTestServer() { return (MGetServerConfig()->GetServerMode() == MSM_QUEST); }


#define SERVER_CONFIG_FILENAME			"./server.ini"


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
