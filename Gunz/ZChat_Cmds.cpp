#include "stdafx.h"

#include "ZChat.h"
#include "MChattingFilter.h"
#include "ZApplication.h"
#include "ZGameInterface.h"
#include "ZPost.h"
#include "ZPostLocal.h"
#include "ZCombatChat.h"
#include "ZCombatInterface.h"
#include "ZIDLResource.h"
#include "MListBox.h"
#include "MLex.h"
#include "MTextArea.h"
#include "ZMyInfo.h"
#include "ZMessages.h"
#include "MUtil.h"
#include "ZConfiguration.h"
#include "ZInterfaceListener.h"
#include "ZNetRepository.h"

#include "ZApplication.h"
#include "ZChat_CmdID.h"

void ChatCmd_Test(const char* line, const int argc, char** const argv);

void ChatCmd_Help(const char* line, const int argc, char** const argv);
void ChatCmd_Go(const char* line, const int argc, char** const argv);
void ChatCmd_Whisper(const char* line, const int argc, char** const argv);
void ChatCmd_CreateChatRoom(const char* line, const int argc, char** const argv);
void ChatCmd_JoinChatRoom(const char* line, const int argc, char** const argv);
void ChatCmd_LeaveChatRoom(const char* line, const int argc, char** const argv);
void ChatCmd_SelectChatRoom(const char* line, const int argc, char** const argv);
void ChatCmd_InviteChatRoom(const char* line, const int argc, char** const argv);
void ChatCmd_VisitChatRoom(const char* line, const int argc, char** const argv);
void ChatCmd_ChatRoomChat(const char* line, const int argc, char** const argv);
void ChatCmd_CopyToTestServer(const char* line, const int argc, char** const argv);
void ChatCmd_StageFollow(const char* line, const int argc, char** const argv);
void ChatCmd_Friend(const char* line, const int argc, char** const argv);
void ChatCmd_Clan(const char* line, const int argc, char** const argv);

void ChatCmd_RequestQuickJoin(const char* line, const int argc, char** const argv);
void ChatCmd_Report119(const char* line, const int argc, char** const argv);
void ChatCmd_RequestPlayerInfo(const char* line, const int argc, char** const argv);
void ChatCmd_Macro(const char* line, const int argc, char** const argv);

void ChatCmd_EmotionTaunt(const char* line, const int argc, char** const argv);
void ChatCmd_EmotionBow(const char* line, const int argc, char** const argv);
void ChatCmd_EmotionWave(const char* line, const int argc, char** const argv);
void ChatCmd_EmotionLaugh(const char* line, const int argc, char** const argv);
void ChatCmd_EmotionCry(const char* line, const int argc, char** const argv);
void ChatCmd_EmotionDance(const char* line, const int argc, char** const argv);
void ChatCmd_Suicide(const char* line, const int argc, char** const argv);
void ChatCmd_Callvote(const char* line, const int argc, char** const argv);
void ChatCmd_VoteYes(const char* line, const int argc, char** const argv);
void ChatCmd_VoteNo(const char* line, const int argc, char** const argv);
void ChatCmd_Kick(const char* line, const int argc, char** const argv);
void ChatCmd_MouseSensitivity(const char* line, const int argc, char** const argv);

void ChatCmd_LadderInvite(const char* line, const int argc, char** const argv);
void ChatCmd_LadderTest(const char* line, const int argc, char** const argv);
void ChatCmd_LaunchTest(const char* line, const int argc, char** const argv);

void ChatCmd_QUESTTEST_God(const char* line, const int argc, char** const argv);
void ChatCmd_QUESTTEST_SpawnNPC(const char* line, const int argc, char** const argv);
void ChatCmd_QUESTTEST_NPCClear(const char* line, const int argc, char** const argv);
void ChatCmd_QUESTTEST_Reload(const char* line, const int argc, char** const argv);
void ChatCmd_QUESTTEST_SectorClear(const char* line, const int argc, char** const argv);
void ChatCmd_QUESTTEST_Finish(const char* line, const int argc, char** const argv);
void ChatCmd_QUESTTEST_LocalSpawnNPC(const char* line, const int argc, char** const argv);
void ChatCmd_QUESTTEST_WeakNPCs(const char* line, const int argc, char** const argv);

void ChatCmd_AdminKickPlayer(const char* line, const int argc, char** const argv);
void ChatCmd_AdminMutePlayer(const char* line, const int argc, char** const argv);
void ChatCmd_AdminBlockPlayer(const char* line, const int argc, char** const argv);

void ChatCmd_AdminPingToAll(const char* line, const int argc, char** const argv);
void ChatCmd_AdminAnnounce(const char* line, const int argc, char** const argv);
void ChatCmd_AdminServerHalt(const char* line, const int argc, char** const argv);
void ChatCmd_AdminSwitchCreateLadderGame(const char* line, const int argc, char** const argv);
void ChatCmd_AdminReloadClientHash(const char* line, const int argc, char** const argv);
void ChatCmd_AdminResetAllHackingBlock(const char* line, const int argc, char** const argv);
void ChatCmd_AdminReloadGambleitem(const char* line, const int argc, char** const argv);
void ChatCmd_AdminDumpGambleitemLog(const char* line, const int argc, char** const argv);
void ChatCmd_AdminAssasin(const char* line, const int argc, char** const argv);

void ChatCmd_ChangeMaster(const char* line, const int argc, char** const argv);
void ChatCmd_ChangePassword(const char* line, const int argc, char** const argv);
void ChatCmd_AdminHide(const char* line, const int argc, char** const argv);
void ChatCmd_RequestJjang(const char* line, const int argc, char** const argv);
void ChatCmd_RemoveJjang(const char* line, const int argc, char** const argv);

void ChatCmd_StageRejoin(const char* line, const int argc, char** const argv);

void _AddCmdFromXml(ZChatCmdManager* pCmdManager, ZCmdXmlParser* pParser,
	int nCmdID, ZChatCmdProc* fnProc, unsigned long int flag,
	int nMinArgs, int nMaxArgs, bool bRepeatEnabled)
{
	ZCmdXmlParser::_CmdStr* pCmdStr = pParser->GetStr(nCmdID);
	if (pCmdStr)
	{
		pCmdManager->AddCommand(nCmdID, pCmdStr->szName, fnProc, flag, nMinArgs, nMaxArgs, bRepeatEnabled,
			pCmdStr->szUsage, pCmdStr->szHelp);

		for (int i = 0; i < (int)pCmdStr->vecAliases.size(); i++)
		{
			pCmdManager->AddAlias(pCmdStr->vecAliases[i].c_str(), pCmdStr->szName);
		}
	}
}

