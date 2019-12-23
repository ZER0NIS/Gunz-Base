#include "stdafx.h"
#include "MQuestLevel.h"
#include "MMatchTransDataType.h"
#include "MMath.h"
#include "MQuestFormula.h"
#include "MQuestNPC.h"
#include "MQuestConst.h"


MQuestNPCQueue::MQuestNPCQueue() : m_nCursor(0), m_bContainKeyNPC(false)
{

}

MQuestNPCQueue::~MQuestNPCQueue()
{

}

void MQuestNPCQueue::Make(int nQLD, MQuestNPCSetInfo* pNPCSetInfo, MQUEST_NPC nKeyNPC)
{
	if (pNPCSetInfo == NULL) return;

	m_nCursor = 0;
	int nSize = nQLD;

	m_Queue.reserve(nSize);			// QLD가 나올 NPC 개수를 의미한다.
	m_Queue.resize(nSize, pNPCSetInfo->nBaseNPC);

	int nNPCSetCount = (int)pNPCSetInfo->vecNPCs.size();
	int cursor = 0;
	for (int i = 0; i < nNPCSetCount; i++)
	{
		MNPCSetNPC npc = pNPCSetInfo->vecNPCs[i];

		//float fSpawnRate = (float)RandomNumber((int)(npc.fMinRate*100.0f), (int)(npc.fMaxRate*100.0f));
		float fSpawnRate = (float)(RandomNumber(npc.nMinRate, npc.nMaxRate) / 100.0f);
		int nSpawnCount = (int)floor(nSize * fSpawnRate);

		// 만약 비율상 1개도 안나오는 NPC가 있으면 1개라도 나올 수 있는 비율을 계산한다.
		if (nSpawnCount <= 0)
		{
			if (RandomNumber(0.0f, 1.0f) < float(nSize / 100.0f))
			{
				nSpawnCount = 1;
			}
		}
		if ((npc.nMaxSpawnCount > 0) && (nSpawnCount > npc.nMaxSpawnCount))
			nSpawnCount = npc.nMaxSpawnCount;

		for (int j = 0; j < nSpawnCount; j++)
		{
			if (cursor < nSize)
			{
				m_Queue[cursor] = npc.nNPC;
				cursor++;
			}
		}
	}

	// shuffle
	for (int i = 0; i < nSize; i++)
	{
		int nTarIndex = RandomNumber(i, nSize-1);
		MQUEST_NPC temp = m_Queue[nTarIndex];
		m_Queue[nTarIndex] = m_Queue[i];
		m_Queue[i] = temp;
	}

	// 키 NPC가 있으면 제일 처음에 넣는다.
	if (nKeyNPC != NPC_NONE)
	{
		m_bContainKeyNPC = true;
		m_Queue[0] = nKeyNPC;
	}
	else
		m_bContainKeyNPC = false;
}

bool MQuestNPCQueue::Pop(MQUEST_NPC& outNPC)
{
	if (IsEmpty()) return false;

	outNPC = m_Queue[m_nCursor];
	m_nCursor++;

	return true;
}

bool MQuestNPCQueue::GetFirst(MQUEST_NPC& outNPC)
{
	if (IsEmpty()) return false;
	outNPC = m_Queue[m_nCursor];

	return true;
}

void MQuestNPCQueue::Clear()
{
	m_Queue.clear();
	m_nCursor = 0;
}

bool MQuestNPCQueue::IsEmpty()
{
	if ((m_Queue.empty()) || (m_nCursor >= GetCount())) return true;
	return false;
}

int MQuestNPCQueue::GetCount()
{
	return (int)m_Queue.size();
}

bool MQuestNPCQueue::IsKeyNPC(MQUEST_NPC npc)
{
	if (!IsEmpty() && IsContainKeyNPC()) // Make()에서 큐 첫번째 칸에 KeyNPC를 넣었으므로 그것과 비교
		return npc==m_Queue[0];

	return false;
}

//////////////////////////////////////////////////////////////////////////////////
MQuestLevel::MQuestLevel()
{

}

MQuestLevel::~MQuestLevel()
{

}

