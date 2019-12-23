#include "stdafx.h"
#include "MBMatchAsyncDBJob_GambleItem.h"

void MBMatchAsyncDBJob_GambleItem::Run( void* pContext )
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	if( m_bIsSpendable )
	{
		if( !pDBMgr->ChangeGambleItemToRewardSpendableItem(m_nCID, m_nCIID, 
			m_nGIID, m_nGRIID, m_nRIID, m_nItemCnt, m_nOutCIID) )
		{
			SetResult(MASYNC_RESULT_FAILED);
			return;
		}
	}
	else
	{
		if( !pDBMgr->ChangeGambleItemToRewardNormalItem(m_nCID, m_nCIID, 
			m_nGIID, m_nGRIID, m_nRIID, m_nRentHourPeriod, m_nOutCIID) )
		{
			SetResult(MASYNC_RESULT_FAILED);
			return;
		}
	}
	
	SetResult( MASYNC_RESULT_SUCCEED );
}