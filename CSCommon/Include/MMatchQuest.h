#ifndef _MMATCHQUEST_H
#define _MMATCHQUEST_H

#include "MBaseQuest.h"
#include "MSurvivalRankInfo.h"

/// 서버용 퀘스트 최고 관장 클래스
class MMatchQuest : public MBaseQuest
{
protected:
	virtual bool OnCreate();				///< 초기화
	virtual void OnDestroy();				///< 해제
	

	MQuestNPCSetCatalogue		m_NPCSetCatalogue;						///< NPC Set 정보
	MQuestScenarioCatalogue		m_ScenarioCatalogue;					///< 퀘스트 시나리오 정보
	MQuestScenarioCatalogue		m_SurvivalScenarioCatalogue;			///< 서바이벌 시나리오 정보
	MSurvivalRankInfo			m_SurvivalRankInfo;						///< 서바이벌 상위권 랭킹 목록

public:
	MMatchQuest();														///< 생성자
	virtual ~MMatchQuest();												///< 소멸자

	inline MQuestNPCSetInfo* GetNPCSetInfo(int nID);					///< NPC Set 정보 반환
	inline MQuestNPCSetInfo* GetNPCSetInfo(const char* szName);			///< NPC Set 정보 반환
	inline MQuestScenarioCatalogue* GetScenarioCatalogue();				///< 퀘스트 시나리오 정보 반환
	inline MQuestScenarioInfo*		GetScenarioInfo(int nScenarioID);	///< 퀘스트 시나리오 정보 반환
	inline MQuestScenarioCatalogue* GetSurvivalScenarioCatalogue();				///< 서바이벌 시나리오 정보 반환
	inline MQuestScenarioInfo*		GetSurvivalScenarioInfo(int nScenarioID);	///< 서바이벌 시나리오 정보 반환
	inline MSurvivalRankInfo*		GetSurvivalRankInfo();						///< 서바이벌 랭킹 목록 반환

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