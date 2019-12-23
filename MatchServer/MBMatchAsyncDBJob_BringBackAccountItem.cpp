#include "stdafx.h"
#include "MBMatchAsyncDBJob_BringBackAccountItem.h"



void MBMatchAsyncDBJob_BringBackAccountItem::Run( void* pContext )
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	if( !pDBMgr->BringBackAccountItem(m_dwAID, m_dwCID, m_dwCIID) )
	{
		SetResult( MASYNC_RESULT_FAILED );
		return;
	}

	SetResult( MASYNC_RESULT_SUCCEED );
}