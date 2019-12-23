#include "stdafx.h"
#include "MMatchRuleDeathMatch.h"
#include "MMatchTransDataType.h"
#include "MBlobArray.h"
#include "MMatchFormula.h"

// TEAM DEATH RULE ///////////////////////////////////////////////////////////////
MMatchRuleTeamDeath::MMatchRuleTeamDeath(MMatchStage* pStage) : MMatchRule(pStage)
{
}

void MMatchRuleTeamDeath::OnBegin()
{
}

void MMatchRuleTeamDeath::OnEnd()
{
}

bool MMatchRuleTeamDeath::OnRun()
{
	bool ret = MMatchRule::OnRun();


	return ret;
}

void MMatchRuleTeamDeath::OnRoundBegin()
{
	MMatchRule::OnRoundBegin();
}

void MMatchRuleTeamDeath::OnRoundEnd()
{
	if (m_pStage != NULL)
	{
		switch(m_nRoundArg)
		{
			case MMATCH_ROUNDRESULT_BLUE_ALL_OUT: m_pStage->OnRoundEnd_FromTeamGame(MMT_RED);break;
			case MMATCH_ROUNDRESULT_RED_ALL_OUT: m_pStage->OnRoundEnd_FromTeamGame(MMT_BLUE); break;
			case MMATCH_ROUNDRESULT_REDWON: m_pStage->OnRoundEnd_FromTeamGame(MMT_RED); break;
			case MMATCH_ROUNDRESULT_BLUEWON: m_pStage->OnRoundEnd_FromTeamGame(MMT_BLUE); break;
			case MMATCH_ROUNDRESULT_DRAW: break;
		}
	}

	MMatchRule::OnRoundEnd();
}

bool MMatchRuleTeamDeath::OnCheckEnableBattleCondition()
{
	// 선승제일 경우는 Free상태가 안된다.
	if (m_pStage->GetStageSetting()->IsTeamWinThePoint() == true)
	{
		return true;
	}

	int nRedTeam = 0, nBlueTeam = 0;
	int nPreRedTeam = 0, nPreBlueTeam = 0;
	int nStageObjects = 0;		// 게임안에 없고 스테이지에 있는 사람

	MMatchStage* pStage = GetStage();
	if (pStage == NULL) return false;

	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) 
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if ((pObj->GetEnterBattle() == false) && (!pObj->IsLaunchedGame()))
		{
			nStageObjects++;
			continue;
		}

		if (pObj->GetTeam() == MMT_RED)
		{
			nRedTeam++;
		}
		else if (pObj->GetTeam() == MMT_BLUE)
		{
			nBlueTeam++;
		}
	}

	if ( nRedTeam == 0 || nBlueTeam == 0)
	{
		return false;
	}

	return true;
}

// 만약 레드팀이나 블루팀에서 팀원이 0명일 경우는 false 반환 , true,false 모두 AliveCount 반환
bool MMatchRuleTeamDeath::GetAliveCount(int* pRedAliveCount, int* pBlueAliveCount)
{
	int nRedCount = 0, nBlueCount = 0;
	int nRedAliveCount = 0, nBlueAliveCount = 0;
	(*pRedAliveCount) = 0;
	(*pBlueAliveCount) = 0;

	MMatchStage* pStage = GetStage();
	if (pStage == NULL) return false;

	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) 
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if (pObj->GetEnterBattle() == false) continue;	// 배틀참가하고 있는 플레이어만 체크

		if (pObj->GetTeam() == MMT_RED)
		{
			nRedCount++;
			if (pObj->CheckAlive()==true)
			{
				nRedAliveCount++;
			}
		}
		else if (pObj->GetTeam() == MMT_BLUE)
		{
			nBlueCount++;
			if (pObj->CheckAlive()==true)
			{
				nBlueAliveCount++;
			}
		}
	}

	(*pRedAliveCount) = nRedAliveCount;
	(*pBlueAliveCount) = nBlueAliveCount;

	if ((nRedAliveCount == 0) || (nBlueAliveCount == 0))
	{
		return false;
	}
	return true;
}

