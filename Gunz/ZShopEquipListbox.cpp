#include "stdafx.h"
#include "ZShopEquipListbox.h"
#include "ZShopEquipInterface.h"
#include "ZItemSlotView.h"
#include "ZShop.h"

ZShopEquipListItem::ZShopEquipListItem(ZShopEquipItem* pItemData)
	: m_pItemData(pItemData)
{
	m_pItemData->GetName(m_szName);
	m_pItemData->GetLevelResText(m_szLevel);
	m_pItemData->GetPriceText(m_szPrice);
}

ZShopEquipListItem::~ZShopEquipListItem()
{
	delete m_pItemData;
}

void ZShopEquipListItem::OnDraw(MRECT& r, MDrawContext* pDC, bool bSelected, bool bMouseOver)
{
	if (bSelected)
		pDC->SetColor(220, 220, 220);
	else if (bMouseOver)
		pDC->SetColor(60, 60, 60);
	else
		pDC->SetColor(30, 30, 30);

	pDC->FillRectangle(r);

	MRECT rcIcon;
	GetIconRect(rcIcon, r);
	rcIcon.x += r.x;
	rcIcon.y += r.y;

	pDC->SetColor(15, 15, 15);
	pDC->FillRectangle(rcIcon);

	pDC->SetBitmap(m_pItemData->GetIcon());
	pDC->Draw(rcIcon);

	MRECT rc;
	rc.x = rcIcon.x + rcIcon.w + 2;
	rc.w = r.w - rcIcon.w - 6;
	rc.y = r.y + 2;
	rc.h = r.h - 4;

	if (!bSelected)
		pDC->SetColor(200, 200, 200);
	else
		pDC->SetColor(20, 20, 20);

	pDC->TextMultiLine2(rc, GetString(), CONVERT600(2), true, MAM_LEFT | MAM_TOP);
	pDC->Text(rc, m_szPrice, MAM_RIGHT | MAM_BOTTOM);

	pDC->SetColor(20, 20, 20);
	pDC->Text(MRECT(rcIcon.x + 1, rcIcon.y, rcIcon.w, rcIcon.h), m_szLevel, MAM_LEFT | MAM_BOTTOM);
	pDC->Text(MRECT(rcIcon.x - 1, rcIcon.y, rcIcon.w, rcIcon.h), m_szLevel, MAM_LEFT | MAM_BOTTOM);
	pDC->Text(MRECT(rcIcon.x, rcIcon.y, rcIcon.w, rcIcon.h + 1), m_szLevel, MAM_LEFT | MAM_BOTTOM);
	pDC->Text(MRECT(rcIcon.x, rcIcon.y, rcIcon.w, rcIcon.h - 1), m_szLevel, MAM_LEFT | MAM_BOTTOM);

	if (ZGetMyInfo()->GetLevel() < m_pItemData->GetLevelRes())
		pDC->SetColor(200, 10, 10);
	else
		pDC->SetColor(200, 200, 200);

	pDC->Text(rcIcon, m_szLevel, MAM_LEFT | MAM_BOTTOM);
}

bool ZShopEquipListItem::GetDragItem(MBitmap** ppDragBitmap, char* szDragString, char* szDragItemString)
{
	if (!m_pItemData) return true;

	*ppDragBitmap = m_pItemData->GetIcon();
	strcpy(szDragString, m_szName);
	strcpy(szDragItemString, m_szName);
	return true;
}

