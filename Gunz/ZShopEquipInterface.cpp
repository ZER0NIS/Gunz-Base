#include "stdafx.h"
#include "ZShopEquipInterface.h"
#include "ZShopEquipListbox.h"
#include "ZConfiguration.h"
#include "ZShop.h"
#include "ZItemSlotView.h"

static ZIDLResource* GetIDLResource() {
	return ZGetGameInterface()->GetIDLResource();
}

void WidgetHideDisableShow(const char* szWidget)
{
	MWidget* pWidget = GetIDLResource()->FindWidget(szWidget);
	if (pWidget)
	{
		pWidget->Show(false);
		pWidget->Enable(false);
		pWidget->Show(true);
	}
}
void WidgetHideDisable(const char* szWidget)
{
	MWidget* pWidget = GetIDLResource()->FindWidget(szWidget);
	if (pWidget)
	{
		pWidget->Show(false);
		pWidget->Enable(false);
	}
}

void WidgetEnableShow(const char* szWidget, bool bEnable, bool bShow)
{
	MWidget* pWidget = GetIDLResource()->FindWidget(szWidget);
	if (pWidget)
	{
		pWidget->Enable(bEnable);
		pWidget->Show(bShow);
	}
}

ZShopEquipInterface::ZShopEquipInterface()
{
	m_nShopTabNum = 0;
	m_nEquipTabNum = 0;

	m_idxArmorWeaponTab = 1;

	m_pItemDescriptionClient = 0;

	m_pItemCountDlg = new ZItemCountDlg;
	m_pSimpleConfirmDlg = new ZSimpleConfirmDlg;
	m_pSellCashItemConfirmDlg = new ZSellCashItemConfirmDlg;
};

ZShopEquipInterface::~ZShopEquipInterface()
{
	delete m_pItemCountDlg;
	delete m_pSimpleConfirmDlg;
	delete m_pSellCashItemConfirmDlg;
}

void ZShopEquipInterface::ConfirmBuySimple()
{
}

ZShopEquipListItem* ZShopEquipInterface::GetListCurSelItem(const char* szListWidget)
{
	ZShopEquipListbox* pListbox = (ZShopEquipListbox*)ZGetGameInterface()->GetIDLResource()->FindWidget(szListWidget);
	if (pListbox)
		return (ZShopEquipListItem*)pListbox->GetSelItem();
	return NULL;
}

void ZShopEquipInterface::OnSellButton(void)
{
	static unsigned long int st_LastRequestTime = 0;
	unsigned long int nNowTime = timeGetTime();
	if ((nNowTime - st_LastRequestTime) < 1000) return;

	st_LastRequestTime = nNowTime;

	ZShopEquipListItem* pListItem = (ZShopEquipListItem*)GetListCurSelItem("MyAllEquipmentList");
	if (!pListItem) {
		ZApplication::GetGameInterface()->ShowErrorMessage(MERR_NO_SELITEM);
		return;
	}

	if (pListItem->GetItemData()->GetHandleSell())
		pListItem->GetItemData()->GetHandleSell()->Sell();
}

void ZShopEquipInterface::OnBuyButton(void)
{
	static unsigned long int st_LastRequestTime = 0;
	unsigned long int nNowTime = timeGetTime();
	if ((nNowTime - st_LastRequestTime) < 1000) return;

	st_LastRequestTime = nNowTime;

	ZShopEquipListItem* pListItem = (ZShopEquipListItem*)GetListCurSelItem("AllEquipmentList");
	if (!pListItem) {
		ZApplication::GetGameInterface()->ShowErrorMessage(MERR_NO_SELITEM);
		return;
	}

	if (pListItem->GetItemData()->GetHandlePurchase())
		pListItem->GetItemData()->GetHandlePurchase()->Buy();
}

