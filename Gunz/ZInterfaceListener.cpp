#include "stdafx.h"

#include "ZApplication.h"
#include "ZInterfaceListener.h"
#include "MWidget.h"
#include "MEdit.h"
#include "MComboBox.h"
#include "ZMapListBox.h"
#include "ZPost.h"
#include "MMatchStage.h"
#include "ZConfiguration.h"
#include "MSlider.h"
#include "MMatchStage.h"
#include "ZCharacterView.h"
#include "ZCharacterViewList.h"
#include "ZCharacterSelectView.h"
#include "ZShop.h"
#include "ZMyItemList.h"
#include "ZMyInfo.h"
#include "ZStageSetting.h"
#include "MChattingFilter.h"
#include "ZRoomListBox.h"
#include "ZPlayerListBox.h"
#include "MDebug.h"
#include "ZChat.h"
#include "ZMsgBox.h"
#include "ZActionKey.h"
#include "ZPlayerSelectListBox.h"
#include "ZChannelListItem.h"
#include "MTabCtrl.h"

#include "ZApplication.h"
#include "ZServerView.h"
#include "ZCharacterView.h"

#include "ZMonsterBookInterface.h"

#include "MMatchGlobal.h"

#include "ZShopEquipInterface.h"
#include "ZShopEquipListbox.h"

#ifdef LOCALE_NHNUSAA
#include "ZNHN_USA_Report.h"
#endif

class MChatInputListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		if (MWidget::IsMsg(szMessage, MEDIT_ENTER_VALUE) == true) {
			const char* szCommand = pWidget->GetText();
			ZChatOutput(szCommand);

			char szChat[512];
			strcpy(szChat, pWidget->GetText());
			if (ZGetGameInterface()->GetChat()->Input(szChat))
			{
			}

			pWidget->SetText("");
			return true;
		}
		else if ((MWidget::IsMsg(szMessage, MEDIT_CHAR_MSG) == true) || (MWidget::IsMsg(szMessage, MEDIT_KEYDOWN_MSG) == true))
		{
			ZGetGameInterface()->GetChat()->FilterWhisperKey(pWidget);
		}

		return false;
	}
};
MChatInputListener	g_ChatInputListener;

class MHotBarButton : public MButton {
protected:
	char	m_szCommandString[256];
protected:
	virtual bool OnDrop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString) {
		m_pIcon = pBitmap;
		AttachToolTip(szString);
		strcpy(m_szCommandString, szItemString);
		return true;
	}

public:
	MHotBarButton(const char* szName = NULL, MWidget* pParent = NULL, MListener* pListener = NULL)
		: MButton(szName, pParent, pListener) {
		strcpy(m_szCommandString, "Command is not assigned");
	}
	virtual bool IsDropable(MWidget* pSender) {
		return true;
	}
	const char* GetCommandString(void) {
		return m_szCommandString;
	}
};

class MHotBarButtonListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		if (MWidget::IsMsg(szMessage, MBTN_CLK_MSG) == true) {
			MHotBarButton* pButton = (MHotBarButton*)pWidget;
			const char* szCommandString = pButton->GetCommandString();

			char szParse[256];
			ZGetGame()->ParseReservedWord(szParse, szCommandString);

			char szErrMsg[256];
			ZChatOutput(MCOLOR(0xFFFFFFFF), szCommandString);
			if (ZGetGameClient()->Post(szErrMsg, 256, szParse) == false) {
				ZChatOutput(MCOLOR(0xFFFFC600), szErrMsg);
			}

			return true;
		}
		return false;
	}
};
MHotBarButtonListener	g_HotBarButtonListener;

class MLoginListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		if (MWidget::IsMsg(szMessage, MBTN_CLK_MSG) == true)
		{
			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

			ZServerView* pServerList = (ZServerView*)pResource->FindWidget("SelectedServer");
			if (!pServerList)
				return false;

			ServerInfo* pServer = pServerList->GetSelectedServer();
			if (pServer)
			{
				if (pServer->nType == 0)
					return false;

				if (!pServer->bIsLive)
					return false;

				MWidget* pWidget = pResource->FindWidget("LoginOK");
				if (pWidget)
					pWidget->Enable(false);

				pWidget = pResource->FindWidget("LoginFrame");
				if (pWidget)
					pWidget->Show(false);

				pWidget = pResource->FindWidget("Login_ConnectingMsg");
				if (pWidget)
					pWidget->Show(true);

				ZGetGameInterface()->m_bLoginTimeout = true;
				ZGetGameInterface()->m_dwLoginTimeout = timeGetTime() + 7000;

				if (pServer->nType == 7)
				{
					MWidget* pAddr = pResource->FindWidget("ServerAddress");
					MWidget* pPort = pResource->FindWidget("ServerPort");

					ZPostConnect(pAddr->GetText(), atoi(pPort->GetText()));
				}
				else
					ZPostConnect(pServer->szAddress, pServer->nPort);

				MLabel* pLabel = (MLabel*)pResource->FindWidget("LoginError");
				if (pLabel)
					pLabel->SetText("");
			}
		}
		return false;
	}
};
MLoginListener	g_LoginListener;

class MLogoutListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		if (MWidget::IsMsg(szMessage, MBTN_CLK_MSG) == true) {
			mlog("MLogoutListener !\n");
			ZPostDisconnect();
			ZGetGameInterface()->SetState(GUNZ_LOGIN);
			return true;
		}
		return false;
	}
};
MLogoutListener	g_LogoutListener;

class MExitListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		if (MWidget::IsMsg(szMessage, MBTN_CLK_MSG) == true) {
			mlog("MExitListener !\n");
			ZApplication::Exit();

			return true;
		}
		return false;
	}
};
MExitListener	g_ExitListener;

class MChannelChatInputListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		if (MWidget::IsMsg(szMessage, MEDIT_ENTER_VALUE) == true) {
			char szChat[512];
			if (strlen(pWidget->GetText()) < 255)
			{
				strcpy(szChat, pWidget->GetText());
				if (ZGetGameInterface()->GetChat()->Input(szChat))
				{
					pWidget->SetText("");
				}
			}
			return true;
		}
		else if ((MWidget::IsMsg(szMessage, MEDIT_CHAR_MSG) == true) || (MWidget::IsMsg(szMessage, MEDIT_KEYDOWN_MSG) == true))
		{
			ZGetGameInterface()->GetChat()->FilterWhisperKey(pWidget);
		}
		return false;
	}
};
MChannelChatInputListener	g_ChannelChatInputListener;

class MStageChatInputListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		if (MWidget::IsMsg(szMessage, MEDIT_ENTER_VALUE) == true) {
			char szChat[512];
			if (strlen(pWidget->GetText()) < 255)
			{
				strcpy(szChat, pWidget->GetText());
				if (ZGetGameInterface()->GetChat()->Input(szChat))
				{
					pWidget->SetText("");
				}
			}
			return true;
		}
		else if ((MWidget::IsMsg(szMessage, MEDIT_CHAR_MSG) == true) || (MWidget::IsMsg(szMessage, MEDIT_KEYDOWN_MSG) == true))
		{
			ZGetGameInterface()->GetChat()->FilterWhisperKey(pWidget);
		}

		return false;
	}
};
MStageChatInputListener	g_StageChatInputListener;

class MGameStartListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		if (MWidget::IsMsg(szMessage, MBTN_CLK_MSG) == true) {
			const MSTAGE_SETTING_NODE* pStageSetting = ZGetGameClient()->GetMatchStageSetting()->GetStageSetting();

			int nPlayerCnt = (int)ZGetGameClient()->GetMatchStageSetting()->m_CharSettingList.size();
			if (nPlayerCnt > pStageSetting->nMaxPlayers&& nPlayerCnt != 0 && pStageSetting->nMaxPlayers != 0 && (ZGetGameTypeManager()->IsQuestDerived(pStageSetting->nGameType)))
			{
				char szText[128] = { 0, };
				sprintf(szText, "%s\n", ZErrStr(MERR_PERSONNEL_TOO_MUCH));
				ZGetGameInterface()->ShowMessage(szText);
				return false;
			}

