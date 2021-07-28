#include "stdafx.h"
#include "MMatchServer.h"
#include "MMatchStage.h"
#include "MSharedCommandTable.h"
#include "MDebug.h"
#include "MMatchConfig.h"
#include "MTeamGameStrategy.h"
#include "MLadderGroup.h"
#include "MBlobArray.h"
#include "MMatchRuleQuest.h"
#include "MMatchRuleDeathMatch.h"
#include "MMatchRuleGladiator.h"
#include "MMatchRuleAssassinate.h"
#include "MMatchRuleTraining.h"
#include "MMatchRuleSurvival.h"
#include "MMatchRuleBerserker.h"
#include "MMatchRuleDuel.h"
#include "MMatchRuleDuelTournament.h"
#include "MMatchCRC32XORCache.h"

MMatchStage::MMatchStage()
{
	m_pRule = NULL;
	m_nIndex = 0;
	m_nStageType = MST_NORMAL;
	m_uidOwnerChannel = MUID(0, 0);
	m_TeamBonus.bApplyTeamBonus = false;
	m_nAdminObjectCount = 0;

	m_nStateTimer = 0;
	m_nLastTick = 0;
	m_nChecksum = 0;
	m_nLastChecksumTick = 0;
	m_nAdminObjectCount = 0;
	m_nStartTime = 0;
	m_nLastRequestStartStageTime = 0;
	m_dwLastResourceCRC32CacheCheckTime = 0;
	m_bIsUseResourceCRC32CacheCheck = false;

	m_vecRelayMapsRemained.clear();
	m_RelayMapType = RELAY_MAP_TURN;
	m_RelayMapRepeatCountRemained = RELAY_MAP_3REPEAT;
	m_bIsLastRelayMap = false;
	memset(m_Teams, 0, sizeof(m_Teams));
}
MMatchStage::~MMatchStage()
{
}

const bool MMatchStage::SetChannelRuleForCreateStage(bool bIsAllowNullChannel)
{
	MMatchChannel* pChannel = MGetMatchServer()->GetChannelMap()->Find(m_uidOwnerChannel);
	if (NULL == pChannel)
	{
		if (MSM_CLAN != MGetServerConfig()->GetServerMode()
			&& bIsAllowNullChannel == false) {
			return false;
		}

		ChangeRule(MMATCH_GAMETYPE_DEFAULT);
		return true;
	}

	MChannelRule* pChannelRule = MGetChannelRuleMgr()->GetRule(pChannel->GetRuleType());
	if (NULL == pChannelRule)
	{
		return false;
	}

	if (pChannelRule->CheckGameType(MMATCH_GAMETYPE_DEFAULT))
	{
		ChangeRule(MMATCH_GAMETYPE_DEFAULT);
	}
	else
	{
		const int nGameType = pChannelRule->GetGameTypeList()->GetFirstGameType();
		if (-1 == nGameType)
		{
			return false;
		}

		ChangeRule(MMATCH_GAMETYPE(nGameType));
	}

	return true;
}

bool MMatchStage::Create(const MUID& uid, const char* pszName, bool bPrivate, const char* pszPassword, bool bIsAllowNullChannel,
	const MMATCH_GAMETYPE GameType, const bool bIsCheckTicket, const DWORD dwTicketItemID)
{
	if ((strlen(pszName) >= STAGENAME_LENGTH) || (strlen(pszPassword) >= STAGENAME_LENGTH)) return false;

	m_nStageType = MST_NORMAL;
	m_uidStage = uid;
	strcpy(m_szStageName, pszName);
	strcpy(m_szStagePassword, pszPassword);
	m_bPrivate = bPrivate;

	ChangeState(STAGE_STATE_STANDBY);

	if (!SetChannelRuleForCreateStage(bIsAllowNullChannel))
	{
		return false;
	}

	SetAgentUID(MUID(0, 0));
	SetAgentReady(false);

	m_nChecksum = 0;
	m_nLastChecksumTick = 0;
	m_nAdminObjectCount = 0;

	m_WorldItemManager.Create(this);
	m_ActiveTrapManager.Create(this);

	SetFirstMasterName("");

	m_StageSetting.SetIsCheckTicket(bIsCheckTicket);
	m_StageSetting.SetTicketItemID(dwTicketItemID);

	return true;
}

void MMatchStage::Destroy()
{
	MUIDRefCache::iterator itor = GetObjBegin();
	while (itor != GetObjEnd()) {
		MUID uidPlayer = (*itor).first;
		itor = RemoveObject(uidPlayer);
	}
	m_ObjUIDCaches.clear();

	m_WorldItemManager.Destroy();
	m_ActiveTrapManager.Destroy();

	if (m_pRule != NULL)
	{
		delete m_pRule;
		m_pRule = NULL;
	}

	ClearDuelTournamentMatchMap();
}

bool MMatchStage::IsChecksumUpdateTime(unsigned long nTick)
{
	if (nTick - m_nLastChecksumTick > CYCLE_STAGE_UPDATECHECKSUM)
		return true;
	else
		return false;
}

void MMatchStage::UpdateChecksum(unsigned long nTick)
{
	m_nChecksum = (m_nIndex +
		GetState() +
		m_StageSetting.GetChecksum() +
		(unsigned long)m_ObjUIDCaches.size());

	m_nLastChecksumTick = nTick;
}

void MMatchStage::UpdateStateTimer()
{
	m_nStateTimer = MMatchServer::GetInstance()->GetGlobalClockCount();
}

void MMatchStage::AddBanList(int nCID)
{
	if (CheckBanList(nCID))
		return;

	m_BanCIDList.push_back(nCID);
}

bool MMatchStage::CheckBanList(int nCID)
{
	list<int>::iterator i = find(m_BanCIDList.begin(), m_BanCIDList.end(), nCID);
	if (i != m_BanCIDList.end())
		return true;
	else
		return false;
}

void MMatchStage::AddObject(const MUID& uid, const MMatchObject* pObj)
{
	m_ObjUIDCaches.Insert(uid, (void*)pObj);

	MMatchObject* pObject = (MMatchObject*)pObj;
	if (IsEnabledObject(pObject))
	{
		if (IsAdminGrade(pObject->GetAccountInfo()->m_nUGrade))
		{
			m_nAdminObjectCount++;
		}

		if (GetStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUELTOURNAMENT) {
			pObject->SetJoinDuelTournament(true);
		}
	}

	if (GetObjCount() == 1)
	{
		SetMasterUID(uid);
	}

	m_VoteMgr.AddVoter(uid);

	pObject->ResetCustomItemUseCount();
}

MUIDRefCache::iterator MMatchStage::RemoveObject(const MUID& uid)
{
	m_VoteMgr.RemoveVoter(uid);
	if (CheckUserWasVoted(uid))
	{
		m_VoteMgr.StopVote(uid);
	}

	MUIDRefCache::iterator i = m_ObjUIDCaches.find(uid);
	if (i == m_ObjUIDCaches.end())
	{
		return i;
	}

	MMatchObject* pObj = MMatchServer::GetInstance()->GetObject(uid);
	if (pObj) {
		if (IsAdminGrade(pObj->GetAccountInfo()->m_nUGrade))
		{
			m_nAdminObjectCount--;
			if (m_nAdminObjectCount < 0) m_nAdminObjectCount = 0;
		}

		LeaveBattle(pObj);

		pObj->SetStageUID(MUID(0, 0));
		pObj->SetForcedEntry(false);
		pObj->SetPlace(MMP_LOBBY);
		pObj->SetStageListTransfer(true);

		if (GetStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUELTOURNAMENT) {
			pObj->SetJoinDuelTournament(false);
			MMatchServer::GetInstance()->SendDuelTournamentCharInfoToPlayer(uid);
		}

		MMatchServer::GetInstance()->SaveGamePlayerLog(pObj, m_nGameLogID);
	}

	MUIDRefCache::iterator itorNext = m_ObjUIDCaches.erase(i);

	if (m_ObjUIDCaches.empty())
		ChangeState(STAGE_STATE_CLOSE);
	else
	{
		if (uid == GetMasterUID())
		{
			if ((GetState() == STAGE_STATE_RUN) && (GetObjInBattleCount() > 0))
				SetMasterUID(RecommandMaster(true));
			else
				SetMasterUID(RecommandMaster(false));
		}
	}

	DeleteResourceCRC32Cache(uid);

	return itorNext;
}

