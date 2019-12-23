#include "stdafx.h"
#include "ZRuleDuel.h"
#include "ZScreenEffectManager.h"

ZRuleDuel::ZRuleDuel(ZMatch* pMatch) : ZRule(pMatch)
{

}

ZRuleDuel::~ZRuleDuel()
{

}

bool ZRuleDuel::OnCommand(MCommand* pCommand)
{
	if (!ZGetGame()) return false;

	switch (pCommand->GetID())
	{
	case MC_MATCH_DUEL_QUEUEINFO:
		{
	
			pCommand->GetParameter(&QInfo,		0, MPT_BLOB);

			if (QInfo.m_bIsRoundEnd)
			{
				rvector pos = ZGetGame()->m_pMyCharacter->GetPosition();
				rvector dir = ZGetGame()->m_pMyCharacter->m_DirectionLower;

				if ((QInfo.m_uidChampion == ZGetMyUID()) || (QInfo.m_uidChallenger == ZGetMyUID()))
				{
					ZMapSpawnData* pSpawnData = ZGetGame()->GetMapDesc()->GetSpawnManager()->GetData( QInfo.m_uidChampion == ZGetMyUID() ? 0 : 1);
					ZPostRequestSpawn(ZGetMyUID(), pSpawnData->m_Pos, pSpawnData->m_Dir);
					ZGetGame()->SetSpawnRequested(true);
				}
				else
				{
					ZCharacter* cha = ZGetCharacterManager()->Find(QInfo.m_uidChampion);
					if (cha != NULL)
						cha->Revival();
					cha = ZGetCharacterManager()->Find(QInfo.m_uidChallenger);
					if (cha != NULL)
						cha->Revival();
					ZGetCombatInterface()->SetObserverMode(true);
				}
			}

			if ((QInfo.m_uidChampion != ZGetGame()->m_pMyCharacter->GetUID()) && (QInfo.m_uidChallenger != ZGetGame()->m_pMyCharacter->GetUID()) && !QInfo.m_bIsRoundEnd)
				ZGetCombatInterface()->SetObserverMode(true);
			else
			{
				for (int i=0; i<QInfo.m_nQueueLength; i++)
				{
					ZCharacter* cha = ZGetCharacterManager()->Find(QInfo.m_WaitQueue[i]);
					if (cha != NULL)
					{
						cha->SetVisible(false);
						cha->ForceDie();
					}
				}
			}
		}
		break;
	}

	return false;
}


int ZRuleDuel::GetQueueIdx(const MUID& uidChar)
{
	if (uidChar == QInfo.m_uidChampion) return 0;
	if (uidChar == QInfo.m_uidChallenger) return 1;
	for (int i=0; i<QInfo.m_nQueueLength; i++)
		if (QInfo.m_WaitQueue[i] == uidChar) return i+2;

	return 100;
}