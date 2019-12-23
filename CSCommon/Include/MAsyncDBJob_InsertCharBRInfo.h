#pragma once

#include "MAsyncDBJob.h"

class MAsyncDBJob_GetCharBRInfo : public MAsyncJob 
{
protected:	// Input Value
	int m_nCID;

	int m_nBRID;
	int m_nBRTID;

	MMatchCharBRInfo m_CharBRInfo;

public:
	MAsyncDBJob_GetCharBRInfo(MUID& uidOwner) : 
	  MAsyncJob(MASYNCJOB_GET_CHAR_BR_INFO, uidOwner) {}

	virtual ~MAsyncDBJob_GetCharBRInfo() {}

	bool Input(int nCID, int nBRID, int nBRTID);

	virtual void Run(void* pContext);	

	int GetCID()	{ return m_nCID; }
	int GetBRID()	{ return m_nBRID;}

	MMatchCharBRInfo& GetCharBRInfo() { return m_CharBRInfo; }
};
