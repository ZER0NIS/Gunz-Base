#include "stdafx.h"
#include "../../sdk/UnitTest++/src/UnitTest++.h"
#include "MDuelTournamentMatchMaker.h"
#include "MMatchDuelTournamentMgr.h"

#ifdef _DEBUG

class MMockMatchObjectContainer : public MMatchObjectContainer 
{
public:
	std::set<MUID> m_setDisabledUid;

	// 테스트에서 uid 유효검사를 처리하기 위해 오버라이딩
	virtual bool IsEnabledUid(const MUID& uid) {
		return m_setDisabledUid.find(uid) == m_setDisabledUid.end();
	}

} g_MockMatchObjectContainer;


TEST(DuelTournamentMatchMaker_AddRemovePlayer)
{
	MUID uidInvalid;
	uidInvalid.SetInvalid();

	MDuelTournamentMatchMaker matchMaker(&g_MockMatchObjectContainer);

	// 유저 3명이 토너먼트 신청을 했다
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,1), 1000, 0));
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,2), 1000, 0));
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,3),  998, 0));
	// 3명이 들어있어야 한다
	CHECK_EQUAL(3, matchMaker.GetNumPlayer());

	// 이미 추가된 유저가 중복 추가되는 경우 -> 실패해야한다
	CHECK_EQUAL(false, matchMaker.AddPlayer(MUID(0,1), 1000, 0));
	CHECK_EQUAL(false, matchMaker.AddPlayer(MUID(0,1), 1111, 0));

	// 유효하지 않은 uid가 추가됐을 때 -> 실패해야한다
	CHECK_EQUAL(false, matchMaker.AddPlayer(uidInvalid, 1000, 0));

	// 유저를 제거한다
	CHECK_EQUAL(true, matchMaker.RemovePlayer(MUID(0,1)));
	// 2명이 들어있어야 한다
	CHECK_EQUAL(2, matchMaker.GetNumPlayer());

	// 없는 유저를 제거하려 한다 -> 실패해야한다
	CHECK_EQUAL(false, matchMaker.RemovePlayer(MUID(0,1)));
	// 유효하지 않은 uid를 주고 제거시킨다 -> 실패해야한다
	CHECK_EQUAL(false, matchMaker.RemovePlayer(uidInvalid));

	// 전부 제거
	matchMaker.RemoveAll();
	CHECK_EQUAL(0, matchMaker.GetNumPlayer());
}

// TP가 비슷한 유저끼리 그룹을 짓는 테스트
TEST(DuelTournamentMatchMaker_PickMatchableGroups)
{
	MMockDTMatchMaker matchMaker(&g_MockMatchObjectContainer);
	vector<MMockDTMatchMaker::PairDTMatch> vecMatchGroup;

	// 1명 넣고 2명을 요구해보자
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,1), 1000, 0));
	matchMaker.PickMatchableGroups(vecMatchGroup, 2, 10);
	CHECK_EQUAL(true, vecMatchGroup.empty());

	// TP가 같은 2명을 넣고 2명 매치
	matchMaker.RemoveAll();
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,1), 1000, 0));
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,2), 1000, 0));
	matchMaker.PickMatchableGroups(vecMatchGroup, 2, 10);
	CHECK_EQUAL(1, (int)vecMatchGroup.size());
	CHECK(MUID(0,1) == vecMatchGroup[0].first.uid);
	CHECK(MUID(0,2) == vecMatchGroup[0].second.uid);

	// tpGap이 0일때 (둘다 TP가 같으므로 매치가 성립해야 한다)
	matchMaker.PickMatchableGroups(vecMatchGroup, 2, 0);
	CHECK_EQUAL(1, (int)vecMatchGroup.size());

	// tpGap이 음수일때
	matchMaker.PickMatchableGroups(vecMatchGroup, 2, -1);
	CHECK_EQUAL(true, vecMatchGroup.empty());

	// 매치인원이 1이하일때
	matchMaker.PickMatchableGroups(vecMatchGroup, 1, 10);
	CHECK_EQUAL(true, vecMatchGroup.empty());
	matchMaker.PickMatchableGroups(vecMatchGroup, 0, 10);
	CHECK_EQUAL(true, vecMatchGroup.empty());
	matchMaker.PickMatchableGroups(vecMatchGroup, -1, 10);
	CHECK_EQUAL(true, vecMatchGroup.empty());

	// 5명을 넣고 2명씩 매치
	matchMaker.RemoveAll();
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,1), 940, 0));
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,2), 1000, 0));
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,3), 1001, 0));
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,4), 1002, 0));
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,5), 1003, 0));
	matchMaker.PickMatchableGroups(vecMatchGroup, 2, 10);
	CHECK_EQUAL(2, (int)vecMatchGroup.size());
	// MUID(0,1)은 남게된다
	CHECK(MUID(0,2) == vecMatchGroup[0].first.uid);
	CHECK(MUID(0,3) == vecMatchGroup[0].second.uid);
	CHECK(MUID(0,4) == vecMatchGroup[1].first.uid);
	CHECK(MUID(0,5) == vecMatchGroup[1].second.uid);
	// 3명씩 매치
	matchMaker.PickMatchableGroups(vecMatchGroup, 3, 10);
	CHECK_EQUAL(1, (int)vecMatchGroup.size());
	// MUID(0,1), MUID(0,5)는 남는다
	CHECK(MUID(0,2) == vecMatchGroup[0].first.uid);
	CHECK(MUID(0,4) == vecMatchGroup[0].second.uid);
}

