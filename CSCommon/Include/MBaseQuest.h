#pragma once

#include "MQuestNPC.h"
#include "MQuestMap.h"
#include "MQuestScenario.h"
#include "MQuestDropTable.h"

#include <map>

struct MQuestNPCInfo;

struct MQuestLevelSectorNode
{
	int		nSectorID;
	int		nNextLinkIndex;
};

class MBaseQuest
{
private:
	bool m_bCreated;
protected:

	MQuestMapCatalogue			m_MapCatalogue;
	MQuestMapCatalogue			m_SurvivalMapCatalogue;
	MQuestNPCCatalogue			m_NPCCatalogue;
	MQuestDropTable				m_DropTable;
	virtual bool OnCreate();
	virtual void OnDestroy();

	void ProcessNPCDropTableMatching();
public:
	MBaseQuest();
	virtual ~MBaseQuest();
	inline MQuestMapSectorInfo* GetSectorInfo(int nSectorID);
	inline MQuestMapSectorInfo* GetSurvivalSectorInfo(int nSectorID);
	inline MQuestNPCInfo* GetNPCInfo(MQUEST_NPC nNPC);
	inline MQuestNPCInfo* GetNPCPageInfo(int nPage);
	inline MQuestDropTable* GetDropTable();
	bool Create();
	int GetNumOfPage(void) { return (int)m_NPCCatalogue.size(); }
	void Destroy();

	MQuestNPCInfo* GetNPCIndexInfo(int nMonsterBibleIndex)
	{
		return m_NPCCatalogue.GetIndexInfo(nMonsterBibleIndex);
	}

	inline MQuestMapCatalogue* GetMapCatalogue();
	inline MQuestMapCatalogue* GetSurvivalMapCatalogue();
	inline MQuestNPCCatalogue* GetNPCCatalogue();
};

inline MQuestMapSectorInfo* MBaseQuest::GetSectorInfo(int nSectorID)
{
	return m_MapCatalogue.GetSectorInfo(nSectorID);
}

inline MQuestMapSectorInfo* MBaseQuest::GetSurvivalSectorInfo(int nSectorID)
{
	return m_SurvivalMapCatalogue.GetSectorInfo(nSectorID);
}

inline MQuestNPCInfo* MBaseQuest::GetNPCInfo(MQUEST_NPC nNPC)
{
	return m_NPCCatalogue.GetInfo(nNPC);
}

inline MQuestNPCInfo* MBaseQuest::GetNPCPageInfo(int nPage)
{
	return m_NPCCatalogue.GetPageInfo(nPage);
}

inline MQuestDropTable* MBaseQuest::GetDropTable()
{
	return &m_DropTable;
}

inline MQuestMapCatalogue* MBaseQuest::GetMapCatalogue()
{
	return &m_MapCatalogue;
}

inline MQuestMapCatalogue* MBaseQuest::GetSurvivalMapCatalogue()
{
	return &m_SurvivalMapCatalogue;
}

inline MQuestNPCCatalogue* MBaseQuest::GetNPCCatalogue()
{
	return &m_NPCCatalogue;
}