#define _CC_AC(X1,X2,X3,X4,X5,X6,X7,X8)		m_CmdManager.AddCommand(0,X1,X2,X3,X4,X5,X6,X7,X8)
#define _CC_ALIAS(NEW,TAR)					m_CmdManager.AddAlias(NEW,TAR)
#define _CC_ACX(X1,X2,X3,X4,X5,X6)			_AddCmdFromXml(&m_CmdManager,&parser,X1,X2,X3,X4,X5,X6)

void ZChat::InitCmds()
{
	ZCmdXmlParser parser;
	if (!parser.ReadXml(ZApplication::GetFileSystem(), FILENAME_CHATCMDS))
	{
		MLog("Error while Read Item Descriptor %s", "system/chatcmds.xml");
	}

	_CC_ACX(CCMD_ID_HELP, &ChatCmd_Help, CCF_ALL, ARGVNoMin, ARGVNoMax, true);
	_CC_ACX(CCMD_ID_WHISPER, &ChatCmd_Whisper, CCF_ALL, ARGVNoMin, 1, false);
	_CC_ACX(CCMD_ID_REPORT119, &ChatCmd_Report119, CCF_ALL, ARGVNoMin, ARGVNoMax, true);
	_CC_ACX(CCMD_ID_FRIEND, &ChatCmd_Friend, CCF_ALL, ARGVNoMin, 1, false);
	_CC_ACX(CCMD_ID_CLAN, &ChatCmd_Clan, CCF_ALL, ARGVNoMin, ARGVNoMax, false);
	_CC_ACX(CCMD_ID_STAGE_FOLLOW, &ChatCmd_StageFollow, CCF_LOBBY, ARGVNoMin, 1, true);
	_CC_ACX(CCMD_ID_REQUEST_QUICK_JOIN, &ChatCmd_RequestQuickJoin, CCF_LOBBY, ARGVNoMin, ARGVNoMax, true);
	_CC_ACX(CCMD_ID_EMOTION_TAUNT, &ChatCmd_EmotionTaunt, CCF_GAME, ARGVNoMin, ARGVNoMax, true);
	_CC_ACX(CCMD_ID_EMOTION_BOW, &ChatCmd_EmotionBow, CCF_GAME, ARGVNoMin, ARGVNoMax, true);
	_CC_ACX(CCMD_ID_EMOTION_WAVE, &ChatCmd_EmotionWave, CCF_GAME, ARGVNoMin, ARGVNoMax, true);
	_CC_ACX(CCMD_ID_EMOTION_LAUGH, &ChatCmd_EmotionLaugh, CCF_GAME, ARGVNoMin, ARGVNoMax, true);
	_CC_ACX(CCMD_ID_EMOTION_CRY, &ChatCmd_EmotionCry, CCF_GAME, ARGVNoMin, ARGVNoMax, true);
	_CC_ACX(CCMD_ID_EMOTION_DANCE, &ChatCmd_EmotionDance, CCF_GAME, ARGVNoMin, ARGVNoMax, true);
	_CC_ACX(CCMD_ID_MACRO, &ChatCmd_Macro, CCF_GAME, ARGVNoMin, ARGVNoMax, true);
	_CC_ACX(CCMD_ID_SUICIDE, &ChatCmd_Suicide, CCF_GAME, ARGVNoMin, ARGVNoMax, true);
	_CC_ACX(CCMD_ID_CALLVOTE, &ChatCmd_Callvote, CCF_GAME, ARGVNoMin, ARGVNoMax, true);
	_CC_ACX(CCMD_ID_VOTEYES, &ChatCmd_VoteYes, CCF_GAME, ARGVNoMin, ARGVNoMax, true);
	_CC_ACX(CCMD_ID_VOTENO, &ChatCmd_VoteNo, CCF_GAME, ARGVNoMin, ARGVNoMax, true);
	_CC_ACX(CCMD_ID_KICK, &ChatCmd_Kick, CCF_GAME, ARGVNoMin, ARGVNoMax, true);
	_CC_ACX(CCMD_ID_CREATE_CHATROOM, &ChatCmd_CreateChatRoom, CCF_ALL, ARGVNoMin, 1, true);
	_CC_ACX(CCMD_ID_JOIN_CHATROOM, &ChatCmd_JoinChatRoom, CCF_ALL, ARGVNoMin, 1, true);
	_CC_ACX(CCMD_ID_LEAVE_CHATROOM, &ChatCmd_LeaveChatRoom, CCF_ALL, ARGVNoMin, 1, true);
	_CC_ACX(CCMD_ID_SELECT_CHATROOM, &ChatCmd_SelectChatRoom, CCF_ALL, ARGVNoMin, 1, true);
	_CC_ACX(CCMD_ID_INVITE_CHATROOM, &ChatCmd_InviteChatRoom, CCF_ALL, ARGVNoMin, 1, true);
	_CC_ACX(CCMD_ID_VISIT_CHATROOM, &ChatCmd_VisitChatRoom, CCF_ALL, ARGVNoMin, 1, true);
	_CC_ACX(CCMD_ID_CHAT_CHATROOM, &ChatCmd_ChatRoomChat, CCF_ALL, ARGVNoMin, 1, false);
	_CC_ACX(CCMD_ID_MOUSE_SENSITIVITY, &ChatCmd_MouseSensitivity, CCF_GAME, ARGVNoMin, 1, true);

	_CC_AC("admin_kick", &ChatCmd_AdminKickPlayer, CCF_ADMIN, ARGVNoMin, 1, true, "/admin_kick <charname>", "");
	_CC_AC("admin_mute", &ChatCmd_AdminMutePlayer, CCF_ADMIN, ARGVNoMin, 2, true, "/admin_mute <charname> <due>", "");
	_CC_AC("admin_block", &ChatCmd_AdminBlockPlayer, CCF_ADMIN, ARGVNoMin, 2, true, "/admin_block <charname> <due>", "");

	_CC_AC("admin_pingtoall", &ChatCmd_AdminPingToAll, CCF_ADMIN, ARGVNoMin, ARGVNoMax, true, "/admin_pingtoall", "");
	_CC_AC("admin_wall", &ChatCmd_AdminAnnounce, CCF_ADMIN, ARGVNoMin, 1, true, "/admin_wall <메시지>", "");
	_CC_AC("admin_halt", &ChatCmd_AdminServerHalt, CCF_ADMIN, ARGVNoMin, ARGVNoMax, true, "/admin_halt", "");
	_CC_AC("changemaster", &ChatCmd_ChangeMaster, CCF_ADMIN | CCF_STAGE | CCF_GAME, ARGVNoMin, ARGVNoMax, true, "/changemaster", "");
	_CC_AC("changepassword", &ChatCmd_ChangePassword, CCF_ADMIN | CCF_STAGE | CCF_GAME, ARGVNoMin, ARGVNoMax, true, "/changepassword", "");
	_CC_AC("admin_hide", &ChatCmd_AdminHide, CCF_ADMIN | CCF_LOBBY, ARGVNoMin, ARGVNoMax, true, "/admin_hide", "");
	_CC_AC("hide", &ChatCmd_AdminHide, CCF_ADMIN | CCF_LOBBY, ARGVNoMin, ARGVNoMax, true, "/hide", "");
	_CC_AC("jjang", &ChatCmd_RequestJjang, CCF_ADMIN | CCF_STAGE | CCF_GAME, ARGVNoMin, ARGVNoMax, true, "/jjang", "");
	_CC_AC("removejjang", &ChatCmd_RemoveJjang, CCF_ADMIN | CCF_STAGE | CCF_GAME, ARGVNoMin, ARGVNoMax, true, "/removejjang", "");
	_CC_AC("admin_reload_hash", &ChatCmd_AdminReloadClientHash, CCF_ADMIN, ARGVNoMin, ARGVNoMax, true, "/admin_reload_hash", "");

	_CC_AC("admin_switch_laddergame", &ChatCmd_AdminSwitchCreateLadderGame, CCF_ADMIN, ARGVNoMin, ARGVNoMax, true, "/admin_switch_laddergame 1", "");
	_CC_AC("admin_reset_all_hacking_block", &ChatCmd_AdminResetAllHackingBlock, CCF_ADMIN, ARGVNoMin, ARGVNoMax, true, "/admin_reset_all_hacking_block", "");
	_CC_AC("admin_reload_gambleitem", &ChatCmd_AdminReloadGambleitem, CCF_ADMIN, ARGVNoMin, ARGVNoMax, true, "/admin_reload_gambleitem", "");
	_CC_AC("admin_dump_gambleitem_log", &ChatCmd_AdminDumpGambleitemLog, CCF_ADMIN, ARGVNoMin, ARGVNoMax, true, "/admin_dump_gambleitem_log", "");
	_CC_AC("admin_commander", &ChatCmd_AdminAssasin, CCF_ADMIN | CCF_GAME, ARGVNoMin, ARGVNoMax, true, "/admin_commander", "");

	_CC_AC("rejoin", &ChatCmd_StageRejoin, CCF_LOBBY, ARGVNoMin, ARGVNoMax, true, "/rejoin", "");
}

