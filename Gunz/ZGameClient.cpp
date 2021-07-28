#include "stdafx.h"

#include <winsock2.h>
#include "MErrorTable.h"
#include "ZConfiguration.h"
#include "ZGameClient.h"
#include "MSharedCommandTable.h"
#include "ZConsole.h"
#include "MCommandLogFrame.h"
#include "ZIDLResource.h"
#include "MBlobArray.h"
#include "ZInterface.h"
#include "ZApplication.h"
#include "ZGameInterface.h"
#include "MMatchGlobal.h"
#include "MMatchChannel.h"
#include "MMatchStage.h"
#include "ZCommandTable.h"
#include "ZPost.h"
#include "ZPostLocal.h"
#include "MMatchNotify.h"
#include "ZMatch.h"
#include "MComboBox.h"
#include "MTextArea.h"
#include "ZCharacterViewList.h"
#include "ZCharacterView.h"
#include "MDebug.h"
#include "ZScreenEffectManager.h"
#include "ZRoomListBox.h"
#include "ZPlayerListBox.h"
#include "ZChat.h"
#include "ZWorldItem.h"
#include "ZChannelRule.h"
#include "ZNetRepository.h"
#include "ZMyInfo.h"
#include "MToolTip.h"
#include "ZColorTable.h"
#include "ZClan.h"
#include "ZItemDesc.h"
#include "ZCharacterSelectView.h"
#include "ZChannelListItem.h"
#include "ZCombatInterface.h"
#include "ZLocale.h"
#include "ZMap.h"
#include "UPnP.h"
#include "MMD5.h"
#include "ZPlayerManager.h"

MCommand* ZNewCmd(int nID)
{
	MCommandDesc* pCmdDesc = ZGetGameClient()->GetCommandManager()->GetCommandDescByID(nID);

	MUID uidTarget;
	if (pCmdDesc->IsFlag(MCDT_PEER2PEER) == true)
		uidTarget = MUID(0, 0);
	else
		uidTarget = ZGetGameClient()->GetServerUID();

	MCommand* pCmd = new MCommand(nID,
		ZGetGameClient()->GetUID(),
		uidTarget,
		ZGetGameClient()->GetCommandManager());
	return pCmd;
}

bool GetUserInfoUID(MUID uid, MCOLOR& _color, MMatchUserGradeID& gid)
{
	if (ZGetGameClient() == NULL)
		return false;

	MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(uid);

	if (pObjCache) {
		gid = pObjCache->GetUGrade();
	}

	return ZGetGame()->GetUserGradeIDColor(gid, _color);
}

extern MCommandLogFrame* m_pLogFrame;
extern ZIDLResource	g_IDLResource;

bool ZPostCommand(MCommand* pCmd)
{
	if (ZGetGame() && ZGetGame()->IsReplay())
	{
		switch (pCmd->GetID())
		{
		case MC_CLOCK_SYNCHRONIZE:
		case MC_MATCH_USER_WHISPER:
		case MC_MATCH_CHATROOM_JOIN:
		case MC_MATCH_CHATROOM_LEAVE:
		case MC_MATCH_CHATROOM_SELECT_WRITE:
		case MC_MATCH_CHATROOM_INVITE:
		case MC_MATCH_CHATROOM_CHAT:
		case MC_MATCH_CLAN_MSG:
		case MC_HSHIELD_PONG:
		case MC_RESPONSE_XTRAP_HASHVALUE:
		case MC_RESPONSE_GAMEGUARD_AUTH:
		case MC_RESPONSE_XTRAP_SEEDKEY:
			break;
		default:
			delete pCmd;
			return false;
		};
		return ZGetGameClient()->Post(pCmd);
	}
	else
	{
		bool bRet = ZGetGameClient()->Post(pCmd);

		int cmdId = pCmd->GetID();
		if (cmdId == MC_ADMIN_ANNOUNCE ||
			cmdId == MC_ADMIN_REQUEST_SERVER_INFO ||
			cmdId == MC_ADMIN_SERVER_HALT ||
			cmdId == MC_ADMIN_REQUEST_UPDATE_ACCOUNT_UGRADE ||
			cmdId == MC_ADMIN_REQUEST_KICK_PLAYER ||
			cmdId == MC_ADMIN_REQUEST_MUTE_PLAYER ||
			cmdId == MC_ADMIN_REQUEST_BLOCK_PLAYER ||
			cmdId == MC_ADMIN_PING_TO_ALL ||
			cmdId == MC_ADMIN_REQUEST_SWITCH_LADDER_GAME ||
			cmdId == MC_ADMIN_HIDE ||
			cmdId == MC_ADMIN_RESET_ALL_HACKING_BLOCK ||
			cmdId == MC_ADMIN_RELOAD_GAMBLEITEM ||
			cmdId == MC_ADMIN_DUMP_GAMBLEITEM_LOG ||
			cmdId == MC_ADMIN_ASSASIN ||

			cmdId == MC_MATCH_GAME_KILL ||
			cmdId == MC_MATCH_GAME_REQUEST_SPAWN ||

			cmdId == MC_MATCH_REQUEST_SUICIDE ||
			cmdId == MC_MATCH_REQUEST_OBTAIN_WORLDITEM ||
			cmdId == MC_MATCH_REQUEST_SPAWN_WORLDITEM ||
			cmdId == MC_MATCH_SET_OBSERVER ||

			cmdId == MC_MATCH_REQUEST_CREATE_CHAR ||
			cmdId == MC_MATCH_REQUEST_DELETE_CHAR ||

			cmdId == MC_MATCH_REQUEST_BUY_ITEM ||
			cmdId == MC_MATCH_REQUEST_SELL_ITEM ||
			cmdId == MC_MATCH_REQUEST_EQUIP_ITEM ||
			cmdId == MC_MATCH_REQUEST_TAKEOFF_ITEM ||

			cmdId == MC_MATCH_REQUEST_GAMBLE ||

			cmdId == MC_QUEST_REQUEST_NPC_DEAD ||
			cmdId == MC_MATCH_QUEST_REQUEST_DEAD ||

			cmdId == MC_QUEST_PEER_NPC_BASICINFO ||
			cmdId == MC_QUEST_PEER_NPC_HPINFO ||
			cmdId == MC_QUEST_PEER_NPC_DEAD ||
			cmdId == MC_QUEST_PEER_NPC_BOSS_HPAP ||
			cmdId == MC_QUEST_REQUEST_MOVETO_PORTAL ||
			cmdId == MC_QUEST_TEST_REQUEST_NPC_SPAWN ||
			cmdId == MC_QUEST_TEST_REQUEST_CLEAR_NPC ||
			cmdId == MC_QUEST_TEST_REQUEST_SECTOR_CLEAR ||
			cmdId == MC_QUEST_TEST_REQUEST_FINISH ||

			cmdId == MC_PEER_MOVE ||
			cmdId == MC_PEER_ATTACK ||
			cmdId == MC_PEER_DAMAGE ||
			cmdId == MC_PEER_SHOT ||
			cmdId == MC_PEER_SHOT_SP ||
			cmdId == MC_PEER_SKILL ||
			cmdId == MC_PEER_SHOT_MELEE ||
			cmdId == MC_PEER_DIE ||
			cmdId == MC_PEER_SPAWN ||
			cmdId == MC_PEER_DASH ||
			cmdId == MC_PEER_CHAT)
		{
			extern DWORD g_dwMainThreadID;
			if (g_dwMainThreadID != GetCurrentThreadId())
			{
#ifdef _DEBUG
				mlog("CMD THREAD MISMATCH : cmdId(%d), mainId(%d), currId(%d)\n", cmdId, g_dwMainThreadID, GetCurrentThreadId());
#endif
				MCommand* pC = ZNewCmd(MC_REQUEST_GIVE_ONESELF_UP);
				ZPostCommand(pC);
			}

			return bRet;
		}
	}
	return true;
}

ZGameClient::ZGameClient() : MMatchClient(), m_pUPnP(NULL)
{
	m_pUPnP = new UPnP;

	m_uidPlayer = MUID(0, 0);
	m_nClockDistance = 0;
	m_fnOnCommandCallback = NULL;
	m_nPrevClockRequestAttribute = 0;
	m_nBridgePeerCount = 0;
	m_tmLastBridgePeer = 0;
	m_bForcedEntry = false;
	IsRejoin = false;

	m_szChannel[0] = NULL;
	m_szStageName[0] = NULL;
	m_szChatRoomInvited[0] = NULL;
	SetChannelRuleName("");

	m_nRoomNo = 0;
	m_nStageCursor = 0;

	m_nCountdown = 0;
	m_tmLastCountdown = 0;
	m_nRequestID = 0;
	m_uidRequestPlayer = MUID(0, 0);
	m_nProposalMode = MPROPOSAL_NONE;
	m_bLadderGame = false;

	m_CurrentChannelType = MCHANNEL_TYPE_PRESET;

	SetRejectWhisper(true);
	SetRejectInvite(true);

	SetVoteInProgress(false);
	SetCanVote(false);

	m_EmblemMgr.Create();
	m_EmblemMgr.PrepareCache();

	memset(&m_dtCharInfo, 0, sizeof(m_dtCharInfo));
	memset(&m_dtCharInfoPrev, 0, sizeof(m_dtCharInfoPrev));

#ifdef _LOCATOR
	m_This = MUID(0, 1);
#endif

	LastVoteID = -1;
	bMatching = false;;

	m_UPDCommadHackShield.Init();
}

ZGameClient::~ZGameClient()
{
	DestroyUPnP();
	m_EmblemMgr.Destroy();

	ZGetMyInfo()->Clear();

#ifdef LOCALE_NHNUSAA
	GetNHNUSAReport().ReportCloseGame();
#endif
}

void ZGameClient::PriorityBoost(bool bBoost)
{
#ifdef _GAMEGUARD
	return;
#endif

	if (bBoost) {
		SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
		m_bPriorityBoost = true;
		OutputDebugString("<<<<  BOOST ON  >>>> \n");
	}
	else {
		SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
		m_bPriorityBoost = false;
		OutputDebugString("<<<<  BOOST OFF  >>>> \n");
	}
}

void ZGameClient::OnRegisterCommand(MCommandManager* pCommandManager)
{
	MMatchClient::OnRegisterCommand(pCommandManager);
	ZAddCommandTable(pCommandManager);
}

void ZGameClient::OnPrepareCommand(MCommand* pCommand)
{
#ifndef _PUBLISH
	m_pLogFrame->AddCommand(GetGlobalClockCount(), pCommand);
#endif
}

