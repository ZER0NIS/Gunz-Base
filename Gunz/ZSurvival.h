#pragma once

#include "ZBaseQuest.h"

class ZSurvival : public ZBaseQuest
{
private:
	set<MUID>	m_CharactersGone;

	ZQuestGameInfo		m_GameInfo;
	bool	m_Cheet[ZQUEST_CHEET_MAX];
	bool	m_bCreatedOnce;
	bool	m_bLoaded;
	bool	m_bIsQuestComplete;
	bool	m_bIsRoundClear;
	DWORD	m_tRemainedTime;
	float	m_fLastWeightTime;

	MQuestCombatState	m_QuestCombatState;

	ZNPCInfoFromServerManager m_NPCInfoFromServerMgr;

#ifdef _QUEST_ITEM
	int				m_nRewardXP;
	int				m_nRewardBP;
	int				m_nReachedRound;
	int				m_nPoint;

	virtual bool OnRewardQuest(MCommand* pCmd) { return true; }
	virtual bool OnNewMonsterInfo(MCommand* pCmd);
	bool OnSurvivalResult(MCommand* pCmd);
	bool OnSurvivalRankingList(MCommand* pCmd);
	bool OnSurvivalPrivateRanking(MCommand* pCmd);

	virtual void GetMyObtainQuestItemList(int nRewardXP, int nRewardBP, void* pMyObtainQuestItemListBlob, void* pMyObtainZItemListBlob);

public:
	virtual int GetRewardXP(void) { return m_nRewardXP; }
	virtual int GetRewardBP(void) { return m_nRewardBP; }
	int GetReachedRound(void) { return m_nReachedRound; }
	int GetPoint(void) { return m_nPoint; }
	virtual bool IsQuestComplete(void) { return m_bIsQuestComplete; }
	virtual bool IsRoundClear(void) { return m_bIsRoundClear; }
	virtual DWORD GetRemainedTime(void) { return m_tRemainedTime; }

	virtual MQuestCombatState GetQuestState() { return m_QuestCombatState; }

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
	virtual bool OnCommand(MCommand* pCommand);
	virtual bool OnGameCommand(MCommand* pCommand);

	virtual void SetCheet(ZQuestCheetType nCheetType, bool bValue);
	virtual bool GetCheet(ZQuestCheetType nCheetType);

	virtual void Reload();
	virtual bool Load();

	virtual ZQuestGameInfo* GetGameInfo() { return &m_GameInfo; }

	virtual bool OnSetMonsterBibleInfo(MCommand* pCmd);

	virtual bool OnPrePeerNPCAttackMelee(MCommand* pCommand);

	void MoveToRealSector(int sectorindex);
};

inline void ZSurvival::SetCheet(ZQuestCheetType nCheetType, bool bValue)
{
	m_Cheet[nCheetType] = bValue;
}

inline bool ZSurvival::GetCheet(ZQuestCheetType nCheetType)
{
	return m_Cheet[nCheetType];
}