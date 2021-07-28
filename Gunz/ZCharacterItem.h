#pragma once

#include "MMatchItem.h"
#include "ZItem.h"
#include "ZFile.h"
#include <list>
#include <algorithm>

class ZCharacterItem
{
private:
protected:
	ZItem					m_Items[MMCIP_END];
	MMatchCharItemParts		m_nSelectedWeapon;
	bool Confirm(MMatchCharItemParts parts, MMatchItemDesc* pDesc);
	bool IsWeaponItem(MMatchCharItemParts parts);
public:
	ZCharacterItem();
	virtual ~ZCharacterItem();
	void SelectWeapon(MMatchCharItemParts parts);
	bool EquipItem(MMatchCharItemParts parts, int nItemDescID, int nItemCount = 1);

	bool Reload();

	ZItem* GetItem(MMatchCharItemParts parts)
	{
		if ((parts < MMCIP_HEAD) || (parts >= MMCIP_END))
		{
			return NULL;
		}
		return &m_Items[(int)parts];
	}
	ZItem* GetSelectedWeapon();
	MMatchCharItemParts GetSelectedWeaponParts() { return (MMatchCharItemParts)m_nSelectedWeapon; }

	MMatchCharItemParts GetSelectedWeaponType() {
		return m_nSelectedWeapon;
	}

	bool Save(ZFile* file);
	bool Load(ZFile* file, int nReplayVersion);
};