int ZGameClient::OnResponseMatchLogin(const MUID& uidServer, int nResult, const char* szServerName, const MMatchServerMode nServerMode,
	const char* szAccountID, const MMatchUserGradeID nUGradeID, const MMatchPremiumGradeID nPGradeID,
	const MUID& uidPlayer, bool bEnabledSurvivalMode, bool bEnabledDuelTournament, unsigned char* pbyGuidReqMsg)
{
	int nRet = MMatchClient::OnResponseMatchLogin(uidServer, nResult, szServerName, nServerMode,
		szAccountID, nUGradeID, nPGradeID, uidPlayer, bEnabledSurvivalMode, bEnabledDuelTournament, pbyGuidReqMsg);

	ZGetMyInfo()->InitAccountInfo(szAccountID, nUGradeID, nPGradeID);

	if ((nResult == 0) && (nRet == MOK)) {
		mlog("Login Successful. \n");

#ifdef _HSHIELD
		int dwRet = _AhnHS_MakeGuidAckMsg(pbyGuidReqMsg,
			ZGetMyInfo()->GetSystemInfo()->pbyGuidAckMsg
		);
		if (dwRet != ERROR_SUCCESS)
			mlog("Making Guid Ack Msg Failed. (Error code = %x)\n", dwRet);
#endif

		ZApplication::GetGameInterface()->ChangeToCharSelection();
	}
	else {
		mlog("Login Failed.(ErrCode=%d) \n", nResult);

#ifdef LOCALE_NHNUSAA
		if (nResult == 10003)
		{
			ZApplication::GetGameInterface()->SetErrMaxPlayer(true);
			ZApplication::GetGameInterface()->SetErrMaxPlayerDelayTime(timeGetTime() + 7000);
		}
		else
		{
			ZPostDisconnect();
			if (nResult != MOK)
				ZApplication::GetGameInterface()->ShowErrorMessage(nResult);
		}
#else
		ZPostDisconnect();

		if (nResult != MOK)
		{
			ZApplication::GetGameInterface()->ShowErrorMessage(nResult);
		}
#endif
		return MOK;
	}

	ZApplication::GetGameInterface()->ShowWidget("NetmarbleLogin", false);

	StartBridgePeer();

	return MOK;
}

void ZGameClient::OnAnnounce(unsigned int nType, char* szMsg)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if (strncmp(szMsg, MTOK_ANNOUNCE_PARAMSTR, strlen(MTOK_ANNOUNCE_PARAMSTR)) == 0)
	{
		const char* szId = szMsg + strlen(MTOK_ANNOUNCE_PARAMSTR);
		int idErrMsg = 0;
		if (1 == sscanf(szId, "%d", &idErrMsg)) {
			char szTranslated[256];
			const char* szErrStr = ZErrStr(idErrMsg);
			const char* szArg = "";

			const char* pSeperator = strchr(szMsg, '\a');
			if (pSeperator) {
				szArg = pSeperator + 1;
			}

			sprintf(szTranslated, szErrStr, szArg);
			ZChatOutput(szTranslated, ZChat::CMT_SYSTEM);
			return;
		}
	}

	ZChatOutput(szMsg, ZChat::CMT_SYSTEM);
}

void ZGameClient::OnBridgePeerACK(const MUID& uidChar, int nCode)
{
	SetBridgePeerFlag(true);
}

void ZGameClient::OnObjectCache(unsigned int nType, void* pBlob, int nCount)
{
	MMatchClient::OnObjectCache(nType, pBlob, nCount);

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	ZPlayerListBox* pList = (ZPlayerListBox*)pResource->FindWidget("StagePlayerList_");

	vector< int > vecClanID;

	if (pList)
	{
		if (nType == MATCHCACHEMODE_UPDATE) {
			pList->RemoveAll();
			ZGetPlayerManager()->Clear();
			for (int i = 0; i < nCount; i++) {
				MMatchObjCache* pCache = (MMatchObjCache*)MGetBlobArrayElement(pBlob, i);
				if (pCache->CheckFlag(MTD_PlayerFlags_AdminHide) == false) {
					pList->AddPlayer(pCache->GetUID(), MOSS_NONREADY, pCache->GetLevel(),
						pCache->GetName(), pCache->GetClanName(), pCache->GetCLID(), false, MMT_ALL, pCache->GetDTGrade());

					if (m_EmblemMgr.CheckEmblem(pCache->GetCLID(), pCache->GetEmblemChecksum())) {
					}
					else if (pCache->GetEmblemChecksum() != 0) {
						vecClanID.push_back(pCache->GetCLID());
					}

					ZGetPlayerManager()->AddPlayer(pCache->GetUID(), pCache->GetName(), pCache->GetRank(), pCache->GetKillCount(), pCache->GetDeathCount());
				}
			}
		}
		else if (nType == MATCHCACHEMODE_ADD) {
			for (int i = 0; i < nCount; i++) {
				MMatchObjCache* pCache = (MMatchObjCache*)MGetBlobArrayElement(pBlob, i);
				if (pCache->CheckFlag(MTD_PlayerFlags_AdminHide) == false) {
					pList->AddPlayer(pCache->GetUID(), MOSS_NONREADY, pCache->GetLevel(),
						pCache->GetName(), pCache->GetClanName(), pCache->GetCLID(), false, MMT_ALL, pCache->GetDTGrade());

					if (m_EmblemMgr.CheckEmblem(pCache->GetCLID(), pCache->GetEmblemChecksum())) {
					}
					else if (pCache->GetEmblemChecksum() != 0) {
						vecClanID.push_back(pCache->GetCLID());
					}

					ZGetPlayerManager()->AddPlayer(pCache->GetUID(), pCache->GetName(), pCache->GetRank(), pCache->GetKillCount(), pCache->GetDeathCount());
				}
			}
		}
		else if (nType == MATCHCACHEMODE_REMOVE) {
			for (int i = 0; i < nCount; i++) {
				MMatchObjCache* pCache = (MMatchObjCache*)MGetBlobArrayElement(pBlob, i);
				pList->DelPlayer(pCache->GetUID());

				ZGetPlayerManager()->RemovePlayer(pCache->GetUID());
			}

			ZApplication::GetGameInterface()->UpdateBlueRedTeam();
		}

		if (vecClanID.size() > 0)
		{
			void* pBlob = MMakeBlobArray(sizeof(int), (int)vecClanID.size());
			int nCount = 0;
			for (vector<int>::iterator it = vecClanID.begin(); it != vecClanID.end(); it++, nCount++)
			{
				int* nClanID = (int*)MGetBlobArrayElement(pBlob, nCount);
				*nClanID = *it;
			}

			ZPostRequestEmblemURL(pBlob);
			MEraseBlobArray(pBlob);
			vecClanID.clear();
		}
	}
}

void ZGameClient::OnChannelResponseJoin(const MUID& uidChannel, MCHANNEL_TYPE nChannelType, const char* szChannelName, bool bEnableInterface)
{
	ZApplication::GetGameInterface()->SetState(GUNZ_LOBBY);

	m_uidChannel = uidChannel;
	strcpy(m_szChannel, szChannelName);
	m_CurrentChannelType = nChannelType;
	m_bEnableInterface = bEnableInterface;

	char szText[256];

	ZGetGameInterface()->GetChat()->Clear(ZChat::CL_LOBBY);
	ZTransMsg(szText, MSG_LOBBY_JOIN_CHANNEL, 1, szChannelName);

	ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_LOBBY);

	switch (GetServerMode())
	{
	case MSM_NORMALS:
	{
		wsprintf(szText,
			ZMsg(MSG_LOBBY_LIMIT_LEVEL));
		ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_LOBBY);
	}
	break;
	case MSM_LADDER:
	{
		wsprintf(szText,
			ZMsg(MSG_LOBBY_LEAGUE));
		ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_LOBBY);
	}
	break;
	case MSM_CLAN:
	{
		if (nChannelType == MCHANNEL_TYPE_CLAN)
		{
			ZPostRequestClanInfo(GetPlayerUID(), szChannelName);
		}
	}
	break;
	}

	ZRoomListBox* pRoomList =
		(ZRoomListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_StageList");
	if (pRoomList) pRoomList->Clear();

	ZApplication::GetGameInterface()->SetRoomNoLight(1);

	ZGetGameInterface()->InitLobbyUIByChannelType();
}

void ZGameClient::OnChannelChat(const MUID& uidChannel, char* szName, char* szChat, int nGrade)
{
	if (GetChannelUID() != uidChannel)		return;
	if ((szChat[0] == 0) || (szName[0] == 0))	return;

	MCOLOR _color = MCOLOR(0, 0, 0);
	MMatchUserGradeID gid = (MMatchUserGradeID)nGrade;

	bool bSpUser = ZGetGame()->GetUserGradeIDColor(gid, _color);
	char szText[512];

	if (bSpUser)
	{
		wsprintf(szText, "%s: %s", szName, szChat);
		ZChatOutput(szText, ZChat::CMT_NORMAL, ZChat::CL_LOBBY, _color);
	}
	else if (!ZGetGameClient()->GetRejectNormalChat() ||
		(strcmp(szName, ZGetMyInfo()->GetCharName()) == 0))
	{
		wsprintf(szText, "^4%s^9: %s", szName, szChat);
		ZChatOutput(szText, ZChat::CMT_NORMAL, ZChat::CL_LOBBY);
	}
}

void ZGameClient::OnChannelList(void* pBlob, int nCount)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MListBox* pWidget = (MListBox*)pResource->FindWidget("ChannelList");
	if (pWidget == NULL) {
		ZGetGameClient()->StopChannelList();
		return;
	}

	int nStartIndex = pWidget->GetStartItem();
	int nSelIndex = pWidget->GetSelIndex();
	const char* szChannelName = NULL;
	pWidget->RemoveAll();
	for (int i = 0; i < nCount; i++) {
		MCHANNELLISTNODE* pNode = (MCHANNELLISTNODE*)MGetBlobArrayElement(pBlob, i);

		if (pNode->szChannelNameStrResId[0] != 0) {
			szChannelName = ZGetStringResManager()->GetStringFromXml(pNode->szChannelNameStrResId);
		}
		else {
			szChannelName = pNode->szChannelName;
		}

		pWidget->Add(
			new ZChannelListItem(pNode->uidChannel, (int)pNode->nNo, szChannelName,
				pNode->nChannelType, (int)pNode->nPlayers, (int)pNode->nMaxPlayers)
		);
	}
	pWidget->SetStartItem(nStartIndex);
	pWidget->SetSelIndex(nSelIndex);
}