bool MMatchRuleTeamDeath::OnCheckRoundFinish()
{
	int nRedAliveCount = 0;
	int nBlueAliveCount = 0;

	// 팀원이 0명인 팀이 있으면 false반환
	if (GetAliveCount(&nRedAliveCount, &nBlueAliveCount) == false)
	{
		int nRedTeam = 0, nBlueTeam = 0;
		int nStageObjects = 0;		// 게임안에 없고 스테이지에 있는 사람

		MMatchStage* pStage = GetStage();

		for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) 
		{
			MMatchObject* pObj = (MMatchObject*)(*i).second;
			if ((pObj->GetEnterBattle() == false) && (!pObj->IsLaunchedGame()))
			{
				nStageObjects++;
				continue;
			}

			if (pObj->GetTeam() == MMT_RED)		nRedTeam++;
			else if (pObj->GetTeam() == MMT_BLUE)	nBlueTeam++;
		}

		if( nBlueTeam ==0 && (pStage->GetTeamScore(MMT_BLUE) > pStage->GetTeamScore(MMT_RED)) )
			SetRoundArg(MMATCH_ROUNDRESULT_BLUE_ALL_OUT);
		else if( nRedTeam ==0 && (pStage->GetTeamScore(MMT_RED) > pStage->GetTeamScore(MMT_BLUE)) )
			SetRoundArg(MMATCH_ROUNDRESULT_RED_ALL_OUT);
		else if ( (nRedAliveCount == 0) && (nBlueAliveCount == 0) )
			SetRoundArg(MMATCH_ROUNDRESULT_DRAW);
		else if (nRedAliveCount == 0)
			SetRoundArg(MMATCH_ROUNDRESULT_BLUEWON);
		else if (nBlueAliveCount == 0)
			SetRoundArg(MMATCH_ROUNDRESULT_REDWON);
	}

	if (nRedAliveCount==0 || nBlueAliveCount==0) return true;
	else return false;
}

void MMatchRuleTeamDeath::OnRoundTimeOut()
{
	int nRedAliveCount = 0;
	int nBlueAliveCount = 0;
	GetAliveCount(&nRedAliveCount, &nBlueAliveCount);

	if (nRedAliveCount > nBlueAliveCount)
		SetRoundArg(MMATCH_ROUNDRESULT_REDWON);
	else if (nBlueAliveCount > nRedAliveCount)
		SetRoundArg(MMATCH_ROUNDRESULT_BLUEWON);
	else SetRoundArg(MMATCH_ROUNDRESULT_DRAW);
}

// 반환값이 false이면 게임이 끝난다.
bool MMatchRuleTeamDeath::RoundCount() 
{
	if (m_pStage == NULL) return false;

	int nTotalRound = m_pStage->GetStageSetting()->GetRoundMax();
	m_nRoundCount++;

	if (m_pStage->GetStageSetting()->IsTeamWinThePoint() == false)
	{
		// 선승제가 아닐 경우
		if (m_nRoundCount < nTotalRound) return true;

	}
	else
	{
		// 선승제일 경우 

		// 팀원이 0명인 팀이 있어도 게임이 끝난다.
		int nRedTeamCount=0, nBlueTeamCount=0;
		m_pStage->GetTeamMemberCount(&nRedTeamCount, &nBlueTeamCount, NULL, true);

		if ((nRedTeamCount == 0) || (nBlueTeamCount == 0))
		{
			return false;
		}

		int nRedScore = m_pStage->GetTeamScore(MMT_RED);
		int nBlueScore = m_pStage->GetTeamScore(MMT_BLUE);
		
		// 래더게임에서 먼저 4승인 팀이 승리
		const int LADDER_WINNING_ROUNT_COUNT = 4;


		// 두팀이 모두 4승이 아니면 true반환
		if ((nRedScore < LADDER_WINNING_ROUNT_COUNT) && (nBlueScore < LADDER_WINNING_ROUNT_COUNT))
		{
			return true;
		}
	}

	return false;
}

void MMatchRuleTeamDeath::CalcTeamBonus(MMatchObject* pAttacker, MMatchObject* pVictim,
								int nSrcExp, int* poutAttackerExp, int* poutTeamExp)
{
	if (m_pStage == NULL)
	{
		*poutAttackerExp = nSrcExp;
		*poutTeamExp = 0;
		return;
	}

	*poutTeamExp = (int)(nSrcExp * m_pStage->GetStageSetting()->GetCurrGameTypeInfo()->fTeamBonusExpRatio);
	*poutAttackerExp = (int)(nSrcExp * m_pStage->GetStageSetting()->GetCurrGameTypeInfo()->fTeamMyExpRatio);
}