int ZShopEquipListItem::GetSortHint()
{
	if (ZSEIT_GAMBLE == m_pItemData->GetType())
		return 1000000;

	if (ZSEIT_MATCH == m_pItemData->GetType())
	{
		int hint = 2000000;
		int nResLv = m_pItemData->GetLevelRes();
		MMatchItemDesc* pItemDesc = ((ZShopEquipItem_Match*)m_pItemData)->GetDesc();
		if (!pItemDesc) { return 0; }

		if (pItemDesc->m_nType.Ref() == MMIT_MELEE)
			return hint + 100000 + pItemDesc->m_nWeaponType.Ref() * 1000 + nResLv;
		if (pItemDesc->m_nType.Ref() == MMIT_RANGE)
			return hint + 200000 + pItemDesc->m_nWeaponType.Ref() * 1000 + nResLv;
		if (pItemDesc->m_nType.Ref() == MMIT_EQUIPMENT)
			return hint + 300000 + pItemDesc->m_nSlot * 1000 + nResLv;

		hint = 4000000;
		if (pItemDesc->m_nType.Ref() == MMIT_CUSTOM)
		{
			hint = hint + 400000 + pItemDesc->m_nWeaponType.Ref() * 1000 + nResLv;
			if (pItemDesc->m_nWeaponType.Ref() == MWT_POTION)
			{
				hint = hint + pItemDesc->m_nEffectId * 100;
			}
			return hint;
		}
		if (pItemDesc->m_nType.Ref() == MMIT_COMMUNITY)
			return hint + 500000 + pItemDesc->m_nWeaponType.Ref() * 1000 + nResLv;
		if (pItemDesc->m_nType.Ref() == MMIT_LONGBUFF)
			return hint + 600000 + pItemDesc->m_nWeaponType.Ref() * 1000 + nResLv;

		return hint + 700000 + nResLv;
	}

	if (ZSEIT_SET == m_pItemData->GetType())
	{
		int nResLv = ((ZShopEquipItem_Set*)m_pItemData)->GetLevelRes();
		return 3000000 + nResLv;
	}

	if (ZSEIT_QUEST == m_pItemData->GetType())
		return 5000000;

	return 9999999;
}

void ZShopEquipListItem::GetIconRect(MRECT& out, const MRECT& rcItem)
{
	int len = rcItem.h - 4;
	out.Set(2, 2, len, len);
}

bool ZShopEquipListItem::IsPtInRectToShowToolTip(MRECT& rcItem, MPOINT& pt)
{
	MRECT rcIcon;
	GetIconRect(rcIcon, rcItem);
	rcIcon.x += rcItem.x;
	rcIcon.y += rcItem.y;
	return (rcIcon.x < pt.x && pt.x < rcIcon.x + rcIcon.w &&
		rcIcon.y < pt.y && pt.y < rcIcon.y + rcIcon.h);
}

ZShopEquipListbox::ZShopEquipListbox(const char* szName, MWidget* pParent, MListener* pListener)
	: MMultiColListBox(szName, pParent, pListener)
{
	m_idxItemLastTooltip = -1;
}

bool ZShopEquipListbox::OnEvent(MEvent* pEvent, MListener* pListener)
{
	MRECT rtClient = GetClientRect();

	if (pEvent->nMessage == MWM_MOUSEMOVE)
	{
		// Custom: Disable tooltip activation
		SetupItemDescTooltip();
		return true;
	}

	return MMultiColListBox::OnEvent(pEvent, pListener);
}

char* ZShopEquipListbox::GetItemDescTooltipName()
{
	if (GetParent() && 0 == strcmp("Shop", GetParent()->m_szIDLName))
		return "Shop_ItemDescription";

	if (GetParent() && 0 == strcmp("Equipment", GetParent()->m_szIDLName))
		return "Equip_ItemDescription";

	return "_noExistWidgetName_";
}

