#include "stdafx.h"
#include "ZModule_HealOverTime.h"
#include "ZGame.h"
#include "ZApplication.h"
#include "ZModule_HPAP.h"

int GetEffectLevel();

ZModule_HealOverTime::ZModule_HealOverTime()
{
	m_fBeginTime.Set_MakeCrc(0);
	m_fNextHealTime.Set_MakeCrc(0);

	m_fHeal.Set_MakeCrc(0);
	m_numHealDesire.Set_MakeCrc(0);
	m_numHealDone.Set_MakeCrc(0);

	m_type.Set_MakeCrc(MMDT_HEAL);
	m_nEffectId = MMIEI_NONE;
	m_nItemId = 0;

	m_bOnHeal.Set_MakeCrc(false);
}

void ZModule_HealOverTime::InitStatus()
{
	Active(false);
}

bool ZModule_HealOverTime::Update(float fElapsed)
{
	ZCharacter* pChar = MDynamicCast(ZCharacter, m_pContainer);
	if (!pChar)
	{
		//_ASSERT(0);
	}
	else
	{
		if(ZGetGame()->GetTime() > m_fNextHealTime.Ref()) {
			m_fNextHealTime.Set_CheckCrc(m_fNextHealTime.Ref()+1.f);		// 1초마다 힐을 받는다
			m_numHealDone.Set_CheckCrc(m_numHealDone.Ref()+1);

			if(pChar->IsDie())
			{
				m_bOnHeal.Set_CheckCrc(false);
			}
			else
			{
				switch (m_type.Ref())
				{
				case MMDT_HEAL:
					pChar->SetHP( min( pChar->GetHP() + m_fHeal.Ref(), pChar->GetMaxHP() ) );
					break;
				case MMDT_REPAIR:
					pChar->SetAP( min( pChar->GetAP() + m_fHeal.Ref(), pChar->GetMaxAP() ) );
					break;
				default:
					{
						break;
					}
					//_ASSERT(0);
				}
				ZGetEffectManager()->AddPotionEffect( pChar->GetPosition(), pChar, m_nEffectId );
			}
		}
	}

	if(m_numHealDone.Ref() == m_numHealDesire.Ref()) {
		m_bOnHeal.Set_CheckCrc(false);
		return false;
	}
	return true;
}

void ZModule_HealOverTime::BeginHeal(MMatchDamageType type, int nHealAmount, int numHeal, MMatchItemEffectId effectId, int nItemId)
{
	if (type != MMDT_HEAL && type != MMDT_REPAIR) { return; }

	m_type.Set_CheckCrc(type);
	m_nEffectId = effectId;

	float fCurrTime = ZGetGame()->GetTime();

	m_fBeginTime.Set_CheckCrc(fCurrTime);

	m_fHeal.Set_CheckCrc((float)nHealAmount);
	m_numHealDesire.Set_CheckCrc(numHeal);
	m_numHealDone.Set_CheckCrc(0);

	if (!m_bOnHeal.Ref())
	{
		m_fNextHealTime.Set_CheckCrc(fCurrTime);
	}

	m_bOnHeal.Set_CheckCrc(true);
	m_nItemId = nItemId;

	Active();
}

bool ZModule_HealOverTime::GetHealOverTimeBuffInfo( MTD_BuffInfo& out )
{
	if (!IsOnHeal()) return false;

	out.nItemId = m_nItemId;
	out.nRemainedTime = m_numHealDesire.Ref() - m_numHealDone.Ref();
	return true;
}

void ZModule_HealOverTime::ShiftFugitiveValues()
{
	m_fBeginTime.ShiftHeapPos_CheckCrc();
	m_fNextHealTime.ShiftHeapPos_CheckCrc();

	m_fHeal.ShiftHeapPos_CheckCrc();
	m_numHealDesire.ShiftHeapPos_CheckCrc();
	m_numHealDone.ShiftHeapPos_CheckCrc();

	m_type.ShiftHeapPos_CheckCrc();

	m_bOnHeal.ShiftHeapPos_CheckCrc();
}