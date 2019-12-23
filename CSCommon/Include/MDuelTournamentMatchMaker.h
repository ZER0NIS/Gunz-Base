#ifndef _MDUELTOURNAMENT_MATCHMAKER_H
#define _MDUELTOURNAMENT_MATCHMAKER_H

#include "MDuelTournamentGroup.h"

//////////////////////////////////////////////////////////////////////
// Duel Tournament의 Match-Up을 시켜주는 MatchMaker 클래스이다.
// 
// 기본적으로 TP를 비교해서 허용 차이 이내이면 매치를 시키고,
// 비슷한 TP의 타 유저가 없으면 그런 유저가 나타날 때까지 기다린다
// 대기 시간이 일정 시간이상 길어지면 그만 기다리고 가능한 가까운 TP의 유저와 매치된다.
//////////////////////////////////////////////////////////////////////

// 단위테스트 작성을 위한 래핑 클래스 : MatchObject에서 정보를 가져오는 용도
class MMatchObjectContainer
{
public:
	// playerUID -> MMatchObject -> DuelTournamentCharInfo 까지의 얻어오는 과정을 래핑
	virtual MMatchObjectDuelTournamentCharInfo* GetDuelTournamentCharInfo(const MUID& uid) { 
		MMatchObject* pObj = MMatchServer::GetInstance()->GetObject(uid);
		return pObj ? pObj->GetDuelTournamentCharInfo() : NULL;
	}

	// uidPlayer가 가리키는 MatchObject가 유효한지 검사하는 과정을 래핑
	virtual bool IsEnabledUid(const MUID& uid) {
		MMatchObject* pObj = MMatchServer::GetInstance()->GetObject(uid);
		return IsEnabledObject(pObj);
	}
};

class MDuelTournamentMatchMaker
{
public:

	struct DTUser {
		MUID uid;	// player uid
		int tp;		// tournament point

		DTUser() : uid(0,0), tp(0) {}
		DTUser(const MUID& uid_, int tp_) : uid(uid_), tp(tp_) {}

		bool operator <(const DTUser &o) const {
			if (tp < o.tp) return true;
			if (tp == o.tp && uid < o.uid) return true;
			return false;
		}
	};

protected:
	MMatchObjectContainer* m_pMatchObjectContainer;

	typedef map<DTUser, int>	MapDTUser;
	typedef MapDTUser::iterator	ItorDTUser;
	typedef pair<DTUser, int>	PairDTUser;
	typedef pair<DTUser,DTUser>	PairDTMatch;	// MapDTUser에서 매치그룹을 나타내는 키쌍 (그룹의 첫째 유저키, 마지막 유저키)

	MapDTUser m_mapUser;

	struct GroupEval { int tpDiff; PairDTMatch group; };

public:

	MDuelTournamentMatchMaker();
	MDuelTournamentMatchMaker(MMatchObjectContainer* moc);
	~MDuelTournamentMatchMaker();

	void SetMatchObjectContainer(MMatchObjectContainer* p) { m_pMatchObjectContainer = p; }

	bool AddPlayer(const MUID& uid, int tp, DWORD curTime);
	bool RemovePlayer(const MUID& uid);
	void RemoveAll();

	int GetNumPlayer() { return (int)m_mapUser.size(); }

	void PickMatchableGroupsAndRemove(MDuelTournamentPickedGroup& out_matchGroup, int numPick, int acceptableTpGap);
	bool PickGroupForPlayerAndRemove(MDuelTournamentPickedGroup& out_matchGroup, int numPick, const DTUser& dtUser);

	// uid가 무효가 된 대기자들을 제거한다 (매치시키기 함수들을 사용하기 전에 한번 실행할 것)
	void CleanDisabledUid();

	// 일정 시간 이상 대기한 유저를 찾아낸다
	const MDuelTournamentMatchMaker::DTUser* FindLongWaitPlayer(DWORD limitWaitTime, DWORD curTime);

	// 플레이어 범위 삭제 - 매칭된 플레이어 그룹을 대기자에서 뺄때 사용
	void RemovePlayers(const DTUser& begin, const DTUser& last);

	// 서비스 시간이 끝났을때 등록된 유저들을 삭제해준다.
	void ServiceTimeClose();

protected:
	// TP가 비슷한 유저끼리 그룹을 짓는다, 복수 그룹을 구성할 수 있다
	void PickMatchableGroups(vector<PairDTMatch>& out_vecMatchGroup, int numPick, int acceptableTpGap);

	// 특정 유저를 포함하는 가장 TP차이가 작은 그룹을 만들어낸다
	bool PickGroupForPlayer(PairDTMatch& out_MatchGroup, int numPick, const DTUser& dtUser);
};

// 단위테스트에서 내부를 까보기 위해 상속한 클래스
class MMockDTMatchMaker : public MDuelTournamentMatchMaker
{
public:
	using MDuelTournamentMatchMaker::MapDTUser;
	using MDuelTournamentMatchMaker::ItorDTUser;
	using MDuelTournamentMatchMaker::PairDTUser;
	using MDuelTournamentMatchMaker::PairDTMatch;

	using MDuelTournamentMatchMaker::PickMatchableGroups;
	using MDuelTournamentMatchMaker::PickGroupForPlayer;
	
	using MDuelTournamentMatchMaker::m_mapUser;

	MMockDTMatchMaker(MMatchObjectContainer* moc) : MDuelTournamentMatchMaker(moc) {}
};

#endif