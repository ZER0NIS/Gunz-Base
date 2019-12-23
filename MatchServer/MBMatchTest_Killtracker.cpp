#include "stdafx.h"
#include "MBMatchTest.h"
#include "MMatchObject.h"
#include "MDebug.h"
#include "MBMatchServer.h"
#include "MMatchKillTrackerConfig.h"
#include "MMatchConfig.h"


#include <utility>

using std::pair;




void MBMatchTest::InitPowerLevelingTest()
{
	MMatchObject*	pAttacker		= NULL;
	MMatchObject*	pVictim			= NULL;
	MMatchCharInfo* pAttCharInfo	= NULL;
	MMatchCharInfo* pVicCharInfo	= NULL;

	pAttacker		= new MMatchObject( MUID(0, 1) );
	pVictim			= new MMatchObject( MUID(0, 2) );
	pAttCharInfo	= new MMatchCharInfo;
	pVicCharInfo	= new MMatchCharInfo;

	pAttCharInfo->m_nCID = 10;
	pVicCharInfo->m_nCID = 11;

	pAttacker->GetAccountInfo()->m_nAID = 1;
	pAttacker->GetAccountInfo()->m_bIsPowerLevelingHacker = true;
	pAttacker->GetAccountInfo()->m_nPowerLevelingRegTimeMin = m_dwCurTime - (1000 * 60 * 10);
	pAttacker->SetCharInfo( pAttCharInfo );


	pVictim->GetAccountInfo()->m_nAID = 2;
	pVictim->GetAccountInfo()->m_bIsPowerLevelingHacker = true;
	pVictim->GetAccountInfo()->m_nPowerLevelingRegTimeMin = m_dwCurTime - (1000 * 60 * 10);
	pVictim->SetCharInfo( pVicCharInfo );

	AddTestObject( "PLAttacker", "MMatchObject", (void*)pAttacker );
	AddTestObject( "PLVictim", "MMatchObject", (void*)pVictim );
}


void MBMatchTest::DoTest_PowerLeveling( const DWORD dwCurTime )
{
	static DWORD dwLastUpdateTime = dwCurTime;

	const DWORD dwMaxKillCount	= MGetServerConfig()->GetKillTrackerConfig().GetMaxKillCountOnTraceTime();
	const DWORD dwTraceTime		= MGetServerConfig()->GetKillTrackerConfig().GetKillCountTraceTime();
	const DWORD dwDelay			= (dwTraceTime / dwMaxKillCount) + (dwTraceTime / 10); /// 10%의 오차 적용.

	TESTOBJECT* pAttacker	= (TESTOBJECT*)GetTestObject( "PLAttacker" );
	TESTOBJECT* pVictim		= (TESTOBJECT*)GetTestObject( "PLVictim" );

	mlog( "\n// KILL TRACKER TEST //////////////////////////////////////////////\n" );
	mlog( "// MAX KILL COUNT : %u\n", dwMaxKillCount );

	for( DWORD i = 0; i < (dwMaxKillCount * 2); ++i )
	{
		_ASSERT( false == testIsPowerLevelingHacker(pAttacker, pVictim) );
		testPowerLevelingHackerState( pAttacker, pVictim );
		_ASSERT( true == testIncreaseKillCount(pAttacker, pVictim, dwLastUpdateTime) );

		dwLastUpdateTime += dwDelay;	
	}

	m_pServer->m_KillTracker.Release();

	mlog( "// KILL TRACKER TEST END /////////////////////////////////////////\n\n" );
}


bool MBMatchTest::testIsPowerLevelingHacker( TESTOBJECT* pAttObj, TESTOBJECT* pVicObj )
{
	MMatchObject* pAttacker	= (MMatchObject*)pAttObj->pObject;
	MMatchObject* pVictim	= (MMatchObject*)pVicObj->pObject;

	bool IsHacking = false;

	if( m_pServer->IsPowerLevelingHacker(pAttacker) )
		IsHacking = true;
	
	if( m_pServer->IsPowerLevelingHacker(pVictim) )
		IsHacking = true;

	return IsHacking;
}


bool MBMatchTest::testPowerLevelingHackerState( TESTOBJECT* pAttObj, TESTOBJECT* pVicObj )
{
	MMatchObject* pAttacker	= (MMatchObject*)pAttObj->pObject;
	MMatchObject* pVictim	= (MMatchObject*)pVicObj->pObject;

	if( pAttacker->GetAccountInfo()->m_bIsPowerLevelingHacker )
	{
		//mlog( "TEST PowerLevelingHacker : Attacker(flag(%d), elapsed(%u)).\n"
		//	, pAttacker->GetAccountInfo()->m_bIsPowerLevelingHacker
		//	, (m_dwCurTime - pAttacker->GetAccountInfo()->m_dwPowerLevelingRegDate) / (1000 * 60) );
	}

	if( pVictim->GetAccountInfo()->m_bIsPowerLevelingHacker )
	{
		//mlog( "TEST PowerLevelingHacker : Victim(flag(%d), elapsed(%u)).\n"
		//	, pVictim->GetAccountInfo()->m_bIsPowerLevelingHacker
		//	, (m_dwCurTime - pVictim->GetAccountInfo()->m_dwPowerLevelingRegDate) / (1000 * 60) );
	}

	return true;
}


bool MBMatchTest::testIncreaseKillCount( TESTOBJECT* pAttObj, TESTOBJECT* pVicObj, const DWORD dwCurTime )
{
	static DWORD dwLastUpdateTime = m_dwCurTime;
	static DWORD dwElapsedTime = 0;

	MMatchObject* pAttacker	= (MMatchObject*)pAttObj->pObject;
	MMatchObject* pVictim	= (MMatchObject*)pVicObj->pObject;

	const DWORD dwAttackerCID	= pAttacker->GetCharInfo()->m_nCID;
	const DWORD dwVictimCID		= pVictim->GetCharInfo()->m_nCID;
	const DWORD dwKillCount		= m_pServer->GetKillTracker().GetKillCount( dwAttackerCID, dwVictimCID );

	if( (0 == dwKillCount) || (1 == dwKillCount) )
	{
		static DWORD dwLastInitTime = m_dwCurTime;

		// mlog( "TEST : Init KillCount start time. Elapse(%u).\n", (m_dwCurTime - dwLastInitTime) / (1000 * 60) );

		pAttObj->dwStartCheckKillCountTime = m_dwCurTime;
		pVicObj->dwStartCheckKillCountTime = m_dwCurTime;

		dwElapsedTime = 0;
	}

	if( !m_pServer->IncreaseAttackerKillCount(pAttacker, pVictim, dwCurTime) )
	{
		// AE : attacker elapsed time.
		// VE : victim elapsed time.
		// C  : Kill Count.
		//mlog( "TEST KillTracker : Hacking! AE(%u), VE(%u), C(%u).\n"
		//	, (m_dwCurTime - pAttObj->dwStartCheckKillCountTime) / (1000 * 60)
		//	, (m_dwCurTime - pVicObj->dwStartCheckKillCountTime) / (1000 * 60)
		//	, dwKillCount );
		return false;
	}

	dwElapsedTime += (m_dwCurTime - dwLastUpdateTime) / 1000;
	dwLastUpdateTime = m_dwCurTime;

	// mlog( "TEST KillCount : A(%u), V(%u), C(%u), EL(%usec)\n", dwAttackerCID, dwVictimCID, dwKillCount, dwElapsedTime );

	return true;
}