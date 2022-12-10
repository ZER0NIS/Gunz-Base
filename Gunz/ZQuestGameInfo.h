#ifndef _ZQUEST_GAMEINFO_H
#define _ZQUEST_GAMEINFO_H

#include "MBaseQuest.h"
#include "MQuestLevel.h"
#include <vector>
using namespace std;

class ZQuestGameInfo
{
private:
	bool							m_bInitialized;
	vector<MQUEST_NPC>				m_NPCInfoVector;
	vector<MQuestLevelSectorNode>	m_MapSectorVector;
	int								m_nQL;
	float							m_fNPC_TC;
	int								m_nNPCCount;
	int								m_nNPCKilled;
	int								m_nCurrSectorIndex;
	int								m_nNumOfObtainQuestItem;
	vector<MUID>					m_Bosses;
	rvector							m_vPortalPos;
	int								m_nMapsetID;
	unsigned int					m_nScenarioID;
	int								m_nRepeat;
	int								m_nRepeated;
public:
	ZQuestGameInfo();
	~ZQuestGameInfo();
	void Init(MTD_QuestGameInfo* pMTDQuestGameInfo);
	void Final();
	void OnMovetoNewSector(int nSectorIndex, int nRepeated);

	int GetNPCInfoCount();
	int GetMapSectorCount();
	MQUEST_NPC GetNPCInfo(int index);
	int GetMapSectorID(int index);
	int GetMapSectorLink(int index);
	bool IsInited();
	inline float GetNPC_TC();
	inline bool IsCurrSectorLastSector();
	inline bool IsLastSectorInSurvival();
	int GetCurrSectorIndex() { return m_nCurrSectorIndex; }
	int GetCurrRepeatIndex() { return m_nRepeated; }
	int GetRepeatCount() { return m_nRepeat; }
	int GetNPCCount(void) { return m_nNPCCount; }
	int GetNPCKilled(void) { return m_nNPCKilled; }
	void ClearNPCKilled(void) { m_nNPCKilled = 0; }
	void IncreaseNPCKilled(void) { m_nNPCKilled++; }
	void SetQuestLevel(int nQL) { m_nQL = nQL; }
	void SetMapsetID(unsigned int nID) { m_nMapsetID = nID; }
	unsigned int GetMapsetID() { return m_nMapsetID; }
	void SetSenarioID(unsigned int nID) { m_nScenarioID = nID; }
	unsigned int GetSenarioID() { return m_nScenarioID; }
	int GetQuestLevel(void) { return m_nQL; }
	int GetNumOfObtainQuestItem(void) { return m_nNumOfObtainQuestItem; }
	void IncreaseObtainQuestItem(void) { m_nNumOfObtainQuestItem++; }
	vector<MUID>& GetBosses(void) { return m_Bosses; }
	MUID GetBoss();
	void SetPortalPos(rvector& pos) { m_vPortalPos = pos; }
	rvector GetPortalPos() { return m_vPortalPos; }
};

inline int ZQuestGameInfo::GetNPCInfoCount()
{
	return (int)m_NPCInfoVector.size();
}
inline int ZQuestGameInfo::GetMapSectorCount()
{
	return (int)m_MapSectorVector.size();
}
inline MQUEST_NPC ZQuestGameInfo::GetNPCInfo(int index)
{
	return m_NPCInfoVector[index];
}
inline int ZQuestGameInfo::GetMapSectorID(int index)
{
	return m_MapSectorVector[index].nSectorID;
}
inline int ZQuestGameInfo::GetMapSectorLink(int index)
{
	return m_MapSectorVector[index].nNextLinkIndex;
}

inline bool ZQuestGameInfo::IsInited()
{
	return m_bInitialized;
}

inline float ZQuestGameInfo::GetNPC_TC()
{
	return m_fNPC_TC;
}

bool ZQuestGameInfo::IsCurrSectorLastSector()
{
	if (GetMapSectorCount() <= (m_nCurrSectorIndex + 1)) return true;
	return false;
}

bool ZQuestGameInfo::IsLastSectorInSurvival()
{
	if (GetMapSectorCount() <= m_nCurrSectorIndex + 1 &&
		m_nRepeat <= m_nRepeated + 1)
		return true;
	return false;
}

#endif