			if (!ZApplication::GetStageInterface()->GetIsRelayMapRegisterComplete())
			{
				ZGetGameInterface()->ShowMessage(MSG_GAME_RELAYMAP_CONFIRM_BUTTON_PUSH);
				return true;
			}

			if (ZGetGameClient()->GetMatchStageSetting()->GetMapName()[0] != 0)
			{
				ZApplication::GetStageInterface()->ChangeStageEnableReady(true);

				ZPostStageStart(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
			}
			else
			{
				ZGetGameInterface()->ShowMessage("선택하신 맵이 없습니다. 맵을 선택해 주세요.");
			}

			return true;
		}
		return false;
	}
};
MGameStartListener	g_GameStartListener;

class MMapChangeListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		if (MWidget::IsMsg(szMessage, MBTN_CLK_MSG) == true) {
			ZGetGameInterface()->ShowWidget("MapFrame", true, true);

			return true;
		}
		return false;
	}
};
MMapChangeListener	g_MapChangeListener;

class MMapSelectListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		if (MWidget::IsMsg(szMessage, MBTN_CLK_MSG) == true) {
			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
			ZMapListBox* pWidget = (ZMapListBox*)pResource->FindWidget("MapList");
			char szMapName[_MAX_DIR];
			strcpy(szMapName, pWidget->GetSelItemString());
			if (szMapName != NULL) {
				ZApplication::GetStageInterface()->SetMapName(szMapName);
				ZPostStageMap(ZGetGameClient()->GetStageUID(), szMapName);

				if (pWidget->GetParent() != NULL) pWidget->GetParent()->Show(false);
			}

			return true;
		}
		return false;
	}
};
MMapSelectListener	g_MapSelectListener;

class MParentCloseListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		if (MWidget::IsMsg(szMessage, MBTN_CLK_MSG) == true) {
			if (pWidget->GetParent() != NULL) pWidget->GetParent()->Show(false);
			return true;
		}
		return false;
	}
};
MParentCloseListener	g_ParentCloseListener;

class MStageCreateFrameCallerListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		if (MWidget::IsMsg(szMessage, MBTN_CLK_MSG) == true) {
			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
			MWidget* pFindWidget = pResource->FindWidget("StageCreateFrame");
			if (pFindWidget != NULL) pFindWidget->Show(true, true);

			MEdit* pPassEdit = (MEdit*)pResource->FindWidget("StagePassword");
			if (pPassEdit != NULL)
			{
				pPassEdit->SetMaxLength(STAGEPASSWD_LENGTH);
				pPassEdit->SetText("");
			}

			return true;
		}
		return false;
	}
};
MStageCreateFrameCallerListener	g_StageCreateFrameCallerListener;

class MSelectCharacterComboBoxListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if (MWidget::IsMsg(szMessage, MCMBBOX_CHANGED) == true)
		{
			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

			if (ZGetGameInterface()->GetCharacterSelectView() != NULL)
			{
				ZGetGameInterface()->GetCharacterSelectView()->SelectChar(ZCharacterSelectView::GetSelectedCharacter());
			}

			return true;
		}

		return false;
	}
};
MSelectCharacterComboBoxListener	g_SelectCharacterComboBoxListener;

MListener* ZGetChatInputListener(void)
{
	return &g_ChatInputListener;
}

MListener* ZGetLoginListener(void)
{
	return &g_LoginListener;
}

MListener* ZGetLogoutListener(void)
{
	return &g_LogoutListener;
}

MListener* ZGetExitListener(void)
{
	return &g_ExitListener;
}

MListener* ZGetChannelChatInputListener(void)
{
	return &g_ChannelChatInputListener;
}

MListener* ZGetStageChatInputListener(void)
{
	return &g_StageChatInputListener;
}

MListener* ZGetGameStartListener(void)
{
	return &g_GameStartListener;
}

MListener* ZGetMapChangeListener(void)
{
	return &g_MapChangeListener;
}

MListener* ZGetMapSelectListener(void)
{
	return &g_MapSelectListener;
}

MListener* ZGetParentCloseListener(void)
{
	return &g_ParentCloseListener;
}

MListener* ZGetStageCreateFrameCallerListener(void)
{
	return &g_StageCreateFrameCallerListener;
}

MListener* ZGetSelectCharacterComboBoxListener(void)
{
	return &g_SelectCharacterComboBoxListener;
}

BEGIN_IMPLEMENT_LISTENER(ZGetMapListListener, MLB_ITEM_DBLCLK)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
ZMapListBox* pMapList = (ZMapListBox*)pResource->FindWidget("MapList");
const char* pszSelItemString = pMapList->GetSelItemString();
if (pszSelItemString) {
	char szMapName[_MAX_DIR];
	sprintf(szMapName, pszSelItemString);
	ZApplication::GetStageInterface()->SetMapName(szMapName);
	ZPostStageMap(ZGetGameClient()->GetStageUID(), szMapName);
	if (pWidget->GetParent() != NULL) pWidget->GetParent()->GetParent()->Show(false);
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageListFrameCallerListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

ZGetGameClient()->StartStageList();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageCreateBtnListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pNameWidget = pResource->FindWidget("StageName");
if (pNameWidget == NULL) return true;
char szStageName[128], szStagePassword[128];
bool bPrivate = false;
strcpy(szStageName, pNameWidget->GetText());

MEdit* pPassEdit = (MEdit*)pResource->FindWidget("StagePassword");
if (pPassEdit)
{
	if ((strlen(pPassEdit->GetText()) > 0) && (strlen(pPassEdit->GetText()) <= STAGEPASSWD_LENGTH))
		bPrivate = true;
	else
		bPrivate = false;

	if (bPrivate == true)
	{
		strcpy(szStagePassword, pPassEdit->GetText());
	}
	else
	{
		memset(szStagePassword, 0, sizeof(szStagePassword));
	}
}

if (pWidget->GetParent() != NULL) pWidget->GetParent()->Show(false);
ZApplication::GetStageInterface()->ChangeStageButtons(false, true, false);

MSTAGE_SETTING_NODE setting;
setting.uidStage = MUID(0, 0);
memset(setting.szMapName, 0, sizeof(setting.szMapName));
setting.nGameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
setting.nRoundMax = 10;
setting.nLimitTime = 10;
setting.nMaxPlayers = 8;
setting.bIsRelayMap = false;

ZApplication::GetStageInterface()->ChangeStageGameSetting(&setting);

if (!MGetChattingFilter()->IsValidStr(szStageName, 1))
{
	char szMsg[256];
	ZTransMsg(szMsg, MSG_WRONG_WORD_NAME, 1, MGetChattingFilter()->GetLastFilteredStr());
	ZGetGameInterface()->ShowMessage(szMsg);
}
else
{
	ZGetGameInterface()->EnableLobbyInterface(false);
	ZPostStageCreate(ZGetGameClient()->GetPlayerUID(), szStageName, bPrivate, szStagePassword);
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetPrivateStageJoinBtnListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
ZRoomListBox* pRoomListBox = (ZRoomListBox*)pResource->FindWidget("Lobby_StageList");
if (pRoomListBox)
{
	pRoomListBox->RequestSelPrivateStageJoin();
}

MWidget* pPrivateStageJoinFrame = pResource->FindWidget("PrivateStageJoinFrame");
if (pPrivateStageJoinFrame)
{
	pPrivateStageJoinFrame->Show(false);
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetChannelListFrameCallerListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pFindWidget = pResource->FindWidget("ChannelListFrame");
if (pFindWidget != NULL) pFindWidget->Show(true, true);

MButton* pButton = (MButton*)pResource->FindWidget("MyClanChannel");
if (pButton)
pButton->Enable(ZGetMyInfo()->IsClanJoined());

pButton = (MButton*)pResource->FindWidget("ChannelList_DuelTournament");
if (pButton)
pButton->Show(ZGetGameClient()->IsEnabledDuelTournament());

MCHANNEL_TYPE nCurrentChannelType = ZGetGameClient()->GetChannelType();
ZGetGameInterface()->InitChannelFrame(nCurrentChannelType);
ZGetGameClient()->StartChannelList(nCurrentChannelType);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetChannelListJoinButtonListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MListBox* pChannelList = (MListBox*)pResource->FindWidget("ChannelList");
ZChannelListItem* pItem = (ZChannelListItem*)pChannelList->GetSelItem();
if (pItem) {
	ZGetGameClient()->RequestChannelJoin(pItem->GetUID());
}
if (pWidget->GetParent() != NULL) pWidget->GetParent()->Show(false);
ZGetGameClient()->StopChannelList();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetChannelListCloseButtonListener, MBTN_CLK_MSG)
if (pWidget->GetParent() != NULL) pWidget->GetParent()->Show(false);
ZGetGameClient()->StopChannelList();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetChannelListListener, MLB_ITEM_DBLCLK)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MListBox* pChannelList = (MListBox*)pResource->FindWidget("ChannelList");
ZChannelListItem* pItem = (ZChannelListItem*)pChannelList->GetSelItem();
if (pItem) {
	ZGetGameClient()->RequestChannelJoin(pItem->GetUID());
}
if (pWidget->GetParent() != NULL) pWidget->GetParent()->Show(false);
ZGetGameClient()->StopChannelList();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageJoinListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
ZRoomListBox* pRoomListBox = (ZRoomListBox*)pResource->FindWidget("Lobby_StageList");
if (pRoomListBox)
{
	pRoomListBox->RequestSelStageJoin();
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageSettingCallerListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("StageSettingFrame");
if (pWidget != NULL)
pWidget->Show(true, true);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageSettingStageTypeListener, MCMBBOX_CHANGED)
{
	ZStageSetting::InitStageSettingGameFromGameType();
	ZStageSetting::PostDataToServer();
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageTeamRedListener, MBTN_CLK_MSG)
MButton* pButton = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("StageTeamRed");
if (pButton && !pButton->GetCheck()) pButton->SetCheck(true);
pButton = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("StageTeamBlue");
if (pButton) pButton->SetCheck(false);
ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_RED);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageTeamBlueListener, MBTN_CLK_MSG)
MButton* pButton = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("StageTeamBlue");
if (pButton && !pButton->GetCheck()) pButton->SetCheck(true);
pButton = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("StageTeamRed");
if (pButton) pButton->SetCheck(false);
ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_BLUE);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageReadyListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

