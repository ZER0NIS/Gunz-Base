#include "stdafx.h"
//#include <winsock2.h>
#include "MMatchServer.h"
#include "MMatchObject.h"
#include "MMatchGlobal.h"
#include "MMatchConfig.h"
#include "MUtil.h"

#include "MAsyncDBJob_InsertCharBRInfo.h"
#include "MAsyncDBJob_UpdateCharBRInfo.h"
#include "MAsyncDBJob_RewardCharBR.h"

#define CYCLE_MATCHSTAGELISTUPDATE			1000
#define CYCLE_MATCHCHANNELLISTUPDATE		1000
#define CYCLE_MATCHCHANNELPLAYERLISTUPDATE	1000
#define CYCLE_MATCHCHANNELCLANMEMBER		1000
#define CYCLE_MATCHBATTLETIMEREWARDUPDATE	500
#define CYCLE_CHAR_BUFF_CHECK				500


#define CYCLE_MATCH_STANDBY_CLANLIST_UPDATE	1000		// 클랜전 대기 클랜 리스트 업데이트 시간은 10초이다.



const DWORD MMatchDisconnStatusInfo::MINTERVAL_DISCONNECT_STATUS_MIN = (5 * 1000);

MMatchObject::MMatchObject(const MUID& uid) : MObject(uid) 
{ 
	m_pCharInfo = NULL;
	m_pFriendInfo = NULL;

	m_dwIP = 0;
	ZeroMemory(m_szIP, sizeof(char)*64);	
	m_nPort=0;
	
	m_uidStage = MUID(0,0);
	m_uidChatRoom = MUID(0,0);

	m_bBridgePeer = false;
	m_bRelayPeer = false;
	m_uidAgent = MUID(0,0);

	m_nPlayerFlags = 0;
	m_nUserOptionFlags = 0;

	m_ChannelInfo.Clear();

	m_bStageListTransfer = false;
	m_nStageListChecksum = 0;
	m_nStageListLastChecksum = 0;
	m_nTimeLastStageListTrans = 0;
	m_nStageCursor = 0;

	m_RefreshClientChannelImpl.SetMatchObject(this);
	m_RefreshClientClanMemberImpl.SetMatchObject(this);

	m_nTeam=MMT_ALL;
	SetLadderGroupID(0);
	m_nStageState = MOSS_NONREADY;
	m_bEnterBattle=false;
	m_bAlive=false;
	m_bForcedEntried = false;
	m_bLadderChallenging = false;
	m_nKillCount = 0;
	m_nDeathCount = 0;
	m_nPlace = MMP_OUTSIDE;
	m_bLaunchedGame = false;
	m_nAllRoundDeathCount = 0;
	m_nAllRoundKillCount = 0;
	m_bNewbie = false;
	m_nDeadTime = 0;

	m_GameInfo.bJoinedGame = false;

	m_bDBFriendListRequested = false;

	m_nTickLastPacketRecved = 0;
	m_nLastHShieldMsgRecved = 0;
	m_bHShieldMsgRecved = false;
	m_bHacker = false;

	//m_dwLastHackCheckedTime			= timeGetTime();
	//m_dwLastRecvNewHashValueTime	= timeGetTime();
	//m_bIsRequestNewHashValue		= false;

	m_dwLastSpawnTime				= timeGetTime();

	m_dwHShieldCheckCount = 0;

	m_nLastPingTime = m_nQuestLatency = 0;
	m_bQuestRecvPong = true;

	m_bIsLoginCompleted				= false;
	m_dwLastSendGambleItemTime		= 0;
	m_IsSendFirstGameguardRequest	= false;
	m_IsRecvFirstGameguardResponse	= false;

	m_DBJobQ.bIsRunningAsyncJob		= false;
	m_DBJobQ.nLastJobID				= 0;

	m_pDuelTournamentCharInfo = NULL;
	m_CharBuffInfo.SetObject(this);

	m_nLastCheckBattleTimeReward = 0;
}

MMatchObject::~MMatchObject()
{
	FreeCharInfo();
	FreeFriendInfo();
	FreeDuelTournamentInfo();

	LoginNotCompleted();
}

