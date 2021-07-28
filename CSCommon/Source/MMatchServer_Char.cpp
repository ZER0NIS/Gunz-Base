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
#include "MAsyncDBJob_FriendList.h"
#include "MAsyncDBJob_CharFinalize.h"
#include "MMatchUtil.h"
#include "MMatchRuleBaseQuest.h"
#include "MMatchLocale.h"
#include "MMatchObject.h"

#include "MAsyncDBJob_UpdateAccountLastLoginTime.h"
#include "MAsyncDBJob_UpdateCharBRInfo.h"

void MMatchServer::OnRequestAccountCharInfo(const MUID& uidPlayer, int nCharNum)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MAsyncDBJob_GetAccountCharInfo* pJob = new MAsyncDBJob_GetAccountCharInfo(uidPlayer, pObj->GetAccountInfo()->m_nAID, nCharNum);

	pObj->m_DBJobQ.DBJobQ.push_back(pJob);
}

void MMatchServer::OnRequestSelectChar(const MUID& uidPlayer, const int nCharIndex)
{
	MMatchObject* pObj = GetObject(uidPlayer);

	if ((pObj == NULL) || (pObj->GetAccountInfo()->m_nAID < 0)) return;
	if ((nCharIndex < 0) || (nCharIndex >= MAX_CHAR_COUNT)) return;

	MAsyncDBJob_GetCharInfo* pJob = new MAsyncDBJob_GetCharInfo(uidPlayer, pObj->GetAccountInfo()->m_nAID, nCharIndex);
	pJob->SetCharInfo(new MMatchCharInfo);
	pObj->m_DBJobQ.DBJobQ.push_back(pJob);
}

void MMatchServer::OnRequestDeleteChar(const MUID& uidPlayer, const int nCharIndex, const char* szCharName)
{
	ResponseDeleteChar(uidPlayer, nCharIndex, szCharName);
}

bool MMatchServer::ResponseDeleteChar(const MUID& uidPlayer, const int nCharIndex, const char* szCharName)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetAccountInfo()->m_nAID < 0)) return false;
	if ((nCharIndex < 0) || (nCharIndex >= MAX_CHAR_COUNT)) return false;

	MAsyncDBJob_DeleteChar* pJob = new MAsyncDBJob_DeleteChar(uidPlayer, pObj->GetAccountInfo()->m_nAID, nCharIndex, szCharName);
	pObj->m_DBJobQ.DBJobQ.push_back(pJob);
	return true;
}

void MMatchServer::OnRequestCreateChar(const MUID& uidPlayer, const int nCharIndex, const char* szCharName,
	const unsigned int nSex, const unsigned int nHair, const unsigned int nFace, const unsigned int nCostume)
{
	MMatchSex sex = (nSex == 0) ? MMS_MALE : MMS_FEMALE;
	ResponseCreateChar(uidPlayer, nCharIndex, szCharName, MMatchSex(nSex), nHair, nFace, nCostume);
}

bool MMatchServer::ResponseCreateChar(const MUID& uidPlayer, const int nCharIndex, const char* szCharName,
	MMatchSex nSex, const unsigned int nHair, const unsigned int nFace, const unsigned int nCostume)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return false;

	if ((pObj->GetAccountInfo()->m_nAID < 0) ||
		(nCharIndex < 0) || (nCharIndex >= MAX_CHAR_COUNT) || (nHair >= MAX_COSTUME_HAIR) ||
		(nFace >= MAX_COSTUME_FACE) || (nCostume >= MAX_COSTUME_TEMPLATE))
	{
		int nResult = -1;
		MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_CREATE_CHAR, MUID(0, 0));
		pNewCmd->AddParameter(new MCommandParameterInt(nResult));
		pNewCmd->AddParameter(new MCommandParameterString(szCharName));
		RouteToListener(pObj, pNewCmd);
		return false;
	}

	int nResult = -1;

	nResult = ValidateMakingName(szCharName, MIN_CHARNAME, MAX_CHARNAME);
	if (nResult != MOK) {
		MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_CREATE_CHAR, MUID(0, 0));
		pNewCmd->AddParameter(new MCommandParameterInt(nResult));
		pNewCmd->AddParameter(new MCommandParameterString(szCharName));
		RouteToListener(pObj, pNewCmd);

		return false;
	}

	MAsyncDBJob_CreateChar* pJob = new MAsyncDBJob_CreateChar(uidPlayer, pObj->GetAccountInfo()->m_nAID,
		szCharName, nCharIndex, nSex, nHair, nFace, nCostume);

	pObj->m_DBJobQ.DBJobQ.push_back(pJob);

	return true;
}