void ChatCmd_StageRejoin(const char* line, const int argc, char** const argv)
{
	ZGetGameClient()->IsRejoin = true;
	ZPostStageRequestRejoin();
}

void OutputCmdHelp(const char* cmd)
{
	ZChatCmdManager* pCCM = ZGetGameInterface()->GetChat()->GetCmdManager();
	ZChatCmd* pCmd = pCCM->GetCommandByName(cmd);
	if (pCmd == NULL) return;

	if ((pCmd->GetFlag() & CCF_ADMIN) && !ZGetMyInfo()->IsAdminGrade())
		return;

	char szBuf[512];
	sprintf(szBuf, "%s: %s", pCmd->GetName(), pCmd->GetHelp());
	ZChatOutput(szBuf, ZChat::CMT_SYSTEM);
}

void OutputCmdUsage(const char* cmd)
{
	ZChatCmdManager* pCCM = ZGetGameInterface()->GetChat()->GetCmdManager();
	ZChatCmd* pCmd = pCCM->GetCommandByName(cmd);
	if (pCmd == NULL) return;

	if ((pCmd->GetFlag() & CCF_ADMIN) && !ZGetMyInfo()->IsAdminGrade())
		return;

	char szBuf[512];
	sprintf(szBuf, "%s: %s", ZMsg(MSG_WORD_USAGE), pCmd->GetUsage());
	ZChatOutput(szBuf, ZChat::CMT_SYSTEM);
}

void OutputCmdWrongArgument(const char* cmd)
{
	ZChatOutput(ZMsg(MSG_WRONG_ARGUMENT));
	OutputCmdUsage(cmd);
}

void ChatCmd_Help(const char* line, const int argc, char** const argv)
{
	ZChatCmdManager* pCCM = ZGetGameInterface()->GetChat()->GetCmdManager();

	char szBuf[1024] = "";

	GunzState state = ZApplication::GetGameInterface()->GetState();

	if (state == GUNZ_GAME) {
		if (ZGetMyInfo()) {
			if (ZGetMyInfo()->GetLevel() < 10) {
				if (ZGetGame()) {
					ZGetGame()->m_HelpScreen.ChangeMode();
					return;
				}
			}
		}
	}

	if (argc == 1)
	{
		ZChatCmdFlag nCurrFlag = CCF_NONE;

		switch (state)
		{
		case GUNZ_LOBBY: nCurrFlag = CCF_LOBBY; break;
		case GUNZ_STAGE: nCurrFlag = CCF_STAGE; break;
		case GUNZ_GAME: nCurrFlag = CCF_GAME; break;
		}

		sprintf(szBuf, "%s: ", ZMsg(MSG_WORD_COMMANDS));

		int nCnt = 0;
		int nCmdCount = pCCM->GetCmdCount();

		for (ZChatCmdMap::iterator itor = pCCM->GetCmdBegin(); itor != pCCM->GetCmdEnd(); ++itor)
		{
			nCnt++;
			ZChatCmd* pCmd = (*itor).second;

			if (pCmd->GetFlag() & CCF_ADMIN) continue;
			if (!(pCmd->GetFlag() & nCurrFlag)) continue;

			strcat(szBuf, pCmd->GetName());

			if (nCnt != nCmdCount) strcat(szBuf, ", ");
		}

		switch (state)
		{
		case GUNZ_LOBBY:
			strcat(szBuf, "go");
			break;
		case GUNZ_STAGE:
			strcat(szBuf, "kick");
			break;
		case GUNZ_GAME:
			break;
		}

		ZChatOutput(szBuf, ZChat::CMT_SYSTEM);

		sprintf(szBuf, "%s: /h %s", ZMsg(MSG_WORD_HELP), ZMsg(MSG_WORD_COMMANDS));
		ZChatOutput(szBuf, ZChat::CMT_SYSTEM);
	}
	else if (argc == 2)
	{
		OutputCmdHelp(argv[1]);
		OutputCmdUsage(argv[1]);
	}
}

