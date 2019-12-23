#pragma once



#include "MAsyncDBJob.h"



class MAsyncDBJob_UpdateAccountLastLoginTime : public MAsyncJob
{
public :
	MAsyncDBJob_UpdateAccountLastLoginTime(const MUID& uidOwner) : MAsyncJob( MASYNCJOB_UPDATEACCOUNTLASTLOGINTIME, uidOwner )
	{
	}

	~MAsyncDBJob_UpdateAccountLastLoginTime()
	{
	}

	void Input( const DWORD dwAID )
	{
		m_dwAID = dwAID;
	}

	void Run( void* pContext )
	{
		MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

		pDBMgr->UpdateAccountLastLoginTime( m_dwAID );
	}

private :
	DWORD m_dwAID;
};