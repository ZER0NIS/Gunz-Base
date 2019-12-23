#ifndef _ZITEM_H
#define _ZITEM_H

#include "MMatchItem.h"


// 이거 나중에 MBaseItem을 부모로 하도록 바뀌어야 한다.
class ZItem : public MMatchItem
{
private:
protected:
	// weapon에만 사용하는 변수들
	MProtectValue<int>	m_nBulletSpare;			// 여분의 탄알수(현재 장전된 탄수는 뺀것)
	MProtectValue<int>	m_nBulletCurrMagazine;	// 현재 탄창에 들어있는 탄알수
public:
	ZItem();
	virtual ~ZItem();

	// weapon에서만 쓰이는 함수
	void InitBullet(int nBulletSpare, int nBulletCurrMagazine);
	bool Shot();
	bool Reload();
	bool isReloadable();

	int GetBulletSpare()				{ return m_nBulletSpare.Ref(); }
	void SetBulletSpare(int nBullet) { m_nBulletSpare.Set_CheckCrc(nBullet); }
	int GetBulletCurrMagazine()	{ return m_nBulletCurrMagazine.Ref(); }
	void SetBulletCurrMagazine(int nBulletPerMagazine)	{ m_nBulletCurrMagazine.Set_CheckCrc(nBulletPerMagazine); }

	static float GetPiercingRatio(MMatchWeaponType wtype,RMeshPartsType partstype);
	float GetKnockbackForce();

	void ShiftFugitiveValues();
};


#endif