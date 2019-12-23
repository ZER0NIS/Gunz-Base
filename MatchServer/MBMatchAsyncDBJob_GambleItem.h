#pragma once

#include "MAsyncDBJob.h"


class MBMatchAsyncDBJob_GambleItem : public MAsyncJob
{
private :
	// Input
	MUID	m_uidPlayer;
	MUID	m_uidGItem;

	int m_nCID;	
	int m_nCIID;
	int m_nGRIID;
	int m_nGIID;
	int m_nRIID;
	int m_nItemCnt;
	int m_nRentHourPeriod;

	int m_bIsSpendable;

	// Output
	int m_nOutCIID;

public :
	MBMatchAsyncDBJob_GambleItem(const MUID& uidOwner) : MAsyncJob( MASYNCJOB_GAMBLE_ITEM, uidOwner ) 
	{
	}

	~MBMatchAsyncDBJob_GambleItem()	
	{
	}

	void Input( const MUID& uidPlayer
		, const MUID& uidGItem
		, int nCID		
		, int nCIID
		, int nGIID
		, int nGRIID
		, int nRIID
		, int nItemCnt
		, int nRentHourPeriod
		, bool bIsSpendable)
	{
		m_uidPlayer			= uidPlayer;
		m_uidGItem			= uidGItem;
		m_nCID				= nCID;		
		m_nCIID				= nCIID;		
		m_nGIID				= nGIID;
		m_nGRIID			= nGRIID;
		m_nRIID				= nRIID;		
		m_nItemCnt			= nItemCnt;
		m_nRentHourPeriod	= nRentHourPeriod;

		m_bIsSpendable = bIsSpendable;
	}

	void Run( void* pContext );

	MUID& GetPlayerUID()			{ return m_uidPlayer;		}
	MUID& GetGItemUID()				{ return m_uidGItem;		}

	int GetGItemCIID()				{ return m_nCIID;			}
	int GetGambleRewardItemID()		{ return m_nGRIID;			}
	int GetRewardItemCIID()			{ return m_nOutCIID;		}
	int GetRewardItemID()			{ return m_nRIID;			}
	int GetRewardItemCnt()			{ return m_nItemCnt;		}
	int GetRentHourPeriod()			{ return m_nRentHourPeriod; }	
};