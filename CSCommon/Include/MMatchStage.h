#ifndef MMATCHSTAGE_H
#define MMATCHSTAGE_H

#include <vector>
#include <list>
using namespace std;

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

// 스테이지 타입
enum MMatchStageType
{
	MST_NORMAL	= 0,			// 일반
	MST_LADDER,					// 레더나 클랜게임
	
	MST_MAX
};

struct MMatchStageTeamBonus
{
	bool		bApplyTeamBonus;		// 해당 게임에서 경험치 보너스가 적용되는지 여부
};

// Ladder전일경우 래더팀 정보
struct MMatchLadderTeamInfo
{
	int		nTID;				// Ladder Team ID
	int		nFirstMemberCount;	// 초기멤버수

	// Clan전에서만 사용하는 변수
	int		nCLID;				// 클랜ID
	int		nCharLevel;			// 캐릭터 평균 레벨
	int		nContPoint;			// 캐릭터 기여도 평균
};


// Stage에서 사용하는 팀의 추가정보
struct MMatchStageTeam
{
	int						nTeamBonusExp;			// 누적된 팀 경험치
	int						nTeamTotalLevel;		// 팀원들의 레벨 총합 - 팀보너스 배분시 사용한다
	int						nScore;					// 스코어
	int						nSeriesOfVictories;		// 연승정보
	int						nTotalKills;			// 누적 팀 킬수 (라운드 시작, 즉 게임 시작때 초기화 - 무한팀데스매치를 위함)
	MMatchLadderTeamInfo	LadderInfo;
};


struct MMatchStageSuicide
{
	MMatchStageSuicide( const MUID& uidUser, const DWORD dwExpireTime )
	{
		m_uidUser		= uidUser;
		m_dwExpireTime	= dwExpireTime;
		m_bIsChecked	= false;
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

	map<int, MMatchDuelTournamentMatch*> DuelTournamentMatchMap;
	int nDuelTournamentTotalRound;
};

typedef map<MUID, MMATCH_RESOURCECHECKINFO>	ResourceCRC32CacheMap;


class MMatchCRC32XORCache;


// 스테이지 메인 클래스 - 스테이지와 관련된 일을 총괄한다.
class MMatchStage {
private:
	int						m_nIndex;
	STAGE_STATE				m_nState;
	MMatchStageType			m_nStageType;
	MUID					m_uidStage;
	MUID					m_uidOwnerChannel;
	char					m_szStageName[ STAGENAME_LENGTH ];
	bool					m_bPrivate;		// 비밀방
	char					m_szStagePassword[ STAGEPASSWD_LENGTH ];
	MMatchStageTeamBonus	m_TeamBonus;
	MMatchStageTeam			m_Teams[MMT_END];

	MUIDRefCache			m_ObjUIDCaches;
	list<int>				m_BanCIDList;

	unsigned long			m_nStateTimer;
	unsigned long			m_nLastTick;
	unsigned long			m_nChecksum;					// 목록및 정보 갱신용
	unsigned long			m_nLastChecksumTick;
	int						m_nAdminObjectCount;
	unsigned long			m_nStartTime;					// 게임 시작한 시간
	unsigned long			m_nLastRequestStartStageTime;

	
	MMatchStageSetting		m_StageSetting;
	MMatchRule*				m_pRule;

	MUID					m_uidAgent;
	bool					m_bAgentReady;
	int						m_nRoundObjCount[MMT_END];

	MVoteMgr				m_VoteMgr;

	char					m_szFirstMasterName[MATCHOBJECT_NAME_LENGTH];

	ResourceCRC32CacheMap	m_ResourceCRC32CacheMap;
	DWORD					m_dwLastResourceCRC32CacheCheckTime;
	bool					m_bIsUseResourceCRC32CacheCheck;
	
	// 릴레이맵
	vector<RelayMap>		m_vecRelayMapsRemained;			// 현재 남은 맵들
	RELAY_MAP_TYPE			m_RelayMapType;
	RELAY_MAP_REPEAT_COUNT	m_RelayMapRepeatCountRemained;	// 앞으로 남은 반복 수
	bool					m_bIsLastRelayMap;
	bool					m_bIsAntiLeading;

	void SetMasterUID(const MUID& uid)	{ m_StageSetting.SetMasterUID(uid);}
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


private :
	const bool SetChannelRuleForCreateStage(bool bIsAllowNullChannel);


public:
	MMatchWorldItemManager	m_WorldItemManager;
	MMatchActiveTrapMgr		m_ActiveTrapManager;