void ZGameClient::OnChannelResponseRule(const MUID& uidchannel, const char* pszRuleName)
{
	MChannelRule* pRule = ZGetChannelRuleMgr()->GetRule(pszRuleName);
	if (pRule == NULL)
		return;

	SetChannelRuleName(pszRuleName);

	MComboBox* pCombo = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("MapSelection");
	if (pCombo != NULL)
	{
		InitMaps(pCombo);
		MListBox* pList = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("MapList");
		pList->RemoveAll();
		if (pList != NULL)
		{
			for (int i = 0; i < pCombo->GetCount(); ++i)
			{
				pList->Add(pCombo->GetString(i));
			}
		}
	}

	bool bEnable = GetEnableInterface();

	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageJoin");
	if (pWidget)		pWidget->Enable(bEnable);

	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageCreateFrameCaller");
	if (pWidget)		pWidget->Enable(bEnable);

	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("QuickJoin");
	if (pWidget)		pWidget->Enable(bEnable);

	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("QuickJoin2");
	if (pWidget)		pWidget->Enable(bEnable);

	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ArrangedTeamGame");
	if (pWidget)		pWidget->Enable(bEnable);

	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ChannelChattingInput");
	if (pWidget)		pWidget->Enable(bEnable);

	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_StageList");
	if (pWidget)		pWidget->Enable(bEnable);

	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("RequestLadderRejoin");
	if (pWidget)		pWidget->Enable(bEnable);
}

void ZGameClient::OnStageEnterBattle(const MUID& uidChar, MCmdEnterBattleParam nParam)
{
	if (uidChar == GetPlayerUID())
	{
		ZPostRequestGameInfo(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());

		MStageCharSettingList::iterator itor = m_MatchStageSetting.m_CharSettingList.begin();
		for (; itor != m_MatchStageSetting.m_CharSettingList.end(); ++itor)
		{
			MSTAGE_CHAR_SETTING_NODE* pCharNode = (*itor);
			pCharNode->nState = MOSS_NONREADY;
		}
	}

	StartUDPTest(uidChar);
}

void ZGameClient::OnStageJoin(const MUID& uidChar, const MUID& uidStage, unsigned int nRoomNo, char* szStageName)
{
	if (uidChar == GetPlayerUID()) {
		m_nStageCursor = 0;
		m_uidStage = uidStage;
		m_nRoomNo = nRoomNo;

		memset(m_szStageName, 0, sizeof(m_szStageName));
		strcpy(m_szStageName, szStageName);

		unsigned int nStageNameChecksum = m_szStageName[0] + m_szStageName[1] + m_szStageName[2] + m_szStageName[3];
		InitPeerCrypt(uidStage, nStageNameChecksum);
		CastStageBridgePeer(uidChar, uidStage);
	}

	MCommand* pCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_REQUEST_STAGESETTING), GetServerUID(), m_This);
	pCmd->AddParameter(new MCommandParameterUID(GetStageUID()));
	Post(pCmd);

	if (uidChar == GetPlayerUID())
	{
		ZChangeGameState(GUNZ_STAGE);
	}

	string name = GetObjName(uidChar);
	char szText[256];
	if (uidChar == GetPlayerUID())
	{
		ZGetGameInterface()->GetChat()->Clear(ZChat::CL_STAGE);

		char szTmp[256];
		sprintf(szTmp, "(%03d)%s", nRoomNo, szStageName);

		ZTransMsg(szText, MSG_JOINED_STAGE, 1, szTmp);
		ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_STAGE);
	}
	else if (GetStageUID() == uidStage)
	{
		MCOLOR _color;
		MMatchUserGradeID gid = MMUG_FREE;

		char name[32];
		char kill[32];
		char death[32];
		char winning[32];

		ZPlayerInfo* pInfo = ZGetPlayerManager()->Find((MUID)uidChar);
		if (pInfo != NULL)
		{
			sprintf(kill, "%d %s", pInfo->GetKill(), ZMsg(MSG_CHARINFO_KILL));
			sprintf(death, "%d %s", pInfo->GetDeath(), ZMsg(MSG_CHARINFO_DEATH));
			sprintf(winning, "%.1f%%", pInfo->GetWinningRatio());
		}
		else
		{
			sprintf(kill, "? %s", ZMsg(MSG_CHARINFO_KILL));
			sprintf(death, "? %s", ZMsg(MSG_CHARINFO_DEATH));
			sprintf(winning, "0.0%%");
		}

		if (GetUserInfoUID(uidChar, _color, gid))
		{
			MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(uidChar);
			if (pObjCache && pObjCache->CheckFlag(MTD_PlayerFlags_AdminHide))
				return;	// Skip on AdminHide

			strcpy(name, pInfo->GetName());
			ZTransMsg(szText, MSG_JOINED_STAGE2, 4, name, kill, death, winning);
			ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_STAGE);
		}
		else
		{
			strcpy(name, pInfo->GetName());
			ZTransMsg(szText, MSG_JOINED_STAGE2, 4, name, kill, death, winning);
			ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_STAGE);
		}
	}
}

void ZGameClient::OnStageLeave(const MUID& uidChar, const MUID& uidStage)
{
	if (uidChar == GetPlayerUID()) {
		m_uidStage = MUID(0, 0);
		m_nRoomNo = 0;
	}

	if (uidChar == GetPlayerUID())
	{
		ZChangeGameState(GUNZ_LOBBY);
	}

	for (MStageCharSettingList::iterator i = m_MatchStageSetting.m_CharSettingList.begin(); i != m_MatchStageSetting.m_CharSettingList.end(); i++) {
		if (uidChar == (*i)->uidChar)
		{
			delete (*i);
			m_MatchStageSetting.m_CharSettingList.erase(i);
			break;
		}
	}

	if (uidChar == GetPlayerUID())
	{
		if (ZGetGame() == NULL)
		{
			std::list<MCommand*>::iterator it = ZGetGameInterface()->m_listDelayedGameCmd.begin();
			for (; it != ZGetGameInterface()->m_listDelayedGameCmd.end();)
			{
				MCommand* pWaitCmd = *it;
				delete pWaitCmd;
				it = ZGetGameInterface()->m_listDelayedGameCmd.erase(it);
			}
		}
	}

	ZGetGameClient()->SetVoteInProgress(false);
	ZGetGameClient()->SetCanVote(false);

	AgentDisconnect();
}

void ZGameClient::OnStageStart(const MUID& uidChar, const MUID& uidStage, int nCountdown)
{
	ZApplication::GetStageInterface()->SetEnableWidgetByRelayMap(false);
	SetCountdown(nCountdown);
}

void ZGameClient::OnStageRelayStart()
{
	SetCountdown(3);
}

void ZGameClient::OnStageLaunch(const MUID& uidStage, const char* pszMapName)
{
	m_bLadderGame = false;

	SetAllowTunneling(false);

	m_MatchStageSetting.SetMapName(const_cast<char*>(pszMapName));

	if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME) {
		ZChangeGameState(GUNZ_GAME);
	}
}

void ZGameClient::OnStageFinishGame(const MUID& uidStage, const bool bIsRelayMapUnFinish)
{
	if (ZApplication::GetGameInterface()->GetState() == GUNZ_GAME)
	{
		ZApplication::GetGameInterface()->FinishGame();
	}
	else if (ZApplication::GetGameInterface()->GetState() == GUNZ_STAGE) {
	}
	bool bEndRelayMap = !bIsRelayMapUnFinish;
	ZApplication::GetStageInterface()->SetEnableWidgetByRelayMap(bEndRelayMap);

	ZPostRequestStageSetting(ZGetGameClient()->GetStageUID());
}

void ZGameClient::OnStageMap(const MUID& uidStage, char* szMapName, bool bIsRelayMap)
{
	if (uidStage != GetStageUID()) return;

	m_MatchStageSetting.SetMapName(szMapName);
	m_MatchStageSetting.SetIsRelayMap(strcmp(MMATCH_MAPNAME_RELAYMAP, szMapName) == 0);

	ZApplication::GetGameInterface()->SerializeStageInterface();
}

void ZGameClient::OnStageTeam(const MUID& uidChar, const MUID& uidStage, unsigned int nTeam)
{
	MMatchObjectStageState nStageState = MOSS_NONREADY;
	MSTAGE_CHAR_SETTING_NODE* pCharNode = m_MatchStageSetting.FindCharSetting(uidChar);
	if (pCharNode)
	{
		nStageState = pCharNode->nState;
	}

	m_MatchStageSetting.UpdateCharSetting(uidChar, nTeam, nStageState);
	ZApplication::GetGameInterface()->SerializeStageInterface();
}

void ZGameClient::OnStagePlayerState(const MUID& uidChar, const MUID& uidStage, MMatchObjectStageState nStageState)
{
	int nTeam = MMT_SPECTATOR;
	MSTAGE_CHAR_SETTING_NODE* pCharNode = m_MatchStageSetting.FindCharSetting(uidChar);
	if (pCharNode != NULL)
	{
		nTeam = pCharNode->nTeam;
	}

	m_MatchStageSetting.UpdateCharSetting(uidChar, nTeam, nStageState);

	GunzState GunzState = ZApplication::GetGameInterface()->GetState();
	if (GunzState == GUNZ_STAGE)
	{
		ZApplication::GetStageInterface()->OnStageCharListSettup();
	}
}

void ZGameClient::OnStageMaster(const MUID& uidStage, const MUID& uidChar)
{
	int nTeam = MMT_SPECTATOR;
	MMatchObjectStageState nStageState = MOSS_NONREADY;
	MSTAGE_CHAR_SETTING_NODE* pCharNode = m_MatchStageSetting.FindCharSetting(uidChar);
	if (pCharNode)
	{
		nTeam = pCharNode->nTeam;
		nStageState = pCharNode->nState;
	}

	m_MatchStageSetting.SetMasterUID(uidChar);
	m_MatchStageSetting.UpdateCharSetting(uidChar, nTeam, nStageState);

	ZApplication::GetGameInterface()->SerializeStageInterface();
}

void ZGameClient::OnStageChat(const MUID& uidChar, const MUID& uidStage, char* szChat)
{
	if (GetStageUID() != uidStage) return;
	if (szChat[0] == 0) return;

	string name = GetObjName(uidChar);
	MCOLOR _color = MCOLOR(0, 0, 0);
	MMatchUserGradeID gid = MMUG_FREE;
	MMatchObjCache* pObjCache = FindObjCache(uidChar);

	if (pObjCache) {
		gid = pObjCache->GetUGrade();
	}

	bool bSpUser = ZGetGame()->GetUserGradeIDColor(gid, _color);

	char szText[512];

	if (bSpUser)
	{
		wsprintf(szText, "%s: %s", pObjCache->GetName(), szChat);
		ZChatOutput(szText, ZChat::CMT_NORMAL, ZChat::CL_STAGE, _color);
	}
	else if (!ZGetGameClient()->GetRejectNormalChat() ||
		(strcmp(pObjCache->GetName(), ZGetMyInfo()->GetCharName()) == 0))
	{
		wsprintf(szText, "^4%s^9: %s", name.c_str(), szChat);
		ZChatOutput(szText, ZChat::CMT_NORMAL, ZChat::CL_STAGE);
	}
}