//////////////////////////////////////////////////////////////////////////////////
// MMatchRuleSoloDeath ///////////////////////////////////////////////////////////
MMatchRuleSoloDeath::MMatchRuleSoloDeath(MMatchStage* pStage) : MMatchRule(pStage)
{

}

void MMatchRuleSoloDeath::OnBegin()
{

}
void MMatchRuleSoloDeath::OnEnd()
{
}

bool MMatchRuleSoloDeath::RoundCount()
{
	if (++m_nRoundCount < 1) return true;
	return false;
}

bool MMatchRuleSoloDeath::CheckKillCount(MMatchObject* pOutObject)
{
	MMatchStage* pStage = GetStage();
	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) 
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if (pObj->GetEnterBattle() == false) continue;

		if (pObj->GetKillCount() >= (unsigned int)pStage->GetStageSetting()->GetRoundMax())
		{
			pOutObject = pObj;
			return true;
		}
	}
	return false;
}

bool MMatchRuleSoloDeath::OnCheckRoundFinish()
{
	MMatchObject* pObject = NULL;

	if (CheckKillCount(pObject))
	{
		return true;
	}
	return false;
}

void MMatchRuleSoloDeath::OnRoundTimeOut()
{
	SetRoundArg(MMATCH_ROUNDRESULT_DRAW);
}




// 무한 팀데스매치 - 추가 by 동섭
//////////////////////////////////////////////////////////////////////////
MMatchRuleTeamDeath2::MMatchRuleTeamDeath2(MMatchStage* pStage) : MMatchRule(pStage)
{
}

void MMatchRuleTeamDeath2::OnBegin()
{
	m_pStage->InitTeamKills();
}

void MMatchRuleTeamDeath2::OnEnd()
{
}

bool MMatchRuleTeamDeath2::OnRun()
{
	bool ret = MMatchRule::OnRun();


	return ret;
}

void MMatchRuleTeamDeath2::OnRoundBegin()
{
	MMatchRule::OnRoundBegin();
}

void MMatchRuleTeamDeath2::OnRoundEnd()
{
	if (m_pStage != NULL)
	{
		if (m_nRoundArg == MMATCH_ROUNDRESULT_REDWON) 
		{
			m_pStage->OnRoundEnd_FromTeamGame(MMT_RED);
		} 
		else if (m_nRoundArg == MMATCH_ROUNDRESULT_BLUEWON) 
		{
			m_pStage->OnRoundEnd_FromTeamGame(MMT_BLUE);
		} 
		else if (m_nRoundArg == MMATCH_ROUNDRESULT_DRAW) 
		{ 
			// Do Nothing
		}
	}

	MMatchRule::OnRoundEnd();
}

// 만약 레드팀이나 블루팀에서 팀원이 0명일 경우는 false 반환 , true,false 모두 AliveCount 반환
void MMatchRuleTeamDeath2::GetTeamScore(int* pRedTeamScore, int* pBlueTeamScore)
{
	(*pRedTeamScore) = 0;
	(*pBlueTeamScore) = 0;

	MMatchStage* pStage = GetStage();
	if (pStage == NULL) return;

	(*pRedTeamScore) = pStage->GetTeamKills(MMT_RED);
	(*pBlueTeamScore) = pStage->GetTeamKills(MMT_BLUE);

	return;
}

bool MMatchRuleTeamDeath2::OnCheckRoundFinish()
{
	int nRedScore, nBlueScore;
	GetTeamScore(&nRedScore, &nBlueScore);

	MMatchStage* pStage = GetStage();

	if (nRedScore >= pStage->GetStageSetting()->GetRoundMax())
	{
		SetRoundArg(MMATCH_ROUNDRESULT_REDWON);
		return true;
	}
	else if (nBlueScore >= pStage->GetStageSetting()->GetRoundMax())
	{
		SetRoundArg(MMATCH_ROUNDRESULT_BLUEWON);
		return true;
	}

	return false;
}

void MMatchRuleTeamDeath2::OnRoundTimeOut()
{
	if (!OnCheckRoundFinish())
		SetRoundArg(MMATCH_ROUNDRESULT_DRAW);
}

// 반환값이 false이면 게임이 끝난다.
bool MMatchRuleTeamDeath2::RoundCount() 
{
	if (++m_nRoundCount < 1) return true;
	return false;
}