// 특정 플레이어를 포함하면서 TP가 가장 유사한 유저끼리의 그룹 짓기 테스트
TEST(DuelTournamentMatchMaker_PickGroupForPlayer)
{
	MMockDTMatchMaker::PairDTMatch matchGroup;

	MMockDTMatchMaker matchMaker(&g_MockMatchObjectContainer);
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,1), 1000, 0));
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,2), 1100, 0));
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,3), 1260, 0));
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,4), 1300, 0));
	// (0,1)은 그룹지을 수 있는 유저가 (0,2) 밖에 없다
	CHECK_EQUAL(true, matchMaker.PickGroupForPlayer(matchGroup, 2, MMockDTMatchMaker::DTUser(MUID(0,1), 1000) ));
	CHECK(MUID(0,1) == matchGroup.first.uid);
	CHECK(MUID(0,2) == matchGroup.second.uid);
	// (0,2)는 (0,3)보다 (0,1)과 더 가깝다.
	CHECK_EQUAL(true, matchMaker.PickGroupForPlayer(matchGroup, 2, MMockDTMatchMaker::DTUser(MUID(0,2), 1100) ));
	CHECK(MUID(0,1) == matchGroup.first.uid);
	CHECK(MUID(0,2) == matchGroup.second.uid);
	CHECK_EQUAL(true, matchMaker.PickGroupForPlayer(matchGroup, 2, MMockDTMatchMaker::DTUser(MUID(0,3), 1260) ));
	CHECK(MUID(0,3) == matchGroup.first.uid);
	CHECK(MUID(0,4) == matchGroup.second.uid);
	CHECK_EQUAL(true, matchMaker.PickGroupForPlayer(matchGroup, 2, MMockDTMatchMaker::DTUser(MUID(0,4), 1300) ));
	CHECK(MUID(0,3) == matchGroup.first.uid);
	CHECK(MUID(0,4) == matchGroup.second.uid);

	// 3명 매치
	CHECK_EQUAL(true, matchMaker.PickGroupForPlayer(matchGroup, 3, MMockDTMatchMaker::DTUser(MUID(0,2), 1100) ));
	CHECK(MUID(0,2) == matchGroup.first.uid);		// (0,1)~(0,3) 보다 이 그룹이 TP차이가 적다
	CHECK(MUID(0,4) == matchGroup.second.uid);

	// 4명 매치
	CHECK_EQUAL(true, matchMaker.PickGroupForPlayer(matchGroup, 4, MMockDTMatchMaker::DTUser(MUID(0,1), 1000) ));
	CHECK(MUID(0,1) == matchGroup.first.uid);
	CHECK(MUID(0,4) == matchGroup.second.uid);
	CHECK_EQUAL(true, matchMaker.PickGroupForPlayer(matchGroup, 4, MMockDTMatchMaker::DTUser(MUID(0,4), 1300) ));
	CHECK(MUID(0,1) == matchGroup.first.uid);
	CHECK(MUID(0,4) == matchGroup.second.uid);

	// 지금 있는 유저보다 많은 인원을 매치시도
	CHECK_EQUAL(false, matchMaker.PickGroupForPlayer(matchGroup, 5, MMockDTMatchMaker::DTUser(MUID(0,1), 1000) ));

	// 매치 인원을 1이하로 주었을때
	CHECK_EQUAL(false, matchMaker.PickGroupForPlayer(matchGroup, 1, MMockDTMatchMaker::DTUser(MUID(0,1), 1000) ));
	CHECK_EQUAL(false, matchMaker.PickGroupForPlayer(matchGroup, 0, MMockDTMatchMaker::DTUser(MUID(0,1), 1000) ));
	CHECK_EQUAL(false, matchMaker.PickGroupForPlayer(matchGroup, -1, MMockDTMatchMaker::DTUser(MUID(0,1), 1000) ));

	// 없는 사람을 기준으로 주었을때
	CHECK_EQUAL(false, matchMaker.PickGroupForPlayer(matchGroup, 2, MMockDTMatchMaker::DTUser(MUID(0,1), 4444) ));
	CHECK_EQUAL(false, matchMaker.PickGroupForPlayer(matchGroup, 2, MMockDTMatchMaker::DTUser(MUID(1,1), 1000) ));
}