void ChatCmd_Go(const char* line, const int argc, char** const argv)
{
	if (argc < 2) return;

	ZRoomListBox* pRoomList = (ZRoomListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_StageList");
	if (pRoomList == NULL)
		return;

	int nRoomNo = atoi(argv[1]);

	ZPostStageGo(nRoomNo);
}

void ChatCmd_Whisper(const char* line, const int argc, char** const argv)
{
	if (argc < 2)
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	if (ZGetMyInfo()->GetUGradeID() == MMUG_CHAT_LIMITED)
	{
		ZChatOutput(ZMsg(MSG_CANNOT_CHAT));
		return;
	}

	char* pszSenderName = "Me";

	char szName[512] = "";
	char szRName[512] = "";

	MLex lex;
	char* pszMsg = lex.GetOneArg(argv[1], szName, szRName);

	if ((int)strlen(pszMsg) > 127)
		return;

	if (!ZGetGameInterface()->GetChat()->CheckChatFilter(pszMsg)) return;

	int nNameLen = (int)strlen(szName);
	if (nNameLen < MIN_CHARNAME)
	{
		const char* str = ZErrStr(MERR_TOO_SHORT_NAME);
		if (str)
		{
			char text[1024];
			sprintf(text, "%s (E%d)", str, MERR_TOO_SHORT_NAME);
			ZChatOutput(MCOLOR(96, 96, 168), text, ZChat::CL_CURRENT);
		}
	}
	else if (nNameLen > MAX_CHARNAME)
	{
		const char* str = ZErrStr(MERR_TOO_LONG_NAME);
		if (str)
		{
			char text[1024];
			sprintf(text, "%s (E%d)", str, MERR_TOO_LONG_NAME);
			ZChatOutput(MCOLOR(96, 96, 168), text, ZChat::CL_CURRENT);
		}
	}
	else
	{
		ZPostWhisper(pszSenderName, szName, pszMsg);

		char szMsg[512];
		sprintf(szMsg, "(To %s) : %s", szRName, pszMsg);
		ZChatOutput(MCOLOR(96, 96, 168), szMsg, ZChat::CL_CURRENT);
	}
}

void ChatCmd_CreateChatRoom(const char* line, const int argc, char** const argv)
{
	if (argc < 2)
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	char* pszCharName = argv[1];

	if (!MGetChattingFilter()->IsValidStr(pszCharName, 1)) {
		char szMsg[256];
		ZTransMsg(szMsg, MSG_WRONG_WORD_NAME, 1, MGetChattingFilter()->GetLastFilteredStr());
		ZApplication::GetGameInterface()->ShowMessage(szMsg, NULL, MSG_WRONG_WORD_NAME);
	}
	else
	{
		ZChatOutput(
			ZMsg(MSG_LOBBY_REQUESTING_CREATE_CHAT_ROOM),
			ZChat::CMT_SYSTEM);

		ZPostChatRoomCreate(ZGetMyUID(), pszCharName);
	}
}
void ChatCmd_JoinChatRoom(const char* line, const int argc, char** const argv)
{
	if (argc < 2)
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	ZChatOutput(
		ZMsg(MSG_LOBBY_REQUESTING_JOIN_CAHT_ROOM),
		ZChat::CMT_SYSTEM);

	char* pszChatRoomName = argv[1];

	ZPostChatRoomJoin(pszChatRoomName);
}

void ChatCmd_LeaveChatRoom(const char* line, const int argc, char** const argv)
{
	if (argc < 2)
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	ZChatOutput(
		ZMsg(MSG_LOBBY_LEAVE_CHAT_ROOM),
		ZChat::CMT_SYSTEM);

	char* pszRoomName = argv[1];

	ZPostChatRoomLeave(pszRoomName);
}

void ChatCmd_SelectChatRoom(const char* line, const int argc, char** const argv)
{
	if (argc < 2)
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	ZChatOutput(
		ZMsg(MSG_LOBBY_CHOICE_CHAT_ROOM),
		ZChat::CMT_SYSTEM);

	char* pszRoomName = argv[1];

	ZPostSelectChatRoom(pszRoomName);
}

void ChatCmd_InviteChatRoom(const char* line, const int argc, char** const argv)
{
	if (argc < 2)
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}
	char* pszPlayerName = argv[1];

	char szLog[128];

	ZTransMsg(szLog, MSG_LOBBY_INVITATION, 1, pszPlayerName);

	ZChatOutput(szLog, ZChat::CMT_SYSTEM);

	ZPostInviteChatRoom(pszPlayerName);
}

void ChatCmd_VisitChatRoom(const char* line, const int argc, char** const argv)
{
	char* pszRoomName = const_cast<char*>(ZGetGameClient()->GetChatRoomInvited());
	ZPostChatRoomJoin(pszRoomName);
}

void ChatCmd_ChatRoomChat(const char* line, const int argc, char** const argv)
{
	if (argc < 2)
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}
	char* pszMsg = argv[1];

	if (!ZGetGameInterface()->GetChat()->CheckChatFilter(pszMsg)) return;

	ZPostChatRoomChat(pszMsg);
}

void ChatCmd_CopyToTestServer(const char* line, const int argc, char** const argv)
{
	return;

	if (argc != 1) return;

	static unsigned long int st_nLastTime = 0;
	unsigned long int nNowTime = timeGetTime();

#define DELAY_POST_COPY_TO_TESTSERVER		(1000 * 60)

	if ((nNowTime - st_nLastTime) > DELAY_POST_COPY_TO_TESTSERVER)
	{
		ZPostRequestCopyToTestServer(ZGetGameClient()->GetPlayerUID());

		st_nLastTime = nNowTime;
	}
}

void ChatCmd_StageFollow(const char* line, const int argc, char** const argv)
{
	if (argc < 2)
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	char* pszTarget = argv[1];

	ZPostStageFollow(pszTarget);
}

void ChatCmd_Friend(const char* line, const int argc, char** const argv)
{
	class ZFriendCommandHelper {
	public:
		enum ZFRIENDCMD {
			ZFRIENDCMD_ADD,
			ZFRIENDCMD_REMOVE,
			ZFRIENDCMD_LIST,
			ZFRIENDCMD_MSG,
			ZFRIENDCMD_UNKNOWN
		};
		ZFRIENDCMD GetSubCommand(const char* pszCmd) {
			if (stricmp(pszCmd, "Add") == 0)
				return ZFRIENDCMD_ADD;
			else if (stricmp(pszCmd, "추가") == 0)
				return ZFRIENDCMD_ADD;
			else if (stricmp(pszCmd, "Remove") == 0)
				return ZFRIENDCMD_REMOVE;
			else if (stricmp(pszCmd, "삭제") == 0)
				return ZFRIENDCMD_REMOVE;
			else if (stricmp(pszCmd, "list") == 0)
				return ZFRIENDCMD_LIST;
			else if (stricmp(pszCmd, "목록") == 0)
				return ZFRIENDCMD_LIST;
			else if (stricmp(pszCmd, "msg") == 0)
				return ZFRIENDCMD_MSG;
			else if (stricmp(pszCmd, "채팅") == 0)
				return ZFRIENDCMD_MSG;
			else return ZFRIENDCMD_UNKNOWN;
		}
	} friendHelper;

	if (argc < 2)
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}
	char szSubCmd[256] = "";
	char szArg[256] = "";

	MLex lex;
	char* pszMsg = lex.GetOneArg(argv[1], szSubCmd);

	switch (friendHelper.GetSubCommand(szSubCmd)) {
	case ZFriendCommandHelper::ZFRIENDCMD_ADD:
	{
		lex.GetOneArg(pszMsg, szArg);
		ZPostFriendAdd(szArg);
	}
	break;
	case ZFriendCommandHelper::ZFRIENDCMD_REMOVE:
	{
		lex.GetOneArg(pszMsg, szArg);
		ZPostFriendRemove(szArg);
	}
	break;
	case ZFriendCommandHelper::ZFRIENDCMD_LIST:
	{
		ZPostFriendList();
	}
	break;
	case ZFriendCommandHelper::ZFRIENDCMD_MSG:
	{
		lex.GetOneArg(pszMsg, szArg);
		ZPostFriendMsg(szArg);
	}
	break;
	default:
		OutputDebugString("Unknown Friend Command \n");
		break;
	};
}

