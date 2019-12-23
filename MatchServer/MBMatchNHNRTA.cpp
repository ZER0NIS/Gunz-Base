#include "stdafx.h"
#include "MBMatchNHNRTA.h"
#include ".\\nhn\\HanRTA.h"
#include "MDebug.h"
#include "MMatchGlobal.h"


#ifndef _DEBUG
#pragma comment( lib, ".\\nhn\\HanRTA.lib" )
#else
#pragma comment( lib, ".\\nhn\\HanRTAD.lib" )
#endif



MBMatchNHNRTA::MBMatchNHNRTA()
{
	m_dwLastRTATime = 0;
}


MBMatchNHNRTA::~MBMatchNHNRTA()
{
}


bool MBMatchNHNRTA::InitRTA( const char* szServerName, const int nNHNServerMode )
{
	if( (0 == szServerName) || (64 < strlen(szServerName)) )
	{
		mlog( "MBMatchNHNRTA::InitRTA - Server name is invalid...\n" );
		return false;
	}

	int nRetInitRep = false;
	if( SERVICE_REAL == nNHNServerMode )
		nRetInitRep = HanRTAInit( NHN_GAMEID, SERVICE_USA | SERVICE_REAL, 10 );
	else if( SERVICE_ALPHA == nNHNServerMode )
		nRetInitRep = HanRTAInit( NHN_GAMEID, SERVICE_USA | SERVICE_ALPHA, 10 );
	if( 0 != nRetInitRep )
	{
		mlog( "MBMatchNHNRTA::InitRTA - HanRTAInit fail...\n" );
		return false;
	}

	strcpy( m_szServerName, szServerName );

	mlog( "NHN RTA Init is completed. ServerName=%s.\n", szServerName );

	return true;
}


bool MBMatchNHNRTA::RTA( const int nPlayerCount, const DWORD dwCurTime )
{
	char szRTA[ 1024 ] = {0,};

	sprintf( szRTA, "gameid=%s&servername=%s&conncount=%d",
		NHN_GAMEID, m_szServerName, nPlayerCount );

	const int nRet = HanRTA( szRTA );

	m_dwLastRTATime = dwCurTime;
	
	if( -1 == nRet )
	{
		mlog( "NHN RTA fault from  function or this system.\n" );
		return false;
	}
	else if( 1 == nRet )
	{
		mlog( "NHN RTA fault from auth server.\n" );
		return false;
	}

	return true;
}


bool MBMatchNHNRTA::IsElapsed( const DWORD dwCurTime )
{
	if( MAX_RTA_DELAY_TIME > (dwCurTime - m_dwLastRTATime) )
		return false;
	
	return true;
}


bool MBMatchNHNRTA::InitRTAReal( const char* szServerName )
{
	return InitRTA( szServerName, SERVICE_REAL );
}


bool MBMatchNHNRTA::InitRTAAlpha( const char* szServerName )
{
	return InitRTA( szServerName, SERVICE_ALPHA );
}


MBMatchNHNRTA& GetNHNRTA()
{
	return MBMatchNHNRTA::GetInstance();
}