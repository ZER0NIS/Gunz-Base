#include "stdafx.h"
#include "MBMatchServer.h"
#include "MMatrix.h"
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
#include "MVoteDiscussImpl.h"
#include "MMatchUtil.h"
#include "MUtil.h"
#include "MMatchGameType.h"
#include "MMatchRuleBaseQuest.h"
#include "MMatchRuleQuest.h"
#include "MMatchRuleBerserker.h"
#include "MMatchRuleDuel.h"
#include "MMatchConfig.h"
#include "MMatchLocale.h"
#include "MBMatchAsyncDBJob_UpdateCharLevel.h"


void MBMatchServer::OnGameKill(const MUID& uidAttacker, const MUID& uidVictim)
{
	MMatchObject* pVictim = GetObject(uidVictim);
	MMatchObject* pAttacker = GetObject(uidAttacker);
	if (pVictim == NULL) return;

	MMatchStage* pStage = FindStage(pVictim->GetStageUID());
	if (pStage == NULL) return;

	// 서버는 죽은줄 알고있었는데 또 죽었다고 신고들어온경우 죽었다는 메시지만 라우팅한다
	if (pVictim->CheckAlive() == false)
	{	
		// 이 요청을 해킹에 의해서 무한 호출이 되면 렉을 유발할 수 있다. - by SungE 2007-04-17
		if( MIN_REQUEST_SUICIDE_TIME > (GetGlobalClockCount() - pVictim->GetLastSendSuicideCmdTime()) )
			return;

		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SUICIDE, MUID(0,0));
		pNew->AddParameter(new MCommandParameterInt(MOK));
		pNew->AddParameter(new MCommandParameterUID(pVictim->GetUID()));
		RouteToBattle(pStage->GetUID(), pNew);

		pVictim->SetLastSendSuicideCmdTime( GetGlobalClockCount() );

		return;
	}

	MUID TheAttackerUID = uidAttacker;
	if (pAttacker != NULL)
	{
		if (pAttacker->GetStageUID() != pVictim->GetStageUID())
		{
			pAttacker = NULL;
			TheAttackerUID = MUID(0,0);
		}
	}

	pVictim->OnDead();

	if (TheAttackerUID != uidVictim) 
	{
		if (pAttacker != NULL) 
		{
			pAttacker->OnKill();
		}
	}

	if ((pAttacker != NULL) && (pVictim != NULL))
	{
		ProcessOnGameKill(pStage, pAttacker, pVictim);
	}

	pStage->OnGameKill(TheAttackerUID, uidVictim);	
}


bool MBMatchServer::IsPowerLevelingHacker( MMatchObject* pObj )
{
	MMatchPowerLevelingConfig& Config = MGetServerConfig()->GetPowerLevelingConfig();

	if( !Config.IsUsePowerLevelingDBBlock() 
		|| (NULL == pObj) 
		|| (NULL == pObj->GetAccountInfo()) ) 
		return false;

	MMatchAccountInfo* pAccInfo = pObj->GetAccountInfo();
	
	if( !pAccInfo->m_bIsPowerLevelingHacker )
		return false;

	_ASSERT( 0 < Config.GetPowerLevelingDBBlockTime() );
	// _ASSERT( 0 < pAccInfo->m_nPowerLevelingRegTimeMin );
	_ASSERT( pAccInfo->m_nPowerLevelingRegTimeMin <= (int)(GetGlobalClockCount() / 60000) );

	// 블럭시간을 계산한다.
	if( (Config.GetPowerLevelingDBBlockTime() / 60000) <
		((GetGlobalClockCount() / 60000) - (pAccInfo->m_nPowerLevelingRegTimeMin)) )
	{
		pAccInfo->m_bIsPowerLevelingHacker = false;
		
		m_MatchDBMgr.UpdateAccountPowerLevelingInfo( pAccInfo->m_nAID, false );
		return false;
	}

	return true;
}