bool MMatchStage::KickBanPlayer(const char* pszName, bool bBanPlayer)
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	for (MUIDRefCache::iterator i = m_ObjUIDCaches.begin(); i != m_ObjUIDCaches.end(); i++)
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if (pObj->GetCharInfo() == NULL)
			continue;
		if (_stricmp(pObj->GetCharInfo()->m_szName, pszName) == 0) {
			if (bBanPlayer)
				AddBanList(pObj->GetCharInfo()->m_nCID);

			pServer->StageLeaveBattle(pObj->GetUID(), true, true);
			pServer->StageLeave(pObj->GetUID());
			return true;
		}
	}
	return false;
}

const MUID MMatchStage::RecommandMaster(bool bInBattleOnly)
{
	for (MUIDRefCache::iterator i = m_ObjUIDCaches.begin(); i != m_ObjUIDCaches.end(); i++)
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if (bInBattleOnly && (pObj->GetEnterBattle() == false))
			continue;
		return (*i).first;
	}
	return MUID(0, 0);
}

void MMatchStage::EnterBattle(MMatchObject* pObj)
{
	pObj->OnEnterBattle();

	if (GetState() == STAGE_STATE_RUN)
	{
		if (pObj->IsForcedEntried())
		{
			if (m_StageSetting.IsWaitforRoundEnd())
			{
				pObj->SetAlive(false);
			}

			m_WorldItemManager.RouteAllItems(pObj);
			m_ActiveTrapManager.RouteAllTraps(pObj);

			MMatchServer::GetInstance()->ResponseRoundState(pObj, GetUID());
		}

		if (m_pRule)
		{
			MUID uidChar = pObj->GetUID();
			m_pRule->OnEnterBattle(uidChar);
		}
	}

	pObj->SetForcedEntry(false);
	pObj->ResetCustomItemUseCount();

	RequestResourceCRC32Cache(pObj->GetUID());
}

void MMatchStage::LeaveBattle(MMatchObject* pObj)
{
	if ((GetState() == STAGE_STATE_RUN) && (m_pRule))
	{
		MUID uidPlayer = pObj->GetUID();
		m_pRule->OnLeaveBattle(uidPlayer);
	}

	pObj->OnLeaveBattle();

	SetDisableCheckResourceCRC32Cache(pObj->GetUID());
}

bool MMatchStage::CheckTick(unsigned long nClock)
{
	if (nClock - m_nLastTick < MTICK_STAGE) return false;
	return true;
}

void MMatchStage::Tick(unsigned long nClock)
{
	ClearGabageObject();

	switch (GetState())
	{
	case STAGE_STATE_STANDBY:
	{
	}
	break;
	case STAGE_STATE_COUNTDOWN:
	{
		OnStartGame();
		ChangeState(STAGE_STATE_RUN);
	}
	break;
	case STAGE_STATE_RUN:
	{
		if (m_pRule)
		{
			m_WorldItemManager.Update();
			m_ActiveTrapManager.Update(nClock);

			CheckSuicideReserve(nClock);

			if (m_pRule->Run() == false)
			{
				OnFinishGame();

				if (GetStageType() == MST_NORMAL && m_pRule->GetGameType() != MMATCH_GAMETYPE_DUELTOURNAMENT)
					ChangeState(STAGE_STATE_STANDBY);
				else
					ChangeState(STAGE_STATE_CLOSE);
			}
		}
	}
	break;
	}

	m_VoteMgr.Tick(nClock);

	if (IsChecksumUpdateTime(nClock))
		UpdateChecksum(nClock);

	m_nLastTick = nClock;

	if ((m_ObjUIDCaches.empty()) && (GetState() != STAGE_STATE_CLOSE))
	{
		ChangeState(STAGE_STATE_CLOSE);
	}

	CheckResourceCRC32Cache(nClock);
}

MMatchRule* MMatchStage::CreateRule(MMATCH_GAMETYPE nGameType)
{
	switch (nGameType)
	{
	case MMATCH_GAMETYPE_DEATHMATCH_SOLO:
	{
		return (new MMatchRuleSoloDeath(this));
	}
	break;
	case MMATCH_GAMETYPE_DEATHMATCH_TEAM:
	{
		return (new MMatchRuleTeamDeath(this));
	}
	break;
	case MMATCH_GAMETYPE_GLADIATOR_SOLO:
	{
		return (new MMatchRuleSoloGladiator(this));
	}
	break;
	case MMATCH_GAMETYPE_GLADIATOR_TEAM:
	{
		return (new MMatchRuleTeamGladiator(this));
	}
	break;
	case MMATCH_GAMETYPE_ASSASSINATE:
	{
		return (new MMatchRuleAssassinate(this));
	}
	break;
	case MMATCH_GAMETYPE_TRAINING:
	{
		return (new MMatchRuleTraining(this));
	}
	break;
	case MMATCH_GAMETYPE_SURVIVAL:
	{
		return (new MMatchRuleSurvival(this));
	}
	break;
	case MMATCH_GAMETYPE_QUEST:
	{
		return (new MMatchRuleQuest(this));
	}
	break;
	case MMATCH_GAMETYPE_BERSERKER:
	{
		return (new MMatchRuleBerserker(this));
	}
	break;
	case MMATCH_GAMETYPE_DEATHMATCH_TEAM2:
	{
		return (new MMatchRuleTeamDeath2(this));
	}
	break;
	case MMATCH_GAMETYPE_DUEL:
	{
		return (new MMatchRuleDuel(this));
	}
	break;
	case MMATCH_GAMETYPE_DUELTOURNAMENT:
	{
		return (new MMatchRuleDuelTournament(this));
	}
	break;
	case MMATCH_GAMETYPE_CTF:
	{
		return (new MMatchRuleTeamCTF(this));
	}
	break;

	default:
	{
	}
	}
	return NULL;
}

void MMatchStage::ChangeRule(MMATCH_GAMETYPE nRule)
{
	if ((m_pRule) && (m_pRule->GetGameType() == nRule)) return;

	if ((nRule < 0) || (nRule >= MMATCH_GAMETYPE_MAX))
	{
		MMatchServer::GetInstance()->LOG(MMatchServer::LOG_DEBUG, "ChangeRule Failed(%d)", nRule);
		return;
	}

	if (m_pRule)
	{
		delete m_pRule;
		m_pRule = NULL;
	}

	m_pRule = CreateRule(nRule);
}

MMatchTeam MMatchStage::GetRecommandedTeam()
{
	int nRed, nBlue;
	GetTeamMemberCount(&nRed, &nBlue, NULL, false);

	if (nRed <= nBlue)
		return MMT_RED;
	else
		return MMT_BLUE;
}

void MMatchStage::PlayerTeam(const MUID& uidPlayer, MMatchTeam nTeam)
{
	MUIDRefCache::iterator i = m_ObjUIDCaches.find(uidPlayer);
	if (i == m_ObjUIDCaches.end())
		return;

	MMatchObject* pObj = (MMatchObject*)(*i).second;
	pObj->SetTeam(nTeam);

	MMatchStageSetting* pSetting = GetStageSetting();
	pSetting->UpdateCharSetting(uidPlayer, nTeam, pObj->GetStageState());
}

void MMatchStage::PlayerState(const MUID& uidPlayer, MMatchObjectStageState nStageState)
{
	MUIDRefCache::iterator i = m_ObjUIDCaches.find(uidPlayer);
	if (i == m_ObjUIDCaches.end())
		return;

	MMatchObject* pObj = (MMatchObject*)(*i).second;

	pObj->SetStageState(nStageState);

	MMatchStageSetting* pSetting = GetStageSetting();
	pSetting->UpdateCharSetting(uidPlayer, pObj->GetTeam(), pObj->GetStageState());
}

