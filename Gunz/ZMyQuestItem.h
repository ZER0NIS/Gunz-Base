#ifndef _ZMYQUESTITEM_H
#define _ZMYQUESTITEM_H


#include "MQuestItem.h"

#include <map>

using std::map;


#define MAX_ZQUEST_ITEM_COUNT 99
#define MIN_ZQUEST_ITEM_COUNT 0

// Description은 MQuestItem에 있는걸 사용.
class ZMyQuestItemNode : MBaseQuestItem
{
public :
	ZMyQuestItemNode() : m_nCount( 0 ), m_nItemID( 0 )
	{
	}

	virtual ~ZMyQuestItemNode()
	{
	}

	unsigned long int	GetItemID()	{ return m_nItemID; }
	int					GetCount()	{ return m_nCount; }
	MQuestItemDesc*		GetDesc()	{ return m_pDesc; }

	void Increase( const int nCount = 1 ) 
	{
		m_nCount += nCount;
		if( m_nCount >= MAX_ZQUEST_ITEM_COUNT )
			m_nCount = MAX_ZQUEST_ITEM_COUNT;

	}

	void Decrease( const int nCount = 1 )
	{
		m_nCount -= nCount;
		if( MIN_ZQUEST_ITEM_COUNT > m_nCount )
			m_nCount = MIN_ZQUEST_ITEM_COUNT;
	}

	void SetItemID( const unsigned long int nItemID )	{ m_nItemID = nItemID; }
	void SetCount( const int nCount )					{ m_nCount = nCount; }
	void SetDesc( MQuestItemDesc* pDesc )				{ m_pDesc = pDesc; }

	void Create( const unsigned long int nItemID, const int nCount, MQuestItemDesc* pDesc )
	{
		m_nItemID	= nItemID;
		m_nCount	= nCount;
		m_pDesc		= pDesc;
	}

public :
	unsigned long int	m_nItemID;
	int					m_nCount;
	MQuestItemDesc*		m_pDesc;
};


class ZMyQuestItemMap : public map< unsigned long int, ZMyQuestItemNode* >
{
public :
	ZMyQuestItemMap();
	~ZMyQuestItemMap();
	
	bool Add( const unsigned long int nItemID, ZMyQuestItemNode* pQuestItem );

	bool CreateQuestItem( const unsigned long int nItemID, const int nCount, MQuestItemDesc* pDesc );

	void Clear();

	ZMyQuestItemNode* Find( const unsigned long int nItemID )
	{
		ZMyQuestItemMap::iterator It;
		It = find( nItemID );
		if( end() != It )
		{
			return It->second;
		}

		return 0;
	}

private :

};

typedef map<unsigned long int, ZMyQuestItemNode*> MQUESTITEMNODEMAP;

#endif