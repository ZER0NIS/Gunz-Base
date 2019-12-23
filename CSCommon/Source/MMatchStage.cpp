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

//////////////////////////////////////////////////////////////////////////////////
MMatchStage::MMatchStage()
{
	m_pRule								= NULL;
	m_nIndex							= 0;
	m_nStageType						= MST_NORMAL;
	m_uidOwnerChannel					= MUID(0,0);
	m_TeamBonus.bApplyTeamBonus			= false;
	m_nAdminObjectCount					= 0;

	m_nStateTimer						= 0;
	m_nLastTick							= 0;
	m_nChecksum							= 0;
	m_nLastChecksumTick					= 0;
	m_nAdminObjectCount					= 0;
	m_nStartTime						= 0;
	m_nLastRequestStartStageTime		= 0;
	m_dwLastResourceCRC32CacheCheckTime = 0;
	m_bIsUseResourceCRC32CacheCheck		= false;

	m_vecRelayMapsRemained.clear();
	m_RelayMapType						= RELAY_MAP_TURN;
	m_RelayMapRepeatCountRemained		= RELAY_MAP_3REPEAT;
	m_bIsLastRelayMap					= false;
	memset(m_Teams, 0, sizeof(m_Teams));
}
MMatchStage::~MMatchStage()
{

}


const bool MMatchStage::SetChannelRuleForCreateStage(bool bIsAllowNullChannel)
{
	MMatchChannel* pChannel = MGetMatchServer()->GetChannelMap()->Find( m_uidOwnerChannel );
	if( NULL == pChannel )
	{
		// 클랜전은 채널이 없다...
		if( MSM_CLAN != MGetServerConfig()->GetServerMode() 
			&& bIsAllowNullChannel == false ) {
			return false;
		}		

		ChangeRule(MMATCH_GAMETYPE_DEFAULT);
		return true;
	}

	MChannelRule* pChannelRule = MGetChannelRuleMgr()->GetRule( pChannel->GetRuleType() );
	if( NULL == pChannelRule )
	{
		return false;
	}

	if( pChannelRule->CheckGameType(MMATCH_GAMETYPE_DEFAULT) )
	{
		ChangeRule(MMATCH_GAMETYPE_DEFAULT);
	}
	else
	{
		// Default game type이 없다면 가능한 타입중 가장 앞에 있는걸 사용한다.
		const int nGameType = pChannelRule->GetGameTypeList()->GetFirstGameType();
		if( -1 == nGameType )
		{
			return false;
		}

		ChangeRule( MMATCH_GAMETYPE(nGameType) );
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

	if( !SetChannelRuleForCreateStage(bIsAllowNullChannel) )
	{
		return false;
	}
	
	SetAgentUID(MUID(0,0));
	SetAgentReady(false);

	m_nChecksum = 0;
	m_nLastChecksumTick = 0;
	m_nAdminObjectCount = 0;
	
	m_WorldItemManager.Create(this);
	m_ActiveTrapManager.Create(this);

	SetFirstMasterName("");

	m_StageSetting.SetIsCheckTicket( bIsCheckTicket );
	m_StageSetting.SetTicketItemID( dwTicketItemID );

	return true;
}

void MMatchStage::Destroy()
{
	MUIDRefCache::iterator itor=GetObjBegin();
	while(itor!=GetObjEnd()) {
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
	if (i!=m_BanCIDList.end())
		return true;
	else
		return false;
}

void MMatchStage::AddObject(const MUID& uid, const MMatchObject* pObj)
{
	m_ObjUIDCaches.Insert(uid, (void*)pObj);


	// 어드민 유저 따로 관리한다.
	MMatchObject* pObject = (MMatchObject*)pObj;
	if (IsEnabledObject(pObject))
	{
		if (IsAdminGrade(pObject->GetAccountInfo()->m_nUGrade))
		{
			m_nAdminObjectCount++;
		}

		if( GetStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUELTOURNAMENT ){
			pObject->SetJoinDuelTournament(true);
		}
	}

	// 방장 등록
	if (GetObjCount() == 1)
	{
		SetMasterUID(uid);
	}

	m_VoteMgr.AddVoter(uid);

	// 관리하기 편하게 하기 우해서 스테이지에 등록이 될때 최대 사용 수량이 있는 아이템은
	//  초기화를 해준다.
	// 이고 말고도 난입과 라운드가 시작할때도 해줘야 한다.
	pObject->ResetCustomItemUseCount();
}

MUIDRefCache::iterator MMatchStage::RemoveObject(const MUID& uid)
{
	m_VoteMgr.RemoveVoter(uid);
	if( CheckUserWasVoted(uid) )
	{
		m_VoteMgr.StopVote( uid );
	}

	// uid가 존제하는지 먼저 검사하고 다음에 유저 상태를 변경한다.
	MUIDRefCache::iterator i = m_ObjUIDCaches.find(uid);
	if (i==m_ObjUIDCaches.end()) 
	{
		//MMatchServer::GetInstance()->LOG(MCommandCommunicator::LOG_FILE, "RemoveObject: Cannot Find Object uid");
		//mlog("RemoveObject: Cannot Find Object uid\n");
		////_ASSERT(0);
		return i;
	}

	MMatchObject* pObj = MMatchServer::GetInstance()->GetObject(uid);	// NULL이라도 m_ObjUIDCaches에선 빼줘야함
	if (pObj) {
		// 어드민 유저 관리
		if (IsAdminGrade(pObj->GetAccountInfo()->m_nUGrade))
		{
			m_nAdminObjectCount--;
			if (m_nAdminObjectCount < 0) m_nAdminObjectCount = 0;
		}

		LeaveBattle(pObj);

		pObj->SetStageUID(MUID(0,0));
		pObj->SetForcedEntry(false);
		pObj->SetPlace(MMP_LOBBY);
		pObj->SetStageListTransfer(true);

		if( GetStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUELTOURNAMENT ){
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
			if ((GetState() == STAGE_STATE_RUN) && (GetObjInBattleCount()>0))
				SetMasterUID(RecommandMaster(true));
			else
				SetMasterUID(RecommandMaster(false));
		}
	}

	// 스테이지에서 나가면 리소스 검사는 더이상 할 필요가 없다.
	DeleteResourceCRC32Cache( uid );

	return itorNext;
}

bool MMatchStage::KickBanPlayer(const char* pszName, bool bBanPlayer)
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	for (MUIDRefCache::iterator i=m_ObjUIDCaches.begin(); i!=m_ObjUIDCaches.end(); i++) 
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if (pObj->GetCharInfo() == NULL) 
			continue;
		if (_stricmp(pObj->GetCharInfo()->m_szName, pszName) == 0) {
			if (bBanPlayer)
				AddBanList(pObj->GetCharInfo()->m_nCID);	// Ban

			pServer->StageLeaveBattle(pObj->GetUID(), true, true);//, GetUID());
			pServer->StageLeave(pObj->GetUID());//, GetUID());
			return true;
		}
	}
	return false;
}