void ChatCmd_Clan(const char* line, const int argc, char** const argv)
{
	class ZClanCommandHelper {
	public:
		enum ZCLANCMD {
			ZCLANCMD_CREATE,
			ZCLANCMD_CLOSE,
			ZCLANCMD_JOIN,
			ZCLANCMD_LEAVE,
			ZCLANCMD_EXPEL_MEMBER,
			ZCLANCMD_LIST,
			ZCLANCMD_MSG,

			ZCLANCMD_CHANGE_GRADE,

			ZCLANCMD_UNKNOWN
		};
		ZCLANCMD GetSubCommand(const char* pszCmd) {
			GunzState nGameState = ZApplication::GetGameInterface()->GetState();

			if ((stricmp(pszCmd, "생성") == 0) || (stricmp(pszCmd, "open") == 0))
			{
				if (nGameState == GUNZ_LOBBY) return ZCLANCMD_CREATE;
			}
			else if ((stricmp(pszCmd, "폐쇄") == 0) || (stricmp(pszCmd, "해체") == 0) || (stricmp(pszCmd, "close") == 0))
			{
				if (nGameState == GUNZ_LOBBY) return ZCLANCMD_CLOSE;
			}
			else if ((stricmp(pszCmd, "초대") == 0) || (stricmp(pszCmd, "invite") == 0))
			{
				if (nGameState == GUNZ_LOBBY) return ZCLANCMD_JOIN;
			}
			else if ((stricmp(pszCmd, "탈퇴") == 0) || (stricmp(pszCmd, "leave") == 0))
			{
				if (nGameState == GUNZ_LOBBY) return ZCLANCMD_LEAVE;
			}
			else if ((stricmp(pszCmd, "권한변경") == 0) || (stricmp(pszCmd, "promote") == 0))
			{
				if (nGameState == GUNZ_LOBBY) return ZCLANCMD_CHANGE_GRADE;
			}
			else if ((stricmp(pszCmd, "강제탈퇴") == 0) || (stricmp(pszCmd, "방출") == 0) || (stricmp(pszCmd, "dismiss") == 0))
			{
				if (nGameState == GUNZ_LOBBY) return ZCLANCMD_EXPEL_MEMBER;
			}
			else if ((stricmp(pszCmd, "list") == 0) || (stricmp(pszCmd, "목록") == 0))
			{
				if (nGameState == GUNZ_LOBBY) return ZCLANCMD_LIST;
			}
			else if ((stricmp(pszCmd, "msg") == 0) || (stricmp(pszCmd, "채팅") == 0))
			{
				return ZCLANCMD_MSG;
			}

			return ZCLANCMD_UNKNOWN;
		}
	} clanHelper;

	if (argc < 2)
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}
	char szSubCmd[256] = "";

	MLex lex;
	char* pszMsg = lex.GetOneArg(argv[1], szSubCmd);

	switch (clanHelper.GetSubCommand(szSubCmd)) {
	case ZClanCommandHelper::ZCLANCMD_CREATE:
	{
		if (argc < 7)
		{
			OutputCmdWrongArgument(argv[0]);
			break;
		}

		ZGetGameClient()->RequestCreateClan(argv[2], &argv[3]);
	}
	break;
	case ZClanCommandHelper::ZCLANCMD_CLOSE:
	{
		if (argc < 3)
		{
			OutputCmdWrongArgument(argv[0]);
			break;
		}

		if (ZGetMyInfo()->GetClanGrade() != MCG_MASTER)
		{
			ZChatOutput(
				ZMsg(MSG_CLAN_ENABLED_TO_MASTER),
				ZChat::CMT_SYSTEM);
			break;
		}

		if (stricmp(ZGetMyInfo()->GetClanName(), argv[2]))
		{
			ZChatOutput(
				ZMsg(MSG_CLAN_WRONG_CLANNAME),
				ZChat::CMT_SYSTEM);
			break;
		}

		ZApplication::GetGameInterface()->ShowConfirmMessage(
			ZMsg(MSG_CLAN_CONFIRM_CLOSE),
			ZGetClanCloseConfirmListenter());
	}
	break;
	case ZClanCommandHelper::ZCLANCMD_JOIN:
	{
		if (argc < 3)
		{
			OutputCmdWrongArgument(argv[0]);
			break;
		}

		if (!ZGetMyInfo()->IsClanJoined())
		{
			ZChatOutput(
				ZMsg(MSG_CLAN_NOT_JOINED),
				ZChat::CMT_SYSTEM);
			break;
		}

		if (!IsUpperClanGrade(ZGetMyInfo()->GetClanGrade(), MCG_ADMIN))
		{
			ZChatOutput(
				ZMsg(MSG_CLAN_ENABLED_TO_MASTER_AND_ADMIN),
				ZChat::CMT_SYSTEM);
			break;
		}

		char szClanName[256];
		strcpy(szClanName, ZGetMyInfo()->GetClanName());
		ZPostRequestJoinClan(ZGetGameClient()->GetPlayerUID(), szClanName, argv[2]);
	}
	break;
	case ZClanCommandHelper::ZCLANCMD_LEAVE:
	{
		if (argc < 2)
		{
			OutputCmdWrongArgument(argv[0]);
			break;
		}

		if (!ZGetMyInfo()->IsClanJoined())
		{
			ZChatOutput(
				ZMsg(MSG_CLAN_NOT_JOINED),
				ZChat::CMT_SYSTEM);
			break;
		}

		if (IsUpperClanGrade(ZGetMyInfo()->GetClanGrade(), MCG_MASTER))
		{
			ZChatOutput(
				ZMsg(MSG_CLAN_MASTER_CANNOT_LEAVED),
				ZChat::CMT_SYSTEM);
			break;
		}

		ZApplication::GetGameInterface()->ShowConfirmMessage(
			ZMsg(MSG_CLAN_CONFIRM_LEAVE),
			ZGetClanLeaveConfirmListenter());
	}
	break;
	case ZClanCommandHelper::ZCLANCMD_CHANGE_GRADE:
	{
		if (argc < 4)
		{
			OutputCmdWrongArgument(argv[0]);
			break;
		}

		if (!ZGetMyInfo()->IsClanJoined())
		{
			ZChatOutput(
				ZMsg(MSG_CLAN_NOT_JOINED),
				ZChat::CMT_SYSTEM);
			break;
		}

		if (!IsUpperClanGrade(ZGetMyInfo()->GetClanGrade(), MCG_MASTER))
		{
			ZChatOutput(
				ZMsg(MSG_CLAN_ENABLED_TO_MASTER),
				ZChat::CMT_SYSTEM);
			break;
		}

		char szMember[256];
		int nClanGrade = 0;

		strcpy(szMember, argv[2]);
		if ((strlen(szMember) < 0) || (strlen(szMember) > CLAN_NAME_LENGTH))
		{
			OutputCmdWrongArgument(argv[0]);
			break;
		}

		if ((!stricmp(argv[3], "클랜운영자")) || (!stricmp(argv[3], "운영자")) || (!stricmp(argv[3], "영자")) || (!stricmp(argv[3], "admin")))
		{
			nClanGrade = (int)MCG_ADMIN;
		}
		else if ((!stricmp(argv[3], "클랜멤버")) || (!stricmp(argv[3], "멤버")) || (!stricmp(argv[3], "클랜원")) || (!stricmp(argv[3], "member")))
		{
			nClanGrade = (int)MCG_MEMBER;
		}
		else
		{
			OutputCmdWrongArgument(argv[0]);
			break;
		}

		ZPostRequestChangeClanGrade(ZGetGameClient()->GetPlayerUID(), szMember, nClanGrade);
	}
	break;
	case ZClanCommandHelper::ZCLANCMD_EXPEL_MEMBER:
	{
		if (argc < 3)
		{
			OutputCmdWrongArgument(argv[0]);
			break;
		}

		if (!ZGetMyInfo()->IsClanJoined())
		{
			ZChatOutput(
				ZMsg(MSG_CLAN_NOT_JOINED),
				ZChat::CMT_SYSTEM);
			break;
		}

		if (!IsUpperClanGrade(ZGetMyInfo()->GetClanGrade(), MCG_ADMIN))
		{
			ZChatOutput(
				ZMsg(MSG_CLAN_ENABLED_TO_MASTER_AND_ADMIN),
				ZChat::CMT_SYSTEM);
			break;
		}

		char szMember[256];
		int nClanGrade = 0;

		strcpy(szMember, argv[2]);
		if ((strlen(szMember) < 0) || (strlen(szMember) > CLAN_NAME_LENGTH))
		{
			OutputCmdWrongArgument(argv[0]);
			break;
		}

		ZPostRequestExpelClanMember(ZGetGameClient()->GetPlayerUID(), szMember);
	}
	break;
	case ZClanCommandHelper::ZCLANCMD_LIST:
	{
	}
	break;
	case ZClanCommandHelper::ZCLANCMD_MSG:
	{
		if (ZGetMyInfo()->GetUGradeID() == MMUG_CHAT_LIMITED)
		{
			ZChatOutput(ZMsg(MSG_CANNOT_CHAT));
			break;
		}

		MLex lex;

		char szLine[512], szTemp1[256] = "", szTemp2[256] = "";
		strcpy(szLine, line);

		char* pszMsg = lex.GetTwoArgs(szLine, szTemp1, szTemp2);

		ZPostClanMsg(ZGetGameClient()->GetPlayerUID(), pszMsg);
	}
	break;
	default:
		ZChatOutput(ZMsg(MSG_CANNOT_CHAT));
		break;
	};
}

