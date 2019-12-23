#ifndef _MDUELTOURNAMENT_MANAGER_H
#define _MDUELTOURNAMENT_MANAGER_H

#include "MDuelTournamentMatchMaker.h"
#include "MDuelTournamentTimeChecker.h"

#define MAX_DT_GROUP_RANKING_COUNT 100

class MDuelTournamentMatchLauncher;

//////////////////////////////////////////////////////////////////////
// Duel Tournament의 관리 클래스이다.
// 역할 1. 듀얼 토너먼트 Match-Up을 시켜주는 MatchMaker 기능
// 역할 2. 듀얼 토너먼트의 Group Ranking을 관리해주는 기능(정해진 시간마다 DB에서 받아오는 기능)
//////////////////////////////////////////////////////////////////////

class MMatchDuelTournamentMgr
{
	MMatchObjectContainer m_matchObjectContainer;
	MDuelTournamentTimeChecker m_TimeChecker;

	DWORD m_lastMatchedTick;		// 마지막으로 매치 처리한 시각
	DWORD m_nLastTimeCheckedTick;	// 마지막으로 TimeChecker를 실행한 시각
	bool m_bIsServiceTime;			// 서비스 시간에 대한 로그를 남기기 위해

protected:	
	DTRankingInfo m_GroupRankingBlob[MAX_DT_GROUP_RANKING_COUNT];
	MDuelTournamentMatchMaker m_DTMatchMakers[MDUELTOURNAMENTTYPE_MAX];
	MDuelTournamentMatchLauncher* m_pDTMatchLauncher;

	void ClearGroupRanking();
	void LaunchMatch(MDUELTOURNAMENTTYPE nType, MDuelTournamentPickedGroup& vecUidPlayer);
public:
	MMatchDuelTournamentMgr();
	~MMatchDuelTournamentMgr();

	void Init();
	void Destory();

	bool AddPlayer(MDUELTOURNAMENTTYPE nType, MUID &uidPlayer);
	bool RemovePlayer(MDUELTOURNAMENTTYPE nType, MUID &uidPlayer);
	
	void Tick(unsigned long nTick);

	void AddGroupRanking(list<DTRankingInfo*>* pRankingList);

	void  SetTimeStamp(const char* szTimeStamp)		{ m_TimeChecker.SetTimeStamp(szTimeStamp); }
	char* GetTimeStamp()							{ return m_TimeChecker.GetTimeStamp(); }
	bool  IsSameTimeStamp(const char* szTimeStamp)  { return m_TimeChecker.IsSameTimeStamp(szTimeStamp); }

	bool GetTimeStampChanged()				{ return m_TimeChecker.GetTimeStampChanged(); }
	void SetTimeStampChanged(bool bValue)	{ m_TimeChecker.SetTimeStampChanged(bValue); }
};


// MMatchDuelTournamentMgr는 이 클래스를 이용해서 MatchMaker가 뽑아낸 그룹을 실제로 경기시킨다
class MDuelTournamentMatchLauncher
{
	DWORD m_dwLimitUserWaitTime;
	DWORD m_dwAcceptableTpGap;

public:
	MDuelTournamentMatchLauncher() : m_dwLimitUserWaitTime(10000), m_dwAcceptableTpGap(10) {}

	void SetLimitUserWaitTime(DWORD n) { m_dwLimitUserWaitTime = n; }
	void SetAcceptableTpGap(DWORD n) { m_dwAcceptableTpGap = n; }

	void LaunchAvailableMatch(MDUELTOURNAMENTTYPE nType, MDuelTournamentMatchMaker& matchMaker, DWORD nCurTick);

protected:
	void LaunchMatchGroups(MDUELTOURNAMENTTYPE nType, MDuelTournamentPickedGroup& vecUidPlayer, MDUELTOURNAMENTMATCHMAKINGFACTOR matchFactor);

	// 실제 MMatchServer에게 경기 실행시키는 부분을 래핑하는 함수 (단위테스트 목적)
	virtual void LaunchMatch(MDUELTOURNAMENTTYPE nType, MDuelTournamentPickedGroup* pPickedGroup, MDUELTOURNAMENTMATCHMAKINGFACTOR matchFactor);
};

#endif