void MMatchServer::OnCharClear(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj)
	{
		ObjectRemove(pObj->GetUID(), NULL);
	}
}

bool MMatchServer::CharInitialize(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return false;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if (pCharInfo == NULL) return false;

	pCharInfo->m_nConnTime = GetTickTime();

	m_objectCommandHistory.SetCharacterInfo(uidPlayer, pCharInfo->m_szName, pCharInfo->m_nCID);

	if (!pCharInfo->EquipFromItemList()) {
		mlog("MMatchServer::CharInitialize - EquipFromItemList Failed(CID - %d)\n", pCharInfo->m_nCID);
		if (!m_MatchDBMgr.ClearAllEquipedItem(pCharInfo->m_nCID)) {
			mlog("MMatchServer::CharInitialize - DB Query(ClearAllEquipedItem) Failed\n");
		}

		pCharInfo->m_EquipedItem.Clear();
	}

	CheckExpiredItems(pObj);
	// Custom: Check invalid items (by sex)
	//CheckInvalidItems(pObj);

	int nWeight, nMaxWeight;
	pCharInfo->GetTotalWeight(&nWeight, &nMaxWeight);
	if (nWeight > nMaxWeight) {
		if (!m_MatchDBMgr.ClearAllEquipedItem(pCharInfo->m_nCID)) {
			mlog("MMatchServer::CharInitialize - DB Query(ClearAllEquipedItem) Failed\n");
		}

		pCharInfo->m_EquipedItem.Clear();
	}

	if (pCharInfo->m_ClanInfo.IsJoined())
	{
		MCommand* pNew = CreateCommand(MC_MATCH_CLAN_MEMBER_CONNECTED, MUID(0, 0));
		pNew->AddParameter(new MCommandParameterString((char*)pCharInfo->m_szName));
		RouteToClan(pCharInfo->m_ClanInfo.m_nClanID, pNew);

		m_ClanMap.AddObject(uidPlayer, pObj);

		if (pObj->GetCharInfo()->m_ClanInfo.GetClanID() >= 9000000)
			LOG(LOG_FILE, "[CharInitialize()] %s's ClanID:%d.", pObj->GetAccountName(), pObj->GetCharInfo()->m_ClanInfo.GetClanID());

		{
			MMatchClan* pClan = m_ClanMap.GetClan(pCharInfo->m_ClanInfo.m_nClanID);
			if (pClan)
			{
				if (!pClan->IsInitedClanInfoEx())
				{
					pClan->InitClanInfoFromDB();
				}
			}
		}
	}

	return true;
}

void MMatchServer::CheckExpiredItems(MMatchObject* pObj)
{
	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if (NULL == pCharInfo)
		return;

	if (!pCharInfo->m_ItemList.HasRentItem()) return;

	vector<unsigned long int> vecExpiredItemIDList;
	vector<MUID> vecExpiredItemUIDList;
	const DWORD dwTick = GetTickTime();

	for (MMatchItemMap::iterator itor = pCharInfo->m_ItemList.begin(); itor != pCharInfo->m_ItemList.end(); ++itor)
	{
		MMatchItem* pCheckItem = (*itor).second;
		if (pCheckItem->IsRentItem())
		{
			if (IsExpiredRentItem(pCheckItem, dwTick))
			{
				MMatchCharItemParts nCheckParts = MMCIP_END;
				if (pCharInfo->m_EquipedItem.IsEquipedItem(itor->first, nCheckParts))
				{
					ResponseTakeoffItem(pObj->GetUID(), nCheckParts);
				}

				int nExpiredItemID = pCheckItem->GetDescID();
				if (nExpiredItemID != 0)
				{
					vecExpiredItemUIDList.push_back(pCheckItem->GetUID());
					vecExpiredItemIDList.push_back(nExpiredItemID);
				}
			}
		}
	}

	if (!vecExpiredItemIDList.empty())
	{
		int nExpiredItemUIDListCount = (int)vecExpiredItemUIDList.size();
		for (int i = 0; i < nExpiredItemUIDListCount; i++)
		{
			RemoveExpiredCharItem(pObj, vecExpiredItemUIDList[i]);
		}

		ResponseExpiredItemIDList(pObj, vecExpiredItemIDList);
	}
}