MMatchCharItemParts ZShopEquipInterface::RecommendEquipParts(MMatchItemSlotType slot)
{
	MMatchCharItemParts parts = MMCIP_END;

	if (slot == MMIST_RANGE)
	{
		if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_PRIMARY) == 0)
		{
			parts = MMCIP_PRIMARY;
		}
		else if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_SECONDARY) == 0)
		{
			parts = MMCIP_SECONDARY;
		}
	}
	else if (slot == MMIST_CUSTOM)
	{
		if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_CUSTOM1) == 0)
		{
			parts = MMCIP_CUSTOM1;
		}
		else if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_CUSTOM2) == 0)
		{
			parts = MMCIP_CUSTOM2;
		}
	}
	else if (slot == MMIST_FINGER)
	{
		if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_FINGERL) == 0)
		{
			parts = MMCIP_FINGERL;
		}
		else if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_FINGERR) == 0)
		{
			parts = MMCIP_FINGERR;
		}
	}
	else if (slot == MMIST_AVATAR)
	{
		if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_AVATAR) == 0)
		{
			parts = MMCIP_AVATAR;
		}
	}
	else if (slot == MMIST_COMMUNITY)
	{
		if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_COMMUNITY1) == 0)
		{
			parts = MMCIP_COMMUNITY1;
		}
		else if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_COMMUNITY2) == 0)
		{
			parts = MMCIP_COMMUNITY2;
		}
	}
	else if (slot == MMIST_LONGBUFF)
	{
		if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_LONGBUFF1) == 0)
		{
			parts = MMCIP_LONGBUFF1;
		}
		else if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_LONGBUFF2) == 0)
		{
			parts = MMCIP_LONGBUFF2;
		}
	}

	if (parts == MMCIP_END)
	{
		parts = GetSuitableItemParts(slot);
	}

	return parts;
}

bool ZShopEquipInterface::Equip(void)
{
	static unsigned long int st_LastRequestTime = 0;
	unsigned long int nNowTime = timeGetTime();
	if ((nNowTime - st_LastRequestTime) < 1000) return false;

	st_LastRequestTime = nNowTime;

	ZShopEquipListItem* pListItem = GetListCurSelItem("EquipmentList");
	if (!pListItem)
	{
		ZApplication::GetGameInterface()->ShowErrorMessage(MERR_NO_SELITEM);
		return false;
	}

	if (ZSEIT_GAMBLE == pListItem->GetItemData()->GetType())
	{
		ZShopEquipItem_Gamble* pWrapperGItem = (ZShopEquipItem_Gamble*)pListItem->GetItemData();
		const ZGambleItemDefine* pGItemDesc = pWrapperGItem->GetDesc();
		if (pGItemDesc)
		{
			if (ZGetGameClient()->GetServerMode() != MSM_ZERONIS && 1000100 <= pGItemDesc->GetGambleItemID() && pGItemDesc->GetGambleItemID() < 1000200)
			{
				ZApplication::GetGameInterface()->ShowErrorMessage(MERR_CANNOT_CAHNGE_THIS_ITEM);
			}
			else
			{
				if (!pWrapperGItem->GetHandleSell()) { return false; }
				ZPostRequestGamble(pWrapperGItem->GetHandleSell()->GetItemUID());
				ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());
			}
		}
	}

	if (ZSEIT_MATCH != pListItem->GetItemData()->GetType())
	{
		return false;
	}

	ZShopEquipItem_Match* pWrappedZItem = (ZShopEquipItem_Match*)pListItem->GetItemData();
	if (!pWrappedZItem->GetHandleSell()) { return false; }

	MUID uidItem = pWrappedZItem->GetHandleSell()->GetItemUID();
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(ZGetMyInfo()->GetItemList()->GetItemID(uidItem));
	if (!pItemDesc) return false;

	if (pItemDesc->m_nSlot == MMIST_NONE)
		return false;

	MMatchCharItemParts parts = RecommendEquipParts(pItemDesc->m_nSlot);
	if (parts == MMCIP_END)
		return false;

	WidgetHideDisableShow("Equip");
	WidgetHideDisableShow("SendAccountItemBtn");

	SetKindableItem(MMIST_NONE);

	return Equip(parts, uidItem);
}

bool ZShopEquipInterface::Equip(MMatchCharItemParts parts, MUID& uidItem)
{
	ZPostRequestEquipItem(ZGetGameClient()->GetPlayerUID(), uidItem, parts);
	return true;
}

void ZShopEquipInterface::SetKindableItem(MMatchItemSlotType nSlotType)
{
	ZItemSlotView* itemSlot;
	for (int i = 0; i < MMCIP_END; i++)
	{
		itemSlot = (ZItemSlotView*)GetIDLResource()->FindWidget(GetItemSlotName("Equip", i));
		if (itemSlot)
		{
			bool bKindable = IsKindableItem(itemSlot->GetParts(), nSlotType);
			itemSlot->SetKindable(bKindable);
		}
	}
}