void ChatCmd_RequestQuickJoin(const char* line, const int argc, char** const argv)
{
	ZGetGameInterface()->RequestQuickJoin();
}

void ChatCmd_Report119(const char* line, const int argc, char** const argv)
{
	ZPostLocalReport119();
}

void ChatCmd_AdminKickPlayer(const char* line, const int argc, char** const argv)
{
	if (argc < 2) {
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	char* pszPlayerName = argv[1];
	ZPostAdminRequestKickPlayer(pszPlayerName);
}

int GetDueHour(char* pszDue)
{
	int nLength = (int)strlen(pszDue);

	for (int i = 0; i < nLength - 1; i++) {
		if (pszDue[i] > '9' || pszDue[i] < '0') {
			return -1;
		}
	}

	int nDueType = toupper(pszDue[nLength - 1]);
	if (nDueType == toupper('d')) {
		int nDay = atoi(pszDue);
		if (nDay < 365 * 10)	return nDay * 24;
		else					return -1;
	}
	else if (nDueType == toupper('h')) {
		int nHour = atoi(pszDue);
		return nHour;
	}
	else {
		return -1;
	}
}

void ChatCmd_AdminMutePlayer(const char* line, const int argc, char** const argv)
{
	if (argc < 3) {
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	char* pszPlayerName = argv[1];

	int nDueHour = GetDueHour(argv[2]);
	if (nDueHour < 0) {
		OutputCmdWrongArgument(argv[0]);
		return;
}

#ifdef _DEBUG
	mlog("Request Mute on Player(%s) While %d Hour\n", pszPlayerName, nDueHour);
#endif

	ZPostAdminRequestMutePlayer(pszPlayerName, nDueHour);
}

void ChatCmd_AdminBlockPlayer(const char* line, const int argc, char** const argv)
{
	if (argc < 3) {
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	char* pszPlayerName = argv[1];

	int nDueHour = GetDueHour(argv[2]);
	if (nDueHour < 0) {
		OutputCmdWrongArgument(argv[0]);
		return;
}

#ifdef _DEBUG
	mlog("Request Block on Player(%s) While %d Hour\n", pszPlayerName, nDueHour);
#endif

	ZPostAdminRequestBlockPlayer(pszPlayerName, nDueHour);
}

void ChatCmd_AdminPingToAll(const char* line, const int argc, char** const argv)
{
	ZPostAdminPingToAll();
}

void ChatCmd_AdminReloadClientHash(const char* line, const int argc, char** const argv)
{
	ZPostAdminReloadClientHash();
}

void ChatCmd_AdminResetAllHackingBlock(const char* line, const int argc, char** const argv)
{
	ZPostAdminResetAllHackingBlock();
}

void ChatCmd_AdminReloadGambleitem(const char* line, const int argc, char** const argv)
{
	ZPostAdminReloadGambleItem();
}

void ChatCmd_AdminDumpGambleitemLog(const char* line, const int argc, char** const argv)
{
	ZPostAdminDumpGambleItemLog();
}

void ChatCmd_AdminAssasin(const char* line, const int argc, char** const argv)
{
	ZPostAdminAssasin();
}

void ChatCmd_ChangeMaster(const char* line, const int argc, char** const argv)
{
	ZPostChangeMaster();
}

void ChatCmd_ChangePassword(const char* line, const int argc, char** const argv)
{
	if (argc < 2)
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}
	char* pszPassword = argv[1];

	ZPostChangePassword(pszPassword);
}

void ChatCmd_AdminHide(const char* line, const int argc, char** const argv)
{
	ZPostAdminHide();
}

void ChatCmd_RequestJjang(const char* line, const int argc, char** const argv)
{
	if (argc < 2)
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}
	char* pszTargetName = argv[1];

	ZPostAdminRequestJjang(pszTargetName);
}

void ChatCmd_RemoveJjang(const char* line, const int argc, char** const argv)
{
	if (argc < 2)
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}
	char* pszTargetName = argv[1];

	ZPostAdminRemoveJjang(pszTargetName);
}

