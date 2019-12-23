#pragma once



//// MMatchItemDesc로 교체되어야 한다.
//struct MMatchItemDesc
//{
//	int		m_nID;
//	char	m_szDesc[256];
//
//	__forceinline const DWORD GetCRC32();
//};
//
//struct MMatchMapsWorldItemSpawnInfoSet
//{
//	int	m_nSoloSpawnCount;
//	int	m_nTeamSpawnCount;
//
//	__forceinline const DWORD GetCRC32();
//};



#include "MMatchItem.h"


typedef map< int, MMatchItemDesc* >						T_ItemDescMap;


static const int TEST_ITEMDESC_COUNT	= 100;
static const int TEST_EQUIPLIST_COUNT	= 5;
static const int TEST_EQUIP_ITEM_COUNT	= 12;

static const int TEST_WITEM_COUNT		= 100;
static const int TEST_WITEM_SPAWN_COUNT	= 5;