const MUID MMatchStage::RecommandMaster(bool bInBattleOnly)
{
	for (MUIDRefCache::iterator i=m_ObjUIDCaches.begin(); i!=m_ObjUIDCaches.end(); i++) 
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if (bInBattleOnly && (pObj->GetEnterBattle() == false))
			continue;
		return (*i).first;
	}
	return MUID(0,0);
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

			// 월드아이템 정보를 보내준다
			m_WorldItemManager.RouteAllItems(pObj);
			m_ActiveTrapManager.RouteAllTraps(pObj);

			// 난입자에게 방상태를 전송한다.
			MMatchServer::GetInstance()->ResponseRoundState(pObj, GetUID());
		}

		if (m_pRule)
		{
			MUID uidChar = pObj->GetUID();
			m_pRule->OnEnterBattle(uidChar);
		}
	}

	// 방에 들어갔으면 난입했는지 여부는 다시 false로 초기화
	pObj->SetForcedEntry(false);
	pObj->ResetCustomItemUseCount();

	RequestResourceCRC32Cache( pObj->GetUID() );
}

void MMatchStage::LeaveBattle(MMatchObject* pObj)
{	
	if ((GetState() == STAGE_STATE_RUN) && (m_pRule))
	{
		MUID uidPlayer = pObj->GetUID();
		m_pRule->OnLeaveBattle(uidPlayer);
	}

	pObj->OnLeaveBattle();

	SetDisableCheckResourceCRC32Cache( pObj->GetUID() );
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

				CheckSuicideReserve( nClock );

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
	// STAGE_STATE_CLOSE 는 MMatchServer::StageRemove 로 처리
	}

	m_VoteMgr.Tick(nClock);

	if (IsChecksumUpdateTime(nClock))
		UpdateChecksum(nClock);

	m_nLastTick = nClock;

	if ((m_ObjUIDCaches.empty()) && (GetState() != STAGE_STATE_CLOSE))
	{
		ChangeState(STAGE_STATE_CLOSE);
	}

	
	CheckResourceCRC32Cache( nClock );

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
			//_ASSERT(0);
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

/*
int MMatchStage::GetTeamMemberCount(MMatchTeam nTeam)
{
	int nSpec = 0;
	int nRed = 0;
	int nBlue = 0;

	for (MUIDRefCache::iterator i=m_ObjUIDCaches.begin(); i!=m_ObjUIDCaches.end(); i++) 
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if (pObj->GetTeam() == MMT_SPECTATOR)
			nSpec++;
		else if (pObj->GetTeam() == MMT_RED)
			nRed++;
		else if (pObj->GetTeam() == MMT_BLUE)
			nBlue++;
	}

	if (nTeam == MMT_SPECTATOR)
		return nSpec;
	else if (nTeam == MMT_RED)
		return nRed;
	else if (nTeam == MMT_BLUE)
		return nBlue;
	return 0;
}
*/

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
	if (i==m_ObjUIDCaches.end())
		return;

	MMatchObject* pObj = (MMatchObject*)(*i).second;
	pObj->SetTeam(nTeam);

	MMatchStageSetting* pSetting = GetStageSetting();
	pSetting->UpdateCharSetting(uidPlayer, nTeam, pObj->GetStageState());
}

void MMatchStage::PlayerState(const MUID& uidPlayer, MMatchObjectStageState nStageState)
{
	MUIDRefCache::iterator i = m_ObjUIDCaches.find(uidPlayer);
	if (i==m_ObjUIDCaches.end())
		return;

	MMatchObject* pObj = (MMatchObject*)(*i).second;

	pObj->SetStageState(nStageState);

	MMatchStageSetting* pSetting = GetStageSetting();
	pSetting->UpdateCharSetting(uidPlayer, pObj->GetTeam(), pObj->GetStageState());
}

// 필요하면 클라이언트 쪽이랑 통합.. 서버쪽은 컬러는 필요없다..
// color 구조체 때문에 민트랑 묶이는 것이...

bool _GetUserGradeIDName(MMatchUserGradeID gid,char* sp_name)
{
	if(gid == MMUG_DEVELOPER) 
	{ 
		if(sp_name) {
			strcpy(sp_name,"개발자");
		}
		return true; 
	}
	else if(gid == MMUG_ADMIN) {
		
		if(sp_name) { 
			strcpy(sp_name,"운영자");
		}
		return true; 
	}

	return false;
}

bool MMatchStage::StartGame( const bool bIsUseResourceCRC32CacheCheck )
{
	// 스테이지가 준비 단계일때만 게임을 시작할 수 있다.
	if( STAGE_STATE_STANDBY != GetState() ) return false;
	
	// 인원수 체크
	int nPlayer = GetCountableObjCount();
	if (nPlayer > m_StageSetting.GetMaxPlayers())
	{
		char szMsg[ 256];
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_PERSONNEL_TOO_MUCH);

		MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_ANNOUNCE, MUID(0,0));
		pCmd->AddParameter(new MCmdParamUInt(0));
		pCmd->AddParameter(new MCmdParamStr(szMsg));
		MMatchServer::GetInstance()->RouteToStage(GetUID(), pCmd);

		return false;
	}

	bool bResult = true;
	bool bNotReadyExist = false;
	MUID uidItem;


	// 현제 선택된 맵이 정상 맵인지 검사를 한다. -> 이 부분은 맵 이름을 설정할때 유효성 검사를 한다.
	// setmap에서 맵 이름을 검사하는데 문제가 있어서 다시 적용.
	// if( !CheckDuelMap() )
	// 	return false;

	// 대기 인원 체크
	for (MUIDRefCache::iterator i=m_ObjUIDCaches.begin(); i!=m_ObjUIDCaches.end(); i++)
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if( NULL == pObj) continue;
		if( !CheckTicket(pObj) ) return false;

		if ((GetMasterUID() != (*i).first) && (pObj->GetStageState() != MOSS_READY)) 
		{
			if (IsAdminGrade(pObj) && pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide))
				continue;	// 투명 운영자는 Ready 안해도됨

			bNotReadyExist = true;
			bResult = false;

			const char* szName = NULL;
			char sp_name[256];

			if(_GetUserGradeIDName(pObj->GetAccountInfo()->m_nUGrade,sp_name))  szName = sp_name;
			else																szName = pObj->GetName();

			char szSend[256];
			sprintf(szSend, "%s%d\a%s", MTOK_ANNOUNCE_PARAMSTR, MERR_HE_IS_NOT_READY, szName);	// 에러메시지ID와 인자를 \a로 구별한다

			MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_ANNOUNCE, MUID(0,0));
			pCmd->AddParameter(new MCmdParamUInt(0));
			//pCmd->AddParameter(new MCmdParamStr(szMsg));
			pCmd->AddParameter(new MCmdParamStr(szSend));
			MMatchServer::GetInstance()->RouteToStage(GetUID(), pCmd);
		}
	}

	// 모든 유저가 Ready를 하지 않아서 게임을 시작할수 없다는것을 통보해 줌. 
	// 새로 추가됨. - by 추교성. 2005.04.19
	if (bNotReadyExist)
	{
		MCommand* pCmdNotReady = MMatchServer::GetInstance()->CreateCommand( MC_GAME_START_FAIL, MUID(0, 0) );
		if( 0 == pCmdNotReady ) {
			mlog( "MMatchStage::StartGame - 커맨드 생성 실패.\n" );
			bResult = false;
		}

		pCmdNotReady->AddParameter( new MCmdParamInt(ALL_PLAYER_NOT_READY) );
		pCmdNotReady->AddParameter( new MCmdParamUID(MUID(0, 0)) );

		MMatchObject* pMaster = MMatchServer::GetInstance()->GetObject( GetMasterUID() );
		if( IsEnabledObject(pMaster) ) {
			MMatchServer::GetInstance()->RouteToListener( pMaster, pCmdNotReady );
		} else {
			delete pCmdNotReady;
			bResult = false;
		}
	}

	if( !CheckQuestGame() ) return false;

	// 마지막 시작후 MIN_REQUEST_STAGESTART_TIME만큼 지나야 다시 시작 요청을 할 수 있다.
	if( MIN_REQUEST_STAGESTART_TIME > (MMatchServer::GetInstance()->GetTickTime() - m_nLastRequestStartStageTime) ) return false;
	m_nLastRequestStartStageTime = MMatchServer::GetInstance()->GetTickTime();

	MMatchObject* pMasterObj = MMatchServer::GetInstance()->GetObject(GetMasterUID());
	if (pMasterObj && IsAdminGrade(pMasterObj) && pMasterObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide))
		bResult = true;
	
	if (bResult == true) {
		for (MUIDRefCache::iterator i=m_ObjUIDCaches.begin(); i!=m_ObjUIDCaches.end(); i++) {
			MMatchObject* pObj = (MMatchObject*)(*i).second;
			pObj->SetLaunchedGame(true);
		}

		ChangeState(STAGE_STATE_COUNTDOWN);
	}

	m_bIsUseResourceCRC32CacheCheck = bIsUseResourceCRC32CacheCheck;

	return bResult;
}

