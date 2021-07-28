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
#include "MLadderMgr.h"
#include "MTeamGameStrategy.h"

int MMatchServer::ValidateChallengeLadderGame(MMatchObject** ppMemberObject, int nMemberCount)
{
	MBaseTeamGameStrategy* pTeamGameStrategy = MBaseTeamGameStrategy::GetInstance(MGetServerConfig()->GetServerMode());
	if (pTeamGameStrategy)
	{
		int nRet = pTeamGameStrategy->ValidateChallenge(ppMemberObject, nMemberCount);
		return nRet;
	};

	return MOK;
}

bool MMatchServer::LadderJoin(const MUID& uidPlayer, const MUID& uidStage, MMatchTeam nTeam)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return false;

	if (pObj->GetStageUID() != MUID(0, 0))
		StageLeave(pObj->GetUID());

	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	ClanRejoiner.insert(ClanReDef::value_type(pObj->GetCharInfo()->m_nCID, new ClanRejoin(nTeam, uidStage)));

	pObj->OnStageJoin();

	pStage->AddObject(uidPlayer, pObj);
	pObj->SetStageUID(uidStage);
	pObj->SetStageState(MOSS_READY);
	pObj->SetLadderChallenging(false);
	pStage->PlayerTeam(uidPlayer, nTeam);
	pStage->PlayerState(uidPlayer, MOSS_READY);

	MCommand* pCmd = CreateCommand(MC_MATCH_LADDER_PREPARE, uidPlayer);
	pCmd->AddParameter(new MCmdParamUID(uidStage));
	pCmd->AddParameter(new MCmdParamInt(nTeam));
	Post(pCmd);

	pStage->SetCLID(pObj->GetCharInfo()->m_ClanInfo.m_nClanID, nTeam);
	return true;
}

void MMatchServer::LadderGameLaunch(MLadderGroup* pGroupA, MLadderGroup* pGroupB, int nMapID)
{
	MUID uidStage = MUID(0, 0);
	if (StageAdd(NULL, "LADDER_GAME", true, "", &uidStage) == false) {
		GetLadderMgr()->CancelChallenge(pGroupA->GetID(), "");
		GetLadderMgr()->CancelChallenge(pGroupB->GetID(), "");
		return;
	}
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) {
		GetLadderMgr()->CancelChallenge(pGroupA->GetID(), "");
		GetLadderMgr()->CancelChallenge(pGroupB->GetID(), "");
		return;
	}

	for (list<MUID>::iterator i = pGroupA->GetPlayerListBegin(); i != pGroupA->GetPlayerListEnd(); i++)
	{
		MUID uidPlayer = (*i);
		LadderJoin(uidPlayer, uidStage, MMT_RED);
	}
	for (list<MUID>::iterator i = pGroupB->GetPlayerListBegin(); i != pGroupB->GetPlayerListEnd(); i++)
	{
		MUID uidPlayer = (*i);
		LadderJoin(uidPlayer, uidStage, MMT_BLUE);
	}

	MBaseTeamGameStrategy* pTeamGameStrategy = MBaseTeamGameStrategy::GetInstance(MGetServerConfig()->GetServerMode());

	MMATCH_GAMETYPE nGameType = MMATCH_GAMETYPE_DEATHMATCH_TEAM;

	pStage->SetStageType(MST_LADDER);
	pStage->ChangeRule(nGameType);

	if (pTeamGameStrategy)
	{
		MMatchLadderTeamInfo a_RedLadderTeamInfo, a_BlueLadderTeamInfo;
		pTeamGameStrategy->SetStageLadderInfo(&a_RedLadderTeamInfo, &a_BlueLadderTeamInfo, pGroupA, pGroupB);

		pStage->SetLadderTeam(&a_RedLadderTeamInfo, &a_BlueLadderTeamInfo);
	};

	MMatchStageSetting* pSetting = pStage->GetStageSetting();
	pSetting->SetMasterUID(MUID(0, 0));
	pSetting->SetMapIndex(nMapID);
	pSetting->SetGameType(nGameType);

	pSetting->SetLimitTime(3);
	pSetting->SetRoundMax(99);
	pSetting->SetAntiLead(pGroupA->GetAntiLeadMatching());

	MCommand* pCmd = CreateCmdResponseStageSetting(uidStage);
	RouteToStage(uidStage, pCmd);

	if ((MGetMapDescMgr()->MIsCorrectMap(nMapID)) && (MGetGameTypeMgr()->IsCorrectGameType(nGameType)))
	{
		if (pStage->StartGame(MGetServerConfig()->IsUseResourceCRC32CacheCheck()) == true) {
			ReserveAgent(pStage);

			MMatchObjectCacheBuilder CacheBuilder;
			CacheBuilder.Reset();
			for (MUIDRefCache::iterator i = pStage->GetObjBegin(); i != pStage->GetObjEnd(); i++) {
				MUID uidObj = (MUID)(*i).first;
				MMatchObject* pScanObj = (MMatchObject*)GetObject(uidObj);
				if (pScanObj) {
					CacheBuilder.AddObject(pScanObj);
				}
			}
			MCommand* pCmdCacheAdd = CacheBuilder.GetResultCmd(MATCHCACHEMODE_UPDATE, this);
			RouteToStage(pStage->GetUID(), pCmdCacheAdd);
			MCommand* pCmd = CreateCommand(MC_MATCH_LADDER_LAUNCH, MUID(0, 0));
			pCmd->AddParameter(new MCmdParamUID(uidStage));
			pCmd->AddParameter(new MCmdParamStr(const_cast<char*>(pStage->GetMapName())));
			RouteToStage(uidStage, pCmd);
		}
		else {
			GetLadderMgr()->CancelChallenge(pGroupA->GetID(), "");
			GetLadderMgr()->CancelChallenge(pGroupB->GetID(), "");
		}
	}
}