// 특정 시간보다 오래 기다린 유저를 찾아내기 테스트
TEST(DuelTournamentMatchMaker_FindLongWaitPlayer)
{
	MMockDTMatchMaker matchMaker(&g_MockMatchObjectContainer);
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,1), 1000, 1500));
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,2), 1100, 1700));
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,3), 1260, 1300));
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,4), 1300, 1900));
	const MMockDTMatchMaker::DTUser* pDTUser;
	pDTUser = matchMaker.FindLongWaitPlayer(690, 2000);
	CHECK(MUID(0,3) == pDTUser->uid);
}

// 유저들을 범위 삭제하는 기능(매칭그룹을 만들고 대기자 목록에서 빼기 위한 기능) 테스트
TEST(DuelTournamentMatchMaker_RemovePlayers)
{
	MMockDTMatchMaker matchMaker(&g_MockMatchObjectContainer);
	vector<MMockDTMatchMaker::PairDTMatch> vecMatchGroup;

	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,1), 1400, 0));
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,2), 1001, 0));	//1
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,3), 1002, 0));	//1
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,4), 1552, 0));	//	3
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,5), 1551, 0));	//  3
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,6), 1551, 0));	//  3
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,7), 1553, 0));	
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,8), 1333, 0));	// 2
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,9), 1003, 0));	//1
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,10), 1334, 0));	// 2
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,11), 800, 0));
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,12), 1324, 0));	// 2
	matchMaker.PickMatchableGroups(vecMatchGroup, 3, 10);
	CHECK_EQUAL(3, (int)vecMatchGroup.size());
	for (int i=0; i<(int)vecMatchGroup.size(); ++i)
		matchMaker.RemovePlayers(vecMatchGroup[i].first, vecMatchGroup[i].second);
	CHECK_EQUAL(3, matchMaker.GetNumPlayer());
	CHECK(matchMaker.m_mapUser.end() != matchMaker.m_mapUser.find(MDuelTournamentMatchMaker::DTUser(MUID(0,1), 1400)));
	CHECK(matchMaker.m_mapUser.end() != matchMaker.m_mapUser.find(MDuelTournamentMatchMaker::DTUser(MUID(0,7), 1553)));
	CHECK(matchMaker.m_mapUser.end() != matchMaker.m_mapUser.find(MDuelTournamentMatchMaker::DTUser(MUID(0,11), 800)));
}

