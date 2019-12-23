#include "stdafx.h"
#include "MAsyncDBJob_DuelTournament.h"

void MAsyncDBJob_GetDuelTournamentTimeStamp::Run(void* pContext)
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	if (!pDBMgr->GetDuelTournamentTimeStamp(m_szTimeStamp) )
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}

void MAsyncDBJob_GetDuelTournamentCharInfo::Run(void* pContext)
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	if (!pDBMgr->GetDuelTournamentCharInfo(m_dwPlayerCID, &m_pDTCharInfo) ) {
		if(!pDBMgr->InsertDuelTournamentCharInfo(m_dwPlayerCID) ) {
			SetResult(MASYNC_RESULT_FAILED);
			return;
		}

		if (!pDBMgr->GetDuelTournamentCharInfo(m_dwPlayerCID, &m_pDTCharInfo) ) {
			SetResult(MASYNC_RESULT_FAILED);
			return;
		};
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}

void MAsyncDBJob_UpdateDuelTournamentCharInfo::Run(void* pContext)
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	if (!pDBMgr->UpdateDuelTournamentCharacterInfo(m_dwPlayerCID, m_szTimeStamp, m_pDTCharInfo))
	{
		SetResult(MASYNC_RESULT_FAILED);;
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}

void MAsyncDBJob_GetDuelTournamentPreviousCharInfo::Run(void* pContext)
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	if (!pDBMgr->GetDuelTournamentPreviousCharacterInfo(m_dwPlayerCID, 
		&m_nPrevTP, &m_nPrevWins, &m_nPrevLoses, &m_nPrevRanking, &m_nPrevFinalWins) )
	{
		SetResult(MASYNC_RESULT_FAILED);;
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}

void MAsyncDBJob_UpdateDuelTournamentGameLog::Run(void* pContext)
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	if (!pDBMgr->UpdateDuelTournamentGameLog(m_szTimeStamp, m_nLogID, m_nChampionCID) )
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}

void MAsyncDBJob_InsertDuelTournamentGameLogDetail::Run(void* pContext)
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	if (!pDBMgr->InsertDuelTournamentGameLogDetail(m_nLogID, m_szTimeStamp, (int)m_nDTRoundState, m_nPlayTime
		, m_nWinnerCID, m_nGainTP, m_nLoserCID, m_nLoseTP) )
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}

void MAsyncDBJob_GetDuelTournamentSideRankingInfo::Run(void* pContext)
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	if (!pDBMgr->GetDuelTournamentSideRankingInfo(m_dwPlayerCID, &m_SideRankingList) )
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}

void MAsyncDBJob_GetDuelTournamentGroupRankingInfo::Run(void* pContext)
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	if (!pDBMgr->GetDuelTournamentGroupRankingInfo(&m_GroupRankingList)) 
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}