bool ZShopEquipInterface::IsKindableItem(MMatchCharItemParts nParts, MMatchItemSlotType nSlotType)
{
	switch (nSlotType)
	{
	case MMIST_MELEE:
		if (nParts == MMCIP_MELEE)
			return true;
		break;
	case MMIST_RANGE:
		if ((nParts == MMCIP_PRIMARY) || (nParts == MMCIP_SECONDARY))
			return true;
		break;
	case MMIST_CUSTOM:
		if ((nParts == MMCIP_CUSTOM1) || (nParts == MMCIP_CUSTOM2))
			return true;
		break;
	case MMIST_HEAD:
		if (nParts == MMCIP_HEAD)
			return true;
		break;
	case MMIST_CHEST:
		if (nParts == MMCIP_CHEST)
			return true;
		break;
	case MMIST_HANDS:
		if (nParts == MMCIP_HANDS)
			return true;
		break;
	case MMIST_LEGS:
		if (nParts == MMCIP_LEGS)
			return true;
		break;
	case MMIST_FEET:
		if (nParts == MMCIP_FEET)
			return true;
		break;
	case MMIST_FINGER:
		if ((nParts == MMCIP_FINGERL) || (nParts == MMCIP_FINGERR))
			return true;
		break;
	case MMIST_AVATAR:
		if (nParts == MMCIP_AVATAR)
			return true;
		break;
	case MMIST_COMMUNITY:
		if ((nParts == MMCIP_COMMUNITY1) || (nParts == MMCIP_COMMUNITY2))
			return true;
		break;
	case MMIST_LONGBUFF:
		if ((nParts == MMCIP_LONGBUFF1) || (nParts == MMCIP_LONGBUFF2))
			return true;
		break;
	}
	return false;
}

int ZShopEquipInterface::_CheckRestrictBringAccountItem()
{
	ZShopEquipListItem* pListItem = GetListCurSelItem("AccountItemList");
	if (pListItem == NULL) return -1;

	if (pListItem->GetItemData()->GetHandleBringAccount())
	{
		int nAIID = pListItem->GetItemData()->GetHandleBringAccount()->GetAIID();
		ZMyItemNode* pMyItemNode = ZGetMyInfo()->GetItemList()->GetAccountItemByAIID(nAIID);

		if (!pMyItemNode) return 0;

		unsigned long nItemID = pMyItemNode->GetItemID();
		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
		if (!pItemDesc)
		{
			if (ZGetGambleItemDefineMgr().GetGambleItemDefine(nItemID)) {
				return 0;
			}
			else {
				return -1;
			}
		}

		if ((pItemDesc->m_nResSex.Ref() != -1) && (ZGetMyInfo()->GetSex() != pItemDesc->m_nResSex.Ref()))
			return 1;
		if (ZGetMyInfo()->GetLevel() < pItemDesc->m_nResLevel.Ref())
			return 2;
	}

	return 0;
}

bool ZShopEquipInterface::CheckRestrictBringAccountItem()
{
	int nResult = _CheckRestrictBringAccountItem();
	if (nResult == 0 || nResult == 2)
		return true;

	if (nResult == -1)
		ZGetGameInterface()->ShowErrorMessage(MERR_NO_SELITEM);

	else if (nResult == 1)
		ZGetGameInterface()->ShowErrorMessage(MERR_BRING_ACCOUNTITEM_BECAUSEOF_SEX);

	return false;
}

void ZShopEquipInterface::OnSendAccountButton()
{
	static unsigned long int st_LastRequestTime = 0;
	unsigned long int nNowTime = timeGetTime();
	if ((nNowTime - st_LastRequestTime) < 1000) return;

	st_LastRequestTime = nNowTime;

	ZShopEquipListItem* pListItem = GetListCurSelItem("EquipmentList");
	if (!pListItem)
	{
		ZApplication::GetGameInterface()->ShowErrorMessage(MERR_NO_SELITEM);
		return;
	}

	if (pListItem->GetItemData()->GetHandleSendAccount())
		pListItem->GetItemData()->GetHandleSendAccount()->Send();

	SetKindableItem(MMIST_NONE);

	WidgetHideDisableShow("SendAccountItemBtn");
	WidgetHideDisableShow("Equip");
}

