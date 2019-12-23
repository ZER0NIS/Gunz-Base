#include "stdafx.h"
#include "ZMyGambleItem.h"
#include "ZGambleItemDefine.h"


ZMyGambleItem::ZMyGambleItem( const MUID& uidItem, const DWORD dwGambleItemID, const int nItemCount ) 
	: MMatchBaseGambleItem( uidItem, dwGambleItemID, nItemCount )
{
}


ZMyGambleItem::~ZMyGambleItem()
{
}


const ZGambleItemDefine* ZMyGambleItem::GetDesc() const
{
	return ZGetGambleItemDefineMgr().GetGambleItemDefine( GetGambleItemID() );
}

/////////////////////////////////////////////////////////////////////////////////////////


ZMyGambleItemManager::~ZMyGambleItemManager()
{
	Release();
}


bool ZMyGambleItemManager::CreateGambleItem( const MUID& uidItem, const DWORD dwGambleItemID, const int nitemCount )
{
	ZMyGambleItem* pGItem = new ZMyGambleItem( uidItem, dwGambleItemID, nitemCount );
	if( NULL == pGItem ) return false;

	m_GambleItemList.push_back( pGItem );
	return true;
}


const ZMyGambleItem* ZMyGambleItemManager::GetGambleItem( const MUID& uidItem )
{
	vector< ZMyGambleItem* >::iterator it, end;
	end = m_GambleItemList.end();
	for( it = m_GambleItemList.begin(); it != end; ++it )
	{
		if( uidItem == (*it)->GetUID() )
			return (*it);
	}

	return NULL;
}


const ZMyGambleItem* ZMyGambleItemManager::GetGambleItemByIndex( const DWORD dwIndex )
{
	if( m_GambleItemList.size() > dwIndex )
		return m_GambleItemList[ dwIndex ];
	return NULL;
}


void ZMyGambleItemManager::DeleteGambleItem( const MUID& uidItem )
{
	vector< ZMyGambleItem* >::iterator it, end;
	end = m_GambleItemList.end();
	for( it = m_GambleItemList.begin(); it != end; ++it )
	{
		if( uidItem == (*it)->GetUID() )
		{
			delete (*it);
			m_GambleItemList.erase( it );
		}
	}
}


void ZMyGambleItemManager::Release()
{
	vector< ZMyGambleItem* >::iterator it, end;
	end = m_GambleItemList.end();
	for( it = m_GambleItemList.begin(); it != end; ++it )
		delete (*it);
	m_GambleItemList.clear();
}