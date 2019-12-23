//////////////////////////////////////////////////////////////////////////////////////////////
// 2009. 9. 23 - Added By Hong KiJu
// Notice By Hong KiJu - 홍기주에게 문의하세용

#include "stdafx.h"
#include "MBlobArray.h"

#include "MAsyncDBJob_DuelTournament.h"

#include "MMatchConfig.h"
#include "MMatchServer.h"
#include "MMatchDuelTournamentMgr.h"

void MMatchServer::ResponseDuelTournamentJoinChallenge(MUID &uidPlayer, MDUELTOURNAMENTTYPE nType)
{
	MMatchObject *pDTObj = GetPlayerByCommUID(uidPlayer);
	if(IsEnabledObject(pDTObj) == false) return;

	MMatchChannel *pChannel = FindChannel(pDTObj->GetChannelUID());
	if( pChannel == NULL ) {
		LOG(LOG_PROG, "MMatchServer::OnRequestDuelTournamentChallenge - Wrong Channel(ChannelUID - %d%d)",
			pDTObj->GetChannelUID().High, pDTObj->GetChannelUID().Low);
		PostCmdDuelTournamentChallenge(uidPlayer, MERR_DT_WRONG_CHANNEL);
		return;
	}

	if( pDTObj->IsChallengeDuelTournament() == true ) {
		LOG(LOG_PROG, "MMatchServer::OnRequestDuelTournamentChallenge - Already Join Challenge(PlayerUID - %d%d)",
			pDTObj->GetUID().High, pDTObj->GetUID().Low);
		PostCmdDuelTournamentChallenge(uidPlayer, MERR_DT_ALREADY_JOIN);
		return;
	}

	if( GetDTMgr()->AddPlayer(nType, uidPlayer) == false ) {
		LOG(LOG_PROG, "MMatchServer::OnRequestDuelTournamentChallenge - Can't Join Challenge the Game(PlayerUID - %d%d)",
			pDTObj->GetUID().High, pDTObj->GetUID().Low);
		PostCmdDuelTournamentChallenge(uidPlayer, MERR_DT_CANNOT_CHALLENGE);
		return;
	}

	pDTObj->SetChallengeDuelTournament(true);
	PostCmdDuelTournamentChallenge(uidPlayer, MOK);
}

void MMatchServer::ResponseDuelTournamentCancelChallenge(MUID &uidPlayer, MDUELTOURNAMENTTYPE nType)
{
	MMatchObject *pDTObj = GetPlayerByCommUID(uidPlayer);
	if(IsEnabledObject(pDTObj) == false) return;

	MMatchChannel *pChannel = FindChannel(pDTObj->GetChannelUID());
	if( pChannel == NULL ) {
		LOG(LOG_PROG, "MMatchServer::OnRequestDuelTournamentCancelChallenge - Wrong Channel(ChannelUID - %d%d)",
			pDTObj->GetChannelUID().High, pDTObj->GetChannelUID().Low);
		return;
	}

	if( pDTObj->IsChallengeDuelTournament() == false ) {
		LOG(LOG_PROG, "MMatchServer::OnRequestDuelTournamentChallenge - Not Join Challenge(PlayerUID - %d%d)",
			pDTObj->GetUID().High, pDTObj->GetUID().Low);
		return;
	}

	if( GetDTMgr()->RemovePlayer(nType, uidPlayer) == false ) {
		LOG(LOG_PROG, "MMatchServer::OnRequestDuelTournamentCancelChallenge - Can't Cancel Challenge the Game(Type - %d, PlayerUID - %d%d)",
			(int)nType, pDTObj->GetUID().High, pDTObj->GetUID().Low);
		return;
	}

	pDTObj->SetChallengeDuelTournament(false);
}

void MMatchServer::ResponseDuelTournamentCharSideRanking(MUID &uidPlayer)
{
	MMatchObject *pDTObj = GetPlayerByCommUID(uidPlayer);
	if(IsEnabledObject(pDTObj) == false) return;

	MMatchObjectDuelTournamentCharInfo *pInfo = pDTObj->GetDuelTournamentCharInfo();
	if( pInfo == NULL ) return;

	if( pInfo->GetSideRankingList()->size() == 0 ){
		OnAsyncRequest_GetDuelTournamentSideRankingInfo(pDTObj->GetUID(), pDTObj->GetCharInfo()->m_nCID);
	} else {
		if( GetDTMgr()->IsSameTimeStamp(pDTObj->GetDuelTournamentCharInfo()->GetTimeStamp()) ){
			PostCmdDuelTournamentCharSideRankingInfo(pDTObj->GetUID(), pInfo->GetSideRankingList());
		} else {
			OnAsyncRequest_GetDuelTournamentSideRankingInfo(pDTObj->GetUID(), pDTObj->GetCharInfo()->m_nCID);
		}
	}
}

