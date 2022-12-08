#include "stdafx.h"
#include "ZObject.h"
#include "ZMyCharacter.h"
#include "ZNetCharacter.h"
#include "ZGame.h"
#include "CheckReturnCallStack.h"

MImplementRTTI(ZObject, ZModuleContainer);

ZObject::ZObject() : m_Direction(1, 0, 0), m_bInitialized(false), m_UID(MUID(0, 0)),
m_fSpawnTime(0.0f), m_fDeadTime(0.0f), m_pVMesh(NULL), m_bVisible(false),
m_bIsNPC(false)
{
	m_Position.Set_MakeCrc(rvector(0, 0, 0));

	m_pModule_Movable = new ZModule_Movable;
	AddModule(m_pModule_Movable, true);
}

ZObject::~ZObject()
{
	RemoveModule(m_pModule_Movable);
	delete m_pModule_Movable;
}

void ZObject::OnDraw()
{
}

void ZObject::OnUpdate(float fDelta)
{
}

void ZObject::Draw()
{
	OnDraw();
}

void ZObject::Update(float fDelta)
{
	OnUpdate(fDelta);
}

bool ZObject::Pick(int x, int y, RPickInfo* pInfo)
{
	if (m_pVMesh)
	{
		return m_pVMesh->Pick(x, y, pInfo);
	}

	return false;
}

bool ZObject::Pick(int x, int y, rvector* v, float* f)
{
	RPickInfo info;
	bool hr = Pick(x, y, &info);
	*v = info.vOut;
	*f = info.t;
	return hr;
}

bool ZObject::Pick(const rvector& pos, const rvector& dir, RPickInfo* pInfo)
{
	if (m_pVMesh)
	{
		return m_pVMesh->Pick(pos, dir, pInfo);
	}

	return false;
}

bool ZObject::GetHistory(rvector* pos, rvector* direction, float fTime)
{
	if (m_pVMesh == NULL) return false;

	*pos = GetPosition();
	*direction = m_Direction;
	return true;
}

void ZObject::SetDirection(const rvector& dir)
{
	m_Direction = dir;
}

void ZObject::SetSpawnTime(float fTime)
{
	m_fSpawnTime = fTime;
}

void ZObject::SetDeadTime(float fTime)
{
	m_fDeadTime = fTime;
}

bool IsPlayerObject(ZObject* pObject)
{
	return ((MIsExactlyClass(ZNetCharacter, pObject)) || ((MIsExactlyClass(ZMyCharacter, pObject))));
}

void ZObject::Tremble(float fValue, DWORD nMaxTime, DWORD nReturnMaxTime)
{
	if (m_pVMesh)
	{
		RFrameTime* ft = &m_pVMesh->m_FrameTime;
		if (ft && !ft->m_bActive)
			ft->Start(fValue, nMaxTime, nReturnMaxTime);
	}
}

void ZObject::OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio, int nMeleeType)
{
	ZModule_HPAP* pModule = (ZModule_HPAP*)GetModule(ZMID_HPAP);

	bool bDebugRegisterValue = !pModule;
	if (!pModule) {
		PROTECT_DEBUG_REGISTER(bDebugRegisterValue) {
			return;
		}
	}

	pModule->OnDamage(pAttacker ? pAttacker->GetUID() : MUID(0, 0), fDamage, fPiercingRatio);
}

void ZObject::OnDamagedSkill(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio, int nMeleeType)
{
	ZModule_HPAP* pModule = (ZModule_HPAP*)GetModule(ZMID_HPAP);
	if (!pModule) return;

	pModule->OnDamage(pAttacker ? pAttacker->GetUID() : MUID(0, 0), fDamage, fPiercingRatio);
}

void ZObject::OnSimpleDamaged(ZObject* pAttacker, float fDamage, float fPiercingRatio)
{
	ZModule_HPAP* pModule = (ZModule_HPAP*)GetModule(ZMID_HPAP);
	if (!pModule) return;

	pModule->OnDamage(pAttacker ? pAttacker->GetUID() : MUID(0, 0), fDamage, fPiercingRatio);
}

bool ZObject::ColTest(const rvector& p1, const rvector& p2, float radius, float fTime)
{
	rvector p, d;
	if (GetHistory(&p, &d, fTime))
	{
		float fCollHeight = m_Collision.GetHeight();
		float fCollRadius = m_Collision.GetRadius();

		rvector a1 = p + rvector(0, 0, (min(fCollHeight, fCollRadius) / 2.0f));
		rvector a2 = p + rvector(0, 0, fCollHeight - (min(fCollHeight, fCollRadius) / 2.0f));

		rvector ap, cp;
		float dist = GetDistanceBetweenLineSegment(p1, p2, a1, a2, &ap, &cp);

		if (dist < (radius + fCollRadius)) return true;
	}

	return false;
}

void ZObject::SetVisualMesh(RVisualMesh* pVMesh)
{
	m_pVMesh = pVMesh;
}

void ZObject::AddHealEffect()
{
	ZGetEffectManager()->AddHealEffect(GetPosition(), this);
}