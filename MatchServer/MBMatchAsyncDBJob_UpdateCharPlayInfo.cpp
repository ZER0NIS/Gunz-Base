#include "stdafx.h"
#include "MBMatchAsyncDBJob_UpdateCharPlayInfo.h"

void MBMatchAsyncDBJob_UpdateCharPlayInfo::Run( void* pContext )
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	if( !pDBMgr->UpdateCharPlayInfo(m_dwAID
									, m_dwCID
									, m_nXP
									, m_nLevel
									, m_nPlayingTimeSec
									, m_nTotalPlayTimeSec
									, m_nTotalKillCount
									, m_nTotalDeathCount
									, m_nBP
									, m_bIsLevelUp ) )
	{
		SetResult( MASYNC_RESULT_FAILED );
		return;
	}

	SetResult( MASYNC_RESULT_SUCCEED );
}