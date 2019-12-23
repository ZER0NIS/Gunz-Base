#include "stdafx.h"
#include "MShopItem.h"

//////////////////////////////////////////////////////////////////////////////////////////

MImplementRootRTTI(MShopBaseItem);

MShopBaseItem::MShopBaseItem(int nMSID, int nID, int nCount, MShopItemType nType) 
{
	m_nMSID		= nMSID;

	m_nID		= nID;
	m_nCount	= nCount;
	m_nType		= nType;
}

MShopBaseItem::~MShopBaseItem()
{
	m_ShopPriceMap.Clear();
}

void MShopBaseItem::AddPrice(int nRentHourPeriod, int nPrice) 
{
	MShopPrice* pPrice = m_ShopPriceMap.GetShopPrice(nRentHourPeriod);
	if( pPrice != NULL ) return;

	m_ShopPriceMap.AddShopPrice(nRentHourPeriod, nPrice);
}

//////////////////////////////////////////////////////////////////////////////////////////

MImplementRTTI(MShopItem, MShopBaseItem);

MShopItem::MShopItem(int nMSID, int nID, MShopItemType nType)
	: MShopBaseItem(nMSID, nID, 1, nType) 
{
	m_nCSID = 0;
}

MShopItem::MShopItem(int nMSID, int nID, int nCount, int nCSID, MShopItemType nType)
	: MShopBaseItem(nMSID, nID, nCount, nType) 
{
	m_nCSID = nCSID;
}

//////////////////////////////////////////////////////////////////////////////////////////

MImplementRTTI(MShopSetItem, MShopBaseItem);

MShopSetItem::MShopSetItem(int nMSID, int nCSSID, vector<int>& vSetItem, char* szName, char* szDesc)	
	: MShopBaseItem(nMSID, nCSSID, 1, MSIT_SITEM) 
{
	int nSize = (int)vSetItem.size();
	_ASSERT(nSize <= MAX_SET_ITEM_COUNT);

	for (int i=0; i<MAX_SET_ITEM_COUNT; ++i)
	{
		if (i < nSize)
			m_nItemID[i] = vSetItem[i];
		else
			m_nItemID[i] = 0;
	}

	strcpy(m_szName, szName);
	strcpy(m_szDescription, szDesc);
}

//////////////////////////////////////////////////////////////////////////////////////////
