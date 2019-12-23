#pragma once


#include "MAsyncDBJob.h"


class MAsyncDBJob_BuyQuestItem : public MAsyncJob
{
public :
	MAsyncDBJob_BuyQuestItem(const MUID& uidOwner) : MAsyncJob( MASYNCJOB_BUYQUESTITEM, uidOwner )
	{
	}

	~MAsyncDBJob_BuyQuestItem() 
	{
	}


	void Input( const MUID& uidPlayer, const DWORD dwCID, const int nItemCount, const int nPrice )
	{
		m_uidPlayer = uidPlayer;
		m_dwCID		= dwCID;
		m_nItemCount = nItemCount;
		m_nPrice	= nPrice * nItemCount;		
	}


	void Run(void* pContext);


	const MUID&	GetPlayerUID()	{ return m_uidPlayer; }
	const int	GetPrice()		{ return m_nPrice; }
	const int	GetItemCount()	{ return m_nItemCount; }

private :
	MUID	m_uidPlayer;
	DWORD	m_dwCID;
	int		m_nPrice;
	int		m_nItemCount;
};