void MMatchRuleTeamDeath2::CalcTeamBonus(MMatchObject* pAttacker, MMatchObject* pVictim,
										int nSrcExp, int* poutAttackerExp, int* poutTeamExp)
{
	if (m_pStage == NULL)
	{
		*poutAttackerExp = nSrcExp;
		*poutTeamExp = 0;
		return;
	}

	*poutTeamExp = (int)(nSrcExp * m_pStage->GetStageSetting()->GetCurrGameTypeInfo()->fTeamBonusExpRatio);
	*poutAttackerExp = (int)(nSrcExp * m_pStage->GetStageSetting()->GetCurrGameTypeInfo()->fTeamMyExpRatio);
}




void MMatchRuleTeamDeath2::OnGameKill(const MUID& uidAttacker, const MUID& uidVictim)
{
	MMatchObject* pAttacker = MMatchServer::GetInstance()->GetObject(uidAttacker);
	MMatchObject* pVictim = MMatchServer::GetInstance()->GetObject(uidVictim);

	if (m_pStage != NULL)
	{
//		if (pAttacker->GetTeam() != pVictim->GetTeam())
//		{
//			m_pStage->AddTeamKills(pAttacker->GetTeam());
//		}

		m_pStage->AddTeamKills(pVictim->GetTeam() == MMT_BLUE ? MMT_RED : MMT_BLUE);		// 죽은사람 반대편팀 킬수 올림
	}
}
// 무한 팀데스매치 - 추가 by 동섭
//////////////////////////////////////////////////////////////////////////
MMatchRuleTeamCTF::MMatchRuleTeamCTF(MMatchStage* pStage) : MMatchRule(pStage)
{
	SetBlueFlagObtained(false);
	SetRedFlagObtained(false);
	SetBlueCarrier(MUID(0,0));
	SetRedCarrier(MUID(0,0));
}

void MMatchRuleTeamCTF::OnBegin()
{
	m_pStage->InitTeamKills();
}

void MMatchRuleTeamCTF::OnEnd()
{
	SetBlueFlagObtained(false);
	SetRedFlagObtained(false);
	SetBlueCarrier(MUID(0,0));
	SetRedCarrier(MUID(0,0));
}

bool MMatchRuleTeamCTF::OnRun()
{
	bool ret = MMatchRule::OnRun();


	return ret;
}

void MMatchRuleTeamCTF::OnRoundBegin()
{
	MMatchRule::OnRoundBegin();
}

void MMatchRuleTeamCTF::OnRoundEnd()
{
	if (m_pStage != NULL)
	{
		if (m_nRoundArg == MMATCH_ROUNDRESULT_REDWON) 
		{
			m_pStage->OnRoundEnd_FromTeamGame(MMT_RED);
		} 
		else if (m_nRoundArg == MMATCH_ROUNDRESULT_BLUEWON) 
		{
			m_pStage->OnRoundEnd_FromTeamGame(MMT_BLUE);
		} 
		else if (m_nRoundArg == MMATCH_ROUNDRESULT_DRAW) 
		{ 
			// Do Nothing
		}
	}

	MMatchRule::OnRoundEnd();
}

// 만약 레드팀이나 블루팀에서 팀원이 0명일 경우는 false 반환 , true,false 모두 AliveCount 반환
void MMatchRuleTeamCTF::GetTeamScore(int* pRedTeamScore, int* pBlueTeamScore)
{
	(*pRedTeamScore) = 0;
	(*pBlueTeamScore) = 0;

	MMatchStage* pStage = GetStage();
	if (pStage == NULL) return;

	(*pRedTeamScore) = pStage->GetTeamKills(MMT_RED);
	(*pBlueTeamScore) = pStage->GetTeamKills(MMT_BLUE);

	return;
}

bool MMatchRuleTeamCTF::OnCheckRoundFinish()
{
	int nRedScore, nBlueScore;
	GetTeamScore(&nRedScore, &nBlueScore);

	MMatchStage* pStage = GetStage();

	if (nRedScore >= pStage->GetStageSetting()->GetRoundMax())
	{
		SetRoundArg(MMATCH_ROUNDRESULT_REDWON);
		return true;
	}
	else if (nBlueScore >= pStage->GetStageSetting()->GetRoundMax())
	{
		SetRoundArg(MMATCH_ROUNDRESULT_BLUEWON);
		return true;
	}

	return false;
}

