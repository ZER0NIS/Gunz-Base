#ifndef _MMATCHRULE_H
#define _MMATCHRULE_H

#include "MMatchItem.h"
#include "MMatchTransDataType.h"
#include "MUID.h"
#include "MMatchGameType.h"
#include "MQuestPlayer.h"
#include "MMatchEventManager.h"

class MMatchObject;
class MMatchStage;

enum MMATCH_ROUNDSTATE {
	MMATCH_ROUNDSTATE_PREPARE = 0,
	MMATCH_ROUNDSTATE_COUNTDOWN = 1,
	MMATCH_ROUNDSTATE_PLAY = 2,
	MMATCH_ROUNDSTATE_FINISH = 3,
	MMATCH_ROUNDSTATE_EXIT = 4,
	MMATCH_ROUNDSTATE_FREE = 5,
	MMATCH_ROUNDSTATE_FAILED = 6,
	MMATCH_ROUNDSTATE_PRE_COUNTDOWN = 7,

	MMATCH_ROUNDSTATE_END
};

enum MMATCH_ROUNDRESULT {
	MMATCH_ROUNDRESULT_DRAW = 0,
	MMATCH_ROUNDRESULT_REDWON,
	MMATCH_ROUNDRESULT_BLUEWON,
	MMATCH_ROUNDRESULT_RED_ALL_OUT,
	MMATCH_ROUNDRESULT_BLUE_ALL_OUT,
	MMATCH_ROUNDRESULT_END
};

class MMatchRule {
protected:
	MMatchStage* m_pStage;
	MMATCH_ROUNDSTATE	m_nRoundState;
	int					m_nRoundCount;
	int					m_nRoundArg;
	unsigned long		m_tmRoundStateTimer;
	int					m_nLastTimeLimitAnnounce;

	MMatchEventManager m_OnBeginEventManager;
	MMatchEventManager m_OnGameEventManager;
	MMatchEventManager m_OnEndEventManager;

protected:
	virtual bool RoundCount() { return false; }
	virtual bool OnRun();
	virtual void OnBegin();
	virtual void OnEnd();
	virtual void OnRoundBegin();
	virtual void OnRoundEnd();
	virtual void OnRoundTimeOut();

	virtual bool OnCheckRoundFinish() = 0;
	virtual bool OnCheckEnableBattleCondition() { return true; }
	virtual bool OnCheckBattleTimeOut(unsigned int tmTimeSpend);

	void SetRoundStateTimer(unsigned long tmTime) { m_tmRoundStateTimer = tmTime; }
	void InitRound();
	void SetRoundState(MMATCH_ROUNDSTATE nState);

	void InitOnBeginEventManager();
	void InitOnGameEventManager();
	void InitOnEndEventManager();

	void CheckOnBeginEvent();
	void CheckOnGameEvent();
	void CheckOnEndEvent();

	void RunOnBeginEvent();
	void RunOnGameEvent();
	void RunOnEndEvent();
public:
	MMatchRule() { }
	MMatchRule(MMatchStage* pStage);
	virtual ~MMatchRule() {}
	MMatchStage* GetStage() { return m_pStage; }

	int GetRoundCount() { return m_nRoundCount; }
	void SetRoundCount(int nRound) { m_nRoundCount = nRound; }
	int GetRoundArg() { return m_nRoundArg; }
	void SetRoundArg(int nArg) { m_nRoundArg = nArg; }

	MMatchEventManager& GetOnBeginEventManager() { return m_OnBeginEventManager; }
	MMatchEventManager& GetOnGameEventManager() { return m_OnGameEventManager; }
	MMatchEventManager& GetOnEndEventManager() { return m_OnEndEventManager; }

	MMATCH_ROUNDSTATE GetRoundState() { return m_nRoundState; }
	unsigned long GetRoundStateTimer() { return m_tmRoundStateTimer; }
	unsigned long GetLastTimeLimitAnnounce() { return m_nLastTimeLimitAnnounce; }
	void SetLastTimeLimitAnnounce(int nSeconds) { m_nLastTimeLimitAnnounce = nSeconds; }

	virtual void* CreateRuleInfoBlob() { return NULL; }

	virtual void CalcTeamBonus(MMatchObject* pAttacker,
		MMatchObject* pVictim,
		int nSrcExp,
		int* poutAttackerExp,
		int* poutTeamExp);
	virtual void OnEnterBattle(MUID& uidChar) {}
	virtual void OnLeaveBattle(MUID& uidChar) {}
	virtual void OnCommand(MCommand* pCommand) {}
	virtual void OnObtainWorldItem(MMatchObject* pObj, int nItemID, int* pnExtraValues) {}
	virtual void OnGameKill(const MUID& uidAttacker, const MUID& uidVictim) {}

	//Custom: virtual function for QuestEnterBattle
	virtual void OnQuestEnterBattle(MUID& uidChar) {}
	virtual void OnQuestStageLaunch() {}

	bool Run();
	void Begin();
	void End();

	void DebugTest();

	virtual bool CheckPlayersAlive() { return true; }
	virtual void OnFailed() {}
	virtual MMATCH_GAMETYPE GetGameType() = 0;
};

inline bool IsGameRuleDeathMatch(MMATCH_GAMETYPE nGameType)
{
	return (
		(nGameType == MMATCH_GAMETYPE_DEATHMATCH_SOLO) ||
		(nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_TRAINING)
		);
}
inline bool IsGameRuleGladiator(MMATCH_GAMETYPE nGameType)
{
	return ((nGameType == MMATCH_GAMETYPE_GLADIATOR_SOLO) ||
		(nGameType == MMATCH_GAMETYPE_GLADIATOR_TEAM));
}

inline bool IsGameRuleCTF(MMATCH_GAMETYPE nGameType)
{
	return (nGameType == MMATCH_GAMETYPE_CTF);
}

#endif