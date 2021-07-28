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
#include "MMatchTransDataType.h"
#include "MUtil.h"

void CopyChannelPlayerListNodeForTrans(MTD_ChannelPlayerListNode* pDest, MMatchObject* pSrcObject)
{
	pDest->uidPlayer = pSrcObject->GetUID();
	strcpy(pDest->szName, pSrcObject->GetCharInfo()->m_szName);
	strcpy(pDest->szClanName, pSrcObject->GetCharInfo()->m_ClanInfo.m_szClanName);
	pDest->nLevel = (char)pSrcObject->GetCharInfo()->m_nLevel;
	pDest->nPlace = pSrcObject->GetPlace();
	pDest->nGrade = (unsigned char)pSrcObject->GetAccountInfo()->m_nUGrade;
	pDest->nPlayerFlags = pSrcObject->GetPlayerFlags();
	pDest->nCLID = pSrcObject->GetCharInfo()->m_ClanInfo.m_nClanID;

	MMatchObjectDuelTournamentCharInfo* pDTCharInfo = pSrcObject->GetDuelTournamentCharInfo();
	pDest->nDTLastWeekGrade = pDTCharInfo ? pDTCharInfo->GetLastWeekGrade() : 0;

	MMatchClan* pClan = MMatchServer::GetInstance()->GetClanMap()->GetClan(pSrcObject->GetCharInfo()->m_ClanInfo.m_nClanID);
	if (pClan)
		pDest->nEmblemChecksum = pClan->GetEmblemChecksum();
	else
		pDest->nEmblemChecksum = 0;
}

MMatchChannel* MMatchServer::FindChannel(const MUID& uidChannel)
{
	return m_ChannelMap.Find(uidChannel);
}

MMatchChannel* MMatchServer::FindChannel(const MCHANNEL_TYPE nChannelType, const char* pszChannelName)
{
	return m_ChannelMap.Find(nChannelType, pszChannelName);
}

bool MMatchServer::ChannelAdd(const char* pszChannelName, const char* pszRuleName, MUID* pAllocUID, MCHANNEL_TYPE nType, int nMaxPlayers, int nLevelMin, int nLevelMax,
	const bool bIsTicketChannel, const DWORD dwTicketItemID, const bool bIsUseTicket, const char* pszChannelNameStrResId)
{
	return m_ChannelMap.Add(pszChannelName, pszRuleName, pAllocUID, nType, nMaxPlayers, nLevelMin, nLevelMax, bIsTicketChannel, dwTicketItemID, bIsUseTicket, pszChannelNameStrResId);
}

bool MMatchServer::ChannelJoin(const MUID& uidPlayer, const MCHANNEL_TYPE nChannelType, const char* pszChannelName)
{
	if ((nChannelType < 0) || (nChannelType >= MCHANNEL_TYPE_MAX)) return false;

	int nChannelNameLen = (int)strlen(pszChannelName);
	if ((nChannelNameLen >= CHANNELNAME_LEN) || (nChannelNameLen <= 0)) return false;

	MUID uidChannel = MUID(0, 0);

	MMatchChannel* pChannel = FindChannel(nChannelType, pszChannelName);

	if (pChannel == NULL)
	{
		if (nChannelType == MCHANNEL_TYPE_PRESET)
			return false;

		if (nChannelType == MCHANNEL_TYPE_DUELTOURNAMENT)
			return false;

		if (!ChannelAdd(pszChannelName, GetDefaultChannelRuleName(), &uidChannel, nChannelType))
			return false;
	}
	else
	{
		uidChannel = pChannel->GetUID();
	}

	return ChannelJoin(uidPlayer, uidChannel);
}

