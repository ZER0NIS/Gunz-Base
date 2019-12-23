#include "stdafx.h"
#include "MBMatchKillTracker.h"
#include "MMatchGlobal.h"
#include "MDebug.h"
#include "MMatchConfig.h"
#include "MUtil.h"
#include "MMatchconfig.h"


MBMatchKillTracker::MBMatchKillTracker()
{
	m_CountryCode		= MC_INVALID;
	m_dwLastUpdateTime	= 0;
}


MBMatchKillTracker::~MBMatchKillTracker()
{
	Release();
}


bool MBMatchKillTracker::Init( MMatchKillTrackerConfig* pConfig )
{ 
	if( NULL == pConfig )
		return false;

	m_IsUseKillTracker = pConfig->IsUseKillTracker();

	m_Helper.Init( pConfig );

	return false;
}


MBMatchTrackerInfo* MBMatchKillTracker::AddTrackerInfo( const DWORD dwAttackerCID, const DWORD dwVictimCID, const DWORD dwCurTime )
{
	MBMatchAttVicPair AttVicPair( dwAttackerCID, dwVictimCID );

	if( NULL != GetTrackerInfo(AttVicPair) )
	{
		_ASSERT( 0 && "ม฿บน" );
		return NULL;
	}

	MBMatchTrackerInfo* pTrackerInfo = new MBMatchTrackerInfo( AttVicPair, dwCurTime );
	if( NULL == pTrackerInfo )
		return NULL;

	m_TrackerInfoMap.insert( TrackerInfoMap::value_type(AttVicPair, pTrackerInfo) );

	return pTrackerInfo;
}


void MBMatchKillTracker::Update( const DWORD dwCurTime )
{
	_ASSERT( m_dwLastUpdateTime <= dwCurTime );

	if( !m_Helper.IsExpiredKillTrackerUpdateTime(dwCurTime - m_dwLastUpdateTime) )
		return;

	vector< MBMatchTrackerInfo* > ExpiredLifetimeList;

	TrackerInfoMap::iterator itTracker, endTracker;
	endTracker = m_TrackerInfoMap.end();
	for( itTracker = m_TrackerInfoMap.begin(); itTracker != endTracker; ++itTracker )
	{
		_ASSERT( itTracker->second->GetLastUpdateTime() <= dwCurTime );

		if( m_Helper.IsExpiredTrackerInfoLifetime(dwCurTime - itTracker->second->GetLastUpdateTime()) )
			ExpiredLifetimeList.push_back( itTracker->second );
	}

	vector< MBMatchTrackerInfo* >::iterator itDel, endDel;
	endDel = ExpiredLifetimeList.end();
	for( itDel = ExpiredLifetimeList.begin(); itDel != endDel; ++itDel )
	{
		DeleteTrackerInfo( (*itDel)->GetAttVicPair(), (*itDel) );
	}

	m_dwLastUpdateTime = dwCurTime;
}


void MBMatchKillTracker::Release()
{ 
	TrackerInfoMap::iterator it, end;
	end = m_TrackerInfoMap.end();
	for( it = m_TrackerInfoMap.begin(); it != end; ++it )
		delete it->second;

	m_TrackerInfoMap.clear();
}


bool MBMatchKillTracker::IncreaseAttackCount( const DWORD dwAttackerCID, const DWORD dwVictimCID, const DWORD dwCurTime )
{
	if( !m_IsUseKillTracker )
		return true;

	if( m_Helper.IsSuicide(dwAttackerCID , dwVictimCID) )
		return true;

	MBMatchAttVicPair AttVicPair( dwAttackerCID, dwVictimCID );

	MBMatchTrackerInfo* pTrackerInfo = GetTrackerInfo( AttVicPair );
	if( NULL == pTrackerInfo )
	{
		pTrackerInfo = AddTrackerInfo( dwAttackerCID, dwVictimCID, dwCurTime );
		if( NULL == pTrackerInfo )
			return false;

		pTrackerInfo->IncreaseKillCount();

		return true;
	}

	_ASSERT( pTrackerInfo->GetLastUpdateTime() <= dwCurTime );
	
	if( m_Helper.IsExpiredTimeForTraceKillCount(dwCurTime - pTrackerInfo->GetLastUpdateTime()) )
		pTrackerInfo->Reset( dwCurTime );
	
	pTrackerInfo->IncreaseKillCount();

	if( m_Helper.IsPowerLevelingKillCount(pTrackerInfo->GetKillCount()) )
		return false;
	
	return true;
}


void MBMatchKillTracker::DeleteTrackerInfo( MBMatchAttVicPair& AttVicPair, MBMatchTrackerInfo* pTrackerInfo )
{
	if( NULL == pTrackerInfo )
	{
		_ASSERT( m_TrackerInfoMap.end() != m_TrackerInfoMap.find(AttVicPair) );
		return;
	}

	_ASSERT( (AttVicPair.GetAttackerCID() == pTrackerInfo->GetAttVicPair().GetAttackerCID())
		&& (AttVicPair.GetVictimCID() == pTrackerInfo->GetAttVicPair().GetVictimCID()) );

	m_TrackerInfoMap.erase( AttVicPair );

	delete pTrackerInfo;	
}


MBMatchTrackerInfo* MBMatchKillTracker::GetTrackerInfo( MBMatchAttVicPair& AttVicPair )
{
	TrackerInfoMap::iterator itFind = m_TrackerInfoMap.find( AttVicPair );
	if( m_TrackerInfoMap.end() == itFind )
		return NULL;

	return itFind->second;
}


const DWORD MBMatchKillTracker::GetKillCount( const DWORD dwAttackerCID, const DWORD dwVictimCID )
{
	MBMatchTrackerInfo* pTrackerInfo = GetTrackerInfo( MBMatchAttVicPair(dwAttackerCID, dwVictimCID) );
	if( NULL == pTrackerInfo )
		return 0;

	return pTrackerInfo->GetKillCount();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void MBMatchKillTrackerHalper::Init( MMatchKillTrackerConfig* pConfig )
{
	m_dwKillTrackerUpdateTime		= pConfig->GetKillTrackerUpdateTime();
	m_dwMaxKillCountOnTraceTime		= pConfig->GetMaxKillCountOnTraceTime();
	m_dwTrackerInfoLifeTime			= pConfig->GetTrackerInfoLifeTime();
	m_dwKillCountTraceTime			= pConfig->GetKillCountTraceTime();
}


bool MBMatchKillTrackerHalper::IsSuicide( const DWORD dwAttackerCID, const DWORD dwVictimCID )
{
	return dwAttackerCID == dwVictimCID;
}


bool MBMatchKillTrackerHalper::IsExpiredTrackerInfoLifetime( const DWORD dwElapsedtime )
{
	_ASSERT( 0 < m_dwKillTrackerUpdateTime );
		
	return m_dwTrackerInfoLifeTime < dwElapsedtime;
}


bool MBMatchKillTrackerHalper::IsPowerLevelingKillCount( const DWORD dwBeKilledCount ) 
{
	return m_dwMaxKillCountOnTraceTime < dwBeKilledCount;
}


bool MBMatchKillTrackerHalper::IsExpiredTimeForTraceKillCount( const DWORD dwElapsedtime )
{
	_ASSERT( 0 < m_dwKillCountTraceTime );

	return m_dwKillCountTraceTime < dwElapsedtime;
}


bool MBMatchKillTrackerHalper::IsExpiredKillTrackerUpdateTime( const DWORD dwElapsedtime )	
{
	_ASSERT( 0 < m_dwMaxKillCountOnTraceTime );

	return m_dwKillTrackerUpdateTime < dwElapsedtime;
}