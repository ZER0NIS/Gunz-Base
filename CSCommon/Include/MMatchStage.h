#pragma once

#include <vector>
#include <list>

#include "MMatchItem.h"
#include "MMatchTransDataType.h"
#include "MUID.h"
#include "MMatchRule.h"
#include "MMatchObject.h"
#include "MMatchWorldItem.h"
#include "MMatchActiveTrapMgr.h"
#include "MMatchStageSetting.h"
#include "MVoteMgr.h"
#include "MMatchGlobal.h"
#include "MDuelTournamentGroup.h"

#define CTF_BLUE_ITEM_ID 205
#define CTF_RED_ITEM_ID 206

#define MTICK_STAGE			100

class MMatchObject;
class MMatchStage;
class MMatchServer;
class MLadderGroup;

enum MMatchStageType
{
	MST_NORMAL = 0,
	MST_LADDER,

	MST_MAX
};

struct MMatchStageTeamBonus
{
	bool		bApplyTeamBonus;
};

struct MMatchLadderTeamInfo
{
	int		nTID;
	int		nFirstMemberCount;

	int		nCLID;
	int		nCharLevel;
	int		nContPoint;
};

struct MMatchStageTeam
{
	int						nTeamBonusExp;
	int						nTeamTotalLevel;
	int						nScore;
	int						nSeriesOfVictories;
	int						nTotalKills;
	MMatchLadderTeamInfo	LadderInfo;
};

struct MMatchStageSuicide
{
	MMatchStageSuicide(const MUID& uidUser, const DWORD dwExpireTime)
	{
		m_uidUser = uidUser;
		m_dwExpireTime = dwExpireTime;
		m_bIsChecked = false;
	}

	MUID	m_uidUser;
	DWORD	m_dwExpireTime;
	bool	m_bIsChecked;
};

struct MMATCH_RESOURCECHECKINFO
{
	DWORD	dwLastRequestTime;
	DWORD	dwResourceCRC32Cache;
	DWORD	dwResourceXORCache;
	bool	bIsEnterBattle;
	bool	bIsChecked;
};

struct MMatchDuelTournamentMatch
{
	int nMatchNumber;
	int nNextMatchNumber;

	MUID uidPlayer1;
	MUID uidPlayer2;

	MDUELTOURNAMENTROUNDSTATE nRoundState;
};

struct MMatchDuelTournamentStageInfo
{
	MDUELTOURNAMENTTYPE nDuelTournamentType;

	int nDuelTournamentNumber;
	char szTimeStamp[DUELTOURNAMENT_TIMESTAMP_MAX_LENGTH + 1];

	std::map<int, MMatchDuelTournamentMatch*> DuelTournamentMatchMap;
	int nDuelTournamentTotalRound;
};

typedef std::map<MUID, MMATCH_RESOURCECHECKINFO>	ResourceCRC32CacheMap;

class MMatchCRC32XORCache;

class MMatchStage {
private:
	int						m_nIndex;
	STAGE_STATE				m_nState;
	MMatchStageType			m_nStageType;
	MUID					m_uidStage;
	MUID					m_uidOwnerChannel;
	char					m_szStageName[STAGENAME_LENGTH];
	bool					m_bPrivate;
	char					m_szStagePassword[STAGEPASSWD_LENGTH];
	MMatchStageTeamBonus	m_TeamBonus;
	MMatchStageTeam			m_Teams[MMT_END];

	MUIDRefCache			m_ObjUIDCaches;
	list<int>				m_BanCIDList;

	unsigned long			m_nStateTimer;
	unsigned long			m_nLastTick;
	unsigned long			m_nChecksum;
	unsigned long			m_nLastChecksumTick;
	int						m_nAdminObjectCount;
	unsigned long			m_nStartTime;
	unsigned long			m_nLastRequestStartStageTime;

	MMatchStageSetting		m_StageSetting;
	MMatchRule* m_pRule;