void MMatchRuleTeamCTF::OnRoundTimeOut()
{
	if (!OnCheckRoundFinish())
		SetRoundArg(MMATCH_ROUNDRESULT_DRAW);
}

// 반환값이 false이면 게임이 끝난다.
bool MMatchRuleTeamCTF::RoundCount() 
{
	if (++m_nRoundCount < 1) return true;
	return false;
}
void MMatchRuleTeamCTF::CalcTeamBonus(MMatchObject* pAttacker, MMatchObject* pVictim,
										int nSrcExp, int* poutAttackerExp, int* poutTeamExp)
{
	if (m_pStage == NULL)
	{
		*poutAttackerExp = nSrcExp;
		*poutTeamExp = 0;
		return;
	}

	*poutTeamExp = (int)(nSrcExp * m_pStage->GetStageSetting()->GetCurrGameTypeInfo()->fTeamBonusExpRatio);
	*poutAttackerExp = (int)(nSrcExp * m_pStage->GetStageSetting()->GetCurrGameTypeInfo()->fTeamMyExpRatio);
}

void MMatchRuleTeamCTF::OnObtainWorldItem(MMatchObject* pObj, int nItemID, int* pnExtraValues)
{
	if( 0 == pObj )
		return;

	if (m_pStage == NULL)
	{
		SetBlueFlagObtained(false);
		SetRedFlagObtained(false);
		SetBlueCarrier(MUID(0,0));
		SetRedCarrier(MUID(0,0));
		return;
	}

	MMatchObject* pObtainer = pObj;
	MMatchTeam nTeam = pObj->GetTeam();

	int nBlueTeamCount = 0;
	int nRedTeamCount = 0;
	for (MUIDRefCache::iterator i=m_pStage->GetObjBegin(); i!=m_pStage->GetObjEnd(); i++) {
		MMatchObject* pTeamObj = (MMatchObject*)(*i).second;
		if (pTeamObj->GetEnterBattle() == true)
		{
				if (pTeamObj->GetTeam() == MMT_RED) 
				{
					nRedTeamCount++;
				}
				else if (pTeamObj->GetTeam() == MMT_BLUE) 
				{
					nBlueTeamCount++;
				}
		}
	}

	if(MMT_BLUE == nTeam)
	{
		if(IsRedFlagTaken() == false && nItemID == CTF_RED_ITEM_ID)
		{
			MUID obtainerUID = pObtainer->GetUID();
			// this is a grab
			SetRedFlagObtained(true);
			m_pStage->m_WorldItemManager.ChangeFlagState(false, MMT_RED);
			SetBlueCarrier(obtainerUID);
			RouteAssignFlag(obtainerUID, nTeam);
			SendAssignState();
		}
		else if(IsBlueFlagTaken() == false && nItemID == CTF_BLUE_ITEM_ID && IsRedFlagTaken() == true)
		{
			SetBlueFlagObtained(false);
			SetRedFlagObtained(false);
			SetBlueCarrier(MUID(0,0));
			SetRedCarrier(MUID(0,0));
			m_pStage->AddTeamKills(nTeam);
			m_pStage->m_WorldItemManager.ChangeFlagState(true, MMT_RED);
			if(pObj->GetCharInfo() && nBlueTeamCount > NUM_APPLYED_TEAMBONUS_TEAM_PLAYERS && nRedTeamCount > NUM_APPLYED_TEAMBONUS_TEAM_PLAYERS)
			{
			unsigned long int nGettingExp = (MMatchFormula::GetGettingExp(pObj->GetCharInfo()->m_nLevel, pObj->GetCharInfo()->m_nLevel) * 3);
			MMatchServer::GetInstance()->ApplyObjectTeamBonus(pObj, nGettingExp * 3);
			m_pStage->OnApplyTeamBonus(nTeam);
			}
			m_pStage->ResetTeamBonus();
			RouteAssignCap(nTeam);
			SendAssignState();
			//this is a cap
		}
		else
		{
			return;
		}
	}
	else if(MMT_RED == nTeam) //reverse logic
	{
		if(IsBlueFlagTaken() == false && nItemID == CTF_BLUE_ITEM_ID)
		{
			MUID obtainerUID = pObtainer->GetUID();
			// this is a grab
			SetBlueFlagObtained(true);
			m_pStage->m_WorldItemManager.ChangeFlagState(false, MMT_BLUE);
			SetRedCarrier(obtainerUID);
			RouteAssignFlag(obtainerUID, nTeam);
			SendAssignState();
		}
		else if(IsBlueFlagTaken() == true && nItemID == CTF_RED_ITEM_ID && IsRedFlagTaken() == false)
		{
			SetBlueFlagObtained(false);
			SetRedFlagObtained(false);
			SetBlueCarrier(MUID(0,0));
			SetRedCarrier(MUID(0,0));
			m_pStage->AddTeamKills(nTeam);
			m_pStage->m_WorldItemManager.ChangeFlagState(true, MMT_BLUE);
			if(pObj->GetCharInfo() && nBlueTeamCount > NUM_APPLYED_TEAMBONUS_TEAM_PLAYERS && nRedTeamCount > NUM_APPLYED_TEAMBONUS_TEAM_PLAYERS)
			{
			unsigned long int nGettingExp = (MMatchFormula::GetGettingExp(pObj->GetCharInfo()->m_nLevel, pObj->GetCharInfo()->m_nLevel) * 3);
			MMatchServer::GetInstance()->ApplyObjectTeamBonus(pObj, nGettingExp * 3);
			m_pStage->OnApplyTeamBonus(nTeam);
			}
			m_pStage->ResetTeamBonus();
			RouteAssignCap(nTeam);
			SendAssignState();
			//this is a cap
		}
		else
		{
			return;
		}
	}
	else 
	{
		return; //haxxors!
	}

}


