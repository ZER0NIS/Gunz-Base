#pragma once

#include "MAsyncDBJob.h"

class MBMatchAsyncDBJob_BringAccountItemStackable : public MAsyncJob 
{
protected:	// Input Argument
	unsigned int		m_nAID;
	unsigned int		m_nCID;
	unsigned int		m_nAIID;
	unsigned int		m_nCIID;

	unsigned int		m_nItemCnt;

protected:	// Output Result
	unsigned long int	m_noutCIID;
	unsigned long int	m_noutItemID;
	unsigned int		m_noutItemCnt;
	int					m_noutRentMinutePeriodRemainder;
	bool				m_boutIsRentItem;	
	WORD				m_woutRentHourPeriod;

public:
	MBMatchAsyncDBJob_BringAccountItemStackable(const MUID& uid) 
		: MAsyncJob(MASYNCJOB_BRING_ACCOUNTITEM_STACKABLE, uid)
	{
		m_nAID		= 0;
		m_nCID		= 0;
		m_nAIID		= 0;
		m_nCIID		= 0;

		m_noutCIID						= 0;
		m_noutItemID					= 0;
		m_noutItemCnt					= 0;
		m_boutIsRentItem				= 0;
		m_noutRentMinutePeriodRemainder	= 0;
		m_woutRentHourPeriod			= 0;
	}
	virtual ~MBMatchAsyncDBJob_BringAccountItemStackable()	{}

	bool Input(unsigned int nAID, unsigned int nCID, unsigned int nAIID, unsigned int nCIID, unsigned int nCount);
	virtual void Run(void* pContext);

	const MUID& GetUID()				{ return m_uidOwner; }

	unsigned long int GetOutCIID()			{ return m_noutCIID;	}
	unsigned long int GetOutItemID()		{ return m_noutItemID;	}
	unsigned int GetOutItemCnt()			{ return m_noutItemCnt; }
	bool GetOutIsRentItem()					{ return m_boutIsRentItem; }	
	int GetOutRentMinutePeriodRemainder()	{ return m_noutRentMinutePeriodRemainder; }
	WORD GetOutRentHourPeriod()				{ return m_woutRentHourPeriod; }
};
