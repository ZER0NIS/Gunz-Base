#include "stdafx.h"
#include "MMatchGlobal.h"
#include "MMatchDuelTournamentMgr.h"
#include "MMatchConfig.h"

MMatchDuelTournamentMgr::MMatchDuelTournamentMgr()
: m_lastMatchedTick(0)
, m_pDTMatchLauncher(new MDuelTournamentMatchLauncher)
, m_nLastTimeCheckedTick(0)
, m_bIsServiceTime(false)
{
	for (int i=0; i<MDUELTOURNAMENTTYPE_MAX; ++i)
		m_DTMatchMakers[i].SetMatchObjectContainer(&m_matchObjectContainer);
}

MMatchDuelTournamentMgr::~MMatchDuelTournamentMgr()
{
	delete m_pDTMatchLauncher;
}

void MMatchDuelTournamentMgr::Init()
{
	ClearGroupRanking();

	const DWORD DT_MATCH_WAIT_TIME_LIMIT = MGetServerConfig()->GetDuelTournamentMatchMakingWaitLimit();
	const DWORD DT_MATCH_ACCEPTABLE_TP_GAP = MGetServerConfig()->GetDuelTournamentMatchMakingAcceptableTpGap();

	m_pDTMatchLauncher->SetLimitUserWaitTime(DT_MATCH_WAIT_TIME_LIMIT);
	m_pDTMatchLauncher->SetAcceptableTpGap(DT_MATCH_ACCEPTABLE_TP_GAP);
}

void MMatchDuelTournamentMgr::Destory()
{
}

void MMatchDuelTournamentMgr::AddGroupRanking(list<DTRankingInfo*>* pRankingList)
{
	ClearGroupRanking();

	list<DTRankingInfo*>::iterator iter = pRankingList->begin();
	for(; iter != pRankingList->end(); ++iter) {
		DTRankingInfo* pInfo = (*iter);
		memcpy(&m_GroupRankingBlob[pInfo->m_nRanking-1], pInfo, sizeof(DTRankingInfo));
	}
}

void MMatchDuelTournamentMgr::ClearGroupRanking()
{
	ZeroMemory(m_GroupRankingBlob, sizeof(DTRankingInfo) * MAX_DT_GROUP_RANKING_COUNT);
}

bool MMatchDuelTournamentMgr::AddPlayer(MDUELTOURNAMENTTYPE nType, MUID &uidPlayer)
{
	if (0 <= nType && nType < MDUELTOURNAMENTTYPE_MAX)
	{
		MMatchObjectDuelTournamentCharInfo* pDTCharInfo = m_matchObjectContainer.GetDuelTournamentCharInfo(uidPlayer);
		if (pDTCharInfo) {
			m_DTMatchMakers[nType].AddPlayer(uidPlayer, pDTCharInfo->GetTP(), MMatchServer::GetInstance()->GetGlobalClockCount());
			return true;
		}
	}
	return false;
}

bool MMatchDuelTournamentMgr::RemovePlayer(MDUELTOURNAMENTTYPE nType, MUID &uidPlayer)
{
	if (0 <= nType && nType < MDUELTOURNAMENTTYPE_MAX)
	{
		return m_DTMatchMakers[nType].RemovePlayer(uidPlayer);	
	}
	return false;
}

