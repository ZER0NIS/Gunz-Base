#include "stdafx.h"
#include "MBMatchAsyncDBJob_BringBackAccountItemStackable.h"

void MBMatchAsyncDBJob_BringBackAccountItemStackable::Run( void* pContext )
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	if( !pDBMgr->BringBackAccountItemStackable(m_dwAID, m_dwCID, m_dwCIID, m_dwItemCnt) )
	{
		SetResult( MASYNC_RESULT_FAILED );
		return;
	}

	SetResult( MASYNC_RESULT_SUCCEED );
}