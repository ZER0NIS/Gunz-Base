#include "stdafx.h"
#include "MTeamGameStrategy.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MMatchObject.h"
#include "MMatchStage.h"
#include "MMatchConfig.h"
#include "MCommandCommunicator.h"
#include "MMatchTransDataType.h"
#include "MDebug.h"
#include "MLadderMgr.h"
#include "MLadderGroup.h"
#include "MMatchLocale.h"

MBaseTeamGameStrategy* MBaseTeamGameStrategy::GetInstance(MMatchServerMode nServerMode)
{
	switch (nServerMode)
	{
	case MSM_LADDER:
		return MLadderGameStrategy::GetInstance();
	case MSM_CLAN:
		return MClanGameStrategy::GetInstance();
	default:
	{
	}
	}
	return NULL;
}

int MLadderGameStrategy::ValidateChallenge(MMatchObject** ppMemberObject, int nMemberCount)
{
	if (nMemberCount > MAX_LADDER_TEAM_MEMBER) return MERR_LADDER_NO_TEAM_MEMBER;
	int nCIDs[MAX_LADDER_TEAM_MEMBER];

	for (int i = 0; i < nMemberCount; i++)
	{
		if (!IsEnabledObject(ppMemberObject[i])) return MERR_LADDER_NO_TEAM_MEMBER;
		if (ppMemberObject[i]->IsLadderChallenging() != false) return MERR_LADDER_EXIST_CANNOT_CHALLENGE_MEMBER;

		nCIDs[i] = ppMemberObject[i]->GetCharInfo()->m_nCID;
	}

	int nTeamID = MMatchServer::GetInstance()->GetLadderTeamIDFromDB(nMemberCount, nCIDs, nMemberCount);
	if (nTeamID == 0) return MERR_LADDER_WRONG_TEAM_MEMBER;

	return MOK;
}

int MLadderGameStrategy::ValidateRequestInviteProposal(MMatchObject* pProposerObject, MMatchObject** ppReplierObjects,
	const int nReplierCount)
{
	int nRet = MERR_UNKNOWN;

	MMatchObject* ppTeamMemberObjects[MAX_REPLIER];
	int nTeamMemberCount = nReplierCount + 1;
	if (nTeamMemberCount <= MAX_LADDER_TEAM_MEMBER)
	{
		ppTeamMemberObjects[0] = pProposerObject;
		for (int i = 0; i < nReplierCount; i++)
		{
			ppTeamMemberObjects[i + 1] = ppReplierObjects[i];
		}
		nRet = ValidateChallenge(ppTeamMemberObjects, nTeamMemberCount);
	}

	return nRet;
}

int MLadderGameStrategy::GetNewGroupID(MMatchObject* pLeaderObject, MMatchObject** ppMemberObjects, int nMemberCount)
{
	int nTeamID = 0;

	int nCIDs[MAX_LADDER_TEAM_MEMBER];
	for (int i = 0; i < nMemberCount; i++)
	{
		nCIDs[i] = ppMemberObjects[i]->GetCharInfo()->m_nCID;
	}
	nTeamID = MMatchServer::GetInstance()->GetLadderTeamIDFromDB(nMemberCount, nCIDs, nMemberCount);

	int nRet = ValidateChallenge(ppMemberObjects, nMemberCount);
	if (nRet != MOK)
	{
		MMatchServer::GetInstance()->RouteResponseToListener(pLeaderObject, MC_MATCH_LADDER_RESPONSE_CHALLENGE, nRet);
		return 0;
	}

	return nTeamID;
}

void MLadderGameStrategy::SetStageLadderInfo(MMatchLadderTeamInfo* poutRedLadderInfo, MMatchLadderTeamInfo* poutBlueLadderInfo,
	MLadderGroup* pRedGroup, MLadderGroup* pBlueGroup)
{
	poutRedLadderInfo->nTID = pRedGroup->GetID();
	poutBlueLadderInfo->nTID = pBlueGroup->GetID();

	poutRedLadderInfo->nFirstMemberCount = (int)pRedGroup->GetLadderType();
	poutBlueLadderInfo->nFirstMemberCount = (int)pBlueGroup->GetLadderType();

	poutRedLadderInfo->nCLID = 0;
	poutBlueLadderInfo->nCLID = 0;
	poutRedLadderInfo->nCharLevel = 0;
	poutBlueLadderInfo->nCharLevel = 0;
	poutRedLadderInfo->nContPoint = 0;
	poutBlueLadderInfo->nContPoint = 0;
}

