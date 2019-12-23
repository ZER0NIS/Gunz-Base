#include "stdafx.h"
#include "MDuelTournamentMatchMaker.h"

MDuelTournamentMatchMaker::MDuelTournamentMatchMaker()
: m_pMatchObjectContainer(NULL)
{
}


MDuelTournamentMatchMaker::MDuelTournamentMatchMaker(MMatchObjectContainer* moc)
: m_pMatchObjectContainer(moc)
{
}

MDuelTournamentMatchMaker::~MDuelTournamentMatchMaker()
{
	RemoveAll();
}

bool MDuelTournamentMatchMaker::AddPlayer( const MUID& uid, int tp, DWORD curTime )
{
	if (uid.IsInvalid()) return false;

	// 이진검색으로 바꿀 수 있을 것 같다..
	for (ItorDTUser it=m_mapUser.begin(); it!=m_mapUser.end(); ++it)
		if (it->first.uid == uid) return false;

	m_mapUser.insert(PairDTUser(DTUser(uid, tp), curTime));

	return true;
}

bool MDuelTournamentMatchMaker::RemovePlayer(const MUID& uid)
{
	// 이진검색으로 바꿀 수 있을 것 같다..
	for (ItorDTUser it=m_mapUser.begin(); it!=m_mapUser.end(); ++it) {
		if (it->first.uid == uid) {
			m_mapUser.erase(it);
			return true;
		}
	}
	return false;
}

void MDuelTournamentMatchMaker::RemoveAll()
{
	m_mapUser.clear();
}

void MDuelTournamentMatchMaker::PickMatchableGroups(vector<PairDTMatch>& out_vecMatchGroup, int numPick, int acceptableTpGap)
{
	out_vecMatchGroup.clear();

	if (numPick <= 1) return;
	if (acceptableTpGap < 0) return;
	int numPlayer = (int)m_mapUser.size();
	if (numPick > numPlayer) return;
	
	ItorDTUser itMin = m_mapUser.begin();	// 그룹내 최저점수 유저
	ItorDTUser itMax = m_mapUser.begin();	// 그룹내 최고점수 유저
	for (int i=0; i<numPick-1; ++i) ++itMax;

	int numGroup = numPlayer-numPick + 1;
	int tpGap = 0;
	// TP정렬된 맵을 앞에서부터 순회하면서 이웃한 유저끼리 TP격차가 허용치를 만족하면 그 그룹의 최저점수 유저의 itor를 담는다
	while (itMax != m_mapUser.end())
	{
		tpGap = itMax->first.tp - itMin->first.tp;
		if (tpGap <= acceptableTpGap)
		{
			out_vecMatchGroup.push_back(PairDTMatch(itMin->first, itMax->first));
			
			// numPick명의 사람들을 그룹에 넣었으니 itor를 전진시켜서 그다음 사람부터 검사를 계속
			for (int i=0; i<numPick; ++i)
			{
				++itMin;
				++itMax;
			}
		}
		else
		{
			++itMin;
			++itMax;
		}
	}
}

bool MDuelTournamentMatchMaker::PickGroupForPlayer(PairDTMatch& out_MatchGroup, int numPick, const DTUser& dtUser)
{
	if (numPick <= 1) return false;
	if (int(m_mapUser.size()) < numPick) return false;

	ItorDTUser itUser = m_mapUser.find(dtUser);
	if (itUser == m_mapUser.end()) return false;

	ItorDTUser lastTP = --m_mapUser.end();	// <- 위의 조건검사를 통과했다면 m_mapUser.size()>=1이 보장되므로 안전함

	/// 이 유저에 인접한 유저들과 가장 TP차이가 작은 그룹을 뽑아내야 한다

	// itor 두개로 그룹의 최저점이 될 유저, 최고점이 될 유저를 나타낸다
	ItorDTUser itMin = itUser;
	for (int i=0; i<numPick-1; ++i) {
		if (itMin == m_mapUser.begin()) break;
		--itMin;
	}
	ItorDTUser itMax = itMin;
	for (int i=0; i<numPick-1; ++i) {
		if (itMax == lastTP) break;
		++itMax;
	}

	// itor 두개로 이 유저가 들어갈 수 있는 그룹들을 예상해보고, 그 그룹의 TP차이를 평가해본다
	std::vector<GroupEval> vecGroupEval;
	GroupEval ge;
	for (int i=0; i<numPick; ++i) {
		ge.tpDiff = itMax->first.tp - itMin->first.tp;
		ge.group = PairDTMatch(itMin->first, itMax->first);
		vecGroupEval.push_back(ge);

		if (itMin == itUser) break;

		++itMin;
		++itMax;

		if (itMax == m_mapUser.end()) break;
	}

	// 가능한 그룹들 중 가장 TP차이가 작은 그룹을 찾자
	int indexBestGroup = -1;
	int evalBestGroup = INT_MAX;
	int size = (int)vecGroupEval.size();
	for (int i=0; i<size; ++i)
	{
		if (evalBestGroup > vecGroupEval[i].tpDiff) {
			evalBestGroup = vecGroupEval[i].tpDiff;
			indexBestGroup = i;
		}
	}

	if (evalBestGroup == INT_MAX) { // 이미 함수 초반에 조건 검사를 했는데 그룹이 단 한 개도 안만들어졌을 수는 없다
		mlog("duel tournament player match making - MakeGroupForPlayer fatal error!\n");
		//_ASSERT(0); 
		return false; 
	}

	out_MatchGroup = vecGroupEval[indexBestGroup].group;
	return true;
}

