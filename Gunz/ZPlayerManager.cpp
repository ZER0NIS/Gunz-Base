#include "stdafx.h"
#include "ZPlayerManager.h"



ZPlayerManager::ZPlayerManager()
{
}

ZPlayerManager::~ZPlayerManager()
{
	Clear();
}

ZPlayerManager* ZPlayerManager::GetInstance()
{
	static ZPlayerManager PlayerManager;
	return &PlayerManager;
}

void ZPlayerManager::AddPlayer( MUID& uID, ZPlayerInfo* pInfo)
{
	m_PlayerList.insert( map<MUID,ZPlayerInfo*>::value_type( uID, pInfo));
}

void ZPlayerManager::AddPlayer( MUID& uID, const char* name, int rank, int kill, int death)
{
	ZPlayerInfo* pInfo = new ZPlayerInfo( name, rank, kill, death);

	AddPlayer( uID, pInfo);
}

void ZPlayerManager::RemovePlayer( MUID& uID)
{
	map<MUID,ZPlayerInfo*>::iterator itr = m_PlayerList.find( uID);

	if ( itr == m_PlayerList.end())
		return;

    
	delete (*itr).second;
	m_PlayerList.erase( itr);
}

void ZPlayerManager::Clear()
{
	while ( !m_PlayerList.empty())
	{
		delete ( *m_PlayerList.begin()).second;
		m_PlayerList.erase( m_PlayerList.begin());
	}
}

ZPlayerInfo* ZPlayerManager::Find( MUID& uID)
{
	map<MUID,ZPlayerInfo*>::iterator itr = m_PlayerList.find( uID);

	if ( itr == m_PlayerList.end())
		return NULL;

	return (*itr).second;
}
