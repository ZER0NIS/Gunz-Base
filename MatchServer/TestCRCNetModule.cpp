#include "stdafx.h"
#include "TestCRCNetModule.h"



TestCRCNetModule::TestCRCNetModule()
{

}


TestCRCNetModule::~TestCRCNetModule()
{
	
}


void TestCRCNetModule::RequestToClient_ResourceCRC32Cache( const MUID uidPlayer, const DWORD dwResourceCRC32Cache, const DWORD dwResourceXORCache )
{
	_ASSERT( MUID(0, 0) != uidPlayer );
	_ASSERT( 0 != dwResourceCRC32Cache );

	ResourceCRC32CacheMap::iterator itFind = m_ResourceCRC32CacheList.find( uidPlayer );
	if( m_ResourceCRC32CacheList.end() != itFind )
	{
		itFind->second.dwResourceCRC32Cache = dwResourceCRC32Cache;
		itFind->second.dwResourceXORCache = dwResourceXORCache;
		itFind->second.dwLastRequestTime	= timeGetTime();
	}
	else
	{
		MMATCH_RESOURCECHECKINFO CRC32CacheInfo;

		CRC32CacheInfo.dwLastRequestTime	= timeGetTime();
		CRC32CacheInfo.dwResourceCRC32Cache = dwResourceCRC32Cache;
		CRC32CacheInfo.dwResourceXORCache = dwResourceXORCache;

		m_ResourceCRC32CacheList.insert( ResourceCRC32CacheMap::value_type(uidPlayer, CRC32CacheInfo) );
	}
}



void TestCRCNetModule::GetPlayerResourceCRC32Cache( const MUID uidPlayer, DWORD& out_crc32, DWORD& out_xor )
{
	ResourceCRC32CacheMap::iterator itFind = m_ResourceCRC32CacheList.find( uidPlayer );
	if( m_ResourceCRC32CacheList.end() == itFind )
	{
		return;
	}

	out_crc32 = itFind->second.dwResourceCRC32Cache;
	out_xor = itFind->second.dwResourceXORCache;
}