void MMatchObject::FreeCharInfo()
{
	if (m_pCharInfo) {
		m_pCharInfo->Clear();
		delete m_pCharInfo;
		m_pCharInfo = NULL;
	}
}

void MMatchObject::FreeFriendInfo()
{
	if (m_pFriendInfo) {
		delete m_pFriendInfo;
		m_pFriendInfo = NULL;
	}
	m_bDBFriendListRequested = false;
}

void MMatchObject::FreeDuelTournamentInfo()
{
	if(m_pDuelTournamentCharInfo){
		delete m_pDuelTournamentCharInfo;
		m_pDuelTournamentCharInfo = NULL;
	}
}

void MMatchObject::SetTeam(MMatchTeam nTeam)
{ 
	m_nTeam = nTeam; 

	if (IsAdminGrade(this) && CheckPlayerFlags(MTD_PlayerFlags_AdminHide))
		m_nTeam = MMT_SPECTATOR;
}

void MMatchObject::SetStageCursor(int nStageCursor)
{
	m_nStageCursor = nStageCursor;
}

void MMatchObject::SetPlace(MMatchPlace nPlace)
{
	m_nPlace = nPlace;

	switch(m_nPlace) {
	case MMP_OUTSIDE:
		{
			MRefreshClientChannelImpl* pChannelImpl = GetRefreshClientChannelImplement();
			pChannelImpl->Enable(false);
		}
		break;
	case MMP_LOBBY:
		{
			MRefreshClientChannelImpl* pChannelImpl = GetRefreshClientChannelImplement();
			pChannelImpl->Enable(true);
		}
		break;
	case MMP_STAGE:
		{
			MRefreshClientChannelImpl* pChannelImpl = GetRefreshClientChannelImplement();
			pChannelImpl->Enable(false);
		}
		break;
	case MMP_BATTLE:
		{
			MRefreshClientChannelImpl* pChannelImpl = GetRefreshClientChannelImplement();
			pChannelImpl->Enable(false);
		}
		break;
	default:
		{
		}break;
	};
}