void MMatchServer::ResponseDuelTournamentCharStatusInfo(MUID &uidPlayer, MCommand *pCommand)
{
	MMatchObject *pDTObj = GetPlayerByCommUID(uidPlayer);
	if(IsEnabledObject(pDTObj) == false) return;

	MMatchStage *pStage = FindStage(pDTObj->GetStageUID());
	if( pStage == NULL ) return;

	pStage->OnCommand(pCommand);
}

void MMatchServer::SendDuelTournamentPreviousCharInfoToPlayer(MUID uidPlayer)
{
	MMatchObject *pDTObj = GetPlayerByCommUID(uidPlayer);
	if(IsEnabledObject(pDTObj) == false) return;

	OnAsyncRequest_GetDuelTournamentPreviousCharacterInfo(uidPlayer, pDTObj->GetCharInfo()->m_nCID);
}

void MMatchServer::SendDuelTournamentCharInfoToPlayer(MUID uidPlayer)
{
	MMatchObject *pDTObj = GetPlayerByCommUID(uidPlayer);
	if(IsEnabledObject(pDTObj) == false) return;

	if( pDTObj->GetDuelTournamentCharInfo() == NULL ) {
		if( pDTObj->GetCharInfo() == NULL ) return;
		OnAsyncRequest_GetDuelTournamentCharacterInfo(uidPlayer, pDTObj->GetCharInfo()->m_nCID);
		return;
	} else {
		if( GetDTMgr()->IsSameTimeStamp(pDTObj->GetDuelTournamentCharInfo()->GetTimeStamp()) ){
			PostCmdDuelTournamentCharInfo(uidPlayer, pDTObj->GetDuelTournamentCharInfo());
		} else {
			LOG(LOG_PROG, "MMatchServer::SendDuelTournamentCharInfoToPlayer - PlayerUID(%d%d) - TimeStamp Changed! %s -> %s",
				pDTObj->GetUID().High, pDTObj->GetUID().Low, pDTObj->GetDuelTournamentCharInfo()->GetTimeStamp(), GetDTMgr()->GetTimeStamp());
			
			OnAsyncRequest_GetDuelTournamentCharacterInfo(uidPlayer, pDTObj->GetCharInfo()->m_nCID);
		}
	}	
}

bool MMatchServer::DuelTournamentJoin(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return false;

	if (pObj->GetStageUID() != MUID(0,0))
		StageLeave(pObj->GetUID());//, pObj->GetStageUID());

	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	pObj->OnStageJoin();

	// Join
	pStage->AddObject(uidPlayer, pObj);
	pObj->SetStageUID(uidStage);
	pObj->SetStageState(MOSS_READY);
	pObj->SetChallengeDuelTournament(false);
	pStage->PlayerState(uidPlayer, MOSS_READY);

	return true;
}

void MMatchServer::SendDuelTournamentServiceTimeClose(const MUID& uidPlayer)
{
	if ( MGetServerConfig()->IsEnabledDuelTournament() == false ) return;

	// 듀얼토너먼트 참가신청 취소
	MMatchObject *pDTObj = GetPlayerByCommUID(uidPlayer);
	if(IsEnabledObject(pDTObj) == false) return;
	pDTObj->SetChallengeDuelTournament(false);
 
	MCommand *pCmd = CreateCommand(MC_MATCH_DUELTOURNAMENT_NOT_SERVICE_TIME, uidPlayer);
	pCmd->AddParameter(new MCommandParameterInt(MGetServerConfig()->GetDuelTournamentServiceStartTime()));
	pCmd->AddParameter(new MCommandParameterInt(MGetServerConfig()->GetDuelTournamentServiceEndTime()));
	Post(pCmd);
}