bool bReady = false;
MButton* pReadyBtn = (MButton*)pResource->FindWidget("StageReady");
if (pReadyBtn) bReady = pReadyBtn->GetCheck();

MMatchObjectStageState nStageState;
if (bReady)
nStageState = MOSS_READY;
else
nStageState = MOSS_NONREADY;

ZPostStageState(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), nStageState);
ZApplication::GetStageInterface()->ChangeStageEnableReady(bReady);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageObserverBtnListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MButton* pObserverBtn = (MButton*)pResource->FindWidget("StageObserverBtn");
if (pObserverBtn)
{
	if (pObserverBtn->GetCheck())
		ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_SPECTATOR);
	else
	{
		MButton* pBlueBtn = (MButton*)pResource->FindWidget("StageTeamBlue");

		if (ZGetGameInterface()->m_bTeamPlay)
		{
			if (pBlueBtn->GetCheck())
				ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_BLUE);
			else
				ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_RED);
		}
		else
		{
			ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_ALL);
		}
	}
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageSettingChangedComboboxListener, MCMBBOX_CHANGED)
ZStageSetting::PostDataToServer();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageSettingApplyBtnListener, MBTN_CLK_MSG)
ZStageSetting::ApplyStageSettingDialog();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyListener, MBTN_CLK_MSG)
ZPostStageLeave(ZGetGameClient()->GetPlayerUID());
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLoginStateButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->SetState(GUNZ_LOGIN);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetGreeterStateButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->SetState(GUNZ_GREETER);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetBattleExitButtonListener, MBTN_CLK_MSG)
if (!ZGetGameClient()->IsLadderGame())
{
	if (pWidget->GetParent() != NULL) pWidget->GetParent()->Show(false);
	ZGetGameInterface()->ReserveLeaveBattle();
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageExitButtonListener, MBTN_CLK_MSG)
if (pWidget->GetParent() != NULL) pWidget->GetParent()->Show(false);
ZGetGameInterface()->ReserveLeaveStage();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCombatMenuCloseButtonListener, MBTN_CLK_MSG)
if (pWidget->GetParent() != NULL) pWidget->GetParent()->Show(false);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetPreviousStateButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->SetState(GUNZ_PREVIOUS);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetShopCallerButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->ShowShopDialog();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetShopCloseButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->ShowShopDialog(false);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentCallerButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->ShowEquipmentDialog();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentCloseButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->ShowEquipmentDialog(false);

{
	int nState = ZGetGameInterface()->GetState();

	if (nState == GUNZ_LOBBY)
	{
		ZCharacterViewList* pVLL = ZGetCharacterViewList(GUNZ_LOBBY);
		if (pVLL)
			pVLL->ChangeCharacterInfo();
	}
	else if (nState == GUNZ_STAGE)
	{
		ZCharacterViewList* pVLS = ZGetCharacterViewList(GUNZ_STAGE);
		if (pVLS)
			pVLS->ChangeCharacterInfo();
	}
}

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCharSelectionCallerButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->ChangeToCharSelection();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetQuickJoinButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->RequestQuickJoin();
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetLobbyCharInfoCallerButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->ShowErrorMessage(MERR_NOT_SUPPORT);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetBuyButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnBuyButton();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSellButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnSellButton();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCountableItemTradeDlgCloseListener, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->GetItemCountDlg()->Close();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCountableItemTradeDlgOkButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->GetItemCountDlg()->OnDlgDone();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCountableItemTradeDlgCountUpButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->GetItemCountDlg()->AddCount(1);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCountableItemTradeDlgCountDnButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->GetItemCountDlg()->AddCount(-1);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCountableItemTradeDlgCountChangeListener, MEDIT_TEXT_CHANGED)
ZGetGameInterface()->GetShopEquipInterface()->GetItemCountDlg()->OnEditBoxChanged();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSellCashItemConfirmDlgOkButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->GetSellCashItemConfirmDlg()->OnOkButton();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSellCashItemConfirmDlgCancelListener, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->GetSellCashItemConfirmDlg()->Close();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetEquipButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->Equip();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentSearchButtonListener, MBTN_CLK_MSG)
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSendAccountItemButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnSendAccountButton();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetBringAccountItemButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnBringAccountButton();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetShopCachRechargeButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->ShowErrorMessage(MERR_NOT_SUPPORT);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetShopSearchCallerButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->ShowErrorMessage(MERR_NOT_SUPPORT);
END_IMPLEMENT_LISTENER()

void PostMapname()
{
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MComboBox* pMapCombo = (MComboBox*)pResource->FindWidget("MapSelection");
	const char* pszSelItemString = pMapCombo->GetSelItemString();

	if (pszSelItemString)
	{
		char szMapName[_MAX_DIR];
		sprintf(szMapName, pszSelItemString);
		ZApplication::GetStageInterface()->SetMapName(szMapName);
		ZPostStageMap(ZGetGameClient()->GetStageUID(), szMapName);

		ZApplication::GetStageInterface()->SetIsRelayMapRegisterComplete(true);

		if (strcmp(MMATCH_MAPNAME_RELAYMAP, pszSelItemString) == 0)
		{
			void* pRelayMapListBlob = MMakeBlobArray(sizeof(MTD_RelayMap), 1);
			MTD_RelayMap* pRelayMapList = (MTD_RelayMap*)MGetBlobArrayElement(pRelayMapListBlob, 0);
			pRelayMapList->nMapID = MGetMapDescMgr()->GetMapID(MMATCH_MAP_MANSION);

			ZPostStageRelayMapInfoUpdate(ZGetGameClient()->GetStageUID(), RELAY_MAP_TURN, RELAY_MAP_3REPEAT, pRelayMapListBlob);
			MEraseBlobArray(pRelayMapListBlob);
		}
	}
}

BEGIN_IMPLEMENT_LISTENER(ZGetMapComboListener, MCMBBOX_CHANGED)
PostMapname();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSelectMapPrevButtonListener, MBTN_CLK_MSG)

ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("MapSelection");

if (pComboBox)
{
	pComboBox->SetPrevSel();
	PostMapname();
}

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSelectMapNextButtonListener, MBTN_CLK_MSG)

ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("MapSelection");

