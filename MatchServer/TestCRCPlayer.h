#pragma once


struct MMatchItemDesc;


typedef map< int, MMatchItemDesc* > TestEquipMap;



class TestCRCPlayer
{
private :
	MUID			m_UID;
	TestEquipMap	m_EquipMap;


private :
	TestCRCPlayer() {}


public :
	TestCRCPlayer( const MUID& uid );
	~TestCRCPlayer();

	const MUID		GetUID()		{ return m_UID; }
	TestEquipMap&	GetEquipMap()	{ return m_EquipMap; }

	const bool		Equip( MMatchItemDesc* pItemDesc );
};



typedef map< MUID, TestCRCPlayer* > TestCRCPlayerMap;