void MMatchObject::Tick(unsigned long int nTime)
{
	MMatchServer* pServer = MMatchServer::GetInstance();

	if (CheckStageListTransfer() == true) {
		// 로비에서 클랜채널에 있으면 클랜전 대기 클랜 리스트 업데이트해준다.
		MMatchChannel* pChannel = pServer->FindChannel(GetChannelUID());
		if ((MGetServerConfig()->GetServerMode() == MSM_CLAN) && (pChannel) && (pChannel->GetChannelType() == MCHANNEL_TYPE_CLAN))
		{
			if ((unsigned int)(nTime - m_nTimeLastStageListTrans) > CYCLE_MATCH_STANDBY_CLANLIST_UPDATE) {
				unsigned long int nCurrStageListChecksum = pServer->GetLadderMgr()->GetChecksum(m_nStageCursor, 
																			TRANS_STANDBY_CLANLIST_NODE_COUNT);
				if (nCurrStageListChecksum != GetStageListChecksum()) {
					m_nTimeLastStageListTrans = nTime;

					pServer->StandbyClanList(GetUID(), m_nStageCursor, true);
					UpdateStageListChecksum(nCurrStageListChecksum);
				}
			}
		}
		else
		{
			// 일반적인 스테이지 리스트 업데이트
			if ((unsigned int)(nTime - m_nTimeLastStageListTrans) > CYCLE_MATCHSTAGELISTUPDATE) {
				unsigned long int nCurrStageListChecksum = pServer->GetStageListChecksum(m_ChannelInfo.uidChannel, 
																		m_nStageCursor, TRANS_STAGELIST_NODE_COUNT);
				if (nCurrStageListChecksum != GetStageListChecksum()) {
					m_nTimeLastStageListTrans = nTime;

					pServer->StageList(GetUID(), m_nStageCursor, true);
					UpdateStageListChecksum(nCurrStageListChecksum);
				}
			}
		}
	}

	if (CheckChannelListTransfer() == true) {
		if ((unsigned int)(nTime - m_ChannelInfo.nTimeLastChannelListTrans) > CYCLE_MATCHCHANNELLISTUPDATE) {
			if (pServer->GetChannelListChecksum() != GetChannelListChecksum()) {
				m_ChannelInfo.nTimeLastChannelListTrans = nTime;

				if ((m_ChannelInfo.nChannelListType != MCHANNEL_TYPE_CLAN) || (GetChannelListChecksum() == 0))
				{
					pServer->ChannelList(GetUID(), m_ChannelInfo.nChannelListType);
					UpdateChannelListChecksum(pServer->GetChannelListChecksum());				
				}
			}
		}
	}

	if (GetRefreshClientChannelImplement()->IsEnable()) {
		if (nTime - GetRefreshClientChannelImplement()->GetLastUpdatedTime() > CYCLE_MATCHCHANNELPLAYERLISTUPDATE) {
			GetRefreshClientChannelImplement()->SetLastUpdatedTime(nTime);

			MMatchChannel* pChannel = pServer->FindChannel(GetChannelUID());
			if (pChannel) {
				pChannel->SyncPlayerList(this, GetRefreshClientChannelImplement()->GetCategory());
			}
		}
	}

	if (GetRefreshClientClanMemberImplement()->IsEnable()) {
		if (nTime - GetRefreshClientClanMemberImplement()->GetLastUpdatedTime() > CYCLE_MATCHCHANNELCLANMEMBER) {
			GetRefreshClientClanMemberImplement()->SetLastUpdatedTime(nTime);

			MMatchClan* pClan = pServer->FindClan(GetCharInfo()->m_ClanInfo.m_nClanID);
			if (pClan) {
				pClan->SyncPlayerList(this, GetRefreshClientClanMemberImplement()->GetCategory());
			}
		}
	}
	

	if( GetEnterBattle() && m_pCharInfo != NULL )
	{
		if( nTime - GetLastCheckBattleTimeReward() > CYCLE_MATCHBATTLETIMEREWARDUPDATE )
		{
			SetLastCheckBattleTimeReward(nTime);

			BattleTimeReward(nTime);
		}		
	}

	GetDisconnStatusInfo().Update( nTime );

	//버프정보임시주석 
	/*if( m_pCharInfo ) {		
		if( nTime - m_CharBuffInfo.GetLastCheckBuffInfoTime() > CYCLE_CHAR_BUFF_CHECK ) {			
			int nPlayTime = MGetTimeDistance(m_pCharInfo->m_nConnTime, nTime) / 1000;
			m_pCharInfo->UpdatePlayTime(nPlayTime);

			bool bChanged = m_CharBuffInfo.Tick(nTime);
			if( bChanged ) {
				pServer->PostCmdCharacterBuffInfo(m_UID);
			}

			m_CharBuffInfo.SetLastCheckBuffInfoTime(nTime);
		}
	}*/
}

void MMatchObject::SetBattleTimeReward(bool bVal)
{
	if( bVal )
	{
		unsigned long int nTime = MMatchServer::GetInstance()->GetGlobalClockCount();

		MMatchCharBattleTimeRewardInfoMap::iterator iter = m_pCharInfo->GetBRInfoMap().begin();
		for( ; iter !=  m_pCharInfo->GetBRInfoMap().end(); iter++ )
		{
			MMatchCharBRInfo* pInfo = iter->second;
			pInfo->SetLastCheckTime(nTime);
			pInfo->SetLastUpdateDBTime(nTime);
		}
	}
}