bool MMatchStage::StartRelayGame( const bool bIsUseResourceCRC32CacheCheck )
{
	if( STAGE_STATE_STANDBY != GetState() ) return false;	//< 스테이지가 준비 단계일때만 게임을 시작할 수 있다.

	// 인원수 체크
	int nPlayer = GetCountableObjCount();
	if (nPlayer > m_StageSetting.GetMaxPlayers()) {
		char szMsg[ 256];
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_PERSONNEL_TOO_MUCH);

		MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_ANNOUNCE, MUID(0,0));
		pCmd->AddParameter(new MCmdParamUInt(0));
		pCmd->AddParameter(new MCmdParamStr(szMsg));
		MMatchServer::GetInstance()->RouteToStage(GetUID(), pCmd);

		return false;
	}

	bool bResult = true;
	bool bNotReadyExist = false;
	MUID uidItem;

	// 대기 인원 체크
	for (MUIDRefCache::iterator i=m_ObjUIDCaches.begin(); i!=m_ObjUIDCaches.end(); i++)
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if( NULL == pObj) continue;
		if( !CheckTicket(pObj) ) return false;
	}

/*
	if( !CheckQuestGame() ) return false;

	// 마지막 시작후 MIN_REQUEST_STAGESTART_TIME만큼 지나야 다시 시작 요청을 할 수 있다.
	if( MIN_REQUEST_STAGESTART_TIME > (MMatchServer::GetInstance()->GetTickTime() - m_nLastRequestStartStageTime) ) return false;
	m_nLastRequestStartStageTime = MMatchServer::GetInstance()->GetTickTime();
*/

	MMatchObject* pMasterObj = MMatchServer::GetInstance()->GetObject(GetMasterUID());
	if (pMasterObj && IsAdminGrade(pMasterObj) && pMasterObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide))
		bResult = true;

	if (bResult == true) {
		for (MUIDRefCache::iterator i=m_ObjUIDCaches.begin(); i!=m_ObjUIDCaches.end(); i++) {
			MMatchObject* pObj = (MMatchObject*)(*i).second;
			if( pObj->GetStageState() == MOSS_READY )	///< Ready한 녀석들만..
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
			// 래더게임이면 선승제로 세팅
			m_StageSetting.SetTeamWinThePoint(true);
		}
		break;
	}

	m_nStageType = nStageType;
}