void ZGameClient::OnStageList(int nPrevStageCount, int nNextStageCount, void* pBlob, int nCount)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	ZRoomListBox* pRoomListBox = (ZRoomListBox*)pResource->FindWidget("Lobby_StageList");
	if (pRoomListBox == NULL)
	{
		ZGetGameClient()->StopStageList();
		return;
	}

	pRoomListBox->Clear();
	for (int i = 0; i < nCount; i++) {
		MTD_StageListNode* pNode = (MTD_StageListNode*)MGetBlobArrayElement(pBlob, i);

		if (pNode)
		{
			bool bForcedEntry = false, bPrivate = false, bLimitLevel = false;
			int nLimitLevel = 0;
			if (pNode->nSettingFlag & MSTAGENODE_FLAG_FORCEDENTRY_ENABLED) bForcedEntry = true;
			if (pNode->nSettingFlag & MSTAGENODE_FLAG_PRIVATE) bPrivate = true;
			if (pNode->nSettingFlag & MSTAGENODE_FLAG_LIMITLEVEL) bLimitLevel = true;

			char szMapName[256] = "";
			for (int tt = 0; tt < MMATCH_MAP_COUNT; tt++)
			{
				if (MGetMapDescMgr()->GetMapID(tt) == pNode->nMapIndex)
				{
					strcpy(szMapName, MGetMapDescMgr()->GetMapName(tt));
					break;
				}
			}

			ZRoomListBox::_RoomInfoArg roominfo;
			roominfo.nIndex = i;
			roominfo.nRoomNumber = (int)pNode->nNo;
			roominfo.uidStage = pNode->uidStage;
			roominfo.szRoomName = pNode->szStageName;
			roominfo.szMapName = szMapName;
			roominfo.nMaxPlayers = pNode->nMaxPlayers;
			roominfo.nCurrPlayers = pNode->nPlayers;
			roominfo.bPrivate = bPrivate;
			roominfo.bForcedEntry = bForcedEntry;
			roominfo.bLimitLevel = bLimitLevel;
			roominfo.nMasterLevel = pNode->nMasterLevel;
			roominfo.nLimitLevel = pNode->nLimitLevel;
			roominfo.nGameType = pNode->nGameType;
			roominfo.nStageState = pNode->nState;
			pRoomListBox->SetRoom(&roominfo);
		}
	}
	pRoomListBox->SetScroll(nPrevStageCount, nNextStageCount);

	MWidget* pBtn = pResource->FindWidget("StageBeforeBtn");
	if (nPrevStageCount != -1)
	{
		if (nPrevStageCount == 0)
		{
			if (pBtn) pBtn->Enable(false);
		}
		else
		{
			if (pBtn) pBtn->Enable(true);
		}
	}

	pBtn = pResource->FindWidget("StageAfterBtn");
	if (nNextStageCount != -1)
	{
		if (nNextStageCount == 0)
		{
			if (pBtn) pBtn->Enable(false);
		}
		else
		{
			if (pBtn) pBtn->Enable(true);
		}
	}
}

ZPlayerListBox* GetProperFriendListOutput()
{
	ZIDLResource* pIDLResource = ZApplication::GetGameInterface()->GetIDLResource();

	GunzState nState = ZApplication::GetGameInterface()->GetState();
	switch (nState) {
	case GUNZ_LOBBY:
	{
		ZPlayerListBox* pList = (ZPlayerListBox*)pIDLResource->FindWidget("LobbyChannelPlayerList");
		if (pList && pList->GetMode() == ZPlayerListBox::PLAYERLISTMODE_CHANNEL_FRIEND)
			return pList;
		else
			return NULL;
	}
	break;
	case GUNZ_STAGE:
	{
		ZPlayerListBox* pList = (ZPlayerListBox*)pIDLResource->FindWidget("StagePlayerList_");
		if (pList && pList->GetMode() == ZPlayerListBox::PLAYERLISTMODE_STAGE_FRIEND)
			return pList;
		else
			return NULL;
	}
	break;
	};
	return NULL;
}

void ZGameClient::OnResponseFriendList(void* pBlob, int nCount)
{
	ZPlayerListBox* pList = GetProperFriendListOutput();
	if (pList)
		pList->RemoveAll();

	char szBuf[128];
	for (int i = 0; i < nCount; i++) {
		MFRIENDLISTNODE* pNode = (MFRIENDLISTNODE*)MGetBlobArrayElement(pBlob, i);

		ePlayerState state;
		switch (pNode->nState)
		{
		case MMP_LOBBY: state = PS_LOBBY; break;
		case MMP_STAGE: state = PS_WAIT; break;
		case MMP_BATTLE: state = PS_FIGHT; break;
		default: state = PS_LOGOUT;
		};

		if (pList) {
			pList->AddPlayer(state, pNode->szName, pNode->szDescription);
		}
		else {
			if (ZApplication::GetGameInterface()->GetState() != GUNZ_LOBBY)
			{
				sprintf(szBuf, "%s (%s)", pNode->szName, pNode->szDescription);
				ZChatOutput(szBuf, ZChat::CMT_NORMAL, ZChat::CL_CURRENT);
			}
		}
	}
}

void ZGameClient::OnChannelPlayerList(int nTotalPlayerCount, int nPage, void* pBlob, int nCount)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	ZPlayerListBox* pPlayerListBox = (ZPlayerListBox*)pResource->FindWidget("LobbyChannelPlayerList");

	if (!pPlayerListBox) return;
	if (pPlayerListBox->GetMode() != ZPlayerListBox::PLAYERLISTMODE_CHANNEL) return;

	MUID selUID = pPlayerListBox->GetSelectedPlayerUID();

	int nStartIndex = pPlayerListBox->GetStartItem();

	if (nCount) {
		pPlayerListBox->RemoveAll();
	}
	else {
		return;
	}

	pPlayerListBox->m_nTotalPlayerCount = nTotalPlayerCount;
	pPlayerListBox->m_nPage = nPage;

	ZLobbyPlayerListItem* pItem = NULL;

	vector< int > vecClanID;

	for (int i = 0; i < nCount; i++)
	{
		MTD_ChannelPlayerListNode* pNode = (MTD_ChannelPlayerListNode*)MGetBlobArrayElement(pBlob, i);
		if (pNode)
		{
			ePlayerState state;
			switch (pNode->nPlace)
			{
			case MMP_LOBBY: state = PS_LOBBY; break;
			case MMP_STAGE: state = PS_WAIT; break;
			case MMP_BATTLE: state = PS_FIGHT; break;
			default: state = PS_LOBBY;
			};

			if ((pNode->nPlayerFlags & MTD_PlayerFlags_AdminHide) == true) {
			}
			else {
				if (m_EmblemMgr.CheckEmblem(pNode->nCLID, pNode->nEmblemChecksum)) {
				}
				else if (pNode->nEmblemChecksum != 0) {
					vecClanID.push_back(pNode->nCLID);
				}

				pPlayerListBox->AddPlayer(pNode->uidPlayer, state, pNode->nLevel, pNode->szName, pNode->szClanName, pNode->nCLID,
					(MMatchUserGradeID)pNode->nGrade, pNode->nDTLastWeekGrade);
			}
		}
	}

	if (vecClanID.size() > 0)
	{
		void* pBlob = MMakeBlobArray(sizeof(int), (int)vecClanID.size());
		int nCount = 0;
		for (vector<int>::iterator it = vecClanID.begin(); it != vecClanID.end(); it++, nCount++)
		{
			int* nClanID = (int*)MGetBlobArrayElement(pBlob, nCount);
			*nClanID = *it;
		}

		ZPostRequestEmblemURL(pBlob);
		MEraseBlobArray(pBlob);
		vecClanID.clear();
	}

	pPlayerListBox->SetStartItem(nStartIndex);
	pPlayerListBox->SelectPlayer(selUID);

	pPlayerListBox->AddTestItems();
}

void ZGameClient::OnChannelAllPlayerList(const MUID& uidChannel, void* pBlob, int nBlobCount)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MListBox* pListBox = NULL;

	MWidget* pDialog = pResource->FindWidget("ClanCreateDialog");
	if (pDialog && pDialog->IsVisible())
		pListBox = (MListBox*)pResource->FindWidget("ClanSponsorSelect");

	pDialog = pResource->FindWidget("ArrangedTeamGameDialog");
	if (pDialog && pDialog->IsVisible())
		pListBox = (MListBox*)pResource->FindWidget("ArrangedTeamSelect");

	if (pListBox && pListBox->IsVisible()) {
		pListBox->RemoveAll();
		for (int i = 0; i < nBlobCount; i++)
		{
			MTD_ChannelPlayerListNode* pNode = (MTD_ChannelPlayerListNode*)MGetBlobArrayElement(pBlob, i);
			if (pNode)
			{
				if (pNode->uidPlayer != GetPlayerUID())
					pListBox->Add(pNode->szName);
			}
		}
	}
}

void ZGameClient::UpdateStageSetting(MSTAGE_SETTING_NODE* pSetting, STAGE_STATE nStageState, const MUID& uidMaster)
{
	m_MatchStageSetting.UpdateStageSetting(pSetting);
	m_MatchStageSetting.SetMasterUID(uidMaster);
	m_MatchStageSetting.SetStageState(nStageState);

	bool bForceEntry = false;
	if (nStageState != STAGE_STATE_STANDBY)
	{
		bForceEntry = true;
	}
	m_bForcedEntry = bForceEntry;

	ZApplication::GetGameInterface()->SerializeStageInterface();
}

