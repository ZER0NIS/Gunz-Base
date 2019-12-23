#include "stdafx.h"
/*#include "MMonitor.h"
#include "MMonitorProtocolBuilder.h"
#include "MMonitorCommand.h"
#include "MMonitorCommandDefine.h"
#include "MBMatchMonitorCommunicator.h"
#include "MBMatchMonitor.h"
#include "MBMatchServer.h"
#include "MBMatchMonitorXml.h"
#include "MBMatchSystemInfo.h"
#include "MMatchConfig.h"
#include "MMonitorUDPXml.h"
#include "MBMatchAsyncJob_GetCpuInfo.h"
#include "MMatchDBMgr.h"

MMonitorCommandElement* MBMatchMonitor::OnRequestMonitorCommandElement( const MMonitorCommandElement* pCmdElement )
{
#ifdef _MONITORING
	if( NULL == pCmdElement )
		return NULL;

	MMonitorCommandElement* pResponseCmdElement = new MMonitorCommandElement;
	if (pResponseCmdElement == NULL)
	{
		return NULL;
	}

	// 모니터링 서버가 줘야하는 정보들 조사하는 작업 추가
	if( pCmdElement->GetProtocolID() == string("CheckDBConnectionToSVR") )
	{
		OnCheckDBConnectionToCmdElement(pCmdElement, pResponseCmdElement);
	}
	else if (pCmdElement->GetProtocolID() == string("CheckProcessRunningTimeToSVR"))
	{
		OnGetProcessRunningTimeToCmdElement(pCmdElement, pResponseCmdElement);
	}
	else if (pCmdElement->GetProtocolID() == string("CheckUserConnectionToSVR"))
	{
		OnGetUserConnectionCountToCmdElement(pCmdElement, pResponseCmdElement);
	}
	else if (pCmdElement->GetProtocolID() == string("CheckMemoryPerformanceToSVR"))
	{
		OnGetMemoryInfoToCmdElement(pCmdElement, pResponseCmdElement);
	}
	else if (pCmdElement->GetProtocolID() == string("CheckDiskPerformanceToSVR"))
	{
		OnGetDiskInfoToCmdElement(pCmdElement, pResponseCmdElement);
	}
	else if (pCmdElement->GetProtocolID() == string("CheckCpuPerformanceToSVR"))
	{
		string strTaskName = pCmdElement->GetTaskName();

		MBMatchAsyncJob_GetCpuInfo* pJob = new MBMatchAsyncJob_GetCpuInfo(pCmdElement->GetProtocolID(), strTaskName);
		if (pJob == 0)
		{
			DeleteMonitorCommandElement( pResponseCmdElement );
			return NULL;
		}

		MMatchServer *pMatchServer = (MBMatchServer*)MMatchServer::GetInstance();
		if (pMatchServer == NULL)
		{
			DeleteMonitorCommandElement( pResponseCmdElement );
			return NULL;
		}
		// 시간이 걸리는 작업은 프록시로 작업을 넘긴다.
		pMatchServer->PostAsyncJob(pJob);
		DeleteMonitorCommandElement( pResponseCmdElement );

		return NULL;
	}
	else
	{
		DeleteMonitorCommandElement( pResponseCmdElement );

		return NULL;
	}

	return pResponseCmdElement;
#else
	return NULL;
#endif
	
}

bool MBMatchMonitor::InitProtocolBuilder()
{
	MMonitorProtocolBuilder* pProtocolBuilder = new MMonitorProtocolBuilder;
	if( NULL == pProtocolBuilder )
		return false;

	MBMatchMonitorXmlDocument* pXmlDoc = new MBMatchMonitorXmlDocument;
	if( NULL == pXmlDoc )
	{
		pProtocolBuilder->Release();
		delete pProtocolBuilder;
	}

	if( !pProtocolBuilder->Init(pXmlDoc) )
	{
		pProtocolBuilder->Release();
		delete pProtocolBuilder;

		pXmlDoc->Destroy();
		delete pXmlDoc;

		return false;
	}

	SetMonitorProtocolBuilder( pProtocolBuilder );

	return true;
}

bool MBMatchMonitor::OnRun( const DWORD dwCurTime )
{
#ifdef _DEBUG
	static DWORD dwUp = dwCurTime;
	if( (1000 * 60) < (dwCurTime - dwUp) )
	{
		mlog( "MRecvQ(%u:%u), MOnCmdQ(%u:%u), MSendQ(%u:%u).\n"
			,GetTotalRecvCount(), GetRecvXmlListSize()
			, GetTotalOnCmdCount(), GetOnCommandXmlListSize()
			, GetTotalSendCount(), GetSendXmlListSize() );
		dwUp = dwCurTime;
	}
#endif

	return true;
}

void MBMatchMonitor::SafePushRecvUDP( const DWORD dwIP, const WORD wPort, const char* pData, const DWORD dwDataSize )
{
	MMonitorUDPXml* pXml = new MMonitorUDPXml;
	if( NULL == pXml )
		return;

	pXml->SetIP( dwIP );
	pXml->SetPortNtoH( wPort );
	pXml->SetXml( pData, dwDataSize );

	SafePushRecvXml( pXml );
}

/////////////////////////////////////////////////
MBMatchMonitor::MBMatchMonitor()
{
	m_pMatchSystemInfo = NULL;
}

MBMatchMonitor::~MBMatchMonitor()
{
}

bool MBMatchMonitor::OnInit()
{
#ifdef _MONITORING
	m_pMatchSystemInfo = new MBMatchSystemInfo;
	if (m_pMatchSystemInfo == NULL)
	{
		return false;
	}
	SetCommunicator(new MBMatchMonitorCommunicator);
#endif
	return true;
}

void MBMatchMonitor::OnRelease()
{
#ifdef _MONITORING
	if (m_pMatchSystemInfo != NULL)
	{
		delete m_pMatchSystemInfo;
		m_pMatchSystemInfo = NULL;
	}
#endif
}

void MBMatchMonitor::OnCheckDBConnectionToCmdElement(const MMonitorCommandElement* pRequestCmdElement, MMonitorCommandElement* pResponseCmdElement)
{
#ifdef _MONITORING
	char strDBConnect[100] = {0};
	char szExeFileName[MAX_PATH] = {0};
	m_pMatchSystemInfo->GetExeFileName(szExeFileName);

	if(MGetMatchServer()->GetDBMgr()->GetDBConnection())
	{
		sprintf(strDBConnect, "DATABASE_CONNECT:%s", szExeFileName);
	}
	else
	{
		sprintf(strDBConnect, "DATABASE_DISCONNECT:%s", szExeFileName);
	}

	string strTaskName = pRequestCmdElement->GetTaskName();

	DWORD dwTick = GetTickCount();
	char strTick[15] = {0};
	_itoa(dwTick, strTick, 10);

	pResponseCmdElement->SetProtocolID(pRequestCmdElement->GetProtocolID());
	pResponseCmdElement->ThisisResponse();
	pResponseCmdElement->AddParam(MMCXML_TRANSMIT_TASKNAME, strTaskName);
	pResponseCmdElement->AddParam(MMCXML_TRANSMIT_OUTPUTSTRING, strDBConnect);
	pResponseCmdElement->AddParam(MMCXML_TRANSMIT_LASTUPDATETIME, strTick);
#endif
}

void MBMatchMonitor::OnGetProcessRunningTimeToCmdElement(const MMonitorCommandElement* pRequestCmdElement, MMonitorCommandElement* pResponseCmdElement)
{
#ifdef _MONITORING
	char szExeFileName[MAX_PATH] = {0};
	m_pMatchSystemInfo->GetExeFileName(szExeFileName);

	SYSTEMTIME stRunnginTime;
	m_pMatchSystemInfo->GetProcessRunningTime(&stRunnginTime);

	char szRunningTime[100] = {0};
	sprintf(szRunningTime, "%dDay %dHour %dMin %dSec", stRunnginTime.wDay, stRunnginTime.wHour, stRunnginTime.wMinute, stRunnginTime.wSecond);
	char strRunningTime[100] = {0};
	sprintf(strRunningTime, "RUN_TIME:%s:%s", szExeFileName, szRunningTime);

	string strTaskName = pRequestCmdElement->GetTaskName();

	DWORD dwTick = GetTickCount();
	char strTick[15] = {0};
	_itoa(dwTick, strTick, 10);

	pResponseCmdElement->SetProtocolID(pRequestCmdElement->GetProtocolID());
	pResponseCmdElement->ThisisResponse();
	pResponseCmdElement->AddParam(MMCXML_TRANSMIT_TASKNAME, strTaskName);
	pResponseCmdElement->AddParam(MMCXML_TRANSMIT_OUTPUTSTRING, strRunningTime);
	pResponseCmdElement->AddParam(MMCXML_TRANSMIT_LASTUPDATETIME, strTick);
#endif
}

void MBMatchMonitor::OnGetMemoryInfoToCmdElement(const MMonitorCommandElement* pRequestCmdElement, MMonitorCommandElement* pResponseCmdElement)
{
#ifdef _MONITORING
	DWORD dwTotalMemMB;
	DWORD dwAvailMemMB;
	DWORD dwVirtualMemMB;
	m_pMatchSystemInfo->GetMemoryInfo(&dwTotalMemMB, &dwAvailMemMB, &dwVirtualMemMB);

	// 사용 중인 메모리 퍼센트 = ((사용 중인 메모리 용량 * 100) / 전체 메모리 용량)
	DWORD dwMemoryPersent = (dwAvailMemMB * 100) / dwTotalMemMB;
	char strMemory[100];
	sprintf(strMemory, "MEMORY_PERFORMANCE:%d", dwMemoryPersent);

	string strTaskName = pRequestCmdElement->GetTaskName();

	DWORD dwTick = GetTickCount();
	char strTick[15] = {0};
	_itoa(dwTick, strTick, 10);

	pResponseCmdElement->SetProtocolID(pRequestCmdElement->GetProtocolID());
	pResponseCmdElement->ThisisResponse();
	pResponseCmdElement->AddParam(MMCXML_TRANSMIT_TASKNAME, strTaskName);
	pResponseCmdElement->AddParam(MMCXML_TRANSMIT_OUTPUTSTRING, strMemory);
	pResponseCmdElement->AddParam(MMCXML_TRANSMIT_LASTUPDATETIME, strTick);
#endif
}

void MBMatchMonitor::OnGetDiskInfoToCmdElement(const MMonitorCommandElement* pRequestCmdElement, MMonitorCommandElement* pResponseCmdElement)
{
#ifdef _MONITORING
	char szDriveName = 'C';
	DWORD dwTotalDriveMB;
	DWORD dwFreeDriveMB;
	m_pMatchSystemInfo->GetDiskInfo(szDriveName, &dwTotalDriveMB, &dwFreeDriveMB);

	// 남아 있는 디스트용량 퍼센트 = (남은 디스크 용량 * 100) / 전체 디스크 용량)
	DWORD dwDiskPersent = (dwFreeDriveMB * 100) / dwTotalDriveMB;
	char strDisk[100] = {0};
	sprintf(strDisk, "DISK_PERFORMANCE:%d", dwDiskPersent);

	string strTaskName = pRequestCmdElement->GetTaskName();

	DWORD dwTick = GetTickCount();
	char strTick[15] = {0};
	_itoa(dwTick, strTick, 10);

	pResponseCmdElement->SetProtocolID(pRequestCmdElement->GetProtocolID());
	pResponseCmdElement->ThisisResponse();
	pResponseCmdElement->AddParam(MMCXML_TRANSMIT_TASKNAME, strTaskName);
	pResponseCmdElement->AddParam(MMCXML_TRANSMIT_OUTPUTSTRING, strDisk);
	pResponseCmdElement->AddParam(MMCXML_TRANSMIT_LASTUPDATETIME, strTick);
#endif
}

void MBMatchMonitor::OnGetUserConnectionCountToCmdElement(const MMonitorCommandElement* pRequestCmdElement, MMonitorCommandElement* pResponseCmdElement)
{
#ifdef _MONITORING
	MMatchServer *pMatchServer = (MBMatchServer*)MMatchServer::GetInstance();
	if (pMatchServer == NULL)
	{
		return;
	}
	int iClientCount = pMatchServer->GetClientCount();
	char strUserCount[100];
	sprintf(strUserCount, "USER_CONNECTION:%d", iClientCount);

	string strTaskName = pRequestCmdElement->GetTaskName();

	DWORD dwTick = GetTickCount();
	char strTick[15];
	_itoa(dwTick, strTick, 10);

	pResponseCmdElement->SetProtocolID(pRequestCmdElement->GetProtocolID());
	pResponseCmdElement->ThisisResponse();
	pResponseCmdElement->AddParam(MMCXML_TRANSMIT_TASKNAME, strTaskName);
	pResponseCmdElement->AddParam(MMCXML_TRANSMIT_OUTPUTSTRING, strUserCount);
	pResponseCmdElement->AddParam(MMCXML_TRANSMIT_LASTUPDATETIME, strTick);
#endif
}

// 프록시에서 완료가 된 결과를 모니터링 중계서버로 데이터를 전송한다.
void MBMatchMonitor::OnPostAsyncCpuUsage(string strProtocolID, string strTaskName, UINT uiCpuUsage)
{
#ifdef _MONITORING
	char strCpu[100];
	sprintf(strCpu, "CPU_PERFORMANCE:%d", uiCpuUsage);

	DWORD dwTick = GetTickCount();
	char strTick[15];
	_itoa(dwTick, strTick, 10);

	MMonitorCommand* pCmd = new MMonitorCommand;
    pCmd->SetIP(MGetServerConfig()->GetMonitorUDPIP());
	pCmd->SetPort(MGetServerConfig()->GetMonitorUDPPORT());

	MMonitorCommandElement* pResponseCmdElement = new MMonitorCommandElement;
	pResponseCmdElement->SetProtocolID(strProtocolID);
	pResponseCmdElement->ThisisResponse();
	pResponseCmdElement->AddParam(MMCXML_TRANSMIT_TASKNAME, strTaskName);
	pResponseCmdElement->AddParam(MMCXML_TRANSMIT_OUTPUTSTRING, strCpu);
	pResponseCmdElement->AddParam(MMCXML_TRANSMIT_LASTUPDATETIME, strTick);

	pCmd->AddCommandElement(pResponseCmdElement);

	PostMonitorCommand(pCmd);
#endif
}*/