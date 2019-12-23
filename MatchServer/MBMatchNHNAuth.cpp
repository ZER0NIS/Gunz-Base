#include "stdafx.h"
#include "stdafx.h"
#include "MBMatchNHNAuth.h"
#include "MDebug.h"
#include "MMatchGlobal.h"


#include ".\\nhn\\HanAuthForSvr.h"


#ifndef _DEBUG
#pragma comment( lib, ".\\nhn\\HanAuthForSvr.lib" )
#else
#pragma comment( lib, ".\\nhn\\HanAuthForSvrD.lib" )
#endif


#ifndef _DEBUG
#define DEFAULT_SERVICE_TYPE (SERVICE_USA | SERVICE_REAL)
#else
#define DEFAULT_SERVICE_TYPE (SERVICE_USA | SERVICE_ALPHA)
#endif




MBMatchNHMModule::MBMatchNHMModule()
{
}


MBMatchNHMModule::~MBMatchNHMModule()
{
}


bool MBMatchNHMModule::InitAuth( const int nServerMode )
{
	// const int nRet = HanAuthInit( NHN_GAMEID, DEFAULT_SERVICE_TYPE, 10 );
	// const int nRet = HanAuthInit( NHN_GAMEID, SERVICE_USA | SERVICE_ALPHA, 10 );

	int nRetInitAuth = false;
	if( SERVICE_REAL == nServerMode )
		nRetInitAuth  = HanAuthInit( NHN_GAMEID, SERVICE_USA | SERVICE_REAL, 10 );
	else if( SERVICE_ALPHA == nServerMode )
		nRetInitAuth  = HanAuthInit( NHN_GAMEID, SERVICE_USA | SERVICE_ALPHA, 10 );
	else
	{
		ASSERT( 0 );
		mlog( "nhn server mode is error...\n" );
		return false;
	}

	if( 0 != nRetInitAuth )
	{
		WriteAuthLog( nRetInitAuth  );
		return false;
	}

	mlog( "NHNAuth init completed.\n" );

	return true;
}


bool MBMatchNHMModule::IsAuthUser( const char* pszUserID, const char* pszAuthString )
{
	if( 0 == pszUserID ) return false;
	if( 0 == pszAuthString ) return false;
	if( NHN_AUTH_LENGTH < strlen(pszAuthString) ) return false;

	char szOutBuff[ NHN_OUTBUFF_LENGTH ] = {0,};

	const int nRet = HanAuthForSvr( const_cast<char*>(pszUserID), const_cast<char*>(pszAuthString), szOutBuff, NHN_OUTBUFF_LENGTH );

	if( 0 != nRet )
	{
		MGetMatchServer()->LOG(MMatchServer::LOG_PROG, "MBMatchNHMModule::IsAuthUser - OutBuffer=%s", szOutBuff );
		WriteAuthLog( nRet );
		return false;
	}

	return true;
}


void MBMatchNHMModule::WriteAuthLog( const int nValue )
{
	char szLog[ 256 ]		= {0,};
	char szErrCode[ 64 ]	= {0,};

	switch( nValue )
	{
	case -1 : 
		sprintf( szErrCode, "general fault." );
		break;

	case 1 : 
		sprintf( szErrCode, "parameter of auth is invalid." );
		break;

	case 2 :
		sprintf( szErrCode, "this ip address is invalid." );
		break;

	case 3 : 
		sprintf( szErrCode, "memberID is invalid." );
		break;

	case 4 :
		sprintf( szErrCode, "password incorrect." );
		break;

	case 5 :
		sprintf( szErrCode, "password mismatch( over 3 times )." );
		break;

	case 6 :
		sprintf( szErrCode, "memberID is not HangameID." );
		break;

	case 7 :
		sprintf( szErrCode, "system error." );
		break;

	default :
		sprintf( szErrCode, "Not Defined." );
		break;
	};

	MGetMatchServer()->LOG(MMatchServer::LOG_PROG, "Authentic fail. error code(%d) : %s\n", nValue, szErrCode);
}


bool MBMatchNHMModule::InitAuthAlpha()
{
	return InitAuth( SERVICE_ALPHA );
}


bool MBMatchNHMModule::InitAuthReal()
{
	return InitAuth( SERVICE_REAL );
}



MBMatchNHMModule& GetNHNModule() 
{
	return MBMatchNHMModule::GetInstance();
}