void MMatchObject::BattleTimeReward(unsigned int nTime)
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	MMatchBRDescriptionMap pMap = pServer->GetBattleTimeRewardMachine().GetBattleTimeRewardDescriptionMap();
	for( MMatchBRDescriptionMap::iterator iter = pMap.begin(); iter != pMap.end(); iter++ )
	{
		MMatchBRDescription* pDesc = iter->second;

		MMatchCharBRInfo* pInfo = m_pCharInfo->GetBRInfoMap().Get(pDesc->GetBRID());

		// 만약 Description은 있지만, 정보가 없다면.. 새로 만들어서 삽입!
		// (DB에 정보가 삽입될 때까지는 체크하지 않는다)
		if( pInfo == NULL )
		{	
			pInfo = new MMatchCharBRInfo(pDesc->GetBRID(), pDesc->GetBRTID(), 0, 0, 0);

			pInfo->SetLastCheckTime(nTime);
			pInfo->SetLastUpdateDBTime(nTime);

			if( m_pCharInfo->GetBRInfoMap().Insert(pInfo->GetBRID(), pInfo) )
			{
				MAsyncDBJob_GetCharBRInfo* pJob = new MAsyncDBJob_GetCharBRInfo(m_UID);
				pJob->Input(m_pCharInfo->m_nCID, pDesc->GetBRID(), pDesc->GetBRTID());
				m_DBJobQ.DBJobQ.push_back(pJob);
			}
		}
		else
		{
			if( !pInfo->IsCheckSkip() )
			{
				BRRESULT nResult = pInfo->CheckBattleTimeReward(nTime, pDesc);

				if( nResult == BRRESULT_DO_REWARD )
				{
					MMatchBRItem* pBRItem = pDesc->GetRewardItem();
					if( pBRItem == NULL ) { return; }

					pServer->OnAsyncRequest_RewardCharBP(m_UID, pInfo->GetBRID(), pInfo->GetBRTID(), 
						pInfo->GetRewardCount(), pInfo->GetBattleTime(), pInfo->GetKillCount(), 
						pBRItem->GetItemID((int)m_pCharInfo->m_nSex), pBRItem->GetItemCnt(), pBRItem->GetRentHourPeriod());

					m_pCharInfo->GetBRInfoMap().Remove(pInfo->GetBRID());
				}
				else if( nResult == BRRESULT_RESET_INFO )
				{
					pInfo->ResetInfo();

					pServer->RouteCmdBattleTimeReward(m_UID, GetStageUID(), pDesc->GetName().c_str(), pDesc->GetResetDesc().c_str(), 0, 0, 0, pDesc->GetRewardCount());
				}
				else if( nResult == BRRESULT_NO_REWARD )
				{
					if( pInfo->IsNeedUpdateDB(nTime) )
					{
						pServer->OnAsyncRequest_UpdateCharBRInfo(m_UID, pInfo->GetBRID(), pInfo->GetBRTID(), 
							pInfo->GetRewardCount(), pInfo->GetBattleTime(), pInfo->GetKillCount());
					}
					else if( pInfo->IsExpired(pDesc->GetBRTID()) )
					{						
#ifdef _DEBUG
						mlog("CID(%d), 의 BR Info(%d, %d, %d, %d)가 만료되었습니다.\n", m_pCharInfo->m_nCID, pInfo->GetBRID(), pInfo->GetBRTID(), 
							pInfo->GetBattleTime(), pInfo->GetKillCount());
#endif

						pServer->OnAsyncRequest_UpdateCharBRInfo(m_UID, pInfo->GetBRID(), pInfo->GetBRTID(), 
							pInfo->GetRewardCount(), pInfo->GetBattleTime(), pInfo->GetKillCount());

						m_pCharInfo->GetBRInfoMap().Remove(pInfo->GetBRID());
					}
				}								
			}
		}
	}
}

void MMatchObject::OnStageJoin()
{
	SetAllRoundDeathCount(0);	
	SetAllRoundKillCount(0);
	SetStageListTransfer(false);	
	SetForcedEntry(false);
	SetPlace(MMP_STAGE);
	m_GameInfo.bJoinedGame = false;
	m_nDeadTime = 0;
}

void MMatchObject::OnEnterBattle()
{	
	SetAlive(false);
	SetKillCount(0);
	SetDeathCount(0);
	SetAllRoundDeathCount(0);	
	SetAllRoundKillCount(0);

	SetEnterBattle(true);
	SetBattleTimeReward(true);

	ResetGamePlayInfo();
}

void MMatchObject::OnLeaveBattle()
{	
	SetKillCount(0);
	SetDeathCount(0);
	SetAlive(false);
	SetStageState(MOSS_NONREADY);
	SetLaunchedGame(false);

	SetEnterBattle(false);
	SetBattleTimeReward(false);
}


void MMatchObject::OnInitRound()
{
	SetAlive(true);
	SetKillCount(0);
	SetDeathCount(0);
	ResetCustomItemUseCount();

	m_GameInfo.bJoinedGame = true;
	m_nDeadTime = 0;
}

