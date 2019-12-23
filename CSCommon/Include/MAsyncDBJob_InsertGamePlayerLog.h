#pragma once

#include "MAsyncDBJob.h"

class MAsyncDBJob_InsertGamePlayerLog : public MAsyncJob 
{
protected: // Input Argument
	int m_nGameLogID;
	int m_nCID;
	int m_nPlayTime;
	int m_nKills;
	int m_nDeaths;
	int m_nXP;
	int m_nBP;

protected:	// Output Result

public:
	MAsyncDBJob_InsertGamePlayerLog() : MAsyncJob(MASYNCJOB_INSERTGAMEPLAYERLOG, MUID(0, 0)) {}
	virtual ~MAsyncDBJob_InsertGamePlayerLog() {}

	bool Input(int nGameLogID, int nCID, int nPlayTime, int nKills, int nDeaths, int nXP, int nBP);

	virtual void Run(void* pContext);
};
