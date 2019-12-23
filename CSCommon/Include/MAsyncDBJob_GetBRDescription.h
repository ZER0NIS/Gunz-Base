#pragma once

#include "MAsyncDBJob.h"

class MAsyncDBJob_GetBattleTimeRewardDescription : public MAsyncJob 
{
protected:	// Output Result
	vector<MMatchBRDescription*>	m_vBattletimeRewardDescription;
	vector<MMatchBRItem*>			m_vBattletimeRewardItem;

public:
	MAsyncDBJob_GetBattleTimeRewardDescription() : MAsyncJob(MASYNCJOB_GET_BR_DESCRIPTION, MUID(0, 0)) {}
	virtual ~MAsyncDBJob_GetBattleTimeRewardDescription() 
	{
		m_vBattletimeRewardDescription.clear();
		m_vBattletimeRewardItem.clear();
	}

	virtual void Run(void* pContext);

	vector<MMatchBRDescription*>& GetBattleTimeRewardDescription()	{ return m_vBattletimeRewardDescription; }
	vector<MMatchBRItem*>& GetBattleTimeRewardItem()					{ return m_vBattletimeRewardItem; }
};