void MMatchObject::SetChannelListTransfer(const bool bVal, const MCHANNEL_TYPE nChannelType)
{ 
	if ((nChannelType < 0) || (nChannelType >= MCHANNEL_TYPE_MAX)) 
	{
		//_ASSERT(0);
		return;
	}

	m_ChannelInfo.bChannelListTransfer = bVal; 
	m_ChannelInfo.nChannelListType = nChannelType;
	UpdateChannelListChecksum(0); 
}

bool MMatchObject::CheckEnableAction(MMO_ACTION nAction)
{
	switch (nAction)
	{
	case MMOA_STAGE_FOLLOW:		// 따라가기가 가능한 상태인지 여부
		{
			if (GetPlace() != MMP_LOBBY) return false;
			if (IsLadderChallenging()) return false;
			if (IsChallengeDuelTournament()) return false;

			return true;
		}
		break;
	default:
		{
			break;
		}
	}

	return true;
}

void MMatchObject::CheckNewbie(int nCharMaxLevel)
{
#define NEWBIE_LEVEL_CUTLINE		20		// 가지고 있는 캐릭터들의 최고레벨이 21레벨이상이면 뉴비가 아니다.

	if (nCharMaxLevel > NEWBIE_LEVEL_CUTLINE) m_bNewbie = false;
	else m_bNewbie = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MMatchCharInfo::EquipFromItemList()
{
	m_EquipedItem.Clear();
	
	for (MMatchItemMap::iterator itor = m_ItemList.begin(); itor != m_ItemList.end(); ++itor)
	{
		MMatchItem* pItem = (*itor).second;
		pItem->SetEquiped(false);
	}

	for (int i = 0; i < MMCIP_END; i++)
	{
		if (m_nEquipedItemCIID[i] == 0) continue;
		for (MMatchItemMap::iterator itor = m_ItemList.begin(); itor != m_ItemList.end(); ++itor)
		{
			MMatchItem* pItem = (*itor).second;
			if ( m_nEquipedItemCIID[i] == pItem->GetCIID() )
			{
				if( m_EquipedItem.SetItem(MMatchCharItemParts(i), itor->first, pItem) ) {
					break;
				} else {
					return false;
				}				
			}
		}
	}

	return true;
}

void MMatchCharInfo::ClearItems()
{
	m_EquipedItem.Clear();
	m_ItemList.Clear();
	m_QuestItemList.Clear();
}

void MMatchCharInfo::Clear()
{
	m_nCID					= 0;
	m_nCharNum				= 0;
	m_nLevel				= 0;
	m_nSex					= MMS_MALE;
	m_nFace					= 0;
	m_nHair					= 0;
	m_nXP					= 0;
	m_nBP					= 0;
	m_fBonusRate			= DEFAULT_CHARINFO_BONUSRATE;
	m_nPrize				= DEFAULT_CHARINFO_PRIZE;
	m_nHP					= 0;
	m_nAP					= 0;
	m_nMaxWeight			= DEFAULT_CHARINFO_MAXWEIGHT;
	m_nSafeFalls			= DEFAULT_CHARINFO_SAFEFALLS;
	m_nFR					= 0;
	m_nCR					= 0;
	m_nER					= 0;
	m_nWR					= 0;
	m_nTotalPlayTimeSec		= 0;
	m_nConnTime				= 0;
	m_nBattleStartTime		= 0;
	m_nBattleStartXP		= 0;
	m_nTotalKillCount		= 0;
	m_nTotalDeathCount		= 0;
	m_nConnKillCount		= 0;
	m_nConnDeathCount		= 0;

	memset(m_szName, 0, MATCHOBJECT_NAME_LENGTH);
	memset(m_nEquipedItemCIID, 0, sizeof(m_nEquipedItemCIID));
	
	m_ClanInfo.Clear();
	
	ClearItems();

	m_IsSendMyItemListByRequestClient = false;
}

void MMatchCharInfo::GetTotalWeight(int* poutWeight, int* poutMaxWeight)
{
	int nWeight, nMaxWeight;

	m_EquipedItem.GetTotalWeight(&nWeight, &nMaxWeight);
	nMaxWeight = nMaxWeight + m_nMaxWeight;

	*poutWeight = nWeight;
	*poutMaxWeight = nMaxWeight;
}


bool IsEquipableItem(unsigned long int nItemID, int nPlayerLevel, MMatchSex nPlayerSex)
{
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc == NULL) return false;

	// 성별 제한 조건
	if (pItemDesc->m_nResSex.Ref() != -1)
	{
		if (pItemDesc->m_nResSex.Ref() != int(nPlayerSex)) return false;
	}

	if (MGetServerConfig()->GetServerMode() != MSM_EVENT) {	// EVENT때 레벨제한 없이 장착한다
		// 레벨 제한 조건
		if (pItemDesc->m_nResLevel.Ref() > nPlayerLevel) return false;
	}

	return true;
}