void MMatchStage::OnStartGame()
{
	// Death, Kill 카운트를 0으로 리셋
	for (MUIDRefCache::iterator i=m_ObjUIDCaches.begin(); i!=m_ObjUIDCaches.end(); i++) 
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		pObj->SetAllRoundDeathCount(0);	
		pObj->SetAllRoundKillCount(0);
		pObj->SetVoteState( false );
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


	// 게임 시작 메세지를 보낸다.
	if (GetStageType() == MST_NORMAL) {
		if( IsRelayMap() && IsStartRelayMap() ) {
			MMatchServer::GetInstance()->StageRelayLaunch(GetUID());
		} else {
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
			int nRedTeamCount=0, nBlueTeamCount=0;

			GetTeamMemberCount(&nRedTeamCount, &nBlueTeamCount, NULL, true);

			
			if( nBlueTeamCount==0 || nRedTeamCount==0 )
			{
				// 이긴팀이 나가면 드로우
				if( ( nBlueTeamCount==0 && (m_Teams[MMT_BLUE].nScore > m_Teams[MMT_RED].nScore) ) ||
					( nRedTeamCount==0 && (m_Teams[MMT_RED].nScore > m_Teams[MMT_BLUE].nScore) ) )
				{
					m_Teams[MMT_RED].nScore = m_Teams[MMT_BLUE].nScore = 0;
					bIsDrawGame = true;
				}
				// red팀 승리
				else if ((m_Teams[MMT_RED].nScore > m_Teams[MMT_BLUE].nScore) )
				{
					nWinnerTeam = MMT_RED;
				}
				// blue팀 승리
				else if ((m_Teams[MMT_RED].nScore < m_Teams[MMT_BLUE].nScore) )
				{
					nWinnerTeam = MMT_BLUE;
				}
				else 
				{
					bIsDrawGame = true;
				}
			}
			// red팀 승리
			else if ((m_Teams[MMT_RED].nScore > m_Teams[MMT_BLUE].nScore) )
			{
				nWinnerTeam = MMT_RED;
			}
			// blue팀 승리
			else if ((m_Teams[MMT_RED].nScore < m_Teams[MMT_BLUE].nScore) )
			{
				nWinnerTeam = MMT_BLUE;
			}
			// draw
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

	// Ready Reset
	for (MUIDRefCache::iterator i=m_ObjUIDCaches.begin(); i!=m_ObjUIDCaches.end(); i++) {
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if (pObj->GetStageState()) pObj->SetStageState(MOSS_NONREADY);
		pObj->SetLaunchedGame(false);
	}


/*	최종결과 나올떄 다른사람이 이미 나간상태가 되어서 봉인 -_-
	MMatchServer* pServer = MMatchServer::GetInstance();
	for (MUIDRefCache::iterator i=m_ObjUIDCaches.begin(); i!=m_ObjUIDCaches.end(); i++) {
		MMatchObject* pObj = (MMatchObject*)(*i).second;

		MCommand* pCmd = pServer->CreateCommand(MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT, pServer->GetUID());
		pCmd->AddParameter(new MCmdParamUID(pObj->GetUID()));
		pCmd->AddParameter(new MCmdParamUID(GetUID()));
		pServer->Post(pCmd);
	}
	*/

	SetDisableAllCheckResourceCRC32Cache();

	m_nStartTime = 0;
}

bool MMatchStage::CheckBattleEntry()
{
	bool bResult = true;
	for (MUIDRefCache::iterator i=m_ObjUIDCaches.begin(); i!=m_ObjUIDCaches.end(); i++) {
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
	for (MUIDRefCache::iterator itor=GetObjBegin(); itor!=GetObjEnd(); ++itor) 
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

	int nItemID=0;
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

	// worlditem id가 100이상인 것만 
	if (nItemID < 100) return;

	if( 201 == nItemID )
	{
		// 201번 아이템은 포탈이다.
		// 포탈은 서버로 요청을 하지 않는다.
		// 이 부분은 급한 문제라 하드 코딩을 했다. 
		// 후에 따로 관리하는 작업을 해줘야 한다. - by SungE 2007-04-04

		mlog( "Potal hacking detected. AID(%u)\n", pObj->GetAccountInfo()->m_nAID );
		
		return;
	}

	m_WorldItemManager.SpawnDynamicItem(pObj, nItemID, x, y, z, fDropDelayTime);
}

void MMatchStage::SpawnServerSideWorldItem(MMatchObject* pObj, const int nItemID, 
							const float x, const float y, const float z, 
							int nLifeTime, int* pnExtraValues )
{
	if (GetState() != STAGE_STATE_RUN) return;

	m_WorldItemManager.SpawnDynamicItem(pObj, nItemID, x, y, z, nLifeTime, pnExtraValues );
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
	// 팀보너스 초기화
	m_TeamBonus.bApplyTeamBonus = false;

	for (int i = 0; i < MMT_END; i++)
	{
		m_Teams[i].nTeamBonusExp = 0;
		m_Teams[i].nTeamTotalLevel = 0;
		m_Teams[i].nTotalKills = 0;
	}

	int nRedTeamCount = 0, nBlueTeamCount = 0;

	// Setup Life
	for (MUIDRefCache::iterator i=GetObjBegin(); i!=GetObjEnd(); i++) {
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
	if( MMT_END > nTeam )
		m_Teams[nTeam].nTeamBonusExp += nExp;
}

void MMatchStage::ResetTeamBonus()
{
		m_Teams[MMT_BLUE].nTeamBonusExp = 0;
		m_Teams[MMT_RED].nTeamBonusExp = 0;
}

void MMatchStage::OnApplyTeamBonus(MMatchTeam nTeam)
{
	if( MMT_END <= nTeam )
		return;

	if (GetStageType() != MMATCH_GAMETYPE_DEATHMATCH_TEAM2)		// 으아아악 이런 갓뎀코드를 만들다니 -_-;
	{
		for (MUIDRefCache::iterator i=GetObjBegin(); i!=GetObjEnd(); i++) 
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
		for (MUIDRefCache::iterator i=GetObjBegin(); i!=GetObjEnd(); i++) 
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


		for (MUIDRefCache::iterator i=GetObjBegin(); i!=GetObjEnd(); i++) 
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
	if( MMT_END <= nWinnerTeam )
		return;

	// 팀 보너스 적용
	if (IsApplyTeamBonus())
	{
		OnApplyTeamBonus(nWinnerTeam);
	}
	m_Teams[nWinnerTeam].nScore++;

	// 연승 기록
	m_Teams[nWinnerTeam].nSeriesOfVictories++;
	m_Teams[NegativeTeam(nWinnerTeam)].nSeriesOfVictories = 0;

	// 오토 팀밸런스 체크
	if (CheckAutoTeamBalancing())
	{
		ShuffleTeamMembers();
	}
}


// Ladder서버는 팀의 ID, 클랜전일 경우 클랜 ID가 들어간다.
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

	for (MUIDRefCache::iterator i=GetObjBegin(); i!=GetObjEnd(); i++) 
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if (!IsEnabledObject(pObj)) continue;

		if (nMinLevel > pObj->GetCharInfo()->m_nLevel) nMinLevel = pObj->GetCharInfo()->m_nLevel;
	}

	return nMinLevel;
}


bool MMatchStage::CheckUserWasVoted( const MUID& uidPlayer )
{
	MMatchObject* pPlayer = MMatchServer::GetInstance()->GetObject( uidPlayer );
	if( !IsEnabledObject(pPlayer) )
		return false;

	MVoteMgr* pVoteMgr = GetVoteMgr();
	if( 0 == pVoteMgr )
		return false;

	if( !pVoteMgr->IsGoingOnVote() )
		return false;

	MVoteDiscuss* pVoteDiscuss = pVoteMgr->GetDiscuss();
	if(  0 == pVoteDiscuss )
		return false;

	string strVoteTarget = pVoteDiscuss->GetImplTarget();
	if( (0 != (strVoteTarget.size() - strlen(pPlayer->GetName()))) )
		return false;
	
	if( 0 != strncmp(strVoteTarget.c_str(),pPlayer->GetName(), strVoteTarget.size()) )
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
	// 래더게임이나 팀게임이 아니면 하지 않는다.
	if ((m_nStageType == MST_LADDER) || (m_StageSetting.IsTeamPlay() == false)) return;
	if (m_ObjUIDCaches.empty()) return;

	int nTeamMemberCount[MMT_END] = {0, };
	MMatchTeam nWinnerTeam;

	GetTeamMemberCount(&nTeamMemberCount[MMT_RED], &nTeamMemberCount[MMT_BLUE], NULL, true);
	if (nTeamMemberCount[MMT_RED] >= nTeamMemberCount[MMT_BLUE]) nWinnerTeam = MMT_RED; 
	else nWinnerTeam = MMT_BLUE;

	int nShuffledMemberCount = abs(nTeamMemberCount[MMT_RED] - nTeamMemberCount[MMT_BLUE]) / 2;
	if (nShuffledMemberCount <= 0) return;

	vector<MMatchObject*> sortedObjectList;

	for (MUIDRefCache::iterator i=m_ObjUIDCaches.begin(); i!=m_ObjUIDCaches.end(); i++) 
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

	// 메세지 전송
	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_RESET_TEAM_MEMBERS, MUID(0,0));
	int nMemberCount = (int)m_ObjUIDCaches.size();
	void* pTeamMemberDataArray = MMakeBlobArray(sizeof(MTD_ResetTeamMembersData), nMemberCount);

	nCounter = 0;
	for (MUIDRefCache::iterator i=m_ObjUIDCaches.begin(); i!=m_ObjUIDCaches.end(); i++) 
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

	int nMemberCount[MMT_END] = {0, };
	GetTeamMemberCount(&nMemberCount[MMT_RED], &nMemberCount[MMT_BLUE], NULL, true);

	// 2명 이상 인원수가 차이나고 인원수 많은 팀이 3연승이상 계속될 경우 팀을 섞는다.
	const int MEMBER_COUNT = 2;
	const int SERIES_OF_VICTORIES = 3;

//	const int MEMBER_COUNT = 1;
//	const int SERIES_OF_VICTORIES = 2;

	if ( ((nMemberCount[MMT_RED] - nMemberCount[MMT_BLUE]) >= MEMBER_COUNT) && 
		 (m_Teams[MMT_RED].nSeriesOfVictories >= SERIES_OF_VICTORIES) )
	{
		return true;
	}
	else if ( ((nMemberCount[MMT_BLUE] - nMemberCount[MMT_RED]) >= MEMBER_COUNT) && 
		 (m_Teams[MMT_BLUE].nSeriesOfVictories >= SERIES_OF_VICTORIES) )
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

	for (MUIDRefCache::iterator itor=GetObjBegin(); itor!=GetObjEnd(); itor++) 
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

	for ( MUIDRefCache::iterator i = GetObjBegin();  i != GetObjEnd();  i++)
	{
		MMatchObject* pObj = (MMatchObject*)((*i).second);
		
		if ( IsAdminGrade(pObj) && pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide))
			continue;

		nPlayers++;
	}

	return nPlayers;
}


bool MMatchStage::CheckDuelMap()
{
	if( MMATCH_GAMETYPE_DUEL != m_StageSetting.GetGameType() )
		return true;

	// 클랜전은 pChannel이 NULL이 되므로 추가적인 처리가 필요하다. - by SungE 2007-03-21.
	// 클랜전은 서버에서 맵을 선택해 주기때문에 처리할 필요가 없다. - by SungE 2007-04-12
	if( MGetServerConfig()->IsClanServer() )
		return true;
	
	MChannelRule* pRule = GetStageChannelRule();
	if( NULL == pRule )
		return false;

	MChannelRuleMapList* pChannelRuleMapList = pRule->GetMapList();
	if( NULL == pChannelRuleMapList )
		return false;

	// 여기까지 오면은 반드시 듀얼 모드여야 한다. - by SungE 2007-03-20
	if( !pChannelRuleMapList->Exist(GetMapName(), true)	) 
	{
		MMatchServer* pServer = MGetMatchServer();

		MMatchObject* pMaster = pServer->GetObject( GetMasterUID() );
		if( NULL == pMaster )
			return false;

		MCommand* pCmd = pServer->CreateCommand( MC_GAME_START_FAIL, MUID(0, 0) );
		if( 0 == pCmd )
			return false;

		pCmd->AddParameter( new MCmdParamInt(INVALID_MAP) );
		pCmd->AddParameter( new MCmdParamUID(MUID(0, 0)) );

		pServer->RouteToListener( pMaster, pCmd );
		
		return false;
	}

	return true;
}


bool MMatchStage::CheckTicket( MMatchObject* pObj )
{
	if( NULL == pObj )			
		return false;

	if( IsAdminGrade(pObj) )	
		return true;

	// 현제 티켓을 사용하는 나라가 없음.
	// 만약 티켓을 사용한다면 서버별, 채널별 설정을 코드에 넣으면 않된다.
	// 이 부분은 정리를 해서 따로 정책을 저장해 놓을수 있는 외부 리소스로 분리를 해야 한다.
	// - by SungE 2007-03-15

	//// 일반 서버일때... 자유/사설/클랜 채널이면 안된다.
	//if ( MGetServerConfig()->GetServerMode() == MSM_NORMAL)
	//{
	//	if ( _stricmp( pChannel->GetRuleName() , MCHANNEL_RULE_NOVICE_STR) == 0)
	//		bInvalid = true;
	//}
	//// 그 외 서버일때... 자유 채널이면 안된다.
	//else
	//{
	//	if ( (pChannel->GetChannelType() == MCHANNEL_TYPE_PRESET) &&
	//		(_stricmp( pChannel->GetRuleName() , MCHANNEL_RULE_NOVICE_STR) == 0))
	//		bInvalid = true;
	//}

	if( !m_StageSetting.IsCheckTicket() )
		return true;

	//static int nMax = 3;
	//static int n = 0;
	//while( nMax > n++ )
	//{
	//	MMatchChannel* pCh = MGetMatchServer()->FindChannel( GetOwnerChannel() );

	//	MGetMatchServer()->LOG( MMatchServer::LOG_PROG, "stage use ticket : %d\n", m_StageSetting.IsCheckTicket() );
	//	MGetMatchServer()->LOG( MMatchServer::LOG_PROG, "ticket channel : %d\n", pCh->IsTicketChannel() );
	//	MGetMatchServer()->LOG( MMatchServer::LOG_PROG, "channel use ticket : %d\n", pCh->IsUseTicket() );
	//}

	// 입장권을 가지고 있는지 검사한다.
	if( !pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide) &&
		pObj->GetCharInfo()->m_ItemList.IsHave(m_StageSetting.GetTicketItemID()) )
		return true;

	MCommand* pCmd = MGetMatchServer()->CreateCommand( MC_GAME_START_FAIL, MUID(0, 0) );
	if( 0 != pCmd )
	{
		pCmd->AddParameter( new MCmdParamInt(INVALID_TACKET_USER) );
		pCmd->AddParameter( new MCmdParamUID(pObj->GetUID()) );

		MGetMatchServer()->RouteToStage( GetUID(), pCmd );
	}

	return false;
}


