#include "stdafx.h"
#include "MBMatchAsyncDBJob_NHNLogin.h"
#include "MBMatchNHNAuth.h"
#include "MMatchObject.h"
#include "MMatchDBMgr.h"


bool MBMatchAsyncDBJob_NHNLogin::Input( const MUID& uidCommUID, const string& strUserID, const string& strAuth, const int nCheckSum,
									   const bool bIsFreeLoginIP, const string strCountryCode3, const bool bIsUseNHNUSAAuth, const int nServerID )
{
	m_CommUID			= uidCommUID;
	m_strUserID			= strUserID;
	m_strAuth			= strAuth;
	m_nCheckSum			= nCheckSum;
	m_bIsFreeLoginIP	= bIsFreeLoginIP;
	m_strCountryCode3	= strCountryCode3;
	m_bIsUseNHNUSAAuth	= bIsUseNHNUSAAuth;
	m_nServerID			= nServerID;

	m_pAccountInfo = new MMatchAccountInfo;
	if( 0 == m_pAccountInfo ) return false;

	m_pAccountPenaltyInfo = new MMatchAccountPenaltyInfo;
	if( 0 == m_pAccountPenaltyInfo ) return false;

	return true;
}


MBMatchAsyncDBJob_NHNLogin::~MBMatchAsyncDBJob_NHNLogin()
{
	DeleteMemory();
}


void MBMatchAsyncDBJob_NHNLogin::Run( void* pContext )
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

    if( m_bIsUseNHNUSAAuth && (!GetNHNModule().IsAuthUser(m_strUserID.c_str(), m_strAuth.c_str())) )
	{
		mlog( "MBMatchAsyncDBJob_NHNLogin - not nhn auth user.\n" );
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

	TCHAR szDBPassword[ 20 ] = {0,};
	if( !pDBMgr->GetLoginInfo(m_strUserID.c_str(), &m_nAID, szDBPassword) )
	{
		// 처음 접속하는 유저.

		// int nGunzSex;	// 건즈디비의 성별값은 넷마블 성별값과 반대이다.
		// if (m_nSex == 0) nGunzSex = 1; else nGunzSex = 0;

		// int nCert = 0;
		// if (strlen(m_szCertificate) > 0)
		// {
		//	if (m_szCertificate[0] == '1') nCert =1;
		// }

		// 지금은 다른 userid외에는 알수 있는 방법이 없어서 임시로 이렇게 한다.
		pDBMgr->CreateAccount(
            	m_strUserID.c_str(),	// user id
				"",						// password
				1,						// cert
				"",						// name
				0,						// age
				0);						//sex
		
		pDBMgr->GetLoginInfo(m_strUserID.c_str(), &m_nAID, szDBPassword);
	}

	// 계정 정보를 읽는다.
	if (!pDBMgr->GetAccountInfo(m_nAID, m_pAccountInfo, m_nServerID))
	{
		mlog( "MBMatchAsyncDBJob_NHNLogin - can't find accont.\n" );
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

	// 계정 페널티 정보를 읽는다. - 2010-08-10 홍기주
	if( !pDBMgr->GetAccountPenaltyInfo(m_nAID, m_pAccountPenaltyInfo) ) 
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}


void MBMatchAsyncDBJob_NHNLogin::DeleteMemory()
{
	if( 0 != m_pAccountInfo )
	{
		delete m_pAccountInfo;
		m_pAccountInfo = 0;
	}

	if( 0 != m_pAccountPenaltyInfo )
	{
		delete m_pAccountPenaltyInfo;
		m_pAccountPenaltyInfo = NULL;
	}
}