void MMatchRuleTeamCTF::RouteAssignFlag(MUID& uidFlagBearer, int nTeam)
{	MCommand* pNew = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_FLAG_EFFECT, MUID(0, 0));
	pNew->AddParameter(new MCmdParamUID(uidFlagBearer));
	pNew->AddParameter(new MCmdParamInt(nTeam));
	MMatchServer::GetInstance()->RouteToBattle(m_pStage->GetUID(), pNew);
}

void MMatchRuleTeamCTF::RouteAssignFlagToJoiner(MUID& uidFlagBearer, MUID& uidSendTo, int nTeam)
{	
	MCommand* pNew = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_FLAG_EFFECT, MUID(0, 0));
	pNew->AddParameter(new MCmdParamUID(uidFlagBearer));
	pNew->AddParameter(new MCmdParamInt(nTeam));
	MMatchServer::GetInstance()->RouteToObjInStage(m_pStage->GetUID(), uidSendTo, pNew);
}

void MMatchRuleTeamCTF::RouteAssignCap(int nTeam)
{	MCommand* pNew = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_FLAG_CAP, MUID(0, 0));
	pNew->AddParameter(new MCmdParamInt(nTeam));
	MMatchServer::GetInstance()->RouteToBattle(m_pStage->GetUID(), pNew);
}

void MMatchRuleTeamCTF::RouteAssignState(MUID uidSendTo)
{
			MMatchRuleTeamCTF* pTeamCTF = (MMatchRuleTeamCTF*)this;

			//Route Blue Flag
			MCommand* pCmdBlue = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_FLAG_STATE, MUID(0,0));
			pCmdBlue->AddParameter(new MCmdParamInt(CTF_BLUE_ITEM_ID));
			pCmdBlue->AddParameter(new MCmdParamShortVector(pTeamCTF->GetBlueFlagPosition().x, pTeamCTF->GetBlueFlagPosition().y, pTeamCTF->GetBlueFlagPosition().z ));
			pCmdBlue->AddParameter(new MCmdParamInt(pTeamCTF->IsBlueFlagTaken()));
			pCmdBlue->AddParameter(new MCmdParamUID(pTeamCTF->GetBlueCarrier()));
			MMatchServer::GetInstance()->RouteToObjInStage(m_pStage->GetUID(), uidSendTo, pCmdBlue);

			//Route Red Flag
			MCommand* pCmdRed = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_FLAG_STATE, MUID(0,0));
			pCmdRed->AddParameter(new MCmdParamInt(CTF_RED_ITEM_ID));
			pCmdRed->AddParameter(new MCmdParamShortVector(pTeamCTF->GetRedFlagPosition().x, pTeamCTF->GetRedFlagPosition().y, pTeamCTF->GetRedFlagPosition().z ));
			pCmdRed->AddParameter(new MCmdParamInt(pTeamCTF->IsRedFlagTaken()));
			pCmdRed->AddParameter(new MCmdParamUID(pTeamCTF->GetRedCarrier()));
			MMatchServer::GetInstance()->RouteToObjInStage(m_pStage->GetUID(), uidSendTo, pCmdRed);
}