bool MMatchServer::IsLadderRequestUserInRequestClanMember(const MUID& uidRequestMember
	, const MTD_LadderTeamMemberNode* pRequestMemberNode)
{
	if (NULL == pRequestMemberNode)
		return false;

	MMatchObject* pRequestMemberObj = GetPlayerByName(pRequestMemberNode->szName);
	if (NULL == pRequestMemberObj)
		return false;

	if (uidRequestMember != pRequestMemberObj->GetUID())
		return false;

	return true;
}

void MMatchServer::OnLadderRequestChallenge(const MUID& uidRequestMember, void* pMemberNamesBlob, unsigned long int nOptions, unsigned long int nAntiLead)
{
	if ((MGetServerConfig()->GetServerMode() != MSM_LADDER) &&
		(MGetServerConfig()->GetServerMode() != MSM_CLAN)) return;

	MMatchObject* pLeaderObject = GetPlayerByCommUID(uidRequestMember);
	if (!IsEnabledObject(pLeaderObject)) return;

	if (!MGetServerConfig()->IsEnabledCreateLadderGame())
	{
		RouteResponseToListener(pLeaderObject, MC_MATCH_LADDER_RESPONSE_CHALLENGE, MERR_LADDER_NOT_SERVICE_TIME);
		return;
	}

	int nBlobCount = MGetBlobArrayCount(pMemberNamesBlob);
	int nMemberCount = nBlobCount;
	if (nMemberCount <= 0) return;

	if (!IsLadderRequestUserInRequestClanMember(uidRequestMember
		, (MTD_LadderTeamMemberNode*)MGetBlobArrayElement(pMemberNamesBlob, 0)))
		return;

	MMatchObject* pMemberObjects[MAX_CLANBATTLE_TEAM_MEMBER];
	for (int i = 0; i < nMemberCount; i++)
	{
		MTD_LadderTeamMemberNode* pNode = (MTD_LadderTeamMemberNode*)MGetBlobArrayElement(pMemberNamesBlob, i);
		if (pNode == NULL) break;
		if ((strlen(pNode->szName) <= 0) || (strlen(pNode->szName) >= MATCHOBJECT_NAME_LENGTH)) return;

		pMemberObjects[i] = GetPlayerByName(pNode->szName);

		if (!IsEnabledObject(pMemberObjects[i]))
		{
			RouteResponseToListener(pLeaderObject, MC_MATCH_LADDER_RESPONSE_CHALLENGE, MERR_LADDER_CANNOT_CHALLENGE);
			return;
		}
	}

	int nRet = ValidateChallengeLadderGame(pMemberObjects, nMemberCount);
	if (nRet != MOK)
	{
		RouteResponseToListener(pLeaderObject, MC_MATCH_LADDER_RESPONSE_CHALLENGE, nRet);
		return;
	}

	int nTeamID = 0;

	MBaseTeamGameStrategy* pTeamGameStrategy = NULL;

	pTeamGameStrategy = MBaseTeamGameStrategy::GetInstance(MGetServerConfig()->GetServerMode());
	if (pTeamGameStrategy)
	{
		nTeamID = pTeamGameStrategy->GetNewGroupID(pLeaderObject, pMemberObjects, nMemberCount);
	}
	if (nTeamID == 0) return;

	MLadderGroup* pGroup = GetLadderMgr()->CreateLadderGroup();
	pGroup->SetID(nTeamID);

	if (nOptions == 1)
	{
		pGroup->SetBalancedMatching(true);
	}
	else
	{
		pGroup->SetBalancedMatching(false);
	}
	if (nAntiLead == 1)
	{
		pGroup->SetAntiLeadMatching(true);
	}
	else
	{
		pGroup->SetAntiLeadMatching(false);
	}

	if (pTeamGameStrategy)
	{
		pTeamGameStrategy->SetLadderGroup(pGroup, pMemberObjects, nMemberCount);
	}

	for (int i = 0; i < nMemberCount; i++) {
		pGroup->AddPlayer(pMemberObjects[i]);
	}

	GetLadderMgr()->Challenge(pGroup);
}

