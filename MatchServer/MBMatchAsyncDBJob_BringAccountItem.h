#ifndef _MBMATCHASYNCDBJOB_BRINGACCOUNTITEM_H
#define _MBMATCHASYNCDBJOB_BRINGACCOUNTITEM_H

#include "MAsyncDBJob.h"

class MBMatchAsyncDBJob_BringAccountItem : public MAsyncJob 
{
protected:	// Input Argument
	unsigned int		m_nAID;
	unsigned int		m_nCID;
	unsigned int		m_nAIID;

protected:	// Output Result
	unsigned long int	m_noutCIID;
	unsigned long int	m_noutItemID;
	bool				m_boutIsRentItem;
	int					m_noutRentMinutePeriodRemainder;
	WORD				m_woutRentHourPeriod;

public:
	MBMatchAsyncDBJob_BringAccountItem(const MUID& uid) : MAsyncJob(MASYNCJOB_BRING_ACCOUNTITEM, uid)
	{
		m_nAID		= 0;
		m_nCID		= 0;
		m_nAIID		= 0;

		m_noutCIID						= 0;
		m_noutItemID					= 0;
		m_boutIsRentItem				= 0;
		m_noutRentMinutePeriodRemainder	= 0;
		m_woutRentHourPeriod			= 0;
	}
	virtual ~MBMatchAsyncDBJob_BringAccountItem()	{}

	bool Input(unsigned int nAID, unsigned int nCID, unsigned int nAIID);
	virtual void Run(void* pContext);

	const MUID& GetUID()					{ return m_uidOwner; }

	unsigned long int GetOutCIID()			{ return m_noutCIID; }
	unsigned long int GetOutItemID()		{ return m_noutItemID; }
	bool GetOutIsRentItem()					{ return m_boutIsRentItem; }	
	int GetOutRentMinutePeriodRemainder()	{ return m_noutRentMinutePeriodRemainder; }
	WORD GetOutRentHourPeriod()				{ return m_woutRentHourPeriod; }
};

#endif