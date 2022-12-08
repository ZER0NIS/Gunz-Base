#include "stdafx.h"
#include "MMatchQuest.h"

#define FILENAME_NPC_DESC				"XML/Quest/npc.xml"
#define FILENAME_SCENARIO				"XML/Quest/scenario.xml"
#define FILENAME_SURVIVALSCENARIO		"XML/Quest/survivalScenario.xml"
#define FILENAME_QUESTMAP				"XML/Quest/questmap.xml"
#define FILENAME_SURVIVALMAP			"XML/Quest/survivalmap.xml"
#define FILENAME_NPCSET_DESC			"XML/Quest/npcset.xml"
#define FILENAME_DROPTABLE				"XML/Quest/droptable.xml"

MMatchQuest::MMatchQuest() : MBaseQuest()
{
}

MMatchQuest::~MMatchQuest()
{
}

bool MMatchQuest::OnCreate()
{
	if (!m_DropTable.ReadXml(FILENAME_DROPTABLE))
	{
		mlog("Droptable Read Failed");
		return false;
	}

	if (!m_NPCCatalogue.ReadXml(FILENAME_NPC_DESC))
	{
		mlog("Read NPC Catalogue Failed");
		return false;
	}

	ProcessNPCDropTableMatching();

	if (!m_NPCSetCatalogue.ReadXml(FILENAME_NPCSET_DESC))
	{
		mlog("Read NPCSet Catalogue Failed.");
		return false;
	}

	if (!m_ScenarioCatalogue.ReadXml(FILENAME_SCENARIO))
	{
		mlog("Read Scenario Catalogue Failed.");
		return false;
	}

	if (!m_MapCatalogue.ReadXml(FILENAME_QUESTMAP))
	{
		mlog("Read Questmap Catalogue Failed.");
		return false;
	}

	if (!m_SurvivalScenarioCatalogue.ReadXml(FILENAME_SURVIVALSCENARIO))
	{
		mlog("Read Survival-Scenario Catalogue Failed.");
		return false;
	}

	if (!m_SurvivalMapCatalogue.ReadXml(FILENAME_SURVIVALMAP))
	{
		mlog("Read Questmap Catalogue Failed.");
		return false;
	}

	MMatchServer::GetInstance()->OnRequestSurvivalModeGroupRanking();
	return true;
}

void MMatchQuest::OnDestroy()
{
}