const bool MQuestLevel::Make_MTDQuestGameInfo(MTD_QuestGameInfo* pout, MMATCH_GAMETYPE eGameType)
{
	if( MAX_QUEST_NPC_INFO_COUNT <= static_cast<int>(m_StaticInfo.NPCs.size()) )
	{
		//_ASSERT( 0 );
		return false;
	}

	if( MAX_QUEST_MAP_SECTOR_COUNT <= static_cast<int>(m_StaticInfo.SectorList.size()) )
	{
		//_ASSERT( 0 );
		return false;
	}

	
	pout->nNPCInfoCount = (int)m_StaticInfo.NPCs.size();
	int idx = 0;
	for (set<MQUEST_NPC>::iterator itor = m_StaticInfo.NPCs.begin(); itor != m_StaticInfo.NPCs.end(); ++itor)
	{
		pout->nNPCInfo[idx] = (*itor);
		idx++;
	}

	pout->nMapSectorCount = (int)m_StaticInfo.SectorList.size();
	for (int i = 0; i < pout->nMapSectorCount; i++)
	{
		pout->nMapSectorID[i]			= m_StaticInfo.SectorList[i].nSectorID;
		pout->nMapSectorLinkIndex[i]	= m_StaticInfo.SectorList[i].nNextLinkIndex;
	}

	pout->nNPCCount = (unsigned short)m_NPCQueue.GetCount();
	pout->fNPC_TC	= m_StaticInfo.fNPC_TC;
	pout->nQL		= m_StaticInfo.nQL;
	pout->eGameType = eGameType;
	pout->nRepeat	= m_StaticInfo.pScenario->nRepeat;

	return true;
}


void MQuestLevel::Init(int nScenarioID, int nDice, MMATCH_GAMETYPE eGameType)
{
	MQuestScenarioInfo* pScenario = NULL;
	MMatchQuest* pQuest = MMatchServer::GetInstance()->GetQuest();

	if (MGetGameTypeMgr()->IsQuestOnly(eGameType))
	{
		pScenario = MMatchServer::GetInstance()->GetQuest()->GetScenarioInfo(nScenarioID);
	}
	else if (MGetGameTypeMgr()->IsSurvivalOnly(eGameType))
	{
		pScenario = MMatchServer::GetInstance()->GetQuest()->GetSurvivalScenarioInfo(nScenarioID);
	}
	else
	{
		ASSERT(0);
		return;
	}

	m_StaticInfo.pScenario = pScenario;

	m_StaticInfo.nDice = nDice - 1;		// 0부터 5까지

	if (m_StaticInfo.pScenario)
	{
		InitSectors(eGameType);
		InitNPCs();
	}

	m_DynamicInfo.nCurrSectorIndex = 0;

	InitStaticInfo(eGameType);	// 난이도 상수, NPC 난이도 조절 계수 등을 설정
	
#ifdef _DEBUG_QUEST
	if( nScenarioID == 100)
		m_StaticInfo.nQLD = 1;
#endif
	
	InitCurrSector(eGameType);
}

