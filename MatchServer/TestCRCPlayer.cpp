#include "stdafx.h"
#include "TestCRCPlayer.h"
#include "TestCRC32Resource.h"



TestCRCPlayer::TestCRCPlayer( const MUID& uid )
{
	_ASSERT( MUID(0, 0) != uid );

	m_UID = uid;
}


TestCRCPlayer::~TestCRCPlayer()
{

}


const bool TestCRCPlayer::Equip( MMatchItemDesc* pItemDesc )
{
	_ASSERT( NULL != pItemDesc );

	if( NULL == pItemDesc )
	{
		return false;
	}

	m_EquipMap.insert( TestEquipMap::value_type(pItemDesc->m_nID, pItemDesc) );

	return true;
}