if (pComboBox)
{
	pComboBox->SetNextSel();
	PostMapname();
}

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSelectCameraLeftButtonListener, MBTN_CLK_MSG)

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSelectCameraRightButtonListener, MBTN_CLK_MSG)

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCreateCharacterLeftButtonListener, MBTN_CLK_MSG)
if (ZGetGameInterface()->GetCharacterSelectView())
ZGetGameInterface()->GetCharacterSelectView()->CharacterLeft();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCreateCharacterRightButtonListener, MBTN_CLK_MSG)
if (ZGetGameInterface()->GetCharacterSelectView())
ZGetGameInterface()->GetCharacterSelectView()->CharacterRight();
END_IMPLEMENT_LISTENER()

static DWORD g_dwClockCharSelBtn = 0;
void CharacterSelect(int nNum)
{
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

	if ((ZCharacterSelectView::GetSelectedCharacter() == nNum) && ((timeGetTime() - g_dwClockCharSelBtn) <= 300))
	{
		ZGetGameInterface()->OnCharSelect();

		return;
	}

	g_dwClockCharSelBtn = timeGetTime();

	ZGetGameInterface()->ChangeSelectedChar(nNum);
}

BEGIN_IMPLEMENT_LISTENER(ZGetSelectCharacterButtonListener0, MBTN_CLK_MSG)
CharacterSelect(0);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSelectCharacterButtonListener1, MBTN_CLK_MSG)
CharacterSelect(1);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSelectCharacterButtonListener2, MBTN_CLK_MSG)
CharacterSelect(2);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSelectCharacterButtonListener3, MBTN_CLK_MSG)
CharacterSelect(3);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSelectCharacterButtonListener, MBTN_CLK_MSG)
if (ZCharacterSelectView::GetNumOfCharacter())
{
	if (ZGetGameInterface()->GetCharacterSelectView() != NULL)
	{
		ZGetGameInterface()->OnCharSelect();

		MWidget* pWidget = ZGetGameInterface()->GetIDLResource()->FindWidget("CS_SelectCharDefKey");
		pWidget->Enable(false);
	}
}
else
{
	ZGetGameInterface()->ShowMessage("해당 슬롯에 캐릭터가 없습니다.");
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetShowCreateCharacterButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->SetState(GUNZ_CHARCREATION);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetDeleteCharacterButtonListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

char szName[256];
sprintf(szName, "CharSel_Name%d", ZCharacterSelectView::GetSelectedCharacter());
MLabel* pLabel = (MLabel*)pResource->FindWidget(szName);

if (ZCharacterSelectView::GetNumOfCharacter())
{
	int ret = ZGetGameClient()->ValidateRequestDeleteChar();

	if (ret != ZOK)
	{
		ZGetGameInterface()->ShowMessage(ret);

		return true;
	}

	MLabel* pCharNameLabel = (MLabel*)pResource->FindWidget("CS_DeleteCharLabel");
	if (pCharNameLabel)
		pCharNameLabel->SetText(pLabel->GetText());

	MEdit* pYesEdit = (MEdit*)pResource->FindWidget("CS_DeleteCharNameEdit");
	if (pYesEdit)
		pYesEdit->SetText("");

	MWidget* pWidget = pResource->FindWidget("CS_ConfirmDeleteChar");
	if (pWidget)
	{
		pWidget->Show(true, true);
		pWidget->SetFocus();
	}
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetConfirmDeleteCharacterButtonListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

char szName[256];
sprintf(szName, "CharSel_Name%d", ZCharacterSelectView::GetSelectedCharacter());
MLabel* pLabel = (MLabel*)pResource->FindWidget(szName);

if (ZCharacterSelectView::GetNumOfCharacter())
{
	MEdit* pYesEdit = (MEdit*)pResource->FindWidget("CS_DeleteCharNameEdit");
	if (pYesEdit)
	{
		if ((!stricmp(pYesEdit->GetText(), ZMsg(MSG_MENUITEM_YES))) && (ZCharacterSelectView::GetSelectedCharacter() >= 0))
		{
			if ((ZCharacterSelectView::m_CharInfo[ZCharacterSelectView::GetSelectedCharacter()].m_bLoaded) &&
				(ZCharacterSelectView::m_CharInfo[ZCharacterSelectView::GetSelectedCharacter()].m_CharInfo.szClanName[0] == 0))
			{
				ZPostDeleteMyChar(ZGetGameClient()->GetPlayerUID(), ZCharacterSelectView::GetSelectedCharacter(), (char*)pLabel->GetText());
			}
			else
				ZGetGameInterface()->ShowMessage(MSG_CLAN_PLEASE_LEAVE_FROM_CHAR_DELETE);
		}
		else
			ZGetGameInterface()->ShowMessage(MSG_CHARDELETE_ERROR);
	}

	MWidget* pWidget = pResource->FindWidget("CS_ConfirmDeleteChar");
	if (pWidget)
		pWidget->Show(false);
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCloseConfirmDeleteCharButtonListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("CS_ConfirmDeleteChar");
if (pWidget)
{
	pWidget->Show(false);
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCreateCharacterButtonListener, MBTN_CLK_MSG)
int nEmptySlotIndex = -1;
for (int i = 0; i < 4; i++)
{
	if (ZGetGameInterface()->GetCharacterSelectView()->IsEmpty(i))
	{
		nEmptySlotIndex = i;
		break;
	}
}

if (nEmptySlotIndex >= 0)
{
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MEdit* pEdit = (MEdit*)pResource->FindWidget("CC_CharName");
	MComboBox* pSexCB, * pHairCB, * pFaceCB, * pCostumeCB;
	pSexCB = (MComboBox*)pResource->FindWidget("CC_Sex");
	pHairCB = (MComboBox*)pResource->FindWidget("CC_Hair");
	pFaceCB = (MComboBox*)pResource->FindWidget("CC_Face");
	pCostumeCB = (MComboBox*)pResource->FindWidget("CC_Costume");

	if ((pSexCB == NULL) || (pHairCB == NULL) || (pFaceCB == NULL) || (pCostumeCB == NULL) && (pEdit == NULL))
		return true;

	int nNameLen = (int)strlen(pEdit->GetText());

	if (nNameLen <= 0)
	{
		ZGetGameInterface()->ShowErrorMessage(MERR_PLZ_INPUT_CHARNAME);
		return true;
	}
	else if (nNameLen < MIN_CHARNAME)
	{
		ZGetGameInterface()->ShowErrorMessage(MERR_TOO_SHORT_NAME);
		return true;
	}
	else if (nNameLen > MAX_CHARNAME)
	{
		ZGetGameInterface()->ShowErrorMessage(MERR_TOO_LONG_NAME);
		return true;
	}

	bool bIsAbuse = MGetChattingFilter()->IsValidStr(pEdit->GetText(), 2, true);

	if (!bIsAbuse)
	{
		char szMsg[256];
		ZTransMsg(szMsg, MSG_WRONG_WORD_NAME, 1, MGetChattingFilter()->GetLastFilteredStr());
		ZGetGameInterface()->ShowMessage(szMsg, NULL, MSG_WRONG_WORD_NAME);

		return true;
	}

	ZPostCreateMyChar(ZGetGameClient()->GetPlayerUID(), nEmptySlotIndex, (char*)pEdit->GetText(), pSexCB->GetSelIndex(),
		pHairCB->GetSelIndex(), pFaceCB->GetSelIndex(), pCostumeCB->GetSelIndex());
}
END_IMPLEMENT_LISTENER()

void SetCharacterInfoGroup(int n)
{
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MButton* pBtn = (MButton*)pResource->FindWidget("ShowChar_infoGroup");
	if (pBtn)pBtn->SetCheck(n == 0);
	pBtn = (MButton*)pResource->FindWidget("ShowEquip_InfoGroup");
	if (pBtn)pBtn->SetCheck(n == 1);

	MWidget* pFrame = (MFrame*)pResource->FindWidget("Char_infoGroup");
	if (pFrame) pFrame->Show(n == 0);
	pFrame = (MFrame*)pResource->FindWidget("Equip_InfoGroup");
	if (pFrame) pFrame->Show(n == 1);
}

BEGIN_IMPLEMENT_LISTENER(ZGetShowCharInfoGroupListener, MBTN_CLK_MSG)
SetCharacterInfoGroup(0);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetShowEquipInfoGroupListener, MBTN_CLK_MSG)
SetCharacterInfoGroup(1);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCancelCreateCharacterButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->SetState(GUNZ_CHARSELECTION);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZChangeCreateCharInfoListener, MCMBBOX_CHANGED)
if (ZGetGameInterface()->GetCharacterSelectView() != NULL)
{
	ZGetGameInterface()->GetCharacterSelectView()->OnChangedCharCostume();
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageForcedEntryToGameListener, MBTN_CLK_MSG)
if (ZGetGameClient()->GetMatchStageSetting()->GetMapName()[0] != 0)
{
	ZApplication::GetStageInterface()->ChangeStageEnableReady(true);

	ZPostRequestForcedEntry(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetAllEquipmentListCallerButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->SelectShopTab(0);
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetMyAllEquipmentListCallerButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->SelectShopTab(1);
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetCashEquipmentListCallerButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->SelectShopTab(2);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentCharacterTabButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->SelectEquipmentTab(0);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentAccountTabButtonListener, MBTN_CLK_MSG)
#define MIN_ACCOUNT_ITEM_REQUEST_TIME		2000

ZGetGameInterface()->GetShopEquipInterface()->SelectEquipmentTab(1);

ZPostRequestAccountItemList(ZGetGameClient()->GetPlayerUID());
END_IMPLEMENT_LISTENER()

class ZLevelConfirmListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		if (MWidget::IsMsg(szMessage, MMSGBOX_YES) == true) {
		}
		pWidget->Show(false);
		return false;
	}
} g_LevelConfirmListener;

