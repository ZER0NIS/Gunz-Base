#include "stdafx.h"
#include "MAsyncDBJob_CharFinalize.h"
#include "MMatchConfig.h"

void MAsyncDBJob_CharFinalize::Run(void* pContext)
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	MASYNC_RESULT nResult = MASYNC_RESULT_SUCCEED;

	if (!pDBMgr->UpdateCharPlayTime(m_nCID, m_nPlayTime)) {
		nResult = MASYNC_RESULT_FAILED;
	}

	if (!pDBMgr->InsertPlayerLog(m_nCID, m_nPlayTime, m_nConnKillCount, m_nConnDeathCount, m_nConnXP, m_nXP)) {
		nResult = MASYNC_RESULT_FAILED;
	}

	if (0 != m_nAID) {
		if (!pDBMgr->UpdateAccountLastLoginTime(m_nAID)) {
			nResult = MASYNC_RESULT_FAILED;
		}
	}

	if (m_QuestItemMap.IsDoneDbAccess())
	{
		if (!pDBMgr->UpdateQuestItem(m_nCID, m_QuestItemMap, m_QuestMonster))
			nResult = MASYNC_RESULT_FAILED;
	}

	SetResult(nResult);
}

bool MAsyncDBJob_CharFinalize::Input(DWORD nAID,
	int nCID,
	unsigned long int nPlayTime,
	int nConnKillCount,
	int nConnDeathCount,
	int nConnXP,
	int nXP,
	MQuestItemMap& rfQuestItemMap,
	MQuestMonsterBible& rfQuestMonster,
	const bool bIsRequestQItemUpdate)
{
	m_nAID = nAID;
	m_nCID = nCID;
	m_nPlayTime = nPlayTime;
	m_nConnKillCount = nConnKillCount;
	m_nConnDeathCount = nConnDeathCount;
	m_nConnXP = nConnXP;
	m_nXP = nXP;

	m_bIsRequestQItemUpdate = bIsRequestQItemUpdate;

	if (bIsRequestQItemUpdate)
	{
		m_QuestItemMap.SetDBAccess(rfQuestItemMap.IsDoneDbAccess());

		MQuestItemMap::iterator	it, end;

		m_QuestItemMap.Clear();

		it = rfQuestItemMap.begin();
		end = rfQuestItemMap.end();
		for (; it != end; ++it)
		{
			MQuestItem* pQuestItem = it->second;
			m_QuestItemMap.CreateQuestItem(pQuestItem->GetItemID(), pQuestItem->GetCount(), pQuestItem->IsKnown());
		}

		m_QuestMonster.Clear();
		m_QuestMonster.Copy(rfQuestMonster.GetData(), sizeof(rfQuestMonster));
	}

	return true;
}