#pragma once


#include <map>
#include <vector>
#include <string>


using std::map;
using std::string;


typedef vector<MUID>		TestCRCStagePlayerUIDList;


class TestCRCServer;


class TestCRCStage
{
private :
	MUID						m_UID;
	MUID						m_uidMaster;
	string						m_strName;
	TestCRCStagePlayerUIDList	m_PlayerUIDList;
	ResourceCRC32CacheMap		m_ResourceCRC32CacheMap;

	
private :
	TestCRCStage() {}

	const bool		WasJoinedPlayer( const MUID uidPlayer );
	void			MakeResourceCRC32( TestCRCServer& tServer, const MUID uidPlayer, DWORD& out_crc32, DWORD& out_xor );
	const bool		SetResourceCRC32( const MUID uidPlayer, const DWORD dwResourceCRC32Cache, const DWORD dwResourceXORCache );
	const DWORD		GetResourceCRC32Cache( const MUID uidPlayer );
	const DWORD		GetResourceXORCache( const MUID uidPlayer );


public :
	TestCRCStage( const MUID& uid );
	~TestCRCStage();


	const bool		AddPlayer( const MUID uidPlayer );


	const MUID		GetUID()																			{ return m_UID; }


	void			SetMasterUID( const MUID uidMaster )												{ m_uidMaster = uidMaster; }
	void			SetName( const string& strName )													{ m_strName = strName; }

	const bool		EnterBattle( TestCRCServer& tServer, const MUID uidPlayer );
	const bool		IsValidResourceCRC32Cache( const MUID uidPlayer, const DWORD dwResourceCRC32Cache, const DWORD dwResourceXORCache );
};


typedef map< MUID, TestCRCStage* > TestCRCStageMap;