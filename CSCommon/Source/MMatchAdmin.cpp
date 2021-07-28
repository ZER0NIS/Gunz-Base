#include "stdafx.h"
#include "MMatchAdmin.h"
#include "MMatchServer.h"
#include "MMatchStage.h"
#include "MMatchItem.h"
#include "MMatchRule.h"
#include "MMatchObject.h"
#include "MMatchObjCache.h"
#include "MSharedCommandTable.h"

MMatchAdmin::MMatchAdmin()
{
	m_pMatchServer = NULL;
}

MMatchAdmin::~MMatchAdmin()
{
}

bool MMatchAdmin::Create(MMatchServer* pServer)
{
	m_pMatchServer = pServer;

	return true;
}

void MMatchAdmin::Destroy()
{
}

bool MMatchAdmin::Execute(const MUID& uidAdmin, const char* szStr)
{
	if (!m_pMatchServer) return false;

	MAdminArgvInfo ai;
	memset(&ai, 0, sizeof(ai));

	char szBuf[1024];
	strcpy(szBuf, szStr);

	if (MakeArgv(szBuf, &ai))
	{
		if (ai.cargc > 0)
		{
			char szOut[65535];
			if (m_pMatchServer->OnAdminExecute(&ai, szOut))
			{
				m_pMatchServer->AdminTerminalOutput(uidAdmin, szOut);
			}
		}
	}
	else
	{
		return false;
	}

	return true;
}

bool MMatchAdmin::MakeArgv(char* szStr, MAdminArgvInfo* pAi)
{
	int c;
	char* scp;
	char* dcp;
	char* dlim;
	char* arg;

	int iArgcMax = (sizeof(pAi->cargv) / sizeof(char*));

	scp = szStr;
	dcp = pAi->argbuf;
	dlim = dcp + sizeof(pAi->argbuf) - 1;

	for (pAi->cargc = 0; pAi->cargc < iArgcMax; )
	{
		for (; ; scp++)
		{
			c = *scp;
			if (!isascii(c)) continue;
			if (isspace(c)) continue;

			if ((c == '\0') || (c == ';') || (c == '\n'))
			{
				pAi->cargv[pAi->cargc] = NULL;
				return true;
			}
			break;
		}

		arg = dcp;
		pAi->cargv[pAi->cargc] = arg;
		(pAi->cargc)++;

		for (; ; )
		{
			c = *scp;
			if ((c == '\0') || (!isascii(c)) || (isspace(c)) || (c == ';') || (c == '\n')) break;

			scp++;

			if (dcp >= dlim) return false;

			*dcp++ = c;
		}
		*dcp++ = '\0';
	}

	return false;
}

bool MMatchServer::OnAdminExecute(MAdminArgvInfo* pAI, char* szOut)
{
	szOut[0] = 0;

	if (pAI->cargc <= 0) return false;

	if (!_stricmp(pAI->cargv[0], "wall"))
	{
		if (pAI->cargc < 3)
		{
			sprintf(szOut, "���ڰ� �����մϴ�.");
			return true;
		}

		char szMsg[256];
		int nMsgType = 0;

		strcpy(szMsg, pAI->cargv[1]);
		nMsgType = atoi(pAI->cargv[2]);

		MCommand* pCmd = CreateCommand(MC_ADMIN_ANNOUNCE, MUID(0, 0));
		pCmd->AddParameter(new MCmdParamUID(MUID(0, 0)));
		pCmd->AddParameter(new MCmdParamStr(szMsg));
		pCmd->AddParameter(new MCmdParamUInt(nMsgType));

		RouteToAllClient(pCmd);
	}
	else
	{
		sprintf(szOut, "%s: no such command", pAI->cargv[0]);
	}

	return true;
}

void MMatchServer::AdminTerminalOutput(const MUID& uidAdmin, const char* szText)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if (pObj == NULL) return;

	if (pObj->GetAccountInfo()->m_nUGrade != MMUG_ADMIN)
	{
		return;
	}

	char szMsg[65535];
	strcpy(szMsg, szText);

	MCommand* pCmd = CreateCommand(MC_ADMIN_TERMINAL, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamUID(uidAdmin));
	pCmd->AddParameter(new MCmdParamStr(szMsg));

	RouteToListener(pObj, pCmd);
}