bool MMatchStage::CheckQuestGame()
{
	// 퀘스트 서버가 아닌데 퀘스트 모드이면 시작이 안된다.
	if( MGetGameTypeMgr()->IsQuestDerived(GetStageSetting()->GetGameType()) ) 
	{
		if( !QuestTestServer() )
			return false;
	}
	else
	{
		return true;
	}
	
	MMatchRuleBaseQuest* pRuleQuest = static_cast< MMatchRuleBaseQuest* >( GetRule() );
	if( 0 == pRuleQuest )
		return false;

	// 퀘스트 게임을 시작하는데 필요한 준비 작업이 정상적으로 처리되었는지 검사.
	// 실패하면 퀘스트를 시작할수 없음.
	// 해당 command는 MMatchRuleBaseQuest관련 함수에서 담당함.
	if( pRuleQuest->PrepareStart() )
	{
		// 여기서 NPC정보를 보내준다.
	}
	else
	{
		

		return false;
	}	

	return true;
}


bool MMatchStage::SetMapName( char* pszMapName )
{
	// DEBUG모드에서는 모든 맵을 사용 할 수 있다. - by SungE 2007-06-05
//#ifndef _DEBUG
	if( !IsValidMap(pszMapName) )
	{
		mlog( "map haking : invlid map name setting." );

		DWORD dwCID = 0;
		MMatchObject* pObj = GetObj( GetMasterUID() );
		if( NULL != pObj )
		{
			if( NULL != pObj->GetCharInfo() )
			{
				dwCID = pObj->GetCharInfo()->m_nCID;
				mlog( " CID(%u)", dwCID );
			}
		}

		mlog(".\n");
		
		return false;
	}
//#endif

	m_StageSetting.SetMapName( pszMapName );

	return true;
}


MChannelRule* MMatchStage::GetStageChannelRule()
{
	MMatchServer* pServer = MGetMatchServer();

	MMatchChannel* pChannel = pServer->FindChannel( GetOwnerChannel() );
	if( NULL == pChannel )
		return NULL;

	return MGetChannelRuleMgr()->GetRule( pChannel->GetRuleType() );
}


bool MMatchStage::IsValidMap( const char* pMapName )
{
	if( NULL == pMapName )
		return false;

	// return true; // 퀘스트 모드와 게임 모드 변경시 문제가 있어서 보류... CheckDuelMap활성 -by SungE 2007-04-19

	// 클랜전은 pChannel이 NULL이 되므로 추가적인 처리가 필요하다. - by SungE 2007-03-21.
	// 클랜전은 서버에서 맵을 선택해 주기때문에 처리할 필요가 없다. - by SungE 2007-04-12
	if( MGetServerConfig()->IsClanServer() )
		return true;

	// 퀘스트는 제외한다.
	if( MGetGameTypeMgr()->IsQuestDerived(GetStageSetting()->GetGameType()) ) 
		return true;

	MChannelRule* pRule = GetStageChannelRule();
	if( NULL == pRule )
		return false;

	bool IsDuel = false;
	bool IsCTF = false;
	if( MMATCH_GAMETYPE_DUEL == m_StageSetting.GetGameType() )
		IsDuel = true;

	if( MMATCH_GAMETYPE_CTF == m_StageSetting.GetGameType() )
		IsCTF = true;

	// 퀘스트 모드에선 사용하지 않기로 함...
	//if( MGetGameTypeMgr()->IsQuestDerived(GetStageSetting()->GetGameType()) ) 
	//{
	//	// 퀘스트 모드에선 다음 맵만 허용... 하드 코딩... 아놔... 좀 만들지... =_=
	//	if ( _stricmp( GetStageSetting()->GetMapName(), "mansion") == 0)			return true;
	//	else if ( _stricmp( GetStageSetting()->GetMapName(), "prison") == 0)		return true;
	//	else if ( _stricmp( GetStageSetting()->GetMapName(), "dungeon") == 0)	return true;
	//	return false;
	//}
	//else
	//{
	//	if( !pRule->CheckGameType(m_StageSetting.GetGameType()) )
	//		return false;
	//}

	if( !pRule->CheckGameType(m_StageSetting.GetGameType()) )
			return false;

	if(IsCTF)
	return pRule->CheckCTFMap(pMapName);


	return pRule->CheckMap( pMapName, IsDuel);
}


