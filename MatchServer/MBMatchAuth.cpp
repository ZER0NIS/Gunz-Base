#include "stdafx.h"
#include "MBMatchAuth.h"
//#include "Netmarble/CPSSOLib.h"
#include <stdlib.h>
#include "MDebug.h"
#include "MMatchStatus.h"
#include "CommonLog.h"

#ifndef NEW_AUTH_MODULE

/*bool MBMatchAuthBuilder::ParseAuthInfo(const char* pszData, MMatchAuthInfo** ppoutAutoInfo)
{
	bool bResult = true;
	MBMatchAuthInfo* pAuthInfo = new MBMatchAuthInfo;

	char szBuf[MAUTHINFO_BUFLEN] = {0,};
	if ( bResult &= GetCPCookieValue(pszData, "UserID", szBuf) ) {
		pAuthInfo->SetUserID(szBuf);
	}
	if ( bResult &= GetCPCookieValue(pszData, "UniID", szBuf) ) {
		pAuthInfo->SetUniqueID(szBuf);
	}
	if ( bResult &= GetCPCookieValue(pszData, "Sex", szBuf) ) {
		pAuthInfo->SetSex(atoi(szBuf));
	}
	if ( bResult &= GetCPCookieValue(pszData, "CCode", szBuf) ) {
		pAuthInfo->SetCCode(atoi(szBuf));		
	}
	if ( bResult &= GetCPCookieValue(pszData, "Age", szBuf) ) {
		pAuthInfo->SetAge(atoi(szBuf));
	}

	if (bResult) {
		*ppoutAutoInfo = pAuthInfo;
	}
	else {
		delete pAuthInfo;
		*ppoutAutoInfo = 0;
	}

	return bResult;
}

#else

#include "MBMatchAsyncDBJob_NetmarbleLogin.h"

// ※ 넷마블 제공 인증 라이브러리 매뉴얼 Gunz\MatchServer\Netmarble\NMAuthLib.chm 참조
MBMatchNetmarbleModule::MBMatchNetmarbleModule()
{
	InitializeCriticalSection(&m_cs);

	DWORD dwThreadId=0;
	m_hThread = CreateThread(NULL, 0, WorkerThread, this, 0, &dwThreadId);
	if (m_hThread == NULL)
	{
		Log("## FAILED TO CREATE NETMARBLE AUTH THREAD ##");
		_ASSERT(0);
	}
}

MBMatchNetmarbleModule::~MBMatchNetmarbleModule()
{
	WaitForSingleObject(m_hThread, INFINITE);

	TerminateThread(m_hThread, 0);
	CloseHandle(m_hThread);

	DestroyModule();
	DeleteCriticalSection(&m_cs);
}

MBMatchNetmarbleModule& MGetNetmarbleModule() 
{
	return MBMatchNetmarbleModule::GetInstance();
}

bool MBMatchNetmarbleModule::InitModule()
{
#ifndef _DEBUG
	ERROR_NMAUTH error;
	error = NMAuthLib::Initialize("gunz");	//넷마블에서 발급한 건즈의 gamecode 문자열

	// 인증 실패시..
	if ( error != ERROR_NMAUTH_SUCCESS )
	{
		this->ErrorLog(error, "init failed");
		return false;
	}
	MLog("Netmarble auth server connected.\n");
#endif
	return true;
}

void MBMatchNetmarbleModule::DestroyModule()
{
	NMAuthLib::Destroy();
}


void MBMatchNetmarbleModule::ErrorLog()
{
	// 넷마블 인증 모듈에서 준 에러코드를 그대로 로깅
	ErrorLog( m_NMAuth.GetLastError(), "");
}

void MBMatchNetmarbleModule::ErrorLog(ERROR_NMAUTH error, const char* szMsg)
{
	// 넷마블 인증 모듈이 준 에러코드와 추가 메시지를 로깅
	LPCTSTR szErrorCode;
	szErrorCode = NMAuthLib::ErrorCode2String(error);

	if (szErrorCode)
		mlog("Netmarble Auth Lib Error : %s (%s)\n", szMsg, szErrorCode);
	else
		mlog("Netmarble Auth Lib Error : %s (%d)\n", szMsg, error);
}

void MBMatchNetmarbleModule::RequestAuth(const MUID& commUid, const char* szAuth, const char* szData, const char* szCp,
										 bool bFreeLoginIP,
										 unsigned long nChecksumPack,
										 bool bCheckPremiumIP,
										 const char* szIP,
										 DWORD dwIP,
										 const char* szCountryCode3)
{
	MBMatchNetmarbleAuthQuery* pQuery = new MBMatchNetmarbleAuthQuery(
		commUid, szAuth, szData, szCp,
		bFreeLoginIP,
		nChecksumPack,
		bCheckPremiumIP,
		szIP,
		dwIP,
		szCountryCode3);

	EnterCriticalSection(&m_cs);
		m_queries.push_back(pQuery);
	LeaveCriticalSection(&m_cs);
}

MBMatchNetmarbleAuthQuery* MBMatchNetmarbleModule::GetNextQuery()
{
	MBMatchNetmarbleAuthQuery* pQuery = NULL;

	EnterCriticalSection(&m_cs);
		if (!m_queries.empty()) {
			pQuery = *m_queries.begin();
			m_queries.pop_front();
		}
	LeaveCriticalSection(&m_cs);

	return pQuery;
}

DWORD WINAPI MBMatchNetmarbleModule::WorkerThread(LPVOID pJobContext)
{
	MBMatchNetmarbleModule* pAuthModule = (MBMatchNetmarbleModule*)pJobContext;
	_ASSERT(pAuthModule);

	ERROR_NMAUTH error;
	MBMatchNetmarbleAuthQuery* pQuery = NULL;

	while(1)
	{
		Sleep(1);

		pQuery = pAuthModule->GetNextQuery();
		if (pQuery == NULL)
			continue;

		// 이전 데이터 파괴.
		pAuthModule->m_NMAuth.Destroy();

		//넷마블 인증모듈에 동기방식으로 요청
		error = pAuthModule->m_NMAuth.Init(pQuery->m_strAuth.c_str(), pQuery->m_strData.c_str(), pQuery->m_strCp.c_str());

		if ( error == ERROR_NMAUTH_SUCCESS )
		{
			// 타임아웃을 입력하지 않으면 기본값으로 처리.
			error = pAuthModule->m_NMAuth.LoadDataFromXML();

			// 성공하면 
			if ( error == ERROR_NMAUTH_SUCCESS )
			{
				//todok del
				//MLog("Netmarble login Auth SUCCEED by XML\n");

				// DB에 유저 정보를 질의
				MBMatchAsyncDBJob_NetmarbleLogin* pNewJob = new MBMatchAsyncDBJob_NetmarbleLogin(pQuery->m_commUid);
				pNewJob->Input(new MMatchAccountInfo(), 
					new MMatchAccountPenaltyInfo,
					pAuthModule->m_NMAuth.GetData("UserID"), 
					pAuthModule->m_NMAuth.GetData("UniID"), 
					pAuthModule->m_NMAuth.GetData("Certificate"), 
					pAuthModule->m_NMAuth.GetData("Name"), 
					atoi(pAuthModule->m_NMAuth.GetData("Age")), 
					atoi(pAuthModule->m_NMAuth.GetData("Sex")), 
					atoi(pAuthModule->m_NMAuth.GetData("CCode")), 
					pQuery->m_bFreeLoginIP, 
					pQuery->m_nChecksumPack,
					pQuery->m_bCheckPremiumIP,
					pQuery->m_strIP.c_str(),
					pQuery->m_dwIP,
					pQuery->m_strCountryCode3);

				MMatchServer::GetInstance()->PostAsyncJob(pNewJob);
			}
			else
			{
				// XML 데이터 로드 실패시 오류를 출력하고,
				pAuthModule->ErrorLog(error, "Netmarble Login Auth Failed (by XML)");

				// 쿠키로부터 데이터를 직접 추출한다.
				error = pAuthModule->m_NMAuth.LoadDataFromCookie();

				if ( error == ERROR_NMAUTH_SUCCESS )
				{
					//todok del
					//MLog("Netmarble login Auth SUCCEED by CPCOOKIE\n");

					// DB에 유저 정보를 질의
					MBMatchAsyncDBJob_NetmarbleLogin* pNewJob = new MBMatchAsyncDBJob_NetmarbleLogin(pQuery->m_commUid);
					pNewJob->Input(new MMatchAccountInfo(), 
						new MMatchAccountPenaltyInfo,
						pAuthModule->m_NMAuth.GetData("UserID"), 
						pAuthModule->m_NMAuth.GetData("UniID"), 
						pAuthModule->m_NMAuth.GetData("Certificate"), 
						pAuthModule->m_NMAuth.GetData("Name"), 
						atoi(pAuthModule->m_NMAuth.GetData("Age")), 
						atoi(pAuthModule->m_NMAuth.GetData("Sex")), 
						atoi(pAuthModule->m_NMAuth.GetData("CCode")), 
						pQuery->m_bFreeLoginIP, 
						pQuery->m_nChecksumPack,
						pQuery->m_bCheckPremiumIP,
						pQuery->m_strIP.c_str(),
						pQuery->m_dwIP,
						pQuery->m_strCountryCode3);

					MMatchServer::GetInstance()->PostAsyncJob(pNewJob);
				}
				else
				{
					// 실패처리
					MGetServerStatusSingleton()->SetRunStatus(5);

					// 원래는 이렇게 되어 있었는데 스레드에 안전하지 않아서 일단 뺌
					//MCommand* pCmd = MGetMatchServer()->CreateCmdMatchResponseLoginFailed(pQuery->m_commUid, MERR_CLIENT_WRONG_PASSWORD);
					//MGetMatchServer()->Post(pCmd);

					MLog("Netmarble Certification Failed\n");
				}
			}
		}

		delete pQuery;
	}
	return (0);
} 
*/
#endif