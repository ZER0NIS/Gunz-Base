#include "stdafx.h"
#include "MCRC32.h"
#include "MMatchCRC32XORCache.h"
#include "TestCRC32XORCache.h"
#include "MMatchItem.h"
#include "MMatchWorldItemDesc.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


typedef map< int, MMatchItemDesc* >						T_ItemDescMap;
typedef map< short, MMatchMapsWorldItemSpawnInfoSet* >	T_MMatchMapsWorldItemSpawnInfoSetMap;


static const int TEST_ITEM_COUNT		= 100;
static const int TEST_EQUIPLIST_COUNT	= 5;
static const int TEST_EQUIP_ITEM_COUNT	= 12;

static const int TEST_WITEM_COUNT		= 100;
static const int TEST_WITEM_SPAWN_COUNT	= 5;



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




T_ItemDescMap							g_ItemDescMgr;
T_ItemDescMap							g_EquipItemList[ TEST_EQUIPLIST_COUNT ];
T_ItemDescMap							g_ChecksumItemDescList;

T_MMatchMapsWorldItemSpawnInfoSetMap	g_WItemDescMgr;
T_MMatchMapsWorldItemSpawnInfoSetMap	g_SpawnInfoList;

DWORD									g_dwXORCRC32_Inc; 
DWORD									g_dwXORCRC32_Des; // g_dwXORCRC32_Inc과 순서 바꿔서.

MMatchCRC32XORCache						g_CRC32XORCache_Inc;
MMatchCRC32XORCache						g_CRC32XORCache_Des;



void T_InitItemDescMgr()
{
	for( int i = 0; TEST_ITEM_COUNT > i; ++i )
	{
		MMatchItemDesc* pItemDesc = new MMatchItemDesc;

		memset( pItemDesc, 0, sizeof(MMatchItemDesc) );

		pItemDesc->m_nID = i;

		sprintf( pItemDesc->m_szDesc, "ItemDesc.nItemID = %d", i );

		g_ItemDescMgr.insert( T_ItemDescMap::value_type(i, pItemDesc) );
	}

	_ASSERT( !g_ItemDescMgr.empty() );

	mlog( "test item desc count : %u\n", TEST_ITEM_COUNT );
}


void T_ReleaseItemDescMap()
{
	T_ItemDescMap::iterator it, end;
	end = g_ItemDescMgr.end();
	for( it = g_ItemDescMgr.begin(); end != it; ++it )
	{
		delete it->second;
	}

	g_ItemDescMgr.clear();
}


MMatchItemDesc* T_FindItemDesc( const int nItemID )
{
	T_ItemDescMap::iterator itFind = g_ItemDescMgr.find( nItemID );
	if( g_ItemDescMgr.end() == itFind )
	{
		return NULL;
	}

	return reinterpret_cast<MMatchItemDesc*>( itFind->second );
}


void T_InitEquipItemList()
{
	for( int i = 0; TEST_EQUIPLIST_COUNT > i; ++i )
	{
		g_EquipItemList[ i ].clear();

		for( int j = 0; TEST_EQUIP_ITEM_COUNT > j; ++j )
		{
			const int nItemID = static_cast<int>( rand() % TEST_ITEM_COUNT );

			MMatchItemDesc* pItemDesc = T_FindItemDesc( nItemID );
			_ASSERT( NULL != pItemDesc );
			
			g_EquipItemList[ i ].insert( T_ItemDescMap::value_type(nItemID, pItemDesc) );
		}
	}

	mlog( "test equip item count : %u\n", TEST_EQUIPLIST_COUNT *  TEST_EQUIP_ITEM_COUNT );
}


void T_InitChecksumItemDescList()
{
	g_ChecksumItemDescList.clear();

	T_ItemDescMap::iterator itEndEq, itEq ;

	for( int i = 0; TEST_EQUIPLIST_COUNT > i; ++i )
	{
		itEndEq = g_EquipItemList[ i ].end();
		itEq	= g_EquipItemList[ i ].begin();

		for( ; itEndEq != itEq; ++itEq )
		{
			const int nItemID = itEq->first;

			g_ChecksumItemDescList.insert( T_ItemDescMap::value_type(nItemID, itEq->second) );
		}
	}
}