void ZShopEquipListbox::SetupItemDescTooltip()
{
	const char* szTextAreaName = GetItemDescTooltipName();
	MTextArea* pItemDescTextArea = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(szTextAreaName);
	if (pItemDescTextArea)
	{
		MPOINT ptInList = MScreenToClient(this, MEvent::LatestPos);
		int idxItem = FindItem(ptInList);
		if (idxItem != -1)
		{
			ZShopEquipListItem* pItem = (ZShopEquipListItem*)Get(idxItem);
			MRECT rcListBox = GetRect();
			MRECT rcItem;
			if (pItem && CalcItemRect(idxItem, rcItem))
			{
				if (pItem->IsPtInRectToShowToolTip(rcItem, ptInList))
				{
					if (m_idxItemLastTooltip == idxItem) return;
					m_idxItemLastTooltip = idxItem;

					pItem->GetItemData()->FillItemDesc(pItemDescTextArea);

					MRECT rcTextArea = pItemDescTextArea->GetRect();
					MPOINT posDesc(rcItem.x, rcItem.y);
					posDesc = MClientToScreen(this, posDesc);
					posDesc.x -= pItemDescTextArea->GetClientWidth();
					if (posDesc.y + rcTextArea.h > rcListBox.y + rcListBox.h)
						posDesc.y = rcListBox.y + rcListBox.h - rcTextArea.h;
					if (posDesc.y < 0)
						posDesc.y = 0;
					pItemDescTextArea->SetPosition(posDesc);
					pItemDescTextArea->SetZOrder(MZ_TOP);
					ZGetGameInterface()->GetShopEquipInterface()->ShowItemDescription(true, pItemDescTextArea, this);
					return;
				}
			}
		}
	}

	m_idxItemLastTooltip = -1;
	ZGetGameInterface()->GetShopEquipInterface()->ShowItemDescription(false, pItemDescTextArea, this);
}

void ShopPurchaseItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
}

void ShopSaleItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
}

void CharacterEquipmentItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	if (pSender == NULL) return;
	if (strcmp(pSender->GetClassName(), MINT_ITEMSLOTVIEW)) return;

	ZItemSlotView* pItemSlotView = (ZItemSlotView*)pSender;

	ZPostRequestTakeoffItem(ZGetGameClient()->GetPlayerUID(), pItemSlotView->GetParts());
}

class ZShopPurchaseItemListBoxListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if (MWidget::IsMsg(szMessage, MLB_ITEM_SEL) == true)
		{
			ZShopEquipListbox* pEquipmentListBox = (ZShopEquipListbox*)pWidget;
			ZShopEquipListItem* pListItem = (ZShopEquipListItem*)pEquipmentListBox->GetSelItem();
			if (pListItem == NULL) return true;

			ZCharacterView* pCharacterView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("EquipmentInformationShop");
			if (pCharacterView)
				pListItem->GetItemData()->UpdateCharacterView(pCharacterView);

			pListItem->GetItemData()->UpdateCharInfoText();

			ZGetGameInterface()->GetShopEquipInterface()->SelectArmorWeaponTabWithSlotType(pListItem->GetItemData()->GetSlotType());

			WidgetEnableShow("BuyConfirmCaller", true, true);

			return true;
		}
		else if (MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK) == true)
		{
			ZGetGameInterface()->GetShopEquipInterface()->OnBuyButton();
			return true;
		}

		return false;
	}
};
ZShopPurchaseItemListBoxListener g_ShopPurchaseItemListBoxListener;

MListener* ZGetShopPurchaseItemListBoxListener(void)
{
	return &g_ShopPurchaseItemListBoxListener;
}

class ZEquipMyItemListBoxListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if (MWidget::IsMsg(szMessage, MLB_ITEM_SEL) == true) {
			ZShopEquipListbox* pEquipmentListBox = (ZShopEquipListbox*)pWidget;
			ZShopEquipListItem* pListItem = (ZShopEquipListItem*)pEquipmentListBox->GetSelItem();
			if (!pListItem) return false;

			ZCharacterView* pCharacterView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("EquipmentInformation");
			if (pCharacterView)
				pListItem->GetItemData()->UpdateCharacterView(pCharacterView);

			pListItem->GetItemData()->UpdateCharInfoText();

			ZGetGameInterface()->GetShopEquipInterface()->SetKindableItem(pListItem->GetItemData()->GetSlotType());
			ZGetGameInterface()->GetShopEquipInterface()->SelectArmorWeaponTabWithSlotType(pListItem->GetItemData()->GetSlotType());

			WidgetEnableShow("Equip", pListItem->GetItemData()->CanEquip(), true);
			WidgetEnableShow("SendAccountItemBtn", pListItem->GetItemData()->CanSendAccount(), true);

			return true;
		}
		else if (MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK) == true)
		{
			ZGetGameInterface()->GetShopEquipInterface()->Equip();
			return true;
		}

		return false;
	}
};
ZEquipMyItemListBoxListener g_EquipmentItemListBoxListener;

