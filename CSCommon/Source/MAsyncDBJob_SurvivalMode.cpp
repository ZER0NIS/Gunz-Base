//////////////////////////////////////////////////////////////////////////////////////////////
// 2009. 6. 3 - Added By Hong KiJu

#include "stdafx.h"
#include "MAsyncDBJob_SurvivalMode.h"

void MAsyncDBJob_InsertSurvivalModeGameLog::Run(void* pContext)
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	if (!pDBMgr->InsertSurvivalModeGameLog(m_szGameName, m_dwScenarioID, m_dwTotalRound, 
											m_dwMasterPlayerCID, m_dwMasterPlayerRankPoint, 
											m_dwPlayer2CID, m_dwPlayer2RankPoint, 
											m_dwPlayer3CID,	m_dwPlayer3RankPoint, 
											m_dwPlayer4CID, m_dwPlayer4RankPoint, 
											m_dwGamePlayTime) )
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}


void MAsyncDBJob_GetSurvivalModeGroupRanking::Run(void* pContext)
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	if (!pDBMgr->GetSurvivalModeGroupRanking(m_RankingVec) )
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}

void MAsyncDBJob_GetSurvivalModePrivateRanking::Run(void* pContext)
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	if (!pDBMgr->GetSurvivalModePrivateRanking(m_dwCID, m_RankingInfo) )
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}