#ifndef _MASYNCDBJOB_UPDATECHARITEMINFODATA_H
#define _MASYNCDBJOB_UPDATECHARITEMINFODATA_H

#include "MAsyncDBJob.h"

typedef struct _UpdateItem
{
	bool bIsSuccess;

	int nCIID;
	int	nChangeCnt;	
} UpdateItem;


class MAsyncDBJob_UpdateCharItemInfoData : public MAsyncJob {
protected:	
	list<UpdateItem *>	m_ItemList;

public:
	MAsyncDBJob_UpdateCharItemInfoData(const MUID& uidOwner) 
		: MAsyncJob(MASYNCJOB_UPDATE_CHARITEM_COUNT, uidOwner){}

	virtual ~MAsyncDBJob_UpdateCharItemInfoData()	
	{
		RemoveAll();
	}

	bool Input(const int nCIID, const int nChangeCnt);
	virtual void Run(void* pContext);

	void RemoveAll();
};
#endif