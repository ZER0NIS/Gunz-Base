#pragma once

#include "MAsyncDBJob.h"

class MBMatchAsyncDBJob_BringBackAccountItemStackable : public MAsyncJob
{
private :
	MUID	m_uidItem;

	DWORD	m_dwAID;
	DWORD	m_dwCID;
	DWORD	m_dwCIID;

	DWORD	m_dwItemCnt;

public :
	MBMatchAsyncDBJob_BringBackAccountItemStackable(const MUID& uidOwner) 
		: MAsyncJob(MASYNCJOB_BRINGBACK_ACCOUNTITEM_STACKABLE, uidOwner){}

		~MBMatchAsyncDBJob_BringBackAccountItemStackable(){}

		void Input( const MUID& uidItem, DWORD dwAID, DWORD dwCID, DWORD dwCIID, DWORD dwItemCnt)
		{
			m_uidItem	= uidItem;

			m_dwAID		= dwAID;
			m_dwCID		= dwCID;
			m_dwCIID	= dwCIID;

			m_dwItemCnt = dwItemCnt;
		}


		void Run( void* pContext );

		const MUID& GetItemUID()	{ return m_uidItem; }
		DWORD GetItemCnt()			{ return m_dwItemCnt; }
};
