#include "stdafx.h"
#include "MBMatchAsyncDBJob_UpdateCharLevel.h"



void MBMatchAsyncDBJob_UpdateCharLevel::Run( void* pContext )
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	if( !pDBMgr->UpdateCharLevel(m_dwCID
		, m_nNewLevel
		, m_nBP
		, m_nTotalKillCount
		, m_nTotalDeathCount
		, m_nTotalPlayTimeSec
		, m_bIsLevelUp) )
	{
		SetResult( MASYNC_RESULT_FAILED );
		return;
	}

	SetResult( MASYNC_RESULT_SUCCEED );
}