	MUID					m_uidAgent;
	bool					m_bAgentReady;
	int						m_nRoundObjCount[MMT_END];

	MVoteMgr				m_VoteMgr;

	char					m_szFirstMasterName[MATCHOBJECT_NAME_LENGTH];

	ResourceCRC32CacheMap	m_ResourceCRC32CacheMap;
	DWORD					m_dwLastResourceCRC32CacheCheckTime;
	bool					m_bIsUseResourceCRC32CacheCheck;

	vector<RelayMap>		m_vecRelayMapsRemained;
	RELAY_MAP_TYPE			m_RelayMapType;
	RELAY_MAP_REPEAT_COUNT	m_RelayMapRepeatCountRemained;
	bool					m_bIsLastRelayMap;
	bool					m_bIsAntiLeading;
	int						m_nRedCLID;
	int						m_nBlueCLID;

	void SetMasterUID(const MUID& uid) { m_StageSetting.SetMasterUID(uid); }
	MMatchRule* CreateRule(MMATCH_GAMETYPE nGameType);

	vector< MMatchStageSuicide > m_SuicideList;
protected:
	inline bool IsChecksumUpdateTime(unsigned long nTick);
	void UpdateChecksum(unsigned long nTick);
	void OnStartGame();
	void OnFinishGame();
protected:
	friend MMatchServer;
	void SetStageType(MMatchStageType nStageType);
	void SetLadderTeam(MMatchLadderTeamInfo* pRedLadderTeamInfo, MMatchLadderTeamInfo* pBlueLadderTeamInfo);

private:
	const bool SetChannelRuleForCreateStage(bool bIsAllowNullChannel);

public:
	MMatchWorldItemManager	m_WorldItemManager;
	MMatchActiveTrapMgr		m_ActiveTrapManager;

	void UpdateStateTimer();
	unsigned long GetStateTimer() { return m_nStateTimer; }
	unsigned long GetChecksum() { return m_nChecksum; }
	unsigned long GetStartTime() { return m_nStartTime; }
	int GetRedCLID() { return m_nRedCLID; }
	int GetBlueCLID() { return m_nBlueCLID; }
	void OnApplyTeamBonus(MMatchTeam nTeam);
	// void OnApplyTeamBonusCTF(MMatchTeam nTeam, int nCaps);
	void SetCLID(int nCLID, MMatchTeam t) { switch (t) { case MMT_RED: m_nRedCLID = nCLID; break; case MMT_BLUE: m_nBlueCLID = nCLID; break; } }
public:
	MMatchStage();
	virtual ~MMatchStage();

	bool Create(const MUID& uid, const char* pszName, bool bPrivate, const char* pszPassword, bool bIsAllowNullChannel,
		const MMATCH_GAMETYPE GameType = MMATCH_GAMETYPE_DEFAULT, const bool bIsCheckTicket = false, const DWORD dwTicketItemID = 0);

	void Destroy();
	void OnCommand(MCommand* pCommand);
	void OnGameKill(const MUID& uidAttacker, const MUID& uidVictim);
	bool CheckAutoTeamBalancing();
	void ShuffleTeamMembers();

	const char* GetName() { return m_szStageName; }
	const char* GetPassword() { return m_szStagePassword; }
	void SetPassword(const char* pszPassword) { strcpy(m_szStagePassword, pszPassword); }
	const bool IsPrivate() { return m_bPrivate; }
	void SetPrivate(bool bVal) { m_bPrivate = bVal; }
	MUID GetUID() { return m_uidStage; }

	const char* GetMapName() { return m_StageSetting.GetMapName(); }
	bool SetMapName(char* pszMapName);

	char* GetFirstMasterName() { return m_szFirstMasterName; }
	void SetFirstMasterName(char* pszName) { strcpy(m_szFirstMasterName, pszName); }