void ZShopEquipInterface::OnBringAccountButton()
{
	static unsigned long int st_LastRequestTime = 0;
	unsigned long int nNowTime = timeGetTime();
	if ((nNowTime - st_LastRequestTime) < 1000) return;

	st_LastRequestTime = nNowTime;

	ZShopEquipListItem* pListItem = GetListCurSelItem("AccountItemList");
	if (!pListItem)
	{
		ZApplication::GetGameInterface()->ShowErrorMessage(MERR_NO_SELITEM);
		return;
	}

	if (pListItem->GetItemData()->GetHandleBringAccount())
		pListItem->GetItemData()->GetHandleBringAccount()->Bring();

	SetKindableItem(MMIST_NONE);

	WidgetHideDisableShow("BringAccountItemBtn");
}

void ZShopEquipInterface::SelectShopTab(int nTabIndex)
{
	if (nTabIndex == -1)
		nTabIndex = m_nShopTabNum;

	ZIDLResource* pResource = GetIDLResource();

#ifndef _DEBUG
#if defined(LOCALE_BRAZIL) || defined(LOCALE_INDIA) || defined(LOCALE_US) || defined(LOCALE_JAPAN) || defined(LOCALE_KOREA) || defined(LOCALE_NHNUSAA)
	{
		if (nTabIndex == 2)
			return;
	}
#endif
#endif

	MWidget* pWidget = pResource->FindWidget("AllEquipmentList");
	if (pWidget != NULL) pWidget->Show(nTabIndex == 0 ? true : false);
	pWidget = pResource->FindWidget("MyAllEquipmentList");
	if (pWidget != NULL) pWidget->Show(nTabIndex == 1 ? true : false);
	pWidget = pResource->FindWidget("CashEquipmentList");
	if (pWidget != NULL) pWidget->Show(nTabIndex == 2 ? true : false);

	MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("Shop_AllEquipmentFilter");
	if (pComboBox) {
		int sel = pComboBox->GetSelIndex();

		ZMyItemList* pil = ZGetMyInfo()->GetItemList();
		if (pil) {
			pil->m_ListFilter = sel;
			pil->Serialize();
		}
	}

	MButton* pButton = (MButton*)pResource->FindWidget("BuyConfirmCaller");
	if (pButton)
	{
		pButton->Show(false);
		pButton->Enable(false);
	}
	pButton = (MButton*)pResource->FindWidget("SellConfirmCaller");
	if (pButton)
	{
		pButton->Show(false);
		pButton->Enable(false);
	}

	if (nTabIndex == 0)
	{
		pButton = (MButton*)pResource->FindWidget("BuyConfirmCaller");
		if (pButton) pButton->Show(true);
	}
	else if (nTabIndex == 1)
	{
		pButton = (MButton*)pResource->FindWidget("SellConfirmCaller");
		if (pButton)
			pButton->Show(true);
	}
	pButton = (MButton*)pResource->FindWidget("AllEquipmentListCaller");
	if (pButton != NULL)
		pButton->Show(nTabIndex != 0 ? true : false);
	pButton = (MButton*)pResource->FindWidget("MyAllEquipmentListCaller");
	if (pButton != NULL)
		pButton->Show(nTabIndex != 1 ? true : false);
	pButton = (MButton*)pResource->FindWidget("CashEquipmentListCaller");
	if (pButton != NULL)
		pButton->Show(nTabIndex != 2 ? true : false);

	MPicture* pPicture;
	MBitmap* pBitmap;
	pPicture = (MPicture*)pResource->FindWidget("Shop_FrameTabLabel1");
	if (pPicture)
		pPicture->Show(nTabIndex == 0 ? true : false);
	pPicture = (MPicture*)pResource->FindWidget("Shop_FrameTabLabel2");
	if (pPicture)
		pPicture->Show(nTabIndex == 1 ? true : false);
	pPicture = (MPicture*)pResource->FindWidget("Shop_FrameTabLabel3");
	if (pPicture)
		pPicture->Show(nTabIndex == 2 ? true : false);

	pPicture = (MPicture*)pResource->FindWidget("Shop_TabLabel");
	if (pPicture)
	{
		if (nTabIndex == 0)
			pBitmap = MBitmapManager::Get("framepaneltab1.tga");
		else if (nTabIndex == 1)
			pBitmap = MBitmapManager::Get("framepaneltab2.tga");
		else if (nTabIndex == 2)
			pBitmap = MBitmapManager::Get("framepaneltab3.tga");

		if (pBitmap)
			pPicture->SetBitmap(pBitmap);
	}

#ifndef _DEBUG
#if defined(LOCALE_BRAZIL) || defined(LOCALE_INDIA) || defined(LOCALE_US) || defined(LOCALE_JAPAN) || defined(LOCALE_KOREA) || defined(LOCALE_NHNUSAA)
	{
		pWidget = pResource->FindWidget("Shop_TabLabelBg");
		if (pWidget)  pWidget->Show(false);

		pWidget = pResource->FindWidget("CashEquipmentListCaller");
		if (pWidget)  pWidget->Show(false);

		pWidget = pResource->FindWidget("Shop_FrameTabLabel3");
		if (pWidget)  pWidget->Show(false);
	}
#endif
#endif

	m_nShopTabNum = nTabIndex;

	DrawCharInfoText();
}

