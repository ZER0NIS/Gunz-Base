#ifndef _MMATCHQUEST_H
#define _MMATCHQUEST_H

#include "MBaseQuest.h"
#include "MSurvivalRankInfo.h"

class MMatchQuest : public MBaseQuest
{
protected:
	virtual bool OnCreate();
	virtual void OnDestroy();

	MQuestNPCSetCatalogue		m_NPCSetCatalogue;
	MQuestScenarioCatalogue		m_ScenarioCatalogue;
	MQuestScenarioCatalogue		m_SurvivalScenarioCatalogue;
	MSurvivalRankInfo			m_SurvivalRankInfo;

public:
	MMatchQuest();
	virtual ~MMatchQuest();

	inline MQuestNPCSetInfo* GetNPCSetInfo(int nID);
	inline MQuestNPCSetInfo* GetNPCSetInfo(const char* szName);
	inline MQuestScenarioCatalogue* GetScenarioCatalogue();
	inline MQuestScenarioInfo* GetScenarioInfo(int nScenarioID);
	inline MQuestScenarioCatalogue* GetSurvivalScenarioCatalogue();
	inline MQuestScenarioInfo* GetSurvivalScenarioInfo(int nScenarioID);
	inline MSurvivalRankInfo* GetSurvivalRankInfo();
};

inline MQuestNPCSetInfo* MMatchQuest::GetNPCSetInfo(int nID)
{
	return m_NPCSetCatalogue.GetInfo(nID);
}

inline MQuestNPCSetInfo* MMatchQuest::GetNPCSetInfo(const char* szName)
{
	return m_NPCSetCatalogue.GetInfo(szName);
}

inline MQuestScenarioCatalogue* MMatchQuest::GetScenarioCatalogue()
{
	return &m_ScenarioCatalogue;
}

inline MQuestScenarioInfo* MMatchQuest::GetScenarioInfo(int nScenarioID)
{
	return m_ScenarioCatalogue.GetInfo(nScenarioID);
}

inline MQuestScenarioCatalogue* MMatchQuest::GetSurvivalScenarioCatalogue()
{
	return &m_SurvivalScenarioCatalogue;
}

inline MQuestScenarioInfo* MMatchQuest::GetSurvivalScenarioInfo(int nScenarioID)
{
	return m_SurvivalScenarioCatalogue.GetInfo(nScenarioID);
}

inline MSurvivalRankInfo* MMatchQuest::GetSurvivalRankInfo()
{
	return &m_SurvivalRankInfo;
}

#endif