void ChatCmd_Test(const char* line, const int argc, char** const argv)
{
	ZChatOutput("Testing...", ZChat::CMT_SYSTEM);

	if (argc < 2)
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}
	char* pszCharName = argv[1];

	ZGameClient* pClient = ZGetGameClient();
	MMatchPeerInfoList* pList = pClient->GetPeers();
	for (MMatchPeerInfoList::iterator i = pList->begin(); i != pList->end(); i++) {
		MMatchPeerInfo* pInfo = (*i).second;
		if (stricmp(pInfo->CharInfo.szName, pszCharName) == 0) {
			MCommand* pCmd = pClient->CreateCommand(MC_TEST_PEERTEST_PING, pInfo->uidChar);
			pClient->Post(pCmd);
		}
	}
}

void ChatCmd_Macro(const char* line, const int argc, char** const argv)
{
	if (argc != 3)
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	int mode = -1;

	if (stricmp(argv[1], "1") == 0) mode = 0;
	else if (stricmp(argv[1], "2") == 0) mode = 1;
	else if (stricmp(argv[1], "3") == 0) mode = 2;
	else if (stricmp(argv[1], "4") == 0) mode = 3;
	else if (stricmp(argv[1], "5") == 0) mode = 4;
	else if (stricmp(argv[1], "6") == 0) mode = 5;
	else if (stricmp(argv[1], "7") == 0) mode = 6;
	else if (stricmp(argv[1], "8") == 0) mode = 7;
	else if (stricmp(argv[1], "9") == 0) mode = 8;
	else
		return;

	ZCONFIG_MACRO* pMacro = NULL;

	if (ZGetConfiguration())
		pMacro = ZGetConfiguration()->GetMacro();

	if (pMacro && argv[2]) {
		strcpy(pMacro->szMacro[mode], argv[2]);
		ZGetConfiguration()->Save(Z_LOCALE_XML_HEADER);
	}
}

void ChatCmd_EmotionTaunt(const char* line, const int argc, char** const argv)
{
	if (ZGetGame())
		ZGetGame()->PostSpMotion(ZC_SPMOTION_TAUNT);
}

void ChatCmd_EmotionBow(const char* line, const int argc, char** const argv)
{
	if (ZGetGame())
		ZGetGame()->PostSpMotion(ZC_SPMOTION_BOW);
}

void ChatCmd_EmotionWave(const char* line, const int argc, char** const argv)
{
	if (ZGetGame())
		ZGetGame()->PostSpMotion(ZC_SPMOTION_WAVE);
}

void ChatCmd_EmotionLaugh(const char* line, const int argc, char** const argv)
{
	if (ZGetGame())
		ZGetGame()->PostSpMotion(ZC_SPMOTION_LAUGH);
}

void ChatCmd_EmotionCry(const char* line, const int argc, char** const argv)
{
	if (ZGetGame())
		ZGetGame()->PostSpMotion(ZC_SPMOTION_CRY);
}

void ChatCmd_EmotionDance(const char* line, const int argc, char** const argv)
{
	if (ZGetGame())
		ZGetGame()->PostSpMotion(ZC_SPMOTION_DANCE);
}

void ChatCmd_RequestPlayerInfo(const char* line, const int argc, char** const argv)
{
	if (argc < 2)
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	ZPostRequestCharInfoDetail(ZGetGameClient()->GetPlayerUID(), argv[1]);
}

void ChatCmd_AdminAnnounce(const char* line, const int argc, char** const argv)
{
	if (argc < 2)
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	char szMsg[256];
	strcpy(szMsg, argv[1]);
	ZPostAdminAnnounce(ZGetGameClient()->GetPlayerUID(), szMsg, ZAAT_CHAT);
}

void ChatCmd_AdminServerHalt(const char* line, const int argc, char** const argv)
{
	ZPostAdminHalt(ZGetGameClient()->GetPlayerUID());
}

void ChatCmd_AdminSwitchCreateLadderGame(const char* line, const int argc, char** const argv)
{
	if (argc < 2)
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	bool bEnabled = true;
	if (!strcmp(argv[1], "0")) bEnabled = false;

	ZPostAdminRequestSwitchLadderGame(ZGetGameClient()->GetPlayerUID(), bEnabled);
}

void ChatCmd_Suicide(const char* line, const int argc, char** const argv)
{
	ZGetGameClient()->RequestGameSuicide();
}

void ChatCmd_LadderInvite(const char* line, const int argc, char** const argv)
{
	if (argc < 3)
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	char szNames[2][256];
	strcpy(szNames[0], argv[1]);
	strcpy(szNames[1], argv[2]);

	ZGetGameClient()->RequestProposal(MPROPOSAL_LADDER_INVITE, &argv[1], 2);
}

void ChatCmd_LadderTest(const char* line, const int argc, char** const argv)
{
#ifdef _DEBUG
	if (argc == 1)
	{
		char szPlayerName[MATCHOBJECT_NAME_LENGTH];
		strcpy(szPlayerName, ZGetMyInfo()->GetCharName());
		char* pName[1];
		pName[0] = szPlayerName;

		ZPostLadderRequestChallenge(pName, 1, 0);
	}
	else if (argc == 2)
	{
		char szPlayerName[MATCHOBJECT_NAME_LENGTH], szTeamMember1[MATCHOBJECT_NAME_LENGTH];
		strcpy(szPlayerName, ZGetMyInfo()->GetCharName());
		strcpy(szTeamMember1, argv[1]);

		char* pName[2];
		pName[0] = szPlayerName;
		pName[1] = szTeamMember1;

		ZPostLadderRequestChallenge(pName, 2, 0);
	}
#endif
}