MListener* ZGetLevelConfirmListenter()
{
	return &g_LevelConfirmListener;
}

BEGIN_IMPLEMENT_LISTENER(ZGetPlayerListPrevListener, MBTN_CLK_MSG)
{
	ZPlayerListBox* pWidget = (ZPlayerListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerList");
	if (pWidget->GetMode() == ZPlayerListBox::PLAYERLISTMODE_CHANNEL_FRIEND) {
		int iStart = pWidget->GetStartItem();
		if (iStart > 0)
			pWidget->SetStartItem(iStart - 1);
		return true;
	}

	int nPage = pWidget->m_nPage;

	nPage--;

	if (nPage < 0) {
		return false;
	}

	ZPostRequestChannelPlayerList(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetChannelUID(), nPage);
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetPlayerListNextListener, MBTN_CLK_MSG)
{
	ZPlayerListBox* pWidget = (ZPlayerListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerList");
	if (pWidget->GetMode() == ZPlayerListBox::PLAYERLISTMODE_CHANNEL_FRIEND) {
		int iStart = pWidget->GetStartItem();
		pWidget->SetStartItem(iStart + 1);
		return true;
	}

	int nMaxPage = 0;
	if (pWidget->m_nTotalPlayerCount)
		nMaxPage = pWidget->m_nTotalPlayerCount / 8;

	int nPage = pWidget->m_nPage;

	nPage++;

	if (nPage > nMaxPage) {
		return false;
	}

	ZPostRequestChannelPlayerList(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetChannelUID(), nPage);
}

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStagePlayerListPrevListener, MBTN_CLK_MSG)
{
	ZPlayerListBox* pWidget = (ZPlayerListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("StagePlayerList_");
	if (pWidget->GetMode() == ZPlayerListBox::PLAYERLISTMODE_STAGE_FRIEND) {
		int iStart = pWidget->GetStartItem();
		if (iStart > 0)
			pWidget->SetStartItem(iStart - 1);
		return true;
	}

	pWidget->SetStartItem(pWidget->GetStartItem() - 1);
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStagePlayerListNextListener, MBTN_CLK_MSG)
{
	ZPlayerListBox* pWidget = (ZPlayerListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("StagePlayerList_");
	if (pWidget->GetMode() == ZPlayerListBox::PLAYERLISTMODE_STAGE_FRIEND) {
		int iStart = pWidget->GetStartItem();
		pWidget->SetStartItem(iStart + 1);
		return true;
	}

	pWidget->SetStartItem(pWidget->GetStartItem() + 1);
}

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetRoomListListener, MLIST_VALUE_CHANGED)
ZRoomListBox* pWidget = (ZRoomListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Lobby_StageList");
pWidget->SetPage();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyPrevRoomListButtonListener, MBTN_CLK_MSG)
ZGetGameClient()->RequestPrevStageList();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyNextRoomListPrevButtonListener, MBTN_CLK_MSG)
ZGetGameClient()->RequestNextStageList();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyNextRoomNoButtonListener, MBTN_CLK_MSG)
MButton* pButton = (MButton*)pWidget;
int nIndexInGroup = pButton->GetIndexInGroup();
nIndexInGroup++;
ZGetGameClient()->RequestStageList(nIndexInGroup);
ZGetGameInterface()->SetRoomNoLight(nIndexInGroup);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentShopCallerButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->ShowEquipmentDialog(false);
ZGetGameInterface()->ShowShopDialog(true);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetShopEquipmentCallerButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->ShowShopDialog(false);
ZGetGameInterface()->ShowEquipmentDialog(true);
END_IMPLEMENT_LISTENER()

class ZMapListListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if (MWidget::IsMsg(szMessage, MLB_ITEM_SEL) == true)
		{
			MListBox* pList = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("MapList");
			if (pList != NULL)
			{
				MComboBox* pCombo = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("MapSelection");
				if (pCombo != NULL)
				{
					int si = pList->GetSelIndex();
					pCombo->SetSelIndex(si);
					PostMapname();
				}
			}

			return true;
		}
		if (MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK) == true)
		{
			MListBox* pList = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("MapList");
			if (pList != NULL)
			{
				pList->Show(FALSE);
			}
			return true;
		}
		return false;
	}
} g_MapListListener;

MListener* ZGetStageMapListSelectionListener()
{
	return &g_MapListListener;
}

BEGIN_IMPLEMENT_LISTENER(ZGetStageMapListCallerListener, MBTN_CLK_MSG)
MListBox* pList = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("MapList");
pList->Show(TRUE);
END_IMPLEMENT_LISTENER();

void ZReport112FromListener()
{
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

	MWidget* pWidget = pResource->FindWidget("112Confirm");
	if (!pWidget)
		return;

	MComboBox* pCombo1 = (MComboBox*)pResource->FindWidget("112_ConfirmID");
	MComboBox* pCombo2 = (MComboBox*)pResource->FindWidget("112_ConfirmReason");

	if (!pCombo1 || !pCombo2)
		return;

	if ((pCombo2->GetSelIndex() < 0) || (pCombo2->GetSelIndex() < 1))
		return;

	__time64_t long_time;
	_time64(&long_time);
	const struct tm* pLocalTime = _localtime64(&long_time);

	char szBuff[256];
	sprintf(szBuff, "%s\n%s\n%03d:%s\n%04d-%02d-%02d %02d:%02d:%02d\n", ZGetMyInfo()->GetCharName(), pCombo1->GetSelItemString(),
		100 + pCombo2->GetSelIndex(), pCombo2->GetSelItemString(),
		pLocalTime->tm_year + 1900, pLocalTime->tm_mon + 1, pLocalTime->tm_mday,
		pLocalTime->tm_hour, pLocalTime->tm_min, pLocalTime->tm_sec);

	ZGetGameInterface()->GetChat()->Report112(szBuff);

	pWidget->Show(false);
}