void MMatchServer::LaunchDuelTournamentMatch(MDUELTOURNAMENTTYPE nType, MDuelTournamentPickedGroup* pPickedGroup, MDUELTOURNAMENTMATCHMAKINGFACTOR matchFactor)
{
	if ( MGetServerConfig()->IsEnabledDuelTournament() == false ) return;

	// 선수들이 뛸 Stage 생성!
	MUID uidStage = MUID(0,0);
	if (StageAdd(NULL, "DuelTournament_Stage", true, "", &uidStage, true) == false) {
		LOG(LOG_PROG, "MMatchServer::LaunchDuelTournamentMatch - Can't Add Stage - StageUID(%d%d)", uidStage.High, uidStage.Low);
		RouteCmdDuelTournamentCancelMatch(pPickedGroup, MERR_DT_CANNOT_MAKE_STAGE);
		return;
	}

	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) {
		LOG(LOG_PROG, "MMatchServer::LaunchDuelTournamentMatch - Can't Find Stage - StageUID(%d%d)", uidStage.High, uidStage.Low);
		RouteCmdDuelTournamentCancelMatch(pPickedGroup, MERR_DT_CANNOT_FIND_STAGE);
		return;
	}

	//////////////////////////////////////////////////////////////////////////////
	// 선수들이 뛸 Stage 설정!

	MMATCH_GAMETYPE nGameType = MMATCH_GAMETYPE_DUELTOURNAMENT;
	
	pStage->ChangeRule(nGameType);	
	pStage->SetStageType(MST_NORMAL);
	pStage->SetDuelTournamentMatchList(nType, pPickedGroup);

	int nRandomMapIndex = pStage->GetDuelTournamentRandomMapIndex();
	int nMaxRound = pStage->GetDuelTournamentTotalRound();
	int nLimitTimeForRound = 1;

	MMatchStageSetting* pSetting = pStage->GetStageSetting();
	pSetting->SetMasterUID(MUID(0,0));
	pSetting->SetMapIndex(nRandomMapIndex);
	pSetting->SetGameType(nGameType);
	pSetting->SetLimitTime(nLimitTimeForRound);	
	pSetting->SetRoundMax(nMaxRound);

#ifdef _DUELTOURNAMENT_LOG_ENABLE_	
	LOG(LOG_PROG, "MMatchServer::LaunchDuelTournamentMatch - MapName=%s(id=%d), MaxRound=%d"
		, MGetMapDescMgr()->GetMapName(nRandomMapIndex), nRandomMapIndex, nMaxRound);
