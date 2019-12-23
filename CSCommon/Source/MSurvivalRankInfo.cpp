#include "stdafx.h"
#include "MSurvivalRankInfo.h"

MSurvivalRankInfo::MSurvivalRankInfo()
{
}

MSurvivalRankInfo::~MSurvivalRankInfo()
{

}

void MSurvivalRankInfo::ClearRanking()
{
	for(int i = 0; i < MAX_SURVIVAL_SCENARIO_COUNT; i++) {
		for(int j = 0; j < MAX_SURVIVAL_RANKING_LIST; j++) {
			memset(m_arRanking[i][j].szCharacterName, 0, MATCHOBJECT_NAME_LENGTH);
			m_arRanking[i][j].dwRankPoint = 0;
			m_arRanking[i][j].dwRank = 0;
		}
	}
}

bool MSurvivalRankInfo::SetRanking( DWORD scenarioIndex, DWORD rankArrayIndex, DWORD realRank, const char* szCharName, DWORD rankPoint )
{
	if (scenarioIndex >= MAX_SURVIVAL_SCENARIO_COUNT) { ASSERT(0);  return false; }
	if (rankArrayIndex >= MAX_SURVIVAL_RANKING_LIST) { ASSERT(0);  return false; }
	if (szCharName == NULL) return false;

	strcpy(m_arRanking[scenarioIndex][rankArrayIndex].szCharacterName, szCharName);
	m_arRanking[scenarioIndex][rankArrayIndex].dwRankPoint = rankPoint;
	m_arRanking[scenarioIndex][rankArrayIndex].dwRank = realRank;	// 공동 순위가 있을 수 있으므로 실제 순위값은 여기에.

	return true;
}

const SurvivalRanking* MSurvivalRankInfo::GetRanking( DWORD scenarioIndex, DWORD rankIndex ) const
{
	if (scenarioIndex >= MAX_SURVIVAL_SCENARIO_COUNT) { ASSERT(0); return NULL; }
	if (rankIndex >= MAX_SURVIVAL_RANKING_LIST) { ASSERT(0); return NULL; }

	return &m_arRanking[scenarioIndex][rankIndex];
}

void MSurvivalRankInfo::FillDummyRankingListForDebug()
{
	for (int i=0; i<MAX_SURVIVAL_SCENARIO_COUNT; ++i)
		for (int k=0; k<MAX_SURVIVAL_RANKING_LIST-1; ++k)
			SetRanking(i, k, i+1, "TestName", 987654321);
}
