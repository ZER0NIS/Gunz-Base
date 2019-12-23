#ifndef _MMatchGambleItem_H
#define _MMatchGambleItem_H

#include "MMatchBaseGambleItem.h"


#include "MUID.h"


#include <list>

using std::list;



class MMatchCharGambleItem : public MMatchBaseGambleItem
{
private :
	DWORD m_dwCIID;

public :
	MMatchCharGambleItem( const MUID& uidItem, const DWORD dwCIID, const DWORD dwGambleItemID, const int nItemCount) 
		: MMatchBaseGambleItem( uidItem, dwGambleItemID, nItemCount )
	{
		m_dwCIID = dwCIID;
	}

	const DWORD GetCIID() const			{ return m_dwCIID; }
};


class MMatchGambleItemManager
{
private :
	list< MMatchCharGambleItem* > m_GambleItemList;

public :
	MMatchGambleItemManager() {}
	~MMatchGambleItemManager() { Release(); }

	const bool					AddGambleItem( const MUID& uidItem, const DWORD dwCIID, const DWORD dwGambleItemID, const int nItemCount = 1 );

	const DWORD					GetCount() const								{ return static_cast<DWORD>(m_GambleItemList.size()); }

	const MMatchCharGambleItem*	GetGambleItemByUID( const MUID& uidItem ) const;
	const MMatchCharGambleItem*	GetGambleItemByCIID( const DWORD CIID ) const;
	const MMatchCharGambleItem*	GetGambleItemByIndex( const DWORD dwIndex ) const;
	const MMatchCharGambleItem*	GetGambleItemByItemID( const DWORD dwItemID ) const;

	bool						SetGambleItemCount( int nCIID, int nItemCount);
	bool						SetGambleItemCount( const MUID& uidItem, int nItemCount);	
	void						DeleteGambleItem( const MUID& uidItem );
	void						Release();
};






#endif