#endif

	// 디비에 로그를 남긴다.
	if ( (MGetMapDescMgr()->MIsCorrectMap(nRandomMapIndex)) && (MGetGameTypeMgr()->IsCorrectGameType(MMATCH_GAMETYPE_DUELTOURNAMENT)) ) {

		// 선수들 입장!
		for (MDuelTournamentPickedGroup::iterator i=pPickedGroup->begin(); i!= pPickedGroup->end(); i++)
		{
			MUID uidPlayer = (*i);
			if( DuelTournamentJoin(uidPlayer, uidStage) == false ){
				LOG(LOG_PROG, "MMatchServer::LaunchDuelTournamentMatch - Can't Join DT Stage - PlayerUID(%d%d)", uidPlayer.High, uidPlayer.Low);

				// 이거 다 취소시켜야되나? -_ㅠ 고민해보자...
				// Added By 홍기주
				return;
			}
		}

		RouteCmdDuelTournamentPrepareMatch(nType, uidStage, pPickedGroup);
		RouteCmdDuelTournamentStageSetting(uidStage);

		if (pStage->StartGame(MGetServerConfig()->IsUseResourceCRC32CacheCheck()) == true) {		// 게임시작
			ReserveAgent(pStage);

			MMatchObjectCacheBuilder CacheBuilder;
			CacheBuilder.Reset();

			for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
				MUID uidObj = (MUID)(*i).first;
				MMatchObject* pScanObj = (MMatchObject*)GetObject(uidObj);
				if (pScanObj) CacheBuilder.AddObject(pScanObj);
			}

			MCommand* pCmdCacheAdd = CacheBuilder.GetResultCmd(MATCHCACHEMODE_UPDATE, this);
			RouteToStage(pStage->GetUID(), pCmdCacheAdd);

			int nOutNumber;
			char szOutTimeStamp[DUELTOURNAMENT_TIMESTAMP_MAX_LENGTH + 1] = {0, };

			if( OnSyncRequest_InsertDuelTournamentGameLog(nType, matchFactor, pPickedGroup, &nOutNumber, szOutTimeStamp) ){				
				pStage->SetDuelTournamentMatchNumber(nOutNumber);
				pStage->SetDuelTournamentMatchTimeStamp(szOutTimeStamp);

				RouteCmdDuelTournamentLaunchMatch(uidStage);

				SaveGameLog(uidStage);
			} else{
				// DB 적재 실패할 경우.. 어떻게 해야되는겁니까? -_ㅠ
				// NHN에서는 가끔 DB접속이 끊기기도 하던데... 가끔 DB 쿼리 실패도 있던데... 
				//
				// 이거 다 취소시켜야되나? -_ㅠ 고민해보자...
				// Added By 홍기주
				//RouteCmdDuelTournamentCancelMatch(pPickedGroup, MERR_DT_CANNOT_ACCESS_DB);
				LOG(LOG_PROG, "MMatchServer::LaunchDuelTournamentMatch - Insert Data to Database Fail- Can't Start Stage");
				return;
			}
		} else {
			LOG(LOG_PROG, "MMatchServer::LaunchDuelTournamentMatch - Can't Start Stage");
			RouteCmdDuelTournamentCancelMatch(pPickedGroup, MERR_DT_CANNOT_START_STAGE);			
		}
	}
	else{
		//_ASSERT(0);//channelrule.xml 에 듀얼토너먼트용 맵을 추가
		LOG(LOG_PROG, "MMatchServer::LaunchDuelTournamentMatch - Wrong Stage Setting");
		RouteCmdDuelTournamentCancelMatch(pPickedGroup, MERR_DT_CANNOT_WRONG_STAGE_SETTING);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sync Request
bool MMatchServer::OnSyncRequest_InsertDuelTournamentGameLog(MDUELTOURNAMENTTYPE nDTType, int nMatchFactor, MDuelTournamentPickedGroup *pPickedGroup, int *nOutNumber, char *szOutTimeStamp)
{
	int nPlayerCID[8] = {0, };

	int nIndex = 0;
	for (MDuelTournamentPickedGroup::iterator i=pPickedGroup->begin(); i!= pPickedGroup->end(); i++)
	{
		MUID uidPlayer = (*i);
		MMatchObject* pObj = GetObject(uidPlayer);
		if (pObj == NULL) return false;

		nPlayerCID[nIndex++] = pObj->GetCharInfo()->m_nCID;
	}

	return m_MatchDBMgr.InsertDuelTournamentGameLog(nDTType, nMatchFactor, nPlayerCID[0], nPlayerCID[1], nPlayerCID[2], nPlayerCID[3], 
		nPlayerCID[4], nPlayerCID[5], nPlayerCID[6], nPlayerCID[7], nOutNumber, szOutTimeStamp);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Async Request
bool MMatchServer::OnAsyncRequest_GetDuelTournamentTimeStamp()
{
	MAsyncDBJob_GetDuelTournamentTimeStamp *pAsyncDbJob = new MAsyncDBJob_GetDuelTournamentTimeStamp;
	if( 0 == pAsyncDbJob ) return false;
	MMatchServer::GetInstance()->PostAsyncJob( pAsyncDbJob );
	return true;
}

bool MMatchServer::OnAsyncRequest_GetDuelTournamentCharacterInfo(MUID uidPlayer, DWORD dwCID)
{
	MAsyncDBJob_GetDuelTournamentCharInfo *pAsyncDbJob = new MAsyncDBJob_GetDuelTournamentCharInfo;
	if( 0 == pAsyncDbJob ) return false;

	pAsyncDbJob->Input(uidPlayer, dwCID);
	MMatchServer::GetInstance()->PostAsyncJob( pAsyncDbJob );
	return true;
}

bool MMatchServer::OnAsyncRequest_GetDuelTournamentPreviousCharacterInfo(MUID uidPlayer, DWORD dwCID)
{
	MAsyncDBJob_GetDuelTournamentPreviousCharInfo *pAsyncDbJob = new MAsyncDBJob_GetDuelTournamentPreviousCharInfo;
	if( 0 == pAsyncDbJob ) return false;

	pAsyncDbJob->Input(uidPlayer, dwCID);
	MMatchServer::GetInstance()->PostAsyncJob( pAsyncDbJob );
	return true;
}

bool MMatchServer::OnAsyncRequest_UpdateDuelTournamentCharacterInfo(MUID uidPlayer, char *szTimeStamp)
{
	MMatchObject *pObj = GetObject(uidPlayer);
	if( pObj == NULL ) return false;

	MAsyncDBJob_UpdateDuelTournamentCharInfo *pAsyncDBJob = new MAsyncDBJob_UpdateDuelTournamentCharInfo;
	if( 0 == pAsyncDBJob ) return false;

	pAsyncDBJob->Input(pObj->GetCharInfo()->m_nCID, szTimeStamp, pObj->GetDuelTournamentCharInfo());
	MMatchServer::GetInstance()->PostAsyncJob( pAsyncDBJob );
	return true;
}

bool MMatchServer::OnAsyncRequest_GetDuelTournamentSideRankingInfo(MUID uidPlayer, DWORD dwCID)
{
	MAsyncDBJob_GetDuelTournamentSideRankingInfo *pAsyncDbJob = new MAsyncDBJob_GetDuelTournamentSideRankingInfo;
	if( 0 == pAsyncDbJob ) return false;

	pAsyncDbJob->Input(uidPlayer, dwCID);
	MMatchServer::GetInstance()->PostAsyncJob( pAsyncDbJob );
	return true;
}

bool MMatchServer::OnAsyncRequest_GetDuelTournamentGroupRankingInfo()
{
	MAsyncDBJob_GetDuelTournamentGroupRankingInfo *pAsyncDbJob = new MAsyncDBJob_GetDuelTournamentGroupRankingInfo;
	if( 0 == pAsyncDbJob ) return false;

	pAsyncDbJob->Input();
	MMatchServer::GetInstance()->PostAsyncJob( pAsyncDbJob );
	return true;
}

bool MMatchServer::OnAsyncRequest_UpdateDuelTournamentGameLog(char* szTimeStamp, int nGameNumber, MUID uidChampion)
{
	MAsyncDBJob_UpdateDuelTournamentGameLog *pAsyncDbJob = new MAsyncDBJob_UpdateDuelTournamentGameLog;
	if( 0 == pAsyncDbJob ) return false;

	MMatchObject *pObj = GetObject(uidChampion);
	if( pObj != NULL )	{ pAsyncDbJob->Input(szTimeStamp, nGameNumber, pObj->GetCharInfo()->m_nCID);}
	else				{ pAsyncDbJob->Input(szTimeStamp, nGameNumber, -1); }

	MMatchServer::GetInstance()->PostAsyncJob( pAsyncDbJob );
	return true;
}

bool MMatchServer::OnAsyncRequest_InsertDuelTournamentGameLogDetail(int nLogID, char* szTimeStamp, MDUELTOURNAMENTROUNDSTATE nDTRoundState, int nWinnerCID, int nLoserCID, int nGainTP, int nLoseTp, int nPlayTime) 
{
	MAsyncDBJob_InsertDuelTournamentGameLogDetail *pAsyncDbJob = new MAsyncDBJob_InsertDuelTournamentGameLogDetail;
	if( 0 == pAsyncDbJob ) return false;

	pAsyncDbJob->Input(nLogID, szTimeStamp, nDTRoundState, nWinnerCID, nLoserCID, nGainTP, nLoseTp, nPlayTime);
	MMatchServer::GetInstance()->PostAsyncJob( pAsyncDbJob );

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Async Response
void MMatchServer::OnAsyncResponse_GetDuelTournamentTimeStamp(MAsyncJob *pJobResult)
{
	MAsyncDBJob_GetDuelTournamentTimeStamp* pJob = (MAsyncDBJob_GetDuelTournamentTimeStamp*)pJobResult;

	if( MASYNC_RESULT_SUCCEED != pJob->GetResult() ) {
		mlog("MMatchServer::OnAsyncResponse_GetDuelTournamentTimeStamp - 실패\n");
		return;
	}

	if( GetDTMgr()->IsSameTimeStamp(pJob->GetTimeStamp()) == false ) {
		GetDTMgr()->SetTimeStamp(pJob->GetTimeStamp());
		GetDTMgr()->SetTimeStampChanged(true);

		LOG(LOG_PROG, "Time Stamp Changed! - %s", pJob->GetTimeStamp());
	}	
}

void MMatchServer::OnAsyncResponse_GetDuelTournamentCharacterInfo(MAsyncJob *pJobResult)
{
	MAsyncDBJob_GetDuelTournamentCharInfo* pJob = (MAsyncDBJob_GetDuelTournamentCharInfo*)pJobResult;

	if( MASYNC_RESULT_SUCCEED != pJob->GetResult() ) {
		mlog("MMatchServer::OnAsyncResponse_GetDuelTournamentCharacterInfo - 실패\n");
		return;
	}

	MMatchObject* pObj = GetObject(pJob->GetPlayerUID());
	if( pObj == NULL ) return;
	if( pJob->GetDTCharInfo()->IsSettingData() == false ) {
		mlog("MMatchServer::OnAsyncResponse_GetDuelTournamentCharacterInfo - 데이터가 이상합니다.\n");
		return;
	} else if( pJob->GetDTCharInfo()->GetTP() < 0 ) {
		mlog("MMatchServer::OnAsyncResponse_GetDuelTournamentCharacterInfo - 데이터가 이상합니다(2)\n");
		return;
	}
	pObj->SetDuelTournamentCharInfo( new MMatchObjectDuelTournamentCharInfo( pJob->GetDTCharInfo() ) );
	PostCmdDuelTournamentCharInfo(pObj->GetUID(), pObj->GetDuelTournamentCharInfo());
}

void MMatchServer::OnAsyncResponse_GetDuelTournamentPreviousCharacterInfo(MAsyncJob *pJobResult)
{
	MAsyncDBJob_GetDuelTournamentPreviousCharInfo* pJob = (MAsyncDBJob_GetDuelTournamentPreviousCharInfo*)pJobResult;

	if( MASYNC_RESULT_SUCCEED != pJob->GetResult() ) {
		mlog("MMatchServer::OnAsyncResponse_GetDuelTournamentPreviousCharacterInfo - 실패\n");
		return;
	}

	PostCmdDuelTournamentCharInfoPrevious(pJob->GetPlayerUID(), pJob->GetPrevTP(), pJob->GetPrevWins(), pJob->GetPrevLoses(), pJob->GetPrevRanking(), pJob->GetPrevFinalWins());
}

void MMatchServer::OnAsyncResponse_GetDuelTournamentSideRanking(MAsyncJob *pJobResult)
{
	MAsyncDBJob_GetDuelTournamentSideRankingInfo* pJob = (MAsyncDBJob_GetDuelTournamentSideRankingInfo*)pJobResult;

	if( MASYNC_RESULT_SUCCEED != pJob->GetResult() ) {
		mlog("MMatchServer::OnAsyncResponse_GetDuelTournamentSideRanking - 실패\n");
		return;
	}

	MMatchObject* pObj = GetObject(pJob->GetPlayerUID());
	if( pObj == NULL ) return;


	MMatchObjectDuelTournamentCharInfo *pCharInfo = pObj->GetDuelTournamentCharInfo();
	if( pCharInfo == NULL ) return;

	pCharInfo->RemoveSideRankingAll();
	list<DTRankingInfo*>::iterator iter = pJob->GetSideRankingList()->begin();
	for(; iter != pJob->GetSideRankingList()->end(); ++iter) {
		pCharInfo->AddSideRankingInfo((*iter));
	}

	PostCmdDuelTournamentCharSideRankingInfo(pObj->GetUID(), pCharInfo->GetSideRankingList());
}

void MMatchServer::OnAsyncResponse_GetDuelTournamentGroupRanking(MAsyncJob *pJobResult)
{
	MAsyncDBJob_GetDuelTournamentGroupRankingInfo* pJob = (MAsyncDBJob_GetDuelTournamentGroupRankingInfo*)pJobResult;

	if( MASYNC_RESULT_SUCCEED != pJob->GetResult() ) {
		mlog("MMatchServer::OnAsyncResponse_GetDuelTournamentGroupRanking - 실패\n");
		return;
	}

	GetDTMgr()->AddGroupRanking(pJob->GetGroupRankingList());
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Post Command To Client(or Route to Stage)
void MMatchServer::PostCmdDuelTournamentChallenge(MUID uidPlayer, int nResult)
{
	MCommand *pCmd = CreateCommand(MC_MATCH_DUELTOURNAMENT_RESPONSE_JOINGAME, uidPlayer);
	pCmd->AddParameter(new MCommandParameterInt(nResult));
	Post(pCmd);
}

void MMatchServer::PostCmdDuelTournamentCharInfo(MUID uidPlayer, MMatchObjectDuelTournamentCharInfo *pDTCharInfo)
{
	if( pDTCharInfo == NULL ) return;

	MCommand *pCmd = CreateCommand(MC_MATCH_DUELTOURNAMENT_CHAR_INFO, uidPlayer);
	pCmd->AddParameter(new MCommandParameterInt(pDTCharInfo->GetTP()));
	pCmd->AddParameter(new MCommandParameterInt(pDTCharInfo->GetWins()));
	pCmd->AddParameter(new MCommandParameterInt(pDTCharInfo->GetLoses()));
	pCmd->AddParameter(new MCommandParameterInt(pDTCharInfo->GetRanking()));
	pCmd->AddParameter(new MCommandParameterInt(pDTCharInfo->GetRankingIncrease()));
	pCmd->AddParameter(new MCommandParameterInt(pDTCharInfo->GetFinalWins()));
	pCmd->AddParameter(new MCommandParameterInt(pDTCharInfo->GetLastWeekGrade()));

	Post(pCmd);
}

void MMatchServer::PostCmdDuelTournamentCharInfoPrevious(MUID uidPlayer, int nPrevTP, int nPrevWins, int nPrevLoses, int nPrevRanking, int nPrevFinalWins)
{
	MCommand *pCmd = CreateCommand(MC_MATCH_DUELTOURNAMENT_CHAR_INFO_PREVIOUS, uidPlayer);

	pCmd->AddParameter(new MCommandParameterInt(nPrevTP));
	pCmd->AddParameter(new MCommandParameterInt(nPrevWins));
	pCmd->AddParameter(new MCommandParameterInt(nPrevLoses));
	pCmd->AddParameter(new MCommandParameterInt(nPrevRanking));
	pCmd->AddParameter(new MCommandParameterInt(nPrevFinalWins));

	Post(pCmd);
}

void MMatchServer::PostCmdDuelTournamentCharSideRankingInfo(MUID uidPlayer, list<DTRankingInfo*>* pSideRankingList)
{
	MMatchObject *pDTObj = GetPlayerByCommUID(uidPlayer);
	if(IsEnabledObject(pDTObj) == false) return;

	void* pBlobRanking = MMakeBlobArray(sizeof(DTRankingInfo), (int)pSideRankingList->size() );

	int index = 0;
	list<DTRankingInfo*>::iterator iter;
	for(iter = pSideRankingList->begin(); iter != pSideRankingList->end(); ++iter){
		DTRankingInfo *pInfoDest = reinterpret_cast<DTRankingInfo *>(MGetBlobArrayElement(pBlobRanking, index++));	
		DTRankingInfo *pInfoSrc = (*iter);

		if( pInfoDest == NULL || pInfoSrc == NULL ) { MEraseBlobArray( pBlobRanking ); return; }
		memcpy(pInfoDest, pInfoSrc, sizeof(DTRankingInfo));
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_DUELTOURNAMENT_RESPONSE_SIDERANKING_INFO, uidPlayer);
	if( NULL == pCmd ) {
		MEraseBlobArray( pBlobRanking );
		return;
	}
	pCmd->AddParameter( new MCommandParameterBlob(pBlobRanking, MGetBlobArraySize(pBlobRanking)) );
	MEraseBlobArray( pBlobRanking );

	Post(pCmd);
}

void MMatchServer::PostCmdDuelTournamentCancelMatch(MUID uidPlayer, int nErrorCode)
{
	MCommand *pCmd = CreateCommand(MC_MATCH_DUELTOURNAMENT_CANCEL_MATCH, uidPlayer);
	pCmd->AddParameter(new MCommandParameterInt(nErrorCode));
	Post(pCmd);
}

void MMatchServer::RouteCmdDuelTournamentPrepareMatch(MDUELTOURNAMENTTYPE nType, MUID uidStage, MDuelTournamentPickedGroup *pPickedGroup)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	void* pBlobPlayerInfo = MMakeBlobArray(sizeof(DTPlayerInfo), (int)pPickedGroup->size() );

	int index = 0;
	MDuelTournamentPickedGroup::iterator iter;
	for(iter = pPickedGroup->begin(); iter != pPickedGroup->end(); ++iter){
		DTPlayerInfo *pPlayerInfo = reinterpret_cast<DTPlayerInfo*>(MGetBlobArrayElement(pBlobPlayerInfo, index++));
		MMatchObject* pObj = GetObject(*iter);
		if (pObj == NULL){
			MEraseBlobArray(pBlobPlayerInfo);
			return;
		}

        pPlayerInfo->m_nTP = pObj->GetDuelTournamentCharInfo()->GetTP();
		pPlayerInfo->uidPlayer = pObj->GetUID();
		strcpy(pPlayerInfo->m_szCharName, pObj->GetCharInfo()->m_szName);
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_DUELTOURNAMENT_PREPARE_MATCH, MUID(0, 0));
	if( pCmd == NULL ) {
		MEraseBlobArray(pBlobPlayerInfo);
		return;
	}

	pCmd->AddParameter(new MCmdParamUID(uidStage));
	pCmd->AddParameter(new MCmdParamInt(nType));
	pCmd->AddParameter(new MCommandParameterBlob(pBlobPlayerInfo, MGetBlobArraySize(pBlobPlayerInfo)));
	MEraseBlobArray( pBlobPlayerInfo );

	RouteToStage(uidStage, pCmd);
}

void MMatchServer::RouteCmdDuelTournamentStageSetting(MUID uidStage)
{
	MCommand* pCmd = CreateCmdResponseStageSetting(uidStage);
	RouteToStage(uidStage, pCmd);
}

void MMatchServer::RouteCmdDuelTournamentLaunchMatch(MUID uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	MCommand* pCmd = CreateCommand(MC_MATCH_DUELTOURNAMENT_LAUNCH_MATCH, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidStage));
	pCmd->AddParameter(new MCmdParamStr( const_cast<char*>(pStage->GetMapName()) ));
	RouteToStage(uidStage, pCmd);
}

void MMatchServer::RouteCmdDuelTournamentCancelMatch(MDuelTournamentPickedGroup *pPickedGroup, int nErrorCode)
{
	for (MDuelTournamentPickedGroup::iterator i=pPickedGroup->begin(); i!= pPickedGroup->end(); i++)
	{
		MUID uidPlayer = (*i);
		PostCmdDuelTournamentCancelMatch(uidPlayer, nErrorCode);
	}
}

void MMatchServer::RouteCmdDuelTournamentMTDGameInfo(const MUID& uidStage, MTD_DuelTournamentGameInfo& GameInfo)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_DUELTOURNAMENT_GAME_INFO, MUID(0,0));
	pCmd->AddParameter(new MCmdParamBlob(&GameInfo, sizeof(MTD_DuelTournamentGameInfo)));
	RouteToBattle(uidStage, pCmd);
}