MListener* ZGetEquipmentMyItemListBoxListener(void)
{
	return &g_EquipmentItemListBoxListener;
}

class ZShopSellItemListBoxListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if (MWidget::IsMsg(szMessage, MLB_ITEM_SEL) == true)
		{
			ZShopEquipListbox* pListbox = (ZShopEquipListbox*)pWidget;
			ZShopEquipListItem* pListItem = (ZShopEquipListItem*)pListbox->GetSelItem();

			if (!pListItem) return false;

			ZGetGameInterface()->GetShopEquipInterface()->DrawCharInfoText();
			ZGetGameInterface()->GetShopEquipInterface()->SetKindableItem(pListItem->GetItemData()->GetSlotType());

			WidgetEnableShow("SellConfirmCaller", pListItem->GetItemData()->CanSell(), true);
			return true;
		}
		else if (MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK) == true)
		{
			ZGetGameInterface()->GetShopEquipInterface()->OnSellButton();
			return true;
		}

		return false;
	}
};

ZShopSellItemListBoxListener g_ShopSellItemListBoxListener;

MListener* ZGetShopSellItemListBoxListener(void)
{
	return &g_ShopSellItemListBoxListener;
}

class ZAccountItemListBoxListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if (MWidget::IsMsg(szMessage, MLB_ITEM_SEL) == true) {
			ZShopEquipListbox* pListbox = (ZShopEquipListbox*)pWidget;
			ZShopEquipListItem* pListItem = (ZShopEquipListItem*)pListbox->GetSelItem();
			if (!pListItem) return false;

			unsigned long int nItemID = ZGetMyInfo()->GetItemList()->GetAccountItemID(pListbox->GetSelIndex());

			ZCharacterView* pCharacterView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("EquipmentInformation");
			if (pCharacterView)
				pListItem->GetItemData()->UpdateCharacterView(pCharacterView);

			pListItem->GetItemData()->UpdateCharInfoText();
			ZGetGameInterface()->GetShopEquipInterface()->SelectArmorWeaponTabWithSlotType(pListItem->GetItemData()->GetSlotType());

			WidgetEnableShow("BringAccountItemBtn", true, true);

			return true;
		}
		else if (MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK) == true)
		{
			ZGetGameInterface()->GetShopEquipInterface()->OnBringAccountButton();
			return true;
		}

		return false;
	}
};

ZAccountItemListBoxListener g_AccountItemListBoxListener;

MListener* ZGetAccountItemListBoxListener(void)
{
	return &g_AccountItemListBoxListener;
}

class ZShopListFilterListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if (MWidget::IsMsg(szMessage, MCMBBOX_CHANGED) == true)
		{
			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

			MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("Shop_AllEquipmentFilter");
			if (pComboBox)
			{
				int sel = pComboBox->GetSelIndex();

				ZGetShop()->m_ListFilter = sel;
				ZGetShop()->Serialize();

				ZMyItemList* pil = ZGetMyInfo()->GetItemList();
				pil->m_ListFilter = sel;
				pil->Serialize();
			}

			ZGetGameInterface()->GetShopEquipInterface()->SelectShopTab(-1);
		}
		return true;
	}
};

ZShopListFilterListener g_ShopListFilterListener;

MListener* ZGetShopListFilterListener()
{
	return &g_ShopListFilterListener;
}

class MEquipListFilterListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if (MWidget::IsMsg(szMessage, MCMBBOX_CHANGED) == true) {
			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

			MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("Equip_AllEquipmentFilter");
			if (pComboBox)
			{
				int sel = pComboBox->GetSelIndex();

				ZMyItemList* pil = ZGetMyInfo()->GetItemList();
				pil->m_ListFilter = sel;
				pil->Serialize();
			}

			ZGetGameInterface()->GetShopEquipInterface()->SelectEquipmentTab(-1);
		}
		return true;
	}
};

MEquipListFilterListener g_EquipListFilterListener;

MListener* ZGetEquipListFilterListener()
{
	return &g_EquipListFilterListener;
}