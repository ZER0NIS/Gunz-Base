#include "stdafx.h"
#include "ZMyQuestItem.h"




ZMyQuestItemMap::ZMyQuestItemMap()
{
}


ZMyQuestItemMap::~ZMyQuestItemMap()
{
	// 만약을 위해서.
	Clear();
}



bool ZMyQuestItemMap::Add( const unsigned long int nItemID, ZMyQuestItemNode* pQuestItem )
{
	if( 0 == pQuestItem )
		return false;

	insert( value_type(nItemID, pQuestItem) );

	return true;
}


void ZMyQuestItemMap::Clear()
{
	if( empty() )
		return;

	iterator It, End;

	End = end();
	for( It = begin(); It != End; ++It )
	{
		delete It->second;
	}

	clear();
}


bool ZMyQuestItemMap::CreateQuestItem( const unsigned long int nItemID, const int nCount, MQuestItemDesc* pDesc )
{
	if( (0 > nCount) || (0 == pDesc) )
		return false;

	ZMyQuestItemNode* pQuestItem = new ZMyQuestItemNode;
	if( 0 == pQuestItem )
		return false;

	pQuestItem->Create( nItemID, nCount, pDesc );

	return Add( nItemID, pQuestItem );
}