void MLadderGameStrategy::SavePointOnFinishGame(MMatchStage* pStage, MMatchTeam nWinnerTeam, bool bIsDrawGame,
	MMatchLadderTeamInfo* pRedLadderInfo, MMatchLadderTeamInfo* pBlueLadderInfo)
{
	int nWinnerTID = 0, nLoserTID = 0;

	if (bIsDrawGame == true)
	{
		nWinnerTID = pRedLadderInfo->nTID;
		nLoserTID = pBlueLadderInfo->nTID;
	}
	else if (nWinnerTeam == MMT_RED)
	{
		nWinnerTID = pRedLadderInfo->nTID;
		nLoserTID = pBlueLadderInfo->nTID;
	}
	else if (nWinnerTeam == MMT_BLUE)
	{
		nWinnerTID = pBlueLadderInfo->nTID;
		nLoserTID = pRedLadderInfo->nTID;
	}
	else
	{
	}

	if ((nWinnerTID == 0) || (nLoserTID == 0)) return;

	int nTeamMemberCount = pBlueLadderInfo->nFirstMemberCount;
}

int MLadderGameStrategy::GetRandomMap(int nTeamMember)
{
	MMatchConfig* pConfig = MMatchConfig::GetInstance();

	list<int> mapList;
	for (int i = 0; i < MMATCH_MAP_MAX; i++) {
		if (pConfig->IsEnableMap(MMATCH_MAP(i)))
		{
			if (i != (int)MMATCH_MAP_RELAYMAP)
				mapList.push_back(i);
		}
	}
	MTime time;
	int nRandomMapIndex = time.MakeNumber(0, (int)mapList.size() - 1);
	int nRandomMap = 0;

	list<int>::iterator mapItor = mapList.begin();
	for (int i = 0; i < nRandomMapIndex; i++) mapItor++;

	if (mapItor != mapList.end()) nRandomMap = (*mapItor);
	else nRandomMap = *mapList.begin();

	return nRandomMap;
}

int MLadderGameStrategy::GetPlayerWarsRandomMap(int nTeamMember)
{
	int nVecIndex = 0;
	int nMaxSize = 0;
	switch (nTeamMember)
	{
	case 1:
		nVecIndex = MLADDERTYPE_NORMAL_1VS1;
		break;
	case 2:
		nVecIndex = MLADDERTYPE_NORMAL_2VS2;
		break;
	case 3:
		nVecIndex = MLADDERTYPE_NORMAL_3VS3;
		break;
	case 4:
		nVecIndex = MLADDERTYPE_NORMAL_4VS4;
		break;
	case 5:
		nVecIndex = MLADDERTYPE_NORMAL_5VS5;
		break;
	case 6:
		nVecIndex = MLADDERTYPE_NORMAL_6VS6;
		break;
	case 7:
		nVecIndex = MLADDERTYPE_NORMAL_7VS7;
		break;
	case 8:
		nVecIndex = MLADDERTYPE_NORMAL_8VS8;
		break;
	};

	nMaxSize = (int)m_RandomMapVec[nVecIndex].size();

	int nRandomMapIndex = 0;
	int nRandomMap = 0;

	if (nMaxSize != 0) {
		nRandomMapIndex = rand() % nMaxSize;
		nRandomMap = m_RandomMapVec[nVecIndex][nRandomMapIndex];
	}

	return nRandomMap;
}

void InsertLadderRandomMap(vector<int>& vec, int nNum, int nCount)
{
	for (int i = 0; i < nCount; i++)
		vec.push_back(nNum);
}