void MMatchDuelTournamentMgr::Tick(unsigned long nCurTick)
{
	const DWORD DT_MATCH_TICK_INTERVAL = MGetServerConfig()->GetDuelTournamentMatchMakingInterval();

	if (nCurTick - m_lastMatchedTick > DT_MATCH_TICK_INTERVAL)
	{
		m_lastMatchedTick = nCurTick;

		SYSTEMTIME stCurTime;
		GetLocalTime( &stCurTime );
		int nStartTime = MGetServerConfig()->GetDuelTournamentServiceStartTime();
		int nEndTime = MGetServerConfig()->GetDuelTournamentServiceEndTime();

		bool bServiceTime = false;
		if (nStartTime > nEndTime)	// 끝 시각이 자정을 넘어가는 경우
		{
			if (nStartTime <= stCurTime.wHour ||	// 시작시간~자정 사이의 순간이거나
				stCurTime.wHour <= nEndTime)		// 자정~끝시간 사이의 순간인 경우 서비스 가능
				bServiceTime = true;
		}
		else // 그날 시작해서 그날 끝나는 경우
		{
			if ( nStartTime <= stCurTime.wHour && stCurTime.wHour <= nEndTime )
				bServiceTime = true;
		}

		if ( bServiceTime)
		{ // 플레이 가능시간 ex)0~23이면 24시간 모두 서비스한다.
			for (int nType=0; nType<MDUELTOURNAMENTTYPE_MAX; ++nType)
			{
				m_pDTMatchLauncher->LaunchAvailableMatch(MDUELTOURNAMENTTYPE(nType), m_DTMatchMakers[nType], nCurTick);
			}
		}
		else
		{
			for (int nType=0; nType<MDUELTOURNAMENTTYPE_MAX; ++nType)
			{
				m_DTMatchMakers[nType].ServiceTimeClose();
			}
		}

		if(m_bIsServiceTime != bServiceTime)
		{
			m_bIsServiceTime = bServiceTime;
			if(m_bIsServiceTime)
				mlog("Dueltournament Service Open(%d-%d-%d Hour%d, Min%d) \n", stCurTime.wYear, stCurTime.wMonth, stCurTime.wDay, stCurTime.wHour, stCurTime.wMinute);
			else
				mlog("Dueltournament Service Close(%d-%d-%d Hour%d, Min%d) \n", stCurTime.wYear, stCurTime.wMonth, stCurTime.wDay, stCurTime.wHour, stCurTime.wMinute);
		}

	}

	if( nCurTick - m_nLastTimeCheckedTick > 1000 ) {
		m_nLastTimeCheckedTick = nCurTick;
		m_TimeChecker.Tick(nCurTick);
	}	
}
//=======================================================================================================================


void MDuelTournamentMatchLauncher::LaunchAvailableMatch( MDUELTOURNAMENTTYPE nType, MDuelTournamentMatchMaker& matchMaker, DWORD nCurTick )
{
	MDuelTournamentPickedGroup vecUidPlayer;

	int nPlayerCount = GetDTPlayerCount(MDUELTOURNAMENTTYPE(nType));

	matchMaker.CleanDisabledUid();

	// 오래 기다린 유저를 우선적으로 매치해준다
	int numPlayer = matchMaker.GetNumPlayer();
	int nSafetyNet = 0;
	while (nSafetyNet++ < numPlayer) // <= while(1)이어도 괜찮지만 만일의 경우 무한루프에 빠지는 것만은 피하고자함
	{
		const MDuelTournamentMatchMaker::DTUser* pDTUser = matchMaker.FindLongWaitPlayer(m_dwLimitUserWaitTime, nCurTick);
		if (NULL == pDTUser)
			break;

		if (false == matchMaker.PickGroupForPlayerAndRemove(vecUidPlayer,nPlayerCount, *pDTUser))
			break;

		LaunchMatchGroups( MDUELTOURNAMENTTYPE(nType), vecUidPlayer, MDUELTOURNAMENTMATCHMAKINGFACTOR_OVERWAIT);
	}

	// TP가 비슷한 유저들을 매치해준다
	matchMaker.PickMatchableGroupsAndRemove(vecUidPlayer, nPlayerCount, m_dwAcceptableTpGap);
	if (!vecUidPlayer.empty())
		LaunchMatchGroups( MDUELTOURNAMENTTYPE(nType), vecUidPlayer, MDUELTOURNAMENTMATCHMAKINGFACTOR_TPGAP);
}

void MDuelTournamentMatchLauncher::LaunchMatchGroups( MDUELTOURNAMENTTYPE nType, MDuelTournamentPickedGroup& vecUidPlayer, MDUELTOURNAMENTMATCHMAKINGFACTOR matchFactor )
{
	MDuelTournamentPickedGroup SmallPickedGroup;

	int nIndex = 0;
	int nPlayerCount = GetDTPlayerCount(nType);
	if (nPlayerCount == 0) {
		//_ASSERT(0);
		return;	// 0으로 나누기 방지
	}

	for( MDuelTournamentPickedGroup::iterator iter = vecUidPlayer.begin(); iter != vecUidPlayer.end(); ++iter)
	{
		nIndex++;

		SmallPickedGroup.push_back(*iter);

		if( nIndex % nPlayerCount == 0 ) {
			LaunchMatch(nType, &SmallPickedGroup, matchFactor);
			SmallPickedGroup.clear();
		}
	}
}

void MDuelTournamentMatchLauncher::LaunchMatch( MDUELTOURNAMENTTYPE nType, MDuelTournamentPickedGroup* pPickedGroup, MDUELTOURNAMENTMATCHMAKINGFACTOR matchFactor )
{
	pPickedGroup->Shuffle();
	MMatchServer* pServer = MMatchServer::GetInstance();
	pServer->LaunchDuelTournamentMatch(nType, pPickedGroup, matchFactor);
}