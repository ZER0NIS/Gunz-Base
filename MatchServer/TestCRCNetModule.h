#pragma once



#include "TestCRCStage.h"



class TestCRCNetModule
{
private :
	ResourceCRC32CacheMap m_ResourceCRC32CacheList;

public :
	TestCRCNetModule();
	~TestCRCNetModule();

	void GetPlayerResourceCRC32Cache( const MUID uidPlayer, DWORD& out_crc32, DWORD& out_xor );

	void RequestToClient_ResourceCRC32Cache( const MUID uidPlayer, const DWORD dwResourceCRC32Cache, const DWORD dwResourceXORCache );
};