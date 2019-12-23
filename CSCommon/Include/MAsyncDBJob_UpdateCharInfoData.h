#ifndef _MASYNCDBJOB_UPDATECHARINFODATA_H
#define _MASYNCDBJOB_UPDATECHARINFODATA_H

#include "MAsyncDBJob.h"

class MAsyncDBJob_UpdateCharInfoData : public MAsyncJob {
protected:	// Input Argument
	int			m_nCID;
	int			m_nAddedXP; 
	int			m_nAddedBP;
    int			m_nAddedKillCount;
	int			m_nAddedDeathCount;
	int			m_nAddedPlayTime;
protected:	// Output Result

public:
	MAsyncDBJob_UpdateCharInfoData(const MUID& uidOwner)
		: MAsyncJob(MASYNCJOB_UPDATECHARINFODATA, uidOwner)
	{

	}
	virtual ~MAsyncDBJob_UpdateCharInfoData()	{}
	bool Input(const int nCID, 
			   const int nAddedXP, 
			   const int nAddedBP, 
               const int nAddedKillCount, 
			   const int nAddedDeathCount,
			   const int nAddedPlayTime);

	virtual void Run(void* pContext);
};





#endif