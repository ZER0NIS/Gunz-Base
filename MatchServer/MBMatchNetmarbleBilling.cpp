#include "StdAfx.h"
#include "MBMatchNetmarbleBilling.h"

MBMatchNetmarbleBilling::MBMatchNetmarbleBilling()
{
	InitializeCriticalSection(&m_cs);	
}

MBMatchNetmarbleBilling::~MBMatchNetmarbleBilling()
{
	EnterCriticalSection(&m_cs);
	{
		MBMatchNetmarbleBillingListIterator iter;
		while( iter != m_RequestList.end() )
		{
			delete (MBMatchNetmarbleBillingRequest*)(*iter);
			iter = m_RequestList.erase(iter);
		}
	}
	LeaveCriticalSection(&m_cs);

	DeleteCriticalSection(&m_cs);
}

MBMatchNetmarbleBilling& MBMatchNetmarbleBilling::GetInstance()
{
	static MBMatchNetmarbleBilling m_stNetmarbleBilling;
	return m_stNetmarbleBilling;
}

bool MBMatchNetmarbleBilling::Create()
{
	DWORD dwThreadId=0;
	m_hThread = CreateThread(NULL, 0, WorkerThread, this, 0, &dwThreadId);
	if (m_hThread == NULL) {
		mlog("## FAILED TO CREATE NETMARBLE BILLING THREAD ##");
		_ASSERT(0);
		return false;
	}

	return true;
}

void MBMatchNetmarbleBilling::Destroy()
{
	WaitForSingleObject(m_hThread, INFINITE);
	TerminateThread(m_hThread, 0);
	CloseHandle(m_hThread);	
}

void MBMatchNetmarbleBilling::AddRequest(MBMatchNetmarbleBillingRequest* pRequest)
{
	EnterCriticalSection(&m_cs);
	{
		m_RequestList.push_back(pRequest);
	}		
	LeaveCriticalSection(&m_cs);
}

MBMatchNetmarbleBillingRequest* MBMatchNetmarbleBilling::GetRequest()
{
	MBMatchNetmarbleBillingRequest* pRequest = NULL;

	EnterCriticalSection(&m_cs);
	{
		MBMatchNetmarbleBillingListIterator iter;
		if( m_RequestList.empty() == false )
		{
			iter = m_RequestList.begin();
			pRequest = (MBMatchNetmarbleBillingRequest*)(*iter);
			m_RequestList.pop_front();
		}
	}
	LeaveCriticalSection(&m_cs);

	return pRequest;	
}

void MBMatchNetmarbleBilling::RequestAuth()
{
	MBMatchNetmarbleBillingRequest* pRequest;
	pRequest = new MBMatchNetmarbleBillingRequest;

	AddRequest(pRequest);
}


DWORD WINAPI MBMatchNetmarbleBilling::WorkerThread(LPVOID pContext)
{
	MBMatchNetmarbleBilling* pNetmarbleBilling = (MBMatchNetmarbleBilling*)pContext;
	_ASSERT(pNetmarbleBilling);

	MBMatchNetmarbleBillingRequest* pRequest = NULL;

	while(1)
	{
		Sleep(1);
/*
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
*/
	}

	return 0;
} 