void MMatchObject::SetFriendInfo(MMatchFriendInfo* pFriendInfo)
{
	m_bDBFriendListRequested = true;
	m_pFriendInfo = pFriendInfo;
}


void MMatchObject::SetCharInfo(MMatchCharInfo* pCharInfo)
{ 
	m_pCharInfo = pCharInfo; 
#ifdef _QUEST_ITEM
	if( MSM_QUEST == MGetServerConfig()->GetServerMode() ) 
	{
		m_pCharInfo->m_DBQuestCachingData.SetCharObject( this );
	}
#endif
}

void MMatchObject::SetDuelTournamentCharInfo(MMatchObjectDuelTournamentCharInfo *pDTCharInfo)
{
	// 이미 있는 정보면 어쩌죠? -.,-;
	// 그럴 일은 없겠지만.. 지우고 다시 할당하자!
	if( m_pDuelTournamentCharInfo ) {
		delete m_pDuelTournamentCharInfo;
		m_pDuelTournamentCharInfo = NULL;
	}

	m_pDuelTournamentCharInfo = pDTCharInfo;
}

void MMatchObject::OnDead()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	m_nDeadTime = pServer->GetTickTime();
	SetAlive(false);
	DeathCount();
}

void MMatchObject::OnKill()
{
	KillCount();
}

bool MMatchObject::IsEnabledRespawnDeathTime(unsigned int nNowTime)
{
	// 캐릭터가 죽은후 6.5초안에 다시 죽었다는 메세지가 넘어오면 처리안해준다.
	// 버그발생, 죽은후 6.5초안에 스테이지 나간 후 다시 배틀에들어와 죽는다면 true 리턴한다

	MMatchBuffSummary* pBuffSummary = GetCharBuff()->GetBuffSummary();
	int nDelayAfterDying = pBuffSummary->GetRespawnTime(RESPAWN_DELAYTIME_AFTER_DYING);

	if ((nNowTime - m_nDeadTime) > (nDelayAfterDying - 500)) return true;
	return false;
}

void MMatchObject::UpdateTickLastPacketRecved()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	m_nTickLastPacketRecved = pServer->GetTickTime();
}

void MMatchObject::UpdateLastHShieldMsgRecved()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	m_nLastHShieldMsgRecved = pServer->GetTickTime();
	SetHShieldMsgRecved(true);		// 이 클라이언트는 최신 ReqMsg에 대한 응답을 보냈다.
}

void MMatchObject::DisconnectHacker( MMatchHackingType eType )
{
	GetDisconnStatusInfo().SetStatus( MMDS_DISCONN_WAIT );

	const PUNISH_TABLE_ITEM& punish = MPunishTable::GetPunish(eType);

	GetDisconnStatusInfo().SetMsgID( punish.dwMessageID ); 
	GetDisconnStatusInfo().SetHackingType( eType ); 
	GetDisconnStatusInfo().SetComment( punish.szComment );
	GetDisconnStatusInfo().SetEndDate( MGetStrLocalTime(punish.nDay, punish.nHour, punish.nMin) ); 
	GetDisconnStatusInfo().SetBlockLevel( punish.eLevel );

	if( NULL != GetAccountInfo() ) {
		GetAccountInfo()->m_HackingType = eType;
	}
}

