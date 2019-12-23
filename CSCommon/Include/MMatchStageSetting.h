#ifndef _MMATCHSTAGESETTING_H
#define _MMATCHSTAGESETTING_H

#include <list>
using namespace std;
#include "MUID.h"
#include "MMatchGlobal.h"
#include "MMatchGameType.h"
#include "MMatchObject.h"
#include "MMatchMap.h"

class MMatchObject;
class MMatchStage;

#define MMATCH_TEAM_MAX_COUNT		2

#define MMATCH_SPECTATOR_STR		"SPECTATOR"
#define MMATCH_TEAM1_NAME_STR		"RED TEAM"
#define MMATCH_TEAM2_NAME_STR		"BLUE TEAM"


inline const char* GetTeamNameStr(MMatchTeam nTeam)
{
	switch (nTeam)
	{
	case MMT_SPECTATOR:
		return MMATCH_SPECTATOR_STR;
	case MMT_RED:
		return MMATCH_TEAM1_NAME_STR;
	case MMT_BLUE:
		return MMATCH_TEAM2_NAME_STR;
	default:
		return "";
	}
	return "";
}


enum STAGE_STATE {
	STAGE_STATE_STANDBY		= 0,
	STAGE_STATE_COUNTDOWN,
	STAGE_STATE_RUN,
	STAGE_STATE_CLOSE,

	STAGE_STATE_COUNT
};

#define MSTAGENODE_FLAG_FORCEDENTRY_ENABLED		1		// 난입
#define MSTAGENODE_FLAG_PRIVATE					2		// 비밀방
#define MSTAGENODE_FLAG_LIMITLEVEL				4		// 레벨제한


/// 스테이지 세팅값. 
/// - 네트웍 전송용으로도 사용한다. 차후 MTD로 이동해야함.
/// 여기에 변수를 추가하려면, 리플레이와도 관련이 있으므로 
/// ZReplayLoader::ConvertStageSettingNode() 함수도 수정해줘야 한다.
struct MSTAGE_SETTING_NODE {
	MUID				uidStage;
	char				szMapName[MAPNAME_LENGTH];	// 맵이름
	char				nMapIndex;					// 맵인덱스
	MMATCH_GAMETYPE		nGameType;					// 게임타입
	int					nRoundMax;					// 라운드
	int					nLimitTime;					// 제한시간(1 - 1분)
	int					nLimitLevel;				// 제한레벨
	int					nMaxPlayers;				// 최대인원
	bool				bTeamKillEnabled;			// 팀킬여부
	bool				bTeamWinThePoint;			// 선승제 여부
	bool				bForcedEntryEnabled;		// 게임중 참가 여부

	// 추가됨
	bool				bAutoTeamBalancing;			// 오토팀밸런스 - 팀플게임에서만 사용
	// 릴레이맵
	bool				bIsRelayMap;				// 릴레이맵 유무
	bool				bIsStartRelayMap;			// 릴레이맵 시작한 유무
	bool				bAntiLead;
	int					nRelayMapListCount;			// 릴레이맵 리스트 갯수
	RelayMap			MapList[MAX_RELAYMAP_LIST_COUNT];	// 릴레이맵 리스트
	RELAY_MAP_TYPE		nRelayMapType;				// 릴레이맵 타입
	RELAY_MAP_REPEAT_COUNT	nRelayMapRepeatCount;	// 릴레이맵 회차
	
#ifdef _VOTESETTING
	bool				bVoteEnabled;				// 투표가능 여부
	bool				bObserverEnabled;			// 관전모드 여부
#endif
};

// 방 처음 만들었을때 스테이지 세팅 초기값
#define MMATCH_DEFAULT_STAGESETTING_MAPNAME			"Mansion"

#define MMATCH_DEFAULT_STAGESETTING_GAMETYPE			MMATCH_GAMETYPE_DEATHMATCH_SOLO
#define MMATCH_DEFAULT_STAGESETTING_ROUNDMAX			50		// 50라운드
#define MMATCH_DEFAULT_STAGESETTING_LIMITTIME			30		// 30분
#define MMATCH_DEFAULT_STAGESETTING_LIMITLEVEL			0		// 무제한
#define MMATCH_DEFAULT_STAGESETTING_MAXPLAYERS			8		// 8명
#define MMATCH_DEFAULT_STAGESETTING_TEAMKILL			false	// 팀킬불가
#define MMATCH_DEFAULT_STAGESETTING_TEAM_WINTHEPOINT	false	// 선승제 여부
#define MMATCH_DEFAULT_STAGESETTING_FORCEDENTRY			true	// 난입가능
#define MMATCH_DEFAULT_STAGESETTING_AUTOTEAMBALANCING	true	// 오토팀밸런스


#define STAGESETTING_LIMITTIME_UNLIMITED				0		// 제한시간이 무제한은 0


struct MSTAGE_CHAR_SETTING_NODE {
	MUID	uidChar;
	int		nTeam;
	MMatchObjectStageState	nState;
	MSTAGE_CHAR_SETTING_NODE() : uidChar(MUID(0,0)), nTeam(0), nState(MOSS_NONREADY) {	}
};
class MStageCharSettingList : public list<MSTAGE_CHAR_SETTING_NODE*> {
public:
	void DeleteAll() {
		for (iterator i=begin(); i!=end(); i++) {
			delete (*i);
		}
		clear();
	}
};




class MMatchStageSetting {
protected:
	MProtectValue<MSTAGE_SETTING_NODE>	m_StageSetting;
	MUID					m_uidMaster;	// 방장
	STAGE_STATE				m_nStageState;	// 현재 State (게임중,대기중,..)

