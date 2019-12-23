#include "stdafx.h"
#include "MMatchGambleItem.h"
#include "MMatchGambleMachine.h"


const MMatchCharGambleItem* MMatchGambleItemManager::GetGambleItemByUID( const MUID& uidItem ) const
{
	list< MMatchCharGambleItem* >::const_iterator it, end;
	end = m_GambleItemList.end();
	for( it = m_GambleItemList.begin(); it != end; ++it )
	{
		if( uidItem == (*it)->GetUID()  )
			return (*it);
	}

	return NULL;

}

const MMatchCharGambleItem* MMatchGambleItemManager::GetGambleItemByIndex( const DWORD dwIndex ) const
{
	if( m_GambleItemList.size() > dwIndex )
	{
		list< MMatchCharGambleItem* >::const_iterator it = m_GambleItemList.begin();
		for( DWORD i = 0; i < dwIndex; ++i )
			++it;

		return (*it);
	}
		
	return NULL;
}

const MMatchCharGambleItem*	MMatchGambleItemManager::GetGambleItemByCIID( const DWORD CIID ) const
{
	list< MMatchCharGambleItem* >::const_iterator it, end;
	end = m_GambleItemList.end();
	for( it = m_GambleItemList.begin(); it != end; ++it )
	{
		if( CIID == (*it)->GetCIID()  )
			return (*it);
	}

	return NULL;
}

const MMatchCharGambleItem*	MMatchGambleItemManager::GetGambleItemByItemID( const DWORD dwItemID ) const
{
	list< MMatchCharGambleItem* >::const_iterator it, end;
	end = m_GambleItemList.end();
	for( it = m_GambleItemList.begin(); it != end; ++it )
	{
		if( dwItemID == (*it)->GetGambleItemID() )
			return (*it);
	}

	return NULL;
}

const bool MMatchGambleItemManager::AddGambleItem( const MUID& uidItem, const DWORD dwCIID, const DWORD dwGambleItemID, const int nItemCount )
{
	if( NULL != GetGambleItemByUID(uidItem) ) return false;

	MMatchCharGambleItem* pGambleItem = new MMatchCharGambleItem( uidItem, dwCIID, dwGambleItemID, nItemCount );
	if( NULL == pGambleItem ) return false;

	m_GambleItemList.push_back( pGambleItem );
	return true;
}

void MMatchGambleItemManager::DeleteGambleItem( const MUID& uidItem )
{
	list< MMatchCharGambleItem* >::iterator it, end;
	end = m_GambleItemList.end();
	for( it = m_GambleItemList.begin(); it != end; ++it )
	{
		if( uidItem == (*it)->GetUID() )
		{
			delete (*it);
			m_GambleItemList.erase( it );
			return;
		}
	}
}

bool MMatchGambleItemManager::SetGambleItemCount( const MUID& uidItem, int nItemCount)
{
	list< MMatchCharGambleItem* >::iterator it, end;
	end = m_GambleItemList.end();
	for( it = m_GambleItemList.begin(); it != end; ++it )
	{
		MMatchCharGambleItem *pGItem = (*it);
		if( uidItem == pGItem->GetUID() ) {			
			pGItem->SetItemCount(nItemCount);
			return true;
		}
	}

	return false;
}

bool MMatchGambleItemManager::SetGambleItemCount( int nCIID, int nItemCount)
{
	list< MMatchCharGambleItem* >::const_iterator it;
	for( it = m_GambleItemList.begin(); it != m_GambleItemList.end(); ++it ) {
		MMatchCharGambleItem *pGItem = (*it);
		if( nCIID == pGItem->GetCIID()  ) {
			pGItem->SetItemCount(nItemCount);
			return true;
		}
	}

	return false;
}

void MMatchGambleItemManager::Release()
{
	list< MMatchCharGambleItem* >::iterator it, end;
	end = m_GambleItemList.end();
	for( it = m_GambleItemList.begin(); it != end; ++it )
	{
		delete (*it);
	}

	m_GambleItemList.clear();
}	