// (접속해지 등으로)무효가 된 uid를 대기자 목록에서 제거하는 기능 테스트
TEST(DuelTournamentMatchMaker_CleanDisabledUid)
{
	MMockDTMatchMaker matchMaker(&g_MockMatchObjectContainer);
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,1), 1000, 0));
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,2), 1000, 0));
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,3), 1000, 0));
	CHECK_EQUAL(true, matchMaker.AddPlayer(MUID(0,4), 1000, 0));
	// 현재 무효가 된 uid는 (0,2) (0,4) 라고 하자
	g_MockMatchObjectContainer.m_setDisabledUid.insert(MUID(0,2));
	g_MockMatchObjectContainer.m_setDisabledUid.insert(MUID(0,4));
	
	matchMaker.CleanDisabledUid();
	// 남은건 2명 뿐이고, 유효한 uid를 가진 유저다
	CHECK_EQUAL(2, matchMaker.GetNumPlayer());
	CHECK(matchMaker.m_mapUser.end() != matchMaker.m_mapUser.find(MDuelTournamentMatchMaker::DTUser(MUID(0,1), 1000)));
	CHECK(matchMaker.m_mapUser.end() != matchMaker.m_mapUser.find(MDuelTournamentMatchMaker::DTUser(MUID(0,3), 1000)));
}


///////////////////////////////////////////////////////////////////////////////////////////////
//			MDuelTournamentMatchLauncher 매치런처에 대한 테스트

struct MOCKUSERDATA 
{
	int tp;
	bool bMatchWait;
};

typedef map<MUID, MOCKUSERDATA> MapMockUser;
typedef MapMockUser::iterator	ItorMockUser;
static ItorMockUser PickRandomly(MapMockUser& m)
{
	int size = (int)m.size();
	int r = rand() % size;
	ItorMockUser it = m.begin();
	for (int i=0; i<r; ++i) ++it;
	return it;
}

struct _functorIntLess {
	bool operator() (int i,int j) { return (i<j);}
} functorIntLess;


// 테스트를 위해 실제로 경기를 런칭하지 말고 대신 로그를 남기도록 상속한 클래스
class MMockDuelTournamentMatchLauncher : public MDuelTournamentMatchLauncher
{
public:
	MapMockUser* pMapMockUser;
	MMockDuelTournamentMatchLauncher(MapMockUser* m) : pMapMockUser(m) {}

	struct MatchResult
	{
		std::vector<int> vecGroupTP;
		bool bOverWaitTimeLimitGroup;		// 대기시간 초과로 만들어진 그룹인가
		void Sort() { std::sort(vecGroupTP.begin(), vecGroupTP.end(), functorIntLess); }
		int GetSmallestTP() { _ASSERT(!vecGroupTP.empty()); return vecGroupTP[0]; }
		int GetBiggestTP()  { _ASSERT(!vecGroupTP.empty()); return *(--vecGroupTP.end()); }
		int GetTpGap() { return GetBiggestTP() - GetSmallestTP(); }
	};
	std::vector<MatchResult> m_vecResult;

	MatchResult m_biggestTpGapResult;

	virtual void LaunchMatch(MDUELTOURNAMENTTYPE nType, MDuelTournamentPickedGroup* pPickedGroup, MDUELTOURNAMENTMATCHMAKINGFACTOR matchFactor) {
		int size = (int)pPickedGroup->size();
		m_vecResult.clear();
		MatchResult result;
		result.bOverWaitTimeLimitGroup = (matchFactor==MDUELTOURNAMENTMATCHMAKINGFACTOR_OVERWAIT ? true : false);
		for (int i=0; i<size; ++i) {
			ItorMockUser it = pMapMockUser->find((*pPickedGroup)[i]);
			result.vecGroupTP.push_back( it->second.tp);
		}
		result.Sort();
		m_vecResult.push_back(result);

		if (m_biggestTpGapResult.vecGroupTP.empty())
			m_biggestTpGapResult = result;
		else if (m_biggestTpGapResult.GetTpGap() < result.GetTpGap())
			m_biggestTpGapResult = result;

		LogMatch(result);
	}

