#include "stdafx.h"

#include "ZCharacterItem.h"
#include "crtdbg.h"

ZCharacterItem::ZCharacterItem()
{
	//memset(&m_Items, 0, sizeof(m_Items));
	m_nSelectedWeapon = MMCIP_END;

	// 우선 임의로 아이템 지급

#ifndef _PUBLISH
	EquipItem(MMCIP_MELEE,		2);				// knife
	EquipItem(MMCIP_PRIMARY,	4);				// rifle
	EquipItem(MMCIP_SECONDARY,	7);
	EquipItem(MMCIP_CUSTOM1,	11);			// grenade

//	SelectWeapon(MMCIP_PRIMARY);

	m_Items[MMCIP_PRIMARY].InitBullet(999, 999);
	m_Items[MMCIP_SECONDARY].InitBullet(999, 999);
	m_Items[MMCIP_CUSTOM1].InitBullet(10, 10);

#endif
}
ZCharacterItem::~ZCharacterItem()
{

}

bool ZCharacterItem::Confirm(MMatchCharItemParts parts, MMatchItemDesc* pDesc)
{
	if (pDesc == NULL) return false;


	switch (parts)
	{
	case MMCIP_HEAD:
	case MMCIP_CHEST:
	case MMCIP_HANDS:
	case MMCIP_LEGS:
	case MMCIP_FEET:
	case MMCIP_FINGERL:
	case MMCIP_FINGERR:
		{
			if (pDesc->m_nType.Ref() != MMIT_EQUIPMENT) 
			{
				return false;
			}
		}
		break;	
	case MMCIP_AVATAR:
		{
			if (pDesc->m_nType.Ref() != MMIT_AVATAR) 
			{
				return false;
			}
		}
		break;
	case MMCIP_MELEE:
		{
			if (pDesc->m_nType.Ref() != MMIT_MELEE) 
			{
				return false;
			}
		}
		break;
	case MMCIP_PRIMARY:
	case MMCIP_SECONDARY:
		{
			if (pDesc->m_nType.Ref() != MMIT_RANGE) 
			{
				return false;
			}
		}
		break;
	case MMCIP_CUSTOM1:
	case MMCIP_CUSTOM2:
		{
			if (pDesc->m_nType.Ref() != MMIT_CUSTOM) 
			{
				return false;
			}
		}
		break;
	case MMCIP_COMMUNITY1:
	case MMCIP_COMMUNITY2:
		{
			if (pDesc->m_nType.Ref() != MMIT_COMMUNITY) 
			{
				return false;
			}
		}
		break;
	case MMCIP_LONGBUFF1:
	case MMCIP_LONGBUFF2:
		{
			if (pDesc->m_nType.Ref() != MMIT_LONGBUFF) 
			{
				return false;
			}
		}
		break;
	}

	return true;
}

bool ZCharacterItem::EquipItem(MMatchCharItemParts parts, int nItemDescID, int nItemCount)
{
	if (nItemDescID == 0) {
		m_Items[parts].Create(MUID(0,0), NULL, 0);
		return true;
	}

	MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemDescID);

	if (pDesc == NULL) { return false; }
	if (!Confirm(parts, pDesc)) {
		return false;
	}

	m_Items[parts].Create(MUID(0,0), pDesc, 1, nItemCount);
	return true;
}

void ZCharacterItem::SelectWeapon(MMatchCharItemParts parts)
{
	//_ASSERT( (parts == MMCIP_MELEE)   || (parts == MMCIP_PRIMARY) || (parts == MMCIP_SECONDARY) 
			//|| (parts == MMCIP_CUSTOM1) || (parts == MMCIP_CUSTOM2) );

	if (!IsWeaponItem(parts)) return;

	if (m_nSelectedWeapon == parts) return;
	m_nSelectedWeapon = parts;
}

bool ZCharacterItem::Reload()
{
	if ((m_nSelectedWeapon != MMCIP_PRIMARY) && (m_nSelectedWeapon != MMCIP_SECONDARY)) return false;

	ZItem* pItem = &m_Items[m_nSelectedWeapon];

	if (pItem->GetDesc() == NULL) return false;
	return pItem->Reload();
}


ZItem* ZCharacterItem::GetSelectedWeapon()
{
	if(m_nSelectedWeapon < MMCIP_MELEE || m_nSelectedWeapon > MMCIP_CUSTOM2) return NULL;
	return &m_Items[(int)m_nSelectedWeapon]; 
}

