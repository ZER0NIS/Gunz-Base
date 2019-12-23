#ifndef _ZITEMLIST_H
#define _ZITEMLIST_H

#include "ZPrerequisites.h"
#include "MMatchItem.h"
#include "MBaseItem.h"
#include "MMatchGambleItem.h"
#include "ZMyItem.h"
#include "ZMyQuestItem.h"
#include "ZGambleItemDefine.h"
#include "ZMyGambleItem.h"

#include <list>
#include <map>
using namespace std;


enum ITEM_EQUIP_PARTS
{
	ITEM_EQUIP_PARTS_ALL = 0,
	ITEM_EQUIP_PARTS_HEAD,
	ITEM_EQUIP_PARTS_CHEST,
	ITEM_EQUIP_PARTS_HANDS,
	ITEM_EQUIP_PARTS_LEGS,
	ITEM_EQUIP_PARTS_FEET,
	ITEM_EQUIP_PARTS_MELEE,
	ITEM_EQUIP_PARTS_PRIMARY,
	ITEM_EQUIP_PARTS_SECONDARY,
	ITEM_EQUIP_PARTS_ITEM,

	ITEM_EQUIP_PARTS_END
};

class ZMyItemList
{
protected:
	bool							m_bCreated;
	MUID							m_uidEquipItems[MMCIP_END];
	unsigned long int				m_nEquipItemID[MMCIP_END];
	unsigned long int				m_nEquipItemCount[MMCIP_END]; 

	MITEMNODEMAP					m_ItemMap;
	vector<MUID>					m_ItemIndexVectorEquip;
	vector<MUID>					m_ItemIndexVector;
	
	// Account Item 관련
	MACCOUNT_ITEMNODEMAP			m_AccountItemMap;
	vector<int>						m_AccountItemVector;

	ZMyQuestItemMap					m_QuestItemMap;
	ZMyGambleItemManager			m_GambleItemMgr;

protected :
	void ClearItemMap();
	void ClearAccountItemMap();

public:

	int		m_ListFilter;

public:
	ZMyItemList();
	virtual ~ZMyItemList();

	bool Create();
	void Destroy();
	void Clear();

	bool CheckTypeWithListFilter(int type, bool bEnchantItem);

	void GetNamedComp( int nItemID, char* szBmpWidgetName, char* szBmpName, char* szLabelWidgetName);

	int GetItemCount() { return (int)m_ItemMap.size(); }
	int GetGambleItemCount() { return (int)m_GambleItemMgr.GetSize(); }

	unsigned long int GetItemID(int nItemIndex);
	unsigned long int GetItemIDEquip(int nItemIndex);
	unsigned long int GetItemID(const MUID& uidItem);
	unsigned long int GetAccountItemID(int nPos);
	unsigned long int GetEquipedItemID(MMatchCharItemParts parts);

	ZMyItemNode* GetItem(int nItemIndex);
	ZMyItemNode* GetItemEquip(int nItemIndex);
	ZMyItemNode* GetItem(const MUID& uidItem);
	ZMyItemNode* GetEquipedItem(MMatchCharItemParts parts);
	ZMyItemNode* GetAccountItem(int nPos);
	ZMyItemNode* GetAccountItemByAIID(int nAIID);

	ZMyItemNode* GetItemByDescId(int nItemId);

	const ZMyGambleItem* GetGambleItem(const MUID& uidItem);
	void SetGambleItemAll(MTD_GambleItemNode* pGItemNodes, int nGItemCount );

	MUID GetEquipedItemUID(MMatchCharItemParts parts);
	void SetEquipItemsAll(MUID* pnEquipItems);
	void SetEquipItemInfo(MUID* pEquipItemUID, unsigned long int* pEquipItemID, unsigned long int* pEquipItemCount);
	
	void SetItemsAll(MTD_ItemNode* pItemNodes, const int nItemCount);	
	bool IsCreated() { return m_bCreated; }

	MUID GetForceItemUID(int nItemIndex);
	MUID GetItemUID(int nItemIndex);
	MUID GetItemUIDEquip(int nItemIndex);

	void Serialize();
	void SerializeAccountItem();		// 창고 인터페이스를 동기화한다
	int GetEquipedTotalWeight();
	int GetEquipedHPModifier();
	int GetEquipedAPModifier();
	int GetMaxWeight();

	void MakeMyItemUIDList();
	
	void SerializeZItemList();
	void SerializeQItemList();
	void SerializeGItemList();
	
	// AccountItem 관련
	void AddAccountItem(int nAIID, unsigned long int nItemID, int nCount, int nRentMinutePeriodRemainder=RENT_MINUTE_PERIOD_UNLIMITED);
	void ClearAccountItems();

	// quest
	ZMyQuestItemMap&	GetQuestItemMap()	{ return m_QuestItemMap; }
	int					GetQuestItemCount()	{ return static_cast< int >( m_QuestItemMap.size() ); }
	void				QuestItemClear()	{ m_QuestItemMap.Clear(); }

	//ZMyGambleItemManager& GetGambleItemMap() { return m_GambleItemMgr;
	void SetQuestItemsAll( MTD_QuestItemNode* pQuestItemNode, const int nQuestItemCount );
};


#endif