void MMatchStage::ReserveSuicide( const MUID& uidUser, const DWORD dwExpireTime )
{
	vector< MMatchStageSuicide >::iterator it, end;
	end = m_SuicideList.end();
	for( it = m_SuicideList.begin(); it != end; ++it )
	{
		if( uidUser == it->m_uidUser )
			return;
	}

	MMatchStageSuicide SuicideUser( uidUser, dwExpireTime + 10000 );

	m_SuicideList.push_back( SuicideUser );

	MCommand* pNew = MGetMatchServer()->CreateCommand( MC_MATCH_RESPONSE_SUICIDE_RESERVE, uidUser );
	if( NULL == pNew )
		return;

	MGetMatchServer()->PostSafeQueue( pNew );
}


void MMatchStage::CheckSuicideReserve( const DWORD dwCurTime )
{
	// 한번에 하나씩만 처리를 한다. 
	vector< MMatchStageSuicide >::iterator it, end;
	end = m_SuicideList.end();
	for( it = m_SuicideList.begin(); it != end; ++it )
	{
		if( (false == it->m_bIsChecked) && (dwCurTime > it->m_dwExpireTime) )
		{
			MMatchObject* pObj = GetObj( it->m_uidUser );
			if( NULL == pObj )
			{
				m_SuicideList.erase( it );
				break;
			}
				
			// MGetMatchServer()->OnGameKill( it->m_uidUser, it->m_uidUser );
			/////////////////////
			//			_ASSERT( 0 );
			MMatchStage* pStage = MMatchServer::GetInstance()->FindStage(pObj->GetStageUID());
			if (pStage == NULL) break;
			if (pObj->CheckAlive() == false)	break;

			pObj->OnDead();
	//		MBMatchServer::GetInstance()->pro//			MBMatchServer::GetInstance()- ->ProcessOnGameKill(pStage, pObj, pObj);
			pStage->OnGameKill(pObj->GetUID(), pObj->GetUID());	

			/////////////////////////////////////

			MCommand* pNew = MGetMatchServer()->CreateCommand( MC_MATCH_RESPONSE_SUICIDE, MUID(0, 0) );
			pNew->AddParameter( new MCommandParameterInt(MOK) );
			pNew->AddParameter( new MCommandParameterUID(it->m_uidUser) );
			MGetMatchServer()->RouteToBattle( GetUID(), pNew );

			// 한번 자살을 요청하면 3분 동안은 자살을 요청 할 수 없다.
			it->m_dwExpireTime	= dwCurTime + MIN_REQUEST_SUICIDE_TIME;
			it->m_bIsChecked	= true;

			if ( MGetGameTypeMgr()->IsQuestDerived(pStage->GetStageSetting()->GetGameType()))
			{ // 퀘스트 및 서바이버에서 자살할때 자살 본인외에는 죽는 처리가 안돼있어서 추가함
				// 죽었다는 메세지 보냄
				MCommand* pCmd = MGetMatchServer()->CreateCommand(MC_MATCH_QUEST_PLAYER_DEAD, MUID(0,0));
				pCmd->AddParameter(new MCommandParameterUID(it->m_uidUser));
				MGetMatchServer()->RouteToBattle(pStage->GetUID(), pCmd);	
			}
			
			break;
		}
		else if( (true == it->m_bIsChecked) && (dwCurTime > it->m_dwExpireTime) )
		{
			// 한번 자살 처리를 하고 3분이 지나면 이곳으로 들어온다.
			// 여기서 삭제를 해야 다음 요청에 리스트에 다시 등록될 수 있다.
			m_SuicideList.erase( it );
			break;
		}
	}
}


void MMatchStage::ResetPlayersCustomItem()
{
	for (MUIDRefCache::iterator itor=GetObjBegin(); itor!=GetObjEnd(); itor++) 
	{
		MMatchObject* pObj = (MMatchObject*)(*itor).second;

		pObj->ResetCustomItemUseCount();
	}
}


void MMatchStage::MakeResourceCRC32Cache( const DWORD dwKey, DWORD& out_crc32, DWORD& out_xor )
{
	MMatchCRC32XORCache CRC32Cache;

	CRC32Cache.Reset();
	CRC32Cache.CRC32XOR( dwKey );

#ifdef _DEBUG
	mlog( "Start ResourceCRC32Cache : %u/%u\n", CRC32Cache.GetCRC32(), CRC32Cache.GetXOR() );
#endif

	MakeItemResourceCRC32Cache( CRC32Cache );


#ifdef _DEBUG
	static DWORD dwOutputCount = 0;
	if( 10 > (++dwOutputCount) )
	{
		mlog( "ResourceCRC32XOR : %u/%u\n", CRC32Cache.GetCRC32(), CRC32Cache.GetXOR() );
	}
#endif

	out_crc32 = CRC32Cache.GetCRC32();
	out_xor = CRC32Cache.GetXOR();
}


/*
 EnterBattle이 완료되면 그유저에 대해서 새로운 ResourceCRC32Cache를 저장한다.
 이 시점은 클라이언트의 Resource로딩이 완료된 시점이고, 게임 중이다.
*/
void MMatchStage::SetResourceCRC32Cache( const MUID& uidPlayer, const DWORD dwCRC32Cache, const DWORD dwXORCache )
{
	ResourceCRC32CacheMap::iterator itFind = m_ResourceCRC32CacheMap.find( uidPlayer );
	if( m_ResourceCRC32CacheMap.end() == itFind )
	{
		MMATCH_RESOURCECHECKINFO CRC32CacheInfo;

		CRC32CacheInfo.dwResourceCRC32Cache	= dwCRC32Cache;
		CRC32CacheInfo.dwResourceXORCache	= dwXORCache;
		CRC32CacheInfo.dwLastRequestTime	= MGetMatchServer()->GetGlobalClockCount();
		CRC32CacheInfo.bIsEnterBattle		= true;
		CRC32CacheInfo.bIsChecked			= false;

		m_ResourceCRC32CacheMap.insert( ResourceCRC32CacheMap::value_type(uidPlayer, CRC32CacheInfo) );
	}
	else
	{
		itFind->second.dwResourceCRC32Cache	= dwCRC32Cache;
		itFind->second.dwResourceXORCache	= dwXORCache;
		itFind->second.dwLastRequestTime	= MGetMatchServer()->GetGlobalClockCount();
		itFind->second.bIsEnterBattle		= true;
		itFind->second.bIsChecked			= false;
	}	
}