bool MMatchServer::ChannelJoin(const MUID& uidPlayer, const MUID& uidChannel)
{
	bool bEnableInterface = true;
	MUID uidChannelTmp = uidChannel;

	MMatchChannel* pChannel = FindChannel(uidChannelTmp);
	if (pChannel == NULL) return false;

	if (MGetServerConfig()->IsUseTicket())
	{
		bool bCheckTicket = false;

		MMatchObject* pObj = GetObject(uidPlayer);
		if (!pObj)	return false;

		if (MGetServerConfig()->GetServerMode() == MSM_NORMALS) {
			if (_stricmp(pChannel->GetRuleName(), MCHANNEL_RULE_NOVICE_STR) == 0) bEnableInterface = false;
			else if (pChannel->IsTicketChannel())									bCheckTicket = true;
		}
		else if ((MGetServerConfig()->GetServerMode() == MSM_CLAN) || (MGetServerConfig()->GetServerMode() == MSM_QUEST)) {
			if ((pChannel->GetChannelType() == MCHANNEL_TYPE_CLAN) || (pChannel->GetChannelType() == MCHANNEL_TYPE_USER)) bCheckTicket = true;
			else if (_stricmp(pChannel->GetRuleName(), MCHANNEL_RULE_NOVICE_STR) == 0)
				bEnableInterface = false;
			else if (pChannel->IsTicketChannel())
				bCheckTicket = true;
		}
		else {
			if (_stricmp(pChannel->GetRuleName(), MCHANNEL_RULE_NOVICE_STR) == 0) bEnableInterface = false;
			else if (pChannel->IsTicketChannel())									bCheckTicket = true;
		}

		if (IsAdminGrade(pObj)) {
			bCheckTicket = false;
			bEnableInterface = true;
		}

		if (bCheckTicket) {
			if (!pObj->GetCharInfo()->m_ItemList.IsHave(GetChannelMap()->GetClanChannelTicketInfo().m_dwTicketItemID)) {
				if (pObj->GetPlace() == MMP_LOBBY)
					RouteResponseToListener(pObj, MC_MATCH_RESPONSE_RESULT, MERR_CANNOT_JOIN_NEED_TICKET);

				const MUID& uidFreeChannel = FindFreeChannel(uidPlayer);

				if (MUID(0, 0) == uidChannel) {
					ASSERT(0 && "들어갈 수 있는 채널을 찾지 못했음.");
					mlog("MMatchServer_Channel.cpp - ChannelJoin : Can't find free channel.\n");
					return false;
				}

				uidChannelTmp = uidFreeChannel;
				bEnableInterface = false;
			}
		}
	}

	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if (pObj == NULL) return false;

	const int ret = ValidateChannelJoin(uidPlayer, uidChannelTmp);
	if (ret != MOK) {
		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_RESULT, ret);
		return false;
	}

	MMatchChannel* pOldChannel = FindChannel(pObj->GetChannelUID());
	if (pOldChannel) {
		pOldChannel->RemoveObject(uidPlayer);
	}

	pObj->SetChannelUID(uidChannelTmp);
	pObj->SetLadderChallenging(false);
	pObj->SetPlace(MMP_LOBBY);
	pObj->SetStageListTransfer(true);
	pObj->SetStageCursor(0);

	pChannel = FindChannel(uidChannelTmp);
	if (pChannel == NULL) return false;

	pChannel->AddObject(uidPlayer, pObj);
	ResponseChannelJoin(uidPlayer, uidChannelTmp, (int)pChannel->GetChannelType(), pChannel->GetNameStringResId(), bEnableInterface);
	ResponseChannelRule(uidPlayer, uidChannelTmp);

	if (pChannel->GetRuleType() != MCHANNEL_RULE_DUELTOURNAMENT) {
		StageList(uidPlayer, 0, false);
	}
	else {
	}

	ChannelResponsePlayerList(uidPlayer, uidChannelTmp, 0);

	if (pChannel->GetChannelType() == MCHANNEL_TYPE_CLAN)
	{
		ClanReDef::iterator it = ClanRejoiner.find(pObj->GetCharInfo()->m_nCID);
		if (it != ClanRejoiner.end())
		{
			MMatchStage* pStage = FindStage(it->second->StageUID);
			if (pStage)
			{
				Announce(pObj, "To rejoin your game type /rejoin");
			}
		}
	}

	return true;
}

