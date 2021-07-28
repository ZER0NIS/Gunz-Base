#include "stdafx.h"
#include "MMatchServer.h"
#include "MMatchStage.h"
#include "MMatchRule.h"
#include "MMatchRuleQuest.h"
#include "MMatchGameType.h"
#include "MMatchConfig.h"
#include "MBlobArray.h"
#include "MMatchShop.h"
#include "MAsyncDBJob_BuyQuestItem.h"

void MMatchServer::OnRequestNPCDead(const MUID& uidSender, const MUID& uidKiller, MUID& uidNPC, MVector& pos)
{
	MMatchObject* pSender = GetObject(uidSender);
	if (!IsEnabledObject(pSender)) { ASSERT(0); return; }

	MMatchStage* pStage = FindStage(pSender->GetStageUID());
	if (pStage == NULL) { ASSERT(0); return; }

	MMatchCharBattleTimeRewardInfoMap::iterator iter = pSender->GetCharInfo()->GetBRInfoMap().begin();
	for (; iter != pSender->GetCharInfo()->GetBRInfoMap().end(); iter++) {
		MMatchCharBRInfo* pInfo = iter->second;
		pInfo->AddKillCount(1);
	}

	if (MGetGameTypeMgr()->IsQuestDerived(pStage->GetStageSetting()->GetGameType()))
	{
		MMatchRule* pRule = pStage->GetRule();
		if (NULL == pRule) return;
		if (false == MGetGameTypeMgr()->IsQuestDerived(pRule->GetGameType())) return;

		MMatchRuleBaseQuest* pQuestRule = reinterpret_cast<MMatchRuleBaseQuest*>(pRule);

		pQuestRule->OnRequestNPCDead((MUID&)uidSender, (MUID&)uidKiller, uidNPC, pos);
	}
	else
	{
		ASSERT(0);
	}
}

void MMatchServer::OnQuestRequestDead(const MUID& uidVictim)
{
	MMatchObject* pVictim = GetObject(uidVictim);
	if (pVictim == NULL) return;

	MMatchStage* pStage = FindStage(pVictim->GetStageUID());
	if (pStage == NULL) return;

	if (!MGetGameTypeMgr()->IsQuestDerived(pStage->GetStageSetting()->GetGameType())) return;

	MMatchRuleBaseQuest* pQuestRule = (MMatchRuleBaseQuest*)pStage->GetRule();
	pQuestRule->OnRequestPlayerDead((MUID&)uidVictim);

	if (pVictim->CheckAlive() == false) {
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SUICIDE, MUID(0, 0));
		int nResult = MOK;
		pNew->AddParameter(new MCommandParameterInt(nResult));
		pNew->AddParameter(new MCommandParameterUID(pVictim->GetUID()));
		RouteToBattle(pStage->GetUID(), pNew);
		return;
	}

	pVictim->OnDead();

	MCommand* pCmd = CreateCommand(MC_MATCH_QUEST_PLAYER_DEAD, MUID(0, 0));
	pCmd->AddParameter(new MCommandParameterUID(uidVictim));
	RouteToBattle(pStage->GetUID(), pCmd);
}

void MMatchServer::OnQuestTestRequestNPCSpawn(const MUID& uidPlayer, int nNPCType, int nNPCCount)
{
	MMatchObject* pPlayer = GetObject(uidPlayer);
	if (pPlayer == NULL) return;
	MMatchStage* pStage = FindStage(pPlayer->GetStageUID());
	if (pStage == NULL) return;

	if (!IsAdminGrade(pPlayer)) return;

	if (MGetGameTypeMgr()->IsQuestDerived(pStage->GetStageSetting()->GetGameType()))
	{
		MMatchRuleBaseQuest* pQuestRule = (MMatchRuleBaseQuest*)pStage->GetRule();
		pQuestRule->OnRequestTestNPCSpawn(nNPCType, nNPCCount);
	}
}

void MMatchServer::OnQuestTestRequestClearNPC(const MUID& uidPlayer)
{
	MMatchObject* pPlayer = GetObject(uidPlayer);
	if (pPlayer == NULL) return;
	MMatchStage* pStage = FindStage(pPlayer->GetStageUID());
	if (pStage == NULL) return;

	if (!IsAdminGrade(pPlayer)) return;

	if (MGetGameTypeMgr()->IsQuestDerived(pStage->GetStageSetting()->GetGameType()))
	{
		MMatchRuleBaseQuest* pQuestRule = (MMatchRuleBaseQuest*)pStage->GetRule();
		pQuestRule->OnRequestTestClearNPC();
	}
}