void ChatCmd_LaunchTest(const char* line, const int argc, char** const argv)
{
#ifdef _DEBUG
	MCommand* pCmd = ZGetGameClient()->CreateCommand(MC_MATCH_LADDER_LAUNCH, ZGetMyUID());
	pCmd->AddParameter(new MCmdParamUID(MUID(0, 0)));
	pCmd->AddParameter(new MCmdParamStr("Mansion"));
	ZGetGameClient()->Post(pCmd);
#endif
}

void ChatCmd_Callvote(const char* line, const int argc, char** const argv)
{
	if ((argv[1] == NULL) || (argv[2] == NULL))
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	ZPOSTCMD2(MC_MATCH_CALLVOTE, MCmdParamStr(argv[1]), MCmdParamStr(argv[2]))
}

void ChatCmd_VoteYes(const char* line, const int argc, char** const argv)
{
	ZPOSTCMD0(MC_MATCH_VOTE_YES);
}

void ChatCmd_VoteNo(const char* line, const int argc, char** const argv)
{
	ZPOSTCMD0(MC_MATCH_VOTE_NO);
}

void ChatCmd_Kick(const char* line, const int argc, char** const argv)
{
	ZGetCombatInterface()->GetVoteInterface()->CallVote("kick");
}

void ChatCmd_MouseSensitivity(const char* line, const int argc, char** const argv)
{
	if (argc == 1)
	{
		ZChatOutputMouseSensitivityCurrent(ZGetConfiguration()->GetMouseSensitivityInInt());
	}
	else if (argc == 2)
	{
		int original = ZGetConfiguration()->GetMouseSensitivityInInt();

		char* szParam = argv[1];
		int asked = atoi(szParam);
		int changed = ZGetConfiguration()->SetMouseSensitivityInInt(asked);

		ZChatOutputMouseSensitivityChanged(original, changed);
	}
	else
		OutputCmdWrongArgument(argv[0]);
}

void ChatCmd_QUESTTEST_God(const char* line, const int argc, char** const argv)
{
#ifdef _DEBUG
	if (!ZGetMyInfo()->IsAdminGrade()) return;
	if (ZGetGame() == NULL) return;

	bool bNowGod = ZGetQuest()->GetCheet(ZQUEST_CHEET_GOD);
	bNowGod = !bNowGod;

	ZGetQuest()->SetCheet(ZQUEST_CHEET_GOD, bNowGod);

	if (bNowGod)
	{
		ZChatOutput("God mode enabled");
}
	else
	{
		ZChatOutput("God mode disabled");
	}
#endif
}

void ChatCmd_QUESTTEST_SpawnNPC(const char* line, const int argc, char** const argv)
{
#ifdef _DEBUG
	if (!ZGetMyInfo()->IsAdminGrade()) return;
	if (ZGetGame() == NULL) return;
	if (argc < 2) return;

	int nNPCID = 0;
	int nCount = 1;

	nNPCID = atoi(argv[1]);
	if (argv[2])
		nCount = atoi(argv[2]);

	ZPostQuestTestNPCSpawn(nNPCID, nCount);
#endif
}

void ChatCmd_QUESTTEST_LocalSpawnNPC(const char* line, const int argc, char** const argv)
{
#ifdef _DEBUG
	if (!ZGetMyInfo()->IsAdminGrade()) return;
	int nNPCID = 0;
	int nCount = 1;

	nNPCID = atoi(argv[1]);
	if (argv[2]) nCount = atoi(argv[2]);

	MCommand* pCmd = ZNewCmd(MC_QUEST_NPC_LOCAL_SPAWN);
	pCmd->AddParameter(new MCmdParamUID(ZGetMyUID()));

	MUID uidLocal;
	uidLocal.High = 10000;
	uidLocal.Low = (unsigned long)ZGetObjectManager()->size();

	pCmd->AddParameter(new MCmdParamUID(uidLocal));
	pCmd->AddParameter(new MCmdParamUChar((unsigned char)nNPCID));
	pCmd->AddParameter(new MCmdParamUChar((unsigned char)0));

	ZPostCommand(pCmd);
#endif
}

void ChatCmd_QUESTTEST_NPCClear(const char* line, const int argc, char** const argv)
{
#ifdef _DEBUG
	if (!ZGetMyInfo()->IsAdminGrade()) return;
	if (ZGetGame() == NULL) return;

	ZPostQuestTestClearNPC();
#endif
}

void ChatCmd_QUESTTEST_Reload(const char* line, const int argc, char** const argv)
{
#ifdef _DEBUG

	if (!ZGetMyInfo()->IsAdminGrade()) return;

	ZGetObjectManager()->ClearNPC();
	ZGetQuest()->Reload();

	ZChatOutput("Reloaded");
#endif
}

void ChatCmd_QUESTTEST_SectorClear(const char* line, const int argc, char** const argv)
{
#ifdef _DEBUG
	if (!ZGetMyInfo()->IsAdminGrade()) return;
	if (ZGetGame() == NULL) return;

	ZPostQuestTestSectorClear();
#endif
}

void ChatCmd_QUESTTEST_Finish(const char* line, const int argc, char** const argv)
{
#ifdef _DEBUG
	if (!ZGetMyInfo()->IsAdminGrade()) return;
	if (ZGetGame() == NULL) return;

	ZPostQuestTestFinish();
#endif
}

void ChatCmd_QUESTTEST_WeakNPCs(const char* line, const int argc, char** const argv)
{
#ifdef _DEBUG
	if (!ZGetMyInfo()->IsAdminGrade()) return;
	if (ZGetGame() == NULL) return;

	bool bNow = ZGetQuest()->GetCheet(ZQUEST_CHEET_WEAKNPCS);
	bNow = !bNow;

	ZGetQuest()->SetCheet(ZQUEST_CHEET_WEAKNPCS, bNow);

	if (bNow)
	{
		ZChatOutput("WeakNPC mode enabled");

		for (ZObjectManager::iterator itor = ZGetObjectManager()->begin();
			itor != ZGetObjectManager()->end(); ++itor)
		{
			ZObject* pObject = (*itor).second;
			if (pObject->IsNPC())
			{
				ZActor* pActor = (ZActor*)pObject;
				ZModule_HPAP* pModule = (ZModule_HPAP*)pActor->GetModule(ZMID_HPAP);
				if (pModule)
				{
					pModule->SetHP(1);
				}
			}
}
}
	else
	{
		ZChatOutput("WeakNPC mode disabled");
	}
#endif
}