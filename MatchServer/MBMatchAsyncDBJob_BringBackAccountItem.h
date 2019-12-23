#pragma once

#include "MAsyncDBJob.h"

class MBMatchAsyncDBJob_BringBackAccountItem : public MAsyncJob
{
private :
	MUID	m_uidItem;

	DWORD	m_dwAID;
	DWORD	m_dwCID;
	DWORD	m_dwCIID;

public :
	MBMatchAsyncDBJob_BringBackAccountItem(const MUID& uidOwner) 
		: MAsyncJob( MASYNCJOB_BRINGBACK_ACCOUNTITEM, uidOwner ){}

	~MBMatchAsyncDBJob_BringBackAccountItem(){}

	void Input( const MUID& uidItem, 
		const DWORD dwAID,
		const DWORD dwCID,
		const DWORD dwCIID)
	{
		m_uidItem	= uidItem;
		m_dwAID		= dwAID;
		m_dwCID		= dwCID;
		m_dwCIID	= dwCIID;
	}

	void Run( void* pContext );

	const MUID& GetItemUID()	{ return m_uidItem; }
};