void MMatchServer::OnQuestTestRequestSectorClear(const MUID& uidPlayer)
{
	MMatchObject* pPlayer = GetObject(uidPlayer);
	if (pPlayer == NULL) return;
	MMatchStage* pStage = FindStage(pPlayer->GetStageUID());
	if (pStage == NULL) return;

	if (!IsAdminGrade(pPlayer)) return;

	if (MGetGameTypeMgr()->IsQuestDerived(pStage->GetStageSetting()->GetGameType()))
	{
		MMatchRuleBaseQuest* pQuestRule = (MMatchRuleBaseQuest*)pStage->GetRule();
		pQuestRule->OnRequestTestSectorClear();
	}
}

void MMatchServer::OnQuestTestRequestQuestFinish(const MUID& uidPlayer)
{
	MMatchObject* pPlayer = GetObject(uidPlayer);
	if (pPlayer == NULL) return;
	MMatchStage* pStage = FindStage(pPlayer->GetStageUID());
	if (pStage == NULL) return;

	if (!IsAdminGrade(pPlayer)) return;

	if (MGetGameTypeMgr()->IsQuestDerived(pStage->GetStageSetting()->GetGameType()))
	{
		MMatchRuleBaseQuest* pQuestRule = (MMatchRuleBaseQuest*)pStage->GetRule();
		pQuestRule->OnRequestTestFinish();
	}
}

void MMatchServer::OnQuestRequestMovetoPortal(const MUID& uidPlayer)
{
	MMatchObject* pPlayer = GetObject(uidPlayer);
	if (pPlayer == NULL) return;
	MMatchStage* pStage = FindStage(pPlayer->GetStageUID());
	if (pStage == NULL) return;

	if (false == MGetGameTypeMgr()->IsQuestDerived(pStage->GetStageSetting()->GetGameType())) return;

	MMatchRuleBaseQuest* pQuestRule = (MMatchRuleBaseQuest*)pStage->GetRule();
	pQuestRule->OnRequestMovetoPortal(uidPlayer);
}

void MMatchServer::OnQuestReadyToNewSector(const MUID& uidPlayer)
{
	MMatchObject* pPlayer = GetObject(uidPlayer);
	if (pPlayer == NULL) return;
	MMatchStage* pStage = FindStage(pPlayer->GetStageUID());
	if (pStage == NULL) return;

	if (false == MGetGameTypeMgr()->IsQuestDerived(pStage->GetStageSetting()->GetGameType())) return;

	MMatchRuleBaseQuest* pQuestRule = (MMatchRuleBaseQuest*)pStage->GetRule();
	pQuestRule->OnReadyToNewSector(uidPlayer);
}

void MMatchServer::OnRequestCharQuestItemList(const MUID& uidSender)
{
	OnResponseCharQuestItemList(uidSender);
}

void MMatchServer::OnResponseCharQuestItemList(const MUID& uidSender)
{
	MMatchObject* pPlayer = GetObject(uidSender);
	if (!IsEnabledObject(pPlayer))
		return;

	if (!pPlayer->GetCharInfo()->m_QuestItemList.IsDoneDbAccess())
	{
		if (!m_MatchDBMgr.GetCharQuestItemInfo(pPlayer->GetCharInfo()))
		{
			mlog("DB Query(ResponseCharacterItemList > GetcharQuestItemInfo) failed\n");
			return;
		}
	}

	MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_CHAR_QUEST_ITEM_LIST, MUID(0, 0));
	if (0 == pNewCmd)
	{
		mlog("MMatchServer::OnResponseCharQuestItemList - Command생성 실패.\n");
		return;
	}

	int					nIndex = 0;
	MTD_QuestItemNode* pQuestItemNode = 0;
	void* pQuestItemArray = MMakeBlobArray(static_cast<int>(sizeof(MTD_QuestItemNode)),
		static_cast<int>(pPlayer->GetCharInfo()->m_QuestItemList.size()));

	MQuestItemMap::iterator itQItem, endQItem;
	endQItem = pPlayer->GetCharInfo()->m_QuestItemList.end();
	for (itQItem = pPlayer->GetCharInfo()->m_QuestItemList.begin(); itQItem != endQItem; ++itQItem)
	{
		pQuestItemNode = reinterpret_cast<MTD_QuestItemNode*>(MGetBlobArrayElement(pQuestItemArray, nIndex++));
		Make_MTDQuestItemNode(pQuestItemNode, itQItem->second->GetItemID(), itQItem->second->GetCount());
	}

	pNewCmd->AddParameter(new MCommandParameterBlob(pQuestItemArray, MGetBlobArraySize(pQuestItemArray)));
	MEraseBlobArray(pQuestItemArray);

	RouteToListener(pPlayer, pNewCmd);
}