	MMatchObject* GetObj(const MUID& uid) { if (m_ObjUIDCaches.count(uid) == 0) return NULL; else return (MMatchObject*)(m_ObjUIDCaches[uid]); }
	size_t GetObjCount() { return m_ObjUIDCaches.size(); }
	int GetPlayers();
	MUIDRefCache::iterator GetObjBegin() { return m_ObjUIDCaches.begin(); }
	MUIDRefCache::iterator GetObjEnd() { return m_ObjUIDCaches.end(); }
	int GetObjInBattleCount();
	int GetCountableObjCount() { return ((int)GetObjCount() - m_nAdminObjectCount); }

	void AddBanList(int nCID);
	bool CheckBanList(int nCID);

	void AddObject(const MUID& uid, const MMatchObject* pObj);
	MUIDRefCache::iterator RemoveObject(const MUID& uid);
	bool KickBanPlayer(const char* pszName, bool bBanPlayer = true);

	const MUID RecommandMaster(bool bInBattleOnly);
	void EnterBattle(MMatchObject* pObj);
	void LeaveBattle(MMatchObject* pObj);

	STAGE_STATE GetState() { return m_nState; }
	void ChangeState(STAGE_STATE nState) { m_nState = nState; UpdateStateTimer(); }

	bool CheckTick(unsigned long nClock);
	void Tick(unsigned long nClock);

	MMatchStageSetting* GetStageSetting() { return &m_StageSetting; }

	MMatchRule* GetRule() { return m_pRule; }
	void ChangeRule(MMATCH_GAMETYPE nRule);
	void GetTeamMemberCount(int* poutnRedTeamMember, int* poutnBlueTeamMember, int* poutSpecMember, bool bInBattle);
	MMatchTeam GetRecommandedTeam();

	MVoteMgr* GetVoteMgr() { return &m_VoteMgr; }

	MUID GetAgentUID() { return m_uidAgent; }
	void SetAgentUID(MUID uid) { m_uidAgent = uid; }
	bool GetAgentReady() { return m_bAgentReady; }
	void SetAgentReady(bool bReady) { m_bAgentReady = bReady; }

	MUID GetMasterUID() { return m_StageSetting.GetMasterUID(); }
	int GetIndex() { return m_nIndex; }

	void SetOwnerChannel(MUID& uidOwnerChannel, int nIndex);
	MUID GetOwnerChannel() { return m_uidOwnerChannel; }

	void PlayerTeam(const MUID& uidPlayer, MMatchTeam nTeam);
	void PlayerState(const MUID& uidPlayer, MMatchObjectStageState nStageState);
	bool StartGame(const bool bIsUseResourceCRC32CacheCheck);
	bool StartRelayGame(const bool bIsUseResourceCRC32CacheCheck);
	bool FinishGame();
	bool CheckBattleEntry();

	void RoundStateFromClient(const MUID& uidStage, int nState, int nRound);
	void ObtainWorldItem(MMatchObject* pObj, const int nItemID);
	void ObtainWorldItemCTF(MMatchObject* pObj, const int nItemID);
	void RequestSpawnWorldItem(MMatchObject* pObj, const int nItemID,
		const float x, const float y, const float z, float fDropDelayTime);
	void SpawnServerSideWorldItem(MMatchObject* pObj, const int nItemID,
		const float x, const float y, const float z,
		int nLifeTime, int* pnExtraValues);
	void OnNotifyThrowTrapItem(const MUID& uidPlayer, const int nItemID);
	void OnNotifyActivatedTrapItem(const MUID& uidPlayer, const int nItemID, const MVector3& pos);

	bool IsApplyTeamBonus();
	void AddTeamBonus(int nExp, MMatchTeam nTeam);
	void ResetTeamBonus();
	int GetTeamScore(MMatchTeam nTeam) { return m_Teams[nTeam].nScore; }