BEGIN_IMPLEMENT_LISTENER(ZGet112ConfirmEditListener, MEDIT_ENTER_VALUE)
ZReport112FromListener();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGet112ConfirmOKButtonListener, MBTN_CLK_MSG)
ZReport112FromListener();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGet112ConfirmCancelButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->Show112Dialog(false);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanSponsorAgreementConfirm_OKButtonListener, MBTN_CLK_MSG)
ZGetGameClient()->AnswerSponsorAgreement(true);

ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("ClanSponsorAgreementConfirm");
if (pWidget != NULL)
{
	pWidget->Show(false);
}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanSponsorAgreementConfirm_CancelButtonListener, MBTN_CLK_MSG)
ZGetGameClient()->AnswerSponsorAgreement(false);

ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("ClanSponsorAgreementConfirm");
if (pWidget != NULL)
{
	pWidget->Show(false);
}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanSponsorAgreementWait_CancelButtonListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("ClanSponsorAgreementWait");
if (pWidget != NULL)
{
	pWidget->Show(false);
}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanJoinerAgreementConfirm_OKButtonListener, MBTN_CLK_MSG)
ZGetGameClient()->AnswerJoinerAgreement(true);

ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("ClanJoinerAgreementConfirm");
if (pWidget != NULL)
{
	pWidget->Show(false);
}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanJoinerAgreementConfirm_CancelButtonListener, MBTN_CLK_MSG)
ZGetGameClient()->AnswerJoinerAgreement(false);

ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("ClanJoinerAgreementConfirm");
if (pWidget != NULL)
{
	pWidget->Show(false);
}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanJoinerAgreementWait_CancelButtonListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("ClanJoinerAgreementWait");
if (pWidget != NULL)
{
	pWidget->Show(false);
}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyPlayerListTabClanCreateButtonListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("ClanCreateDialog");
if (pWidget != NULL)
{
	pWidget->Show(true, true);

	unsigned long int nPlaceFilter = 0;
	SetBitSet(nPlaceFilter, MMP_LOBBY);

	ZPostRequestChannelAllPlayerList(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetChannelUID(), nPlaceFilter,
		MCP_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST_NONCLAN);
}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanCreateDialogOk, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("ClanCreateDialog");
if (pWidget != NULL)
{
	pWidget->Show(false);

	ZPlayerSelectListBox* pPlayerList = (ZPlayerSelectListBox*)pResource->FindWidget("ClanSponsorSelect");
	if (pPlayerList)
	{
		char szSponsors[CLAN_SPONSORS_COUNT][MATCHOBJECT_NAME_LENGTH];
		char* ppSponsors[CLAN_SPONSORS_COUNT];
		int nCount = 0;
		for (int i = 0; i < pPlayerList->GetCount(); i++)
		{
			MListItem* pItem = pPlayerList->Get(i);
			if (pItem->m_bSelected) {
				if (nCount >= CLAN_SPONSORS_COUNT) break;
				strcpy(szSponsors[nCount], pItem->GetString());
				ppSponsors[nCount] = szSponsors[nCount];
				nCount++;
			}
		}
		if (nCount == CLAN_SPONSORS_COUNT)
		{
			MEdit* pEditClanName = (MEdit*)pResource->FindWidget("ClanCreate_ClanName");
			if (!pEditClanName)
				return true;

			int nNameLen = (int)strlen(pEditClanName->GetText());

			if (nNameLen <= 0)
			{
				ZGetGameInterface()->ShowErrorMessage(MERR_PLZ_INPUT_CHARNAME);
				return true;
			}
			else if (nNameLen < MIN_CLANNAME)
			{
				ZGetGameInterface()->ShowErrorMessage(MERR_TOO_SHORT_NAME);
				return true;
			}
			else if (nNameLen > MAX_CLANNAME)
			{
				ZGetGameInterface()->ShowErrorMessage(MERR_TOO_LONG_NAME);
				return true;
			}

			if (!MGetChattingFilter()->IsValidStr(pEditClanName->GetText(), 2, true))
			{
				char szMsg[256];
				ZTransMsg(szMsg, MSG_WRONG_WORD_NAME, 1, MGetChattingFilter()->GetLastFilteredStr());
				ZGetGameInterface()->ShowMessage(szMsg, NULL, MSG_WRONG_WORD_NAME);
			}
			else if (pEditClanName)
			{
				char szClanName[CLAN_NAME_LENGTH] = { 0, };
				strcpy(szClanName, pEditClanName->GetText());
				ZGetGameClient()->RequestCreateClan(szClanName, ppSponsors);
			}
		}
		else
		{
			char szMsgBox[256];
			char szArg[20];
			_itoa(CLAN_SPONSORS_COUNT, szArg, 10);

			ZTransMsg(szMsgBox, MSG_CLAN_CREATE_NEED_SOME_SPONSOR, 1, szArg);
			ZGetGameInterface()->ShowMessage(szMsgBox, NULL, MSG_CLAN_CREATE_NEED_SOME_SPONSOR);
		}
	}
}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanCreateDialogClose, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("ClanCreateDialog");
if (pWidget != NULL)
{
	pWidget->Show(false);
}
END_IMPLEMENT_LISTENER();

class ZLanguageChangeConfirmListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		pWidget->Show(false);
		if (MWidget::IsMsg(szMessage, MMSGBOX_YES) == true) {
			ZGetGameInterface()->ReserveResetApp(true);
		}
		return false;
	}
} g_LanguageChangeConfirmListener;

MListener* ZGetLanguageChangeConfirmListenter()
{
	return &g_LanguageChangeConfirmListener;
}

class ZClanCloseConfirmListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		if (MWidget::IsMsg(szMessage, MMSGBOX_YES) == true) {
			char szClanName[256];
			strcpy(szClanName, ZGetMyInfo()->GetClanName());
			ZPostRequestCloseClan(ZGetGameClient()->GetPlayerUID(), szClanName);
		}
		else {
		}
		pWidget->Show(false);
		return false;
	}
} g_ClanCloseConfirmListener;

MListener* ZGetClanCloseConfirmListenter()
{
	return &g_ClanCloseConfirmListener;
}

class ZClanLeaveConfirmListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		if (MWidget::IsMsg(szMessage, MMSGBOX_YES) == true) {
			ZPostRequestLeaveClan(ZGetMyUID());
		}
		else {
		}
		pWidget->Show(false);
		return false;
	}
} g_ClanLeaveConfirmListener;

MListener* ZGetClanLeaveConfirmListenter()
{
	return &g_ClanLeaveConfirmListener;
}

BEGIN_IMPLEMENT_LISTENER(ZGetArrangedTeamGameListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("ArrangedTeamGameDialog");
if (pWidget != NULL)
{
	pWidget->Show(true, true);

	unsigned long int nPlaceFilter = 0;
	SetBitSet(nPlaceFilter, MMP_LOBBY);

	ZPostRequestChannelAllPlayerList(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetChannelUID(), nPlaceFilter,
		MCP_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST_MYCLAN);
}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetRequestLadderRejoinGameListener, MBTN_CLK_MSG)
ZGetGameClient()->IsRejoin = true;
ZPostStageRequestRejoin();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetArrangedTeamDialogOkListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("ArrangedTeamGameDialog");
if (pWidget != NULL)
pWidget->Show(false);

ZPlayerSelectListBox* pPlayerList = (ZPlayerSelectListBox*)pResource->FindWidget("ArrangedTeamSelect");
if (pPlayerList)
{
	const int nMaxInviteCount = max(MAX_LADDER_TEAM_MEMBER, MAX_CLANBATTLE_TEAM_MEMBER) - 1;

	char szNames[nMaxInviteCount][MATCHOBJECT_NAME_LENGTH];
	char* ppNames[nMaxInviteCount];
	int nCount = 0;
	for (int i = 0; i < pPlayerList->GetCount(); i++)
	{
		MListItem* pItem = pPlayerList->Get(i);
		if (pItem->m_bSelected) {
			if (nCount >= nMaxInviteCount) {
				nCount++;
				break;
			}
			strcpy(szNames[nCount], pItem->GetString());
			ppNames[nCount] = szNames[nCount];
			nCount++;
		}
	}

	switch (ZGetGameClient()->GetServerMode())
	{
	case MSM_LADDER:
	{
		if (0 < nCount && nCount <= nMaxInviteCount) {
			ZGetGameClient()->RequestProposal(MPROPOSAL_LADDER_INVITE, ppNames, nCount);
		}
		else
		{
			ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM),
				ZErrStr(MSG_LADDER_INVALID_COUNT));
		}
	}
	break;
	case MSM_CLAN:
	{
		bool bRightMember = false;
		for (int i = 0; i < MLADDERTYPE_MAX; i++)
		{
			if ((g_nNeedLadderMemberCount[i] - 1) == nCount)
			{
				bRightMember = true;
				break;
			}
		}

		if ((0 < nCount) && (bRightMember))
		{
			ZGetGameClient()->RequestProposal(MPROPOSAL_CLAN_INVITE, ppNames, nCount);
		}
		else if (nCount == 0)
		{
			char szMember[1][MATCHOBJECT_NAME_LENGTH];
			char* ppMember[1];

			ppMember[0] = szMember[0];
			strcpy(szMember[0], ZGetMyInfo()->GetCharName());

			int nBalancedMatching = 0;
			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
			MButton* pButton = (MButton*)pResource->FindWidget("BalancedMatchingCheckBox");
			if ((pButton) && (pButton->GetCheck()))
			{
				nBalancedMatching = 1;
			}

			int nAntiLead = 0;
			MButton* pButton2 = (MButton*)pResource->FindWidget("AntiLeadCheckBox");
			if ((pButton2) && (pButton2->GetCheck()))
			{
				nAntiLead = 1;
			}

			ZPostLadderRequestChallenge(ppMember, 1, nBalancedMatching, nAntiLead);
		}

		else
		{
			ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM),
				ZMsg(MSG_LADDER_INVALID_COUNT));
		}
	}
	break;
	}
}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetArrangedTeamDialogCloseListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("ArrangedTeamGameDialog");
if (pWidget != NULL)
pWidget->Show(false);