void MMatchServer::OnRequestBuyQuestItem(const MUID& uidSender, const unsigned long int nItemID, const int nItemCount)
{
	OnResponseBuyQuestItem(uidSender, nItemID, nItemCount);
}
void MMatchServer::OnResponseBuyQuestItem(const MUID& uidSender, const unsigned long int nItemID, const int nItemCount)
{
	MMatchObject* pPlayer = GetObject(uidSender);
	if (!IsEnabledObject(pPlayer)) return;

	MQuestItemDescManager::iterator itQItemDesc = GetQuestItemDescMgr().find(nItemID);
	if (GetQuestItemDescMgr().end() == itQItemDesc) {
		mlog("MMatchServer::OnResponseBuyQuestItem - %d아이템 description을 찾지 못했습니다.\n", nItemID);
		return;
	}

	MQuestItemDesc* pQuestItemDesc = itQItemDesc->second;
	if (0 == pQuestItemDesc) {
		mlog("MMatchServer::OnRequestBuyQuestItem - %d의 item description이 비정상적입니다.\n", nItemID);
		return;
	}

	if (!MGetMatchShop()->IsSellItem(pQuestItemDesc->m_nItemID)) {
		mlog("MMatchServer::OnRequestBuyQuestItem - %d는 상점에서 판매되고 있는 아이템이 아님.\n", pQuestItemDesc->m_nItemID);
		return;
	}

	if (pPlayer->GetCharInfo()->m_nBP < (itQItemDesc->second->m_nPrice * nItemCount)) {
		MCommand* pBPLess = CreateCommand(MC_MATCH_RESPONSE_BUY_QUEST_ITEM, MUID(0, 0));
		pBPLess->AddParameter(new MCmdParamInt(MERR_TOO_EXPENSIVE_BOUNTY));
		pBPLess->AddParameter(new MCmdParamInt(pPlayer->GetCharInfo()->m_nBP));
		RouteToListener(pPlayer, pBPLess);
		return;
	}

	MQuestItemMap::iterator itQItem = pPlayer->GetCharInfo()->m_QuestItemList.find(nItemID);
	if (pPlayer->GetCharInfo()->m_QuestItemList.end() != itQItem)
	{
		if (MAX_QUEST_ITEM_COUNT > itQItem->second->GetCount() + nItemCount) {
			itQItem->second->Increase(nItemCount);
		}
		else {
			MCommand* pTooMany = CreateCommand(MC_MATCH_RESPONSE_BUY_QUEST_ITEM, MUID(0, 0));
			pTooMany->AddParameter(new MCmdParamInt(MERR_TOO_MANY_ITEM));
			pTooMany->AddParameter(new MCmdParamInt(pPlayer->GetCharInfo()->m_nBP));
			RouteToListener(pPlayer, pTooMany);
			return;
		}
	}
	else {
		MQuestItem* pNewQuestItem = new MQuestItem;
		if (0 == pNewQuestItem) {
			mlog("MMatchServer::OnResponseBuyQuestItem - 새로운 퀘스트 아이템 생성 실패.\n");
			return;
		}

		if (!pNewQuestItem->Create(nItemID, nItemCount, GetQuestItemDescMgr().FindQItemDesc(nItemID))) {
			delete pNewQuestItem;
			mlog("MMatchServer::OnResponseBuyQeustItem - %d번호 아이템 Create( ... )함수 호출 실패.\n");
			return;
		}

		pPlayer->GetCharInfo()->m_QuestItemList.insert(MQuestItemMap::value_type(nItemID, pNewQuestItem));
	}

	UpdateCharDBCachingData(pPlayer);

	MAsyncDBJob_BuyQuestItem* pBuyQuestItemJob = new MAsyncDBJob_BuyQuestItem(uidSender);
	if (NULL == pBuyQuestItemJob) { return; }
	pBuyQuestItemJob->Input(uidSender, pPlayer->GetCharInfo()->m_nCID, nItemCount, pQuestItemDesc->m_nPrice);
	pPlayer->m_DBJobQ.DBJobQ.push_back(pBuyQuestItemJob);
}

