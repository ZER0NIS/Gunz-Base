#include "stdafx.h"
#include "MMatchRuleAssassinate.h"
#include "MBlobArray.h"

MMatchRuleAssassinate::MMatchRuleAssassinate(MMatchStage* pStage) : MMatchRuleTeamDeath(pStage)
{
	m_uidRedCommander = MUID(0, 0);
	m_uidBlueCommander = MUID(0, 0);
	m_bIsAdminCommander = false;
}

void MMatchRuleAssassinate::ChooseAdminAsCommander()
{
	m_bIsAdminCommander = !m_bIsAdminCommander;

	MMatchServer* pServer = MMatchServer::GetInstance();
	MMatchStage* pStage = GetStage();
	if (pServer == NULL || pStage == NULL) return;

	m_uidRedCommander = ChooseCommander(MMT_RED);
	m_uidBlueCommander = ChooseCommander(MMT_BLUE);
	if ((m_uidRedCommander == MUID(0, 0)) || (m_uidBlueCommander == MUID(0, 0))) {
		SetRoundState(MMATCH_ROUNDSTATE_FREE);
		return;
	}

	MCommand* pCmd = pServer->CreateCommand(MC_MATCH_ASSIGN_COMMANDER, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamUID(m_uidRedCommander));
	pCmd->AddParameter(new MCmdParamUID(m_uidBlueCommander));
	pServer->RouteToStage(pStage->GetUID(), pCmd);
}

const MUID MMatchRuleAssassinate::ChooseCommander(int nTeam)
{
	MMatchStage* pStage = GetStage();
	if (pStage == NULL) return MUID(0, 0);

	int nRedAliveCount, nBlueAliveCount, nChooseTeamCount;
	if (GetAliveCount(&nRedAliveCount, &nBlueAliveCount) == false) return MUID(0, 0);
	if (nTeam == MMT_RED) {
		if (nRedAliveCount <= 0) return MUID(0, 0);
		nChooseTeamCount = nRedAliveCount;
	}
	if (nTeam == MMT_BLUE) {
		if (nBlueAliveCount <= 0) return MUID(0, 0);
		nChooseTeamCount = nBlueAliveCount;
	}

	if (m_bIsAdminCommander == true)
	{
		for (MUIDRefCache::iterator itor = pStage->GetObjBegin(); itor != pStage->GetObjEnd(); itor++) {
			MMatchObject* pObj = (MMatchObject*)(*itor).second;

			if (pObj->GetEnterBattle() == false)
				continue;

			if (pObj->GetTeam() == nTeam && pObj->GetAccountInfo()->m_nUGrade == MMUG_ADMIN)
			{
				return pObj->GetUID();
			}
		}
	}

	MTime time;
	int nChoose = time.MakeNumber(1, nChooseTeamCount);

	int nCount = 0;
	for (MUIDRefCache::iterator itor = pStage->GetObjBegin(); itor != pStage->GetObjEnd(); itor++) {
		MMatchObject* pObj = (MMatchObject*)(*itor).second;
		if (pObj->GetEnterBattle() == false) continue;
		if (pObj->GetTeam() == nTeam) {
			nCount++;
			if (nCount == nChoose) {
				return pObj->GetUID();
			}
		}
	}
	return MUID(0, 0);
}

void MMatchRuleAssassinate::OnRoundBegin()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	MMatchStage* pStage = GetStage();
	if (pServer == NULL || pStage == NULL) return;

	m_uidRedCommander = ChooseCommander(MMT_RED);
	m_uidBlueCommander = ChooseCommander(MMT_BLUE);
	if ((m_uidRedCommander == MUID(0, 0)) || (m_uidBlueCommander == MUID(0, 0))) {
		SetRoundState(MMATCH_ROUNDSTATE_FREE);
		return;
	}

	MCommand* pCmd = pServer->CreateCommand(MC_MATCH_ASSIGN_COMMANDER, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamUID(m_uidRedCommander));
	pCmd->AddParameter(new MCmdParamUID(m_uidBlueCommander));
	pServer->RouteToStage(pStage->GetUID(), pCmd);
}