bool MBMatchServer::IncreaseAttackerKillCount( MMatchObject* pAttacker, MMatchObject* pVictim, const DWORD dwCurTime )
{
	if( (NULL == pAttacker) || (NULL == pVictim) )
		return false;

	if( !m_KillTracker.IncreaseAttackCount(pAttacker->GetCharInfo()->m_nCID
		, pVictim->GetCharInfo()->m_nCID
		, dwCurTime) )
	{
		if( NULL != pVictim->GetAccountInfo() )
		{
			pAttacker->GetAccountInfo()->m_bIsPowerLevelingHacker = true;
			pAttacker->GetAccountInfo()->m_nPowerLevelingRegTimeMin = dwCurTime / 60000;

			GetDBMgr()->UpdateAccountPowerLevelingInfo( pVictim->GetAccountInfo()->m_nAID, true );
		}
		
		return false;	
	}

	return true;
}


// 적을 죽였을 경우 경험치 계산
void MBMatchServer::ProcessOnGameKill(MMatchStage* pStage, MMatchObject* pAttacker, MMatchObject* pVictim)
{
	/*
		경험치 계산
		죽었다는 메세지 전송
		캐릭터에 경험치 적용
		레벨 계산
		Kill, Death, 바운티 계산, 적용
		DB캐싱 업데이트
		레벨업 메세지 전송
	*/

	bool bSuicide = false;		// 자살
	if (pAttacker == pVictim) bSuicide = true;		

	int nAttackerExp = 0;
	int nVictimExp = 0;
	int nAttackerLevel = pAttacker->GetCharInfo()->m_nLevel;
	int nVictimLevel = pVictim->GetCharInfo()->m_nLevel;
	MMATCH_GAMETYPE nGameType = pStage->GetStageSetting()->GetGameType();
	float fGameExpRatio = MGetGameTypeMgr()->GetInfo(nGameType)->fGameExpRatio;
	MUID uidStage = pAttacker->GetStageUID();

	// 경험치 계산
	CalcExpOnGameKill(pStage, pAttacker, pVictim, &nAttackerExp, &nVictimExp);

	// 죽었다는 메세지를 방전체에게 보낸다.
	PostGameDeadOnGameKill(uidStage, pAttacker, pVictim, nAttackerExp, nVictimExp);

	if( IsPowerLevelingHacker(pAttacker) ) return;
	if( !IncreaseAttackerKillCount(pAttacker, pVictim) ) return;

	// 만약 게임모드가 Training이면 Xp등의 업데이트를 하지 않는다.
	if (nGameType == MMATCH_GAMETYPE_TRAINING) return;

	// 캐릭터 XP 업데이트
	pAttacker->GetCharInfo()->IncXP(nAttackerExp);
	pVictim->GetCharInfo()->DecXP(nVictimExp);

	
	////////////////////////////////////////////////////////////////////////////////////////////
	// 케릭터 PlayTime 업데이트 해주기!									2010-06-07 홍기주
	int nPlayTime;
	
	nPlayTime = MGetTimeDistance(pAttacker->GetCharInfo()->m_nConnTime, GetTickTime()) / 1000;
	pAttacker->GetCharInfo()->UpdatePlayTime(nPlayTime);
	
	nPlayTime = MGetTimeDistance(pVictim->GetCharInfo()->m_nConnTime, GetTickTime()) / 1000;
	pVictim->GetCharInfo()->UpdatePlayTime(nPlayTime);
	////////////////////////////////////////////////////////////////////////////////////////////
		

	// 레벨 계산
	int nNewAttackerLevel = -1, nNewVictimLevel = -1;
	if ((!bSuicide) && (pAttacker->GetCharInfo()->m_nLevel < MAX_LEVEL) &&
		(pAttacker->GetCharInfo()->m_nXP >= MMatchFormula::GetNeedExp(nAttackerLevel)))
	{
		nNewAttackerLevel = MMatchFormula::GetLevelFromExp(pAttacker->GetCharInfo()->m_nXP);
		if (nNewAttackerLevel != pAttacker->GetCharInfo()->m_nLevel) pAttacker->GetCharInfo()->m_nLevel = nNewAttackerLevel;
	}

	if ((pVictim->GetCharInfo()->m_nLevel > 0) &&
		(pVictim->GetCharInfo()->m_nXP < MMatchFormula::GetNeedExp(nVictimLevel-1)))
	{
		nNewVictimLevel = MMatchFormula::GetLevelFromExp(pVictim->GetCharInfo()->m_nXP);
		if (nNewVictimLevel != pVictim->GetCharInfo()->m_nLevel) pVictim->GetCharInfo()->m_nLevel = nNewVictimLevel;
	}

	if (!bSuicide)
	{
		// KillCount 계산
		pAttacker->GetCharInfo()->IncKill();

		// 바운티 추가해준다
		// int nAddedBP = (int)(MMatchFormula::GetGettingBounty(nAttackerLevel, nVictimLevel) * fGameExpRatio);
		const int nBPBonus = CalcBPonGameKill( pStage, pAttacker, nAttackerLevel, nVictimLevel );

		pAttacker->GetCharInfo()->IncBP(nBPBonus);

		MMatchCharBattleTimeRewardInfoMap::iterator iter = pAttacker->GetCharInfo()->GetBRInfoMap().begin();
		for( ; iter !=  pAttacker->GetCharInfo()->GetBRInfoMap().end(); iter++ ) {
			MMatchCharBRInfo* pInfo = iter->second;
			pInfo->AddKillCount(10);	///< 사람 Kill은 10 Kill로 계산!
		}
	}

	// DeathCount 계산
	pVictim->GetCharInfo()->IncDeath();

	// DB 캐슁 업데이트
	if (pAttacker->GetCharInfo()->GetDBCachingData()->IsRequestUpdate()) {
		UpdateCharDBCachingData(pAttacker);
	}

	if (pVictim->GetCharInfo()->GetDBCachingData()->IsRequestUpdate()) {
		UpdateCharDBCachingData(pAttacker);
	}

	// 만약 레벨이 바뀌면 따로 레벨업한다.
	if ((!bSuicide) && (nNewAttackerLevel >= 0) && (nNewAttackerLevel != nAttackerLevel))
	{
		// 레벨이 바뀌면 바로 캐슁 업데이트한다
		UpdateCharDBCachingData(pAttacker);

		pAttacker->GetCharInfo()->m_nLevel = nNewAttackerLevel;

		MBMatchAsyncDBJob_UpdateCharLevel* pUpdateAttackerCharlevel = new MBMatchAsyncDBJob_UpdateCharLevel(pAttacker->GetUID());
		if( NULL == pUpdateAttackerCharlevel ) {
			return;
		}

		pUpdateAttackerCharlevel->Input( pAttacker->GetCharInfo()->m_nCID
									, nNewAttackerLevel
									, pAttacker->GetCharInfo()->m_nBP
									, pAttacker->GetCharInfo()->m_nTotalKillCount
									, pAttacker->GetCharInfo()->m_nTotalDeathCount
									, pAttacker->GetCharInfo()->m_nTotalPlayTimeSec
									, true );

		// PostAsyncJob( pUpdateAttackerCharlevel );
		pAttacker->m_DBJobQ.DBJobQ.push_back( pUpdateAttackerCharlevel );
	}
	if ((nNewVictimLevel >= 0) && (nNewVictimLevel != nVictimLevel))
	{
		// 레벨이 바뀌면 바로 캐슁 업데이트한다
		UpdateCharDBCachingData(pVictim);

		pVictim->GetCharInfo()->m_nLevel = nNewVictimLevel;


		MBMatchAsyncDBJob_UpdateCharLevel* pUpdateVictimCharlevel = new MBMatchAsyncDBJob_UpdateCharLevel(pVictim->GetUID());
		if( NULL == pUpdateVictimCharlevel )
		{
			return;
		}

		pUpdateVictimCharlevel->Input( pVictim->GetCharInfo()->m_nCID
			, nNewVictimLevel
			, pVictim->GetCharInfo()->m_nBP
			, pVictim->GetCharInfo()->m_nTotalKillCount
			, pVictim->GetCharInfo()->m_nTotalDeathCount
			, pVictim->GetCharInfo()->m_nTotalPlayTimeSec
			, true );

		// PostAsyncJob( pUpdateVictimCharlevel );

		pVictim->m_DBJobQ.DBJobQ.push_back( pUpdateVictimCharlevel );
	}



	// 레벨업, 레벨 다운 메세지 보내기
	if ((!bSuicide) && (nNewAttackerLevel >= 0) && (nNewAttackerLevel > nAttackerLevel))
	{
		MCommand* pCmd = CreateCommand(MC_MATCH_GAME_LEVEL_UP, MUID(0,0));
		pCmd->AddParameter(new MCommandParameterUID(pAttacker->GetUID()));
		pCmd->AddParameter(new MCommandParameterInt(nNewAttackerLevel));
		RouteToBattle(uidStage, pCmd);	

	}
	if ((nNewVictimLevel >= 0) && (nNewVictimLevel < nVictimLevel))
	{
		MCommand* pCmd = CreateCommand(MC_MATCH_GAME_LEVEL_DOWN, MUID(0,0));
		pCmd->AddParameter(new MCommandParameterUID(pVictim->GetUID()));
		pCmd->AddParameter(new MCommandParameterInt(nNewVictimLevel));
		RouteToBattle(uidStage, pCmd);	
	}
}


