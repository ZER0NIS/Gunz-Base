#include "stdafx.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MMatchObject.h"
#include "MMatchItem.h"
#include "MAgentObject.h"
#include "MMatchNotify.h"
#include "Msg.h"
#include "MMatchObjCache.h"
#include "MMatchStage.h"
#include "MMatchTransDataType.h"
#include "MMatchFormula.h"
#include "MMatchConfig.h"
#include "MCommandCommunicator.h"
#include "MMatchShop.h"
#include "MMatchTransDataType.h"
#include "MDebug.h"
#include "MMatchAuth.h"
#include "MMatchStatus.h"
#include "MAsyncDBJob.h"
#include "../MAsyncDBJob_GetAccountItemList.h"

bool MMatchServer::DistributeZItem(const MUID& uidPlayer, const unsigned long int nItemID, bool bRentItem, int nRentPeriodHour, int nItemCount)
{
	MMatchObject* pObject = GetObject(uidPlayer);
	if (!IsEnabledObject(pObject)) return false;

	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc == NULL) return false;

	if (!pItemDesc->IsSpendableItem())
	{
		unsigned long int nNewCIID = 0;
		if (!m_MatchDBMgr.InsertDistributeItem(pObject->GetCharInfo()->m_nCID, nItemID, bRentItem, nRentPeriodHour, &nNewCIID)) return false;

		int nRentMinutePeriodRemainder = nRentPeriodHour * 60;

		MUID uidNew = MMatchItemMap::UseUID();
		pObject->GetCharInfo()->m_ItemList.CreateItem(uidNew, nNewCIID, nItemID, bRentItem, nRentMinutePeriodRemainder, nRentPeriodHour);
	}
	else
	{
		return false;
	}

	return true;
}

bool MMatchServer::RemoveExpiredCharItem(MMatchObject* pObject, MUID& uidItem)
{
	MMatchItem* pItem = pObject->GetCharInfo()->m_ItemList.GetItem(uidItem);
	if (pItem == NULL) return false;

	if (!m_MatchDBMgr.DeleteExpiredCharItem(pObject->GetCharInfo()->m_nCID, pItem->GetCIID())) {
		return false;
	}

	MMatchCharItemParts nCheckParts = MMCIP_END;
	if (pObject->GetCharInfo()->m_EquipedItem.IsEquipedItem(uidItem, nCheckParts)) {
		pObject->GetCharInfo()->m_EquipedItem.Remove(nCheckParts);
	}

	pObject->GetCharInfo()->m_ItemList.RemoveItem(uidItem);

	return true;
}

void MMatchServer::OnRequestAccountItemList(const MUID& uidPlayer)
{
	ResponseAccountItemList(uidPlayer);
}
void MMatchServer::ResponseAccountItemList(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL) || (pObj->GetAccountInfo() == NULL)) return;

	MAsyncDBJob_GetAccountItemList* pGetAccountItemListJob = new MAsyncDBJob_GetAccountItemList(uidPlayer);
	if (NULL == pGetAccountItemListJob) return;

	pGetAccountItemListJob->Input(uidPlayer, pObj->GetAccountInfo()->m_nAID);
	pObj->m_DBJobQ.DBJobQ.push_back(pGetAccountItemListJob);
}

void MMatchServer::OnRequestUseSpendableNormalItem(const MUID& uidPlayer, const MUID& uidItem)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if (pCharInfo == NULL) return;

	MMatchItem* pItem = pCharInfo->m_ItemList.GetItem(uidItem);
	if (pItem == NULL) {
		mlog("Use Spendable Item Failed[CID : %d, MUID(%d%d)]\n", pCharInfo->m_nCID, uidItem.High, uidItem.Low);
		return;
	}

	if (pItem->GetDesc()->IsSpendableItem())
	{
		UseSpendableItem(uidPlayer, uidItem);
	}
}

void MMatchServer::UseSpendableItem(const MUID& uidPlayer, const MUID& uidItem)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if (pCharInfo == NULL) return;

	MMatchItem* pItem = pCharInfo->m_ItemList.GetItem(uidItem);
	if (pItem == NULL) {
		mlog("Use Spendable Item Failed[CID : %d, MUID(%d%d)]\n", pCharInfo->m_nCID, uidItem.High, uidItem.Low);
		return;
	}

	if (pItem->GetItemCount() > 0)
	{
		pItem->DecreaseCountWithCaching(1);

#ifdef _DEBUG
		mlog("Item UID(%d%d)가 사용되었습니다. 아이템 항목 삭제\n", uidItem.High, uidItem.Low);
#endif
	}
	else {
		mlog("Item UID(%d%d)가 비정상적으로 사용되었습니다.\n", uidItem.High, uidItem.Low);
	}
}