MClanGameStrategy::MClanGameStrategy()
{
	for (int i = MLADDERTYPE_NORMAL_1VS1; i <= MLADDERTYPE_NORMAL_2VS2; i++)
	{
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_BATTLE_ARENA, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_TOWN, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_HALL, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_CASTLE, 10);
	}

	for (int i = MLADDERTYPE_NORMAL_2VS2; i <= MLADDERTYPE_NORMAL_3VS3; i++)
	{
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_MANSION, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_PRISON_II, 5);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_BATTLE_ARENA, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_TOWN, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_DUNGEON, 2);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_PORT, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_CASTLE, 5);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_ISLAND, 5);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_GARDEN, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_FACTORY, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_HIGH_HAVEN, 10);
	}

	for (int i = MLADDERTYPE_NORMAL_4VS4; i < MLADDERTYPE_MAX; i++)
	{
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_MANSION, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_PRISON_II, 5);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_BATTLE_ARENA, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_TOWN, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_DUNGEON, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_PORT, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_ISLAND, 5);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_GARDEN, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_CASTLE, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_FACTORY, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_HIGH_HAVEN, 10);
	}
	for (int i = MLADDERTYPE_NORMAL_5VS5; i < MLADDERTYPE_MAX; i++)
	{
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_MANSION, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_PRISON_II, 5);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_BATTLE_ARENA, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_TOWN, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_DUNGEON, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_PORT, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_ISLAND, 5);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_GARDEN, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_CASTLE, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_FACTORY, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_HIGH_HAVEN, 10);
	}
	for (int i = MLADDERTYPE_NORMAL_6VS6; i < MLADDERTYPE_MAX; i++)
	{
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_MANSION, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_PRISON_II, 5);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_BATTLE_ARENA, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_TOWN, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_DUNGEON, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_PORT, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_ISLAND, 5);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_GARDEN, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_CASTLE, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_FACTORY, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_HIGH_HAVEN, 10);
	}
	for (int i = MLADDERTYPE_NORMAL_7VS7; i < MLADDERTYPE_MAX; i++)
	{
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_MANSION, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_PRISON_II, 5);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_BATTLE_ARENA, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_TOWN, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_DUNGEON, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_PORT, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_ISLAND, 5);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_GARDEN, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_CASTLE, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_FACTORY, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_HIGH_HAVEN, 10);
	}
	for (int i = MLADDERTYPE_NORMAL_8VS8; i < MLADDERTYPE_MAX; i++)
	{
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_MANSION, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_PRISON_II, 5);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_BATTLE_ARENA, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_TOWN, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_DUNGEON, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_PORT, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_ISLAND, 5);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_GARDEN, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_CASTLE, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_FACTORY, 10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_HIGH_HAVEN, 10);
	}
}

int MClanGameStrategy::ValidateChallenge(MMatchObject** ppMemberObject, int nMemberCount)
{
	if ((nMemberCount < 0) || (nMemberCount > MAX_CLANBATTLE_TEAM_MEMBER)) return MERR_CB_NO_TEAM_MEMBER;

	bool bFit = false;
	for (int i = 0; i < MLADDERTYPE_MAX; i++)
	{
		if (nMemberCount == MMatchServer::GetInstance()->GetLadderMgr()->GetNeedMemberCount(MLADDERTYPE(i)))
		{
			bFit = true;
			break;
		}
	}
	if (!bFit) return MERR_CB_NO_TEAM_MEMBER;

	bool bAllSameClan = true;
	int nCLIDs[MAX_CLANBATTLE_TEAM_MEMBER] = { 0, };

	for (int i = 0; i < nMemberCount; i++)
	{
		if (!IsEnabledObject(ppMemberObject[i])) return MERR_CB_NO_TEAM_MEMBER;
		if (!ppMemberObject[i]->GetCharInfo()->m_ClanInfo.IsJoined()) return MERR_CB_WRONG_TEAM_MEMBER;
		if (ppMemberObject[i]->IsLadderChallenging() == true) return MERR_CB_EXIST_CANNOT_CHALLENGE_MEMBER;

		nCLIDs[i] = ppMemberObject[i]->GetCharInfo()->m_ClanInfo.m_nClanID;
	}

	for (int i = 0; i < nMemberCount - 1; i++)
	{
		for (int j = i + 1; j < nMemberCount; j++)
		{
			if (nCLIDs[i] != nCLIDs[j]) return MERR_CB_WRONG_TEAM_MEMBER;
		}
	}

	MUID uidLastChannel = ppMemberObject[0]->GetChannelUID();
	for (int i = 1; i < nMemberCount; i++)
	{
		if (ppMemberObject[i]->GetChannelUID() != uidLastChannel)
		{
			return MERR_LADDER_EXIST_CANNOT_CHALLENGE_MEMBER;
		}
		uidLastChannel = ppMemberObject[i]->GetChannelUID();
	}

	return MOK;
}

int MClanGameStrategy::ValidateRequestInviteProposal(MMatchObject* pProposerObject, MMatchObject** ppReplierObjects,
	const int nReplierCount)
{
	int nRet = MERR_UNKNOWN;

	MMatchObject* ppTeamMemberObjects[MAX_REPLIER];
	int nTeamMemberCount = nReplierCount + 1;
	if (nTeamMemberCount <= MAX_CLANBATTLE_TEAM_MEMBER)
	{
		ppTeamMemberObjects[0] = pProposerObject;
		for (int i = 0; i < nReplierCount; i++)
		{
			ppTeamMemberObjects[i + 1] = ppReplierObjects[i];
		}
		nRet = ValidateChallenge(ppTeamMemberObjects, nTeamMemberCount);
	}
	else
	{
		nRet = MERR_CB_WRONG_TEAM_MEMBER;
	}

	return nRet;
}

