#ifndef _ZOBJECT_H
#define _ZOBJECT_H

#include "ZPrerequisites.h"
#include "MUID.h"
#include "RTypes.h"
#include "MRTTI.h"
#include "ZModule.h"
#include "ZModule_Movable.h"
#include "ZModule_HPAP.h"
#include "ZCharacterItem.h"

#include "MMemoryProxy.h"

#include <list>
#include <string>

using namespace std;

_USING_NAMESPACE_REALSPACE2

enum ZOBJECTHITTEST {
	ZOH_NONE = 0,
	ZOH_BODY = 1,
	ZOH_HEAD = 2,
	ZOH_LEGS = 3
};

enum ZDAMAGETYPE {
	ZD_NONE = -1,
	ZD_BULLET,
	ZD_MELEE,
	ZD_FALLING,
	ZD_EXPLOSION,
	ZD_BULLET_HEADSHOT,
	ZD_KATANA_SPLASH,
	ZD_HEAL,
	ZD_REPAIR,
	ZD_MAGIC,
	ZD_FIRE,
	ZD_COLD,
	ZD_LIGHTNING,
	ZD_POISON,

	ZD_END
};

enum ZC_ENCHANT {
	ZC_ENCHANT_NONE = 0,
	ZC_ENCHANT_FIRE,
	ZC_ENCHANT_COLD,
	ZC_ENCHANT_LIGHTNING,
	ZC_ENCHANT_POISON,

	ZC_ENCHANT_END
};

struct ZObjectCollision
{
private:
	MProtectValue<bool>		bCollideable;
	MProtectValue<float>	fRadius;
	MProtectValue<float>	fHeight;
public:
	ZObjectCollision() {
		bCollideable.Set_MakeCrc(true);
		fRadius.Set_MakeCrc(0);
		fHeight.Set_MakeCrc(0);
	}

	float GetRadius() { return fRadius.Ref(); }
	float GetHeight() { return fHeight.Ref(); }

	void SetRadius(float r) { fRadius.Set_CheckCrc(r); }
	void SetHeight(float h) { fHeight.Set_CheckCrc(h); }

	bool IsCollideable() { return bCollideable.Ref(); }
	void SetCollideable(bool b) { bCollideable.Set_CheckCrc(b); }
};

class ZObject : public ZModuleContainer
{
	MDeclareRTTI;

public:
	ZModule_Movable* m_pModule_Movable;

	bool					m_bRendered;

	ZCharacterItem			m_Items;

	MProtectValue<rvector>	m_Position;

	RVisualMesh* m_pVMesh;

	rvector					m_Direction;

private:
	bool					m_bVisible;
	float					m_fSpawnTime;
	float					m_fDeadTime;
protected:
	MUID					m_UID;
	bool					m_bIsNPC;
	bool					m_bInitialized;
	bool					m_bInitialized_DebugRegister;

	ZObjectCollision		m_Collision;

	virtual void OnDraw();
	virtual void OnUpdate(float fDelta);
	virtual void OnDie() {}

public:
	ZObject();
	virtual ~ZObject();

	__forceinline const rvector& GetPosition() const
	{
		return m_Position.Ref();
	}

	float GetCollRadius()
	{
		return m_Collision.GetRadius();
	}
	float GetCollHeight()
	{
		return m_Collision.GetHeight();
	}
	rvector GetCenterPos()
	{
		return GetPosition() + rvector(0.0f, 0.0f, m_Collision.GetHeight() * 0.5f);
	}
	virtual bool IsCollideable()
	{
		return m_Collision.IsCollideable();
	}

	__forceinline void SetPosition(const rvector& pos)
	{
		m_Position.Set_CheckCrc(pos);
	}

	void Draw();
	void Update(float fDelta);
	virtual bool Pick(int x, int y, RPickInfo* pInfo);
	virtual bool Pick(int x, int y, rvector* v, float* f);
	virtual bool Pick(const rvector& pos, const rvector& dir, RPickInfo* pInfo = NULL);
	virtual bool GetHistory(rvector* pos, rvector* direction, float fTime);

	void SetVisualMesh(RVisualMesh* pVMesh);
	RVisualMesh* GetVisualMesh() { return m_pVMesh; }
	bool IsVisible() { return m_bVisible; }
	void SetVisible(bool bVisible) { m_bVisible = bVisible; }
	bool GetInitialized() { return m_bInitialized; }
	MUID& GetUID() { return m_UID; }
	void SetUID(MUID& uid) { m_UID = uid; }
	void SetSpawnTime(float fTime);
	float GetSpawnTime() { return m_fSpawnTime; }
	void SetDeadTime(float fTime);
	float GetDeadTime() { return m_fDeadTime; }
	bool IsNPC() { return m_bIsNPC; }
	const rvector& GetDirection() { return m_Direction; }

	const rvector& GetVelocity() { return m_pModule_Movable->GetVelocity(); }
	void SetVelocity(rvector& vel) { m_pModule_Movable->SetVelocity(vel); }
	void SetVelocity(float x, float y, float z) { SetVelocity(rvector(x, y, z)); }
	void AddVelocity(rvector& add) { SetVelocity(GetVelocity() + add); }

	float GetDistToFloor() { return m_pModule_Movable->GetDistToFloor(); }

	ZCharacterItem* GetItems() { return &m_Items; }

public:
	virtual float ColTest(const rvector& pos, const rvector& vec, float radius, rplane* out = 0) { return 1.0f; }
	virtual bool ColTest(const rvector& p1, const rvector& p2, float radius, float fTime);
	virtual bool IsAttackable() { return true; }
	virtual bool IsDie() { return false; }
	virtual void SetDirection(const rvector& dir);
	virtual bool IsGuard() { return false; }
	virtual MMatchTeam GetTeamID() const { return MMT_ALL; }

	virtual ZOBJECTHITTEST HitTest(const rvector& origin, const rvector& to, float fTime, rvector* pOutPos = NULL) = 0;

	virtual void OnBlast(rvector& dir) { }
	virtual void OnBlastDagger(rvector& dir, rvector& pos) { }

	virtual void OnGuardSuccess() { }
	virtual void OnMeleeGuardSuccess() { }

	virtual void OnKnockback(rvector& dir, float fForce) { }

	void Tremble(float fValue, DWORD nMaxTime, DWORD nReturnMaxTime);

	virtual void OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio = 1.f, int nMeleeType = -1);

	virtual void OnDamagedSkill(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio = 1.f, int nMeleeType = -1);

	virtual void OnDamagedAnimation(ZObject* pAttacker, int type) { }

	virtual void OnScream() { }

	__forceinline void OnHealing(ZObject* pOwner, int nHP, int nAP);

	void OnSimpleDamaged(ZObject* pAttacker, float fDamage, float fPiercingRatio);

	virtual void OnStun(float fTime) { }

public:
	virtual bool IsRendered() { return m_bRendered; }

protected:
	bool Move(rvector& diff) { return m_pModule_Movable->Move(diff); }

private:
	void AddHealEffect();
};

bool IsPlayerObject(ZObject* pObject);

__forceinline void ZObject::OnHealing(ZObject* pOwner, int nHP, int nAP)
{
	if (nHP <= 0 && nAP <= 0) return;

	ZModule_HPAP* pModule = (ZModule_HPAP*)GetModule(ZMID_HPAP);
	if (!pModule) return;

	pModule->SetHP(min(pModule->GetHP() + nHP, pModule->GetMaxHP()));
	pModule->SetAP(min(pModule->GetAP() + nAP, pModule->GetMaxAP()));

	AddHealEffect();
}

#endif