bool MMatchStage::StartGame(const bool bIsUseResourceCRC32CacheCheck)
{
	if (STAGE_STATE_STANDBY != GetState()) return false;

	int nPlayer = GetCountableObjCount();
	if (nPlayer > m_StageSetting.GetMaxPlayers())
	{
		char szMsg[256];
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_PERSONNEL_TOO_MUCH);

		MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_ANNOUNCE, MUID(0, 0));
		pCmd->AddParameter(new MCmdParamUInt(0));
		pCmd->AddParameter(new MCmdParamStr(szMsg));
		MMatchServer::GetInstance()->RouteToStage(GetUID(), pCmd);

		return false;
	}

	bool bResult = true;
	bool bNotReadyExist = false;
	MUID uidItem;

	for (MUIDRefCache::iterator i = m_ObjUIDCaches.begin(); i != m_ObjUIDCaches.end(); i++)
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if (NULL == pObj) continue;
		if (!CheckTicket(pObj)) return false;

		if ((GetMasterUID() != (*i).first) && (pObj->GetStageState() != MOSS_READY))
		{
			if (IsAdminGrade(pObj) && pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide))
				continue;

			bNotReadyExist = true;
			bResult = false;

			const char* szName = NULL;

			szName = pObj->GetName();

			char szSend[256];
			sprintf(szSend, "%s%d\a%s", MTOK_ANNOUNCE_PARAMSTR, MERR_HE_IS_NOT_READY, szName);

			MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_ANNOUNCE, MUID(0, 0));
			pCmd->AddParameter(new MCmdParamUInt(0));
			pCmd->AddParameter(new MCmdParamStr(szSend));
			MMatchServer::GetInstance()->RouteToStage(GetUID(), pCmd);
		}
	}

	if (bNotReadyExist)
	{
		MCommand* pCmdNotReady = MMatchServer::GetInstance()->CreateCommand(MC_GAME_START_FAIL, MUID(0, 0));
		if (0 == pCmdNotReady) {
			mlog("MMatchStage::StartGame - 커맨드 생성 실패.\n");
			bResult = false;
		}

		pCmdNotReady->AddParameter(new MCmdParamInt(ALL_PLAYER_NOT_READY));
		pCmdNotReady->AddParameter(new MCmdParamUID(MUID(0, 0)));

		MMatchObject* pMaster = MMatchServer::GetInstance()->GetObject(GetMasterUID());
		if (IsEnabledObject(pMaster)) {
			MMatchServer::GetInstance()->RouteToListener(pMaster, pCmdNotReady);
		}
		else {
			delete pCmdNotReady;
			bResult = false;
		}
	}

	if (!CheckQuestGame()) return false;

	if (MIN_REQUEST_STAGESTART_TIME > (MMatchServer::GetInstance()->GetTickTime() - m_nLastRequestStartStageTime)) return false;
	m_nLastRequestStartStageTime = MMatchServer::GetInstance()->GetTickTime();

	MMatchObject* pMasterObj = MMatchServer::GetInstance()->GetObject(GetMasterUID());
	if (pMasterObj && IsAdminGrade(pMasterObj) && pMasterObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide))
		bResult = true;

	if (bResult == true) {
		for (MUIDRefCache::iterator i = m_ObjUIDCaches.begin(); i != m_ObjUIDCaches.end(); i++) {
			MMatchObject* pObj = (MMatchObject*)(*i).second;
			pObj->SetLaunchedGame(true);
		}

		ChangeState(STAGE_STATE_COUNTDOWN);
	}

	m_bIsUseResourceCRC32CacheCheck = bIsUseResourceCRC32CacheCheck;

	return bResult;
}

bool MMatchStage::StartRelayGame(const bool bIsUseResourceCRC32CacheCheck)
{
	if (STAGE_STATE_STANDBY != GetState()) return false;

	int nPlayer = GetCountableObjCount();
	if (nPlayer > m_StageSetting.GetMaxPlayers()) {
		char szMsg[256];
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_PERSONNEL_TOO_MUCH);

		MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_ANNOUNCE, MUID(0, 0));
		pCmd->AddParameter(new MCmdParamUInt(0));
		pCmd->AddParameter(new MCmdParamStr(szMsg));
		MMatchServer::GetInstance()->RouteToStage(GetUID(), pCmd);

		return false;
	}

	bool bResult = true;
	bool bNotReadyExist = false;
	MUID uidItem;

	for (MUIDRefCache::iterator i = m_ObjUIDCaches.begin(); i != m_ObjUIDCaches.end(); i++)
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if (NULL == pObj) continue;
		if (!CheckTicket(pObj)) return false;
	}

	MMatchObject* pMasterObj = MMatchServer::GetInstance()->GetObject(GetMasterUID());
	if (pMasterObj && IsAdminGrade(pMasterObj) && pMasterObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide))
		bResult = true;

	if (bResult == true) {
		for (MUIDRefCache::iterator i = m_ObjUIDCaches.begin(); i != m_ObjUIDCaches.end(); i++) {
			MMatchObject* pObj = (MMatchObject*)(*i).second;
			if (pObj->GetStageState() == MOSS_READY)
				pObj->SetLaunchedGame(true);
		}

		ChangeState(STAGE_STATE_COUNTDOWN);
	}

	m_bIsUseResourceCRC32CacheCheck = bIsUseResourceCRC32CacheCheck;

	return bResult;
}

void MMatchStage::SetStageType(MMatchStageType nStageType)
{
	if (m_nStageType == nStageType) return;

	switch (nStageType)
	{
	case MST_NORMAL:
	{
		m_StageSetting.SetTeamWinThePoint(false);
	}
	break;
	case MST_LADDER:
	{
		m_StageSetting.SetTeamWinThePoint(true);
	}
	break;
	}

	m_nStageType = nStageType;
}

void MMatchStage::OnStartGame()
{
	for (MUIDRefCache::iterator i = m_ObjUIDCaches.begin(); i != m_ObjUIDCaches.end(); i++)
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		pObj->SetAllRoundDeathCount(0);
		pObj->SetAllRoundKillCount(0);
		pObj->SetVoteState(false);
	}

	for (int i = 0; i < MMT_END; i++)
	{
		m_Teams[i].nScore = 0;
		m_Teams[i].nSeriesOfVictories = 0;
	}

	if (m_pRule) {
		m_pRule->Begin();
	}

	m_nStartTime = MMatchServer::GetInstance()->GetTickTime();
	m_WorldItemManager.OnStageBegin(&m_StageSetting);
	m_ActiveTrapManager.Clear();

	if (GetStageType() == MST_NORMAL) {
		if (IsRelayMap() && IsStartRelayMap()) {
			MMatchServer::GetInstance()->StageRelayLaunch(GetUID());
		}
		else {
			MMatchServer::GetInstance()->StageLaunch(GetUID());
		}
	}
}

void MMatchStage::OnFinishGame()
{
	m_WorldItemManager.OnStageEnd();
	m_ActiveTrapManager.Clear();

	if (m_pRule)
	{
		m_pRule->End();
	}
	MMatchServer::GetInstance()->StageFinishGame(GetUID());

	if ((MGetServerConfig()->GetServerMode() == MSM_LADDER) || (MGetServerConfig()->GetServerMode() == MSM_CLAN))
	{
		if ((m_nStageType == MST_LADDER) && (GetStageSetting()->IsTeamPlay()))
		{
			MMatchTeam nWinnerTeam = MMT_RED;
			bool bIsDrawGame = false;
			int nRedTeamCount = 0, nBlueTeamCount = 0;

			GetTeamMemberCount(&nRedTeamCount, &nBlueTeamCount, NULL, true);

			if (nBlueTeamCount == 0 || nRedTeamCount == 0)
			{
				if ((nBlueTeamCount == 0 && (m_Teams[MMT_BLUE].nScore > m_Teams[MMT_RED].nScore)) ||
					(nRedTeamCount == 0 && (m_Teams[MMT_RED].nScore > m_Teams[MMT_BLUE].nScore)))
				{
					m_Teams[MMT_RED].nScore = m_Teams[MMT_BLUE].nScore = 0;
					bIsDrawGame = true;
				}
				else if ((m_Teams[MMT_RED].nScore > m_Teams[MMT_BLUE].nScore))
				{
					nWinnerTeam = MMT_RED;
				}
				else if ((m_Teams[MMT_RED].nScore < m_Teams[MMT_BLUE].nScore))
				{
					nWinnerTeam = MMT_BLUE;
				}
				else
				{
					bIsDrawGame = true;
				}
			}
			else if ((m_Teams[MMT_RED].nScore > m_Teams[MMT_BLUE].nScore))
			{
				nWinnerTeam = MMT_RED;
			}
			else if ((m_Teams[MMT_RED].nScore < m_Teams[MMT_BLUE].nScore))
			{
				nWinnerTeam = MMT_BLUE;
			}
			else
			{
				bIsDrawGame = true;
			}

			MBaseTeamGameStrategy* pTeamGameStrategy = MBaseTeamGameStrategy::GetInstance(MGetServerConfig()->GetServerMode());
			if (pTeamGameStrategy)
			{
				pTeamGameStrategy->SavePointOnFinishGame(this, nWinnerTeam, bIsDrawGame, &m_Teams[MMT_RED].LadderInfo,
					&m_Teams[MMT_BLUE].LadderInfo);
			};
		}
	}

	for (MUIDRefCache::iterator i = m_ObjUIDCaches.begin(); i != m_ObjUIDCaches.end(); i++) {
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if (pObj->GetStageState()) pObj->SetStageState(MOSS_NONREADY);
		pObj->SetLaunchedGame(false);
	}

	SetDisableAllCheckResourceCRC32Cache();

	m_nStartTime = 0;
}