	void UpdateStateTimer();
	unsigned long GetStateTimer()	{ return m_nStateTimer; }
	unsigned long GetChecksum()		{ return m_nChecksum; }
	unsigned long GetStartTime()	{ return m_nStartTime; }		///< 게임 시작한 시간
	void OnApplyTeamBonus(MMatchTeam nTeam);
	void OnApplyTeamBonusCTF(MMatchTeam nTeam, int nCaps);
public:
	MMatchStage();
	virtual ~MMatchStage();

	bool Create(const MUID& uid, const char* pszName, bool bPrivate, const char* pszPassword, bool bIsAllowNullChannel, 
		const MMATCH_GAMETYPE GameType = MMATCH_GAMETYPE_DEFAULT, const bool bIsCheckTicket = false, const DWORD dwTicketItemID = 0);

	void Destroy();
	void OnCommand(MCommand* pCommand);
	void OnGameKill(const MUID& uidAttacker, const MUID& uidVictim);
	bool CheckAutoTeamBalancing();	// true이면 팀원을 섞어야 한다.
	void ShuffleTeamMembers();


	const char* GetName()		{ return m_szStageName; }
	const char* GetPassword()	{ return m_szStagePassword; }
	void SetPassword(const char* pszPassword)	{ strcpy(m_szStagePassword, pszPassword); }
	const bool IsPrivate()		{ return m_bPrivate; }
	void SetPrivate(bool bVal)	{ m_bPrivate = bVal; }
	MUID GetUID()				{ return m_uidStage; }

	const char* GetMapName()	{ return m_StageSetting.GetMapName(); }
	bool SetMapName(char* pszMapName); // 	{ m_StageSetting.SetMapName(pszMapName); }
	
	char* GetFirstMasterName()	{ return m_szFirstMasterName; }
	void SetFirstMasterName(char* pszName)	{ strcpy(m_szFirstMasterName, pszName); }

	MMatchObject* GetObj(const MUID& uid)	{ if (m_ObjUIDCaches.count(uid) == 0) return NULL; else return (MMatchObject*)(m_ObjUIDCaches[uid]); }			///< 추가by 동섭, 듀얼을 위해 -_-
	size_t GetObjCount()					{ return m_ObjUIDCaches.size(); }
	int GetPlayers();
	MUIDRefCache::iterator GetObjBegin()	{ return m_ObjUIDCaches.begin(); }
	MUIDRefCache::iterator GetObjEnd()		{ return m_ObjUIDCaches.end(); }
	int GetObjInBattleCount();															///< 전투하고 있는 플레이어수
	int GetCountableObjCount()				{ return ((int)GetObjCount() - m_nAdminObjectCount); }	///< 운영자를 제외한 플레이어수


	void AddBanList(int nCID);
	bool CheckBanList(int nCID);

	void AddObject(const MUID& uid, const MMatchObject* pObj);
	MUIDRefCache::iterator RemoveObject(const MUID& uid);
	bool KickBanPlayer(const char* pszName, bool bBanPlayer=true);

	const MUID RecommandMaster(bool bInBattleOnly);
	void EnterBattle(MMatchObject* pObj);
	void LeaveBattle(MMatchObject* pObj);

	STAGE_STATE GetState()					{ return m_nState; }
	void ChangeState(STAGE_STATE nState)	{ m_nState = nState; UpdateStateTimer(); }

	bool CheckTick(unsigned long nClock);
	void Tick(unsigned long nClock);

	MMatchStageSetting* GetStageSetting() { return &m_StageSetting; }

	MMatchRule* GetRule()			{ return m_pRule; }
	void ChangeRule(MMATCH_GAMETYPE nRule);
	void GetTeamMemberCount(int* poutnRedTeamMember, int* poutnBlueTeamMember, int* poutSpecMember, bool bInBattle);
	MMatchTeam GetRecommandedTeam();

	MVoteMgr* GetVoteMgr()			{ return &m_VoteMgr; }

	MUID GetAgentUID()				{ return m_uidAgent; }
	void SetAgentUID(MUID uid)		{ m_uidAgent = uid; }
	bool GetAgentReady()			{ return m_bAgentReady; }
	void SetAgentReady(bool bReady)	{ m_bAgentReady = bReady; }

	MUID GetMasterUID()				{ return m_StageSetting.GetMasterUID(); }
	int GetIndex()					{ return m_nIndex; }

	void SetOwnerChannel(MUID& uidOwnerChannel, int nIndex);
	MUID GetOwnerChannel() { return m_uidOwnerChannel; }

	void PlayerTeam(const MUID& uidPlayer, MMatchTeam nTeam);
	void PlayerState(const MUID& uidPlayer, MMatchObjectStageState nStageState);
	bool StartGame( const bool bIsUseResourceCRC32CacheCheck );
	bool StartRelayGame( const bool bIsUseResourceCRC32CacheCheck );
	bool FinishGame();
	bool CheckBattleEntry();

