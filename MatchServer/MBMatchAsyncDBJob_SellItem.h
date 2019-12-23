#pragma once

#include "MAsyncDBJob.h"

class MBMatchAsyncDBJob_SellItem : public MAsyncJob
{
public:
	MBMatchAsyncDBJob_SellItem(const MUID& uidOwner, int nJobID) : MAsyncJob( nJobID, uidOwner ){}
	~MBMatchAsyncDBJob_SellItem(){}

	void Input(const MUID& uidPlayer, 
				const MUID& uidItem, 
				const int nCID, 
				const int nCIID, 
				const int nSellItemID, 
				const int nSellPrice, 
				const int nSellCount,
				const int CharBP)
	{
		m_uidPlayer		= uidPlayer;
		m_uidItem		= uidItem;

		m_nCID			= nCID;
		m_nCIID			= nCIID;
		m_nSellItemID	= nSellItemID;
		m_nSellPrice	= nSellPrice;
		m_nSellCount	= nSellCount;

		m_CharBP		= CharBP;
	}

	virtual void Run( void* pContext );

	
	MUID&	GetPlayerUID()		{ return m_uidPlayer;	}
	MUID&	GetItemUID()		{ return m_uidItem;		}
	int		GetSellPrice()		{ return m_nSellPrice;	}
	int		GetSellItemCount()  { return m_nSellCount;	}

protected :
	MUID	m_uidPlayer;
	MUID	m_uidItem;
	int		m_nCID;
	int		m_nCIID;
	int		m_nSellItemID;
	int		m_nSellPrice;
	int		m_nSellCount;

	int		m_CharBP;
};