void ZGameClient::OnStageRelayMapListUpdate(int nRelayMapType, int nRelayMapRepeatCount, void* pStageRelayMapListBlob)
{
	MComboBox* pCBRelayMapType = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_RelayMapType");
	if (pCBRelayMapType)
		pCBRelayMapType->SetSelIndex(nRelayMapType);
	MComboBox* pCBRelayMapRepeatCount = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_RelayMapRepeatCount");
	if (pCBRelayMapRepeatCount)
		pCBRelayMapRepeatCount->SetSelIndex(nRelayMapRepeatCount);

	MListBox* pRelaMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_RelayMapListbox");
	if (pRelaMapListBox == NULL) return;

	RelayMap relayMapList[MAX_RELAYMAP_LIST_COUNT];
	for (int i = 0; i < MAX_RELAYMAP_LIST_COUNT; i++)
		relayMapList[i].nMapID = -1;
	pRelaMapListBox->RemoveAll();
	int nRelayMapListCount = MGetBlobArrayCount(pStageRelayMapListBlob);
	for (int i = 0; i < nRelayMapListCount; ++i)
	{
		MTD_RelayMap* pNode = (MTD_RelayMap*)MGetBlobArrayElement(pStageRelayMapListBlob, i);
		RelayMapList* pRelayMapList = new RelayMapList(MGetMapDescMgr()->GetMapName(MGetMapDescMgr()->GetMapID(pNode->nMapID)), MBitmapManager::Get("Mark_X.bmp"));
		pRelaMapListBox->Add(pRelayMapList);
		relayMapList[i].nMapID = MGetMapDescMgr()->GetMapID(pNode->nMapID);
	}

	ZGetGameClient()->GetMatchStageSetting()->SetRelayMapListCount(nRelayMapListCount);
	ZGetGameClient()->GetMatchStageSetting()->SetRelayMapList(relayMapList);

	ZApplication::GetStageInterface()->SetStageRelayMapImage();
}

void ZGameClient::OnStageRelayMapElementUpdate(int nRelayMapType, int nRelayMapRepeatCount)
{
	MComboBox* pCombo = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_RelayMapType");
	if (pCombo)
		pCombo->SetSelIndex(nRelayMapType);
	pCombo = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_RelayMapRepeatCount");
	if (pCombo)
		pCombo->SetSelIndex(nRelayMapRepeatCount);
}

void ZGameClient::OnResponseStageSetting(const MUID& uidStage, void* pStageBlob, int nStageCount, void* pCharBlob,
	int nCharCount, STAGE_STATE nStageState, const MUID& uidMaster)
{
	if (GetStageUID() != uidStage) return;
	if (nStageCount <= 0 || nCharCount <= 0) return;

	MSTAGE_SETTING_NODE* pNode = (MSTAGE_SETTING_NODE*)MGetBlobArrayElement(pStageBlob, 0);
	UpdateStageSetting(pNode, nStageState, uidMaster);

	m_MatchStageSetting.ResetCharSetting();
	for (int i = 0; i < nCharCount; i++) {
		MSTAGE_CHAR_SETTING_NODE* pCharSetting = (MSTAGE_CHAR_SETTING_NODE*)MGetBlobArrayElement(pCharBlob, i);
		m_MatchStageSetting.UpdateCharSetting(pCharSetting->uidChar, pCharSetting->nTeam, pCharSetting->nState);
	}

	ZApplication::GetGameInterface()->SerializeStageInterface();
}

void ZGameClient::OnAgentError(int nError)
{
	if (ZGetGame()) {
		const MCOLOR ChatColor = MCOLOR(0xffffffff);
		ZChatOutput(ChatColor, "Agent Error : Agent not available");
	}
}

void ZGameClient::OnMatchNotify(unsigned int nMsgID)
{
	string strMsg;
	NotifyMessage(nMsgID, &strMsg);

	if ((nMsgID == MATCHNOTIFY_GAME_SPEEDHACK) ||
		(nMsgID == MATCHNOTIFY_GAME_MEMORYHACK))
	{
		ZGetGameInterface()->ShowMessage(strMsg.c_str());
	}

	ZChatOutput(MCOLOR(255, 70, 70), strMsg.data());
}

void ZGameClient::OutputMessage(const char* szMessage, MZMOMType nType)
{
	OutputToConsole(szMessage);
	ZChatOutput(MCOLOR(0xFFFFC600), szMessage);
}

int ZGameClient::OnConnected(SOCKET sock, MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp)
{
	mlog("Server Connected\n");

	int ret = MMatchClient::OnConnected(sock, pTargetUID, pAllocUID, nTimeStamp);

	if (sock == m_ClientSocket.GetSocket())
	{
		char szID[256];
		char szPassword[256];
		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		MWidget* pWidget = pResource->FindWidget("LoginID");
		if (pWidget == NULL) return true;
		strcpy(szID, pWidget->GetText());
		pWidget = pResource->FindWidget("LoginPassword");
		if (pWidget == NULL) return true;
		strcpy(szPassword, pWidget->GetText());

		unsigned long nChecksum = 0;

		char szEncryptMD5Value[MAX_MD5LENGH] = { 0, };
		GetEncryptMD5HashValue(szEncryptMD5Value);

		ZPostLogin(szID, szPassword, nChecksum, szEncryptMD5Value);

		mlog("Login Posted\n");
	}

	return ret;
}

void ZGameClient::GetEncryptMD5HashValue(char* szEncryptMD5Value)
{
	char filePath[MAX_PATH] = { 0, };
	GetModuleFileName(NULL, filePath, MAX_PATH);

	unsigned char szMD5Value[MAX_MD5LENGH] = { 0, };
	MMD5* pMD5 = new MMD5;
	if (pMD5->md5_file(filePath, szMD5Value) != 0)
	{
		return;
	}
	delete pMD5;

	char szEncryptValue[MAX_MD5LENGH] = { 0, };
	memcpy(szEncryptValue, szMD5Value, MAX_MD5LENGH);

	m_ServerPacketCrypter.Encrypt(szEncryptValue, MAX_MD5LENGH, (MPacketCrypterKey*)m_ServerPacketCrypter.GetKey());

	memcpy(szEncryptMD5Value, szEncryptValue, MAX_MD5LENGH);
}

bool ZGameClient::OnSockConnect(SOCKET sock)
{
	ZPOSTCMD0(MC_NET_ONCONNECT);
	return MMatchClient::OnSockConnect(sock);
}

bool ZGameClient::OnSockDisconnect(SOCKET sock)
{
	if (sock == m_ClientSocket.GetSocket()) {
		AgentDisconnect();

		ZChangeGameState(GUNZ_LOGIN);
		ZPOSTCMD0(MC_NET_ONDISCONNECT);

		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		MButton* pWidget = (MButton*)pResource->FindWidget("LoginOK");
		if (pWidget) pWidget->Enable(true);
		MWidget* pLogin = pResource->FindWidget("LoginFrame");
		if (pLogin) pLogin->Show(true);
		pLogin = pResource->FindWidget("Login_ConnectingMsg");
		if (pLogin) pLogin->Show(false);

		ZGetGameInterface()->m_bLoginTimeout = false;
	}
	else if (sock == m_AgentSocket.GetSocket()) {
	}

	return true;
}

void ZGameClient::OnSockError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int& ErrorCode)
{
	MMatchClient::OnSockError(sock, ErrorEvent, ErrorCode);

	ZPOSTCMD1(MC_NET_ONERROR, MCmdParamInt(ErrorCode));

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MButton* pWidget = (MButton*)pResource->FindWidget("LoginOK");
	if (pWidget) pWidget->Enable(true);
	MWidget* pLogin = pResource->FindWidget("LoginFrame");
	if (pLogin) pLogin->Show(true);
	pLogin = pResource->FindWidget("Login_ConnectingMsg");
	if (pLogin) pLogin->Show(false);

	MLabel* pLabel = (MLabel*)pResource->FindWidget("LoginError");

	if (pLabel) {
		pLabel->SetText(ZErrStr(MERR_CLIENT_CONNECT_FAILED));
	}

	ZGetGameInterface()->m_bLoginTimeout = false;
}

#include "MListBox.h"
class MCharListItem : public MListItem {
	MUID	m_uid;
	char	m_szName[32];
public:
	MCharListItem(MUID uid, char* szName) {
		m_uid = uid; strcpy(m_szName, szName);
	}
	virtual ~MCharListItem() {}
	virtual const char* GetString(void) { return m_szName; }
	MUID GetUID() { return m_uid; }
	char* GetName() { return m_szName; }

public:
};

int ZGameClient::FindListItem(MListBox* pListBox, const MUID& uid)
{
	for (int i = 0; i < pListBox->GetCount(); i++) {
		MCharListItem* pItem = (MCharListItem*)pListBox->Get(i);
		if (pItem->GetUID() == uid) return i;
	}
	return -1;
}

unsigned long int ZGameClient::GetGlobalClockCount(void)
{
	unsigned long int nLocalClock = GetClockCount();
	if (m_bIsBigGlobalClock) return (nLocalClock + m_nClockDistance);
	else return (nLocalClock - m_nClockDistance);
}

unsigned long int ZGetClockDistance(unsigned long int nGlobalClock, unsigned long int nLocalClock)
{
	if (nGlobalClock > nLocalClock) {
		return nGlobalClock - nLocalClock;
	}
	else {
		return nLocalClock + (UINT_MAX - nGlobalClock + 1);
	}
}

void ZGameClient::StartBridgePeer()
{
	SetBridgePeerFlag(false);
	SetBridgePeerCount(10);

	UpdateBridgePeerTime(0);
}

void ZGameClient::Tick(void)
{
	m_MatchStageSetting.AntiHack_CheckCrc();

	unsigned long int nClock = GetGlobalClockCount();

	m_EmblemMgr.Tick(nClock);

	if ((GetBridgePeerCount() > 0) && (GetBridgePeerFlag() == false)) {
#define CLOCK_BRIDGE_PEER	200
		if (nClock - m_tmLastBridgePeer > CLOCK_BRIDGE_PEER) {
			SetBridgePeerCount(GetBridgePeerCount() - 1);
			UpdateBridgePeerTime(nClock);
			CastStageBridgePeer(GetPlayerUID(), GetStageUID());
		}
	}

	if (GetUDPTestProcess()) {
#define CLOCK_UDPTEST	500
		static unsigned long nUDPTestTimer = 0;
		if (nClock - nUDPTestTimer > CLOCK_UDPTEST) {
			nUDPTestTimer = nClock;

			MMatchPeerInfoList* PeerList = GetPeers();
			for (MMatchPeerInfoList::iterator i = PeerList->begin(); i != PeerList->end(); i++) {
				MMatchPeerInfo* pPeer = (*i).second;
				if (pPeer->GetProcess()) {
					MCommand* pCmd = CreateCommand(MC_PEER_UDPTEST, pPeer->uidChar);
					SendCommandByUDP(pCmd, pPeer->szIP, pPeer->nPort);
					delete pCmd;
				}
			}

			UpdateUDPTestProcess();
		}
	}

	if ((GetAgentPeerCount() > 0) && (GetAgentPeerFlag() == false)) {
		static unsigned long tmLastAgentPeer = 0;
#define CLOCK_AGENT_PEER	200
		if (nClock - tmLastAgentPeer > CLOCK_AGENT_PEER) {
			SetAgentPeerCount(GetAgentPeerCount() - 1);
			CastAgentPeerConnect();
			tmLastAgentPeer = nClock;
		}
	}

	if (ZApplication::GetGameInterface()->GetState() == GUNZ_STAGE) {
	}
}

