#include "stdafx.h"
#include "MBMatchTest.h"
#include "MMatchObject.h"
#include "MDebug.h"
#include "MBMatchServer.h"
#include "MMatchKillTrackerConfig.h"
#include "MMatchConfig.h"
#include "TestCRC32XORCache.h"
#include "TestResourceCRC32Cache_ServerClient.h"


#include <utility>

using std::pair;





////////////////////////////////////////////////


void DeleteMMatchObject( void* p )
{
	MMatchObject* pObj = (MMatchObject*)(p);

	delete pObj;
}


void DeleteTestObject( const string& strName, void* p )
{
	if( string("MMatchObject") == strName )
		DeleteMMatchObject( p );
}


////////////////////////////////////////////////

void TESTOBJECT::DeleteMyObject()
{
	DeleteTestObject( strTypeName, pObject );
}

////////////////////////////////////////////////

void MBMatchTest::Init( MBMatchServer* pServer ) 
{ 
	_ASSERT( NULL != pServer );

	m_pServer	= pServer; 
	m_dwCurTime = pServer->GetGlobalClockCount();

	InitPowerLevelingTest();	
	// InitGameguardTest();
}


void MBMatchTest::AddTestObject( const string& strName, const string& strTypeName, void* pObject )
{
	TESTOBJECT* pTestObject = new TESTOBJECT;

	pTestObject->strTypeName				= strTypeName;
	pTestObject->pObject					= pObject;
	pTestObject->dwStartCheckKillCountTime	= 0;

	m_ObjMap.insert( pair<string, TESTOBJECT*>(strName, pTestObject) );

	mlog( "TEST : Add test object. Name(%s), TypeName(%s).\n"
		, strName.c_str()
		, pTestObject->strTypeName.c_str() );
}



TESTOBJECT* MBMatchTest::GetTestObject( const string& strName )
{
	TestObjectPointMap::iterator itFind = m_ObjMap.find( strName );
	if( m_ObjMap.end() == itFind )
	{
		_ASSERT( 0 );
		return NULL;
	}

	return itFind->second;
}


void MBMatchTest::Release()
{
	TestObjectPointMap::iterator it, end;
	end = m_ObjMap.end();
	for( it = m_ObjMap.begin(); it != end; ++it )
	{
		TESTOBJECT* pTestObject = (TESTOBJECT*)it->second;

		mlog( "TEST : Del test object. Name(%s), TypeName(%s).\n"
			, it->first.c_str()
			, pTestObject->strTypeName.c_str() );

		pTestObject->DeleteMyObject();

		delete pTestObject;
	}

	m_ObjMap.clear();
}


void MBMatchTest::DoTest( const DWORD dwCurTime )
{
	m_dwCurTime = dwCurTime;

	DoTest_PowerLeveling( dwCurTime );
	// DoTest_GameguardFirstRequest( dwCurTime );

	mlog( "\n== Start resources checksum test ==\n" );
	DoTestResourceCRC32Cache();
	DoTestServerAndClient();
	mlog( "== Resources checksum test OK! ==\n\n" );
}