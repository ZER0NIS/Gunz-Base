#pragma once
#include "MMultiColListBox.h"
#include "ZShopEquipItem.h"

class ZShopEquipListItem : public MMultiColListItem
{
	ZShopEquipItem* m_pItemData;
	char m_szName[256];
	char m_szLevel[256];
	char m_szPrice[256];

public:
	ZShopEquipListItem(ZShopEquipItem* pItemData);
	virtual ~ZShopEquipListItem();

	ZShopEquipItem* GetItemData() { return m_pItemData; }

	virtual void OnDraw(MRECT& r, MDrawContext* pDC, bool bSelected, bool bMouseOver);
	virtual const char* GetString() { return m_szName; }

	virtual bool GetDragItem(MBitmap** ppDragBitmap, char* szDragString, char* szDragItemString);
	virtual int GetSortHint();
	void GetIconRect(MRECT& out, const MRECT& rcItem);
	bool IsPtInRectToShowToolTip(MRECT& rcItem, MPOINT& pt);
};


class ZShopEquipListbox : public MMultiColListBox
{
	int m_idxItemLastTooltip;
public:
	ZShopEquipListbox(const char* szName, MWidget* pParent=NULL, MListener* pListener=NULL);

	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);

	void SetupItemDescTooltip();
	char* GetItemDescTooltipName();

public:
#define MINT_EQUIPMENTLISTBOX	"EquipmentListBox"
	virtual const char* GetClassName(void){ return MINT_EQUIPMENTLISTBOX; }

};


void ShopPurchaseItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);
void ShopSaleItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);
void CharacterEquipmentItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);


MListener* ZGetShopListFilterListener(void);
MListener* ZGetEquipListFilterListener(void);

MListener* ZGetShopSellItemListBoxListener(void);
//MListener* ZGetCashShopItemListBoxListener(void);
MListener* ZGetShopPurchaseItemListBoxListener(void);
MListener* ZGetEquipmentMyItemListBoxListener(void);
MListener* ZGetAccountItemListBoxListener(void);