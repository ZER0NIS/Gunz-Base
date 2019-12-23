#ifndef _MASYNCDBJOB_CHARFINALIZE_H
#define _MASYNCDBJOB_CHARFINALIZE_H

#include "MAsyncDBJob.h"

#include "MQuestItem.h"

class MAsyncDBJob_CharFinalize : public MAsyncJob {
protected:	// Input Argument
	DWORD				m_nAID;
	int					m_nCID;
	unsigned long int	m_nPlayTime;
	int					m_nConnKillCount;
	int					m_nConnDeathCount;
	int					m_nConnXP;
	int					m_nXP;
	MQuestItemMap		m_QuestItemMap;
	MQuestMonsterBible	m_QuestMonster;
	bool				m_bIsRequestQItemUpdate;
	
protected:	// Output Result

public:
	MAsyncDBJob_CharFinalize(const MUID& uidOwner)
		: MAsyncJob(MASYNCJOB_CHARFINALIZE, uidOwner), m_bIsRequestQItemUpdate( false )
	{
		m_nAID = 0;
	}
	virtual ~MAsyncDBJob_CharFinalize()
	{
		m_QuestItemMap.Clear();
	}

	bool Input( DWORD nAID,
				int	nCID, 
				unsigned long int nPlayTime, 
				int nConnKillCount, 
				int nConnDeathCount, 
				int nConnXP, 
				int nXP,
				MQuestItemMap& rfQuestItemMap,
				MQuestMonsterBible& rfQuestMonster,
				const bool bIsRequestQItemUpdate );


	virtual void Run(void* pContext);
};





#endif