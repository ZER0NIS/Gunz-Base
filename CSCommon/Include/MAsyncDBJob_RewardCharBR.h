#pragma once

#include "MAsyncDBJob.h"

class MAsyncDBJob_RewardCharBR : public MAsyncJob 
{
protected:	// Input Value
	int m_nCID;

	int m_nBRID;
	int m_nBRTID;

	int m_nRewardCount;

	int m_nBattleTime;
	int m_nKillCount;

	int m_nItemID;
	int m_nItemCnt;
	int m_nRentHourPeriod;

	bool m_bIsSpendable;

protected:	// Output Value
	int m_nCIID;

public:

	MAsyncDBJob_RewardCharBR(const MUID& uidOwner) : 
	  MAsyncJob(MASYNCJOB_REWARD_CHAR_BR, uidOwner) {}

	  virtual ~MAsyncDBJob_RewardCharBR() {}

	  bool Input(int nCID, int nBRID, int nBRTID, int nRewardCount, int nBattleTime, int nKillCount, int nItemID, int nItemCnt, int nRentHourPeriod, bool bIsSpendable);

	  virtual void Run(void* pContext);	

	  int GetCID()				{ return m_nCID; }
	  int GetBRID()				{ return m_nBRID; }
	  int GetBRTID()			{ return m_nBRTID; }
	  int GetRewardCount()		{ return m_nRewardCount; }
	  int GetBattleTime()		{ return m_nBattleTime; }
	  int GetKillCount()		{ return m_nKillCount; }
	  int GetItemID()			{ return m_nItemID; }
	  int GetItemCnt()			{ return m_nItemCnt; }
	  int GetRentHourPeriod()	{ return m_nRentHourPeriod; }

	  int GetCIID() { return m_nCIID; }
};
