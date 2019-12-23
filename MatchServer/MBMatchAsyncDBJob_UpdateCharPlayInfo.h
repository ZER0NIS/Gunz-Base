#pragma once

#include "MAsyncDBJob.h"

class MBMatchAsyncDBJob_UpdateCharPlayInfo : public MAsyncJob
{
public:
	MBMatchAsyncDBJob_UpdateCharPlayInfo(const MUID& uid) : MAsyncJob( MASYNCJOB_UPDATECHARPLAYINFO, uid )
	{
	}

	~MBMatchAsyncDBJob_UpdateCharPlayInfo()
	{
	}


	void Input( const DWORD dwAID
		, const DWORD dwCID
		, const int nXP
		, const int nLevel
		, const int nPlayingTimeSec
		, const int nTotalPlayTimeSec
		, const int nTotalKillCount
		, const int nTotalDeathCount
		, const int nBP
		, const bool bIsLevelUp )
	{
		m_dwAID				= dwAID;
		m_dwCID				= dwCID;
		m_nXP				= nXP;
		m_nLevel			= nLevel;
		m_nPlayingTimeSec	= nPlayingTimeSec;
		m_nTotalPlayTimeSec = nTotalPlayTimeSec;
		m_nTotalKillCount	= nTotalKillCount;
		m_nTotalDeathCount	= nTotalDeathCount;
		m_nBP				= nBP;
		m_bIsLevelUp		= bIsLevelUp;
	}

	void Run( void* pContext );	

private :
	DWORD	m_dwAID;
	DWORD	m_dwCID;
	int		m_nXP;
	int		m_nLevel;
	int		m_nPlayingTimeSec;		// 게임을 진행한 시간 
	int		m_nTotalPlayTimeSec;	// 플레이 시간
	int		m_nTotalKillCount;
	int		m_nTotalDeathCount;
	int		m_nBP;
	bool	m_bIsLevelUp;
};