	bool					m_bIsCheckTicket;
	DWORD					m_dwTicketItemID;
public:
	MStageCharSettingList	m_CharSettingList;
public:
	MMatchStageSetting();
	virtual ~MMatchStageSetting();
	void Clear();
	void SetDefault();
	unsigned long GetChecksum();
	MSTAGE_CHAR_SETTING_NODE* FindCharSetting(const MUID& uid);

	// Get
	const char*					GetMapName()				{ return m_StageSetting.Ref().szMapName; }
	int							GetMapIndex()				{ return m_StageSetting.Ref().nMapIndex; }
	int							GetRoundMax()				{ return m_StageSetting.Ref().nRoundMax; }
	int							GetLimitTime()				{ return m_StageSetting.Ref().nLimitTime; }
	int							GetLimitLevel()				{ return m_StageSetting.Ref().nLimitLevel; }
	MUID						GetMasterUID()				{ return m_uidMaster; }
	STAGE_STATE					GetStageState()				{ return m_nStageState; }
	MMATCH_GAMETYPE				GetGameType()				{ return m_StageSetting.Ref().nGameType; }
	int							GetMaxPlayers()				{ return m_StageSetting.Ref().nMaxPlayers; }
	bool						GetForcedEntry()			{ return m_StageSetting.Ref().bForcedEntryEnabled; }
	bool						GetAutoTeamBalancing()		{ return m_StageSetting.Ref().bAutoTeamBalancing; }
	const MSTAGE_SETTING_NODE*	GetStageSetting()			{ return &m_StageSetting.Ref(); }
	const MMatchGameTypeInfo*	GetCurrGameTypeInfo();
	DWORD						GetTicketItemID()			{ return m_dwTicketItemID; }
	int							GetRelayMapListCount()		{ return m_StageSetting.Ref().nRelayMapListCount; }
	const RelayMap*				GetRelayMapList()			{ return m_StageSetting.Ref().MapList; }
	RELAY_MAP_TYPE				GetRelayMapType()			{ return m_StageSetting.Ref().nRelayMapType; }
	RELAY_MAP_REPEAT_COUNT		GetRelayMapRepeatCount()	{ return m_StageSetting.Ref().nRelayMapRepeatCount; }

	bool						IsRelayMap()				{ return m_StageSetting.Ref().bIsRelayMap; }
	bool						IsStartRelayMap()			{ return m_StageSetting.Ref().bIsStartRelayMap; }
	bool						GetAntiLead()				{ return m_StageSetting.Ref().bAntiLead; }
	// Set
	void SetMasterUID(const MUID& uid)				{ m_uidMaster = uid; }
	void SetMapName(char* pszName);
	void SetMapIndex(int nMapIndex);
	void SetRoundMax(int nRound)					{ MEMBER_SET_CHECKCRC(m_StageSetting, nRoundMax, nRound); }
	void SetLimitTime(int nTime)					{ MEMBER_SET_CHECKCRC(m_StageSetting, nLimitTime, nTime); }
	void SetGameType(MMATCH_GAMETYPE type)			{ MEMBER_SET_CHECKCRC(m_StageSetting, nGameType, type); }
	void SetStageState(STAGE_STATE nState)			{ m_nStageState = nState; }
	void SetTeamWinThePoint(bool bValue)			{ MEMBER_SET_CHECKCRC(m_StageSetting, bTeamWinThePoint, bValue); }
	void SetAutoTeamBalancing(bool bValue)			{ MEMBER_SET_CHECKCRC(m_StageSetting, bAutoTeamBalancing, bValue); }
	void SetIsCheckTicket( bool bIsCheck )			{ m_bIsCheckTicket = bIsCheck; }
	void SetTicketItemID( DWORD dwTicketItemID )	{ m_dwTicketItemID = dwTicketItemID; }
	
	void SetIsRelayMap(bool bValue)					{ MEMBER_SET_CHECKCRC(m_StageSetting, bIsRelayMap, bValue); }
	void SetIsStartRelayMap(bool bValue)			{ MEMBER_SET_CHECKCRC(m_StageSetting, bIsStartRelayMap, bValue); }

	void SetRelayMapListCount(int nValue)			{ MEMBER_SET_CHECKCRC(m_StageSetting, nRelayMapListCount, nValue); }
	void SetRelayMapList(RelayMap* pValue);
	void SetRelayMapType(RELAY_MAP_TYPE bValue)		{ MEMBER_SET_CHECKCRC(m_StageSetting, nRelayMapType, bValue); }
	void SetRelayMapRepeatCount(RELAY_MAP_REPEAT_COUNT bValue)	{ MEMBER_SET_CHECKCRC(m_StageSetting, nRelayMapRepeatCount, bValue); }
	void SetAntiLead(bool bValue)			{ MEMBER_SET_CHECKCRC(m_StageSetting, bAntiLead, bValue); }
	
	void UpdateStageSetting(MSTAGE_SETTING_NODE* pSetting);
	void UpdateCharSetting(const MUID& uid, unsigned int nTeam, MMatchObjectStageState nStageState);

	void ResetCharSetting()			{ m_CharSettingList.DeleteAll(); }
	bool IsTeamPlay();
	bool IsWaitforRoundEnd();
	bool IsQuestDrived();
	bool IsTeamWinThePoint()		{ return m_StageSetting.Ref().bTeamWinThePoint; }		///< 선승제 여부

	bool	IsCheckTicket()			{ return m_bIsCheckTicket; }
	
	// 핵방어용 클라에서만 호출
	void AntiHack_ShiftHeapPos()	{ m_StageSetting.ShiftHeapPos(); }
	void AntiHack_CheckCrc()		{ m_StageSetting.CheckCrc(); }
};












#endif