bool MQuestLevel::InitSectors(MMATCH_GAMETYPE eGameType)
{
	if (m_StaticInfo.pScenario == NULL) 
	{
		//_ASSERT(0);
		return false;
	}

	MMatchQuest* pQuest = MMatchServer::GetInstance()->GetQuest();

	int nSectorCount = m_StaticInfo.pScenario->GetSectorCount(m_StaticInfo.nDice);
	int nKeySector = m_StaticInfo.pScenario->Maps[m_StaticInfo.nDice].nKeySectorID;


	m_StaticInfo.SectorList.reserve(nSectorCount);
	m_StaticInfo.SectorList.resize(nSectorCount);

	int nSectorIndex = nSectorCount-1;

	int nSectorID = nKeySector;
	int nLinkIndex = 0;
	for (int i = 0; i < nSectorCount; i++)
	{
		MQuestMapSectorInfo* pSector = NULL;

		if (MGetGameTypeMgr()->IsQuestOnly(eGameType))
			pSector = pQuest->GetSectorInfo(nSectorID);
		else if (MGetGameTypeMgr()->IsSurvivalOnly(eGameType))
			pSector = pQuest->GetSurvivalSectorInfo(nSectorID);
		else
			ASSERT(0);

		if (pSector == NULL) 
		{
			//_ASSERT(0);
			return false;
		}

		// 섹터 정보 입력
		MQuestLevelSectorNode node;
		node.nSectorID = nSectorID;
		node.nNextLinkIndex = nLinkIndex;
		m_StaticInfo.SectorList[nSectorIndex] = node;


		if (i != (nSectorCount-1)) 
		{
			// 현재 섹터노드를 바탕으로 이전 섹터와 링크를 결정한다.
			int nBacklinkCount = (int)pSector->VecBacklinks.size();
			if (nBacklinkCount > 0)
			{
				bool bSameNode = false;
				int nLoopCount = 0;
				do
				{
					nLoopCount++;

					int backlink_index = RandomNumber(0, (nBacklinkCount-1));
					nSectorID = pSector->VecBacklinks[backlink_index].nSectorID;
					nLinkIndex = pSector->VecBacklinks[backlink_index].nLinkIndex;

					// 같은 노드가 두번 반복해서 걸리지 않도록 한다.
					if ((nBacklinkCount > 1) && ((nSectorIndex+1) < nSectorCount))
					{
						if (nSectorID == m_StaticInfo.SectorList[nSectorIndex+1].nSectorID)
						{
							bSameNode = true;
						}
					}
				}
				while ((bSameNode) && (nLoopCount < 2));	// 이전 노드랑 같은 노드가 걸리면 반복

			}
			else
			{
				// 역링크가 하나라도 있어야 한다.
				//_ASSERT(0);
				return false;
			}

			nSectorIndex--;
		}
	}

	return true;
}

bool MQuestLevel::InitNPCs()
{
	if (m_StaticInfo.pScenario == NULL) 
	{
		//_ASSERT(0);
		return false;
	}

	int nDice = m_StaticInfo.nDice;
	MMatchQuest* pQuest = MMatchServer::GetInstance()->GetQuest();
	int nArraySize = (int)m_StaticInfo.pScenario->Maps[nDice].vecNPCSetArray.size();
	for (int i = 0; i < nArraySize; i++)
	{
		int nNPCSetID = m_StaticInfo.pScenario->Maps[nDice].vecNPCSetArray[i];
		MQuestNPCSetInfo* pNPCSetInfo = pQuest->GetNPCSetInfo(nNPCSetID);
		if (pNPCSetInfo == NULL) 
		{
			//_ASSERT(0);
			return false;
		}
		
		// base npc는 따로 넣는다.
		m_StaticInfo.NPCs.insert(set<MQUEST_NPC>::value_type(pNPCSetInfo->nBaseNPC));

		int nNPCSize = (int)pNPCSetInfo->vecNPCs.size();
		for (int j = 0; j < nNPCSize; j++)
		{
			MQUEST_NPC npc = (MQUEST_NPC)pNPCSetInfo->vecNPCs[j].nNPC;
			m_StaticInfo.NPCs.insert(set<MQUEST_NPC>::value_type(npc));
		}
	}

	
	return true;
}


int MQuestLevel::GetMapSectorCount()
{
	return (int)m_StaticInfo.SectorList.size();
}


bool MQuestLevel::MoveToNextSector(MMATCH_GAMETYPE eGameType)
{
	if (MGetGameTypeMgr()->IsQuestOnly(eGameType))
	{
		if ((m_DynamicInfo.nCurrSectorIndex+1) >= GetMapSectorCount()) return false;
	
		MMatchQuest* pQuest = MMatchServer::GetInstance()->GetQuest();

		m_DynamicInfo.nCurrSectorIndex++;
	}
	else if (MGetGameTypeMgr()->IsSurvivalOnly(eGameType))
	{
		m_DynamicInfo.nCurrSectorIndex++;

		if (m_DynamicInfo.nCurrSectorIndex >= GetMapSectorCount())
		{
			m_DynamicInfo.nCurrSectorIndex = 0;
			m_DynamicInfo.nRepeated++;
		}
	}
	else
	{
		//_ASSERT(0);
		return false;
	}

	InitCurrSector(eGameType);
	return true;
}

