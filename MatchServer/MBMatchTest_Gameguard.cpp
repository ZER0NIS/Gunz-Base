#include "stdafx.h"
#include "MBMatchTest.h"
#include "MMatchObject.h"
#include "MDebug.h"
#include "MBMatchServer.h"
#include "MMatchKillTrackerConfig.h"
#include "MMatchConfig.h"
#include "MErrorTable.h"
#include "MSharedCommandTable.h"
#include "MCommand.h"
#include "MUID.h"


void MBMatchTest::InitGameguardTest()
{
	MUID uid = m_pServer->UseUID();

	_ASSERT( MOK == m_pServer->ObjectAdd(uid) );

	MMatchObject* pObj = m_pServer->GetObject( uid );
	_ASSERT( NULL != pObj);

	AddTestObject( "GGObject", "MMatchObject", (void*)pObj );
}


void MBMatchTest::DoTest_GameguardFirstRequest( const DWORD dwCurTime )
{

}