#include "stdafx.h"
#include "MAsyncDBJob_BuyQuestItem.h"



void MAsyncDBJob_BuyQuestItem::Run( void* pContext )
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	if( !pDBMgr->UpdateCharBP(m_dwCID, -m_nPrice) )
	{
		SetResult( MASYNC_RESULT_FAILED );
		return;
	}

	SetResult( MASYNC_RESULT_SUCCEED );
}