void MQuestLevel::InitCurrSector(MMATCH_GAMETYPE eGameType)
{
	// npc queue 세팅
	MMatchQuest* pQuest = MMatchServer::GetInstance()->GetQuest();
	
	int nNPCSetID = 0;

	if( m_DynamicInfo.nCurrSectorIndex < (int)m_StaticInfo.pScenario->Maps[m_StaticInfo.nDice].vecNPCSetArray.size() )
	{
		nNPCSetID = m_StaticInfo.pScenario->Maps[m_StaticInfo.nDice].vecNPCSetArray[m_DynamicInfo.nCurrSectorIndex];
	}
	else
	{
		//_ASSERT( 0 && "NPC set의 크기에 문제가 있음. 리소스 검사가 필요함." );
		mlog( "NPC set의 크기에 문제가 있음. 리소스 검사가 필요함.\n" );
		return;
	}

	MQuestNPCSetInfo* pNPCSetInfo = pQuest->GetNPCSetInfo(nNPCSetID);

	m_NPCQueue.Clear();

	m_DynamicInfo.bCurrBossSector = false;

	if (MGetGameTypeMgr()->IsQuestOnly(eGameType))
	{
		// 만약 키 NPC가 있고, 마지막 섹터이면 키 NPC 세팅
		if ((m_StaticInfo.pScenario->Maps[m_StaticInfo.nDice].nKeyNPCID != 0) &&
			(m_DynamicInfo.nCurrSectorIndex == GetMapSectorCount() - 1))
		{
			m_NPCQueue.Make(m_StaticInfo.nQLD, pNPCSetInfo, MQUEST_NPC(m_StaticInfo.pScenario->Maps[m_StaticInfo.nDice].nKeyNPCID));
			if (m_StaticInfo.pScenario->Maps[m_StaticInfo.nDice].bKeyNPCIsBoss)
			{
				m_DynamicInfo.bCurrBossSector = true;
			}
		}
		else
		{
			m_NPCQueue.Make(m_StaticInfo.nQLD, pNPCSetInfo);
		}
	}
	else if (MGetGameTypeMgr()->IsSurvivalOnly(eGameType))
	{
		const int nCurrSector = m_DynamicInfo.nCurrSectorIndex;
		const std::vector<int>& vecKeyNpc = m_StaticInfo.pScenario->Maps[m_StaticInfo.nDice].vecKeyNPCArray;

		// 이 섹터의 키NPC를 세팅
		int keyNpcID = NPC_NONE;
		if (nCurrSector < (int)vecKeyNpc.size())
		{
			keyNpcID = vecKeyNpc[nCurrSector];
		}

		if (keyNpcID != NPC_NONE)
		{
			m_NPCQueue.Make(m_StaticInfo.nQLD, pNPCSetInfo, MQUEST_NPC(keyNpcID));
			m_DynamicInfo.bCurrBossSector = false;	// 서바이벌엔 보스가 없다(몹은 보스용 몹이지만 퀘스트에서의 보스처럼 다루지 않음)
		}
		else
		{
			m_NPCQueue.Make(m_StaticInfo.nQLD, pNPCSetInfo);
		}
	}

	// spawn index 세팅
	memset(m_SpawnInfos, 0, sizeof(m_SpawnInfos));

	int nSectorID = m_StaticInfo.SectorList[m_DynamicInfo.nCurrSectorIndex].nSectorID;

	if (MGetGameTypeMgr()->IsQuestOnly(eGameType))
		m_DynamicInfo.pCurrSector = pQuest->GetSectorInfo(nSectorID);
	else if (MGetGameTypeMgr()->IsSurvivalOnly(eGameType))
		m_DynamicInfo.pCurrSector = pQuest->GetSurvivalSectorInfo(nSectorID);
}

