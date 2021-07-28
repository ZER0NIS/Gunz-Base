#include "stdafx.h"

#include "ZItem.h"
#include "crtdbg.h"

ZItem::ZItem() : MMatchItem()
{
	m_nBulletSpare.Set_MakeCrc(0);
	m_nBulletCurrMagazine.Set_MakeCrc(0);
}

ZItem::~ZItem()
{
}

void ZItem::InitBullet(int nBulletSpare, int nBulletCurrMagazine)
{
	if (m_pDesc == NULL) return;
	if (!m_pDesc->IsSpendableItem() &&
		nBulletSpare < nBulletCurrMagazine)
	{
		nBulletCurrMagazine = nBulletSpare;
	}

	if (GetItemType() == MMIT_RANGE)
	{
		m_nBulletCurrMagazine.Set_CheckCrc(nBulletCurrMagazine);
		m_nBulletSpare.Set_CheckCrc(nBulletSpare - nBulletCurrMagazine);
	}
	else if (GetItemType() == MMIT_CUSTOM)
	{
		m_nBulletCurrMagazine.Set_CheckCrc(nBulletCurrMagazine);
		m_nBulletSpare.Set_CheckCrc(nBulletSpare);
	}
}

bool ZItem::Shot()
{
	if (m_pDesc == NULL) {
		return false;
	}

	((GetItemType() == MMIT_MELEE) ||
		(GetItemType() == MMIT_CUSTOM));

	if (GetItemType() == MMIT_MELEE)
	{
		return true;
	}
	else if (GetItemType() == MMIT_CUSTOM)
	{
		if (m_nBulletCurrMagazine.Ref() > 0)
		{
			m_nBulletCurrMagazine.CheckCrc();
			m_nBulletCurrMagazine.Ref() -= 1;
			m_nBulletCurrMagazine.MakeCrc();
		}
		else return false;
	}
	else
	{
		if (m_nBulletCurrMagazine.Ref() > 0)
		{
			m_nBulletCurrMagazine.CheckCrc();
			m_nBulletCurrMagazine.Ref() -= 1;
			m_nBulletCurrMagazine.MakeCrc();
		}
		else return false;
	}

	return true;
}

bool ZItem::Reload()
{
	if (m_pDesc == NULL)
	{
		return false;
	}

	int nAddedBullet = m_pDesc->m_nMagazine.Ref() - m_nBulletCurrMagazine.Ref();
	if (nAddedBullet > m_nBulletSpare.Ref()) nAddedBullet = m_nBulletSpare.Ref();
	if (nAddedBullet <= 0) return false;

	m_nBulletCurrMagazine.Set_CheckCrc(m_nBulletCurrMagazine.Ref() + nAddedBullet);
	m_nBulletSpare.Set_CheckCrc(m_nBulletSpare.Ref() - nAddedBullet);

	return true;
}

bool ZItem::isReloadable()
{
	if (m_pDesc == NULL) {
		return false;
	}

	if (GetItemType() == MMIT_CUSTOM) {
		return false;
	}

	if (m_nBulletSpare.Ref() > 0) {
		if (m_pDesc->m_nMagazine.Ref() != m_nBulletCurrMagazine.Ref())
			return true;
	}

	return false;
}

float ZItem::GetPiercingRatio(MMatchWeaponType wtype, RMeshPartsType partstype)
{
	float fRatio = 0.5f;

	bool bHead = false;

	if (partstype == eq_parts_head) {
		bHead = true;
	}

	switch (wtype) {
	case MWT_DAGGER:
	case MWT_DUAL_DAGGER:
	{
		if (bHead)	fRatio = 0.75f;
		else		fRatio = 0.7f;
	}
	break;

	case MWT_KATANA:
	{
		if (bHead)	fRatio = 0.65f;
		else		fRatio = 0.6f;
	}
	break;

	case MWT_DOUBLE_KATANA:
	{
		if (bHead)	fRatio = 0.65f;
		else		fRatio = 0.6f;
	}
	break;

	case MWT_GREAT_SWORD:
	{
		if (bHead)	fRatio = 0.65f;
		else		fRatio = 0.6f;
	}
	break;

	case MWT_PISTOL:
	case MWT_PISTOLx2:
	{
		if (bHead)	fRatio = 0.7f;
		else		fRatio = 0.5f;
	}
	break;

	case MWT_REVOLVER:
	case MWT_REVOLVERx2:
	{
		if (bHead)	fRatio = 0.75f;
		else		fRatio = 0.55f;
	}
	break;

	case MWT_SMG:
	case MWT_SMGx2:
	{
		if (bHead)	fRatio = 0.5f;
		else		fRatio = 0.3f;
	}
	break;

	case MWT_SHOTGUN:
	case MWT_SAWED_SHOTGUN:
	{
		if (bHead)	fRatio = 0.2f;
		else		fRatio = 0.2f;
	}
	break;

	case MWT_MACHINEGUN:
	{
		if (bHead)	fRatio = 0.8f;
		else		fRatio = 0.4f;
	}
	break;

	case MWT_RIFLE:
	{
		if (bHead)	fRatio = 0.8f;
		else		fRatio = 0.4f;
	}
	break;

	case MWT_SNIFER:
	{
		if (bHead)	fRatio = 0.8f;
		else		fRatio = 0.4f;
	}
	break;

	case MWT_FRAGMENTATION:
	case MWT_FLASH_BANG:
	case MWT_SMOKE_GRENADE:
	case MWT_ROCKET:
	{
		if (bHead)	fRatio = 0.4f;
		else		fRatio = 0.4f;
	}
	break;

	case MWT_TRAP:
	{
		fRatio = 0;
	}
	break;

	case MWT_DYNAMITYE:
	{
		fRatio = 0.4f;
	}
	break;

	default:
		break;
	}

	return fRatio;
}

float ZItem::GetKnockbackForce()
{
	float fKnockbackForce = 0.0f;

	MMatchItemDesc* pDesc = GetDesc();
	if (pDesc) {
		if (pDesc->m_nType.Ref() == MMIT_MELEE) {
			fKnockbackForce = 200.0f;
		}
		else
			if (pDesc->m_nType.Ref() == MMIT_RANGE) {
				if (pDesc->m_pEffect != NULL)
					fKnockbackForce = (float)(pDesc->m_pEffect->m_nKnockBack);
				else
					fKnockbackForce = 200.0f;
			}
	}
	else {
		return 0;
	}

	return fKnockbackForce;
}