#ifndef _ZSURVIVAL_H
#define _ZSURVIVAL_H

#include "ZBaseQuest.h"

// 서바이벌 관련 전역 클래스
class ZSurvival : public ZBaseQuest
{
private:
	set<MUID>	m_CharactersGone;	// 다음섹터로 이동한 캐릭터들

	ZQuestGameInfo		m_GameInfo;
	bool	m_Cheet[ZQUEST_CHEET_MAX];
	bool	m_bCreatedOnce;
	bool	m_bLoaded;
	bool	m_bIsQuestComplete;
	bool	m_bIsRoundClear;
	DWORD	m_tRemainedTime;					// 라운드가 끝나고 다음 라운드로 넘어가기까지 남은 시간
	float	m_fLastWeightTime;

	MQuestCombatState	m_QuestCombatState;

	ZNPCInfoFromServerManager m_NPCInfoFromServerMgr;


#ifdef _QUEST_ITEM
	int				m_nRewardXP;				// 라운드당 받은 XP 누적
	int				m_nRewardBP;				// 라운드당 받은 BP 누적
	int				m_nReachedRound;			// 도달한 라운드 (결과 커맨드에서 세팅됨)
	int				m_nPoint;					// 게임 점수
	
	virtual bool OnRewardQuest( MCommand* pCmd ) { return true; }		// 서바이벌에서 사용않음
	virtual bool OnNewMonsterInfo( MCommand* pCmd );	// 몬스터 모감에 등록될 새로 습득한 몬스터 정보.
	bool OnSurvivalResult(MCommand* pCmd);
	bool OnSurvivalRankingList(MCommand* pCmd);
	bool OnSurvivalPrivateRanking(MCommand* pCmd);

	virtual void GetMyObtainQuestItemList( int nRewardXP, int nRewardBP, void* pMyObtainQuestItemListBlob, void* pMyObtainZItemListBlob );

public :
	virtual int GetRewardXP( void)							{ return m_nRewardXP; }
	virtual int GetRewardBP( void)							{ return m_nRewardBP; }
			int GetReachedRound( void)						{ return m_nReachedRound; }
			int GetPoint( void)								{ return m_nPoint; }
	virtual bool IsQuestComplete( void)						{ return m_bIsQuestComplete; }
	virtual bool IsRoundClear( void)						{ return m_bIsRoundClear; }
	virtual DWORD GetRemainedTime( void)					{ return m_tRemainedTime; }

	virtual MQuestCombatState GetQuestState()				{ return m_QuestCombatState; }

	virtual ZNPCInfoFromServerManager& GetNPCInfoFromServerMgr() { return m_NPCInfoFromServerMgr; }

#endif

	virtual bool OnNPCSpawn(MCommand* pCommand);
	virtual bool OnNPCDead(MCommand* pCommand);
	virtual bool OnPeerNPCDead(MCommand* pCommand);
	virtual bool OnEntrustNPCControl(MCommand* pCommand);
	virtual bool OnPeerNPCBasicInfo(MCommand* pCommand);
	virtual bool OnPeerNPCHPInfo(MCommand* pCommand);
	virtual bool OnPeerNPCAttackMelee(MCommand* pCommand);
	virtual bool OnPeerNPCAttackRange(MCommand* pCommand);
	virtual bool OnPeerNPCSkillStart(MCommand* pCommand);
	virtual bool OnPeerNPCSkillExecute(MCommand* pCommand);
	virtual bool OnPeerNPCBossHpAp(MCommand* pCommand);
	virtual bool OnRefreshPlayerStatus(MCommand* pCommand);
	virtual bool OnClearAllNPC(MCommand* pCommand);
	virtual bool OnQuestRoundStart(MCommand* pCommand);
	virtual bool OnQuestPlayerDead(MCommand* pCommand);
	virtual bool OnQuestGameInfo(MCommand* pCommand);
	virtual bool OnQuestCombatState(MCommand* pCommand);
	virtual bool OnMovetoPortal(MCommand* pCommand);
	virtual bool OnReadyToNewSector(MCommand* pCommand);
	virtual bool OnSectorStart(MCommand* pCommand);
	virtual bool OnObtainQuestItem(MCommand* pCommand);
	virtual bool OnObtainZItem(MCommand* pCommand);
	virtual bool OnSectorBonus(MCommand* pCommand);
	virtual bool OnQuestCompleted(MCommand* pCommand);
	virtual bool OnQuestFailed(MCommand* pCommand);
	virtual bool OnQuestPing(MCommand* pCommand);


	//ZQuestMap			m_Map;
	virtual void LoadNPCMeshes();
	virtual void LoadNPCSounds();
	virtual void MoveToNextSector();
	virtual void UpdateNavMeshWeight(float fDelta);
protected:
	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual bool OnCreateOnce();
	virtual void OnDestroyOnce();
public:
	ZSurvival();
	virtual ~ZSurvival();
public:
	virtual void OnGameCreate();
	virtual void OnGameDestroy();
	virtual void OnGameUpdate(float fElapsed);
	virtual bool OnCommand(MCommand* pCommand);				///< 게임 이외에 날라오는 커맨드 처리
	virtual bool OnGameCommand(MCommand* pCommand);			///< 게임중 날라오는 커맨드 처리

	virtual void SetCheet(ZQuestCheetType nCheetType, bool bValue);
	virtual bool GetCheet(ZQuestCheetType nCheetType);

	virtual void Reload();
	virtual bool Load();

	
	// interface
	virtual ZQuestGameInfo* GetGameInfo()		{ return &m_GameInfo; }

	// 상태에 상관없이 사용될수 있는 퀘스트 관련된 커맨드.
	virtual bool OnSetMonsterBibleInfo( MCommand* pCmd );


	virtual bool OnPrePeerNPCAttackMelee(MCommand* pCommand);	// 실제로 처리하는건 한타이밍 늦다
	
};




/////////////////////////////////////////////////////////////////////

inline void ZSurvival::SetCheet(ZQuestCheetType nCheetType, bool bValue) 
{ 
	m_Cheet[nCheetType] = bValue; 
}

inline bool ZSurvival::GetCheet(ZQuestCheetType nCheetType) 
{ 
	if (!ZIsLaunchDevelop()) return false;
	return m_Cheet[nCheetType];
}



#endif