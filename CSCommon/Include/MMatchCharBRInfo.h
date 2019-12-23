#ifndef MMATCHOBJECTCHARTIMECHECKER_H
#define MMATCHOBJECTCHARTIMECHECKER_H

#include "MMatchBRMachine.h"

#define CYCLE_CHAR_BATTLE_TIME_CHECK		1000

#ifdef _DEBUG
	#define MINUTE_PERIOD_UNIT	30 * 1000
	#define CYCLE_CHAR_BATTLE_TIME_UPDATE_DB	10 * 1000
#else
	#define MINUTE_PERIOD_UNIT	60 * 1000
	#define CYCLE_CHAR_BATTLE_TIME_UPDATE_DB	3 * 60 * 1000
#endif

enum BRRESULT
{
	BRRESULT_NO_REWARD = 0,
	BRRESULT_DO_REWARD,
	BRRESULT_RESET_INFO,
};

class MMatchCharBRInfo
{
protected:
	int m_nBRID;
	int m_nBRTID;

	unsigned long int m_nBattleTime;

	int m_nRewardCount;
	int m_nKillCount;
	
	bool m_bCheckSkip;

	unsigned long int m_nLastCheckTime;
	unsigned long int m_nLastUpdateDBTime;

public:
	MMatchCharBRInfo();
	MMatchCharBRInfo(int nBRID, int nBRTID, unsigned long int nBattleTime, int nRewardCount, int nKillCount);

	void ResetInfo();
	void SetBRInfo(int nBRID, int nBRTID, unsigned long int nBattleTime, int nRewardCount, int nKillCount);

	int GetBRID()			{ return m_nBRID; }
	int GetBRTID()			{ return m_nBRTID; }
	int GetRewardCount()	{ return m_nRewardCount; }
	int GetBattleTime()		{ return m_nBattleTime; }

	void AddKillCount(int nVal)		{ m_nKillCount += nVal; }
	int GetKillCount()				{ return m_nKillCount; }

	void SetCheckSkip(bool bVal) { m_bCheckSkip = bVal; }
	bool IsCheckSkip() { return m_bCheckSkip; }

	bool IsExpired(int nBRTID);
	bool IsNeedUpdateDB(unsigned long int nTick);

	BRRESULT CheckBattleTimeReward(unsigned long int nTick, MMatchBRDescription* pDesc);
	//bool IsRewardTarget(unsigned long int nTick, MMatchBRDescription* pDesc);	

	unsigned int GetLastCheckTime() { return m_nLastCheckTime; }
	void SetLastCheckTime(unsigned long int nTime) { m_nLastCheckTime = nTime; }

	unsigned int GetLastUpdateDBTime() { return m_nLastUpdateDBTime; }
	void SetLastUpdateDBTime(unsigned long int nTime) { m_nLastUpdateDBTime = nTime; }
};

class MMatchCharBattleTimeRewardInfoMap : public map<int, MMatchCharBRInfo*>
{
public:
	bool Insert(int nBRID, MMatchCharBRInfo* pInfo)
	{
		if( Get(nBRID) != NULL ) return false;
		insert(pair<int, MMatchCharBRInfo*>(nBRID, pInfo));
		return true;
	}

	MMatchCharBRInfo* Get(int nBRID)
	{
		iterator iter = find(nBRID);
		if( iter != end() ) return iter->second;
		return NULL;
	}

	void Remove(int nBRID)
	{
		iterator iter = find(nBRID);
		if( iter != end() )	{
			delete iter->second;
			erase(iter);
		}
	}
};

#endif