void ZGameClient::OnResponseRecommandedChannel(const MUID& uidChannel)
{
	RequestChannelJoin(uidChannel);
}

void ZGameClient::OnBirdTest()
{
#ifdef _PUBLISH
	return;
#endif

	char szText[256];
	char szList[256]; szList[0] = '\0';

	int nCount = (int)m_ObjCacheMap.size();
	for (MMatchObjCacheMap::iterator itor = m_ObjCacheMap.begin(); itor != m_ObjCacheMap.end(); ++itor)
	{
		MMatchObjCache* pObj = (*itor).second;
		strcat(szList, pObj->GetName());
		strcat(szList, ", ");
	}

	sprintf(szText, "BirdTest: %d, %s", nCount, szList);
	MClient::OutputMessage(MZMOM_LOCALREPLY, szText);

	ZCharacterViewList* pWidget = ZGetCharacterViewList(GUNZ_STAGE);
	pWidget->RemoveAll();

	pWidget = ZGetCharacterViewList(GUNZ_LOBBY);
	pWidget->RemoveAll();
}

void ZGameClient::OnForcedEntryToGame()
{
	m_bLadderGame = false;
	m_bForcedEntry = true;
	SetAllowTunneling(false);
	ZChangeGameState(GUNZ_GAME);
}

void ZGameClient::ClearStageSetting()
{
	m_bForcedEntry = false;

	m_MatchStageSetting.Clear();
}

void ZGameClient::OnLoadingComplete(const MUID& uidChar, int nPercent)
{
	if (ZGetGame())
	{
		ZCharacter* pCharacter = ZGetGame()->m_CharacterManager.Find(uidChar);
		if (pCharacter != NULL)
		{
			MEMBER_SET_CHECKCRC(pCharacter->GetStatus(), nLoadingPercent, nPercent);
		}
	}
}

void ZGameClient::OnResponsePeerRelay(const MUID& uidPeer)
{
	string strNotify = "Unknown Notify";
	NotifyMessage(MATCHNOTIFY_NETWORK_NAT_ESTABLISH, &strNotify);

	char* pszName = "UnknownPlayer";
	MMatchPeerInfo* pPeer = FindPeer(uidPeer);
	if (pPeer) pszName = pPeer->CharInfo.szName;

	char szMsg[128];
	sprintf(szMsg, "%s : from %s", strNotify.c_str(), pszName);

	ZCharacter* pChar = ZGetCharacterManager()->Find(uidPeer);
	if (pChar && pChar->IsAdminHide())
		return;

	ZChatOutput(szMsg, ZChat::CMT_SYSTEM);
}

void ZGameClient::StartStageList()
{
	MCommand* pCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_LIST_START), GetServerUID(), m_This);
	Post(pCmd);
}

void ZGameClient::StopStageList()
{
	MCommand* pCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_LIST_STOP), GetServerUID(), m_This);
	Post(pCmd);
}

void ZGameClient::StartChannelList(MCHANNEL_TYPE nChannelType)
{
	ZPostStartChannelList(GetPlayerUID(), (int)nChannelType);
}

void ZGameClient::StopChannelList()
{
	ZPostStopChannelList(GetPlayerUID());
}

void ZGameClient::ReleaseForcedEntry()
{
	m_bForcedEntry = false;
}

void ZGameClient::OnAdminAnnounce(const char* szMsg, const ZAdminAnnounceType nType)
{
	switch (nType)
	{
	case ZAAT_CHAT:
	{
		char szText[512];
		ZTransMsg(szText, MSG_ADMIN_ANNOUNCE, 1, szMsg);
		ZChatOutput(szText, ZChat::CMT_SYSTEM);
	}
	break;
	case ZAAT_MSGBOX:
	{
		if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME)
		{
			ZApplication::GetGameInterface()->ShowMessage(szMsg);
		}
		else
		{
			ZChatOutput(szMsg);
		}
	}
	break;
	}
}

void ZGameClient::OnGameLevelUp(const MUID& uidChar)
{
	if (ZGetGame())
	{
		ZCharacter* pCharacter = ZGetGame()->m_CharacterManager.Find(uidChar);
		if (pCharacter) {
			pCharacter->LevelUp();

			char temp[256] = "";
			ZTransMsg(temp, MSG_GAME_LEVEL_UP, 1, pCharacter->GetUserAndClanName());
			ZChatOutput(MCOLOR(ZCOLOR_GAME_INFO), temp);
		}
	}
}

void ZGameClient::OnGameLevelDown(const MUID& uidChar)
{
	if (ZGetGame())
	{
		ZCharacter* pCharacter = ZGetGame()->m_CharacterManager.Find(uidChar);
		if (pCharacter) {
			pCharacter->LevelDown();

			char temp[256] = "";
			ZTransMsg(temp, MSG_GAME_LEVEL_DOWN, 1, pCharacter->GetUserAndClanName());
			ZChatOutput(MCOLOR(ZCOLOR_GAME_INFO), temp);
		}
	}
}

void ZGameClient::OnResponseGameInfo(const MUID& uidStage, void* pGameInfoBlob, void* pRuleInfoBlob, void* pPlayerInfoBlob)
{
	if (ZGetGame() == NULL) return;

	int nGameInfoCount = MGetBlobArrayCount(pGameInfoBlob);
	if (nGameInfoCount > 0) {
		MTD_GameInfo* pGameInfo = (MTD_GameInfo*)MGetBlobArrayElement(pGameInfoBlob, 0);
		ZGetGame()->GetMatch()->SetTeamScore(MMT_RED, pGameInfo->nRedTeamScore);
		ZGetGame()->GetMatch()->SetTeamScore(MMT_BLUE, pGameInfo->nBlueTeamScore);
		ZGetGame()->GetMatch()->SetTeamKills(MMT_RED, pGameInfo->nRedTeamKills);
		ZGetGame()->GetMatch()->SetTeamKills(MMT_BLUE, pGameInfo->nBlueTeamKills);
	}

	int nPlayerCount = MGetBlobArrayCount(pPlayerInfoBlob);

	for (int i = 0; i < nPlayerCount; i++)
	{
		MTD_GameInfoPlayerItem* pPlayerInfo = (MTD_GameInfoPlayerItem*)MGetBlobArrayElement(pPlayerInfoBlob, i);
		ZCharacter* pCharacter = ZGetGame()->m_CharacterManager.Find(pPlayerInfo->uidPlayer);
		if (pCharacter == NULL) continue;

		if (pPlayerInfo->bAlive == true)
		{
			pCharacter->Revival();
		}
		else
		{
			if ((ZGetGame()->GetMatch()->IsTeamPlay()) && (ZGetGame()->GetMatch()->GetRoundState() != MMATCH_ROUNDSTATE_FREE))
			{
				pCharacter->ForceDie();
				pCharacter->SetVisible(false);
			}
		}

		pCharacter->GetStatus().CheckCrc();
		pCharacter->GetStatus().Ref().nKills = pPlayerInfo->nKillCount;
		pCharacter->GetStatus().Ref().nDeaths = pPlayerInfo->nDeathCount;
		pCharacter->GetStatus().MakeCrc();
	}

	int nRuleCount = MGetBlobArrayCount(pRuleInfoBlob);
	if (nRuleCount > 0) {
		MTD_RuleInfo* pRuleInfoHeader = (MTD_RuleInfo*)MGetBlobArrayElement(pRuleInfoBlob, 0);

		ZGetGame()->GetMatch()->OnResponseRuleInfo(pRuleInfoHeader);
	}
}

void ZGameClient::OnObtainWorldItem(const MUID& uidChar, const int nItemUID)
{
	if (ZGetGame() == NULL) return;

	ZCharacter* pCharacter = ZGetGame()->m_CharacterManager.Find(uidChar);
	if (pCharacter)
	{
		ZGetWorldItemManager()->ApplyWorldItem(nItemUID, pCharacter);

		ZWeapon* pWeapon = ZGetGame()->m_WeaponManager.GetWorldItem(nItemUID);

		ZWeaponItemKit* pItemKit = MDynamicCast(ZWeaponItemKit, pWeapon);
		if (pItemKit) { pItemKit->m_bDeath = true; }
	}
}

void ZGameClient::OnSpawnWorldItem(void* pBlob)
{
	if (ZGetGame() == NULL) return;

	int nWorldItemCount = MGetBlobArrayCount(pBlob);

	ZWeaponItemKit* pItemKit = NULL;

	ZMovingWeapon* pMWeapon = NULL;
	ZWorldItem* pWorldItem = NULL;

	for (int i = 0; i < nWorldItemCount; i++)
	{
		MTD_WorldItem* pWorldItemNode = (MTD_WorldItem*)MGetBlobArrayElement(pBlob, i);

		pWorldItem = ZGetWorldItemManager()->AddWorldItem(pWorldItemNode->nUID, pWorldItemNode->nItemID,
			(MTD_WorldItemSubType)pWorldItemNode->nItemSubType,
			rvector((float)pWorldItemNode->x, (float)pWorldItemNode->y, (float)pWorldItemNode->z));

		pMWeapon = ZGetGame()->m_WeaponManager.UpdateWorldItem(pWorldItemNode->nItemID, rvector(pWorldItemNode->x, pWorldItemNode->y, pWorldItemNode->z));

		if (pWorldItem && (pItemKit = MDynamicCast(ZWeaponItemKit, pMWeapon)))
		{
			pItemKit->SetItemUID(pWorldItemNode->nUID);
			pWorldItem->m_bisDraw = false;
		}
	}
}

void ZGameClient::OnRemoveWorldItem(const int nItemUID)
{
	if (ZGetGame() == NULL) return;

	ZGetWorldItemManager()->DeleteWorldItem(nItemUID, true);

	ZWeapon* pWeapon = ZGetGame()->m_WeaponManager.GetWorldItem(nItemUID);
	ZWeaponItemKit* pItemKit = MDynamicCast(ZWeaponItemKit, pWeapon);
	if (pItemKit) { pItemKit->m_bDeath = true; }
}