const MDuelTournamentMatchMaker::DTUser*
	MDuelTournamentMatchMaker::FindLongWaitPlayer(DWORD limitWaitTime, DWORD curTime)
{
	for (ItorDTUser it=m_mapUser.begin(); it!=m_mapUser.end(); ++it)
	{
		if ( (curTime - it->second) >= limitWaitTime)
			return &it->first;
	}
	return NULL;
}

void MDuelTournamentMatchMaker::RemovePlayers(const DTUser& begin, const DTUser& last)
{
	// remove users [begin, last]
	ItorDTUser it1 = m_mapUser.find(begin);
	if (it1 == m_mapUser.end()) return;
	ItorDTUser it2 = m_mapUser.find(last);
	if (it2 == m_mapUser.end()) return;
	++it2;

	m_mapUser.erase(it1, it2);
}

void MDuelTournamentMatchMaker::CleanDisabledUid()
{
	if (!m_pMatchObjectContainer) {
		//_ASSERT(0);
		return;
	}

	for (ItorDTUser it=m_mapUser.begin(); it!=m_mapUser.end(); )
	{
		if (false == m_pMatchObjectContainer->IsEnabledUid(it->first.uid))
			it = m_mapUser.erase(it);
		else
			++it;
	}
}

void MDuelTournamentMatchMaker::PickMatchableGroupsAndRemove( MDuelTournamentPickedGroup& out_matchGroup, int numPick, int acceptableTpGap )
{
	vector<MMockDTMatchMaker::PairDTMatch> vecMatchGroup;
	PickMatchableGroups(vecMatchGroup, numPick, acceptableTpGap);

	int numGroup = (int)vecMatchGroup.size();

	out_matchGroup.clear();
	out_matchGroup.reserve(numPick * numGroup);

	for (int i=0; i<numGroup; ++i)
	{
		ItorDTUser it = m_mapUser.find(vecMatchGroup[i].first);
		if (it == m_mapUser.end()) {
			mlog("duel tournament player match making - PickMatchableGroupsAndRemove fatal error!\n");
			continue;
		}

		for (int k=0; k<numPick; ++k) {
			out_matchGroup.push_back( it->first.uid );
			++it;
		}

		RemovePlayers(vecMatchGroup[i].first, vecMatchGroup[i].second);
	}
}

bool MDuelTournamentMatchMaker::PickGroupForPlayerAndRemove( MDuelTournamentPickedGroup& out_matchGroup, int numPick, const DTUser& dtUser )
{
	PairDTMatch group;
	bool result = PickGroupForPlayer(group, numPick, dtUser);
	if (false == result)
		return false;

	out_matchGroup.clear();
	out_matchGroup.reserve(numPick);

	ItorDTUser it = m_mapUser.find(group.first);
	if (it == m_mapUser.end()) {
		mlog("duel tournament player match making - PickGroupForPlayerAndRemove fatal error!\n");
		//_ASSERT(0);
		return false;
	}

	for (int i=0; i<numPick; ++i) {
		out_matchGroup.push_back( it->first.uid );
		++it;
	}

	RemovePlayers(group.first, group.second);

	return true;
}

void MDuelTournamentMatchMaker::ServiceTimeClose()
{
	for (ItorDTUser it=m_mapUser.begin(); it!=m_mapUser.end();++it)
	{
		DTUser dtUser = it->first;
		MMatchServer* pServer = MMatchServer::GetInstance();
		pServer->SendDuelTournamentServiceTimeClose(dtUser.uid);

		it = m_mapUser.erase(it);
	}
}