void MMatchServer::ResponseChannelJoin(MUID uidPlayer, MUID uidChannel, int nChannelType
	, const char* szChannelStrResId, bool bEnableInterface)
{
	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if (pObj == NULL) return;

	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return;

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_CHANNEL_RESPONSE_JOIN), MUID(0, 0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidChannel));
	pNew->AddParameter(new MCommandParameterInt(nChannelType));

	if (szChannelStrResId[0] != 0) {
		pNew->AddParameter(new MCommandParameterString((char*)szChannelStrResId));
	}
	else {
		pNew->AddParameter(new MCommandParameterString((char*)pChannel->GetName()));
	}

	pNew->AddParameter(new MCommandParameterBool(bEnableInterface));
	RouteToListener(pObj, pNew);
}

bool MMatchServer::ChannelLeave(const MUID& uidPlayer, const MUID& uidChannel)
{
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return false;
	pChannel->RemoveObject(uidPlayer);

	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if (pObj == NULL) return false;

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_CHANNEL_LEAVE), MUID(0, 0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidPlayer));
	pNew->AddParameter(new MCommandParameterUID(pChannel->GetUID()));
	RouteToListener(pObj, pNew);

	if (pObj)
	{
		pObj->SetChannelUID(MUID(0, 0));
		pObj->SetPlace(MMP_OUTSIDE);
		pObj->SetStageListTransfer(false);
	}
	return true;
}

bool MMatchServer::ChannelChat(const MUID& uidPlayer, const MUID& uidChannel, char* pszChat)
{
	if (0 == strlen(pszChat)) return false;
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return false;
	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) return false;
	if (pObj->GetAccountInfo()->m_nUGrade == MMUG_CHAT_LIMITED) return false;

	int nGrade = (int)pObj->GetAccountInfo()->m_nUGrade;

	if (uidChannel != pObj->GetChannelUID())
	{
		return false;
	}

	MUID uidStage = pObj->GetStageUID();
	if (uidStage != MUID(0, 0))
	{
		return false;
	}

	if (pObj->GetAccountPenaltyInfo()->IsBlock(MPC_CHAT_BLOCK)) {
		return false;
	}

	if (pObj->IsChatBanUser() == true)	return false;
	if (pObj->CheckChatFlooding())
	{
		pObj->SetChatBanUser();

		MCommand* pCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_CHANNEL_DUMB_CHAT), pObj->GetUID(), m_This);
		Post(pCmd);

		LOG(LOG_FILE, "MMatchServer::ChannelChat - Set Dumb Player(MUID:%d%d, Name:%s)", pObj->GetUID().High, pObj->GetUID().Low, pObj->GetName());
		return false;
	}

	MCommand* pCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_CHANNEL_CHAT), MUID(0, 0), m_This);
	pCmd->AddParameter(new MCommandParameterUID(uidChannel));
	pCmd->AddParameter(new MCommandParameterString(pObj->GetCharInfo()->m_szName));
	pCmd->AddParameter(new MCommandParameterString(pszChat));
	pCmd->AddParameter(new MCommandParameterInt(nGrade));

	RouteToChannelLobby(uidChannel, pCmd);
	return true;
}

void MMatchServer::OnRequestRecommendedChannel(const MUID& uidComm)
{
	MUID uidChannel = FindFreeChannel(uidComm);

	if (MUID(0, 0) == uidChannel)
	{
		if (!ChannelAdd(GetDefaultChannelName(), GetDefaultChannelRuleName(), &uidChannel, MCHANNEL_TYPE_PRESET))
		{
			mlog("Channel Add fail for recommand.\n");
			return;
		}
	}

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_RESPONSE_RECOMMANDED_CHANNEL),
		uidComm, m_This);
	pNew->AddParameter(new MCommandParameterUID(uidChannel));
	Post(pNew);
}

void MMatchServer::OnRequestChannelJoin(const MUID& uidPlayer, const MUID& uidChannel)
{
	ChannelJoin(uidPlayer, uidChannel);
}

void MMatchServer::OnRequestChannelJoin(const MUID& uidPlayer, const MCHANNEL_TYPE nChannelType, const char* pszChannelName)
{
	if ((nChannelType < 0) || (nChannelType >= MCHANNEL_TYPE_MAX)) return;

	ChannelJoin(uidPlayer, nChannelType, pszChannelName);
}