void ZGameClient::OnNotifyActivatedTrapItemList(void* pBlob)
{
	if (ZGetGame() == NULL) return;

	int numTrap = MGetBlobArrayCount(pBlob);

	rvector pos;
	ZObject* pOwner;
	for (int i = 0; i < numTrap; i++)
	{
		MTD_ActivatedTrap* pTrap = (MTD_ActivatedTrap*)MGetBlobArrayElement(pBlob, i);

		pos.x = (float)pTrap->x;
		pos.y = (float)pTrap->y;
		pos.z = (float)pTrap->z;

		pOwner = ZGetCharacterManager()->Find(pTrap->uidOwner);
		if (pOwner == NULL)
			continue;
		float fTimeActivated = ZGetGame()->GetTime() - pTrap->nTimeElapsed * 0.001f;

		ZGetGame()->m_WeaponManager.AddTrapAlreadyActivated(pos, fTimeActivated, pTrap->nItemID, pOwner);
	}
}

void ZGameClient::OnUserWhisper(char* pszSenderName, char* pszTargetName, char* pszMessage)
{
	char szText[256];
	ZTransMsg(szText, MSG_GAME_WHISPER, 2, pszSenderName, pszMessage);

	if (ZGetGame())
	{
		if ((ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL) && !ZGetGame()->m_pMyCharacter->IsDie())
			ZTransMsg(szText, MSG_GAME_WHISPER, 2, pszSenderName, ". . . . .");
	}

	ZChatOutput(MCOLOR(ZCOLOR_CHAT_WHISPER), szText, ZChat::CL_CURRENT);

	ZGetGameInterface()->GetChat()->SetWhisperLastSender(pszSenderName);

	if ((ZApplication::GetGameInterface()->GetState() == GUNZ_GAME) && (ZGetGame()))
	{
		if (ZGetCombatInterface())
		{
			if (!ZGetConfiguration()->GetViewGameChat())
			{
				ZGetCombatInterface()->ShowChatOutput(true);
			}
		}
	}
}

void ZGameClient::OnChatRoomJoin(char* pszPlayerName, char* pszChatRoomName)
{
	char szText[256];
	ZTransMsg(szText, MSG_LOBBY_WHO_CHAT_ROMM_JOIN, 2, pszChatRoomName, pszPlayerName);
	ZChatOutput(szText, ZChat::CMT_NORMAL, ZChat::CL_CURRENT);
}

void ZGameClient::OnChatRoomLeave(char* pszPlayerName, char* pszChatRoomName)
{
	char szText[256];
	ZTransMsg(szText, MSG_LOBBY_WHO_CHAT_ROOM_EXIT, 2, pszChatRoomName, pszPlayerName);
	ZChatOutput(szText, ZChat::CMT_NORMAL, ZChat::CL_CURRENT);
}

void ZGameClient::OnChatRoomSelectWrite(char* pszChatRoomName)
{
	char szText[256];
	ZTransMsg(szText, MSG_LOBBY_CHAT_ROOM_CHANGE, 1, pszChatRoomName);
	ZChatOutput(szText, ZChat::CMT_NORMAL, ZChat::CL_CURRENT);
}

void ZGameClient::OnChatRoomInvite(char* pszSenderName, char* pszRoomName)
{
	char szLog[256];
	ZTransMsg(szLog, MSG_LOBBY_WHO_INVITATION, 2, pszSenderName, pszRoomName);
	ZChatOutput(szLog, ZChat::CMT_NORMAL, ZChat::CL_CURRENT);

	SetChatRoomInvited(pszRoomName);
}

void ZGameClient::OnChatRoomChat(char* pszChatRoomName, char* pszPlayerName, char* pszChat)
{
	char szText[256];
	ZTransMsg(szText, MRESULT_CHAT_ROOM, 3, pszChatRoomName, pszPlayerName, pszChat);
	ZChatOutput(MCOLOR(ZCOLOR_CHAT_ROOMCHAT), szText, ZChat::CL_CURRENT);
}

void ZGameClient::RequestPrevStageList()
{
	int nStageCursor;
	ZRoomListBox* pRoomList =
		(ZRoomListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_StageList");
	if (!pRoomList) return;

	nStageCursor = pRoomList->GetFirstStageCursor() - NUM_DISPLAY_ROOM;
	if (nStageCursor < 0) nStageCursor = 0;

	ZPostRequestStageList(m_uidPlayer, m_uidChannel, nStageCursor);

	int nPage = (nStageCursor / TRANS_STAGELIST_NODE_COUNT) + 1;
	ZApplication::GetGameInterface()->SetRoomNoLight(nPage);
}

void ZGameClient::RequestNextStageList()
{
	int nStageCursor;
	ZRoomListBox* pRoomList =
		(ZRoomListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_StageList");
	if (!pRoomList) return;

	nStageCursor = pRoomList->GetLastStageCursor() + 1;
	if (nStageCursor > 100) nStageCursor = 100;

	ZPostRequestStageList(m_uidPlayer, m_uidChannel, nStageCursor);

	int nPage = (nStageCursor / TRANS_STAGELIST_NODE_COUNT) + 1;
	ZApplication::GetGameInterface()->SetRoomNoLight(nPage);
}

void ZGameClient::RequestStageList(int nPage)
{
	int nStageCursor;

	nStageCursor = (nPage - 1) * TRANS_STAGELIST_NODE_COUNT;
	if (nStageCursor < 0) nStageCursor = 0;
	else if (nStageCursor > 100) nStageCursor = 100;

	ZPostRequestStageList(m_uidPlayer, m_uidChannel, nStageCursor);
}

void ZGameClient::OnLocalReport119()
{
	ZApplication::GetGameInterface()->Show112Dialog(true);
}

int ZGameClient::ValidateRequestDeleteChar()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	int nCharIndex = ZCharacterSelectView::GetSelectedCharacter();
	if ((nCharIndex < 0) || (nCharIndex >= MAX_CHAR_COUNT)) return ZERR_UNKNOWN;

	ZSelectCharacterInfo* pSelectCharInfo = &ZCharacterSelectView::m_CharInfo[nCharIndex];
	MTD_AccountCharInfo* pAccountCharInfo = &pSelectCharInfo->m_AccountCharInfo;
	MTD_CharInfo* pCharInfo = &pSelectCharInfo->m_CharInfo;

	if (!pSelectCharInfo->m_bLoaded) return ZERR_UNKNOWN;

	if (pCharInfo->szClanName[0] != 0)
		return MSG_CLAN_PLEASE_LEAVE_FROM_CHAR_DELETE;

	for (int i = 0; i < MMCIP_END; i++)
	{
		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(pCharInfo->nEquipedItemDesc[i]);
		if (pItemDesc)
		{
			if (pItemDesc->IsCashItem()) return MSG_CANNOT_DELETE_CHAR_FOR_CASHITEM;
		}
	}

	return ZOK;
}

void ZGameClient::RequestChannelJoin(const MUID& uidChannel)
{
	ZPostChannelRequestJoin(GetPlayerUID(), uidChannel);
}

void ZGameClient::RequestChannelJoin(const MCHANNEL_TYPE nChannelType, char* szChannelName)
{
	ZPostChannelRequestJoinFromChannelName(GetPlayerUID(), (int)nChannelType, szChannelName);
}

void ZGameClient::RequestGameSuicide()
{
	ZGame* pGame = ZGetGameInterface()->GetGame();
	if (!pGame) return;

	ZMyCharacter* pMyCharacter = pGame->m_pMyCharacter;
	if (!pMyCharacter) return;

	if ((!pMyCharacter->IsDie()) && (pGame->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PLAY))
	{
		pMyCharacter->SetLastDamageType(ZD_NONE);

		ZPostRequestSuicide(ZGetGameClient()->GetPlayerUID());
	}
}

void ZGameClient::OnResponseResult(const int nResult)
{
	if (nResult != MOK)
	{
		if (ZApplication::GetGameInterface()->GetState() == GUNZ_GAME)
		{
			ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), ZErrStr(nResult));
		}
		else
		{
			ZApplication::GetGameInterface()->ShowErrorMessage(nResult);
		}
	}
}

void blog(const char* pFormat, ...)
{
	char szBuf[256];

	va_list args;
	va_start(args, pFormat);
	vsprintf(szBuf, pFormat, args);
	va_end(args);

	strcat(szBuf, "\n");

	if (ZApplication::GetGameInterface()->GetState() == GUNZ_LOBBY)
		ZChatOutput(szBuf, ZChat::CMT_SYSTEM, ZChat::CL_LOBBY);
	else if (ZApplication::GetGameInterface()->GetState() == GUNZ_STAGE)
		ZChatOutput(szBuf, ZChat::CMT_SYSTEM, ZChat::CL_STAGE);
}

void ZGameClient::OnResponseCharInfoDetail(void* pBlob)
{
#ifndef _DEBUG
	return;
#endif

	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Characterinfo");
	if (pWidget)
		pWidget->Show();

	int nCount = MGetBlobArrayCount(pBlob);
	if (nCount != 1) return;

	MTD_CharInfo_Detail* pCharInfoDetail = (MTD_CharInfo_Detail*)MGetBlobArrayElement(pBlob, 0);

	blog("^9%s", ZMsg(MSG_CHARINFO_TITLE));
	blog("^9%s : ^1%s^9(%s)", ZMsg(MSG_CHARINFO_NAME),
		pCharInfoDetail->szName,
		ZGetSexStr(MMatchSex(pCharInfoDetail->nSex), true));
	char sztemp[256];
	if (strcmp(pCharInfoDetail->szClanName, "") == 0)
		strcpy(sztemp, "---");
	else
		sprintf(sztemp, "%s(%s)", pCharInfoDetail->szClanName, ZGetClanGradeStr(pCharInfoDetail->nClanGrade));
	blog("^9%s : %s", ZMsg(MSG_CHARINFO_CLAN), sztemp);
	blog("^9%s : %d %s", ZMsg(MSG_CHARINFO_LEVEL), pCharInfoDetail->nLevel, ZMsg(MSG_CHARINFO_LEVELMARKER));
	int nWinPercent = (int)((float)pCharInfoDetail->nKillCount / (float)(pCharInfoDetail->nKillCount + pCharInfoDetail->nDeathCount) * 100.0f);
	blog("^9%s : %d%s/%d%s(%d%%)", ZMsg(MSG_CHARINFO_WINPERCENT),
		pCharInfoDetail->nKillCount,
		ZMsg(MSG_CHARINFO_WIN),
		pCharInfoDetail->nDeathCount,
		ZMsg(MSG_CHARINFO_LOSE),
		nWinPercent);
	ZGetTimeStrFromSec(sztemp, pCharInfoDetail->nConnPlayTimeSec);
	blog("^9%s : %s", ZMsg(MSG_CHARINFO_CONNTIME), sztemp);
	blog("");
}