void MMatchRuleTeamCTF::SendAssignState()
{
			MMatchRuleTeamCTF* pTeamCTF = (MMatchRuleTeamCTF*)this;

			//Route Blue Flag
			MCommand* pCmdBlue = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_FLAG_STATE, MUID(0,0));
			pCmdBlue->AddParameter(new MCmdParamInt(CTF_BLUE_ITEM_ID));
			pCmdBlue->AddParameter(new MCmdParamShortVector(pTeamCTF->GetBlueFlagPosition().x, pTeamCTF->GetBlueFlagPosition().y, pTeamCTF->GetBlueFlagPosition().z ));
			pCmdBlue->AddParameter(new MCmdParamInt(pTeamCTF->IsBlueFlagTaken()));
			pCmdBlue->AddParameter(new MCmdParamUID(pTeamCTF->GetBlueCarrier()));
			MMatchServer::GetInstance()->RouteToBattle(m_pStage->GetUID(), pCmdBlue);

			//Route Red Flag
			MCommand* pCmdRed = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_FLAG_STATE, MUID(0,0));
			pCmdRed->AddParameter(new MCmdParamInt(CTF_RED_ITEM_ID));
			pCmdRed->AddParameter(new MCmdParamShortVector(pTeamCTF->GetRedFlagPosition().x, pTeamCTF->GetRedFlagPosition().y, pTeamCTF->GetRedFlagPosition().z ));
			pCmdRed->AddParameter(new MCmdParamInt(pTeamCTF->IsRedFlagTaken()));
			pCmdRed->AddParameter(new MCmdParamUID(pTeamCTF->GetRedCarrier()));
			MMatchServer::GetInstance()->RouteToBattle(m_pStage->GetUID(), pCmdRed);
}

void MMatchRuleTeamCTF::OnLeaveBattle(MUID& uidChar)
{
	if (uidChar == GetBlueCarrier())
	{
	MUID m_uidBearer = MUID(0, 0);
	SetBlueCarrier(m_uidBearer);
	SetRedFlagObtained(false);
	m_pStage->m_WorldItemManager.ChangeFlagState(true, MMT_RED);
	RouteAssignFlag(m_uidBearer, MMT_BLUE);
	}

	else if (uidChar == GetRedCarrier())
	{
	MUID m_uidBearer = MUID(0, 0);
	SetRedCarrier(m_uidBearer);
	SetBlueFlagObtained(false);
	m_pStage->m_WorldItemManager.ChangeFlagState(true, MMT_BLUE);
	RouteAssignFlag(m_uidBearer, MMT_RED);
	}

	SendAssignState();
}

void MMatchRuleTeamCTF::OnEnterBattle(MUID& uidChar)
{
	RouteAssignFlagToJoiner(GetBlueCarrier(), uidChar, MMT_ALL);
	RouteAssignFlagToJoiner(GetRedCarrier(), uidChar, MMT_END);
	RouteAssignState(uidChar);
}


void MMatchRuleTeamCTF::OnGameKill(const MUID& uidAttacker, const MUID& uidVictim)
{
	MMatchObject* pVictim = MMatchServer::GetInstance()->GetObject(uidVictim);

	if (m_pStage != NULL)
	{
		MUID uidChar = pVictim->GetUID();

		if (uidChar == GetBlueCarrier())
		{
		MUID m_uidBearer = MUID(0, 0);
		SetBlueCarrier(m_uidBearer);
		SetRedFlagObtained(false);
		m_pStage->m_WorldItemManager.ChangeFlagState(true, MMT_RED);
		RouteAssignFlag(m_uidBearer, MMT_BLUE);
		SendAssignState();
		}

		else if (uidChar == GetRedCarrier())
		{
		MUID m_uidBearer = MUID(0, 0);
		SetRedCarrier(m_uidBearer);
		SetBlueFlagObtained(false);
		m_pStage->m_WorldItemManager.ChangeFlagState(true, MMT_BLUE);
		RouteAssignFlag(m_uidBearer, MMT_RED);
		SendAssignState();
		}
	}
}