void MMatchServer::OnLadderRequestCancelChallenge(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;
	if (pObj->GetLadderGroupID() == 0) return;

	GetLadderMgr()->CancelChallenge(pObj->GetLadderGroupID(), pObj->GetCharInfo()->m_szName);
}

void MMatchServer::OnRequestProposal(const MUID& uidProposer, const int nProposalMode, const int nRequestID,
	const int nReplierCount, void* pReplierNamesBlob)
{
	MMatchObject* pProposerObject = GetObject(uidProposer);
	if (!IsEnabledObject(pProposerObject)) return;

	if ((nReplierCount > MAX_REPLIER) || (nReplierCount < 0))
	{
		return;
	}

	if (!MGetServerConfig()->IsEnabledCreateLadderGame())
	{
		MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_PROPOSAL, MUID(0, 0));
		pNewCmd->AddParameter(new MCommandParameterInt(MERR_LADDER_NOT_SERVICE_TIME));
		pNewCmd->AddParameter(new MCommandParameterInt(nProposalMode));
		pNewCmd->AddParameter(new MCommandParameterInt(nRequestID));
		RouteToListener(pProposerObject, pNewCmd);
		return;
	}

	int nBlobCount = MGetBlobArrayCount(pReplierNamesBlob);
	if (nBlobCount != nReplierCount) return;

	MMatchObject* ppReplierObjects[MAX_REPLIER];

	for (int i = 0; i < nReplierCount; i++)
	{
		MTD_ReplierNode* pNode = (MTD_ReplierNode*)MGetBlobArrayElement(pReplierNamesBlob, i);
		if (pNode == NULL) return;
		if ((strlen(pNode->szName) <= 0) || (strlen(pNode->szName) >= MATCHOBJECT_NAME_LENGTH)) return;

		ppReplierObjects[i] = GetPlayerByName(pNode->szName);

		if (!IsEnabledObject(ppReplierObjects[i]))
		{
			MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_PROPOSAL, MUID(0, 0));
			pNewCmd->AddParameter(new MCommandParameterInt(MERR_NO_TARGET));
			pNewCmd->AddParameter(new MCommandParameterInt(nProposalMode));
			pNewCmd->AddParameter(new MCommandParameterInt(nRequestID));
			RouteToListener(pProposerObject, pNewCmd);

			return;
		}
	}

	int nRet = MERR_UNKNOWN;
	switch (nProposalMode)
	{
	case MPROPOSAL_LADDER_INVITE:
	{
		MLadderGameStrategy* pLadderGameStrategy = MLadderGameStrategy::GetInstance();
		nRet = pLadderGameStrategy->ValidateRequestInviteProposal(pProposerObject, ppReplierObjects, nReplierCount);
	}
	break;
	case MPROPOSAL_CLAN_INVITE:
	{
		MClanGameStrategy* pClanGameStrategy = MClanGameStrategy::GetInstance();
		nRet = pClanGameStrategy->ValidateRequestInviteProposal(pProposerObject, ppReplierObjects, nReplierCount);
	}
	break;
	};

	if (nRet != MOK)
	{
		MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_PROPOSAL, MUID(0, 0));
		pNewCmd->AddParameter(new MCommandParameterInt(nRet));
		pNewCmd->AddParameter(new MCommandParameterInt(nProposalMode));
		pNewCmd->AddParameter(new MCommandParameterInt(nRequestID));
		RouteToListener(pProposerObject, pNewCmd);
		return;
	}

	int nMemberCount = nReplierCount + 1;
	void* pBlobMembersNameArray = MMakeBlobArray(sizeof(MTD_ReplierNode), nMemberCount);

	MTD_ReplierNode* pProposerNode = (MTD_ReplierNode*)MGetBlobArrayElement(pBlobMembersNameArray, 0);
	strcpy(pProposerNode->szName, pProposerObject->GetCharInfo()->m_szName);

	for (int k = 0; k < nReplierCount; k++)
	{
		MTD_ReplierNode* pMemberNode = (MTD_ReplierNode*)MGetBlobArrayElement(pBlobMembersNameArray, k + 1);
		strcpy(pMemberNode->szName, ppReplierObjects[k]->GetCharInfo()->m_szName);
	}

	for (int i = 0; i < nReplierCount; i++)
	{
		MCommand* pNewCmd = CreateCommand(MC_MATCH_ASK_AGREEMENT, MUID(0, 0));
		pNewCmd->AddParameter(new MCommandParameterUID(uidProposer));
		pNewCmd->AddParameter(new MCommandParameterBlob(pBlobMembersNameArray, MGetBlobArraySize(pBlobMembersNameArray)));

		pNewCmd->AddParameter(new MCommandParameterInt(nProposalMode));
		pNewCmd->AddParameter(new MCommandParameterInt(nRequestID));
		RouteToListener(ppReplierObjects[i], pNewCmd);
	}
	MEraseBlobArray(pBlobMembersNameArray);

	MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_PROPOSAL, MUID(0, 0));
	pNewCmd->AddParameter(new MCommandParameterInt(nRet));
	pNewCmd->AddParameter(new MCommandParameterInt(nProposalMode));
	pNewCmd->AddParameter(new MCommandParameterInt(nRequestID));
	RouteToListener(pProposerObject, pNewCmd);
}

void MMatchServer::OnReplyAgreement(MUID& uidProposer, MUID& uidReplier, const char* szReplierName,
	const int nProposalMode, const int nRequestID, const bool bAgreement)
{
	MMatchObject* pProposerObject = GetObject(uidProposer);
	if (!IsEnabledObject(pProposerObject)) return;

	MCommand* pNewCmd = CreateCommand(MC_MATCH_REPLY_AGREEMENT, MUID(0, 0));
	pNewCmd->AddParameter(new MCommandParameterUID(uidProposer));
	pNewCmd->AddParameter(new MCommandParameterUID(uidReplier));
	pNewCmd->AddParameter(new MCommandParameterString(szReplierName));
	pNewCmd->AddParameter(new MCommandParameterInt(nProposalMode));
	pNewCmd->AddParameter(new MCommandParameterInt(nRequestID));
	pNewCmd->AddParameter(new MCommandParameterBool(bAgreement));

	RouteToListener(pProposerObject, pNewCmd);
}