void MMatchStage::RequestResourceCRC32Cache( const MUID& uidPlayer )
{
	if( !m_bIsUseResourceCRC32CacheCheck )
	{
		return;
	}

	MMatchObject* pObj = MGetMatchServer()->GetObject( uidPlayer );
	if( NULL == pObj )
	{
		return;
	}

	const DWORD dwKey = static_cast<DWORD>( RandomNumber(1, RAND_MAX) );

	DWORD dwCRC32Cache, dwXORCache;
	MakeResourceCRC32Cache( dwKey , dwCRC32Cache, dwXORCache);

	SetResourceCRC32Cache( uidPlayer, dwCRC32Cache, dwXORCache );

	MCommand* pCmd = MGetMatchServer()->CreateCommand( MC_REQUEST_RESOURCE_CRC32, uidPlayer );
	pCmd->AddParameter( new MCmdParamUInt(dwKey) );

	MGetMatchServer()->Post( pCmd );
}


void MMatchStage::DeleteResourceCRC32Cache( const MUID& uidPlayer )
{
	m_ResourceCRC32CacheMap.erase( uidPlayer );
}


const bool MMatchStage::IsValidResourceCRC32Cache( const MUID& uidPlayer, const DWORD dwResourceCRC32Cache, const DWORD dwResourceXORCache )
{
	ResourceCRC32CacheMap::iterator itFind = m_ResourceCRC32CacheMap.find( uidPlayer );
	if( m_ResourceCRC32CacheMap.end() == itFind )
	{
		mlog( "Can't find Resource crc.\n" );
		return false;
	}

	if( dwResourceCRC32Cache != itFind->second.dwResourceCRC32Cache ||
		dwResourceXORCache != itFind->second.dwResourceXORCache)
	{
		mlog( "invalid resource crc : s(%u/%u), c(%u/%u).\n"
			, itFind->second.dwResourceCRC32Cache, itFind->second.dwResourceXORCache
			, dwResourceCRC32Cache, dwResourceXORCache );

		return false;
	}

	itFind->second.bIsChecked = true;
    
	return true;
}


void MMatchStage::CheckResourceCRC32Cache( const DWORD dwClock )
{
	if( !m_bIsUseResourceCRC32CacheCheck )
	{
		return;
	}

	static const DWORD MAX_ELAPSED_UPDATE_CRC32CACHE	= 20000;
	static const DWORD CRC32CACHE_CHECK_TIME			= 10000;
#ifndef _DEBUG
	static const DWORD CRC32CACHE_CEHCK_REPEAT_TERM		= 1000 * 60 * 5;
#else
	static const DWORD CRC32CACHE_CEHCK_REPEAT_TERM		= 1000 * 10;
#endif

	if( CRC32CACHE_CHECK_TIME > (dwClock - m_dwLastResourceCRC32CacheCheckTime) )
	{
		return;
	}

	ResourceCRC32CacheMap::const_iterator	end = m_ResourceCRC32CacheMap.end();
	ResourceCRC32CacheMap::iterator			it	= m_ResourceCRC32CacheMap.begin();

	for( ; end != it; ++it )
	{
		// 게임을 진행하고 있는 유저에 대해서만 검사를 한다.
		if( !it->second.bIsEnterBattle )
		{
			continue;
		}

		if( it->second.bIsChecked )
		{
			// CRC32CACHE_CEHCK_REPEAT_TERM마다 다시 검사한다.
			if( CRC32CACHE_CEHCK_REPEAT_TERM < (dwClock - it->second.dwLastRequestTime) )
			{
				RequestResourceCRC32Cache( it->first );
			}

			continue;
		}

		if( MAX_ELAPSED_UPDATE_CRC32CACHE < (dwClock - it->second.dwLastRequestTime) )
		{
			// 허용 시간안에 응답을 안한 유저는 해킹 유저나 비정상 유저로 판단한다.
			// 한번에 하나씩 처리한다.
            MGetMatchServer()->StageLeaveBattle(it->first, true, true);
			MGetMatchServer()->StageLeave(it->first);//, GetUID() );
			
			MMatchObject* pObj = MGetMatchServer()->GetObject( it->first );
			if( (NULL != pObj) && (NULL != pObj->GetCharInfo()) )
			{
				MGetMatchServer()->LOG(MMatchServer::LOG_PROG, "dynamic resource crc32 check : hackuser(%s).\n"
					, pObj->GetCharInfo()->m_szName );
			}
			return;
		}
	}

	m_dwLastResourceCRC32CacheCheckTime = dwClock;
}


void MMatchStage::SetDisableCheckResourceCRC32Cache( const MUID& uidPlayer )
{
	if( !m_bIsUseResourceCRC32CacheCheck )
	{
		return;
	}

	ResourceCRC32CacheMap::iterator itFind = m_ResourceCRC32CacheMap.find( uidPlayer );
	if( m_ResourceCRC32CacheMap.end() == itFind )
	{
		return;
	}

	itFind->second.bIsEnterBattle = false;
}


void MMatchStage::SetDisableAllCheckResourceCRC32Cache()
{
	ResourceCRC32CacheMap::const_iterator	end = m_ResourceCRC32CacheMap.end();
	ResourceCRC32CacheMap::iterator			it	= m_ResourceCRC32CacheMap.begin();

	for( ; end != it; ++it )
	{
		it->second.bIsEnterBattle = false;
	}
}


void MMatchStage::MakeItemResourceCRC32Cache( MMatchCRC32XORCache& CRC32Cache )
{
	ClearGabageObject();

	MMatchObject*					pObj		= NULL;
	MUIDRefCache::const_iterator	end			= m_ObjUIDCaches.end();
	MUIDRefCache::iterator			it			= m_ObjUIDCaches.begin();
	MMatchItem*						pItem		= NULL;

	MMatchServer* pServer = MGetMatchServer();
	
#ifdef _DEBUG
	static DWORD dwOutputCount = 0;
	++dwOutputCount;
#endif

	for( ; end != it; ++it )
	{
		pObj = reinterpret_cast<MMatchObject*>( it->second );

		for( int i = 0; i < MMCIP_END; ++i )
		{
			pItem = pObj->GetCharInfo()->m_EquipedItem.GetItem( MMatchCharItemParts(i) );
            if( NULL == pItem )
			{
				continue;
			}

			if( NULL == pItem->GetDesc() )
			{
				continue;
			}

			pItem->GetDesc()->CacheCRC32( CRC32Cache );

#ifdef _DEBUG
			if( 10 > dwOutputCount )
			{
				MMatchItemDesc* pItemDesc = pItem->GetDesc();
				mlog( "ItemID : %d, CRCCache : %u\n"
					, pItemDesc->m_nID
					, CRC32Cache.GetXOR() );
			}
#endif
		}
	}
}


void MMatchStage::ClearGabageObject()
{
	for (MUIDRefCache::iterator i=GetObjBegin(); i!=GetObjEnd(); i++) {
		//MMatchObject* pObj = (MMatchObject*)(*i).second;

		MUID uidObj = (MUID)(*i).first;
		MMatchObject* pObj = MGetMatchServer()->GetObject(uidObj);
		if (!pObj) 
		{
			MGetMatchServer()->LogObjectCommandHistory(uidObj);
			mlog( "WARNING(RouteToBattle) : stage Not Existing Obj(%u:%u)\n", uidObj.High, uidObj.Low);
			i=RemoveObject(uidObj);	// RAONHAJE : 방에 쓰레기UID 남는것 발견시 로그&청소			
		}
	}
}

