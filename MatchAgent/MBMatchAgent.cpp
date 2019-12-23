#include "stdafx.h"
#include "MatchAgent.h"
#include "MBMatchAgent.h"
#include "MatchAgentDoc.h"
#include "OutputView.h"
#include <atltime.h>
#include "MErrorTable.h"
#include "CommandLogView.h"
#include "MSharedCommandTable.h"
#include "MDebug.h"


bool MBMatchAgent::OnCreate(void)
{
	CMatchAgentApp* pApp = (CMatchAgentApp*)AfxGetApp();
	WriteAgentInfoLog();
	return true;
}

void MBMatchAgent::OnDestroy(void)
{
}

void MBMatchAgent::OnPrepareCommand(MCommand* pCommand)
{
#ifndef _DEBUG
	return;		// _DEBUG 버전아니면 리턴
#endif

	// 커맨드 로그 남기기
	if(m_pCmdLogView==NULL) return;

	if(pCommand->GetID() == MC_AGENT_TUNNELING_TCP) return;
	if(pCommand->GetID() == MC_AGENT_TUNNELING_UDP) return;

	CCommandLogView::CCommandType t=CCommandLogView::CCT_LOCAL;
	if(pCommand->m_pCommandDesc->IsFlag(MCDT_LOCAL)==true) t = CCommandLogView::CCT_LOCAL;
	else if(pCommand->m_Sender==m_This) t = CCommandLogView::CCT_SEND;
	else if(pCommand->m_Receiver==m_This) t = CCommandLogView::CCT_RECEIVE;
//	else _ASSERT(FALSE);
	m_pCmdLogView->AddCommand(GetGlobalClockCount(), t, pCommand);
}

bool MBMatchAgent::OnCommand(MCommand* pCommand)
{
	if( MMatchAgent::OnCommand(pCommand) )
		return true;

#ifndef _DEBUG
	CCommandLogView::CCommandType t = CCommandLogView::CCT_LOCAL;
	if(pCommand->m_pCommandDesc->IsFlag(MCDT_LOCAL)==true) t = CCommandLogView::CCT_LOCAL;
	else if(pCommand->m_Sender==m_This) t = CCommandLogView::CCT_SEND;
	else if(pCommand->m_Receiver==m_This) t = CCommandLogView::CCT_RECEIVE;

	m_pCmdLogView->AddCommand(GetGlobalClockCount(), t, pCommand);
#endif

	return false;
}

MBMatchAgent::MBMatchAgent(COutputView* pView)
{
	m_pView = pView;
	m_pCmdLogView = NULL;
}


void MBMatchAgent::WriteAgentInfoLog()
{
	mlog( "\n" );
	mlog( "================================== Agent configure info ==================================\n" );
	char szTemp[256];
	sprintf(szTemp, "Release Date : %s\n", __DATE__);
	Log(LOG_PROG, szTemp);
	mlog( "===========================================================================================\n" );
	mlog( "\n" );
}


void MBMatchAgent::Log(unsigned int nLogLevel, const char* szLog)
{
	CTime theTime = CTime::GetCurrentTime();
	CString strTime = theTime.Format( "[%c] " );

	if((nLogLevel & LOG_DEBUG) == LOG_DEBUG)
	{
		OutputDebugString(szLog);
		OutputDebugString("\n");
	}	

	if ((nLogLevel & LOG_FILE) == LOG_FILE)
	{
		char szTemp[1024];
		strcpy(szTemp, strTime);
		strcat(szTemp, szLog);
		mlog(szTemp);
	}

	if ((nLogLevel & LOG_PROG) == LOG_PROG)
	{
		if(m_pView==NULL) return;
		m_pView->AddString(strTime, TIME_COLOR, false);
		m_pView->AddString(szLog, RGB(0,0,0));
	}
}