bool ZCharacterItem::IsWeaponItem(MMatchCharItemParts parts)
{
	if ((parts == MMCIP_MELEE)		||
		(parts == MMCIP_PRIMARY)	||
		(parts == MMCIP_SECONDARY)	||
		(parts == MMCIP_CUSTOM1 )	||
		(parts == MMCIP_CUSTOM2 ) ) return true;

	return false;
}

bool ZCharacterItem::Save(ZFile *file)
{
	size_t n;
	for(int i=0;i<MMCIP_END;i++)
	{
		ZItem *pItem=GetItem(MMatchCharItemParts(i));
		int nBullet=pItem->GetBulletSpare();
		n=zfwrite(&nBullet,sizeof(nBullet),1,file);
		if(n!=1) return false;

		int nBulletCurrMagazine=pItem->GetBulletCurrMagazine();
		n=zfwrite(&nBulletCurrMagazine,sizeof(nBulletCurrMagazine),1,file);
		if(n!=1) return false;
	}
	return true;
}

bool ZCharacterItem::Load(ZFile *file, int nReplayVersion)
{
	enum MMatchCharItemParts_v0
	{
		MMCIP_HEAD_V0		= 0,
		MMCIP_CHEST_V0		= 1,
		MMCIP_HANDS_V0		= 2,
		MMCIP_LEGS_V0		= 3,
		MMCIP_FEET_V0		= 4,
		MMCIP_FINGERL_V0	= 5,
		MMCIP_FINGERR_V0	= 6,
		MMCIP_MELEE_V0		= 7,
		MMCIP_PRIMARY_V0	= 8,
		MMCIP_SECONDARY_V0	= 9,
		MMCIP_CUSTOM1_V0	= 10,
		MMCIP_CUSTOM2_V0	= 11,
		MMCIP_END_V0
	};

	struct Converter
	{
		static int v0_to_v6(int v0)
		{
			switch (v0)
			{
			case MMCIP_HEAD_V0:			return MMCIP_HEAD;
			case MMCIP_CHEST_V0:		return MMCIP_CHEST;
			case MMCIP_HANDS_V0:		return MMCIP_HANDS;
			case MMCIP_LEGS_V0:			return MMCIP_LEGS;
			case MMCIP_FEET_V0:			return MMCIP_FEET;
			case MMCIP_FINGERL_V0:		return MMCIP_FINGERL;
			case MMCIP_FINGERR_V0:		return MMCIP_FINGERR;
			case MMCIP_MELEE_V0:		return MMCIP_MELEE;
			case MMCIP_PRIMARY_V0:		return MMCIP_PRIMARY;
			case MMCIP_SECONDARY_V0:	return MMCIP_SECONDARY;
			case MMCIP_CUSTOM1_V0:		return MMCIP_CUSTOM1;
			case MMCIP_CUSTOM2_V0:		return MMCIP_CUSTOM2;
			}
			return MMCIP_END;
		}

		static int convert(int old, int nReplayVersion)
		{
			int curr = old;
			switch (nReplayVersion)
			{
			case 0:case 1:case 2:case 3:case 4:case 5:
				curr = v0_to_v6(curr);
			}
			return curr;
		}
	};

	int numCharItemParts = 0;
	if (0 <= nReplayVersion && nReplayVersion <= 5)
		numCharItemParts = MMCIP_END_V0;
	else
		numCharItemParts = MMCIP_END;

	size_t n;
	for(int i=0;i<numCharItemParts;i++)
	{
		int idxParts = Converter::convert(i, nReplayVersion);
		ZItem *pItem=GetItem(MMatchCharItemParts(idxParts));
		int nBullet;
		n=zfread(&nBullet,sizeof(nBullet),1,file);
		pItem->SetBulletSpare(nBullet);
		if(n!=1) return false;

		int nBulletCurrMagazine;
		n=zfread(&nBulletCurrMagazine,sizeof(nBulletCurrMagazine),1,file);
		pItem->SetBulletCurrMagazine(nBulletCurrMagazine);
		if(n!=1) return false;
	}

	return true;
}

void ZCharacterItem::ShiftFugitiveValues()
{
	for(int i=0;i<MMCIP_END;i++)
		m_Items[i].ShiftFugitiveValues();
}