bool MMatchStage::CheckBattleEntry()
{
	bool bResult = true;
	for (MUIDRefCache::iterator i = m_ObjUIDCaches.begin(); i != m_ObjUIDCaches.end(); i++) {
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if (pObj->IsLaunchedGame())
		{
			if (pObj->GetEnterBattle() == false) bResult = false;
		}
	}
	return bResult;
}

void MMatchStage::RoundStateFromClient(const MUID& uidStage, int nState, int nRound)
{
}

int MMatchStage::GetObjInBattleCount()
{
	int nCount = 0;
	for (MUIDRefCache::iterator itor = GetObjBegin(); itor != GetObjEnd(); ++itor)
	{
		MMatchObject* pObj = (MMatchObject*)(*itor).second;
		if (pObj->GetEnterBattle() == true)
		{
			nCount++;
		}
	}

	return nCount;
}

void MMatchStage::SetOwnerChannel(MUID& uidOwnerChannel, int nIndex)
{
	m_uidOwnerChannel = uidOwnerChannel;
	m_nIndex = nIndex;
}

void MMatchStage::ObtainWorldItem(MMatchObject* pObj, const int nItemUID)
{
	if (GetState() != STAGE_STATE_RUN) return;

	int nItemID = 0;
	int nExtraValues[WORLDITEM_EXTRAVALUE_NUM];

	if (m_WorldItemManager.Obtain(pObj, short(nItemUID), &nItemID, nExtraValues))
	{
		if (m_pRule)
		{
			m_pRule->OnObtainWorldItem(pObj, nItemID, nExtraValues);
		}
	}
}

void MMatchStage::RequestSpawnWorldItem(MMatchObject* pObj, const int nItemID, const float x, const float y, const float z, float fDropDelayTime)
{
	if (GetState() != STAGE_STATE_RUN) return;

	if (nItemID < 100) return;

	if (201 == nItemID)
	{
		mlog("Potal hacking detected. AID(%u)\n", pObj->GetAccountInfo()->m_nAID);

		return;
	}

	m_WorldItemManager.SpawnDynamicItem(pObj, nItemID, x, y, z, fDropDelayTime);
}

void MMatchStage::SpawnServerSideWorldItem(MMatchObject* pObj, const int nItemID,
	const float x, const float y, const float z,
	int nLifeTime, int* pnExtraValues)
{
	if (GetState() != STAGE_STATE_RUN) return;

	m_WorldItemManager.SpawnDynamicItem(pObj, nItemID, x, y, z, nLifeTime, pnExtraValues);
}

void MMatchStage::OnNotifyThrowTrapItem(const MUID& uidPlayer, const int nItemID)
{
	if (GetState() != STAGE_STATE_RUN) return;

	m_ActiveTrapManager.AddThrowedTrap(uidPlayer, nItemID);
}

void MMatchStage::OnNotifyActivatedTrapItem(const MUID& uidPlayer, const int nItemID, const MVector3& pos)
{
	if (GetState() != STAGE_STATE_RUN) return;

	m_ActiveTrapManager.OnActivated(uidPlayer, nItemID, pos);
}

bool MMatchStage::IsApplyTeamBonus()
{
	if ((m_StageSetting.IsTeamPlay()) && (m_TeamBonus.bApplyTeamBonus == true))
	{
		return true;
	}
	return false;
}

void MMatchStage::OnInitRound()
{
	m_TeamBonus.bApplyTeamBonus = false;

	for (int i = 0; i < MMT_END; i++)
	{
		m_Teams[i].nTeamBonusExp = 0;
		m_Teams[i].nTeamTotalLevel = 0;
		m_Teams[i].nTotalKills = 0;
	}

	int nRedTeamCount = 0, nBlueTeamCount = 0;

	for (MUIDRefCache::iterator i = GetObjBegin(); i != GetObjEnd(); i++) {
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if (pObj->GetEnterBattle() == true)
		{
			pObj->OnInitRound();

			if (m_StageSetting.IsTeamPlay())
			{
				if (pObj->GetTeam() == MMT_RED)
				{
					nRedTeamCount++;
					if (pObj->GetCharInfo())
						m_Teams[MMT_RED].nTeamTotalLevel += pObj->GetCharInfo()->m_nLevel;
				}
				else if (pObj->GetTeam() == MMT_BLUE)
				{
					nBlueTeamCount++;
					if (pObj->GetCharInfo())
						m_Teams[MMT_BLUE].nTeamTotalLevel += pObj->GetCharInfo()->m_nLevel;
				}
			}
		}
	}

	if (m_StageSetting.IsTeamPlay())
	{
		if ((nRedTeamCount > NUM_APPLYED_TEAMBONUS_TEAM_PLAYERS) &&
			(nBlueTeamCount > NUM_APPLYED_TEAMBONUS_TEAM_PLAYERS))
		{
			m_TeamBonus.bApplyTeamBonus = true;
		}
	}
}

void MMatchStage::AddTeamBonus(int nExp, MMatchTeam nTeam)
{
	if (MMT_END > nTeam)
		m_Teams[nTeam].nTeamBonusExp += nExp;
}

void MMatchStage::ResetTeamBonus()
{
	m_Teams[MMT_BLUE].nTeamBonusExp = 0;
	m_Teams[MMT_RED].nTeamBonusExp = 0;
}

void MMatchStage::OnApplyTeamBonus(MMatchTeam nTeam)
{
	if (MMT_END <= nTeam)
		return;

	if (GetStageType() != MMATCH_GAMETYPE_DEATHMATCH_TEAM2)
	{
		for (MUIDRefCache::iterator i = GetObjBegin(); i != GetObjEnd(); i++)
		{
			MMatchObject* pObj = (MMatchObject*)(*i).second;
			if (pObj->GetEnterBattle() == true)
			{
				if ((pObj->GetTeam() == nTeam) && (pObj->GetGameInfo()->bJoinedGame == true))
				{
					int nAddedExp = 0, nChrLevel = 0;
					if (pObj->GetCharInfo()) nChrLevel = pObj->GetCharInfo()->m_nLevel;
					if (m_Teams[nTeam].nTeamTotalLevel != 0)
					{
						nAddedExp = (int)(m_Teams[nTeam].nTeamBonusExp * ((float)nChrLevel / (float)m_Teams[nTeam].nTeamTotalLevel));
					}
					MMatchServer::GetInstance()->ApplyObjectTeamBonus(pObj, nAddedExp);
				}
			}
		}
	}
	else
	{
		int TotalKills = 0;
		for (MUIDRefCache::iterator i = GetObjBegin(); i != GetObjEnd(); i++)
		{
			MMatchObject* pObj = (MMatchObject*)(*i).second;
			if (pObj->GetEnterBattle() == true)
			{
				if ((pObj->GetTeam() == nTeam) && (pObj->GetGameInfo()->bJoinedGame == true))
				{
					TotalKills += pObj->GetKillCount() + 1;
				}
			}
		}

		if (TotalKills == 0)
			TotalKills = 10000000;

		for (MUIDRefCache::iterator i = GetObjBegin(); i != GetObjEnd(); i++)
		{
			MMatchObject* pObj = (MMatchObject*)(*i).second;
			if (pObj->GetEnterBattle() == true)
			{
				if ((pObj->GetTeam() == nTeam) && (pObj->GetGameInfo()->bJoinedGame == true))
				{
					int nAddedExp = 0;
					nAddedExp = (int)(m_Teams[nTeam].nTeamBonusExp * ((float)(pObj->GetKillCount() + 1) / (float)TotalKills));
					int nMaxExp = (pObj->GetCharInfo()->m_nLevel * 30 - 10) * 2 * pObj->GetKillCount();
					if (nAddedExp > nMaxExp) nAddedExp = nMaxExp;
					MMatchServer::GetInstance()->ApplyObjectTeamBonus(pObj, nAddedExp);
				}
			}
		}
	}
}