const DWORD T_BuildResourceChecksum( const DWORD dwChecksum, BYTE* pBuff, const DWORD nSize )
{
	_ASSERT( NULL != pBuff );

	const DWORD dwNewChecksum = MCRC32::BuildCRC32( pBuff, nSize );

	return dwChecksum ^ (dwNewChecksum >> 2);
}


void TestSameValueChecksumParamOutputIsNotZero()
{
	DWORD a = 5;
	DWORD b = T_BuildResourceChecksum( a, (BYTE*)&a, sizeof(a) );

	_ASSERT( 0 != b );
}


void T_RandomOrderCheckParamOutputIsAllEqual()
{
	DWORD a				= 4;
	DWORD b				= 5;
	DWORD c				= 6;

	DWORD dwChecksum1	= 0;
	DWORD dwChecksum2	= 0;
	DWORD dwChecksum3	= 0;

	dwChecksum1 = T_BuildResourceChecksum( dwChecksum1, (BYTE*)&a, 4 );
	dwChecksum1 = T_BuildResourceChecksum( dwChecksum1, (BYTE*)&b, 4 );
	dwChecksum1 = T_BuildResourceChecksum( dwChecksum1, (BYTE*)&c, 4 );

	dwChecksum2 = T_BuildResourceChecksum( dwChecksum2, (BYTE*)&b, 4 );
	dwChecksum2 = T_BuildResourceChecksum( dwChecksum2, (BYTE*)&c, 4 );
	dwChecksum2 = T_BuildResourceChecksum( dwChecksum2, (BYTE*)&a, 4 );

	dwChecksum3 = T_BuildResourceChecksum( dwChecksum3, (BYTE*)&c, 4 );
	dwChecksum3 = T_BuildResourceChecksum( dwChecksum3, (BYTE*)&a, 4 );
	dwChecksum3 = T_BuildResourceChecksum( dwChecksum3, (BYTE*)&b, 4 );

	_ASSERT( dwChecksum1 == dwChecksum2 );
	_ASSERT( dwChecksum2 == dwChecksum3 );
	_ASSERT( dwChecksum3 == dwChecksum1 );
}


const DWORD	T_BuildEquipResourceChecksum( const DWORD dwChecksum )
{
	DWORD					dwNewChecksum	= dwChecksum;
	T_ItemDescMap::iterator	it				= g_ChecksumItemDescList.begin();
	T_ItemDescMap::iterator	end				= g_ChecksumItemDescList.end();

	for( ; end != it; ++it )
	{
		MMatchItemDesc* pItemDesc = reinterpret_cast<MMatchItemDesc*>( it->second );

		dwNewChecksum = T_BuildResourceChecksum( dwNewChecksum, (BYTE*)pItemDesc, sizeof(MMatchItemDesc) );
	}

	_ASSERT( 0 != dwNewChecksum );
	_ASSERT( 0xffffffff != dwNewChecksum );

	return dwNewChecksum;
}


const DWORD T_BuildWItemResourceChecksum( const DWORD dwChecksum )
{
	DWORD											dwNewChecksum	= dwChecksum;
	T_MMatchMapsWorldItemSpawnInfoSetMap::iterator	it				= g_SpawnInfoList.begin();
	T_MMatchMapsWorldItemSpawnInfoSetMap::iterator	end				= g_SpawnInfoList.end();

	for( ; end != it; ++it )
	{
		MMatchMapsWorldItemSpawnInfoSet* pWItemDesc = reinterpret_cast<MMatchMapsWorldItemSpawnInfoSet*>( it->second );

		dwNewChecksum = T_BuildResourceChecksum( dwNewChecksum, (BYTE*)pWItemDesc, sizeof(MMatchMapsWorldItemSpawnInfoSet) );
	}

	_ASSERT( 0 != dwNewChecksum );
	_ASSERT( 0xffffffff != dwNewChecksum );

	return dwNewChecksum;
}