void MMatchServer::OnChannelChat(const MUID& uidPlayer, const MUID& uidChannel, char* pszChat)
{
	ChannelChat(uidPlayer, uidChannel, pszChat);
}

void MMatchServer::OnStartChannelList(const MUID& uidPlayer, const int nChannelType)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	pObj->SetChannelListTransfer(true, MCHANNEL_TYPE(nChannelType));
}

void MMatchServer::OnStopChannelList(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	pObj->SetChannelListTransfer(false);
}

void MMatchServer::ChannelList(const MUID& uidPlayer, MCHANNEL_TYPE nChannelType)
{
	MMatchObject* pChar = GetObject(uidPlayer);
	if (!IsEnabledObject(pChar)) return;

	if (pChar->GetPlace() != MMP_LOBBY) return;
	if ((nChannelType < 0) || (nChannelType >= MCHANNEL_TYPE_MAX)) return;

	int nChannelCount = (int)m_ChannelMap.GetChannelCount(nChannelType);
	if (nChannelCount <= 0) return;

#define MAX_CHANNEL_LIST_NODE		100

	nChannelCount = min(nChannelCount, MAX_CHANNEL_LIST_NODE);

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_CHANNEL_LIST), MUID(0, 0), m_This);

	void* pChannelArray = MMakeBlobArray(sizeof(MCHANNELLISTNODE), nChannelCount);
	int nIndex = 0;
	for (map<MUID, MMatchChannel*>::iterator itor = m_ChannelMap.GetTypesChannelMapBegin(nChannelType);
		itor != m_ChannelMap.GetTypesChannelMapEnd(nChannelType); itor++) {
		if (nIndex >= nChannelCount) break;

		MMatchChannel* pChannel = (*itor).second;

		MCHANNELLISTNODE* pNode = (MCHANNELLISTNODE*)MGetBlobArrayElement(pChannelArray, nIndex++);
		pNode->uidChannel = pChannel->GetUID();
		pNode->nNo = nIndex;
		pNode->nPlayers = (unsigned char)pChannel->GetObjCount();
		pNode->nMaxPlayers = pChannel->GetMaxPlayers();
		pNode->nChannelType = pChannel->GetChannelType();
		strcpy(pNode->szChannelName, pChannel->GetName());
		strcpy(pNode->szChannelNameStrResId, pChannel->GetNameStringResId());
		pNode->bIsUseTicket = pChannel->IsUseTicket();
		pNode->nTicketID = pChannel->GetTicketItemID();
	}
	pNew->AddParameter(new MCommandParameterBlob(pChannelArray, MGetBlobArraySize(pChannelArray)));
	MEraseBlobArray(pChannelArray);

	RouteToListener(pChar, pNew);
}

void MMatchServer::OnChannelRequestPlayerList(const MUID& uidPlayer, const MUID& uidChannel, int nPage)
{
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return;
	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;

	MRefreshClientChannelImpl* pImpl = pObj->GetRefreshClientChannelImplement();
	pImpl->SetCategory(nPage);
	pImpl->SetChecksum(0);
	pImpl->Enable(true);
	pChannel->SyncPlayerList(pObj, nPage);
}

