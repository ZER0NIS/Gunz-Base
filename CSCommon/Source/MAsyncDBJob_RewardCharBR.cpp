#include "stdafx.h"
#include "MAsyncDBJob_RewardCharBR.h"

bool MAsyncDBJob_RewardCharBR::Input(int nCID, int nBRID, int nBRTID, int nRewardCount, int nBattleTime, int nKillCount, int nItemID, int nItemCnt, int nRentHourPeriod, bool bIsSpendable)
{
	m_nCID = nCID;

	m_nBRID = nBRID;
	m_nBRTID = nBRTID;

	m_nRewardCount = nRewardCount;

	m_nBattleTime = nBattleTime;
	m_nKillCount = nKillCount;

	m_nItemID = nItemID;
	m_nItemCnt = nItemCnt;

	m_nRentHourPeriod = nRentHourPeriod;

	m_bIsSpendable = bIsSpendable;

	return true;
}

void MAsyncDBJob_RewardCharBR::Run(void* pContext)
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	if( !pDBMgr->RewardCharBattleTimeReward(m_nCID, m_nBRID, m_nBRTID, m_nBattleTime, m_nKillCount, m_nItemID, m_nItemCnt, m_nRentHourPeriod, m_bIsSpendable, &m_nCIID) )
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}