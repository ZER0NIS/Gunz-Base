#include "stdafx.h"
#include "ZRuleBerserker.h"

#define BERSERKER_UPDATE_HEALTH_TIME		5.0f
#define BERSERKER_UPDATE_HEALTH				10
#define BERSERKER_BONUS_HEALTH				50

ZRuleBerserker::ZRuleBerserker(ZMatch* pMatch) : ZRule(pMatch), m_uidBerserker(0, 0) { }

ZRuleBerserker::~ZRuleBerserker() { }

bool ZRuleBerserker::OnCommand(MCommand* pCommand)
{
	if (!ZGetGame())
		return false;

	switch (pCommand->GetID())
	{
	case MC_MATCH_ASSIGN_BERSERKER:
	{
		MUID uidBerserker;
		pCommand->GetParameter(&uidBerserker, 0, MPT_UID);

		AssignBerserker(uidBerserker);
	}
	break;
	case MC_MATCH_GAME_DEAD:
	{
		MUID uidAttacker, uidVictim;
		unsigned long int nAttackerArg, nVictimArg;

		pCommand->GetParameter(&uidAttacker, 0, MPT_UID);
		pCommand->GetParameter(&nAttackerArg, 1, MPT_UINT);
		pCommand->GetParameter(&uidVictim, 2, MPT_UID);
		pCommand->GetParameter(&nVictimArg, 3, MPT_UINT);

		bool bSuicide = false;
		if (uidAttacker == uidVictim) bSuicide = true;

		if ((uidAttacker != MUID(0, 0)) && (uidAttacker == m_uidBerserker))
		{
			if (!bSuicide)
			{
				ZCharacter* pAttacker = ZGetGame()->m_CharacterManager.Find(uidAttacker);
				BonusHealth(pAttacker);
			}
		}
	}
	break;
	}

	return false;
}

void ZRuleBerserker::OnResponseRuleInfo(MTD_RuleInfo* pInfo)
{
	MTD_RuleInfo_Berserker* pBerserkerRule = (MTD_RuleInfo_Berserker*)pInfo;
	AssignBerserker(pBerserkerRule->uidBerserker);
}

void ZRuleBerserker::AssignBerserker(MUID& uidBerserker)
{
	if (!ZGetGame()) return;

	for (ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.begin();
		itor != ZGetGame()->m_CharacterManager.end(); ++itor)
	{
		ZCharacter* pCharacter = (*itor).second;
		pCharacter->SetTagger(false);
	}

	ZCharacter* pBerserkerChar = ZGetGame()->m_CharacterManager.Find(uidBerserker);
	if (pBerserkerChar)
	{
		ZGetEffectManager()->AddBerserkerIcon(pBerserkerChar);
		pBerserkerChar->SetTagger(true);

		if (!pBerserkerChar->IsDie())
		{
			float fMaxHP = pBerserkerChar->GetMaxHP();
			float fMaxAP = pBerserkerChar->GetMaxAP();

			pBerserkerChar->SetHP(fMaxHP);
			pBerserkerChar->SetAP(fMaxAP);

			if (uidBerserker == ZGetMyUID())
				ZGetGameInterface()->PlayVoiceSound(VOICE_GOT_BERSERKER, 1600);
			else
				ZGetGameInterface()->PlayVoiceSound(VOICE_BERSERKER_DOWN, 1200);
		}
	}

	m_uidBerserker = uidBerserker;
	m_fElapsedHealthUpdateTime = 0.0f;
}

void ZRuleBerserker::OnUpdate(float fDelta)
{
	m_fElapsedHealthUpdateTime += fDelta;

	if (BERSERKER_UPDATE_HEALTH_TIME < m_fElapsedHealthUpdateTime)
	{
		m_fElapsedHealthUpdateTime = 0.0f;

		ZCharacter* pBerserker = ZGetGame()->m_CharacterManager.Find(m_uidBerserker);
		PenaltyHealth(pBerserker);
	}
}

void ZRuleBerserker::BonusHealth(ZCharacter* pBerserker)
{
	if (pBerserker)
	{
		if (pBerserker->IsDie())
			return;

		float fBonusAP = 0.0f;
		float fBonusHP = BERSERKER_BONUS_HEALTH;

		float fMaxHP = pBerserker->GetMaxHP();
		if ((fMaxHP - pBerserker->GetHP()) < BERSERKER_BONUS_HEALTH)
		{
			fBonusHP = fMaxHP - pBerserker->GetHP();
			fBonusAP = BERSERKER_BONUS_HEALTH - fBonusHP;
		}

		pBerserker->SetHP(pBerserker->GetHP() + fBonusHP);
		pBerserker->SetAP(pBerserker->GetAP() + fBonusAP);
	}
}

void ZRuleBerserker::PenaltyHealth(ZCharacter* pBerserker)
{
	if (pBerserker)
	{
		if (pBerserker->GetAP() > 0)
		{
			float fAP = max(0.0f, pBerserker->GetAP() - BERSERKER_UPDATE_HEALTH);
			pBerserker->SetAP(fAP);
		}
		else
		{
			float fHP = max(1.0f, pBerserker->GetHP() - BERSERKER_UPDATE_HEALTH);
			pBerserker->SetHP(fHP);
		}

		pBerserker->SetLastAttacker(pBerserker->GetUID());
	}
}