void MMatchServer::OnRequestSellQuestItem(const MUID& uidSender, const unsigned long int nItemID, const int nCount)
{
	OnResponseSellQuestItem(uidSender, nItemID, nCount);
}

void MMatchServer::OnResponseSellQuestItem(const MUID& uidSender, const unsigned long int nItemID, const int nCount)
{
	MMatchObject* pPlayer = GetObject(uidSender);
	if (!IsEnabledObject(pPlayer))
	{
		mlog("MMatchServer::OnResponseSellQuestItem - find user fail.\n");
		return;
	}

	MQuestItemDescManager::iterator itQItemDesc = GetQuestItemDescMgr().find(nItemID);
	if (GetQuestItemDescMgr().end() == itQItemDesc)
	{
		mlog("MMatchServer::OnResponseSellQuestItem - find item(%u) description fail.\n", nItemID);
		return;
	}

	MQuestItemDesc* pQItemDesc = itQItemDesc->second;
	if (0 == pQItemDesc)
	{
		mlog("MMatchServer::OnResponseSellQuestItem - item(%u) description is null point.\n", nItemID);
		return;
	}

	MQuestItemMap::iterator itQItem = pPlayer->GetCharInfo()->m_QuestItemList.find(nItemID);
	if (pPlayer->GetCharInfo()->m_QuestItemList.end() != itQItem) {
		if (nCount > itQItem->second->GetCount()) {
			return;
		}

		UpdateCharDBCachingData(pPlayer);
		int nPrice = (pQItemDesc->GetSellBountyValue(nCount));
		if (!m_MatchDBMgr.UpdateCharBP(pPlayer->GetCharInfo()->m_nCID, nPrice)) {
			return;
		}

		itQItem->second->Decrease(nCount);
		pPlayer->GetCharInfo()->m_nBP += nPrice;
	}
	else {
		mlog("MMatchServer::OnResponseSellQuestItem - user is not owner. itemid(%u)\n", nItemID);
		ASSERT(0);
		return;
	}

	pPlayer->GetCharInfo()->GetDBQuestCachingData().IncreaseShopTradeCount();

	MCommand* pCmd = CreateCommand(MC_MATCH_RESPONSE_SELL_QUEST_ITEM, MUID(0, 0));
	if (0 == pCmd) {
		return;
	}

	pCmd->AddParameter(new MCmdParamInt(MOK));
	pCmd->AddParameter(new MCmdParamInt(pPlayer->GetCharInfo()->m_nBP));
	RouteToListener(pPlayer, pCmd);

	OnRequestCharQuestItemList(pPlayer->GetUID());
}

void MMatchServer::OnRequestDropSacrificeItemOnSlot(const MUID& uidSender, const int nSlotIndex, const unsigned long int nItemID)
{
	MMatchObject* pPlayer = GetObject(uidSender);
	if (!IsEnabledObject(pPlayer))
	{
		mlog("MMatchServer::OnRequestDropSacrificeItemOnSlot - invalid user.\n");
		return;
	}

	MMatchStage* pStage = FindStage(pPlayer->GetStageUID());
	if (0 != pStage)
	{
		if (STAGE_STATE_RUN == pStage->GetState())
			return;

		const MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
		if (0 == pNode)
		{
			mlog("MMatchServer::OnRequestDropSacrificeItemOnSlot - find stage fail.\n");
			return;
		}

		if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
		{
			MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast<MMatchRuleBaseQuest*>(pStage->GetRule());
			if (0 == pRuleQuest)
				return;

			pRuleQuest->OnRequestDropSacrificeItemOnSlot(uidSender, nSlotIndex, nItemID);
		}
	}
}

void MMatchServer::OnRequestCallbackSacrificeItem(const MUID& uidSender, const int nSlotIndex, const unsigned long int nItemID)
{
	MMatchObject* pPlayer = GetObject(uidSender);
	if (!IsEnabledObject(pPlayer))
	{
		mlog("MMatchServer::OnRequestDropSacrificeItemOnSlot - invalid user.\n");
		return;
	}

	MMatchStage* pStage = FindStage(pPlayer->GetStageUID());
	if (0 != pStage)
	{
		const MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
		if (0 == pNode)
		{
			mlog("MMatchServer::OnRequestCallbackSacrificeItem - find stage fail.\n");
			return;
		}

		if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
		{
			MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast<MMatchRuleBaseQuest*>(pStage->GetRule());
			if (0 == pRuleQuest)
				return;

			pRuleQuest->OnRequestCallbackSacrificeItem(uidSender, nSlotIndex, nItemID);
		}
	}
}