void MMatchStage::OnRoundEnd_FromTeamGame(MMatchTeam nWinnerTeam)
{
	if (MMT_END <= nWinnerTeam)
		return;

	if (IsApplyTeamBonus())
	{
		OnApplyTeamBonus(nWinnerTeam);
	}
	m_Teams[nWinnerTeam].nScore++;

	m_Teams[nWinnerTeam].nSeriesOfVictories++;
	m_Teams[NegativeTeam(nWinnerTeam)].nSeriesOfVictories = 0;

	if (CheckAutoTeamBalancing())
	{
		ShuffleTeamMembers();
	}
}

void MMatchStage::SetLadderTeam(MMatchLadderTeamInfo* pRedLadderTeamInfo, MMatchLadderTeamInfo* pBlueLadderTeamInfo)
{
	memcpy(&m_Teams[MMT_RED].LadderInfo, pRedLadderTeamInfo, sizeof(MMatchLadderTeamInfo));
	memcpy(&m_Teams[MMT_BLUE].LadderInfo, pBlueLadderTeamInfo, sizeof(MMatchLadderTeamInfo));
}

void MMatchStage::OnCommand(MCommand* pCommand)
{
	if (m_pRule) m_pRule->OnCommand(pCommand);
}

int MMatchStage::GetMinPlayerLevel()
{
	int nMinLevel = MAX_CHAR_LEVEL;

	for (MUIDRefCache::iterator i = GetObjBegin(); i != GetObjEnd(); i++)
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if (!IsEnabledObject(pObj)) continue;

		if (nMinLevel > pObj->GetCharInfo()->m_nLevel) nMinLevel = pObj->GetCharInfo()->m_nLevel;
	}

	return nMinLevel;
}

bool MMatchStage::CheckUserWasVoted(const MUID& uidPlayer)
{
	MMatchObject* pPlayer = MMatchServer::GetInstance()->GetObject(uidPlayer);
	if (!IsEnabledObject(pPlayer))
		return false;

	MVoteMgr* pVoteMgr = GetVoteMgr();
	if (0 == pVoteMgr)
		return false;

	if (!pVoteMgr->IsGoingOnVote())
		return false;

	MVoteDiscuss* pVoteDiscuss = pVoteMgr->GetDiscuss();
	if (0 == pVoteDiscuss)
		return false;

	string strVoteTarget = pVoteDiscuss->GetImplTarget();
	if ((0 != (strVoteTarget.size() - strlen(pPlayer->GetName()))))
		return false;

	if (0 != strncmp(strVoteTarget.c_str(), pPlayer->GetName(), strVoteTarget.size()))
		return false;

	return true;
}

MMatchItemBonusType GetStageBonusType(MMatchStageSetting* pStageSetting)
{
	if (pStageSetting->IsQuestDrived()) return MIBT_QUEST;
	else if (pStageSetting->IsTeamPlay()) return MIBT_TEAM;

	return MIBT_SOLO;
}

void MMatchStage::OnGameKill(const MUID& uidAttacker, const MUID& uidVictim)
{
	if (m_pRule)
	{
		m_pRule->OnGameKill(uidAttacker, uidVictim);
	}
}

bool moreTeamMemberKills(MMatchObject* pObject1, MMatchObject* pObject2)
{
	return (pObject1->GetAllRoundKillCount() > pObject2->GetAllRoundKillCount());
}

void MMatchStage::ShuffleTeamMembers()
{
	if ((m_nStageType == MST_LADDER) || (m_StageSetting.IsTeamPlay() == false)) return;
	if (m_ObjUIDCaches.empty()) return;

	int nTeamMemberCount[MMT_END] = { 0, };
	MMatchTeam nWinnerTeam;

	GetTeamMemberCount(&nTeamMemberCount[MMT_RED], &nTeamMemberCount[MMT_BLUE], NULL, true);
	if (nTeamMemberCount[MMT_RED] >= nTeamMemberCount[MMT_BLUE]) nWinnerTeam = MMT_RED;
	else nWinnerTeam = MMT_BLUE;

	int nShuffledMemberCount = abs(nTeamMemberCount[MMT_RED] - nTeamMemberCount[MMT_BLUE]) / 2;
	if (nShuffledMemberCount <= 0) return;

	vector<MMatchObject*> sortedObjectList;

	for (MUIDRefCache::iterator i = m_ObjUIDCaches.begin(); i != m_ObjUIDCaches.end(); i++)
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;

		if ((pObj->GetEnterBattle() == true) && (pObj->GetGameInfo()->bJoinedGame == true))
		{
			if ((pObj->GetTeam() == nWinnerTeam) && (!IsAdminGrade(pObj)))
			{
				sortedObjectList.push_back(pObj);
			}
		}
	}

	std::sort(sortedObjectList.begin(), sortedObjectList.end(), moreTeamMemberKills);

	int nCounter = 0;
	for (vector<MMatchObject*>::iterator itor = sortedObjectList.begin(); itor != sortedObjectList.end(); ++itor)
	{
		MMatchObject* pObj = (*itor);
		PlayerTeam(pObj->GetUID(), NegativeTeam(MMatchTeam(pObj->GetTeam())));
		nCounter++;

		if (nCounter >= nShuffledMemberCount) break;
	}

	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_RESET_TEAM_MEMBERS, MUID(0, 0));
	int nMemberCount = (int)m_ObjUIDCaches.size();
	void* pTeamMemberDataArray = MMakeBlobArray(sizeof(MTD_ResetTeamMembersData), nMemberCount);

	nCounter = 0;
	for (MUIDRefCache::iterator i = m_ObjUIDCaches.begin(); i != m_ObjUIDCaches.end(); i++)
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		MTD_ResetTeamMembersData* pNode = (MTD_ResetTeamMembersData*)MGetBlobArrayElement(pTeamMemberDataArray, nCounter);
		pNode->m_uidPlayer = pObj->GetUID();
		pNode->nTeam = (char)pObj->GetTeam();

		nCounter++;
	}

	pCmd->AddParameter(new MCommandParameterBlob(pTeamMemberDataArray, MGetBlobArraySize(pTeamMemberDataArray)));
	MEraseBlobArray(pTeamMemberDataArray);
	MMatchServer::GetInstance()->RouteToBattle(GetUID(), pCmd);
}

bool MMatchStage::CheckAutoTeamBalancing()
{
	if ((m_nStageType == MST_LADDER) || (m_StageSetting.IsTeamPlay() == false)) return false;
	if (m_StageSetting.GetAutoTeamBalancing() == false) return false;

	int nMemberCount[MMT_END] = { 0, };
	GetTeamMemberCount(&nMemberCount[MMT_RED], &nMemberCount[MMT_BLUE], NULL, true);

	const int MEMBER_COUNT = 2;
	const int SERIES_OF_VICTORIES = 3;

	if (((nMemberCount[MMT_RED] - nMemberCount[MMT_BLUE]) >= MEMBER_COUNT) &&
		(m_Teams[MMT_RED].nSeriesOfVictories >= SERIES_OF_VICTORIES))
	{
		return true;
	}
	else if (((nMemberCount[MMT_BLUE] - nMemberCount[MMT_RED]) >= MEMBER_COUNT) &&
		(m_Teams[MMT_BLUE].nSeriesOfVictories >= SERIES_OF_VICTORIES))
	{
		return true;
	}

	return false;
}

void MMatchStage::GetTeamMemberCount(int* poutnRedTeamMember, int* poutnBlueTeamMember, int* poutSpecMember, bool bInBattle)
{
	if (poutnRedTeamMember) *poutnRedTeamMember = 0;
	if (poutnBlueTeamMember) *poutnBlueTeamMember = 0;
	if (poutSpecMember) *poutSpecMember = 0;

	for (MUIDRefCache::iterator itor = GetObjBegin(); itor != GetObjEnd(); itor++)
	{
		MMatchObject* pObj = (MMatchObject*)(*itor).second;

		if (((bInBattle == true) && (pObj->GetEnterBattle() == true)) || (bInBattle == false))
		{
			switch (pObj->GetTeam())
			{
			case MMT_RED:		if (poutnRedTeamMember) (*poutnRedTeamMember)++; break;
			case MMT_BLUE:		if (poutnBlueTeamMember) (*poutnBlueTeamMember)++; break;
			case MMT_SPECTATOR:	if (poutSpecMember) (*poutSpecMember)++; break;
			};
		}
	}
}