void T_BuildEquipListCRC32XORCache( MMatchCRC32XORCache* pCRC32XORCache )
{
	_ASSERT( NULL != pCRC32XORCache );

	T_ItemDescMap::iterator	itItem	= g_ChecksumItemDescList.begin();
	T_ItemDescMap::iterator	endItem	= g_ChecksumItemDescList.end();

	for( ; endItem != itItem; ++itItem )
	{
		MMatchItemDesc* pItemDesc = reinterpret_cast<MMatchItemDesc*>( itItem->second );

		pItemDesc->CacheCRC32( *pCRC32XORCache );
	}

	_ASSERT( 0 != pCRC32XORCache->GetXOR() );
	_ASSERT( 0xffffffff != pCRC32XORCache->GetXOR() );
}


void T_BuildSpawnInfoSetCRC32XORCache( MMatchCRC32XORCache* pCRC32XORCache )
{
	_ASSERT( NULL != pCRC32XORCache );

	T_MMatchMapsWorldItemSpawnInfoSetMap::iterator	itWItem	= g_SpawnInfoList.begin();
	T_MMatchMapsWorldItemSpawnInfoSetMap::iterator	endWItem = g_SpawnInfoList.end();

	for( ; endWItem != itWItem; ++itWItem )
	{
		MMatchMapsWorldItemSpawnInfoSet* pWItemDesc = reinterpret_cast<MMatchMapsWorldItemSpawnInfoSet*>( itWItem->second );

		pCRC32XORCache->CRC32XOR( pWItemDesc->GetCRC32() );
	}

	_ASSERT( 0 != pCRC32XORCache->GetXOR() );
	_ASSERT( 0xffffffff != pCRC32XORCache->GetXOR() );
}


void T_BuildResourceListChecksum()
{
	g_dwXORCRC32_Inc = 0;

	g_dwXORCRC32_Inc = T_BuildEquipResourceChecksum( g_dwXORCRC32_Inc );
	g_dwXORCRC32_Inc = T_BuildWItemResourceChecksum( g_dwXORCRC32_Inc );
	_ASSERT( 0 != g_dwXORCRC32_Inc );

	g_dwXORCRC32_Des = 0;

	g_dwXORCRC32_Des = T_BuildWItemResourceChecksum( g_dwXORCRC32_Des );
	g_dwXORCRC32_Des = T_BuildEquipResourceChecksum( g_dwXORCRC32_Des );
	_ASSERT( 0 != g_dwXORCRC32_Des );

	g_CRC32XORCache_Inc.Reset();
	T_BuildEquipListCRC32XORCache( &g_CRC32XORCache_Inc );
	T_BuildSpawnInfoSetCRC32XORCache( &g_CRC32XORCache_Inc );
	_ASSERT( 0 != g_CRC32XORCache_Inc.GetXOR() );

	g_CRC32XORCache_Des.Reset();
	T_BuildSpawnInfoSetCRC32XORCache( &g_CRC32XORCache_Des );
	T_BuildEquipListCRC32XORCache( &g_CRC32XORCache_Des );	
	_ASSERT( 0 != g_CRC32XORCache_Des.GetXOR() );

	// 순서가 다르더라고 최종값은 같아야 한다.
	_ASSERT( g_dwXORCRC32_Inc				== g_dwXORCRC32_Des );
	_ASSERT( g_CRC32XORCache_Inc.GetXOR() == g_CRC32XORCache_Des.GetXOR() );
	_ASSERT( g_dwXORCRC32_Inc				== g_CRC32XORCache_Inc.GetXOR() );
	_ASSERT( g_dwXORCRC32_Des				== g_CRC32XORCache_Des.GetXOR() );
}