void ZShopEquipInterface::SelectEquipmentTab(int nTabIndex)
{
	if (nTabIndex == -1)
		nTabIndex = m_nEquipTabNum;

	ZIDLResource* pResource = GetIDLResource();

	SetKindableItem(MMIST_NONE);

	MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("Equip_AllEquipmentFilter");
	if (pComboBox) {
		int sel = pComboBox->GetSelIndex();

		ZMyItemList* pil = ZGetMyInfo()->GetItemList();
		if (pil) {
			pil->m_ListFilter = sel;
			pil->Serialize();
		}
	}

	MWidget* pWidget = pResource->FindWidget("EquipmentList");
	if (pWidget != NULL) pWidget->Show(nTabIndex == 0 ? true : false);
	pWidget = pResource->FindWidget("AccountItemList");
	if (pWidget != NULL) pWidget->Show(nTabIndex == 0 ? false : true);

	MButton* pButton = (MButton*)pResource->FindWidget("Equip");
	if (pButton)
	{
		pButton->Show(false);
		pButton->Enable(false);
	}

	pButton = (MButton*)pResource->FindWidget("SendAccountItemBtn");
	if (pButton)
	{
		pButton->Show(false);
		pButton->Enable(false);
	}

	pButton = (MButton*)pResource->FindWidget("BringAccountItemBtn");
	if (pButton)
	{
		pButton->Show(false);
		pButton->Enable(false);
	}

	pButton = (MButton*)pResource->FindWidget("BringAccountSpendableItemConfirmOpen");
	if (pButton)
	{
		pButton->Show(false);
		pButton->Enable(false);
	}

	if (nTabIndex == 0)
	{
		pButton = (MButton*)pResource->FindWidget("Equip");
		if (pButton) pButton->Show(true);

		pButton = (MButton*)pResource->FindWidget("SendAccountItemBtn");
		if (pButton) pButton->Show(true);
	}
	else if (nTabIndex == 1)
	{
		pButton = (MButton*)pResource->FindWidget("BringAccountItemBtn");
		if (pButton) pButton->Show(true);

		pButton = (MButton*)pResource->FindWidget("BringAccountSpendableItemConfirmOpen");
		if (pButton) pButton->Show(false);
	}

	pButton = (MButton*)pResource->FindWidget("Equipment_CharacterTab");
	if (pButton)
		pButton->Show(nTabIndex == 0 ? false : true);
	pButton = (MButton*)pResource->FindWidget("Equipment_AccountTab");
	if (pButton)
		pButton->Show(nTabIndex == 1 ? false : true);

	MLabel* pLabel;
	pLabel = (MLabel*)pResource->FindWidget("Equipment_FrameTabLabel1");
	if (pLabel)
		pLabel->Show(nTabIndex == 0 ? true : false);
	pLabel = (MLabel*)pResource->FindWidget("Equipment_FrameTabLabel2");
	if (pLabel)
		pLabel->Show(nTabIndex == 1 ? true : false);

	MPicture* pPicture;
	pPicture = (MPicture*)pResource->FindWidget("Equip_ListLabel1");
	if (pPicture)
		pPicture->Show(nTabIndex == 0 ? true : false);
	pPicture = (MPicture*)pResource->FindWidget("Equip_ListLabel2");
	if (pPicture)
		pPicture->Show(nTabIndex == 1 ? true : false);

	pPicture = (MPicture*)pResource->FindWidget("Equip_TabLabel");
	MBitmap* pBitmap;
	if (pPicture)
	{
		if (nTabIndex == 0)
			pBitmap = MBitmapManager::Get("framepaneltab1.tga");
		else
			pBitmap = MBitmapManager::Get("framepaneltab2.tga");

		if (pBitmap)
			pPicture->SetBitmap(pBitmap);
	}

	if (nTabIndex == 1)
	{
		ZGetMyInfo()->GetItemList()->ClearAccountItems();
		ZGetMyInfo()->GetItemList()->SerializeAccountItem();
	}

	for (int i = 0; i < MMCIP_END; i++)
	{
		ZItemSlotView* pItemSlot = (ZItemSlotView*)GetIDLResource()->FindWidget(GetItemSlotName("Equip", i));
		if (pItemSlot) pItemSlot->EnableDragAndDrop(nTabIndex == 0 ? true : false);
	}

	m_nEquipTabNum = nTabIndex;

	DrawCharInfoText();
}