void MMatchServer::ResponseExpiredItemIDList(MMatchObject* pObj, vector<unsigned long int>& vecExpiredItemIDList)
{
	int nBlobSize = (int)vecExpiredItemIDList.size();
	MCommand* pNewCmd = CreateCommand(MC_MATCH_EXPIRED_RENT_ITEM, MUID(0, 0));

	void* pExpiredItemIDArray = MMakeBlobArray(sizeof(unsigned long int), nBlobSize);

	for (int i = 0; i < nBlobSize; i++)
	{
		unsigned long int* pItemID = (unsigned long int*)MGetBlobArrayElement(pExpiredItemIDArray, i);
		*pItemID = vecExpiredItemIDList[i];
	}
	pNewCmd->AddParameter(new MCommandParameterBlob(pExpiredItemIDArray, MGetBlobArraySize(pExpiredItemIDArray)));
	MEraseBlobArray(pExpiredItemIDArray);
	RouteToListener(pObj, pNewCmd);
}

void MMatchServer::CheckInvalidItems(MMatchObject* pPlayer)
{
	MMatchCharInfo* pCharInfo = pPlayer->GetCharInfo();
	if (NULL == pCharInfo)
		return;

	for (int nPart = 0; nPart < MMCIP_END; ++nPart)
	{
		if (pPlayer->GetCharInfo()->m_EquipedItem.IsEmpty((MMatchCharItemParts)nPart) == false)
		{
			MMatchItem* pItem = pPlayer->GetCharInfo()->m_EquipedItem.GetItem((MMatchCharItemParts)nPart);

			if (pItem->GetDesc())
			{
				if (pItem->GetDesc()->m_nResSex.Ref() != -1 && pItem->GetDesc()->m_nResSex.Ref() != (int)pCharInfo->m_nSex)
				{
					ResponseTakeoffItem(pPlayer->GetUID(), (MMatchCharItemParts)nPart);
				}
			}
		}
	}
}

bool MMatchServer::CorrectEquipmentByLevel(MMatchObject* pPlayer, MMatchCharItemParts nPart, int nLegalItemLevelDiff)
{
	if (!IsEnabledObject(pPlayer)) return false;

	if (pPlayer->GetCharInfo()->m_EquipedItem.IsEmpty(nPart) == false) {
		MMatchItem* pItem = pPlayer->GetCharInfo()->m_EquipedItem.GetItem(nPart);
		if (pItem->GetDesc())
		{
			if (pItem->GetDesc()->m_nResLevel.Ref() > (pPlayer->GetCharInfo()->m_nLevel + nLegalItemLevelDiff)) {
				ResponseTakeoffItem(pPlayer->GetUID(), nPart);
				return true;
			}
		}
	}
	return false;
}

bool MMatchServer::CharFinalize(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return false;

	UpdateCharDBCachingData(pObj);
	UpdateCharItemDBCachingData(pObj);

	DWORD nAID = 0;
	if (NULL != pObj->GetAccountInfo())
		nAID = pObj->GetAccountInfo()->m_nAID;

	MAsyncDBJob_UpdateAccountLastLoginTime* pUpdateAccountLastLoginTimeJob = new MAsyncDBJob_UpdateAccountLastLoginTime(uidPlayer);
	if (NULL != pUpdateAccountLastLoginTimeJob)
	{
		pUpdateAccountLastLoginTimeJob->Input(pObj->GetAccountInfo()->m_nAID);
		pObj->m_DBJobQ.DBJobQ.push_back(pUpdateAccountLastLoginTimeJob);
	}

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if (NULL == pCharInfo) return false;

	if (pCharInfo->m_ClanInfo.IsJoined())
	{
		m_ClanMap.RemoveObject(uidPlayer, pObj);
	}

	if (pCharInfo->m_nCID != 0)
	{
		unsigned long int nPlayTime = 0;
		unsigned long int nNowTime = GetTickTime();

		if (pCharInfo->m_nConnTime != 0) {
			nPlayTime = MGetTimeDistance(pCharInfo->m_nConnTime, nNowTime) / 1000;
		}

		MAsyncDBJob_CharFinalize* pJob = new MAsyncDBJob_CharFinalize(uidPlayer);
		pJob->Input(nAID,
			pCharInfo->m_nCID,
			nPlayTime,
			pCharInfo->m_nConnKillCount,
			pCharInfo->m_nConnDeathCount,
			pCharInfo->m_nConnXP,
			pCharInfo->m_nXP,
			pCharInfo->m_QuestItemList,
			pCharInfo->m_QMonsterBible,
			pCharInfo->m_DBQuestCachingData.IsRequestUpdateWhenLogout());
		pObj->m_DBJobQ.DBJobQ.push_back(pJob);

		pCharInfo->m_DBQuestCachingData.Reset();
	}

	for (MMatchCharBattleTimeRewardInfoMap::iterator iter = pCharInfo->GetBRInfoMap().begin();
		iter != pCharInfo->GetBRInfoMap().end(); iter++)
	{
		MMatchCharBRInfo* pInfo = iter->second;
		OnAsyncRequest_UpdateCharBRInfo(uidPlayer, pInfo->GetBRID(), pInfo->GetBRTID(), pInfo->GetRewardCount(), pInfo->GetBattleTime(), pInfo->GetKillCount());
	}

	return true;
}

