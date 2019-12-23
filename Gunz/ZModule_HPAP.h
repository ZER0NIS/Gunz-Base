#ifndef _ZMODULE_HPAP_H
#define _ZMODULE_HPAP_H

#include "ZModule.h"
#include "ZModuleID.h"
#include "MMemoryProxy.h"

class ZModule_HPAP : public ZModule {
private:
	// 핵 제작을 방해하기 위해 멤버변수의 위치를 빌드때마다 뒤섞기 위한 주석매크로(runtime/ShuffleCode.bat 실행)
	// 멤버변수들 모두 중요하다. 무적핵을 만들기 위해 사용된다.
	/* [[SHUFFLE_LINE]] moduleHPAP */	MProtectValue<float>	m_fMaxHP;

	/* [[SHUFFLE_LINE]] moduleHPAP */	bool					m_bAccumulationDamage;	// 누적 대미지 on off
	/* [[SHUFFLE_LINE]] moduleHPAP */	MProtectValue<float>	m_fMask;				// HP AP 숫자를 감추기위해 더하는 수

	/* [[SHUFFLE_LINE]] moduleHPAP */	bool					m_bRealDamage_DebugRegister;	//jintriple3 디버그 레지스터 핵 용..
	/* [[SHUFFLE_LINE]] moduleHPAP */	MProtectValue<float>	m_fHP;
	
	/* [[SHUFFLE_LINE]] moduleHPAP */	MUID					m_LastAttacker;		///< 제일 마지막에 나한테 공격한 사람
	/* [[SHUFFLE_LINE]] moduleHPAP */	MProtectValue<float>	m_fAP;
	/* [[SHUFFLE_LINE]] moduleHPAP */	MProtectValue<bool>		m_bRealDamage;		// 실제로 데미지를 먹는지. (내가 컨트롤 안하는 것들은 안먹도록)
	/* [[SHUFFLE_LINE]] moduleHPAP */	float					m_fAccumulationDamage;	// 내가 죽기전까지 얻은 대미지 누적값(듀얼토너먼트 때문에 추가)
	/* [[SHUFFLE_LINE]] moduleHPAP */	MProtectValue<float>	m_fMaxAP;

	float	GetMask() { return m_fMask.Ref(); }

	bool	CheckQuestCheet();

public:
	DECLARE_ID(ZMID_HPAP)
	ZModule_HPAP();
	~ZModule_HPAP();

	float	GetMaxHP()				{ return m_fMaxHP.Ref() - GetMask(); }
	float	GetMaxAP()				{ return m_fMaxAP.Ref() - GetMask(); }
	__forceinline void	SetMaxHP(float fMaxHP) { m_fMaxHP.Set_CheckCrc( fMaxHP + GetMask()); }
	__forceinline void	SetMaxAP(float fMaxAP) { m_fMaxAP.Set_CheckCrc( fMaxAP + GetMask()); }


	float	GetHP();
	float	GetAP();
	__forceinline void	SetHP(float nHP);
	__forceinline void	SetAP(float nAP);

	bool	IsFullHP() { return GetHP()==GetMaxHP(); }
	bool	IsFullAP() { return GetAP()==GetMaxAP(); }

	void	InitAccumulationDamage() { m_fAccumulationDamage = 0.f; }
	float	GetAccumulationDamage()	{ return m_fAccumulationDamage; }
	void	AccumulateDamage(float fDamege) { m_fAccumulationDamage += fDamege; }
	bool	IsAccumulationDamage() { return m_bAccumulationDamage; }
	void	EnableAccumulationDamage(bool bAccumulationDamage) { m_bAccumulationDamage = bAccumulationDamage; }

	void	SetRealDamage(bool bReal) 
	{ 
		m_bRealDamage.Set_CheckCrc(bReal);

		/**
		디버그 레지스터 해킹을 막기위한 조건문의 
		코드 최적화 과정을 막기 위해서.
		자세한건 영진씨한테로 ㄱㄱ 
		*/
		m_bRealDamage_DebugRegister = bReal;
	}
	bool	GetRealDamage() { return m_bRealDamage.Ref(); }
	
	void	SetLastAttacker(MUID uid)	{ m_LastAttacker = uid; }
	MUID	GetLastAttacker() { return m_LastAttacker; }

	void	OnDamage(MUID uidAttacker,float fDamage, float fRatio);

	void	InitStatus();
	void	ShiftFugitiveValues();
};

__forceinline void ZModule_HPAP::SetHP(float fHP) 
{ 
	fHP = min(max(0,fHP),GetMaxHP());
	m_fHP.Set_CheckCrc( fHP+GetMask());
}

__forceinline void ZModule_HPAP::SetAP(float fAP) 
{ 
	fAP = min(max(0,fAP),GetMaxAP());
	m_fAP.Set_CheckCrc( fAP+GetMask()); 
}


#endif