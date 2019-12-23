#include "stdafx.h"
#include "MBMatchAsyncDBJob_GambleQuestItem.h"

void MBMatchAsyncDBJob_GambleQuestItem::Run( void* pContext )
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;
	
	// 자신의 인벤토리에 존재하는 아이템인지 확인한다. //
	MMatchObject* pObj = MGetMatchServer()->GetObject(m_uidPlayer);
	if (pObj == NULL) { return; }

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if (pCharInfo == NULL) { return; }

	// 유저가 가지고 있는 아이템 고유 아이디로 해당 아이템 찾기
	const MMatchCharGambleItem* pGambleItem = pCharInfo->m_GambleItemManager.GetGambleItemByUID(m_uidGItem);
	if (pGambleItem == NULL) { return; }


	if( !pDBMgr->UpdateQuestItem(pCharInfo->m_nCID, pCharInfo->m_QuestItemList, pCharInfo->m_QMonsterBible)  )
	{
		SetResult( MASYNC_RESULT_FAILED );
		return;
	}

	if( !pDBMgr->UpdateCharGambleItemCount(pGambleItem->GetCIID(), pGambleItem->GetGambleItemID()) )
	{
		SetResult( MASYNC_RESULT_FAILED );
		return;
	}

	SetResult( MASYNC_RESULT_SUCCEED );
}