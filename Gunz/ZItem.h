#pragma once

#include "MMatchItem.h"

class ZItem : public MMatchItem
{
private:
protected:
	MProtectValue<int>	m_nBulletSpare;
	MProtectValue<int>	m_nBulletCurrMagazine;
public:
	ZItem();
	virtual ~ZItem();

	void InitBullet(int nBulletSpare, int nBulletCurrMagazine);
	bool Shot();
	bool Reload();
	bool isReloadable();

	int GetBulletSpare() { return m_nBulletSpare.Ref(); }
	void SetBulletSpare(int nBullet) { m_nBulletSpare.Set_CheckCrc(nBullet); }
	int GetBulletCurrMagazine() { return m_nBulletCurrMagazine.Ref(); }
	void SetBulletCurrMagazine(int nBulletPerMagazine) { m_nBulletCurrMagazine.Set_CheckCrc(nBulletPerMagazine); }

	static float GetPiercingRatio(MMatchWeaponType wtype, RMeshPartsType partstype);
	float GetKnockbackForce();
};