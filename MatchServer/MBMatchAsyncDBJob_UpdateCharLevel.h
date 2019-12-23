#pragma once



#include "MAsyncDBJob.h"



class MBMatchAsyncDBJob_UpdateCharLevel : public MAsyncJob
{
public :
	MBMatchAsyncDBJob_UpdateCharLevel(const MUID& uid) : MAsyncJob( MASYNCJOB_UPDATECHARLEVEL, uid )
	{
	}

	~MBMatchAsyncDBJob_UpdateCharLevel()
	{
	}


	void Input( const DWORD dwCID
		, const int nNewLevel
		, const int nBP
		, const int nTotalKillCount
		, const int nTotalDeathCount
		, const int nTotalPlayTimeSec
		, const bool bIsLevelUp )
	{
		m_dwCID				= dwCID;
		m_nNewLevel			= nNewLevel;
		m_nBP				= nBP;
		m_nTotalKillCount	= nTotalKillCount;
		m_nTotalDeathCount	= nTotalDeathCount;
		m_nTotalPlayTimeSec = nTotalPlayTimeSec;
		m_bIsLevelUp		= bIsLevelUp;
	}

	void Run( void* pContext );	

private :
	DWORD	m_dwCID;
	int		m_nNewLevel;
	int		m_nBP;
	int		m_nTotalKillCount;
	int		m_nTotalDeathCount;
	int		m_nTotalPlayTimeSec;
	bool	m_bIsLevelUp;
};