	void InitTeamKills() { m_Teams[MMT_BLUE].nTotalKills = m_Teams[MMT_RED].nTotalKills = 0; }
	int GetTeamKills(MMatchTeam nTeam) { return m_Teams[nTeam].nTotalKills; }
	void AddTeamKills(MMatchTeam nTeam, int amount = 1) { m_Teams[nTeam].nTotalKills += amount; }

	const MMatchStageType GetStageType() { return m_nStageType; }
	int GetMinPlayerLevel();

	bool CheckUserWasVoted(const MUID& uidPlayer);

	bool CheckDuelMap();
	bool CheckTicket(MMatchObject* pObj);
	bool CheckQuestGame();

	bool IsRelayMap() { return m_StageSetting.IsRelayMap(); }
	bool IsStartRelayMap() { return m_StageSetting.IsStartRelayMap(); }
	int						GetRelayMapListCount() { return m_StageSetting.GetRelayMapListCount(); }
	const RelayMap* GetRelayMapList() { return m_StageSetting.GetRelayMapList(); }
	RELAY_MAP_TYPE			GetRelayMapType() { return m_StageSetting.GetRelayMapType(); }
	RELAY_MAP_REPEAT_COUNT  GetRelayMapRepeatCount() { return m_StageSetting.GetRelayMapRepeatCount(); }
	bool IsAntiLead() { return m_StageSetting.GetAntiLead(); }

	void InitCurrRelayMap();

	void SetIsRelayMap(bool bIsRelayMap) { m_StageSetting.SetIsRelayMap(bIsRelayMap); }
	void SetIsStartRelayMap(bool bVal) { m_StageSetting.SetIsStartRelayMap(bVal); }
	void SetRelayMapType(RELAY_MAP_TYPE nRelayMapType) { m_StageSetting.SetRelayMapType(nRelayMapType); }
	void SetRelayMapRepeatCount(RELAY_MAP_REPEAT_COUNT nRelayRepeatCount) { m_StageSetting.SetRelayMapRepeatCount(nRelayRepeatCount); }
	void SetRelayMapCurrList(const RelayMap* relayMapList);
	void SetRelayMapListCount(int nRelayMapListCount);
	void SetRelayMapList(RelayMap* RelayMapList);
	void SetAntiLead(bool bAntiLead) { m_StageSetting.SetAntiLead(bAntiLead); }

public:
	void OnRoundEnd_FromTeamGame(MMatchTeam nWinnerTeam);
	void OnInitRound();

public:
	void ReserveSuicide(const MUID& uidUser, const DWORD dwExpireTime);
	void CheckSuicideReserve(const DWORD dwCurTime);

public:
	void ResetPlayersCustomItem();
	void ClearGabageObject();

private:
	MChannelRule* GetStageChannelRule();
	bool			IsValidMap(const char* pMapName);
	bool			IsClanServer();

	void			MakeResourceCRC32Cache(const DWORD dwKey, DWORD& out_crc32, DWORD& out_xor);
	void			MakeItemResourceCRC32Cache(MMatchCRC32XORCache& CRC32Cache);
	void			SetResourceCRC32Cache(const MUID& uidPlayer, const DWORD dwCRC32Cache, const DWORD dwXORCache);
	void			RequestResourceCRC32Cache(const MUID& uidPlayer);
	void			DeleteResourceCRC32Cache(const MUID& uidPlayer);
	const bool 		IsValidResourceCRC32Cache(const MUID& uidPlayer, const DWORD dwResourceCRC32Cache, const DWORD dwResourceXORCache);
	void			SetDisableCheckResourceCRC32Cache(const MUID& uidPlayer);
	void			SetDisableAllCheckResourceCRC32Cache();

	void			CheckResourceCRC32Cache(const DWORD dwClock);

private:
	MMatchDuelTournamentStageInfo m_nDTStageInfo;

	void MakeDuelTournamentMatchMap(MDUELTOURNAMENTROUNDSTATE nRoundState, int nMatchNumber);
	void ClearDuelTournamentMatchMap();