void MMatchServer::ChannelResponsePlayerList(const MUID& uidPlayer, const MUID& uidChannel, int nPage)
{
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return;
	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;

	int nObjCount = (int)pChannel->GetObjCount();
	int nNodeCount = 0;
	int nPlayerIndex;

	if (nPage < 0) nPage = 0;

	nPlayerIndex = nPage * NUM_PLAYERLIST_NODE;
	if (nPlayerIndex >= nObjCount)
	{
		nPage = (nObjCount / NUM_PLAYERLIST_NODE);
		nPlayerIndex = nPage * NUM_PLAYERLIST_NODE;
	}

	MUIDRefCache::iterator FirstItor = pChannel->GetObjBegin();

	for (int i = 0; i < nPlayerIndex; i++)
	{
		if (FirstItor == pChannel->GetObjEnd()) break;
		FirstItor++;
	}

	nNodeCount = nObjCount - nPlayerIndex;
	if (nNodeCount <= 0)
	{
		return;
	}
	else if (nNodeCount > NUM_PLAYERLIST_NODE) nNodeCount = NUM_PLAYERLIST_NODE;

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_CHANNEL_RESPONSE_PLAYER_LIST), MUID(0, 0), m_This);
	pNew->AddParameter(new MCommandParameterUChar((unsigned char)nObjCount));
	pNew->AddParameter(new MCommandParameterUChar((unsigned char)nPage));

	void* pPlayerArray = MMakeBlobArray(sizeof(MTD_ChannelPlayerListNode), nNodeCount);

	int nArrayIndex = 0;
	for (MUIDRefCache::iterator i = FirstItor; i != pChannel->GetObjEnd(); i++)
	{
		MMatchObject* pScanObj = (MMatchObject*)(*i).second;

		MTD_ChannelPlayerListNode* pNode = (MTD_ChannelPlayerListNode*)MGetBlobArrayElement(pPlayerArray, nArrayIndex++);

		if (IsEnabledObject(pScanObj))
		{
			CopyChannelPlayerListNodeForTrans(pNode, pScanObj);
		}

		if (nArrayIndex >= nNodeCount) break;
	}

	pNew->AddParameter(new MCommandParameterBlob(pPlayerArray, MGetBlobArraySize(pPlayerArray)));
	MEraseBlobArray(pPlayerArray);
	RouteToListener(pObj, pNew);
}

void MMatchServer::OnChannelRequestAllPlayerList(const MUID& uidPlayer, const MUID& uidChannel, unsigned long int nPlaceFilter,
	unsigned long int nOptions)
{
	ChannelResponseAllPlayerList(uidPlayer, uidChannel, nPlaceFilter, nOptions);
}

void MMatchServer::ChannelResponseAllPlayerList(const MUID& uidPlayer, const MUID& uidChannel, unsigned long int nPlaceFilter,
	unsigned long int nOptions)
{
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return;
	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;

	int nNodeCount = 0;

	MMatchObject* ppTransObjectArray[DEFAULT_CHANNEL_MAXPLAYERS];
	memset(ppTransObjectArray, 0, sizeof(MMatchObject*) * DEFAULT_CHANNEL_MAXPLAYERS);

	for (MUIDRefCache::iterator i = pChannel->GetObjBegin(); i != pChannel->GetObjEnd(); i++)
	{
		MMatchObject* pScanObj = (MMatchObject*)(*i).second;

		if (IsEnabledObject(pScanObj))
		{
			if (CheckBitSet(nPlaceFilter, (pScanObj->GetPlace())))
			{
				bool bScanObjOK = true;
				switch (nOptions)
				{
				case MCP_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST_NONCLAN:
				{
					if (pScanObj->GetCharInfo()->m_ClanInfo.IsJoined()) bScanObjOK = false;
				}
				break;
				case MCP_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST_MYCLAN:
				{
					if (!pObj->GetCharInfo()->m_ClanInfo.IsJoined())
					{
						bScanObjOK = false;
					}
					else if (pScanObj->GetCharInfo()->m_ClanInfo.m_nClanID != pObj->GetCharInfo()->m_ClanInfo.m_nClanID)
					{
						bScanObjOK = false;
					}
				}
				break;
				}

				if (bScanObjOK)
				{
					ppTransObjectArray[nNodeCount] = pScanObj;
					nNodeCount++;

					if (nNodeCount >= DEFAULT_CHANNEL_MAXPLAYERS) break;
				}
			}
		}
	}

	if (nNodeCount <= 0) return;

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_CHANNEL_RESPONSE_ALL_PLAYER_LIST), MUID(0, 0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidChannel));

	void* pPlayerArray = MMakeBlobArray(sizeof(MTD_ChannelPlayerListNode), nNodeCount);

	for (int i = 0; i < nNodeCount; i++)
	{
		MMatchObject* pScanObj = ppTransObjectArray[i];

		MTD_ChannelPlayerListNode* pNode = (MTD_ChannelPlayerListNode*)MGetBlobArrayElement(pPlayerArray, i);

		if (IsEnabledObject(pScanObj))
		{
			CopyChannelPlayerListNodeForTrans(pNode, pScanObj);
		}
	}

	pNew->AddParameter(new MCommandParameterBlob(pPlayerArray, MGetBlobArraySize(pPlayerArray)));
	MEraseBlobArray(pPlayerArray);
	RouteToListener(pObj, pNew);
}