int MClanGameStrategy::GetNewGroupID(MMatchObject* pLeaderObject, MMatchObject** ppMemberObjects, int nMemberCount)
{
	return MMatchServer::GetInstance()->GetLadderMgr()->GenerateID();
}

void MClanGameStrategy::SetLadderGroup(MLadderGroup* pGroup, MMatchObject** ppMemberObjects, int nMemberCount)
{
	if (nMemberCount > 0)
	{
		pGroup->SetCLID(ppMemberObjects[0]->GetCharInfo()->m_ClanInfo.m_nClanID);
	}
}

void MClanGameStrategy::SetStageLadderInfo(MMatchLadderTeamInfo* poutRedLadderInfo, MMatchLadderTeamInfo* poutBlueLadderInfo,
	MLadderGroup* pRedGroup, MLadderGroup* pBlueGroup)
{
	poutRedLadderInfo->nTID = pRedGroup->GetID();
	poutBlueLadderInfo->nTID = pBlueGroup->GetID();

	poutRedLadderInfo->nCLID = pRedGroup->GetCLID();
	poutBlueLadderInfo->nCLID = pBlueGroup->GetCLID();

	poutRedLadderInfo->nFirstMemberCount = (int)pRedGroup->GetLadderType();
	poutBlueLadderInfo->nFirstMemberCount = (int)pBlueGroup->GetLadderType();

	poutRedLadderInfo->nCharLevel = pRedGroup->GetCharLevel();
	poutBlueLadderInfo->nCharLevel = pBlueGroup->GetCharLevel();

	poutRedLadderInfo->nContPoint = pRedGroup->GetContPoint();
	poutBlueLadderInfo->nContPoint = pBlueGroup->GetContPoint();
}

void MClanGameStrategy::SavePointOnFinishGame(MMatchStage* pStage, MMatchTeam nWinnerTeam, bool bIsDrawGame,
	MMatchLadderTeamInfo* pRedLadderInfo, MMatchLadderTeamInfo* pBlueLadderInfo)
{
	int nWinnerCLID = 0, nLoserCLID = 0;

	MMatchTeam nLoserTeam = (nWinnerTeam == MMT_RED) ? MMT_BLUE : MMT_RED;

	if (bIsDrawGame == true)
	{
		nWinnerCLID = pRedLadderInfo->nCLID;
		nLoserCLID = pBlueLadderInfo->nCLID;
	}
	else if (nWinnerTeam == MMT_RED)
	{
		nWinnerCLID = pRedLadderInfo->nCLID;
		nLoserCLID = pBlueLadderInfo->nCLID;
	}
	else if (nWinnerTeam == MMT_BLUE)
	{
		nWinnerCLID = pBlueLadderInfo->nCLID;
		nLoserCLID = pRedLadderInfo->nCLID;
	}
	else
	{
	}

	if ((nWinnerCLID == 0) || (nLoserCLID == 0)) return;

	MMatchClan* pWinnerClan = MMatchServer::GetInstance()->GetClanMap()->GetClan(nWinnerCLID);
	MMatchClan* pLoserClan = MMatchServer::GetInstance()->GetClanMap()->GetClan(nLoserCLID);

	if ((!pWinnerClan) || (!pLoserClan)) return;

	int nFirstMemberCount = pRedLadderInfo->nFirstMemberCount;

	char szWinnerMembers[512] = "";
	char szLoserMembers[512] = "";
	list<MUID>		WinnerObjUIDs;

	for (MUIDRefCache::iterator itor = pStage->GetObjBegin(); itor != pStage->GetObjEnd(); itor++)
	{
		MMatchObject* pObj = (MMatchObject*)(*itor).second;
		if (IsEnabledObject(pObj))
		{
			if (pObj->GetTeam() == nWinnerTeam)
			{
				WinnerObjUIDs.push_back(pObj->GetUID());
				strcat(szWinnerMembers, pObj->GetCharInfo()->m_szName);
				strcat(szWinnerMembers, " ");
			}
			else
			{
				strcat(szLoserMembers, pObj->GetCharInfo()->m_szName);
				strcat(szLoserMembers, " ");
			}
		}
	}

	int nMapID = pStage->GetStageSetting()->GetMapIndex();
	int nGameType = (int)pStage->GetStageType();
	int nRoundWins = pStage->GetTeamScore(nWinnerTeam);
	int nRoundLosses = pStage->GetTeamScore(nLoserTeam);

	int nLoserSeriesOfVictories = pLoserClan->GetSeriesOfVictories();
	float fPointRatio = 1.0f;

	if (!bIsDrawGame)
	{
		MLadderStatistics* pLS = MMatchServer::GetInstance()->GetLadderMgr()->GetStatistics();
		pLS->InsertLevelRecord(pRedLadderInfo->nCharLevel, pBlueLadderInfo->nCharLevel, nWinnerTeam);
		pLS->InsertContPointRecord(pRedLadderInfo->nContPoint, pBlueLadderInfo->nContPoint, nWinnerTeam);
		pLS->InsertClanPointRecord(pWinnerClan->GetClanInfoEx()->nPoint, pLoserClan->GetClanInfoEx()->nPoint, MMT_RED);

		int nWinnerSeriesOfVictories = pWinnerClan->GetSeriesOfVictories();

		if (nLoserSeriesOfVictories >= 10)
		{
			MMatchServer::GetInstance()->BroadCastClanInterruptVictories(pWinnerClan->GetName(), pLoserClan->GetName(),
				nLoserSeriesOfVictories + 1);

			fPointRatio = 2.0f;
		}
		else if ((nWinnerSeriesOfVictories == 3) || (nWinnerSeriesOfVictories == 5) ||
			(nWinnerSeriesOfVictories == 7) || (nWinnerSeriesOfVictories >= 10))
		{
			MMatchServer::GetInstance()->BroadCastClanRenewVictories(pWinnerClan->GetName(), pLoserClan->GetName(),
				nWinnerSeriesOfVictories);
		}
	}

	MMatchServer::GetInstance()->SaveClanPoint(pWinnerClan, pLoserClan, bIsDrawGame,
		nRoundWins, nRoundLosses, nMapID, nGameType,
		nFirstMemberCount, WinnerObjUIDs,
		szWinnerMembers, szLoserMembers, fPointRatio);
}

