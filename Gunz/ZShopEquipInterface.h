#pragma once
#include "ZShopEquipItemConfirm.h"


class ZShopEquipListItem;

void WidgetEnableShow(const char* szWidget, bool bEnable, bool bShow);


// 원래 ZGameInterface에 있던 상점과 장비창에 관련한 코드를 여기로 옮깁니다

class ZShopEquipInterface
{
	int					m_nShopTabNum;
	int					m_nEquipTabNum;

	int					m_nSellQuestItemCount;
	int					m_nSellSpendableItemCount;
	int					m_nBuySpendableItemCount;

	ZItemCountDlg*				m_pItemCountDlg;
	ZSimpleConfirmDlg*			m_pSimpleConfirmDlg;
	ZSellCashItemConfirmDlg*	m_pSellCashItemConfirmDlg;

public:
	ZShopEquipInterface();
	~ZShopEquipInterface();

	void ConfirmBuySimple();

	ZShopEquipListItem* GetListCurSelItem(const char* szListWidget);

	ZItemCountDlg* GetItemCountDlg() { return m_pItemCountDlg; }
	ZSimpleConfirmDlg* GetSimpleConfirmDlg() { return m_pSimpleConfirmDlg; }
	ZSellCashItemConfirmDlg* GetSellCashItemConfirmDlg() { return m_pSellCashItemConfirmDlg; }

	void OnSellButton(void);

	void OnBuyButton(void);

	MMatchCharItemParts RecommendEquipParts(MMatchItemSlotType slot);

	bool Equip(void);
	bool Equip(MMatchCharItemParts parts, MUID& uidItem);

	void SetKindableItem( MMatchItemSlotType nSlotType);
	bool IsKindableItem(MMatchCharItemParts nParts, MMatchItemSlotType nSlotType);

	int _CheckRestrictBringAccountItem();
	bool CheckRestrictBringAccountItem();

	void OnSendAccountButton();
	void OnBringAccountButton();

	void SelectShopTab(int nTabIndex);
	void SelectEquipmentTab(int nTabIndex);

	void Update();

	// 상점/내장비창에서 좌측에 내 현재 장비를 나타내는 프레임
public:
	void SelectEquipmentFrameList( const char* szName, bool bOpen);
	int GetArmorWeaponTabIndexContainItemParts(MMatchCharItemParts parts);
	bool IsEquipmentFrameListOpened( const char* szName="Shop");

	// 내 현재 장비 목록의 탭 인덱스 (무기 리스트 탭/방어구 리스트 탭)
private:
	int m_idxArmorWeaponTab;
public:
	void SetArmorWeaponTabIndex(int idx) { if(idx < 0 || idx > 2) return; else m_idxArmorWeaponTab = idx;}
	int GetArmorWeaponTabIndex()	{ return m_idxArmorWeaponTab; }
	void SelectArmorWeaponTabWithSlotType(MMatchItemSlotType slotType);	// 인자의 슬롯타입에 맞는 탭을 선택 상태로
	void OnArmorWeaponTabButtonClicked(int nTab);

public:
	void ShowItemDescription(bool bShow, MTextArea* pTextArea, void* pCaller);
	
	// 상점/장비창에서 캐릭터의 기본 정보나, 특정 장비를 교체했을 때의 정보 변화를 텍스트로 표시한다	
	void DrawCharInfoText();
	void DrawCharInfoText(char* szShopOrEquip, int nReqLevel, int nNewWT, int nNewMaxWT, int nNewHP, int nNewAP, int nReqBounty, int nReqCash);
private:
	// 툴팁을 여기저기서 사용하기 때문에 남이 사용중인 툴팁을 멋대로 hide하지 못하도록 하기 위해서 툴팁 사용자를 기억함
	void* m_pItemDescriptionClient;
};
