#pragma once

#include "MMatchGlobal.h"
#include "MLadderGroup.h"
#include "MLadderStatistics.h"
#include <vector>
#include <list>

class MMatchObject;

struct LadderGameMapVoteInfo
{
	MLadderGroup* pGroupA;
	MLadderGroup* pGroupB;
	int Maps[3];
	int Votes[3];
	int SelectedMap;
	unsigned long int RegisterTime;
};

class MLadderMgr {
protected:
	int					m_idGenerate;
	unsigned long		m_nLastTick;

	MLadderGroupMap		m_WaitingMaps[MLADDERTYPE_MAX];
	std::list<MLadderGroup*>	m_GroupList;

	MLadderStatistics	m_Stat;
protected:
	unsigned long int counter;
	std::map<unsigned long int, LadderGameMapVoteInfo*> WaitingMapSelectionGames;

	unsigned long GetLastTick() { return m_nLastTick; }
	void SetLastTick(unsigned long nTick) { m_nLastTick = nTick; }

	void UpdateMapCountDown(unsigned long int NowTime);

	inline MLadderGroupMap* GetWaitGroupContainer(MLADDERTYPE nLadderType);

	void AddGroup(MLADDERTYPE nLadderType, MLadderGroup* pGroup);
	int MakeMatch(MLADDERTYPE nLadderType);
	void LaunchLadder(MLADDERTYPE nLadderType, int nGroupA, int nGroupB);
	void RemoveFromGroupList(MLadderGroup* pGroup);
	void CleaningGarbages();
	unsigned long int GetTickInterval();
public:
	MLadderMgr() {
		m_idGenerate = 0;
		counter = 0;
	}
	bool Init();
	MLadderGroup* CreateLadderGroup();
	MLadderGroup* FindLadderGroup(int nGroupID);
	bool Challenge(MLadderGroup* pGroup);
	void CancelChallenge(int nGroupID, const char* pszCancelName);

	void UpdatePlayerVote(int VoteID, MMatchObject* pObj);

	void Tick(unsigned long nTick);
	int GenerateID() { return ++m_idGenerate; }
	int GetNeedMemberCount(MLADDERTYPE nLadderType);
	int GetTotalGroupCount();
	unsigned long GetChecksum(int nFirstIndex, int nGroupCount);

	auto GetGroupListBegin() { return m_GroupList.begin(); }
	auto GetGroupListEnd() { return m_GroupList.end(); }
	size_t GetGroupCount() { return m_GroupList.size(); }

	MLadderStatistics* GetStatistics() { return &m_Stat; }
};