	void RoundStateFromClient(const MUID& uidStage, int nState, int nRound);
	void ObtainWorldItem(MMatchObject* pObj, const int nItemID);
	void ObtainWorldItemCTF(MMatchObject* pObj, const int nItemID);
	void RequestSpawnWorldItem(MMatchObject* pObj, const int nItemID, 
							   const float x, const float y, const float z, float fDropDelayTime);
	void SpawnServerSideWorldItem(MMatchObject* pObj, const int nItemID, 
							   const float x, const float y, const float z, 
							   int nLifeTime, int* pnExtraValues );
	void OnNotifyThrowTrapItem(const MUID& uidPlayer, const int nItemID);
	void OnNotifyActivatedTrapItem(const MUID& uidPlayer, const int nItemID, const MVector3& pos);


	bool IsApplyTeamBonus();	// 팀전 보너스 적용여부 확인
	void AddTeamBonus(int nExp, MMatchTeam nTeam);
	void ResetTeamBonus();
	int GetTeamScore(MMatchTeam nTeam)		{ return m_Teams[nTeam].nScore; }

	void InitTeamKills()									{ m_Teams[MMT_BLUE].nTotalKills = m_Teams[MMT_RED].nTotalKills = 0; }
	int GetTeamKills(MMatchTeam nTeam)						{ return m_Teams[nTeam].nTotalKills; }
	void AddTeamKills(MMatchTeam nTeam, int amount=1)		{ m_Teams[nTeam].nTotalKills+=amount; }	

	const MMatchStageType GetStageType()	{ return m_nStageType; }
	int GetMinPlayerLevel();	// 방에 있는 플레이어중 최소 레벨을 구한다.

	bool CheckUserWasVoted( const MUID& uidPlayer );

	bool CheckDuelMap();
	bool CheckTicket( MMatchObject* pObj );
	bool CheckQuestGame();

	// 릴레이맵
	bool IsRelayMap()									{ return m_StageSetting.IsRelayMap(); }
	bool IsStartRelayMap()								{ return m_StageSetting.IsStartRelayMap(); }
	int						GetRelayMapListCount()		{ return m_StageSetting.GetRelayMapListCount(); }
	const RelayMap*			GetRelayMapList()			{ return m_StageSetting.GetRelayMapList(); }
	RELAY_MAP_TYPE			GetRelayMapType()			{ return m_StageSetting.GetRelayMapType(); }
	RELAY_MAP_REPEAT_COUNT  GetRelayMapRepeatCount()	{ return m_StageSetting.GetRelayMapRepeatCount(); }
	bool IsAntiLead()									{ return m_StageSetting.GetAntiLead(); }

	void InitCurrRelayMap();

	void SetIsRelayMap(bool bIsRelayMap)									{ m_StageSetting.SetIsRelayMap(bIsRelayMap); }	
	void SetIsStartRelayMap(bool bVal)										{ m_StageSetting.SetIsStartRelayMap(bVal); }
	void SetRelayMapType(RELAY_MAP_TYPE nRelayMapType)						{ m_StageSetting.SetRelayMapType(nRelayMapType); }
	void SetRelayMapRepeatCount(RELAY_MAP_REPEAT_COUNT nRelayRepeatCount)	{ m_StageSetting.SetRelayMapRepeatCount(nRelayRepeatCount); }
	void SetRelayMapCurrList(const RelayMap* relayMapList);
	void SetRelayMapListCount(int nRelayMapListCount);
	void SetRelayMapList(RelayMap* RelayMapList);	
	void SetAntiLead(bool bAntiLead)									{ m_StageSetting.SetAntiLead(bAntiLead); }	
	

public:
	// Rule에서 호출하는 함수들
	void OnRoundEnd_FromTeamGame(MMatchTeam nWinnerTeam);
	void OnInitRound();			// 라운드 시작시 Rule클래스에서 호출

public :
	// suicide.
	void ReserveSuicide( const MUID& uidUser, const DWORD dwExpireTime );
	void CheckSuicideReserve( const DWORD dwCurTime );

public :
	void ResetPlayersCustomItem();
	void ClearGabageObject();

private :
	MChannelRule*	GetStageChannelRule();
	bool			IsValidMap( const char* pMapName );
	bool			IsClanServer();