void MMatchServer::OnRequestQL(const MUID& uidSender)
{
	MMatchObject* pPlayer = GetObject(uidSender);
	if (!IsEnabledObject(pPlayer))
	{
		mlog("MMatchServer::OnRequestQL - invlaid user.\n");
		return;
	}

	MMatchStage* pStage = FindStage(pPlayer->GetStageUID());
	if (0 != pStage)
	{
		const MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
		if (0 == pNode)
		{
			mlog("MMatchServer::OnRequestQL - find stage fail.\n");
			return;
		}

		if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
		{
			MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast<MMatchRuleBaseQuest*>(pStage->GetRule());
			if (0 == pRuleQuest)
				return;

			pRuleQuest->OnRequestQL(uidSender);
		}
	}
}

void MMatchServer::OnRequestSacrificeSlotInfo(const MUID& uidSender)
{
	MMatchObject* pPlayer = GetObject(uidSender);
	if (!IsEnabledObject(pPlayer))
	{
		mlog("MMatchServer::OnRequestSacrificeSlotInfo - invalid user.\n");
		return;
	}

	MMatchStage* pStage = FindStage(pPlayer->GetStageUID());
	if (0 != pStage)
	{
		const MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
		if (0 == pNode)
		{
			mlog("MMatchServer::OnRequestSacrificeSlotInfo - find stage fail.\n");
			return;
		}

		if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
		{
			MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast<MMatchRuleBaseQuest*>(pStage->GetRule());
			if (0 == pRuleQuest)
				return;

			pRuleQuest->OnRequestSacrificeSlotInfo(uidSender);
		}
	}
}

void MMatchServer::OnQuestStageMapset(const MUID& uidStage, int nMapsetID)
{
}

void MMatchServer::OnRequestMonsterBibleInfo(const MUID& uidSender)
{
	MMatchObject* pPlayer = GetObject(uidSender);
	if (!IsEnabledObject(pPlayer))
	{
		mlog("MMatchServer::OnRequestMonsterBibleInfo - invalid user.\n");
		return;
	}

	OnResponseMonsterBibleInfo(uidSender);
}

void MMatchServer::OnResponseMonsterBibleInfo(const MUID& uidSender)
{
	MMatchObject* pObj = GetObject(uidSender);
	if (!IsEnabledObject(pObj))
		return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if (0 == pCharInfo)
		return;

	if (!pCharInfo->m_QuestItemList.IsDoneDbAccess())
	{
		mlog("MMatchServer::OnResponseMonsterBibleInfo - not load db monsterbible info.\n");
		return;
	}

	void* pMonBibleInfoBlob = MMakeBlobArray(MONSTER_BIBLE_SIZE, 1);
	if (0 == pMonBibleInfoBlob)
	{
		mlog("MMatchServer::OnResponseMonsterBibleInfo - make blob fail.\n");
		return;
	}

	MQuestMonsterBible* pMonBible = reinterpret_cast<MQuestMonsterBible*>(MGetBlobArrayElement(pMonBibleInfoBlob, 0));
	if (0 == pMonBible)
	{
		mlog("MMatchServer::OnResponseMonsterBibleInfo - typecast fail.\n");
		return;
	}

	memcpy(pMonBible, &(pCharInfo->m_QMonsterBible), MONSTER_BIBLE_SIZE);

	MCommand* pCmd = CreateCommand(MC_MATCH_RESPONSE_MONSTER_BIBLE_INFO, MUID(0, 0));
	if (0 == pCmd)
	{
		mlog("MMatchServer::OnResponseMonsterBibleInfo - create command fail.\n");
		return;
	}

	pCmd->AddParameter(new MCmdParamUID(uidSender));
	pCmd->AddParameter(new MCommandParameterBlob(pMonBibleInfoBlob, MGetBlobArraySize(pMonBibleInfoBlob)));

	RouteToListener(pObj, pCmd);

	MEraseBlobArray(pMonBibleInfoBlob);
}

void MMatchServer::OnQuestPong(const MUID& uidSender)
{
	MMatchObject* pObj = GetObject(uidSender);
	if (0 == pObj)
		return;

	pObj->SetQuestLatency(GetGlobalClockCount());
	pObj->m_bQuestRecvPong = true;
}