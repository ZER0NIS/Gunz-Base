#ifndef _MMATCHRULEQUEST_H
#define _MMATCHRULEQUEST_H

#include "MMatchRule.h"
#include "MMatchRuleBaseQuest.h"
#include "MMatchNPCObject.h"
#include "MMatchQuestRound.h"
#include "MSacrificeQItemTable.h"
#include "MQuestItem.h"
#include "MMatchQuestGameLog.h"
#include "MQuestNPCSpawnTrigger.h"
#include "MBaseGameType.h"

typedef pair< MUID, unsigned long int > SacrificeSlot;

class MQuestSacrificeSlot
{
public:
	MQuestSacrificeSlot()
	{
		Release();
	}

	~MQuestSacrificeSlot()
	{
	}

	MUID& GetOwnerUID() { return m_SacrificeSlot.first; }
	unsigned long int	GetItemID() { return m_SacrificeSlot.second; }

	bool IsEmpty()
	{
		if ((MUID(0, 0) == m_SacrificeSlot.first) && (0 == m_SacrificeSlot.second))
			return true;
		return false;
	}

	void SetOwnerUID(const MUID& uidItemOwner) { m_SacrificeSlot.first = uidItemOwner; }
	void SetItemID(const unsigned long int nItemID) { m_SacrificeSlot.second = nItemID; }
	void SetAll(const MUID& uidItemOwner, const unsigned long int nItemID)
	{
		SetOwnerUID(uidItemOwner);
		SetItemID(nItemID);
	}

	bool IsOwner(const MUID& uidRequester, const unsigned long int nItemID)
	{
		if ((m_SacrificeSlot.first == uidRequester) && (m_SacrificeSlot.second == nItemID))
			return true;
		return false;
	}

	void Release()
	{
		m_SacrificeSlot.first = MUID(0, 0);
		m_SacrificeSlot.second = 0;
	}

private:
	SacrificeSlot	m_SacrificeSlot;
};

class MQuestLevel;

class MQuestNPCSpawner
{
public:
};

class MMatchRuleQuest : public MMatchRuleBaseQuest {
private:
	struct MQuestStageGameInfo
	{
		int				nQL;
		int				nPlayerQL;
		int				nMapsetID;
		unsigned int	nScenarioID;
	};
	enum COMBAT_PLAY_RESULT
	{
		CPR_PLAYING = 0,
		CPR_COMPLETE,
		CPR_FAILED
	};

	unsigned long	m_nPrepareStartTime;
	unsigned long	m_nCombatStartTime;
	unsigned long	m_nQuestCompleteTime;

	MQuestSacrificeSlot				m_SacrificeSlot[MAX_SACRIFICE_SLOT_COUNT];
	int								m_nPlayerCount;
	MMatchQuestGameLogInfoManager	m_QuestGameLogInfoMgr;

	MQuestStageGameInfo				m_StageGameInfo;

	void ClearQuestLevel();
	void MakeStageGameInfo();
	void InitJacoSpawnTrigger();
	void MakeNPCnSpawn(MQUEST_NPC nNPCID, bool bAddQuestDropItem);
protected:
	MQuestLevel* m_pQuestLevel;
	MQuestNPCSpawnTrigger	m_JacoSpawnTrigger;
	MQuestCombatState		m_nCombatState;

	virtual void ProcessNPCSpawn();
	virtual bool CheckNPCSpawnEnable();
	virtual void RouteStageGameInfo();
	virtual void RouteCompleted();
	virtual void RouteFailed();
	virtual void OnCompleted();
	virtual void OnFailed();
	virtual void DistributeReward();

