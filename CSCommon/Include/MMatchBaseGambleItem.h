#ifndef _MMATCHBASEGAMBLEITEM_H
#define _MMATCHBASEGAMBLEITEM_H

// 클라이언트와 서버가 겜블아이템에 대해서 공통으로 사용하는 데이터.
class MMatchBaseGambleItem
{
protected :
	MUID	m_uidItem;
	DWORD	m_dwGambleItemID;
	int		m_nItemCount;
	
protected :
	MMatchBaseGambleItem() {}

public :
	MMatchBaseGambleItem( const MUID& uidItem, const DWORD dwGambleItemID, const int nItemCount = 1) 
	{
		m_uidItem		 = uidItem;
		m_dwGambleItemID = dwGambleItemID;
		m_nItemCount	 = nItemCount;
	}

	virtual ~MMatchBaseGambleItem() {}

	const MUID& GetUID() const			{ return m_uidItem; }
	const DWORD GetGambleItemID() const { return m_dwGambleItemID; }
	const int	GetItemCount() const	{ return m_nItemCount; }

	void SetItemCount(int nVal)	{ m_nItemCount = nVal; }
};

#endif