int	MMatchStage::GetDuelTournamentRandomMapIndex()
{
	MChannelRule *pChannelRule = MGetChannelRuleMgr()->GetRule(MCHANNEL_RULE_DUELTOURNAMENT);
	if( pChannelRule == NULL ) return -1;

	MChannelRuleMapList* pMapList = pChannelRule->GetMapList();
	if( pMapList == NULL ) return -1;

	int nMaxIndex = (int)pMapList->size();

	if( nMaxIndex != 0 ){

		int nRandomMapIndex;
		int nRandomVal = rand() % nMaxIndex;

		MChannelRuleMapList::iterator iter;
		for(iter = pMapList->begin(); iter != pMapList->end(); iter++)
		{
			if( nRandomVal == 0 ) 
				nRandomMapIndex = *iter;
			
			nRandomVal = nRandomVal - 1;
		}

		return nRandomMapIndex;
	}

	return -1;
}

void MMatchStage::SetDuelTournamentMatchList(MDUELTOURNAMENTTYPE nType, MDuelTournamentPickedGroup *pPickedGroup)
{
	m_nDTStageInfo.nDuelTournamentType = nType;

	///////////////////////////////////////////////////////////////////////////////

	m_nDTStageInfo.nDuelTournamentTotalRound = 0;
	m_nDTStageInfo.DuelTournamentMatchMap.clear();

	MDUELTOURNAMENTROUNDSTATE nRoundState = GetDuelTournamentRoundState(nType);
	MakeDuelTournamentMatchMap(nRoundState, 1);

	///////////////////////////////////////////////////////////////////////////////

	
	///////////////////////////////////////////////////////////////////////////////
	int nIndex = 0;

	MMatchDuelTournamentMatch *pMatch;

	map<int, MMatchDuelTournamentMatch*>::iterator iter = m_nDTStageInfo.DuelTournamentMatchMap.begin();
	for (MDuelTournamentPickedGroup::iterator i=pPickedGroup->begin(); i!= pPickedGroup->end(); i++)
	{
		if( nIndex % 2 == 0 ) {
			pMatch = iter->second;
			iter++;

			pMatch->uidPlayer1 = (*i);
			pMatch->uidPlayer2 = MUID(0, 0);
		} else {
			pMatch->uidPlayer2 = (*i);
		}

		nIndex++;		
	}

	///////////////////////////////////////////////////////////////////////////////

#ifdef _DUELTOURNAMENT_LOG_ENABLE_	
	for(map<int, MMatchDuelTournamentMatch*>::iterator iter = m_nDTStageInfo.DuelTournamentMatchMap.begin();
		iter != m_nDTStageInfo.DuelTournamentMatchMap.end(); iter++)
	{
		MMatchDuelTournamentMatch* pMatch = iter->second;

		MGetMatchServer()->LOG(MMatchServer::LOG_PROG, "RoundState=%d, Order=%d, NextOrder=%d, P1=(%d%d), P2=(%d%d)",
			pMatch->nRoundState, pMatch->nMatchNumber, pMatch->nNextMatchNumber, pMatch->uidPlayer1.High, pMatch->uidPlayer1.Low
			, pMatch->uidPlayer2.High, pMatch->uidPlayer2.Low);
	}
#endif
	
	///////////////////////////////////////////////////////////////////////////////
}

void MMatchStage::MakeDuelTournamentMatchMap(MDUELTOURNAMENTROUNDSTATE nRoundState, int nMatchNumber)
{
	if(nRoundState == MDUELTOURNAMENTROUNDSTATE_MAX ) return;

	int nRemainCount;

	switch(nRoundState){
		case MDUELTOURNAMENTROUNDSTATE_QUATERFINAL :		nRemainCount = 4; break;
		case MDUELTOURNAMENTROUNDSTATE_SEMIFINAL :			nRemainCount = 2; break;
		case MDUELTOURNAMENTROUNDSTATE_FINAL :				nRemainCount = 1; break;
	}

	int nTemp = 0;
	for(int i = 0; i < nRemainCount; i++){
		MMatchDuelTournamentMatch *pMatch = new MMatchDuelTournamentMatch;		
		memset(pMatch, 0, sizeof(MMatchDuelTournamentMatch));

		pMatch->nRoundState = nRoundState;
		pMatch->nNextMatchNumber = nMatchNumber + nRemainCount + nTemp - i;	
		pMatch->nMatchNumber = nMatchNumber;		

		pMatch->uidPlayer1 = MUID(0, 0);
		pMatch->uidPlayer2 = MUID(0, 0);

		if( nMatchNumber % 2 == 0 ) nTemp++;
		if( nRoundState == MDUELTOURNAMENTROUNDSTATE_FINAL ) pMatch->nNextMatchNumber = 0;

		m_nDTStageInfo.DuelTournamentMatchMap.insert(pair<int, MMatchDuelTournamentMatch*>(nMatchNumber++, pMatch));
		
		if( pMatch->nRoundState == MDUELTOURNAMENTROUNDSTATE_FINAL )			m_nDTStageInfo.nDuelTournamentTotalRound += 3;
		else if( pMatch->nRoundState == MDUELTOURNAMENTROUNDSTATE_SEMIFINAL )	m_nDTStageInfo.nDuelTournamentTotalRound += 3;
		else																	m_nDTStageInfo.nDuelTournamentTotalRound += 1;
	}

	MakeDuelTournamentMatchMap(GetDuelTournamentNextRoundState(nRoundState), nMatchNumber);
}

void MMatchStage::ClearDuelTournamentMatchMap()
{
	for(map<int, MMatchDuelTournamentMatch*>::iterator iter = m_nDTStageInfo.DuelTournamentMatchMap.begin();
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

	switch(nRoundState){
		case MDUELTOURNAMENTROUNDSTATE_QUATERFINAL :		nAdditionalOrder = 4; break;
		case MDUELTOURNAMENTROUNDSTATE_SEMIFINAL :			nAdditionalOrder = 2; break;
		case MDUELTOURNAMENTROUNDSTATE_FINAL :				nAdditionalOrder = 0; break;		
		default : ASSERT(0);
	}

	// nOrder가 2의 배수이면 1을 빼준다.
	if( nOrder % 2 == 0 )	nResult = nOrder + nAdditionalOrder - nTemp - 1;
	else					nResult = nOrder + nAdditionalOrder - nTemp ;

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
	for(int i=0; GetRelayMapListCount() > i; ++i)
	{
		m_vecRelayMapsRemained.push_back(pRelayMapList[i]);
	}
}
void MMatchStage::SetRelayMapListCount(int nRelayMapListCount)
{
	if(nRelayMapListCount > MAX_RELAYMAP_LIST_COUNT) 
		nRelayMapListCount = 20;
	m_StageSetting.SetRelayMapListCount(nRelayMapListCount); 
}

void MMatchStage::SetRelayMapList(RelayMap* pRelayMapList)
{
	// 연속으로 유효한 맵 갯수가 몇갠지 센다
	int count = 0;
	for (int i=0; i<MAX_RELAYMAP_LIST_COUNT; ++i)
	{
		if (!MGetMapDescMgr()->MIsCorrectMap(pRelayMapList[i].nMapID))
			break;
		++count;
	}

	// 하나도 없으면 맨션이라도 하나 넣어주자
	if (count == 0)
	{
		pRelayMapList[0].nMapID = MMATCH_MAP_MANSION;
		count = 1;
	}

	SetRelayMapListCount(count);
	m_StageSetting.SetRelayMapList(pRelayMapList);
}