void MMatchServer::ResponseChannelRule(const MUID& uidPlayer, const MUID& uidChannel)
{
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return;
	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) return;

	MCommand* pNew = CreateCommand(MC_MATCH_CHANNEL_RESPONSE_RULE, MUID(0, 0));
	pNew->AddParameter(new MCommandParameterUID(uidChannel));
	pNew->AddParameter(new MCmdParamStr(const_cast<char*>(pChannel->GetRuleName())));
	RouteToListener(pObj, pNew);
}

const MUID MMatchServer::FindFreeChannel(const MUID& uidPlayer)
{
	MUID uidChannel = MUID(0, 0);

	if (uidChannel == MUID(0, 0) &&
		MGetServerConfig()->IsEnabledDuelTournament() &&
		MGetServerConfig()->IsSendLoginUserToDuelTournamentChannel())
	{
		for (map<MUID, MMatchChannel*>::iterator itor = m_ChannelMap.GetTypesChannelMapBegin(MCHANNEL_TYPE_DUELTOURNAMENT);
			itor != m_ChannelMap.GetTypesChannelMapEnd(MCHANNEL_TYPE_DUELTOURNAMENT); itor++) {
			MUID uid = (*itor).first;
			if (MOK == ValidateChannelJoin(uidPlayer, uid)) {
				MMatchChannel* pChannel = FindChannel(uid);
				if (pChannel) {
					if (pChannel->GetMaxPlayers() * 0.8 < pChannel->GetObjCount()) continue;
					uidChannel = uid;
					break;
				}
			}
		}
	}

	if (uidChannel == MUID(0, 0))
	{
		for (map<MUID, MMatchChannel*>::const_iterator itor = m_ChannelMap.GetTypesChannelMapBegin(MCHANNEL_TYPE_PRESET);
			itor != m_ChannelMap.GetTypesChannelMapEnd(MCHANNEL_TYPE_PRESET); itor++) {
			MUID uid = (*itor).first;
			if (MOK == ValidateChannelJoin(uidPlayer, uid)) {
				MMatchChannel* pChannel = FindChannel(uid);
				if (pChannel) {
					if (pChannel->GetLevelMin() <= 0) continue;
					if (pChannel->GetMaxPlayers() * 0.8 < pChannel->GetObjCount()) continue;
					uidChannel = uid;
					break;
				}
			}
		}
	}

	if (uidChannel == MUID(0, 0))
	{
		for (map<MUID, MMatchChannel*>::iterator itor = m_ChannelMap.GetTypesChannelMapBegin(MCHANNEL_TYPE_PRESET);
			itor != m_ChannelMap.GetTypesChannelMapEnd(MCHANNEL_TYPE_PRESET); itor++) {
			MUID uid = (*itor).first;
			if (MOK == ValidateChannelJoin(uidPlayer, uid)) {
				MMatchChannel* pChannel = FindChannel(uid);
				if (pChannel) {
					if (pChannel->GetMaxPlayers() * 0.8 < pChannel->GetObjCount()) continue;
					uidChannel = uid;
					break;
				}
			}
		}
	}

	if (uidChannel == MUID(0, 0))
	{
		for (map<MUID, MMatchChannel*>::iterator itor = m_ChannelMap.GetTypesChannelMapBegin(MCHANNEL_TYPE_USER);
			itor != m_ChannelMap.GetTypesChannelMapEnd(MCHANNEL_TYPE_USER); itor++) {
			MUID uid = (*itor).first;
			if (MOK == ValidateChannelJoin(uidPlayer, uid)) {
				MMatchChannel* pChannel = FindChannel(uid);
				if (pChannel) {
					uidChannel = uid;
					break;
				}
			}
		}
	}

	return uidChannel;
}