/*
void MMatchObject::SetBadUserDisconnectWaitInfo( const MMatchDisconnectStatus DisStatus )
{
	GetDisconnStatusInfo().SetStatus( DisStatus );

	GetDisconnStatusInfo().SetMsgID( 130001 ); 
	GetDisconnStatusInfo().SetHackingType( MMHT_BADUSER ); // 이 기능은 X-Trap에서 사용함.
	GetDisconnStatusInfo().SetComment( "irregularity player" );
	GetDisconnStatusInfo().SetEndDate( MGetStrLocalTime(0, 0, 0, 0, 10) ); // 10분후 접속 가능.
	GetDisconnStatusInfo().SetBlockLevel( MMBL_LOGONLY );

	if( NULL != GetAccountInfo() )
		GetAccountInfo()->m_HackingType = MMHT_BADUSER;
}


void MMatchObject::SetXTrapHackerDisconnectWaitInfo( const MMatchDisconnectStatus DisStatus )
{
	GetDisconnStatusInfo().SetStatus( DisStatus );

	GetDisconnStatusInfo().SetMsgID( 130001 ); 
	GetDisconnStatusInfo().SetHackingType( MMHT_XTRAP_HACKER ); // 이 기능은 X-Trap에서 사용함.
	GetDisconnStatusInfo().SetComment( "x-trap hacking detected." );
	GetDisconnStatusInfo().SetEndDate( MGetStrLocalTime(0, 0, 0, 1, 0) ); // 1시간 후에 접속 가능.
	GetDisconnStatusInfo().SetBlockLevel( MMBL_LOGONLY );

	if( NULL != GetAccountInfo() )
		GetAccountInfo()->m_HackingType = MMHT_XTRAP_HACKER;
}


void MMatchObject::SetHShieldHackerDisconnectWaitInfo( const MMatchDisconnectStatus DisStatus )
{
	GetDisconnStatusInfo().SetStatus( DisStatus );

	GetDisconnStatusInfo().SetMsgID( 130001 ); 
	GetDisconnStatusInfo().SetHackingType( MMHT_HSHIELD_HACKER ); // 이 기능은 HShield에서 사용.
	GetDisconnStatusInfo().SetComment( "hackshield hacking detected." );
	GetDisconnStatusInfo().SetEndDate( MGetStrLocalTime(0, 0, 0, 1, 0) ); // 1시간 후에 접속 가능.
	GetDisconnStatusInfo().SetBlockLevel( MMBL_LOGONLY );

	if( NULL != GetAccountInfo() )
		GetAccountInfo()->m_HackingType = MMHT_HSHIELD_HACKER;
}


void MMatchObject::SetBadFileCRCDisconnectWaitInfo( const MMatchDisconnectStatus DisStatus )
{
	GetDisconnStatusInfo().SetStatus( DisStatus );

	GetDisconnStatusInfo().SetMsgID( 130004 ); 
	GetDisconnStatusInfo().SetHackingType( MMHT_BADFILECRC ); // 이 기능은 X-Trap에서 사용함.
	GetDisconnStatusInfo().SetComment( "bad filecrc." );
	GetDisconnStatusInfo().SetEndDate( MGetStrLocalTime(0, 0, 0, 0, 0) ); // 1시간 후에 접속 가능.
	GetDisconnStatusInfo().SetBlockLevel( MMBL_LOGONLY );

	if( NULL != GetAccountInfo() )
		GetAccountInfo()->m_HackingType = MMHT_BADFILECRC;
}


void MMatchObject::SetGameguardHackerDisconnectWaitInfo( const MMatchDisconnectStatus DisStatus )
{
	GetDisconnStatusInfo().SetStatus( DisStatus );

	GetDisconnStatusInfo().SetMsgID( 130001 ); 
	GetDisconnStatusInfo().SetHackingType( MMHT_GAMEGUARD_HACKER ); 
	GetDisconnStatusInfo().SetComment( "gameguard hacker" );
	GetDisconnStatusInfo().SetEndDate( MGetStrLocalTime(0, 0, 0, 0, 0) ); 
	GetDisconnStatusInfo().SetBlockLevel( MMBL_LOGONLY );

	if( NULL != GetAccountInfo() )
		GetAccountInfo()->m_HackingType = MMHT_GAMEGUARD_HACKER;
}


void MMatchObject::SetDllInjectionDisconnectWaitInfo( const MMatchDisconnectStatus DisStatus )
{
	GetDisconnStatusInfo().SetStatus( DisStatus );

	GetDisconnStatusInfo().SetMsgID( 130001 ); 
	GetDisconnStatusInfo().SetHackingType( MMHT_GIVE_ONESELF_UP_DLLINJECTION ); 
	GetDisconnStatusInfo().SetComment( "dll injectoin" );
	GetDisconnStatusInfo().SetEndDate( MGetStrLocalTime(0, 0, 0, 0, 0) ); 
	GetDisconnStatusInfo().SetBlockLevel( MMBL_LOGONLY );

	if( NULL != GetAccountInfo() )
	{
		GetAccountInfo()->m_HackingType = MMHT_GIVE_ONESELF_UP_DLLINJECTION;
	}
}


void MMatchObject::SetInvalidStageSettingDisconnectWaitInfo( const MMatchDisconnectStatus DisStatus )
{
	GetDisconnStatusInfo().SetStatus( DisStatus );

	GetDisconnStatusInfo().SetMsgID( 130001 ); 
	GetDisconnStatusInfo().SetHackingType( MMHT_INVALIDSTAGESETTING ); 
	GetDisconnStatusInfo().SetComment( "invalid stage setting." );
	GetDisconnStatusInfo().SetEndDate( MGetStrLocalTime(0, 0, 0, 0, 0) ); 
	GetDisconnStatusInfo().SetBlockLevel( MMBL_LOGONLY );

	if( NULL != GetAccountInfo() )
	{
		GetAccountInfo()->m_HackingType = MMHT_INVALIDSTAGESETTING;
	}
}
*/