	void			MakeResourceCRC32Cache( const DWORD dwKey, DWORD& out_crc32, DWORD& out_xor );
	void			MakeItemResourceCRC32Cache( MMatchCRC32XORCache& CRC32Cache );
	void			SetResourceCRC32Cache( const MUID& uidPlayer, const DWORD dwCRC32Cache, const DWORD dwXORCache );
	void			RequestResourceCRC32Cache( const MUID& uidPlayer );
	void			DeleteResourceCRC32Cache( const MUID& uidPlayer );
	const bool 		IsValidResourceCRC32Cache( const MUID& uidPlayer, const DWORD dwResourceCRC32Cache, const DWORD dwResourceXORCache);
	void			SetDisableCheckResourceCRC32Cache( const MUID& uidPlayer );
	void			SetDisableAllCheckResourceCRC32Cache();

	void			CheckResourceCRC32Cache( const DWORD dwClock );

private:
	MMatchDuelTournamentStageInfo m_nDTStageInfo;

	void MakeDuelTournamentMatchMap(MDUELTOURNAMENTROUNDSTATE nRoundState, int nMatchNumber);
	void ClearDuelTournamentMatchMap();

	MDUELTOURNAMENTROUNDSTATE GetDuelTournamentRoundState(MDUELTOURNAMENTTYPE nType);
	MDUELTOURNAMENTROUNDSTATE GetDuelTournamentNextRoundState(MDUELTOURNAMENTROUNDSTATE nRoundState);

	int GetDuelTournamentNextOrder(MDUELTOURNAMENTROUNDSTATE nRoundState, int nOrder, int nTemp);
public:
	void SetDuelTournamentMatchList(MDUELTOURNAMENTTYPE nType, MDuelTournamentPickedGroup *pPickedGroup);
	void SetDuelTournamentMatchNumber(int nValue)						{ m_nDTStageInfo.nDuelTournamentNumber = nValue; }	
	void SetDuelTournamentMatchTimeStamp(char* szTimeStamp)				{ strcpy(m_nDTStageInfo.szTimeStamp, szTimeStamp); }
	
	const MDUELTOURNAMENTTYPE GetDuelTournamentType()							{ return m_nDTStageInfo.nDuelTournamentType; }
	const int	GetDuelTournamentTotalRound()									{ return m_nDTStageInfo.nDuelTournamentTotalRound; }
	const int	GetDuelTournamentMatchNumber()									{ return m_nDTStageInfo.nDuelTournamentNumber; }	
	const char* GetDuelTournamentMatchTimeStamp()								{ return m_nDTStageInfo.szTimeStamp; }
	map<int, MMatchDuelTournamentMatch*>* GetDuelTournamentMatchMap()			{ return &(m_nDTStageInfo.DuelTournamentMatchMap); }

	int	GetDuelTournamentRandomMapIndex();	

private:
	unsigned int m_nGameLogID;

public:
	void SetGameLogID(unsigned int nVal)	{ m_nGameLogID = nVal; }
	unsigned int GetGameLogID()				{ return m_nGameLogID; }

};


inline MDUELTOURNAMENTROUNDSTATE MMatchStage::GetDuelTournamentNextRoundState(MDUELTOURNAMENTROUNDSTATE nRoundState)
{
	switch(nRoundState){
		case MDUELTOURNAMENTROUNDSTATE_QUATERFINAL :		return MDUELTOURNAMENTROUNDSTATE_SEMIFINAL;
		case MDUELTOURNAMENTROUNDSTATE_SEMIFINAL :			return MDUELTOURNAMENTROUNDSTATE_FINAL;
		default :											return MDUELTOURNAMENTROUNDSTATE_MAX;
	}
}

inline MDUELTOURNAMENTROUNDSTATE MMatchStage::GetDuelTournamentRoundState(MDUELTOURNAMENTTYPE nType)
{
	switch(nType) {		
		case MDUELTOURNAMENTTYPE_QUATERFINAL :		return MDUELTOURNAMENTROUNDSTATE_QUATERFINAL;
		case MDUELTOURNAMENTTYPE_SEMIFINAL :		return MDUELTOURNAMENTROUNDSTATE_SEMIFINAL;		
		case MDUELTOURNAMENTTYPE_FINAL :			return MDUELTOURNAMENTROUNDSTATE_FINAL;
		default : ASSERT(0);
	}

	return MDUELTOURNAMENTROUNDSTATE_MAX;
}


class MMatchStageMap : public map<MUID, MMatchStage*> {
	MUID	m_uidGenerate;
public:
	MMatchStageMap()			{	m_uidGenerate = MUID(0,0);	}
	virtual ~MMatchStageMap()	{	}
	MUID UseUID()				{	m_uidGenerate.Increase();	return m_uidGenerate;	}
	void Insert(const MUID& uid, MMatchStage* pStage)	{	insert(value_type(uid, pStage));	}
};

MMatchItemBonusType GetStageBonusType(MMatchStageSetting* pStageSetting);


#define TRANS_STAGELIST_NODE_COUNT	8	// 한번에 클라이언트에게 보내주는 스테이지노드 개수



#endif
