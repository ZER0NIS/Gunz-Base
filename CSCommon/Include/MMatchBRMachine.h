#pragma once

#include "MMatchCRC32XORCache.h"

#define MAX_REWARD_ITEM_RATE 1000 - 1

class MMatchBRItem
{
protected:
	int m_nBRID;
	int m_nBRIID;

	int m_nItemIDMale;
	int m_nItemIDFemale;
	int m_nItemCnt;

	int m_nRentHourPeriod;
	int m_nRate;
	int m_nRateRange;

public:
	MMatchBRItem(int nBRID, int nBRIID, int nItemIDMale, int nItemIDFemale, int nItemCnt, int nRentHourPeriod, int nRate)
	{
		m_nBRID = nBRID;
		m_nBRIID = nBRIID;

		m_nItemIDMale = nItemIDMale;
		m_nItemIDFemale = nItemIDFemale;
		m_nItemCnt = nItemCnt;

		m_nRentHourPeriod = nRentHourPeriod;
		m_nRate = nRate;

		m_nRateRange = 0;
	}

	int GetBRID()				{ return m_nBRID; }
	int GetBRIID()				{ return m_nBRIID; }
	int GetItemIDMale()			{ return m_nItemIDMale; }
	int GetItemIDFemale()		{ return m_nItemIDFemale; }
	int GetItemCnt()			{ return m_nItemCnt; }
	int GetRentHourPeriod()		{ return m_nRentHourPeriod; }
	int GetRate()				{ return m_nRate; }

	int GetItemID(int nVal)	
	{ 
		if( nVal == 0)		return m_nItemIDMale; 
		else if(nVal == 1)	return m_nItemIDFemale; 
		
		return 0; 
	}
	

	int GetRateRange()					{ return m_nRateRange; }
	void SetRateRange(int nRateRage)	{ m_nRateRange = nRateRage; }
};

class MMatchBRDescription
{
protected:
	string m_strName;
	string m_strResetDesc;

	int m_nBRID;
	int m_nBRTID;

	int m_nRewardMinutePeriod;			///< 보상 주기
	int m_nRewardCount;					///< 보상 가능한 갯수
	int m_nRewardKillCount;				///< 보상 받기 위해 필요한 킬수

	vector<MMatchBRItem*> m_RewardItemList;

	int m_nTotalRate;

public:
	MMatchBRDescription(int nBRID, string strName, string strResetDesc, int nBRTID, int nRewardMinutePeriod, int nRewardCount, int nRewardKillCount);
	~MMatchBRDescription();

	void AddRewardItem(MMatchBRItem* pRewardItem);
	MMatchBRItem* GetRewardItem();

	int	GetBRID()				{ return m_nBRID; }
	int GetBRTID()				{ return m_nBRTID; }
	int	GetRewardMinutePeriod()	{ return m_nRewardMinutePeriod; }
	int	GetRewardCount()		{ return m_nRewardCount; }
	int	GetRewardkillCount()	{ return m_nRewardKillCount; }

	string GetName()			{ return m_strName; }
	string GetResetDesc()		{ return m_strResetDesc; }

	int GetTotalRate() { return m_nTotalRate; }

	DWORD GetCRC32();

	vector<MMatchBRItem*>& GetBattleRewardItemList() { return m_RewardItemList; }

};

class MMatchBRDescriptionMap : public map<int, MMatchBRDescription*>
{
protected:
	MMatchCRC32XORCache m_CRC32;

public:
	MMatchBRDescriptionMap();

	void Clear();

	void MakeCRC32();
	DWORD GetCRC32() { return m_CRC32.GetCRC32(); }
};

class MMatchBRMachine
{
protected:
	unsigned long int m_nLastUpdatedTime;

	MMatchCRC32XORCache m_RewardDescriptionCRC;
	MMatchBRDescriptionMap m_RewardDescription;

public:
	MMatchBRMachine(void);
	~MMatchBRMachine(void);

	void SetBattleTimeRewardMachine(MMatchBRDescriptionMap DescriptionMap);

	bool IsValidBattleTimeRewardID(int nBRID);

	MMatchBRDescriptionMap& GetBattleTimeRewardDescriptionMap() { return m_RewardDescription;}
	MMatchBRDescription* GetBattleTimeRewardDescription(int nBRID);

	int GetLastUpdateTime()							{ return m_nLastUpdatedTime; }
	void SetLastUpdateTime(unsigned long int nTime) { m_nLastUpdatedTime = nTime; }
};