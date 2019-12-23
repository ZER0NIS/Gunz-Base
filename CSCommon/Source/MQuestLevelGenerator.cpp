#include "stdafx.h"
#include "MQuestLevelGenerator.h"
#include "MQuestLevel.h"
#include "MMath.h"
#include "MMatchServer.h"
//#include "MSacrificeQItemTable.h"

///////////////////////////////////////////////////////////////////////////////
MQuestLevelGenerator::MQuestLevelGenerator(MMATCH_GAMETYPE eGameType) : 
m_eGameType(eGameType),
m_nPlayerQL(0), m_nMapsetID(-1), m_nScenarioID(-1)
{
	memset(m_nSacriQItemID, 0, sizeof(unsigned int) * MAX_SCENARIO_SACRI_ITEM);
	
	ASSERT(MGetGameTypeMgr()->IsQuestDerived(m_eGameType));
}

MQuestLevelGenerator::~MQuestLevelGenerator()
{

}

void MQuestLevelGenerator::BuildPlayerQL(int nPlayerQL)
{
	m_nPlayerQL = nPlayerQL;
}

void MQuestLevelGenerator::BuildMapset(int nMapsetID)
{
	m_nMapsetID = nMapsetID;
}

void MQuestLevelGenerator::BuildSacriQItem(unsigned int nItemID)
{
	for (int i = 0; i < MAX_SACRIFICE_SLOT_COUNT; i++)
	{
		if ((m_nSacriQItemID[i] == 0) || (i == (MAX_SCENARIO_SACRI_ITEM-1)))
		{
			m_nSacriQItemID[i] = nItemID;
			break;
		}
	}
}

MQuestLevel* MQuestLevelGenerator::MakeLevel()
{
	// 시나리오 결정
	m_nScenarioID = MakeScenarioID();


	// 주사위 굴림
	int dice = (int)Dice(1, SCENARIO_STANDARD_DICE_SIDES, 0);

	MQuestLevel* pNewLevel = new MQuestLevel();
	pNewLevel->Init(m_nScenarioID, dice, m_eGameType);

	return pNewLevel;
}


int MQuestLevelGenerator::MakeScenarioID()
{
	MQuestScenarioCatalogue* pScenarioCatalog = NULL;
	if (MGetGameTypeMgr()->IsQuestOnly(m_eGameType))
	{
		pScenarioCatalog = MMatchServer::GetInstance()->GetQuest()->GetScenarioCatalogue();
	}
	else if (MGetGameTypeMgr()->IsSurvivalOnly(m_eGameType))
	{
		pScenarioCatalog = MMatchServer::GetInstance()->GetQuest()->GetSurvivalScenarioCatalogue();
	}
	else
	{
		ASSERT(0);
		return 0;
	}

	int id = pScenarioCatalog->MakeScenarioID(m_nMapsetID, m_nPlayerQL, m_nSacriQItemID);

	// 시나리오가 없으면 기본 시나리오.
	if (!pScenarioCatalog->GetInfo(id))
	{
		id = pScenarioCatalog->GetDefaultStandardScenarioID();
	}

	return id;
}

int MQuestLevelGenerator::ReturnScenarioID()
{
	return MakeScenarioID();
}