void ZGameClient::OnNotifyCallVote(const char* pszDiscuss, const char* pszArg)
{
	SetVoteInProgress(true);
	SetCanVote(true);

	char szText[256] = "";
	if (stricmp(pszDiscuss, "joke") == 0) {
		ZTransMsg(szText, MSG_VOTE_START, 1, pszArg);
		ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_CURRENT);
	}
	else if (stricmp(pszDiscuss, "kick") == 0) {
		sprintf(m_szVoteText, ZMsg(MSG_VOTE_KICK), pszArg);
		ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_CURRENT);
	}
}

void ZGameClient::OnNotifyVoteResult(const char* pszDiscuss, int nResult)
{
	if (ZGetGameInterface()->GetCombatInterface() == NULL)
		return;

	ZGetGameInterface()->GetCombatInterface()->GetVoteInterface()->ShowTargetList(false);

	SetVoteInProgress(false);
	SetCanVote(false);

	if (nResult == 0) {
		ZChatOutput(ZMsg(MSG_VOTE_REJECTED), ZChat::CMT_SYSTEM, ZChat::CL_CURRENT);
	}
	else if (nResult == 1) {
		ZChatOutput(ZMsg(MSG_VOTE_PASSED), ZChat::CMT_SYSTEM, ZChat::CL_CURRENT);
	}
}

void ZGameClient::OnVoteAbort(const int nMsgCode)
{
	ZChatOutput(ZMsg(nMsgCode), ZChat::CMT_SYSTEM, ZChat::CL_CURRENT);
}

void ZGameClient::RequestOnLobbyCreated()
{
	ZPostRequestStageList(GetPlayerUID(), GetChannelUID(), 0);
	ZPostRequestChannelPlayerList(GetPlayerUID(), GetChannelUID(), 0);
}

void ZGameClient::RequestOnGameDestroyed()
{
	ZPostRequestMySimpleCharInfo(ZGetGameClient()->GetPlayerUID());

	if ((GetServerMode() == MSM_CLAN) && (GetChannelType() == MCHANNEL_TYPE_CLAN))
	{
		ZPostRequestClanInfo(GetPlayerUID(), m_szChannel);
	}
}

void ZGameClient::OnFollowResponse(const int nMsgID)
{
	ZGetGameInterface()->GetChat()->Clear(ZChat::CL_LOBBY);
	const char* pszMsg = ZErrStr(nMsgID);
	if (0 == pszMsg)
		return;

	ZChatOutput(pszMsg, ZChat::CMT_SYSTEM, ZChat::CL_LOBBY);
}

void ZGameClient::OnClanResponseEmblemURL(unsigned int nCLID, unsigned int nEmblemChecksum, const char* szEmblemURL)
{
	char szFullURL[2048] = "";

	sprintf(szFullURL, "%s%s", Z_LOCALE_EMBLEM_URL, szEmblemURL);

	if (0 == strlen(szEmblemURL)) {
		mlog("Emblem url is null! clanID(%d)\n", nCLID);
		return;
	}

	m_EmblemMgr.ProcessEmblem(nCLID, szFullURL, nEmblemChecksum);
}

void ZGameClient::OnClanEmblemReady(unsigned int nCLID, const char* szURL)
{
	ZGetEmblemInterface()->ReloadClanInfo(nCLID);

	if (ZGetNetRepository()->GetClanInfo()->nCLID == nCLID) {
		ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();
		MPicture* pPicture = (MPicture*)pRes->FindWidget("Lobby_ClanInfoEmblem");
		if (pPicture)
			pPicture->SetBitmap(ZGetEmblemInterface()->GetClanEmblem(nCLID));
	}
}

void ZGameClient::OnExpiredRentItem(void* pBlob)
{
	int nBlobSize = MGetBlobArrayCount(pBlob);

	char szText[1024];
	sprintf(szText, "%s\n", ZMsg(MSG_EXPIRED));

	for (int i = 0; i < nBlobSize; i++)
	{
		unsigned long int* pExpiredItemID = (unsigned long int*)MGetBlobArrayElement(pBlob, i);

		char szItemText[256];

		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(*pExpiredItemID);
		if (pItemDesc)
		{
			sprintf(szItemText, "[%d] %s\n", i + 1, pItemDesc->m_pMItemName->Ref().m_szItemName);
			if ((strlen(szText) + strlen(szItemText)) <= 1022) strcat(szText, szItemText);
		}
	}

	ZApplication::GetGameInterface()->ShowMessage(szText);
}

bool ZGameClient::CreateUPnP(unsigned short nUDPPort)
{
	if (!m_pUPnP)
		m_pUPnP = new UPnP;

	if (m_pUPnP->Create(nUDPPort))
	{
		TRACE("UPnP: Port: %d\n", nUDPPort);
		mlog("%d upnp port forward initialized.\n", nUDPPort);
		return true;
	}
	else
	{
		TRACE("UPnP: Failed to forward port\n");
	}
	return false;
}

bool ZGameClient::DestroyUPnP()
{
	if (m_pUPnP)
	{
		m_pUPnP->Destroy();
		delete m_pUPnP;
	}

	return true;
}

void ZGameClient::OnBroadcastDuelRenewVictories(const char* pszChampionName, const char* pszChannelName, int nRoomno, int nVictories)
{
	char szText[256];
	char szVic[32], szRoomno[32];

	sprintf(szVic, "%d", nVictories);
	sprintf(szRoomno, "%d", nRoomno);

	ZTransMsg(szText, MSG_DUEL_BROADCAST_RENEW_VICTORIES, 4, pszChampionName, pszChannelName, szRoomno, szVic);

	ZChatOutput(szText, ZChat::CMT_BROADCAST);
}

void ZGameClient::OnBroadcastDuelInterruptVictories(const char* pszChampionName, const char* pszInterrupterName, int nVictories)
{
	char szText[256];
	char szVic[32];
	sprintf(szVic, "%d", nVictories);
	ZTransMsg(szText, MSG_DUEL_BROADCAST_INTERRUPT_VICTORIES, 3, pszChampionName, pszInterrupterName, szVic);

	ZChatOutput(szText, ZChat::CMT_BROADCAST);
}

void ZGameClient::ChangeQuestStage()
{
	if ((0 != ZGetGameClient()) &&
		(0 == stricmp(MCHANNEL_RULE_QUEST_STR, ZGetGameClient()->GetChannelRuleName())) &&
		(!ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType())) &&
		ZGetGameClient()->AmIStageMaster())
	{
		MSTAGE_SETTING_NODE StageSetting;

		StageSetting.bAutoTeamBalancing = true;
		StageSetting.bForcedEntryEnabled = true;
		StageSetting.bTeamKillEnabled = false;
		StageSetting.bTeamWinThePoint = false;
		StageSetting.nGameType = MMATCH_GAMETYPE_QUEST;
		StageSetting.nLimitLevel = 0;
		StageSetting.nLimitTime = 20;
		StageSetting.nMapIndex = 0;
		StageSetting.nMaxPlayers = 8;
		StageSetting.nRoundMax = 10;
		memset(StageSetting.szMapName, 0, 32);
		strncpy(StageSetting.szMapName, "Mansion", 7);
		StageSetting.uidStage = ZGetGameClient()->GetStageUID();

		StageSetting.bIsRelayMap = false;
		StageSetting.nRelayMapListCount = 0;
		StageSetting.nRelayMapType = RELAY_MAP_TURN;
		StageSetting.nRelayMapRepeatCount = RELAY_MAP_3REPEAT;

		ZPostStageSetting(ZGetGameClient()->GetUID(), ZGetGameClient()->GetStageUID(), &StageSetting);
	}
}

void ZGameClient::OnRecieveGambleItem(unsigned int nRecvItem, unsigned int nCnt, unsigned int nTime)
{
	char szText[256];
	char szName[256];

	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nRecvItem);
	if (pItemDesc)
	{
		sprintf(szName, "%s (x%d)", pItemDesc->m_pMItemName->Ref().m_szItemName, nCnt);
	}
	MQuestItemDesc* pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc(nRecvItem);
	if (pQuestItemDesc)
	{
		sprintf(szName, "%s (x%d)", pQuestItemDesc->m_szQuestItemName, nCnt);
	}

	if (!pItemDesc && !pQuestItemDesc)
		return;

	ZTransMsg(szText, MSG_RECEIVE_GAMBLE_ITEM, 1, szName);
	ZApplication::GetGameInterface()->ShowMessage(szText);
}

void ZGameClient::OnResponseUpdateStageEquipLook(const MUID& uidPlayer, const int nParts, const int nItemID)
{
	MMatchObjCacheMap::iterator itFind = m_ObjCacheMap.find(uidPlayer);
	if (m_ObjCacheMap.end() == itFind)
	{
		return;
	}

	MMatchObjCache* pObjCache = itFind->second;

	pObjCache->GetCostume()->nEquipedItemID[nParts] = nItemID;
}

void ZGameClient::OnPrepareRun(void)
{
}

bool ZGameClient::IsUDPCommandValidationCheck(int nCommandID)
{
	return !m_UPDCommadHackShield.IsDeniedCommand(nCommandID);
}

void ZGameClient::OnAdminResponseKickPlayer(int nResult)
{
	if (IsAdminGrade(ZGetMyInfo()->GetUGradeID()))
	{
		if (nResult != MOK) {
			const char* strError = ZErrStr(nResult);
			if (strError) {
				ZChatOutput(strError, ZChat::CMT_NORMAL, ZChat::CL_LOBBY, MCOLOR(255, 128, 64));
			}
		}
	}
}

void ZGameClient::OnAdminResponseBlockPlayer(int nResult)
{
	if (IsAdminGrade(ZGetMyInfo()->GetUGradeID()))
	{
		if (nResult != MOK) {
			const char* strError = ZErrStr(nResult);
			if (strError) {
				ZChatOutput(strError, ZChat::CMT_NORMAL, ZChat::CL_LOBBY, MCOLOR(255, 128, 64));
			}
		}
	}
}

void ZGameClient::OnAdminResponseMutePlayer(int nResult)
{
	if (IsAdminGrade(ZGetMyInfo()->GetUGradeID())) {
		if (nResult != MOK) {
			const char* strError = ZErrStr(nResult);
			if (strError) {
				ZChatOutput(strError, ZChat::CMT_NORMAL, ZChat::CL_LOBBY, MCOLOR(255, 128, 64));
			}
		}
	}
	else {
		if (nResult == MOK) {
			const char* strMsg = ZMsg(MSG_CHAT_MUTE_BY_ADMIN);
			if (strMsg) {
				ZChatOutput(strMsg, ZChat::CMT_NORMAL, ZChat::CL_LOBBY);
			}
		}
	}
}