void MBMatchServer::OnStageJoin(const MUID& uidChar, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	if( !IsEnabledObject(pObj) )
		return;

	MMatchStage* pStage = NULL;

	if (uidStage == MUID(0,0)) {
		return;
	} else {
		pStage = FindStage(uidStage);
	}

	if (pStage == NULL) {
		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, MERR_STAGE_NOT_EXIST);
		return;
	}

	if ((IsAdminGrade(pObj) == false) && pStage->IsPrivate())
	{
		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, MERR_CANNOT_JOIN_STAGE_BY_PASSWORD);
		return;
	}

	StageJoin(uidChar, pStage->GetUID());
}


// 플레이어가 방에 들어갈 수 있는지 검증한다.
int MBMatchServer::ValidateStageJoin(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (! IsEnabledObject(pObj)) return MERR_CANNOT_JOIN_STAGE;

	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return MERR_CANNOT_JOIN_STAGE;

	// close상태인지 체크
	if (pStage->GetState() == STAGE_STATE_CLOSE) return MERR_CANNOT_JOIN_STAGE;

	// 운영자, 관리자면 제한없이 입장
	if (!IsAdminGrade(pObj))
	{
		// 인원체크
		if (pStage->GetStageSetting()->GetMaxPlayers() <= pStage->GetCountableObjCount())
		{
			return MERR_CANNOT_JOIN_STAGE_BY_MAXPLAYERS;
		}

		// 레벨체크
		if (pStage->GetStageSetting()->GetLimitLevel() != 0)
		{
			int nMasterLevel, nLimitLevel;
			MMatchObject* pMaster = GetObject(pStage->GetMasterUID());

			if (IsEnabledObject(pMaster))
			{
				nMasterLevel = pMaster->GetCharInfo()->m_nLevel;
				nLimitLevel = pStage->GetStageSetting()->GetLimitLevel();
				if (abs(pObj->GetCharInfo()->m_nLevel - nMasterLevel) > nLimitLevel)
				{
					return MERR_CANNOT_JOIN_STAGE_BY_LEVEL;
				}
			}
		}

		// 게임중참가
		if ((pStage->GetStageSetting()->GetForcedEntry() == false) && 
			(pStage->GetState() != STAGE_STATE_STANDBY))
		{
			return MERR_CANNOT_JOIN_STAGE_BY_FORCEDENTRY;
		}

		// Ban Check
		if (pStage->CheckBanList(pObj->GetCharInfo()->m_nCID))
			return MERR_CANNOT_JOIN_STAGE_BY_BAN;
	}

	return MOK;
}