void MMatchRuleAssassinate::OnRoundEnd()
{
	MMatchRule::OnRoundEnd();
}

bool MMatchRuleAssassinate::OnCheckRoundFinish()
{
	MMatchStage* pStage = GetStage();
	if (pStage == NULL) {
		SetRoundArg(MMATCH_ROUNDRESULT_DRAW);
		return true;
	}

	MMatchObject* pRedCommanderObj = MMatchServer::GetInstance()->GetObject(m_uidRedCommander);
	if ((pRedCommanderObj == NULL) ||
		(pRedCommanderObj->GetStageUID() != pStage->GetUID())) {
		SetRoundArg(MMATCH_ROUNDRESULT_BLUEWON);
		return true;
	}

	MMatchObject* pBlueCommanderObj = MMatchServer::GetInstance()->GetObject(m_uidBlueCommander);
	if ((pBlueCommanderObj == NULL) ||
		(pBlueCommanderObj->GetStageUID() != pStage->GetUID())) {
		SetRoundArg(MMATCH_ROUNDRESULT_REDWON);
		return true;
	}

	if ((pRedCommanderObj->CheckAlive() == false) && (pBlueCommanderObj->CheckAlive() == false)) {
		SetRoundArg(MMATCH_ROUNDRESULT_DRAW);
		return true;
	}
	if (pRedCommanderObj->CheckAlive() == false) {
		SetRoundArg(MMATCH_ROUNDRESULT_BLUEWON);
		return true;
	}
	if (pBlueCommanderObj->CheckAlive() == false) {
		SetRoundArg(MMATCH_ROUNDRESULT_REDWON);
		return true;
	}

	return false;
}

void* MMatchRuleAssassinate::CreateRuleInfoBlob()
{
	void* pRuleInfoArray = MMakeBlobArray(sizeof(MTD_RuleInfo_Assassinate), 1);
	MTD_RuleInfo_Assassinate* pRuleItem = (MTD_RuleInfo_Assassinate*)MGetBlobArrayElement(pRuleInfoArray, 0);
	memset(pRuleItem, 0, sizeof(MTD_RuleInfo_Assassinate));

	pRuleItem->nRuleType = MMATCH_GAMETYPE_ASSASSINATE;
	pRuleItem->uidRedCommander = m_uidRedCommander;
	pRuleItem->uidBlueCommander = m_uidBlueCommander;

	return pRuleInfoArray;
}

void MMatchRuleAssassinate::CalcTeamBonus(MMatchObject* pAttacker, MMatchObject* pVictim,
	int nSrcExp, int* poutAttackerExp, int* poutTeamExp)
{
	if ((m_pStage == NULL) || (pAttacker == NULL) || (pVictim == NULL))
	{
		*poutAttackerExp = nSrcExp;
		*poutTeamExp = 0;
		return;
	}

	bool bVictimIsCommander = false;
	if (pVictim->GetTeam() == MMT_RED)
	{
		if (m_uidRedCommander == pVictim->GetUID()) bVictimIsCommander = true;
	}
	else if (pVictim->GetTeam() == MMT_BLUE)
	{
		if (m_uidBlueCommander == pVictim->GetUID()) bVictimIsCommander = true;
	}

	if (bVictimIsCommander)
	{
		*poutTeamExp = 0;
		*poutAttackerExp = nSrcExp * 2;
	}
	else
	{
		*poutTeamExp = (int)(nSrcExp * m_pStage->GetStageSetting()->GetCurrGameTypeInfo()->fTeamBonusExpRatio);
		*poutAttackerExp = (int)(nSrcExp * m_pStage->GetStageSetting()->GetCurrGameTypeInfo()->fTeamMyExpRatio);
	}
}