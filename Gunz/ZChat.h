#ifndef _ZCHAT_H
#define _ZCHAT_H

#include "ZPrerequisites.h"
#include <vector>
using namespace std;

#include "ZChatCmd.h"
#include "ZColorTable.h"

class MWidget;

class ZChat
{
private:
	ZChatCmdManager		m_CmdManager;

	unsigned long int	m_nLastInputTime;
	int					m_nSameMsgCount;
	char				m_nLastInputMsg[512];
	unsigned long int	m_nLastAbuseTime;
	int					m_nAbuseCounter;
	char				m_szWhisperLastSender[64];

	void LobbyChatOutput(const char* szChat,MCOLOR color = MCOLOR(ZCOLOR_CHAT_LOBBY_DEFALT) );
	void StageChatOutput(const char* szChat,MCOLOR color = MCOLOR(ZCOLOR_CHAT_STAGE_DEFAULT) );

	void InitCmds();
	bool CheckRepeatInput(char* szMsg);
public:
	enum ZCHAT_MSG_TYPE
	{
		CMT_NORMAL = 0,
		CMT_SYSTEM = 1,
		CMT_BROADCAST = 2,

		CMT_END
	};
	enum ZCHAT_LOC
	{
		CL_CURRENT = 0,	
		CL_LOBBY = 1,	
		CL_STAGE = 2,	
		CL_GAME = 3,
		CL_END
	};

	ZChat();
	virtual ~ZChat();

	bool Input(char* szMsg);
	void Output(const char* szMsg, ZCHAT_MSG_TYPE msgtype = CMT_NORMAL, ZCHAT_LOC loc=CL_CURRENT,MCOLOR _color=MCOLOR(0,0,0));
	void Output(MCOLOR color, const char* szMsg, ZCHAT_LOC loc=CL_CURRENT);

	void Clear(ZCHAT_LOC loc=CL_CURRENT);
	void Report112(const char* szReason);
	bool CheckChatFilter(const char* szMsg);
	void FilterWhisperKey(MWidget* pWidget);
	void SetWhisperLastSender(char* szSenderName) { strcpy(m_szWhisperLastSender, szSenderName); }

	ZChatCmdManager* GetCmdManager() { return &m_CmdManager; }
};

void ZChatOutput(const char* szMsg, ZChat::ZCHAT_MSG_TYPE msgtype=ZChat::CMT_NORMAL, ZChat::ZCHAT_LOC loc=ZChat::CL_CURRENT,MCOLOR _color=MCOLOR(ZCOLOR_CHAT_SYSTEM));
void ZChatOutput(MCOLOR color, const char* szMsg, ZChat::ZCHAT_LOC loc=ZChat::CL_CURRENT);
void ZChatOutputMouseSensitivityChanged(int old, int neo);
void ZChatOutputMouseSensitivityCurrent(int i);


#endif