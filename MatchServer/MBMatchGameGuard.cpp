#include "stdafx.h"
#include "MBMatchGameGuard.h"
#include "MBMatchGGSvrRef.h"
#include "mmsystem.h"
#include "MDebug.h"


#include ".\\Gameguard\\ggerror.h"



//-----------------------------------------------------------------------------------------------


MBMatchGameguard::MBMatchGameguard( const DWORD dwCurTime ) : MBMatchUserSecurityInfo( dwCurTime )
{
	m_bIsInit				= false;
	m_dwLastError			= 0;
	m_pServerAuth			= NULL;
	m_dwCreateAuthCount		= 0;
}



MBMatchGameguard::~MBMatchGameguard()
{
	if( true == m_bIsInit )
		Close();
}


const bool MBMatchGameguard::Init()
{
	if( m_bIsInit )
	{
		ASSERT( 0 && "duplicated init" );
	}


	if( NULL == m_pServerAuth )
	{
		m_pServerAuth = new MBMatchGGSvrRef;
		if( 0 == m_pServerAuth )
		{
			mlog( "Create gameguard fail.\n" );
			return false;
		}
	}
	else
	{
		m_pServerAuth->Close();
	}

	m_pServerAuth->Init();
	
	m_bIsInit = true;

	return true;
}


void MBMatchGameguard::Close()
{
#ifdef _DEBUG
	static int nCloseCnt = 0;
	++nCloseCnt;
#endif

	if( false == m_bIsInit )
	{
#ifdef _DEBUG
		if( 2 < nCloseCnt )
		{
			ASSERT( 0 && "duplicated close" );
		}
#endif
		return;
	}

	if( NULL != m_pServerAuth )
	{
		m_pServerAuth->Close();
		delete m_pServerAuth;
		m_pServerAuth = NULL;
	}

	m_bIsInit = false;
}


bool MBMatchGameguard::CreateAuthQuery()
{
	const DWORD dwRet = m_pServerAuth->CreateAuthQuery();
	if( ERROR_SUCCESS != dwRet )
	{
		m_dwLastError = dwRet;
		return false;
	}

	++m_dwCreateAuthCount;
	
	return true;
}


bool MBMatchGameguard::CheckAuthAnswer( const DWORD dwCurTime )
{
	UpdateLastCheckTime( dwCurTime ); 

	const DWORD dwRet= m_pServerAuth->CheckAuthAnswer();
	if( ERROR_SUCCESS != dwRet )
	{
		m_dwLastError = dwRet;
		return false;
	}
	
    return true; 
}


void MBMatchGameguard::SetAuthAnswer( const DWORD dwIndex, const DWORD dwValue1, const DWORD dwValue2, const DWORD dwValue3 )
{
	m_pServerAuth->SetAuthAnswer( dwIndex, dwValue1, dwValue2, dwValue3 );
}


//------------------------------------------------------------------------------------------------


const DWORD InitMBMatchGameguardAuth()
{
	// const DWORD dwRetInitGameguardAuth = InitGameguardAuth( GAMEGUARD_DLL_PATH, ACTIVE_NUM );
	const DWORD dwRetInitGameguardAuth = InitGameguardAuth( GAMEGUARD_DLL_PATH, ACTIVE_NUM, true, 0x03 );
	if( ERROR_SUCCESS != dwRetInitGameguardAuth )
	{
		// 초기화 실패 처리를 해줘야 함

		return dwRetInitGameguardAuth;
	}

#define GAMEGUARD_ACTIVE_UPDATE_TIME (30) // 분.
#define MIN_UPDATED_PERCENT (50)

	SetUpdateCondition( GAMEGUARD_ACTIVE_UPDATE_TIME, MIN_UPDATED_PERCENT );

	return dwRetInitGameguardAuth;
}


void CleanupMBMatchGameguardAuth()
{
	CleanupGameguardAuth();
}


void UpdateGambeguard( const DWORD dwCurTime )
{
#define MAX_UPDATE_GAMEGUARD_ELAPSED_TIME (5 * 60 * 1000)
	static DWORD dwLastUpdateTime = 0;
	if( MAX_UPDATE_GAMEGUARD_ELAPSED_TIME < (dwCurTime - dwLastUpdateTime) )
	{
		GGAuthUpdateTimer();
		dwLastUpdateTime = dwCurTime;
	}
}

GGAUTHS_API void GGAuthUpdateCallback(PGG_UPREPORT report)
{
	mlog("GGAuth version update [%s] : [%ld] -> [%ld] \n", // for examples
			report->nType==1?"GameGuard Ver":"Protocol Num", 
			report->dwBefore,
			report->dwNext); 
}


GGAUTHS_API void NpLog(int mode, char* msg)
{
//	if(mode & (NPLOG_DEBUG | NPLOG_ERROR)) // for examples 
//#ifdef WIN32
//		OutputDebugString(msg);
//#else
//		printf(msg); 
//#endif
}