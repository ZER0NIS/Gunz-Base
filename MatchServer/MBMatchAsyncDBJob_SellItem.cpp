#include "StdAfx.h"
#include "MBMatchAsyncDBJob_SellItem.h"

void MBMatchAsyncDBJob_SellItem::Run( void* pContext )
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	bool bResult = false;

	switch( m_nJobID )
	{
	case MASYNCJOB_SELL_ITEM_TO_BOUNTY:	
		{
			bResult = pDBMgr->SellingItemToBounty(m_nCID, m_nCIID, m_nSellItemID, m_nSellPrice, m_CharBP);
		}
		break;

	case MASYNCJOB_SELL_SPENDABLEITEM_TO_BOUNTY:
		{
			bResult = pDBMgr->SellingSpendableItemToBounty(m_nCID, m_nCIID, m_nSellItemID, m_nSellCount, m_nSellPrice, m_CharBP);
		}
		break;
	}

	if( bResult )	SetResult( MASYNC_RESULT_SUCCEED );
	else			SetResult( MASYNC_RESULT_FAILED );
}