#include "stdafx.h"
#include "MAsyncDBJob_GetAccountItemList.h"


void MAsyncDBJob_GetAccountItemList::Run( void* pContext )
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	MAccountItemNode ExpiredItemList[MAX_EXPIRED_ACCOUNT_ITEM];
	int nExpiredItemCount = 0;

	// 디비에서 AccountItem을 가져온다
	if (!pDBMgr->GetAccountItemInfo(m_dwAID, m_AccountItems, &m_nItemCount, MAX_ACCOUNT_ITEM
		,ExpiredItemList, &nExpiredItemCount, MAX_EXPIRED_ACCOUNT_ITEM))
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

	m_vExpiredItems.clear();

	// 여기서 중앙은행의 기간만료 아이템이 있는지 체크한다.
	if (nExpiredItemCount > 0) {
		for (int i = 0; i < nExpiredItemCount; i++) {
			// 디비에서 기간만료된 AccountItem을 지운다.
			if (pDBMgr->DeleteExpiredAccountItem(m_dwAID, ExpiredItemList[i].nAIID)) {
				m_vExpiredItems.push_back(ExpiredItemList[i].nItemID);
			}
		}
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}