pWidget = pResource->FindWidget("LobbyFindClanTeam");
if (pWidget != NULL)
pWidget->Show(false);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetProposalAgreementWait_CancelButtonListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("ProposalAgreementWait");
if (pWidget != NULL)
{
	pWidget->Show(false);
}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetProposalAgreementConfirm_OKButtonListener, MBTN_CLK_MSG)
ZGetGameClient()->ReplyAgreement(true);

ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("ProposalAgreementConfirm");
if (pWidget != NULL)
{
	pWidget->Show(false);
}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetProposalAgreementConfirm_CancelButtonListener, MBTN_CLK_MSG)
ZGetGameClient()->ReplyAgreement(false);

ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("ProposalAgreementConfirm");
if (pWidget != NULL)
{
	pWidget->Show(false);
}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetArrangedTeamGame_CancelListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("LobbyFindClanTeam");
if (pWidget != NULL)
pWidget->Show(false);

ZPostLadderCancel();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetPrivateChannelEnterListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MEdit* pEdit = (MEdit*)pResource->FindWidget("PrivateChannelInput");
if (pEdit != NULL)
{
	int nNameLen = (int)strlen(pEdit->GetText());

	if (nNameLen <= 0)
	{
		ZGetGameInterface()->ShowErrorMessage(MERR_PLZ_INPUT_CHARNAME);
		return true;
	}
	else if (nNameLen < MIN_CLANNAME)
	{
		ZGetGameInterface()->ShowErrorMessage(MERR_TOO_SHORT_NAME);
		return true;
	}
	else if (nNameLen > MAX_CLANNAME)
	{
		ZGetGameInterface()->ShowErrorMessage(MERR_TOO_LONG_NAME);
		return true;
	}

	if (!MGetChattingFilter()->IsValidStr(pEdit->GetText(), 1))
	{
		char szMsg[256];
		ZTransMsg(szMsg, MSG_WRONG_WORD_NAME, 1, MGetChattingFilter()->GetLastFilteredStr());
		ZGetGameInterface()->ShowMessage(szMsg, NULL, MSG_WRONG_WORD_NAME);
	}
	else
	{
		ZPostChannelRequestJoinFromChannelName(ZGetMyUID(), MCHANNEL_TYPE_USER, pEdit->GetText());

		MWidget* pFindWidget = pResource->FindWidget("ChannelListFrame");
		if (pFindWidget != NULL) pFindWidget->Show(false);
	}
}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetChannelList, MBTN_CLK_MSG)
MButton* pButton = (MButton*)pWidget;
int nIndexInGroup = pButton->GetIndexInGroup();
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

MCHANNEL_TYPE nChannelType = MCHANNEL_TYPE_PRESET;
switch (nIndexInGroup) {
case 0: nChannelType = MCHANNEL_TYPE_PRESET; break;
case 1: nChannelType = MCHANNEL_TYPE_USER; break;
case 2: nChannelType = MCHANNEL_TYPE_CLAN; break;
case 3: nChannelType = MCHANNEL_TYPE_DUELTOURNAMENT; break;

default: break;
}

ZGetGameInterface()->InitChannelFrame(nChannelType);
ZGetGameClient()->StartChannelList(nChannelType);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetMyClanChannel, MBTN_CLK_MSG)
if (ZGetMyInfo()->IsClanJoined())
{
	ZPostChannelRequestJoinFromChannelName(ZGetMyUID(), MCHANNEL_TYPE_CLAN, ZGetMyInfo()->GetClanName());

#ifdef LOCALE_NHNUSAA
	GetNHNUSAReport().ReportJoinChannel();
#endif

	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MWidget* pFindWidget = pResource->FindWidget("ChannelListFrame");
	if (pFindWidget != NULL) pFindWidget->Show(false);
}
else {
	ZGetGameInterface()->ShowMessage(MSG_LOBBY_NO_CLAN);
}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZShopItemEquipmentTabOpen, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnArmorWeaponTabButtonClicked(0);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZShopWeaponEquipmentTabOpen, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnArmorWeaponTabButtonClicked(1);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZShopListFrameClose, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->SelectEquipmentFrameList(NULL, false);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZShopListFrameOpen, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->SelectEquipmentFrameList(NULL, true);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZEquipItemEquipmentTabOpen, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnArmorWeaponTabButtonClicked(0);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZEquipWeaponEquipmentTabOpen, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnArmorWeaponTabButtonClicked(1);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZEquipListFrameClose, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->SelectEquipmentFrameList(NULL, false);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZEquipListFrameOpen, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->SelectEquipmentFrameList(NULL, true);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZEquipmetRotateBtn, MBTN_CLK_MSG)
ZCharacterView* pCharView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("EquipmentInformation");
if (pCharView)
{
	pCharView->EnableAutoRotate(!pCharView->IsAutoRotate());

	MBmButton* pButton = (MBmButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("Equipment_CharacterRotate");
	if (pButton)
	{
		if (pCharView->IsAutoRotate())
		{
			pButton->SetUpBitmap(MBitmapManager::Get("btn_rotate.tga"));
			pButton->SetDownBitmap(MBitmapManager::Get("btn_rotate.tga"));
			pButton->SetOverBitmap(MBitmapManager::Get("btn_rotate.tga"));
		}
		else
		{
			pButton->SetUpBitmap(MBitmapManager::Get("btn_stop.tga"));
			pButton->SetDownBitmap(MBitmapManager::Get("btn_stop.tga"));
			pButton->SetOverBitmap(MBitmapManager::Get("btn_stop.tga"));
		}
	}
}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZReplayOk, MBTN_CLK_MSG)
ZGetGameInterface()->OnReplay();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetReplayCallerButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->ShowReplayDialog(true);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetReplayViewButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->ViewReplay();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetReplayExitButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->ShowReplayDialog(false);
END_IMPLEMENT_LISTENER();

MListener* ZGetReplayFileListBoxListener(void)
{
	class ListenerClass : public MListener
	{
	public:
		virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
		{
			if (MWidget::IsMsg(szMessage, MLB_ITEM_SEL) == true)
			{
				MWidget* pFindWidget = ZGetGameInterface()->GetIDLResource()->FindWidget("Replay_View");
				if (pFindWidget != NULL)
					pFindWidget->Enable(true);

				return true;
			}
			else if (MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK) == true)
			{
				ZGetGameInterface()->ViewReplay();

				return true;
			}

			return false;
		}
	};
	static ListenerClass	Listener;
	return &Listener;
}

BEGIN_IMPLEMENT_LISTENER(ZGetLeaveClanOKListener, MBTN_CLK_MSG)
MWidget* pWidget = (MWidget*)ZGetGameInterface()->GetIDLResource()->FindWidget("ConfirmLeaveClan");
if (pWidget)
pWidget->Show(false);

ZPostRequestLeaveClan(ZGetMyUID());

