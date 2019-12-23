#ifndef _MBMATCHTEST_H
#define _MBMATCHTEST_H


#include <map>
#include <string>

using std::map;
using std::string;


struct TESTOBJECT
{
	string	strTypeName;
	void*	pObject;

	DWORD	dwStartCheckKillCountTime;
	
	void DeleteMyObject();
};


class MMatchObject;
class MBMatchServer;



typedef map< string, TESTOBJECT* > TestObjectPointMap;

class MBMatchTest
{
private :
	MBMatchServer*		m_pServer;
	DWORD				m_dwCurTime;

	TestObjectPointMap	m_ObjMap;

private :
	void AddTestObject( const string& strName, const string& strTypeName, void* pObject );
	TESTOBJECT* GetTestObject( const string& strName );

public :
	MBMatchTest()
	{
		m_dwCurTime = 0;
	}

	void Init( MBMatchServer* pServer );
	void Release();

	void DoTest( const DWORD dwCurTime );


public :
	void InitPowerLevelingTest();
	void InitGameguardTest();

	void DoTest_PowerLeveling( const DWORD dwCurTime );
	void DoTest_GameguardFirstRequest( const DWORD dwCurTime );

	bool testIsPowerLevelingHacker( TESTOBJECT* pAttObj, TESTOBJECT* pVicObj );
	bool testIncreaseKillCount( TESTOBJECT* pAttObj, TESTOBJECT* pVicObj, const DWORD dwCurTime );
	bool testPowerLevelingHackerState( TESTOBJECT* pAttObj, TESTOBJECT* pVicObj );
};


void DeleteMMatchObject( void* p );
void DeleteTestObject( const string& strName, void* p );


#endif