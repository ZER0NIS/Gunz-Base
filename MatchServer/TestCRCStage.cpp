#include "stdafx.h"
#include "TestCRCStage.h"
#include "TestCRC32Server.h"
#include "MMatchCRC32XORCache.h"



TestCRCStage::TestCRCStage( const MUID& uid )
{
	_ASSERT( MUID(0, 0) != uid );

	m_UID = uid;
};


TestCRCStage::~TestCRCStage()
{

}


const bool TestCRCStage::WasJoinedPlayer( const MUID uidPlayer )
{
	TestCRCStagePlayerUIDList::const_iterator	end = m_PlayerUIDList.end();
	TestCRCStagePlayerUIDList::iterator			it	= m_PlayerUIDList.begin();

	for( ; end != it; ++it )
	{
		if( uidPlayer == (*it) )
		{
			return true;
		}
	}

	return false;
}


const bool TestCRCStage::AddPlayer( const MUID uidPlayer )
{
	if( WasJoinedPlayer(uidPlayer) )
	{
		return false;
	}
	
	m_PlayerUIDList.push_back( uidPlayer );

	MMATCH_RESOURCECHECKINFO CRC32CacheInfo;

	CRC32CacheInfo.dwLastRequestTime	= 0;
	CRC32CacheInfo.dwResourceCRC32Cache = 0;
	CRC32CacheInfo.dwResourceXORCache   = 0;

	m_ResourceCRC32CacheMap.insert( ResourceCRC32CacheMap::value_type(uidPlayer, CRC32CacheInfo) );

	return true;
}


void TestCRCStage::MakeResourceCRC32( TestCRCServer& tServer, const MUID uidPlayer, DWORD& out_crc32, DWORD& out_xor )
{
	TestCRCPlayer* pPlayer = tServer.GetPlayer( uidPlayer );
	if( NULL == pPlayer )
	{
		return;
	}

	MMatchCRC32XORCache				CRC32Cacher;
	MMatchItemDesc*					pItemDesc	= NULL;
	TestEquipMap&					EquipMap	= pPlayer->GetEquipMap();
	TestEquipMap::const_iterator	end			= EquipMap.end();
	TestEquipMap::iterator			it			= EquipMap.begin();

	CRC32Cacher.Reset();

	for( ; end != it; ++it )
	{
		pItemDesc = reinterpret_cast<MMatchItemDesc*>( it->second );

		pItemDesc->CacheCRC32( CRC32Cacher );
	}

	mlog( "ResourceCRC : %u\n", CRC32Cacher.GetCRC32() );

	out_crc32 = CRC32Cacher.GetCRC32();
	out_xor = CRC32Cacher.GetXOR();
}


const bool TestCRCStage::SetResourceCRC32( const MUID uidPlayer, const DWORD dwResourceCRC32Cache, const DWORD dwResourceXORCache )
{
	ResourceCRC32CacheMap::iterator itFind = m_ResourceCRC32CacheMap.find( uidPlayer );
	if( m_ResourceCRC32CacheMap.end() == itFind )
	{
		return false;
	}

	itFind->second.dwResourceCRC32Cache = dwResourceCRC32Cache;
	itFind->second.dwResourceXORCache   = dwResourceXORCache;
	itFind->second.dwLastRequestTime	= timeGetTime();

	return true;
}


const bool TestCRCStage::EnterBattle( TestCRCServer& tServer, const MUID uidPlayer )
{
	TestCRCStagePlayerUIDList::const_iterator	end	= m_PlayerUIDList.end();
	TestCRCStagePlayerUIDList::iterator			it	= m_PlayerUIDList.begin();
	
	DWORD dwCRC32, dwXOR;

	for( ; end != it; ++it )
	{
		MakeResourceCRC32(tServer, uidPlayer, dwCRC32, dwXOR);
		if( !SetResourceCRC32(uidPlayer, dwCRC32, dwXOR) )
		{
			return false;
		}
	}

	MakeResourceCRC32(tServer, uidPlayer, dwCRC32, dwXOR);
	tServer.GetTestNetModule().RequestToClient_ResourceCRC32Cache( uidPlayer, dwCRC32, dwXOR );

	return true;
}


const DWORD TestCRCStage::GetResourceCRC32Cache( const MUID uidPlayer )
{
	ResourceCRC32CacheMap::iterator itFind = m_ResourceCRC32CacheMap.find( uidPlayer );
	if( m_ResourceCRC32CacheMap.end() == itFind )
		return 0;

	return static_cast<DWORD>( itFind->second.dwResourceCRC32Cache );
}

const DWORD TestCRCStage::GetResourceXORCache( const MUID uidPlayer )
{
	ResourceCRC32CacheMap::iterator itFind = m_ResourceCRC32CacheMap.find( uidPlayer );
	if( m_ResourceCRC32CacheMap.end() == itFind )
		return 0;

	return static_cast<DWORD>( itFind->second.dwResourceXORCache );
}


const bool TestCRCStage::IsValidResourceCRC32Cache( const MUID uidPlayer, const DWORD dwResourceCRC32Cache, const DWORD dwResourceXORCache )
{
	_ASSERT( 0 != dwResourceCRC32Cache );

	return (dwResourceCRC32Cache == GetResourceCRC32Cache( uidPlayer ) &&
			dwResourceXORCache == GetResourceXORCache( uidPlayer ));

}