	MDUELTOURNAMENTROUNDSTATE GetDuelTournamentRoundState(MDUELTOURNAMENTTYPE nType);
	MDUELTOURNAMENTROUNDSTATE GetDuelTournamentNextRoundState(MDUELTOURNAMENTROUNDSTATE nRoundState);

	int GetDuelTournamentNextOrder(MDUELTOURNAMENTROUNDSTATE nRoundState, int nOrder, int nTemp);
public:
	void SetDuelTournamentMatchList(MDUELTOURNAMENTTYPE nType, MDuelTournamentPickedGroup* pPickedGroup);
	void SetDuelTournamentMatchNumber(int nValue) { m_nDTStageInfo.nDuelTournamentNumber = nValue; }
	void SetDuelTournamentMatchTimeStamp(char* szTimeStamp) { strcpy(m_nDTStageInfo.szTimeStamp, szTimeStamp); }

	const MDUELTOURNAMENTTYPE GetDuelTournamentType() { return m_nDTStageInfo.nDuelTournamentType; }
	const int	GetDuelTournamentTotalRound() { return m_nDTStageInfo.nDuelTournamentTotalRound; }
	const int	GetDuelTournamentMatchNumber() { return m_nDTStageInfo.nDuelTournamentNumber; }
	const char* GetDuelTournamentMatchTimeStamp() { return m_nDTStageInfo.szTimeStamp; }
	std::map<int, MMatchDuelTournamentMatch*>* GetDuelTournamentMatchMap() { return &(m_nDTStageInfo.DuelTournamentMatchMap); }

	int	GetDuelTournamentRandomMapIndex();

private:
	unsigned int m_nGameLogID;

public:
	void SetGameLogID(unsigned int nVal) { m_nGameLogID = nVal; }
	unsigned int GetGameLogID() { return m_nGameLogID; }
};

inline MDUELTOURNAMENTROUNDSTATE MMatchStage::GetDuelTournamentNextRoundState(MDUELTOURNAMENTROUNDSTATE nRoundState)
{
	switch (nRoundState) {
	case MDUELTOURNAMENTROUNDSTATE_QUATERFINAL:		return MDUELTOURNAMENTROUNDSTATE_SEMIFINAL;
	case MDUELTOURNAMENTROUNDSTATE_SEMIFINAL:			return MDUELTOURNAMENTROUNDSTATE_FINAL;
	default:											return MDUELTOURNAMENTROUNDSTATE_MAX;
	}
}

inline MDUELTOURNAMENTROUNDSTATE MMatchStage::GetDuelTournamentRoundState(MDUELTOURNAMENTTYPE nType)
{
	switch (nType) {
	case MDUELTOURNAMENTTYPE_QUATERFINAL:		return MDUELTOURNAMENTROUNDSTATE_QUATERFINAL;
	case MDUELTOURNAMENTTYPE_SEMIFINAL:		return MDUELTOURNAMENTROUNDSTATE_SEMIFINAL;
	case MDUELTOURNAMENTTYPE_FINAL:			return MDUELTOURNAMENTROUNDSTATE_FINAL;
	default: ASSERT(0);
	}

	return MDUELTOURNAMENTROUNDSTATE_MAX;
}

class MMatchStageMap : public std::map<MUID, MMatchStage*> {
	MUID	m_uidGenerate;
public:
	MMatchStageMap() { m_uidGenerate = MUID(0, 0); }
	virtual ~MMatchStageMap() {	}
	MUID UseUID() { m_uidGenerate.Increase();	return m_uidGenerate; }
	void Insert(const MUID& uid, MMatchStage* pStage) { insert(value_type(uid, pStage)); }
};

MMatchItemBonusType GetStageBonusType(MMatchStageSetting* pStageSetting);

#define TRANS_STAGELIST_NODE_COUNT	8