	void RouteMapSectorStart();
	void RouteMovetoPortal(const MUID& uidPlayer);
	void RouteReadyToNewSector(const MUID& uidPlayer);
	void RouteObtainQuestItem(unsigned long int nQuestItemID);
	void RouteObtainZItem(unsigned long int nItemID);
	void RouteSectorBonus(const MUID& uidPlayer, unsigned long int nEXPValue, unsigned long int nBP);
	void RouteCombatState();
	bool MakeQuestLevel();
	void CombatProcess();
	void MoveToNextSector();
	void SetCombatState(MQuestCombatState nState);
	bool CheckReadytoNewSector();
	COMBAT_PLAY_RESULT CheckCombatPlay();
	bool CheckQuestCompleted();
	bool CheckQuestCompleteDelayTime();
	void OnSectorCompleted();
	void ProcessCombatPlay();

	void OnBeginCombatState(MQuestCombatState nState);
	void OnEndCombatState(MQuestCombatState nState);

	void MakeRewardList();
	void DistributeXPnBP(MQuestPlayerInfo* pPlayerInfo, const int nRewardXP, const int nRewardBP, const int nScenarioQL);
	bool DistributeQItem(MQuestPlayerInfo* pPlayerInfo, void** ppoutSimpleQuestItemBlob);
	bool DistributeZItem(MQuestPlayerInfo* pPlayerInfo, void** ppoutQuestRewardZItemBlob);
protected:
	virtual void OnBegin();
	virtual void OnEnd();
	virtual void OnEnterBattle(MUID& uidChar);
	virtual bool OnRun();
	virtual void OnCommand(MCommand* pCommand);
	virtual bool OnCheckRoundFinish();
public:
	MMatchRuleQuest(MMatchStage* pStage);
	virtual ~MMatchRuleQuest();

	virtual void RefreshStageGameInfo();

	virtual void OnRequestPlayerDead(const MUID& uidVictim);
	virtual void OnObtainWorldItem(MMatchObject* pObj, int nItemID, int* pnExtraValues);

	virtual void OnRequestTestSectorClear();
	virtual void OnRequestTestFinish();

	virtual void OnRequestMovetoPortal(const MUID& uidPlayer);
	virtual void OnReadyToNewSector(const MUID& uidPlayer);

	virtual void OnRequestDropSacrificeItemOnSlot(const MUID& uidSender, const int nSlotIndex, const unsigned long int nItemID);
	virtual void OnResponseDropSacrificeItemOnSlot(const MUID& uidSender, const int nSlotIndex, const unsigned long int nItemID);
	virtual void OnRequestCallbackSacrificeItem(const MUID& uidSender, const int nSlotIndex, const unsigned long int nItemID);
	virtual void OnResponseCallBackSacrificeItem(const MUID& uidSender, const int nSlotIndex, const unsigned long int nItemID);
	virtual void OnRequestQL(const MUID& uidSender);
	virtual void OnResponseQL_ToStage(const MUID& uidStage);
	virtual void OnRequestSacrificeSlotInfo(const MUID& uidSender);
	virtual void OnResponseSacrificeSlotInfoToListener(const MUID& uidSender);
	virtual void OnResponseSacrificeSlotInfoToStage(const MUID& uidStage);
	virtual void OnChangeCondition();

	virtual bool							PrepareStart();
	virtual bool							IsSacrificeItemDuplicated(const MUID& uidSender, const int nSlotIndex, const unsigned long int nItemID);
	virtual void							PreProcessLeaveStage(const MUID& uidLeaverUID);
	virtual void							DestroyAllSlot();
	virtual MMATCH_GAMETYPE GetGameType() { return MMATCH_GAMETYPE_QUEST; }

	void InsertNoParamQItemToPlayer(MMatchObject* pPlayer, MQuestItem* pQItem);

	void PostInsertQuestGameLogAsyncJob();

	void CollectStartingQuestGameLogInfo();
	void CollectEndQuestGameLogInfo();

	void RouteRewardCommandToStage(MMatchObject* pPlayer, const int nRewardXP, const int nRewardBP, void* pSimpleQuestItemBlob, void* pSimpleZItemBlob);

	virtual void RouteGameInfo();
	void RouteGameInfo(MUID const& lateJoiner);

private:
	int CalcuOwnerQItemCount(const MUID& uidPlayer, const unsigned long nItemID);
	const bool PostNPCInfo();
};

#endif