void MMatchServer::OnRequestMySimpleCharInfo(const MUID& uidPlayer)
{
	ResponseMySimpleCharInfo(uidPlayer);
}

void MMatchServer::ResponseMySimpleCharInfo(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();

	MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_MY_SIMPLE_CHARINFO, MUID(0, 0));

	void* pMyCharInfoArray = MMakeBlobArray(sizeof(MTD_MySimpleCharInfo), 1);
	MTD_MySimpleCharInfo* pMyCharInfo = (MTD_MySimpleCharInfo*)MGetBlobArrayElement(pMyCharInfoArray, 0);

	pMyCharInfo->nXP = pCharInfo->m_nXP;
	pMyCharInfo->nBP = pCharInfo->m_nBP;
	pMyCharInfo->nLevel = pCharInfo->m_nLevel;

	pNewCmd->AddParameter(new MCommandParameterBlob(pMyCharInfoArray, MGetBlobArraySize(pMyCharInfoArray)));
	MEraseBlobArray(pMyCharInfoArray);

	RouteToListener(pObj, pNewCmd);
}

void MMatchServer::OnRequestCopyToTestServer(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if (pCharInfo == NULL) return;

	int nResult = MERR_UNKNOWN;

	ResponseCopyToTestServer(uidPlayer, nResult);
}

void MMatchServer::ResponseCopyToTestServer(const MUID& uidPlayer, const int nResult)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_COPY_TO_TESTSERVER, MUID(0, 0));
	pNewCmd->AddParameter(new MCommandParameterInt(nResult));
	RouteToListener(pObj, pNewCmd);
}

void MMatchServer::OnFriendAdd(const MUID& uidPlayer, const char* pszName)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;

	if (pObj->GetFriendInfo() == NULL) return;

	// Custom: Prevent users from adding themselves
	if (!stricmp(pObj->GetName(), pszName))
		return;

	MMatchObject* pTargetObj = GetPlayerByName(pszName);
	if (!IsEnabledObject(pTargetObj)) {
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	const int nTargetCID = pTargetObj->GetCharInfo()->m_nCID;

	if ((IsAdminGrade(pObj) == false) && (IsAdminGrade(pTargetObj) == true)) {
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	MMatchFriendNode* pNode = pObj->GetFriendInfo()->Find(pszName);
	if (pNode) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_FRIEND_ALREADY_EXIST);
		return;
	}

	if (pObj->GetFriendInfo()->m_FriendList.size() > MAX_FRIEND_COUNT) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_FRIEND_TOO_MANY_ADDED);
		return;
	}

	int nCID = pObj->GetCharInfo()->m_nCID;
	int nFriendCID = 0;
	if (m_MatchDBMgr.GetCharCID(pszName, &nFriendCID) == false) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHARACTER_NOT_EXIST);
		return;
	}

	if (nTargetCID != nFriendCID)
	{
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHARACTER_NOT_EXIST);
		return;
	}

	if (m_MatchDBMgr.FriendAdd(nCID, nFriendCID, 0) == false) {
		mlog("DB Query(FriendAdd) Failed\n");
		return;
	}

	// Custom: Don't use pszName since MLex (in Gunz) always forces it to lowercase.
	pObj->GetFriendInfo()->Add(nFriendCID, 0, pTargetObj->GetName());
	NotifyMessage(uidPlayer, MATCHNOTIFY_FRIEND_ADD_SUCCEED);
}

