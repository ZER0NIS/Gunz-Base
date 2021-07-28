#pragma once

#include "ZModule.h"
#include "ZModuleID.h"

class ZModule_HealOverTime : public ZModule {
	MProtectValue<float>	m_fBeginTime;
	MProtectValue<float>	m_fNextHealTime;

	MProtectValue<float>	m_fHeal;
	MProtectValue<int>		m_numHealDesire;
	MProtectValue<int>		m_numHealDone;

	MProtectValue<MMatchDamageType> m_type;
	MMatchItemEffectId		m_nEffectId;
	int						m_nItemId;

	MProtectValue<bool>		m_bOnHeal;

public:
	DECLARE_ID(ZMID_HEALOVERTIME)
	ZModule_HealOverTime();

	virtual bool Update(float fElapsed);
	virtual void InitStatus();

	void BeginHeal(MMatchDamageType type, int nHealAmount, int numHeal, MMatchItemEffectId effectId, int nItemId);
	bool IsOnHeal() { return m_bOnHeal.Ref(); }
	float GetHealBeginTime() { return m_fBeginTime.Ref(); }

	bool GetHealOverTimeBuffInfo(MTD_BuffInfo& out);
};