int MMatchStage::GetPlayers()
{
	int nPlayers = 0;

	for (MUIDRefCache::iterator i = GetObjBegin(); i != GetObjEnd(); i++)
	{
		MMatchObject* pObj = (MMatchObject*)((*i).second);

		if (IsAdminGrade(pObj) && pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide))
			continue;

		nPlayers++;
	}

	return nPlayers;
}

bool MMatchStage::CheckDuelMap()
{
	if (MMATCH_GAMETYPE_DUEL != m_StageSetting.GetGameType())
		return true;

	if (MGetServerConfig()->IsClanServer())
		return true;

	MChannelRule* pRule = GetStageChannelRule();
	if (NULL == pRule)
		return false;

	MChannelRuleMapList* pChannelRuleMapList = pRule->GetMapList();
	if (NULL == pChannelRuleMapList)
		return false;

	if (!pChannelRuleMapList->Exist(GetMapName(), true))
	{
		MMatchServer* pServer = MGetMatchServer();

		MMatchObject* pMaster = pServer->GetObject(GetMasterUID());
		if (NULL == pMaster)
			return false;

		MCommand* pCmd = pServer->CreateCommand(MC_GAME_START_FAIL, MUID(0, 0));
		if (0 == pCmd)
			return false;

		pCmd->AddParameter(new MCmdParamInt(INVALID_MAP));
		pCmd->AddParameter(new MCmdParamUID(MUID(0, 0)));

		pServer->RouteToListener(pMaster, pCmd);

		return false;
	}

	return true;
}

bool MMatchStage::CheckTicket(MMatchObject* pObj)
{
	if (NULL == pObj)
		return false;

	if (IsAdminGrade(pObj))
		return true;

	if (!m_StageSetting.IsCheckTicket())
		return true;

	if (!pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide) &&
		pObj->GetCharInfo()->m_ItemList.IsHave(m_StageSetting.GetTicketItemID()))
		return true;

	MCommand* pCmd = MGetMatchServer()->CreateCommand(MC_GAME_START_FAIL, MUID(0, 0));
	if (0 != pCmd)
	{
		pCmd->AddParameter(new MCmdParamInt(INVALID_TACKET_USER));
		pCmd->AddParameter(new MCmdParamUID(pObj->GetUID()));

		MGetMatchServer()->RouteToStage(GetUID(), pCmd);
	}

	return false;
}

bool MMatchStage::CheckQuestGame()
{
	if (MGetGameTypeMgr()->IsQuestDerived(GetStageSetting()->GetGameType()))
	{
		if (!QuestTestServer())
			return false;
	}
	else
	{
		return true;
	}

	MMatchRuleBaseQuest* pRuleQuest = static_cast<MMatchRuleBaseQuest*>(GetRule());
	if (0 == pRuleQuest)
		return false;

	if (pRuleQuest->PrepareStart())
	{
	}
	else
	{
		return false;
	}

	return true;
}

bool MMatchStage::SetMapName(char* pszMapName)
{
	if (!IsValidMap(pszMapName))
	{
		mlog("map haking : invlid map name setting.");

		DWORD dwCID = 0;
		MMatchObject* pObj = GetObj(GetMasterUID());
		if (NULL != pObj)
		{
			if (NULL != pObj->GetCharInfo())
			{
				dwCID = pObj->GetCharInfo()->m_nCID;
				mlog(" CID(%u)", dwCID);
			}
		}

		mlog(".\n");

		return false;
	}
	m_StageSetting.SetMapName(pszMapName);

	return true;
}

MChannelRule* MMatchStage::GetStageChannelRule()
{
	MMatchServer* pServer = MGetMatchServer();

	MMatchChannel* pChannel = pServer->FindChannel(GetOwnerChannel());
	if (NULL == pChannel)
		return NULL;

	return MGetChannelRuleMgr()->GetRule(pChannel->GetRuleType());
}

bool MMatchStage::IsValidMap(const char* pMapName)
{
	if (NULL == pMapName)
		return false;

	if (MGetServerConfig()->IsClanServer())
		return true;

	if (MGetGameTypeMgr()->IsQuestDerived(GetStageSetting()->GetGameType()))
		return true;

	MChannelRule* pRule = GetStageChannelRule();
	if (NULL == pRule)
		return false;

	bool IsDuel = false;
	bool IsCTF = false;
	if (MMATCH_GAMETYPE_DUEL == m_StageSetting.GetGameType())
		IsDuel = true;

	if (MMATCH_GAMETYPE_CTF == m_StageSetting.GetGameType())
		IsCTF = true;

	if (!pRule->CheckGameType(m_StageSetting.GetGameType()))
		return false;

	if (IsCTF)
		return pRule->CheckCTFMap(pMapName);

	return pRule->CheckMap(pMapName, IsDuel);
}

void MMatchStage::ReserveSuicide(const MUID& uidUser, const DWORD dwExpireTime)
{
	vector< MMatchStageSuicide >::iterator it, end;
	end = m_SuicideList.end();
	for (it = m_SuicideList.begin(); it != end; ++it)
	{
		if (uidUser == it->m_uidUser)
			return;
	}

	MMatchStageSuicide SuicideUser(uidUser, dwExpireTime + 10000);

	m_SuicideList.push_back(SuicideUser);

	MCommand* pNew = MGetMatchServer()->CreateCommand(MC_MATCH_RESPONSE_SUICIDE_RESERVE, uidUser);
	if (NULL == pNew)
		return;

	MGetMatchServer()->PostSafeQueue(pNew);
}

void MMatchStage::CheckSuicideReserve(const DWORD dwCurTime)
{
	vector< MMatchStageSuicide >::iterator it, end;
	end = m_SuicideList.end();
	for (it = m_SuicideList.begin(); it != end; ++it)
	{
		if ((false == it->m_bIsChecked) && (dwCurTime > it->m_dwExpireTime))
		{
			MMatchObject* pObj = GetObj(it->m_uidUser);
			if (NULL == pObj)
			{
				m_SuicideList.erase(it);
				break;
			}

			MMatchStage* pStage = MMatchServer::GetInstance()->FindStage(pObj->GetStageUID());
			if (pStage == NULL) break;
			if (pObj->CheckAlive() == false)	break;

			pObj->OnDead();
			pStage->OnGameKill(pObj->GetUID(), pObj->GetUID());

			MCommand* pNew = MGetMatchServer()->CreateCommand(MC_MATCH_RESPONSE_SUICIDE, MUID(0, 0));
			pNew->AddParameter(new MCommandParameterInt(MOK));
			pNew->AddParameter(new MCommandParameterUID(it->m_uidUser));
			MGetMatchServer()->RouteToBattle(GetUID(), pNew);

			it->m_dwExpireTime = dwCurTime + MIN_REQUEST_SUICIDE_TIME;
			it->m_bIsChecked = true;

			if (MGetGameTypeMgr()->IsQuestDerived(pStage->GetStageSetting()->GetGameType()))
			{
				MCommand* pCmd = MGetMatchServer()->CreateCommand(MC_MATCH_QUEST_PLAYER_DEAD, MUID(0, 0));
				pCmd->AddParameter(new MCommandParameterUID(it->m_uidUser));
				MGetMatchServer()->RouteToBattle(pStage->GetUID(), pCmd);
			}

			break;
		}
		else if ((true == it->m_bIsChecked) && (dwCurTime > it->m_dwExpireTime))
		{
			m_SuicideList.erase(it);
			break;
		}
	}
}

void MMatchStage::ResetPlayersCustomItem()
{
	for (MUIDRefCache::iterator itor = GetObjBegin(); itor != GetObjEnd(); itor++)
	{
		MMatchObject* pObj = (MMatchObject*)(*itor).second;

		pObj->ResetCustomItemUseCount();
	}
}