int MClanGameStrategy::GetPlayerWarsRandomMap(int nTeamMember)
{
	int nVecIndex = 0;
	int nMaxSize = 0;
	switch (nTeamMember)
	{
	case 1:
		nVecIndex = MLADDERTYPE_NORMAL_1VS1;
		break;
	case 2:
		nVecIndex = MLADDERTYPE_NORMAL_2VS2;
		break;
	case 3:
		nVecIndex = MLADDERTYPE_NORMAL_3VS3;
		break;
	case 4:
		nVecIndex = MLADDERTYPE_NORMAL_4VS4;
		break;
	case 5:
		nVecIndex = MLADDERTYPE_NORMAL_5VS5;
		break;
	case 6:
		nVecIndex = MLADDERTYPE_NORMAL_6VS6;
		break;
	case 7:
		nVecIndex = MLADDERTYPE_NORMAL_7VS7;
		break;
	case 8:
		nVecIndex = MLADDERTYPE_NORMAL_8VS8;
		break;
	};

	nMaxSize = (int)m_RandomMapVec[nVecIndex].size();

	int nRandomMapIndex = 0;
	int nRandomMap = 0;

	if (nMaxSize != 0) {
		nRandomMapIndex = rand() % nMaxSize;
		nRandomMap = m_RandomMapVec[nVecIndex][nRandomMapIndex];
	}

	return nRandomMap;
}

int MClanGameStrategy::GetRandomMap(int nTeamMember)
{
	int nVecIndex = 0;
	int nMaxSize = 0;
	switch (nTeamMember)
	{
	case 1:
		nVecIndex = MLADDERTYPE_NORMAL_1VS1;
		break;
	case 2:
		nVecIndex = MLADDERTYPE_NORMAL_2VS2;
		break;
	case 3:
		nVecIndex = MLADDERTYPE_NORMAL_3VS3;
		break;
	case 4:
		nVecIndex = MLADDERTYPE_NORMAL_4VS4;
		break;
	case 5:
		nVecIndex = MLADDERTYPE_NORMAL_5VS5;
		break;
	case 6:
		nVecIndex = MLADDERTYPE_NORMAL_6VS6;
		break;
	case 7:
		nVecIndex = MLADDERTYPE_NORMAL_7VS7;
		break;
	case 8:
		nVecIndex = MLADDERTYPE_NORMAL_8VS8;
		break;
	};

	nMaxSize = (int)m_RandomMapVec[nVecIndex].size();

	int nRandomMapIndex = 0;
	int nRandomMap = 0;

	if (nMaxSize != 0) {
		nRandomMapIndex = rand() % nMaxSize;
		nRandomMap = m_RandomMapVec[nVecIndex][nRandomMapIndex];
	}

	return nRandomMap;
}