void ZShopEquipInterface::SelectEquipmentFrameList(const char* szName, bool bOpen)
{
	if (szName == NULL)
	{
		SelectEquipmentFrameList("Shop", bOpen);
		SelectEquipmentFrameList("Equip", bOpen);
		return;
	}

	char szTemp[256];

	ZIDLResource* pResource = GetIDLResource();

	MPicture* pPicture;
	strcpy(szTemp, szName);
	strcat(szTemp, "_ArmorBGListFrameOpen");
	pPicture = (MPicture*)pResource->FindWidget(szTemp);
	if (pPicture != NULL) {
		if (bOpen && GetArmorWeaponTabIndex() == 0) { pPicture->Show(true); }
		else { pPicture->Show(false); }
	}

	strcpy(szTemp, szName);
	strcat(szTemp, "_ArmorBGListFrameClose");
	pPicture = (MPicture*)pResource->FindWidget(szTemp);
	if (pPicture != NULL) {
		if (!bOpen && GetArmorWeaponTabIndex() == 0) { pPicture->Show(true); }
		else { pPicture->Show(false); }
	}

	strcpy(szTemp, szName);
	strcat(szTemp, "_WeaponBGListFrameOpen");
	pPicture = (MPicture*)pResource->FindWidget(szTemp);
	if (pPicture != NULL) {
		if (bOpen && GetArmorWeaponTabIndex() == 1) { pPicture->Show(true); }
		else { pPicture->Show(false); }
	}

	strcpy(szTemp, szName);
	strcat(szTemp, "_WeaponBGListFrameClose");
	pPicture = (MPicture*)pResource->FindWidget(szTemp);
	if (pPicture != NULL) {
		if (!bOpen && GetArmorWeaponTabIndex() == 1) { pPicture->Show(true); }
		else { pPicture->Show(false); }
	}

	MButton* pButton;
	strcpy(szTemp, szName);
	strcat(szTemp, "_EquipListFrameCloseButton");
	pButton = (MButton*)pResource->FindWidget(szTemp);
	if (pButton != NULL) pButton->Show(bOpen);

	strcpy(szTemp, szName);
	strcat(szTemp, "_EquipListFrameOpenButton");
	pButton = (MButton*)pResource->FindWidget(szTemp);
	if (pButton != NULL) pButton->Show(!bOpen);

	char szWidgetName[256];
	sprintf(szWidgetName, "%s_EquipmentSlot_Head", szName);
	MWidget* itemSlot = (MWidget*)pResource->FindWidget(szWidgetName);
	if (itemSlot) {
		MRECT rect = itemSlot->GetRect();

		int nWidth;
		if (bOpen) nWidth = 220.0f * (float)RGetScreenWidth() / 800.0f;
		else		nWidth = min(rect.w, rect.h);

		for (int i = 0; i < MMCIP_END; i++) {
			itemSlot = (MWidget*)pResource->FindWidget(GetItemSlotName(szName, i));

			if (itemSlot) {
				if (GetArmorWeaponTabIndex() == GetArmorWeaponTabIndexContainItemParts((MMatchCharItemParts)i)) {
					rect = itemSlot->GetRect();

					itemSlot->SetBounds(rect.x, rect.y, nWidth, rect.h);
					itemSlot->Show(true);
				}
				else {
					itemSlot->Show(false);
				}
			}
		}
	}

	MBmButton* pTabBtn;
	if (GetArmorWeaponTabIndex() == 0)
	{
		pTabBtn = (MBmButton*)pResource->FindWidget("Shop_ArmorEquipmentTab");
		if (pTabBtn) pTabBtn->SetCheck(true);
		pTabBtn = (MBmButton*)pResource->FindWidget("Equip_ArmorEquipmentTab");
		if (pTabBtn) pTabBtn->SetCheck(true);
	}
	else if (GetArmorWeaponTabIndex() == 1)
	{
		pTabBtn = (MBmButton*)pResource->FindWidget("Shop_WeaponEquipmentTab");
		if (pTabBtn) pTabBtn->SetCheck(true);
		pTabBtn = (MBmButton*)pResource->FindWidget("Equip_WeaponEquipmentTab");
		if (pTabBtn) pTabBtn->SetCheck(true);
	}
}

