#include "stdafx.h"
#include "MBMatchAsyncDBJob_BringAccountItemStackable.h"

void MBMatchAsyncDBJob_BringAccountItemStackable::Run(void* pContext)
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	if (!pDBMgr->BringAccountItemStackable(m_nAID,
		m_nCID, 
		m_nAIID,
		m_nCIID,
		m_nItemCnt,
		&m_noutCIID, 
		&m_noutItemID, 
		&m_boutIsRentItem, 
		&m_noutRentMinutePeriodRemainder,
		&m_noutItemCnt,
		m_woutRentHourPeriod))
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}


bool MBMatchAsyncDBJob_BringAccountItemStackable::Input(unsigned int nAID, unsigned int nCID, 
														unsigned int nAIID, unsigned int nCIID, unsigned int nCount)
{
	m_nAID = nAID;
	m_nCID = nCID;
	m_nAIID = nAIID;
	m_nCIID = nCIID;

	m_nItemCnt = nCount;

	return true;
}