void MMatchStage::MakeResourceCRC32Cache(const DWORD dwKey, DWORD& out_crc32, DWORD& out_xor)
{
	MMatchCRC32XORCache CRC32Cache;

	CRC32Cache.Reset();
	CRC32Cache.CRC32XOR(dwKey);

#ifdef _DEBUG
	mlog("Start ResourceCRC32Cache : %u/%u\n", CRC32Cache.GetCRC32(), CRC32Cache.GetXOR());
#endif

	MakeItemResourceCRC32Cache(CRC32Cache);

#ifdef _DEBUG
	static DWORD dwOutputCount = 0;
	if (10 > (++dwOutputCount))
	{
		mlog("ResourceCRC32XOR : %u/%u\n", CRC32Cache.GetCRC32(), CRC32Cache.GetXOR());
	}
#endif

	out_crc32 = CRC32Cache.GetCRC32();
	out_xor = CRC32Cache.GetXOR();
}

void MMatchStage::SetResourceCRC32Cache(const MUID& uidPlayer, const DWORD dwCRC32Cache, const DWORD dwXORCache)
{
	ResourceCRC32CacheMap::iterator itFind = m_ResourceCRC32CacheMap.find(uidPlayer);
	if (m_ResourceCRC32CacheMap.end() == itFind)
	{
		MMATCH_RESOURCECHECKINFO CRC32CacheInfo;

		CRC32CacheInfo.dwResourceCRC32Cache = dwCRC32Cache;
		CRC32CacheInfo.dwResourceXORCache = dwXORCache;
		CRC32CacheInfo.dwLastRequestTime = MGetMatchServer()->GetGlobalClockCount();
		CRC32CacheInfo.bIsEnterBattle = true;
		CRC32CacheInfo.bIsChecked = false;

		m_ResourceCRC32CacheMap.insert(ResourceCRC32CacheMap::value_type(uidPlayer, CRC32CacheInfo));
	}
	else
	{
		itFind->second.dwResourceCRC32Cache = dwCRC32Cache;
		itFind->second.dwResourceXORCache = dwXORCache;
		itFind->second.dwLastRequestTime = MGetMatchServer()->GetGlobalClockCount();
		itFind->second.bIsEnterBattle = true;
		itFind->second.bIsChecked = false;
	}
}

void MMatchStage::RequestResourceCRC32Cache(const MUID& uidPlayer)
{
	if (!m_bIsUseResourceCRC32CacheCheck)
	{
		return;
	}

	MMatchObject* pObj = MGetMatchServer()->GetObject(uidPlayer);
	if (NULL == pObj)
	{
		return;
	}

	const DWORD dwKey = static_cast<DWORD>(RandomNumber(1, RAND_MAX));

	DWORD dwCRC32Cache, dwXORCache;
	MakeResourceCRC32Cache(dwKey, dwCRC32Cache, dwXORCache);

	SetResourceCRC32Cache(uidPlayer, dwCRC32Cache, dwXORCache);

	MCommand* pCmd = MGetMatchServer()->CreateCommand(MC_REQUEST_RESOURCE_CRC32, uidPlayer);
	pCmd->AddParameter(new MCmdParamUInt(dwKey));

	MGetMatchServer()->Post(pCmd);
}

void MMatchStage::DeleteResourceCRC32Cache(const MUID& uidPlayer)
{
	m_ResourceCRC32CacheMap.erase(uidPlayer);
}

const bool MMatchStage::IsValidResourceCRC32Cache(const MUID& uidPlayer, const DWORD dwResourceCRC32Cache, const DWORD dwResourceXORCache)
{
	ResourceCRC32CacheMap::iterator itFind = m_ResourceCRC32CacheMap.find(uidPlayer);
	if (m_ResourceCRC32CacheMap.end() == itFind)
	{
		mlog("Can't find Resource crc.\n");
		return false;
	}

	if (dwResourceCRC32Cache != itFind->second.dwResourceCRC32Cache ||
		dwResourceXORCache != itFind->second.dwResourceXORCache)
	{
		mlog("invalid resource crc : s(%u/%u), c(%u/%u).\n"
			, itFind->second.dwResourceCRC32Cache, itFind->second.dwResourceXORCache
			, dwResourceCRC32Cache, dwResourceXORCache);

		return false;
	}

	itFind->second.bIsChecked = true;

	return true;
}

void MMatchStage::CheckResourceCRC32Cache(const DWORD dwClock)
{
	if (!m_bIsUseResourceCRC32CacheCheck)
	{
		return;
	}

	static const DWORD MAX_ELAPSED_UPDATE_CRC32CACHE = 20000;
	static const DWORD CRC32CACHE_CHECK_TIME = 10000;
#ifndef _DEBUG
	static const DWORD CRC32CACHE_CEHCK_REPEAT_TERM = 1000 * 60 * 5;
#else
	static const DWORD CRC32CACHE_CEHCK_REPEAT_TERM = 1000 * 10;
#endif

	if (CRC32CACHE_CHECK_TIME > (dwClock - m_dwLastResourceCRC32CacheCheckTime))
	{
		return;
	}

	ResourceCRC32CacheMap::const_iterator	end = m_ResourceCRC32CacheMap.end();
	ResourceCRC32CacheMap::iterator			it = m_ResourceCRC32CacheMap.begin();

	for (; end != it; ++it)
	{
		if (!it->second.bIsEnterBattle)
		{
			continue;
		}

		if (it->second.bIsChecked)
		{
			if (CRC32CACHE_CEHCK_REPEAT_TERM < (dwClock - it->second.dwLastRequestTime))
			{
				RequestResourceCRC32Cache(it->first);
			}

			continue;
		}

		if (MAX_ELAPSED_UPDATE_CRC32CACHE < (dwClock - it->second.dwLastRequestTime))
		{
			MGetMatchServer()->StageLeaveBattle(it->first, true, true);
			MGetMatchServer()->StageLeave(it->first);

			MMatchObject* pObj = MGetMatchServer()->GetObject(it->first);
			if ((NULL != pObj) && (NULL != pObj->GetCharInfo()))
			{
				MGetMatchServer()->LOG(MMatchServer::LOG_PROG, "dynamic resource crc32 check : hackuser(%s).\n"
					, pObj->GetCharInfo()->m_szName);
			}
			return;
		}
	}

	m_dwLastResourceCRC32CacheCheckTime = dwClock;
}

void MMatchStage::SetDisableCheckResourceCRC32Cache(const MUID& uidPlayer)
{
	if (!m_bIsUseResourceCRC32CacheCheck)
	{
		return;
	}

	ResourceCRC32CacheMap::iterator itFind = m_ResourceCRC32CacheMap.find(uidPlayer);
	if (m_ResourceCRC32CacheMap.end() == itFind)
	{
		return;
	}

	itFind->second.bIsEnterBattle = false;
}

void MMatchStage::SetDisableAllCheckResourceCRC32Cache()
{
	ResourceCRC32CacheMap::const_iterator	end = m_ResourceCRC32CacheMap.end();
	ResourceCRC32CacheMap::iterator			it = m_ResourceCRC32CacheMap.begin();

	for (; end != it; ++it)
	{
		it->second.bIsEnterBattle = false;
	}
}

void MMatchStage::MakeItemResourceCRC32Cache(MMatchCRC32XORCache& CRC32Cache)
{
	ClearGabageObject();

	MMatchObject* pObj = NULL;
	MUIDRefCache::const_iterator	end = m_ObjUIDCaches.end();
	MUIDRefCache::iterator			it = m_ObjUIDCaches.begin();
	MMatchItem* pItem = NULL;

	MMatchServer* pServer = MGetMatchServer();

#ifdef _DEBUG
	static DWORD dwOutputCount = 0;
	++dwOutputCount;
#endif

	for (; end != it; ++it)
	{
		pObj = reinterpret_cast<MMatchObject*>(it->second);

		for (int i = 0; i < MMCIP_END; ++i)
		{
			pItem = pObj->GetCharInfo()->m_EquipedItem.GetItem(MMatchCharItemParts(i));
			if (NULL == pItem)
			{
				continue;
			}

			if (NULL == pItem->GetDesc())
			{
				continue;
			}

			pItem->GetDesc()->CacheCRC32(CRC32Cache);

#ifdef _DEBUG
			if (10 > dwOutputCount)
			{
				MMatchItemDesc* pItemDesc = pItem->GetDesc();
				mlog("ItemID : %d, CRCCache : %u\n"
					, pItemDesc->m_nID
					, CRC32Cache.GetXOR());
			}
#endif
		}
	}
}