int ZShopEquipInterface::GetArmorWeaponTabIndexContainItemParts(MMatchCharItemParts parts)
{
	if (parts < MMCIP_MELEE || parts == MMCIP_AVATAR)
		return 0;
	if (parts >= MMCIP_MELEE && parts < MMCIP_LONGBUFF2 + 1 && parts != MMCIP_AVATAR)
		return 1;

	return 0;
}

bool ZShopEquipInterface::IsEquipmentFrameListOpened(const char* szName)
{
	ZIDLResource* pResource = GetIDLResource();

	char szTemp[256];
	MPicture* pPicture;
	strcpy(szTemp, szName);
	if (GetArmorWeaponTabIndex() == 0)
	{
		strcat(szTemp, "_ArmorBGListFrameOpen");
		pPicture = (MPicture*)pResource->FindWidget(szTemp);
		if (pPicture)
			return pPicture->IsVisible();
	}
	else if (GetArmorWeaponTabIndex() == 1)
	{
		strcat(szTemp, "_WeaponBGListFrameOpen");
		pPicture = (MPicture*)pResource->FindWidget(szTemp);
		if (pPicture)
			return pPicture->IsVisible();
	}
	return false;
}

void ZShopEquipInterface::SelectArmorWeaponTabWithSlotType(MMatchItemSlotType nSlotType)
{
	ZItemSlotView* pItemSlot;
	for (int i = 0; i < MMCIP_END; i++)
	{
		pItemSlot = (ZItemSlotView*)GetIDLResource()->FindWidget(GetItemSlotName("Equip", i));
		if (pItemSlot)
		{
			if (IsKindableItem(pItemSlot->GetParts(), nSlotType))
			{
				int tabIndex = GetArmorWeaponTabIndexContainItemParts(pItemSlot->GetParts());

				bool bOpened = ZGetGameInterface()->GetShopEquipInterface()->IsEquipmentFrameListOpened();
				ZGetGameInterface()->GetShopEquipInterface()->SetArmorWeaponTabIndex(tabIndex);
				ZGetGameInterface()->GetShopEquipInterface()->SelectEquipmentFrameList(NULL, bOpened);

				return;
			}
		}
	}
}

void ZShopEquipInterface::OnArmorWeaponTabButtonClicked(int nTab)
{
	bool bOpened = IsEquipmentFrameListOpened();
	SetArmorWeaponTabIndex(nTab);
	SelectEquipmentFrameList(NULL, bOpened);
}

void ZShopEquipInterface::ShowItemDescription(bool bShow, MTextArea* pTextArea, void* pCaller)
{
	if (!pTextArea) return;

	if (bShow)
	{
		pTextArea->Show(true);
		m_pItemDescriptionClient = pCaller;
	}
	else if (!bShow && pCaller == m_pItemDescriptionClient)
	{
		pTextArea->Show(false);
	}
}

void ZShopEquipInterface::DrawCharInfoText()
{
	DrawCharInfoText(NULL, 0,
		ZGetMyInfo()->GetItemList()->GetEquipedTotalWeight(),
		ZGetMyInfo()->GetItemList()->GetMaxWeight(),
		ZGetMyInfo()->GetHP(),
		ZGetMyInfo()->GetAP(),
		0, 0);
}