void MQuestLevel::InitStaticInfo(MMATCH_GAMETYPE eGameType)
{
	if (m_StaticInfo.pScenario)
	{
		m_StaticInfo.nQL = m_StaticInfo.pScenario->nQL;

		if (MGetGameTypeMgr()->IsQuestOnly(eGameType))
		{
			m_StaticInfo.nQLD = (int)(MQuestFormula::CalcQLD(m_StaticInfo.nQL) * m_StaticInfo.pScenario->fDC);
			m_StaticInfo.nLMT = (int)(MQuestFormula::CalcLMT(m_StaticInfo.nQL) * m_StaticInfo.pScenario->fDC);
		}
		else if (MGetGameTypeMgr()->IsSurvivalOnly(eGameType))
		{
			// 서바이벌 모드의 경우 xml에 정의된 값을 사용
			m_StaticInfo.nQLD = m_StaticInfo.pScenario->nMaxSpawn;				// 최대 스폰 수
			m_StaticInfo.nLMT = m_StaticInfo.pScenario->nMaxSpawnSameTime;		// 최대 동시 스폰 수
		}
		else 
			ASSERT(0);

		m_StaticInfo.fNPC_TC = MQuestFormula::CalcTC(m_StaticInfo.nQL);

	}
}

int MQuestLevel::GetCurrSectorIndex()
{
	return m_DynamicInfo.nCurrSectorIndex;
}

int MQuestLevel::GetSpawnPositionCount(MQuestNPCSpawnType nSpawnType)
{
	if (m_DynamicInfo.pCurrSector)
	{
		return m_DynamicInfo.pCurrSector->nSpawnPointCount[nSpawnType];
	}

	return 0;
}

int MQuestLevel::GetRecommendedSpawnPosition(MQuestNPCSpawnType nSpawnType, unsigned long int nTickTime)
{
	if (m_DynamicInfo.pCurrSector)
	{
		// 서바이벌모드를 위한 예외 처리- 보스스폰지점이 없는 맵은 그냥 밀리지점에 스폰시킴 //////
		if (nSpawnType == MNST_BOSS)
		{
			if (m_DynamicInfo.pCurrSector->nSpawnPointCount[MNST_BOSS] == 0)
				nSpawnType = MNST_MELEE;
		}
		//////////////////////////////////////////////////////////////////////////////////////////

		int nRecommendIndex = m_SpawnInfos[nSpawnType].nIndex;

		// 스폰포지션을 추천받았을때 스폰시간도 세팅한다.
		if (nRecommendIndex < MAX_SPAWN_COUNT)
		{
			m_SpawnInfos[nSpawnType].nRecentSpawnTime[nRecommendIndex] = nTickTime;
		}

		m_SpawnInfos[nSpawnType].nIndex++;

		int nSpawnMax = m_DynamicInfo.pCurrSector->nSpawnPointCount[nSpawnType];
		if (m_SpawnInfos[nSpawnType].nIndex >= nSpawnMax) m_SpawnInfos[nSpawnType].nIndex = 0;

		return nRecommendIndex;
	}

	return 0;
}

bool MQuestLevel::IsEnableSpawnNow(MQuestNPCSpawnType nSpawnType, unsigned long int nNowTime)
{
	if (m_DynamicInfo.pCurrSector)
	{
		int idx = m_SpawnInfos[nSpawnType].nIndex;
		if ((nNowTime - m_SpawnInfos[nSpawnType].nRecentSpawnTime[idx]) > SAME_SPAWN_DELAY_TIME) return true;
	}

	return false;
}

void MQuestLevel::OnItemCreated(unsigned long int nItemID, int nRentPeriodHour)
{
	MQuestLevelItem* pNewItem = new MQuestLevelItem();
	pNewItem->nItemID = nItemID;
	pNewItem->nRentPeriodHour = nRentPeriodHour;
	pNewItem->bObtained = false;

	m_DynamicInfo.ItemMap.insert(make_pair(nItemID, pNewItem));
}

bool MQuestLevel::OnItemObtained( MMatchObject* pPlayer, unsigned long int nItemID )
{
	if( 0 == pPlayer ) return false;

	for (MQuestLevelItemMap::iterator itor = m_DynamicInfo.ItemMap.lower_bound(nItemID);
		itor != m_DynamicInfo.ItemMap.upper_bound(nItemID); ++itor)
	{
		MQuestLevelItem* pQuestItem = (*itor).second;
		if (!pQuestItem->bObtained)
		{
			pQuestItem->bObtained = true;
			// pPlayer->GetCharInfo()->m_QMonsterBible.SetPage( 
			return true;
		}
	}
	
	// 만약 false이면 플레이어가 치팅을 하는 것임..-_-;
	return false;
}