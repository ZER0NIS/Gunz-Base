#ifndef _ZMODULE_HPAP_H
#define _ZMODULE_HPAP_H

#include "ZModule.h"
#include "ZModuleID.h"
#include "MMemoryProxy.h"

class ZModule_HPAP : public ZModule {
private:
	MProtectValue<float>	m_fMaxHP;

	bool					m_bAccumulationDamage;
	MProtectValue<float>	m_fMask;

	bool					m_bRealDamage_DebugRegister;
	MProtectValue<float>	m_fHP;

	MUID					m_LastAttacker;
	MProtectValue<float>	m_fAP;
	MProtectValue<bool>		m_bRealDamage;
	float					m_fAccumulationDamage;
	MProtectValue<float>	m_fMaxAP;

	float	GetMask() { return m_fMask.Ref(); }

	bool	CheckQuestCheet();

public:
	DECLARE_ID(ZMID_HPAP)
	ZModule_HPAP();
	~ZModule_HPAP();

	float	GetMaxHP() { return m_fMaxHP.Ref() - GetMask(); }
	float	GetMaxAP() { return m_fMaxAP.Ref() - GetMask(); }
	__forceinline void	SetMaxHP(float fMaxHP) { m_fMaxHP.Set_CheckCrc(fMaxHP + GetMask()); }
	__forceinline void	SetMaxAP(float fMaxAP) { m_fMaxAP.Set_CheckCrc(fMaxAP + GetMask()); }

	float	GetHP();
	float	GetAP();
	__forceinline void	SetHP(float nHP);
	__forceinline void	SetAP(float nAP);

	bool	IsFullHP() { return GetHP() == GetMaxHP(); }
	bool	IsFullAP() { return GetAP() == GetMaxAP(); }

	void	InitAccumulationDamage() { m_fAccumulationDamage = 0.f; }
	float	GetAccumulationDamage() { return m_fAccumulationDamage; }
	void	AccumulateDamage(float fDamege) { m_fAccumulationDamage += fDamege; }
	bool	IsAccumulationDamage() { return m_bAccumulationDamage; }
	void	EnableAccumulationDamage(bool bAccumulationDamage) { m_bAccumulationDamage = bAccumulationDamage; }

	void	SetRealDamage(bool bReal)
	{
		m_bRealDamage.Set_CheckCrc(bReal);

		m_bRealDamage_DebugRegister = bReal;
	}
	bool	GetRealDamage() { return m_bRealDamage.Ref(); }

	void	SetLastAttacker(MUID uid) { m_LastAttacker = uid; }
	MUID	GetLastAttacker() { return m_LastAttacker; }

	void	OnDamage(MUID uidAttacker, float fDamage, float fRatio);

	void	InitStatus();
};

__forceinline void ZModule_HPAP::SetHP(float fHP)
{
	fHP = min(max(0, fHP), GetMaxHP());
	m_fHP.Set_CheckCrc(fHP + GetMask());
}

__forceinline void ZModule_HPAP::SetAP(float fAP)
{
	fAP = min(max(0, fAP), GetMaxAP());
	m_fAP.Set_CheckCrc(fAP + GetMask());
}

#endif