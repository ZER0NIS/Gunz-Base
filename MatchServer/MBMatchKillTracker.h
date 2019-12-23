#ifndef _MBMATCHKILLTRACKER_H
#define _MBMATCHKILLTRACKER_H


#include "MUID.h"
#include "MBMatchAttackerVictimPair.h"
#include "MBaseLocale.h"

#include <map>
#include <vector>
#include <string>

using std::map;
using std::vector;
using std::string;


class MMatchKillTrackerConfig;


class MBMatchTrackerInfo
{
private :
	MBMatchAttVicPair	m_AttVicPair;
	DWORD				m_dwKillCount;
	char				m_szKillCount[ 8 ];
	DWORD				m_dwLastUpdateTime;
	bool				m_bIsWroteLogToDB;

private :
	MBMatchTrackerInfo() : m_AttVicPair( 0, 0)
	{
		Reset( 0 );
	}

public :
	MBMatchTrackerInfo( const MBMatchAttVicPair& AttVicPair, const DWORD dwCurTime ) 
		: m_AttVicPair( AttVicPair.GetAttackerCID(), AttVicPair.GetVictimCID() )
	{
		Reset( dwCurTime );
	}

	~MBMatchTrackerInfo() {}

	DWORD				GetKillCount()				{ return m_dwKillCount; }
	DWORD				GetLastUpdateTime()			{ return m_dwLastUpdateTime; }
	MBMatchAttVicPair&	GetAttVicPair()				{ return m_AttVicPair; }
	
	void				IncreaseKillCount()			{ ++m_dwKillCount; }

	bool				IsWroteLogToDB()			{ return m_bIsWroteLogToDB; }
	void				SetWriteLogToDB()			{ m_bIsWroteLogToDB = true; }


	const char* GetKillCountToString()		
	{
		memset( m_szKillCount, 0, 8 );

		_snprintf( m_szKillCount, 8, "%u", m_dwKillCount );

		return m_szKillCount;
	}


	void Reset( const DWORD dwCurTime )
	{
		m_dwLastUpdateTime	= dwCurTime;
		m_dwKillCount		= 0;
		m_bIsWroteLogToDB	= false;
	}
};


class MBMatchKillTrackerHalper
{
private :
	DWORD	m_dwKillTrackerUpdateTime;
    DWORD	m_dwTrackerInfoLifeTime;
	DWORD	m_dwMaxKillCountOnTraceTime;
	DWORD	m_dwKillCountTraceTime;
	
public :
	void	Init( MMatchKillTrackerConfig* pConfig );

	bool	IsPowerLevelingKillCount( const DWORD dwBeKilledCount );
	bool	IsExpiredTimeForTraceKillCount( const DWORD dwElapsedtime );
	bool	IsExpiredTrackerInfoLifetime( const DWORD dwElapsedtime );
	bool	IsExpiredKillTrackerUpdateTime( const DWORD dwElapsedtime );
	bool	IsSuicide( const DWORD dwAttackerCID, const DWORD dwVictimCID );
};



typedef map< MBMatchAttVicPair, MBMatchTrackerInfo* >	TrackerInfoMap;


class MBMatchKillTracker
{
private :
	TrackerInfoMap				m_TrackerInfoMap;
	MBMatchKillTrackerHalper	m_Helper;
	DWORD						m_dwLastUpdateTime;
	MCountry					m_CountryCode;
	bool						m_IsUseKillTracker;
	
	
private :
	MBMatchTrackerInfo*			AddTrackerInfo( const DWORD dwAttackerCID, const DWORD dwVictimCID, const DWORD dwCurTime );
	MBMatchTrackerInfo*			GetTrackerInfo( MBMatchAttVicPair& AttVicPair );
	void						DeleteTrackerInfo( MBMatchAttVicPair& AttVicPair, MBMatchTrackerInfo* pTrackerInfo );

	
				
public :
	MBMatchKillTracker();
	~MBMatchKillTracker();

    bool						Init( MMatchKillTrackerConfig* pConfig );
	bool						IncreaseAttackCount( const DWORD dwAttackerCID, const DWORD dwVictimCID, const DWORD dwCurTime );
	void						Update( const DWORD dwCurTime );
	void						Release();
	const DWORD					GetKillCount( const DWORD dwAttackerCID, const DWORD dwVictimCID );
};


#endif