void T_InitWItemDescMgr()
{
	g_WItemDescMgr.clear();

	for( int i = 0; TEST_WITEM_COUNT > i; ++i )
	{
		MMatchMapsWorldItemSpawnInfoSet* pWItemDesc = new MMatchMapsWorldItemSpawnInfoSet;

		pWItemDesc->m_nSoloSpawnCount = i;
		pWItemDesc->m_nTeamSpawnCount = i + 10;

		g_WItemDescMgr.insert( T_MMatchMapsWorldItemSpawnInfoSetMap::value_type(i, pWItemDesc) );
	}

	_ASSERT( !g_WItemDescMgr.empty() );

	mlog( "test spawn info desc count : %u\n", TEST_WITEM_COUNT );
}


void T_ReleaseWItemDescMap()
{
	T_MMatchMapsWorldItemSpawnInfoSetMap::iterator it	= g_WItemDescMgr.begin();
	T_MMatchMapsWorldItemSpawnInfoSetMap::iterator end	= g_WItemDescMgr.end();

	for( ; end != it; ++it )
	{
		delete it->second;
	}

	g_WItemDescMgr.clear();
}


MMatchMapsWorldItemSpawnInfoSet* T_FindWItemDesc( const int nWItemID )
{
	T_MMatchMapsWorldItemSpawnInfoSetMap::iterator itFind = g_WItemDescMgr.find( nWItemID );
	if( g_WItemDescMgr.end() == itFind )
	{
		return NULL;
	}

	return reinterpret_cast<MMatchMapsWorldItemSpawnInfoSet*>( itFind->second );
}


void T_InitSpawnWItemList()
{
	for( int i = 0; TEST_WITEM_SPAWN_COUNT > i; ++i )
	{
		const int nWItemID = rand() % TEST_WITEM_COUNT;

		MMatchMapsWorldItemSpawnInfoSet* pWItemDesc = T_FindWItemDesc( nWItemID );
		_ASSERT( NULL != pWItemDesc );

		g_SpawnInfoList.insert( T_MMatchMapsWorldItemSpawnInfoSetMap::value_type(nWItemID, pWItemDesc) );
	}

	mlog( "test spawn item count : %u\n", TEST_WITEM_SPAWN_COUNT );
}


void T_ResourceCRC32MemberFunction()
{
	//MMatchItemDesc* pItemDesc = new MMatchItemDesc;

	//pItemDesc->m_nID = 1;
	//memset( pItemDesc->m_szDesc, 0, sizeof(pItemDesc->m_szDesc) );
	//sprintf(pItemDesc->m_szDesc, "test" );


	//const DWORD dw1 = pItemDesc->GetCRC32();


	//MMemoryProxy<MMatchItemName>*	pItemName	= pItemDesc->m_pMItemName;
	//MMatchItemEffectDesc*			pErrect		= pItemDesc->m_pEffect;

	//pItemDesc->m_pMItemName	= NULL;
	//pItemDesc->m_pEffect	= NULL;

	//const DWORD dw2 = MCRC32::BuildCRC32( (BYTE*)pItemDesc, DWORD(sizeof(MMatchItemDesc)) );

	//pItemDesc->m_pMItemName	= pItemName;
	//pItemDesc->m_pEffect	= pErrect;

	//_ASSERT( dw1 == dw2 );

	//delete pItemDesc;
}