void MMatchServer::OnFriendRemove(const MUID& uidPlayer, const char* pszName)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;
	if (pObj->GetFriendInfo() == NULL) return;

	MMatchFriendNode* pNode = pObj->GetFriendInfo()->Find(pszName);
	if (pNode == NULL) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_FRIEND_NOT_EXIST);
		return;
	}

	const int nNodeCID = pNode->nFriendCID;

	int nCID = pObj->GetCharInfo()->m_nCID;
	int nFriendCID = 0;
	if (m_MatchDBMgr.GetCharCID(pszName, &nFriendCID) == false) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHARACTER_NOT_EXIST);
		return;
	}

	if (nNodeCID != nFriendCID)
	{
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHARACTER_NOT_EXIST);
		return;
	}

	if (m_MatchDBMgr.FriendRemove(nCID, nFriendCID) == false) {
		mlog("DB Query(FriendRemove) Failed\n");
		return;
	}

	pObj->GetFriendInfo()->Remove(pszName);
	NotifyMessage(uidPlayer, MATCHNOTIFY_FRIEND_REMOVE_SUCCEED);
}

void MMatchServer::OnFriendList(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;

	if (!pObj->DBFriendListRequested())
	{
		MAsyncDBJob_FriendList* pJob = new MAsyncDBJob_FriendList(uidPlayer, pObj->GetCharInfo()->m_nCID);
		pJob->SetFriendInfo(new MMatchFriendInfo);
		pObj->m_DBJobQ.DBJobQ.push_back(pJob);
	}
	else if (!pObj->GetFriendInfo())
	{
		return;
	}

	FriendList(uidPlayer);
}

void MMatchServer::FriendList(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;
	if (pObj->GetFriendInfo() == NULL) return;

	pObj->GetFriendInfo()->UpdateDesc();

	MMatchFriendList* pList = &pObj->GetFriendInfo()->m_FriendList;

	void* pListArray = MMakeBlobArray(sizeof(MFRIENDLISTNODE), (int)pList->size());
	int nIndex = 0;
	for (MMatchFriendList::iterator i = pList->begin(); i != pList->end(); i++) {
		MMatchFriendNode* pNode = (*i);
		MFRIENDLISTNODE* pListNode = (MFRIENDLISTNODE*)MGetBlobArrayElement(pListArray, nIndex++);
		pListNode->nState = pNode->nState;
		strcpy(pListNode->szName, pNode->szName);
		strcpy(pListNode->szDescription, pNode->szDescription);
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_RESPONSE_FRIENDLIST, MUID(0, 0));
	pCmd->AddParameter(new MCommandParameterBlob(pListArray, MGetBlobArraySize(pListArray)));
	MEraseBlobArray(pListArray);
	RouteToListener(pObj, pCmd);
}

void MMatchServer::OnFriendMsg(const MUID& uidPlayer, const char* szMsg)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;
}

void MMatchServer::OnRequestCharInfoDetail(const MUID& uidChar, const char* szCharName)
{
	ResponseCharInfoDetail(uidChar, szCharName);
}

void MMatchServer::ResponseCharInfoDetail(const MUID& uidChar, const char* szCharName)
{
	MMatchObject* pObject = GetObject(uidChar);
	if (!IsEnabledObject(pObject)) return;

	MMatchObject* pTarObject = GetPlayerByName(szCharName);
	if (!IsEnabledObject(pTarObject))
	{
		RouteResponseToListener(pObject, MC_MATCH_RESPONSE_RESULT, MERR_NO_TARGET);
		return;
	}

	MTD_CharInfo_Detail trans_charinfo_detail;
	CopyCharInfoDetailForTrans(&trans_charinfo_detail, pTarObject->GetCharInfo(), pTarObject);

	MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_CHARINFO_DETAIL, MUID(0, 0));

	void* pCharInfoArray = MMakeBlobArray(sizeof(MTD_CharInfo_Detail), 1);
	MTD_CharInfo_Detail* pTransCharInfoDetail = (MTD_CharInfo_Detail*)MGetBlobArrayElement(pCharInfoArray, 0);
	memcpy(pTransCharInfoDetail, &trans_charinfo_detail, sizeof(MTD_CharInfo_Detail));
	pNewCmd->AddParameter(new MCommandParameterBlob(pCharInfoArray, MGetBlobArraySize(pCharInfoArray)));
	MEraseBlobArray(pCharInfoArray);

	RouteToListener(pObject, pNewCmd);
}