	void LogMatch(MatchResult& result)
	{
		return; // 로그찍느라 서버실행이 너무 느려서
		if (result.bOverWaitTimeLimitGroup)
			mlog("TIMEOVER: ");
		else 
			mlog("TP      : ");
		int size = (int)result.vecGroupTP.size();
		for (int i=0; i<size; ++i) {
			mlog("%d ", result.vecGroupTP[i]);
		}
		mlog("\n");
	}
};

// 전체기능을 테스트해보자
TEST(DuelTournamentMatchLauncher_FullFunctionalityTest)
{
	srand(timeGetTime());
	// 랜덤하게 유저풀을 구성한다
	std::map<MUID, MOCKUSERDATA> mapUser;
	for (int i=0; i<500; ++i) {
		MUID uid(0,i+1);
		MOCKUSERDATA userdata;
		userdata.tp = rand()%500 + 1000;
		userdata.bMatchWait = false;
		mapUser.insert(pair<MUID, MOCKUSERDATA>(uid, userdata));
	}

	const ACCEPTABLE_TPGAP = 15;

	MMockDTMatchMaker matchMaker(&g_MockMatchObjectContainer);
	MMockDuelTournamentMatchLauncher launcher(&mapUser);
	launcher.SetLimitUserWaitTime(10000);
	launcher.SetAcceptableTpGap(ACCEPTABLE_TPGAP);

	// [매치신청=>그루핑=>매치결과검사]를 충분한 횟수를 반복해보자
	DWORD curTime = 0;
	for (int i=0; i<100; ++i)
	{
		curTime += 500;

		// 일부를 참가시킨다
		int numJoin = rand() % 50;
		for (int k=0; k<numJoin; ++k) {
			ItorMockUser it = PickRandomly(mapUser);
			if (it->second.bMatchWait) continue;
			matchMaker.AddPlayer(it->first, it->second.tp, curTime++);
		}
        
		// 런칭시킨다
		if (i%5 == 0) {	// 루프 5번에 한번 런칭하자
			launcher.LaunchAvailableMatch(MDUELTOURNAMENTTYPE_QUATERFINAL, matchMaker, curTime);

			int size = (int)launcher.m_vecResult.size();
			for (int k=0; k<size; ++k)
			{
				CHECK_EQUAL(GetDTPlayerCount(MDUELTOURNAMENTTYPE_QUATERFINAL), (int)launcher.m_vecResult[k].vecGroupTP.size());
				
				if (false == launcher.m_vecResult[k].bOverWaitTimeLimitGroup)
				{
					int smallestTP = launcher.m_vecResult[k].GetSmallestTP();
					int biggestTP  = launcher.m_vecResult[k].GetBiggestTP();
					int tpGap = biggestTP - smallestTP;
					CHECK(0 < tpGap && tpGap <= ACCEPTABLE_TPGAP);
				}

				// 대기시간 오버로 그룹지어진 경우는 그룹이 적절한지 확인할 방법이 없네.. -_-
			}
		}
	}

	mlog("\n<the biggest tp gap match>\n");
	launcher.LogMatch(launcher.m_biggestTpGapResult);
}

// CleanDisabledUid를 하더라도, 매칭되는 순간에 나가버리거나 하는 유저는 .. 어떻게 처리될 것인가 ? 부전승이 되는 것인가. 확인해보자
// 만에 하나 서버가 죽을만한 연산은 없는가? 코드를 다시 한번 보자
// 맨앞과 맨뒤가 소외당해서 점수차이 큰 유저와 붙게 되지 않을지 확인해보자
// 지금까지 수행속도를 고려하지 않고 테스트를 통과하는데만 집중했다. 이것이 서버의 일부라는 조건에 걸맞는 구현으로 개선해야한다.

#endif //_DEBUG