void T_BitXORTestForCRC32XORCache()
{
	const int a	= 1234;
	const int b	= 2345;
	const int c = 3456;

	int nChecksum1 = 0;

	nChecksum1 ^= (a >> 2);
	nChecksum1 ^= (b >> 2);
	nChecksum1 ^= (c >> 2);

	_ASSERT( 0 != nChecksum1 );
	
	int nChecksum2 = 0; ///< - a

	nChecksum2 ^= (b >> 2);
	nChecksum2 ^= (c >> 2);

	_ASSERT( 0 != nChecksum2 );

	int nChecksum3 = 0; ///< - b
	nChecksum3 ^= (a >> 2);
	nChecksum3 ^= (c >> 2);

	_ASSERT( 0 != nChecksum3 );

	int nChecksum4 = 0; ///< - c
	nChecksum4 ^= (a >> 2);
	nChecksum4 ^= (b >> 2);

	_ASSERT( 0 != nChecksum4 );

	// 모두 xor한 checksum1에서 각각 빼면은 순서에 상관없이 같아야 한다.
	_ASSERT( nChecksum2 == (nChecksum1 ^ (a >> 2)) );
	_ASSERT( nChecksum3 == (nChecksum1 ^ (b >> 2)) );
	_ASSERT( nChecksum4 == (nChecksum1 ^ (c >> 2)) );

	// 뺀것을 다시 xor하면 모두 xor한 checksum1과 같아야 한다.
	_ASSERT( nChecksum1 == (nChecksum2 ^ (a >> 2)) );
	_ASSERT( nChecksum1 == (nChecksum3 ^ (b >> 2)) );
	_ASSERT( nChecksum1 == (nChecksum4 ^ (c >> 2)) );

	int nChecksum5 = 0;
	nChecksum5 = nChecksum1 ^ (a >> 2);
	nChecksum5 ^= (b >> 2);
	nChecksum5 ^= (c >> 2);

	// 모두 xor한 checksum1에 다시 모두 xor하면 초기값 0이 나와야 한다.
	_ASSERT( 0 == nChecksum5 );
}