ZPlayerListBox* pPlayerListBox = (ZPlayerListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerList");
if (pPlayerListBox)
pPlayerListBox->SetMode(ZPlayerListBox::PLAYERLISTMODE_CHANNEL_CLAN);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetLeaveClanCancelListener, MBTN_CLK_MSG)
MWidget* pWidget = (MWidget*)ZGetGameInterface()->GetIDLResource()->FindWidget("ConfirmLeaveClan");
if (pWidget)
pWidget->Show(false);
END_IMPLEMENT_LISTENER();

int g_lastPressedDuelTournamentGameBtn = 8;

BEGIN_IMPLEMENT_LISTENER(ZGetDuelTournamentGameButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->OnDuelTournamentGameUI(true);
ZPostDuelTournamentRequestJoinGame(ZGetMyUID(), MDUELTOURNAMENTTYPE_QUATERFINAL);
g_lastPressedDuelTournamentGameBtn = 8;
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetDuelTournamentGame2TestButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->OnDuelTournamentGameUI(true);
ZPostDuelTournamentRequestJoinGame(ZGetMyUID(), MDUELTOURNAMENTTYPE_FINAL);
g_lastPressedDuelTournamentGameBtn = 2;
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetDuelTournamentGame4TestButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->OnDuelTournamentGameUI(true);
ZPostDuelTournamentRequestJoinGame(ZGetMyUID(), MDUELTOURNAMENTTYPE_SEMIFINAL);
g_lastPressedDuelTournamentGameBtn = 4;
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetDuelTournamentWaitCancelButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->OnDuelTournamentGameUI(false);

#ifdef _PUBLISH
ZPostDuelTournamentRequestCancelGame(ZGetMyUID(), MDUELTOURNAMENTTYPE_QUATERFINAL);
#else
switch (g_lastPressedDuelTournamentGameBtn) {
case 2: ZPostDuelTournamentRequestCancelGame(ZGetMyUID(), MDUELTOURNAMENTTYPE_FINAL); break;
case 4: ZPostDuelTournamentRequestCancelGame(ZGetMyUID(), MDUELTOURNAMENTTYPE_SEMIFINAL); break;
default: ZPostDuelTournamentRequestCancelGame(ZGetMyUID(), MDUELTOURNAMENTTYPE_QUATERFINAL); break;
}
#endif

END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZStageSacrificeItem0, MBTN_CLK_MSG)
ZApplication::GetStageInterface()->OnSacrificeItem0();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZStageSacrificeItem1, MBTN_CLK_MSG)
ZApplication::GetStageInterface()->OnSacrificeItem1();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZStagePutSacrificeItem, MBTN_CLK_MSG)
if (!ZApplication::GetStageInterface()->m_SacrificeItem[0].IsExist())
ZApplication::GetStageInterface()->OnDropSacrificeItem(0);
else if (!ZApplication::GetStageInterface()->m_SacrificeItem[1].IsExist())
ZApplication::GetStageInterface()->OnDropSacrificeItem(1);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZStageSacrificeItemBoxOpen, MBTN_CLK_MSG)
ZApplication::GetStageInterface()->OpenSacrificeItemBox();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZStageSacrificeItemBoxClose, MBTN_CLK_MSG)
ZApplication::GetStageInterface()->CloseSacrificeItemBox();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetRelayMapTypeListener, MCMBBOX_CHANGED)
ZApplication::GetStageInterface()->PostRelayMapElementUpdate();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetRelayMapTurnCountListener, MCMBBOX_CHANGED)
ZApplication::GetStageInterface()->PostRelayMapElementUpdate();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZStageRelayMapBoxOpen, MBTN_CLK_MSG)
ZApplication::GetStageInterface()->OpenRelayMapBox();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZStageRelayMapBoxClose, MBTN_CLK_MSG)
ZApplication::GetStageInterface()->CloseRelayMapBox();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetRelayMapOKButtonListener, MBTN_CLK_MSG)
ZApplication::GetStageInterface()->PostRelayMapInfoUpdate();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetGameResultQuit, MBTN_CLK_MSG)
if (ZGetGameClient()->IsLadderGame() || ZGetGameClient()->IsDuelTournamentGame())
PostMessage(g_hWnd, WM_CHANGE_GAMESTATE, GUNZ_LOBBY, 0);
else
PostMessage(g_hWnd, WM_CHANGE_GAMESTATE, GUNZ_STAGE, 0);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetMonsterBookCaller, MBTN_CLK_MSG)
ZGetGameInterface()->GetMonsterBookInterface()->OnCreate();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetMonsterInterfacePrevPage, MBTN_CLK_MSG)
ZGetGameInterface()->GetMonsterBookInterface()->OnPrevPage();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetMonsterInterfaceNextPage, MBTN_CLK_MSG)
ZGetGameInterface()->GetMonsterBookInterface()->OnNextPage();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetMonsterInterfaceQuit, MBTN_CLK_MSG)
ZGetGameInterface()->GetMonsterBookInterface()->OnDestroy();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetRegisterListener, MBTN_CLK_MSG)
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetArrangedPlayerWarsListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("PlayerWarsMapVote");
if (pWidget != NULL && pWidget->IsVisible()) return true;
pWidget = pResource->FindWidget("PlayerWarsGameDialog");
if (pWidget != NULL && pWidget->IsVisible()) return true;
pWidget = pResource->FindWidget("LobbyFindClanTeam");
if (pWidget != NULL && pWidget->IsVisible()) return true;

pWidget = pResource->FindWidget("PlayerWarsGameDialog");
if (pWidget != NULL)
{
	pWidget->Show(true, true);
	unsigned long int nPlaceFilter = 0;
	SetBitSet(nPlaceFilter, MMP_LOBBY);
	char Name[100];
	for (int i = 0; i < 3; i++)
	{
		sprintf(Name, "PlayerWarsVote%d", i);
		MLabel* pLabel = (MLabel*)pResource->FindWidget(Name);
		if (pLabel)
			pLabel->SetTextColor(MCOLOR(255, 255, 15));
	}
}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetPlayerWarsVote0, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
char Name[100];
for (int i = 0; i < 3; i++)
{
	sprintf(Name, "PlayerWarsVote%d", i);
	MLabel* pLabel = (MLabel*)pResource->FindWidget(Name);
	if (pLabel)
		pLabel->SetTextColor(MCOLOR(255, 255, 15));
}
ZGetGameClient()->LastVoteID = 0;
MLabel* pLabel = (MLabel*)pResource->FindWidget("PlayerWarsVote0");
if (pLabel)
pLabel->SetTextColor(MCOLOR(102, 205, 0));
ZPostPlayerWarsVote(0);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetPlayerWarsVote1, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
char Name[100];
for (int i = 0; i < 3; i++)
{
	sprintf(Name, "PlayerWarsVote%d", i);
	MLabel* pLabel = (MLabel*)pResource->FindWidget(Name);
	if (pLabel)
		pLabel->SetTextColor(MCOLOR(255, 255, 15));
}
ZGetGameClient()->LastVoteID = 1;
MLabel* pLabel = (MLabel*)pResource->FindWidget("PlayerWarsVote1");
if (pLabel)
pLabel->SetTextColor(MCOLOR(102, 205, 0));
ZPostPlayerWarsVote(1);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetPlayerWarsVote2, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
char Name[100];
for (int i = 0; i < 3; i++)
{
	sprintf(Name, "PlayerWarsVote%d", i);
	MLabel* pLabel = (MLabel*)pResource->FindWidget(Name);
	if (pLabel)
		pLabel->SetTextColor(MCOLOR(255, 255, 15));
}
ZGetGameClient()->LastVoteID = 2;
MLabel* pLabel = (MLabel*)pResource->FindWidget("PlayerWarsVote2");
if (pLabel)
pLabel->SetTextColor(MCOLOR(102, 205, 0));
ZPostPlayerWarsVote(2);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetArrangedPlayerWarsListenerOpen, MBTN_CLK_MSG)
if (ZGetGameClient()->bMatching) //checking if true
ZGetGameInterface()->OnPlayerWarsShowerOpen(true);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetArrangedPlayerWarsListenerCloser, MBTN_CLK_MSG)
ZGetGameInterface()->OnPlayerWarsShower(false);
END_IMPLEMENT_LISTENER();