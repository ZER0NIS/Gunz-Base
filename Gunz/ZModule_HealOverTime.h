#ifndef _ZMODULE_HEALOVERTIME_H
#define _ZMODULE_HEALOVERTIME_H

#include "ZModule.h"
#include "ZModuleID.h"

// 시간의 흐름에 따라서 HP나 AP가 회복되는 효과를 위한 모듈

class ZModule_HealOverTime : public ZModule {
	MProtectValue<float>	m_fBeginTime;		// 힐 시작한 시간
	MProtectValue<float>	m_fNextHealTime;	// 다음번 힐 받을 시간

	MProtectValue<float>	m_fHeal;			// 1회 회복량
	MProtectValue<int>		m_numHealDesire;	// 총 목표 회복횟수
	MProtectValue<int>		m_numHealDone;		// 현재 회복한 횟수

	MProtectValue<MMatchDamageType> m_type;
	MMatchItemEffectId		m_nEffectId;
	int						m_nItemId;			// 효과를 일으킨 아이템ID

	MProtectValue<bool>		m_bOnHeal;	// 힐 받고 있는 중인가

public:
	DECLARE_ID(ZMID_HEALOVERTIME)
	ZModule_HealOverTime();

	virtual bool Update(float fElapsed);
	virtual void InitStatus();

	void BeginHeal(MMatchDamageType type, int nHealAmount, int numHeal, MMatchItemEffectId effectId, int nItemId);
	bool IsOnHeal() { return m_bOnHeal.Ref(); }
	float GetHealBeginTime() { return m_fBeginTime.Ref(); }

	bool GetHealOverTimeBuffInfo(MTD_BuffInfo& out);

	void ShiftFugitiveValues();
};

#endif