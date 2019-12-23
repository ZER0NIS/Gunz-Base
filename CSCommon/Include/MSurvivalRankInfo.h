#ifndef MSURVIVALRANKINFO_H
#define MSURVIVALRANKINFO_H

struct SurvivalRanking
{
	char szCharacterName[MATCHOBJECT_NAME_LENGTH];
	DWORD dwRankPoint;
	DWORD dwRank;

	SurvivalRanking() : dwRank(0), dwRankPoint(0) { szCharacterName[0]=0; }
};

class MSurvivalRankInfo
{
	// 시나리오별 상위 랭킹 리스트
	SurvivalRanking m_arRanking[MAX_SURVIVAL_SCENARIO_COUNT][MAX_SURVIVAL_RANKING_LIST];

public:
	MSurvivalRankInfo();
	~MSurvivalRankInfo();

	const SurvivalRanking* GetRanking(DWORD scenarioIndex, DWORD rankIndex) const;
	bool SetRanking(DWORD scenarioIndex, DWORD rankArrayIndex, DWORD realRank, const char* szCharName, DWORD rankPoint);

	void ClearRanking();
	
	void FillDummyRankingListForDebug();
};


#endif