#pragma once
#include <map>
#include <list>

#include "MMatchNPCObject.h"

struct RewardZItemInfo
{
	unsigned int		nItemID;
	int					nRentPeriodHour;
	int					nItemCnt;
};

class MQuestRewardZItemList : public std::list<RewardZItemInfo> { };

struct MQuestPlayerInfo
{
	MMatchObject* pObject;
	unsigned long int	nNPCControlCheckSum;
	MMatchNPCObjectMap	NPCObjects;
	bool				bEnableNPCControl;

	int GetNPCControlScore() { return (int)(NPCObjects.size()); }

	bool				bMovedtoNewSector;

	int						nQL;
	int						nDeathCount;
	int						nUsedPageSacriItemCount;
	int						nUsedExtraSacriItemCount;
	int						nXP;
	int						nBP;
	int						nKilledNpcHpApAccum;

	MQuestItemMap			RewardQuestItemMap;
	MQuestRewardZItemList	RewardZItemList;

	void Init(MMatchObject* pObj, int a_nQL)
	{
		pObject = pObj;
		bEnableNPCControl = true;
		nNPCControlCheckSum = 0;
		NPCObjects.clear();
		bMovedtoNewSector = true;

		nQL = a_nQL;
		nDeathCount = 0;
		nUsedPageSacriItemCount = 0;
		nUsedExtraSacriItemCount = 0;
		nXP = 0;
		nBP = 0;
		nKilledNpcHpApAccum = 0;

		RewardQuestItemMap.Clear();
		RewardZItemList.clear();
	}

	MQuestPlayerInfo() : nXP(0), nBP(0), nKilledNpcHpApAccum(0) { }
};

class MQuestPlayerManager : public std::map<MUID, MQuestPlayerInfo*>
{
private:
	MMatchStage* m_pStage;
public:
	MQuestPlayerManager();
	~MQuestPlayerManager();
	void Create(MMatchStage* pStage);
	void Destroy();
	void AddPlayer(MUID& uidPlayer);
	void DelPlayer(MUID& uidPlayer);
	void Clear();
	MQuestPlayerInfo* GetPlayerInfo(const MUID& uidPlayer);
};