void MMatchObject::ResetCustomItemUseCount()
{
	for(int i = MMCIP_CUSTOM1; i < MMCIP_CUSTOM2 + 1; i++)
	{
		MMatchItem* pCustomItem = GetCharInfo()->m_EquipedItem.GetItem( MMatchCharItemParts(i) );
		if( pCustomItem ) 
		{
			if( pCustomItem->GetDesc()->IsSpendableItem() == false )
			{
				pCustomItem->ResetUseCountOfNonDestroyItem();
			}
		}
	}
}


const bool MMatchObject::IsHaveCustomItem()
{
	// 단순히 커스텀 아이템을 착용하고 있는지 확인
	if( NULL != GetCharInfo()->m_EquipedItem.GetItem(MMCIP_CUSTOM1) ) return true;
	if( NULL != GetCharInfo()->m_EquipedItem.GetItem(MMCIP_CUSTOM2) ) return true;

	return false;
}

const bool MMatchObject::IncreaseCustomItemUseCount()
{
	/*
	처음 착용한 곳을 검사해서 더이상 사용 할 수 없다면 다음 착용한 곳을 검사한다.
	만약 다음 착용한 부분마저 수량이 남지 않았다면 작업은 실패한다.
	*/

	for(int i = MMCIP_CUSTOM1; i < MMCIP_CUSTOM2 + 1; i++)
	{
		MMatchItem* pCustomItem = GetCharInfo()->m_EquipedItem.GetItem( MMatchCharItemParts(i) );
		if( pCustomItem ) 
		{
			if( pCustomItem->GetDesc()->IsSpendableItem() ) 
			{
				return true;
			}
			else if( pCustomItem->GetDesc()->m_nMagazine.Ref() > pCustomItem->GetUseCountOfNonDestroyItem() )
			{
				pCustomItem->IncreaseUseCountOfNonDestroyItem();
				return true;
			}
		}
	}

#ifdef _DEBUG
	static int nTestCnt = 0;
#endif

	return false;
}

bool MMatchObject::IsEquipCustomItem(int nItemId)
{
	MMatchItem* pItem;
	for (int i=MMCIP_CUSTOM1; i<=MMCIP_CUSTOM2; ++i)
	{
		pItem = GetCharInfo()->m_EquipedItem.GetItem((MMatchCharItemParts)i);
		if (!pItem)
			continue;
		if (pItem->GetDescID() == nItemId)
			return true;
	}
	return false;
}