void MMatchStage::ClearGabageObject()
{
	for (MUIDRefCache::iterator i = GetObjBegin(); i != GetObjEnd(); i++) {
		MUID uidObj = (MUID)(*i).first;
		MMatchObject* pObj = MGetMatchServer()->GetObject(uidObj);
		if (!pObj)
		{
			MGetMatchServer()->LogObjectCommandHistory(uidObj);
			mlog("WARNING(RouteToBattle) : stage Not Existing Obj(%u:%u)\n", uidObj.High, uidObj.Low);
			i = RemoveObject(uidObj);
		}
	}
}

int	MMatchStage::GetDuelTournamentRandomMapIndex()
{
	MChannelRule* pChannelRule = MGetChannelRuleMgr()->GetRule(MCHANNEL_RULE_DUELTOURNAMENT);
	if (pChannelRule == NULL) return -1;

	MChannelRuleMapList* pMapList = pChannelRule->GetMapList();
	if (pMapList == NULL) return -1;

	int nMaxIndex = (int)pMapList->size();

	if (nMaxIndex != 0) {
		int nRandomMapIndex;
		int nRandomVal = rand() % nMaxIndex;

		MChannelRuleMapList::iterator iter;
		for (iter = pMapList->begin(); iter != pMapList->end(); iter++)
		{
			if (nRandomVal == 0)
				nRandomMapIndex = *iter;

			nRandomVal = nRandomVal - 1;
		}

		return nRandomMapIndex;
	}

	return -1;
}

void MMatchStage::SetDuelTournamentMatchList(MDUELTOURNAMENTTYPE nType, MDuelTournamentPickedGroup* pPickedGroup)
{
	m_nDTStageInfo.nDuelTournamentType = nType;

	m_nDTStageInfo.nDuelTournamentTotalRound = 0;
	m_nDTStageInfo.DuelTournamentMatchMap.clear();

	MDUELTOURNAMENTROUNDSTATE nRoundState = GetDuelTournamentRoundState(nType);
	MakeDuelTournamentMatchMap(nRoundState, 1);

	int nIndex = 0;

	MMatchDuelTournamentMatch* pMatch;

	map<int, MMatchDuelTournamentMatch*>::iterator iter = m_nDTStageInfo.DuelTournamentMatchMap.begin();
	for (MDuelTournamentPickedGroup::iterator i = pPickedGroup->begin(); i != pPickedGroup->end(); i++)
	{
		if (nIndex % 2 == 0) {
			pMatch = iter->second;
			iter++;

			pMatch->uidPlayer1 = (*i);
			pMatch->uidPlayer2 = MUID(0, 0);
		}
		else {
			pMatch->uidPlayer2 = (*i);
		}

		nIndex++;
	}

#ifdef _DUELTOURNAMENT_LOG_ENABLE_
	for (map<int, MMatchDuelTournamentMatch*>::iterator iter = m_nDTStageInfo.DuelTournamentMatchMap.begin();
		iter != m_nDTStageInfo.DuelTournamentMatchMap.end(); iter++)
	{
		MMatchDuelTournamentMatch* pMatch = iter->second;

		MGetMatchServer()->LOG(MMatchServer::LOG_PROG, "RoundState=%d, Order=%d, NextOrder=%d, P1=(%d%d), P2=(%d%d)",
			pMatch->nRoundState, pMatch->nMatchNumber, pMatch->nNextMatchNumber, pMatch->uidPlayer1.High, pMatch->uidPlayer1.Low
			, pMatch->uidPlayer2.High, pMatch->uidPlayer2.Low);
	}
#endif
}

void MMatchStage::MakeDuelTournamentMatchMap(MDUELTOURNAMENTROUNDSTATE nRoundState, int nMatchNumber)
{
	if (nRoundState == MDUELTOURNAMENTROUNDSTATE_MAX) return;

	int nRemainCount;

	switch (nRoundState) {
	case MDUELTOURNAMENTROUNDSTATE_QUATERFINAL:		nRemainCount = 4; break;
	case MDUELTOURNAMENTROUNDSTATE_SEMIFINAL:			nRemainCount = 2; break;
	case MDUELTOURNAMENTROUNDSTATE_FINAL:				nRemainCount = 1; break;
	}

	int nTemp = 0;
	for (int i = 0; i < nRemainCount; i++) {
		MMatchDuelTournamentMatch* pMatch = new MMatchDuelTournamentMatch;
		memset(pMatch, 0, sizeof(MMatchDuelTournamentMatch));

		pMatch->nRoundState = nRoundState;
		pMatch->nNextMatchNumber = nMatchNumber + nRemainCount + nTemp - i;
		pMatch->nMatchNumber = nMatchNumber;

		pMatch->uidPlayer1 = MUID(0, 0);
		pMatch->uidPlayer2 = MUID(0, 0);

		if (nMatchNumber % 2 == 0) nTemp++;
		if (nRoundState == MDUELTOURNAMENTROUNDSTATE_FINAL) pMatch->nNextMatchNumber = 0;

		m_nDTStageInfo.DuelTournamentMatchMap.insert(pair<int, MMatchDuelTournamentMatch*>(nMatchNumber++, pMatch));

		if (pMatch->nRoundState == MDUELTOURNAMENTROUNDSTATE_FINAL)			m_nDTStageInfo.nDuelTournamentTotalRound += 3;
		else if (pMatch->nRoundState == MDUELTOURNAMENTROUNDSTATE_SEMIFINAL)	m_nDTStageInfo.nDuelTournamentTotalRound += 3;
		else																	m_nDTStageInfo.nDuelTournamentTotalRound += 1;
	}

	MakeDuelTournamentMatchMap(GetDuelTournamentNextRoundState(nRoundState), nMatchNumber);
}

void MMatchStage::ClearDuelTournamentMatchMap()
{
	for (map<int, MMatchDuelTournamentMatch*>::iterator iter = m_nDTStageInfo.DuelTournamentMatchMap.begin();
		iter != m_nDTStageInfo.DuelTournamentMatchMap.end(); iter++)
	{
		MMatchDuelTournamentMatch* pMatch = iter->second;
		delete pMatch;
	}

	m_nDTStageInfo.DuelTournamentMatchMap.clear();
}

int MMatchStage::GetDuelTournamentNextOrder(MDUELTOURNAMENTROUNDSTATE nRoundState, int nOrder, int nTemp)
{
	int nResult;
	int nAdditionalOrder = 0;

	switch (nRoundState) {
	case MDUELTOURNAMENTROUNDSTATE_QUATERFINAL:		nAdditionalOrder = 4; break;
	case MDUELTOURNAMENTROUNDSTATE_SEMIFINAL:			nAdditionalOrder = 2; break;
	case MDUELTOURNAMENTROUNDSTATE_FINAL:				nAdditionalOrder = 0; break;
	default: ASSERT(0);
	}

	if (nOrder % 2 == 0)	nResult = nOrder + nAdditionalOrder - nTemp - 1;
	else					nResult = nOrder + nAdditionalOrder - nTemp;

	return nResult;
}

void MMatchStage::InitCurrRelayMap()
{
	SetRelayMapCurrList(m_StageSetting.GetRelayMapList());
	m_RelayMapRepeatCountRemained = m_StageSetting.GetRelayMapRepeatCount();
	m_RelayMapType = m_StageSetting.GetRelayMapType();
	m_bIsLastRelayMap = false;
}

void MMatchStage::SetRelayMapCurrList(const RelayMap* pRelayMapList)
{
	m_vecRelayMapsRemained.clear();
	for (int i = 0; GetRelayMapListCount() > i; ++i)
	{
		m_vecRelayMapsRemained.push_back(pRelayMapList[i]);
	}
}
void MMatchStage::SetRelayMapListCount(int nRelayMapListCount)
{
	if (nRelayMapListCount > MAX_RELAYMAP_LIST_COUNT)
		nRelayMapListCount = 20;
	m_StageSetting.SetRelayMapListCount(nRelayMapListCount);
}

void MMatchStage::SetRelayMapList(RelayMap* pRelayMapList)
{
	int count = 0;
	for (int i = 0; i < MAX_RELAYMAP_LIST_COUNT; ++i)
	{
		if (!MGetMapDescMgr()->MIsCorrectMap(pRelayMapList[i].nMapID))
			break;
		++count;
	}

	if (count == 0)
	{
		pRelayMapList[0].nMapID = MMATCH_MAP_MANSION;
		count = 1;
	}

	SetRelayMapListCount(count);
	m_StageSetting.SetRelayMapList(pRelayMapList);
}