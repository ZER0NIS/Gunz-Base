#include "stdafx.h"

#include "ZGameClient.h"
#include "ZConsole.h"
#include "MCommand.h"
#include "MCommandManager.h"
#include "ZApplication.h"
#include "ZPost.h"

static MConsoleFrame* g_pConsole = NULL;
MConsoleFrame* ZGetConsole() { return g_pConsole; }

void OutputToConsole(const char* pFormat, ...)
{
	if (!g_pConsole) return;

	va_list args;
	static char temp[1024];

	va_start(args, pFormat);
	vsprintf(temp, pFormat, args);

	g_pConsole->OutputMessage(temp);

	va_end(args);
}

void ConsoleInputEvent(const char* szInputStr)
{
	if (ZGetGameClient())
	{
#ifdef _DEBUG
		if ((szInputStr[0] == '@') && (szInputStr[1] == '@') && (szInputStr[2] == ' '))
		{
			char szMsg[1024];
			strcpy(szMsg, &szInputStr[3]);
			ZPostAdminTerminal(ZGetGameClient()->GetPlayerUID(), szMsg);
			return;
		}

		if (strcmp(szInputStr, ""))
		{
			if (!ZGetGameClient()->Post(szErr, 256, szInputStr)) OutputToConsole(szErr);
		}
#endif
	}
}

void CreateConsole(MCommandManager* pCM)
{
	g_pConsole = new MConsoleFrame("Console", Mint::GetInstance()->GetMainFrame(), Mint::GetInstance()->GetMainFrame());
	g_pConsole->Show(false);
	g_pConsole->SetInputCallback(ConsoleInputEvent);
	g_pConsole->SetBounds(0, 0, MGetWorkspaceWidth() / 2 - 20, MGetWorkspaceHeight() / 2);

	for (int i = 0; i < pCM->GetCommandDescCount(); i++) {
		MCommandDesc* pCD = pCM->GetCommandDesc(i);
		g_pConsole->AddCommand(pCD->GetName());
	}
}

void DestroyConsole()
{
	if (g_pConsole)
	{
		delete g_pConsole; g_pConsole = NULL;
	}
}