void ZShopEquipInterface::DrawCharInfoText(char* szShopOrEquip, int nReqLevel, int nNewWT, int nNewMaxWT, int nNewHP, int nNewAP, int nReqBounty, int nReqCash)
{
	if (szShopOrEquip == NULL)
	{
		DrawCharInfoText("Shop", nReqLevel, nNewWT, nNewMaxWT, nNewHP, nNewAP, nReqBounty, nReqCash);
		DrawCharInfoText("Equip", nReqLevel, nNewWT, nNewMaxWT, nNewHP, nNewAP, nReqBounty, nReqCash);
		return;
	}

	if (0 != strcmp(szShopOrEquip, "Shop") && 0 != strcmp(szShopOrEquip, "Equip")) return;

	MTextArea* pTextArea[3];
	char sz1[256], sz2[256], szTextArea[64];
	const char* szRed = "^1";
	const char* szGreen = "^2";
	const char* szGray = "^9";
	const char* szColor = NULL;

	for (int i = 0; i < 3; ++i)
	{
		sprintf(szTextArea, "%s_MyInfo%d", szShopOrEquip, i + 1);
		pTextArea[i] = (MTextArea*)GetIDLResource()->FindWidget(szTextArea);
		if (NULL == pTextArea[i])
		{
			return;
		}

		pTextArea[i]->Clear();
	}

	szColor = szGray;
	if (nReqLevel > ZGetMyInfo()->GetLevel())
		szColor = szRed;
	sprintf(sz1, "^9%s : %s%d ^9%s", ZMsg(MSG_CHARINFO_LEVEL), szColor, ZGetMyInfo()->GetLevel(), ZMsg(MSG_CHARINFO_LEVELMARKER));
	pTextArea[0]->AddText(sz1);

	int nCurrMaxWT = ZGetMyInfo()->GetItemList()->GetMaxWeight();
	sprintf(sz1, "^9%s : ", ZMsg(MSG_CHARINFO_WEIGHT));
	szColor = (nNewWT > nNewMaxWT) ? szRed : szGray;
	sprintf(sz2, "%s%d", szColor, nNewWT);
	strcat(sz1, sz2);
	sprintf(sz2, "^9/%d", nCurrMaxWT);
	strcat(sz1, sz2);
	int nDiffMaxWT = nNewMaxWT - nCurrMaxWT;
	if (nDiffMaxWT != 0)
	{
		szColor = (nDiffMaxWT > 0) ? szGreen : szRed;
		sprintf(sz2, "%s%+d", szColor, nDiffMaxWT);
		strcat(sz1, sz2);
	}
	pTextArea[0]->AddText(sz1);

	sprintf(sz1, "^9%s : %d ", ZMsg(MSG_CHARINFO_HP), ZGetMyInfo()->GetHP());
	int nDiffHP = nNewHP - ZGetMyInfo()->GetHP();
	if (nDiffHP != 0)
	{
		szColor = (nDiffHP > 0) ? szGreen : szRed;
		sprintf(sz2, "%s%+d", szColor, nDiffHP);
		strcat(sz1, sz2);
	}
	pTextArea[1]->AddText(sz1);

	sprintf(sz1, "^9%s : %d ", ZMsg(MSG_CHARINFO_AP), ZGetMyInfo()->GetAP());
	int nDiffAP = nNewAP - ZGetMyInfo()->GetAP();
	if (nDiffAP != 0)
	{
		szColor = (nDiffAP > 0) ? szGreen : szRed;
		sprintf(sz2, "%s%+d", szColor, nDiffAP);
		strcat(sz1, sz2);
	}
	pTextArea[1]->AddText(sz1);

	sprintf(sz1, "^9%s : ", ZMsg(MSG_CHARINFO_BOUNTY));
	szColor = (nReqBounty > ZGetMyInfo()->GetBP()) ? szRed : szGray;
	sprintf(sz2, "%s%d", szColor, ZGetMyInfo()->GetBP());
	strcat(sz1, sz2);
	pTextArea[2]->AddText(sz1);
}

void ZShopEquipInterface::Update()
{
	if (m_pSellCashItemConfirmDlg)
		m_pSellCashItemConfirmDlg->Update();
}