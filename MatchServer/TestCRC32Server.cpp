#include "stdafx.h"
#include "TestCRC32Server.h"
#include "MUID.h"



TestCRCServer::TestCRCServer()
{
	m_UIDCache = 0;
}


TestCRCServer::~TestCRCServer()
{
	_ASSERT( m_ItemDescMap.empty() );
	_ASSERT( m_PlayerMap.empty() );
	_ASSERT( m_StageMap.empty() );

	m_UIDCache = 0;
}



const bool TestCRCServer::AddItemDesc( const int nItemID, MMatchItemDesc* pItemDesc )
{
	_ASSERT( 0 <= nItemID );
	_ASSERT( NULL != pItemDesc );

	if( 0 > nItemID )
	{
		return false;
	}

	if( NULL == pItemDesc )
	{
		return false;
	}

	m_ItemDescMap.insert( T_ItemDescMap::value_type(nItemID, pItemDesc) );

	return true;
}


void TestCRCServer::ReleaseItemDescMap()
{
	T_ItemDescMap::iterator			it			= m_ItemDescMap.begin();
	T_ItemDescMap::const_iterator	end			= m_ItemDescMap.end();
	MMatchItemDesc*					pItemDesc	= NULL;

	for( ; end != it; ++it )
	{
		pItemDesc = it->second;

		delete pItemDesc;
	}

	m_ItemDescMap.clear();
}


const MUID TestCRCServer::CreateNewUID()
{
	return ++m_UIDCache;
}


const bool	TestCRCServer::AddPlayer( TestCRCPlayer* pPlayer )
{
	_ASSERT( NULL != pPlayer );
	_ASSERT( MUID(0, 0) != pPlayer->GetUID() );

	if( NULL == pPlayer )
	{
		return false;
	}

	if( NULL != GetPlayer(pPlayer->GetUID()) )
	{
		return false;
	}

	m_PlayerMap.insert( TestCRCPlayerMap::value_type(pPlayer->GetUID(), pPlayer) );

	return true;
}


void TestCRCServer::ReleasePlayerMap()
{
	TestCRCPlayerMap::iterator			it		= m_PlayerMap.begin();
	TestCRCPlayerMap::const_iterator	end		= m_PlayerMap.end();
	TestCRCPlayer*						pPlayer = NULL;

	for( ; end != it; ++it )
	{
		pPlayer = it->second;

		delete pPlayer;
	}

	m_PlayerMap.clear();
}


MMatchItemDesc* TestCRCServer::GetItemDesc( const int nItemID )
{
	T_ItemDescMap::const_iterator itFind = m_ItemDescMap.find( nItemID );
	if( m_ItemDescMap.end() == itFind )
	{
		return NULL;
	}

	return reinterpret_cast<MMatchItemDesc*>( itFind->second );
}


TestCRCPlayer* TestCRCServer::GetPlayer( const MUID uidPlayer )
{
	TestCRCPlayerMap::const_iterator itFind = m_PlayerMap.find( uidPlayer );
	if( m_PlayerMap.end() == itFind )
	{
		return NULL;
	}

	return reinterpret_cast<TestCRCPlayer*>( itFind->second );
}


const bool TestCRCServer::AddStage( TestCRCStage* pStage )
{
	_ASSERT( NULL != pStage );

	TestCRCStageMap::const_iterator itFind = m_StageMap.find( pStage->GetUID() );
	if( m_StageMap.end() != itFind )
	{
		return false;
	}

	m_StageMap.insert( TestCRCStageMap::value_type(pStage->GetUID(), pStage) );

	return true;
}


TestCRCStage* TestCRCServer::GetStage( const MUID uidStage )
{
	TestCRCStageMap::const_iterator itFind = m_StageMap.find( uidStage );
	if( m_StageMap.end() == itFind )
	{
		return NULL;
	}

	return reinterpret_cast<TestCRCStage*>( itFind->second );
}


const bool TestCRCServer::JoinStage( const MUID uidStage, const MUID uidPlayer )
{
	TestCRCStage* pStage = GetStage( uidStage );
	if( NULL == pStage )
	{
		return false;
	}

	return pStage->AddPlayer( uidPlayer );
}


void TestCRCServer::ReleaseStageMap()
{
	TestCRCStageMap::const_iterator end		= m_StageMap.end();
	TestCRCStageMap::iterator		it		= m_StageMap.begin();
	TestCRCStage*					pStage	= NULL;

	for( ; end != it; ++it )
	{
		pStage = it->second;

		delete pStage;
	}

	m_StageMap.clear();
}