void T_DeleteCRC32ForCRC32XORCache()
{
	const DWORD dwChecksumInc_Org = g_CRC32XORCache_Inc.GetXOR();
	const DWORD dwChecksumDes_Org = g_CRC32XORCache_Des.GetXOR();

	MMatchItemDesc* pTestItemDesc1 = T_FindItemDesc( int(rand() % TEST_ITEM_COUNT) );
	MMatchItemDesc* pTestItemDesc2 = T_FindItemDesc( int(rand() % TEST_ITEM_COUNT) );

	_ASSERT( NULL != pTestItemDesc1 );
	_ASSERT( NULL != pTestItemDesc2 );

	pTestItemDesc1->CacheCRC32( g_CRC32XORCache_Inc );
	pTestItemDesc1->CacheCRC32( g_CRC32XORCache_Des );

	_ASSERT( g_CRC32XORCache_Inc.GetXOR() == g_CRC32XORCache_Des.GetXOR() );

	const DWORD dwChecksumInc_1 = g_CRC32XORCache_Inc.GetXOR();
	const DWORD dwChecksumDes_1 = g_CRC32XORCache_Des.GetXOR();

	pTestItemDesc2->CacheCRC32( g_CRC32XORCache_Inc );
	pTestItemDesc2->CacheCRC32( g_CRC32XORCache_Des );
	
	_ASSERT( g_CRC32XORCache_Inc.GetXOR() == g_CRC32XORCache_Des.GetXOR() );

	const DWORD dwChecksumInc_2 = g_CRC32XORCache_Inc.GetXOR();
	const DWORD dwChecksumDes_2 = g_CRC32XORCache_Des.GetXOR();

	MMatchMapsWorldItemSpawnInfoSet* pTestWItemSpanInfo1 = T_FindWItemDesc( int(rand() % TEST_WITEM_COUNT) );
	MMatchMapsWorldItemSpawnInfoSet* pTestWItemSpanInfo2 = T_FindWItemDesc( int(rand() % TEST_WITEM_COUNT) );

	_ASSERT( NULL != pTestWItemSpanInfo1 );
	_ASSERT( NULL != pTestWItemSpanInfo2 );

	g_CRC32XORCache_Inc.CRC32XOR( pTestWItemSpanInfo1->GetCRC32() );
	g_CRC32XORCache_Des.CRC32XOR( pTestWItemSpanInfo1->GetCRC32() );

	_ASSERT( g_CRC32XORCache_Inc.GetXOR() == g_CRC32XORCache_Des.GetXOR() );

	const DWORD dwChecksumInc_3 = g_CRC32XORCache_Inc.GetXOR();
	const DWORD dwChecksumDes_3 = g_CRC32XORCache_Des.GetXOR();

	g_CRC32XORCache_Inc.CRC32XOR( pTestWItemSpanInfo2->GetCRC32() );
	g_CRC32XORCache_Des.CRC32XOR( pTestWItemSpanInfo2->GetCRC32() );

	_ASSERT( g_CRC32XORCache_Inc.GetXOR() == g_CRC32XORCache_Des.GetXOR() );

	//// 다시 빼보자.

	g_CRC32XORCache_Inc.CRC32XOR( pTestWItemSpanInfo2->GetCRC32() );
	g_CRC32XORCache_Des.CRC32XOR( pTestWItemSpanInfo2->GetCRC32() );

	_ASSERT( g_CRC32XORCache_Inc.GetXOR() == g_CRC32XORCache_Des.GetXOR() );

	_ASSERT( dwChecksumInc_3 == g_CRC32XORCache_Inc.GetXOR() );
	_ASSERT( dwChecksumDes_3 == g_CRC32XORCache_Des.GetXOR() );

	g_CRC32XORCache_Inc.CRC32XOR( pTestWItemSpanInfo1->GetCRC32() );
	g_CRC32XORCache_Des.CRC32XOR( pTestWItemSpanInfo1->GetCRC32() );

	_ASSERT( g_CRC32XORCache_Inc.GetXOR() == g_CRC32XORCache_Des.GetXOR() );

	_ASSERT( dwChecksumInc_2 == g_CRC32XORCache_Inc.GetXOR() );
	_ASSERT( dwChecksumDes_2 == g_CRC32XORCache_Des.GetXOR() );

	pTestItemDesc2->CacheCRC32( g_CRC32XORCache_Inc );
	pTestItemDesc2->CacheCRC32( g_CRC32XORCache_Des );
	
	_ASSERT( g_CRC32XORCache_Inc.GetXOR() == g_CRC32XORCache_Des.GetXOR() );

	_ASSERT( dwChecksumInc_1 == g_CRC32XORCache_Inc.GetXOR() );
	_ASSERT( dwChecksumDes_1 == g_CRC32XORCache_Des.GetXOR() );

	pTestItemDesc1->CacheCRC32( g_CRC32XORCache_Inc );
	pTestItemDesc1->CacheCRC32( g_CRC32XORCache_Des );
	
	_ASSERT( g_CRC32XORCache_Inc.GetXOR() == g_CRC32XORCache_Des.GetXOR() );

	_ASSERT( dwChecksumInc_Org == g_CRC32XORCache_Inc.GetXOR() );
	_ASSERT( dwChecksumDes_Org == g_CRC32XORCache_Des.GetXOR() );
}


void DoTestResourceCRC32Cache()
{
	// 테스트에 필요한 리소스 초기화.
	T_InitItemDescMgr();
	T_InitWItemDescMgr();

	T_BitXORTestForCRC32XORCache();
	
	// 같은 값이 들어가도 Checksum은 새로운 값이 나와야 한다.
	TestSameValueChecksumParamOutputIsNotZero();

	// 순서가 다르더라고 같은 데이터는 최종값이 항상 같아야 한다.
	T_RandomOrderCheckParamOutputIsAllEqual();

	// Checksum구성에 필요한 리스트 구성.
	T_InitEquipItemList();
	T_InitChecksumItemDescList();
	T_InitSpawnWItemList();

	// 리소스 리스트로 Checksum구하기.
	T_BuildResourceListChecksum();

	T_ResourceCRC32MemberFunction();

	T_DeleteCRC32ForCRC32XORCache();

	// 5.테스트를 위해 사용했던 리소스 정리.
	T_ReleaseItemDescMap();
	T_ReleaseWItemDescMap();
}