void MMatchServer::RouteCmdDuelTournamentMTDNextGamePlayerInfo(const MUID& uidStage, MTD_DuelTournamentNextMatchPlayerInfo& PlayerInfo)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_DUELTOURNAMENT_GAME_NEXT_MATCH_PLYAERINFO, MUID(0,0));
	pCmd->AddParameter(new MCmdParamBlob(&PlayerInfo, sizeof(MTD_DuelTournamentNextMatchPlayerInfo)));
	RouteToBattle(uidStage, pCmd);
}


void MMatchServer::RouteCmdDuelTournamentMTDRoundResultInfo(const MUID& uidStage, MTD_DuelTournamentRoundResultInfo* RoundResultInfo)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_DUELTOURNAMENT_GAME_ROUND_RESULT_INFO, MUID(0,0));
	pCmd->AddParameter(new MCmdParamBlob(RoundResultInfo, sizeof(MTD_DuelTournamentRoundResultInfo)));
	RouteToBattle(uidStage, pCmd);
}

void MMatchServer::RouteCmdDuelTournamentMTDMatchResultInfo(const MUID& uidStage, MTD_DuelTournamentMatchResultInfo* MatchResultInfo)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_DUELTOURNAMENT_GAME_MATCH_RESULT_INFO, MUID(0,0));
	pCmd->AddParameter(new MCmdParamBlob(MatchResultInfo, sizeof(MTD_DuelTournamentMatchResultInfo)));
	RouteToBattle(uidStage, pCmd);
}

