#include "stdafx.h"
#include "TestResourceCRC32Cache_ServerClient.h"
#include "TestCRC32Server.h"
#include "TestCRCPlayer.h"




TestCRCServer	g_tServer;

MUID			g_uidMainPlayer;
MUID			g_uidTester1;
MUID			g_uidTester2;
MUID			g_uidTester3;

MUID			g_uidMainStage;


void ShowExceptionMsg( CException* pException )
{
	TCHAR szErr[ 1024 ] = {0,};
	pException->GetErrorMessage( szErr, 1024 );
	mlog( "Memory exception - %s\n", szErr );
}



const bool AddItemDesc( const int nItemID )
{
	MMatchItemDesc* pItemDesc = NULL;

	try
	{
		pItemDesc = new MMatchItemDesc;
		if( NULL == pItemDesc )
		{
			return false;
		}
	}
	catch (CMemoryException* e)
	{
		ShowExceptionMsg( e );
		return false;
	}
	
	memset( pItemDesc, 0, sizeof(MMatchItemDesc) );

	pItemDesc->m_nID = nItemID;

	sprintf( pItemDesc->m_szDesc, "ItemDesc.nItemID = %d", nItemID );
	
	_ASSERT( g_tServer.AddItemDesc(nItemID, pItemDesc) );
	
	return true;
}


const bool AddPlayer( const MUID uidPlayer )
{
	_ASSERT( MUID(0, 0) != uidPlayer );

	try
	{
		TestCRCPlayer* ptMainPlayer = new TestCRCPlayer( uidPlayer );
		_ASSERT( g_tServer.AddPlayer(ptMainPlayer) );
	}
	catch (CException* e)
	{
		ShowExceptionMsg( e );
		return false;
	}

	return true;
}


//  테스트 용으로 3개까지.
void Equip( const MUID uidPlayer, const int nE1, const int nE2, const int nE3 )
{
	TestCRCPlayer* pPlayer = g_tServer.GetPlayer( uidPlayer );
	_ASSERT( NULL != pPlayer );

	if( 0 != nE1 )
	{
		MMatchItemDesc* pItemDesc1 = g_tServer.GetItemDesc( nE1 );
		_ASSERT( 0 != pItemDesc1 );
		_ASSERT( pPlayer->Equip(pItemDesc1) );
	}

	if( 0 != nE2 )
	{
		MMatchItemDesc* pItemDesc2 = g_tServer.GetItemDesc( nE2 );
		_ASSERT( 0 != pItemDesc2 );
		_ASSERT( pPlayer->Equip(pItemDesc2) );
	}
	
	if( 0 != nE3 )
	{
		MMatchItemDesc* pItemDesc3 = g_tServer.GetItemDesc( nE3 );
		_ASSERT( 0 != pItemDesc3 );
		_ASSERT( pPlayer->Equip(pItemDesc3) );
	}
}


const bool CreateStage( const string& strStageName, const MUID& uidStage, const MUID uidMaster )
{
	TestCRCPlayer* pMaster = g_tServer.GetPlayer( uidMaster );
	_ASSERT( NULL != pMaster );

	TestCRCStage* pStage = new TestCRCStage( uidStage );
	_ASSERT( NULL != pStage );

	pStage->SetMasterUID( uidMaster );
	pStage->SetName( strStageName );

	pStage->AddPlayer( uidMaster );

	return g_tServer.AddStage( pStage );
}


void EnterBattle( const MUID uidStage, const MUID uidPlayer )
{
	TestCRCStage* pStage = g_tServer.GetStage( uidStage );
	_ASSERT( NULL != pStage );

	pStage->EnterBattle( g_tServer, uidPlayer );
}


const bool ResponseToServer_ResourceCRC32Cache( const MUID uidStage, const MUID uidPlayer )
{
	TestCRCStage* pStage = g_tServer.GetStage( uidStage );
	if( NULL == pStage )
	{
		return false;
	}

	DWORD dwResourceCRC32Cache, dwResourceXORCache;
	g_tServer.GetTestNetModule().GetPlayerResourceCRC32Cache( uidPlayer, dwResourceCRC32Cache, dwResourceXORCache);
	if( 0 == dwResourceCRC32Cache )
	{
		return false;
	}
	
	return pStage->IsValidResourceCRC32Cache( uidPlayer, dwResourceCRC32Cache, dwResourceXORCache );
}


void DoTestServerAndClient()
{
	for( int i = 0; TEST_ITEMDESC_COUNT > i; ++i )
	{
		_ASSERT( AddItemDesc(i) );
	}


	g_uidMainPlayer = g_tServer.CreateNewUID();
	g_uidTester1	= g_tServer.CreateNewUID();
	g_uidTester2	= g_tServer.CreateNewUID();
	g_uidTester3	= g_tServer.CreateNewUID();


	_ASSERT( AddPlayer(g_uidMainPlayer) );
	_ASSERT( AddPlayer(g_uidTester1) );
	_ASSERT( AddPlayer(g_uidTester2) );
	_ASSERT( AddPlayer(g_uidTester3) );


	Equip( g_uidMainPlayer, 1, 2, 3 );
	Equip( g_uidTester1, 4, 5, 6 );
	Equip( g_uidTester2, 7, 8, 9 );
	Equip( g_uidTester3, 10, 11, 12 );

	
	g_uidMainStage = g_tServer.CreateNewUID();
	_ASSERT( CreateStage("Test1", g_uidMainStage, g_uidMainPlayer) );
	_ASSERT( g_tServer.JoinStage(g_uidMainStage, g_uidTester1) ); 

	
	EnterBattle( g_uidMainStage, g_uidMainPlayer );
	EnterBattle( g_uidMainStage, g_uidTester1 );
	

	_ASSERT( g_tServer.JoinStage(g_uidMainStage, g_uidTester2) ); 
	_ASSERT( g_tServer.JoinStage(g_uidMainStage, g_uidTester3) ); 


	EnterBattle( g_uidMainStage, g_uidTester3 );
	EnterBattle( g_uidMainStage, g_uidTester2 );
	

	_ASSERT( ResponseToServer_ResourceCRC32Cache(g_uidMainStage, g_uidTester3) );
	_ASSERT( ResponseToServer_ResourceCRC32Cache(g_uidMainStage, g_uidTester1) );
	_ASSERT( ResponseToServer_ResourceCRC32Cache(g_uidMainStage, g_uidMainPlayer) );
	_ASSERT( ResponseToServer_ResourceCRC32Cache(g_uidMainStage, g_uidTester2) );


	g_tServer.ReleasePlayerMap();
	g_tServer.ReleaseItemDescMap();
	g_tServer.ReleaseStageMap();
}