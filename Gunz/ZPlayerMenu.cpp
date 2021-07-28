#include "stdafx.h"
#include "ZPlayerMenu.h"
#include "ZApplication.h"
#include "ZGameInterface.h"
#include "ZGameClient.h"
#include "ZPost.h"
#include "ZIDLResource.h"
#include "ZPlayerListBox.h"
#include "ZMyInfo.h"
#include "ZChat_CmdID.h"

ZPlayerMenuListener listenerPlayerMenu;

ZPlayerMenuItem::ZPlayerMenuItem(ZCMD_PLAYERMENU nCmdID, const char* szName) : MMenuItem(szName)
{
	m_nCmdID = nCmdID;
}

ZPlayerMenu::ZPlayerMenu(const char* szName, MWidget* pParent, MListener* pListener, MPopupMenuTypes t)
	: MPopupMenu(szName, pParent, pListener, t)
{
	m_szPlayerName[0] = NULL;
}

void ZPlayerMenu::AddMenuItem(ZPlayerMenuItem* pMenuItem)
{
	MPopupMenu::AddMenuItem(pMenuItem);
	pMenuItem->SetListener(&listenerPlayerMenu);
}

void ZPlayerMenu::SetupMenu(ZPLAYERMENU_SET nMenuSet)
{
	RemoveAllMenuItem();

	SetBounds(0, 0, 200, GetChildCount() * 20);

	if (nMenuSet == ZPLAYERMENU_SET_LOBBY) {
		AddMenuItem(new ZPlayerMenuItem(ZCMD_PLAYERMENU_WHISPER, ZMsg(MSG_MENUITEM_FRIENDWHISPER)));
		if (ZGetGameClient()->GetChannelType() != MCHANNEL_TYPE_DUELTOURNAMENT)
			AddMenuItem(new ZPlayerMenuItem(ZCMD_PLAYERMENU_FOLLOW, ZMsg(MSG_MENUITEM_FRIENDFOLLOW)));

		AddMenuItem(new ZPlayerMenuItem(ZCMD_PLAYERMENU_FRIEND_ADD, ZMsg(MSG_MENUITEM_FRIENDADD)));

		MMatchClanGrade myGrade = ZGetMyInfo()->GetClanGrade();
		if (myGrade == MCG_MASTER || myGrade == MCG_ADMIN) {
			MPopupMenu::AddMenuItem("--------");
			AddMenuItem(new ZPlayerMenuItem(ZCMD_PLAYERMENU_CLAN_INVITE, ZMsg(MSG_MENUITEM_FRIENDCLANINVITE)));
		}
	}
	else if (nMenuSet == ZPLAYERMENU_SET_STAGE) {
		AddMenuItem(new ZPlayerMenuItem(ZCMD_PLAYERMENU_WHISPER, ZMsg(MSG_MENUITEM_FRIENDWHISPER)));
		AddMenuItem(new ZPlayerMenuItem(ZCMD_PLAYERMENU_KICK, ZMsg(MSG_MENUITEM_FRIENDKICK)));
		AddMenuItem(new ZPlayerMenuItem(ZCMD_PLAYERMENU_FRIEND_ADD, ZMsg(MSG_MENUITEM_FRIENDADD)));
	}
	else if (nMenuSet == ZPLAYERMENU_SET_FRIEND) {
		AddMenuItem(new ZPlayerMenuItem(ZCMD_PLAYERMENU_WHISPER, ZMsg(MSG_MENUITEM_FRIENDWHISPER)));
		AddMenuItem(new ZPlayerMenuItem(ZCMD_PLAYERMENU_WHERE, ZMsg(MSG_MENUITEM_FRIENDWHERE)));
		AddMenuItem(new ZPlayerMenuItem(ZCMD_PLAYERMENU_FRIEND_REMOVE, ZMsg(MSG_MENUITEM_FRIENDREMOVE)));

#ifdef _VOTESETTING
		if (ZGetGameClient()->GetChannelType() != MCHANNEL_TYPE_DUELTOURNAMENT)
			AddMenuItem(new ZPlayerMenuItem(ZCMD_PLAYERMENU_FRIEND_FOLLOW, ZMsg(MSG_MENUITEM_FRIENDFOLLOW)));
#endif
	}
	else if (nMenuSet == ZPLAYERMENU_SET_CLAN) {
		AddMenuItem(new ZPlayerMenuItem(ZCMD_PLAYERMENU_WHISPER, ZMsg(MSG_MENUITEM_FRIENDWHISPER)));
		AddMenuItem(new ZPlayerMenuItem(ZCMD_PLAYERMENU_WHERE, ZMsg(MSG_MENUITEM_FRIENDWHERE)));

#ifdef _VOTESETTING
		if (ZGetGameClient()->GetChannelType() != MCHANNEL_TYPE_DUELTOURNAMENT)
			AddMenuItem(new ZPlayerMenuItem(ZCMD_PLAYERMENU_CLAN_FOLLOW, ZMsg(MSG_MENUITEM_FRIENDFOLLOW)));
#endif

		MMatchClanGrade myGrade = ZGetMyInfo()->GetClanGrade();
		if (myGrade == MCG_MASTER || myGrade == MCG_ADMIN)
			MPopupMenu::AddMenuItem("--------");

		if (myGrade == MCG_MASTER)
		{
			AddMenuItem(new ZPlayerMenuItem(ZCMD_PLAYERMENU_CLAN_GRADE_ADMIN, ZMsg(MSG_MENUITEM_CLANGRADEADMIN)));
			AddMenuItem(new ZPlayerMenuItem(ZCMD_PLAYERMENU_CLAN_GRADE_MEMBER, ZMsg(MSG_MENUITEM_CLANMEMBER)));
	}
		if (myGrade == MCG_MASTER || myGrade == MCG_ADMIN)
		{
			AddMenuItem(new ZPlayerMenuItem(ZCMD_PLAYERMENU_CLAN_KICK, ZMsg(MSG_MENUITEM_CLANKICK)));
		}
}
	else if (nMenuSet == ZPLAYERMENU_SET_CLAN_ME) {
		AddMenuItem(new ZPlayerMenuItem(ZCMD_PLAYERMENU_CLAN_LEAVE, ZMsg(MSG_MENUITEM_CLANLEAVE)));
	}
	else {
		((MPopupMenu*)this)->AddMenuItem(ZMsg(MSG_MENUITEM_NONE));
	}
}

void ZPlayerMenu::Show(int x, int y, bool bVisible)
{
	MPopupMenu::Show(x, y, bVisible);
}

bool ZPlayerMenuListener::OnCommand(MWidget* pWidget, const char* szMessage)
{
	GunzState GunzState = ZApplication::GetGameInterface()->GetState();
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	ZPlayerMenu* pMenu = (ZPlayerMenu*)pWidget->GetParent();
	((MPopupMenu*)pMenu)->Show(false);
	ZPlayerMenuItem* pItem = (ZPlayerMenuItem*)pWidget;

	switch (pItem->GetCmdID()) {
	case ZCMD_PLAYERMENU_WHISPER:
	{
		char szMsg[128] = "";
		ZChatCmd* pWhisperCmd = ZApplication::GetGameInterface()->GetChat()->GetCmdManager()->GetCommandByID(CCMD_ID_WHISPER);
		if (pWhisperCmd)
		{
			sprintf(szMsg, "/%s %s ", pWhisperCmd->GetName(), pMenu->GetTargetName());
		}

		if (GunzState == GUNZ_LOBBY) {
			MEdit* pEdit = (MEdit*)pResource->FindWidget("ChannelChattingInput");
			if (pEdit) {
				pEdit->SetText(szMsg);
				pEdit->SetFocus();
			}
		}
		else if (GunzState == GUNZ_STAGE) {
			MEdit* pEdit = (MEdit*)pResource->FindWidget("StageChattingInput");
			if (pEdit) {
				pEdit->SetText(szMsg);
				pEdit->SetFocus();
			}
		}
		else if (GunzState == GUNZ_GAME) {
			MEdit* pEdit = (MEdit*)pResource->FindWidget("ChatInput");
			if (pEdit) {
				pEdit->SetText(szMsg);
				pEdit->SetFocus();
			}
		}
	}
	return true;
	case ZCMD_PLAYERMENU_FOLLOW:
	{
		if (GunzState == GUNZ_LOBBY) {
			ZPostStageFollow(pMenu->GetTargetName());
		}
	}
	return true;
	case ZCMD_PLAYERMENU_KICK:
	{
		if (GunzState == GUNZ_STAGE) {
			if ((ZGetGameClient()->AmIStageMaster()) || (ZGetMyInfo()->IsAdminGrade()))
			{
				char szMsg[128];
				sprintf(szMsg, "/kick %s", pMenu->GetTargetName());
				ZPostStageChat(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), szMsg);
			}
		}
	}
	return true;
	case ZCMD_PLAYERMENU_WHERE:
	{
		ZPostWhere(const_cast<char*>(pMenu->GetTargetName()));
	}
	return true;
	case ZCMD_PLAYERMENU_FRIEND_ADD:
	{
		ZPostFriendAdd(pMenu->GetTargetName());
	}
	return true;
	case ZCMD_PLAYERMENU_FRIEND_REMOVE:
	{
		ZPostFriendRemove(pMenu->GetTargetName());
		ZPostFriendList();
	}
	return true;
	case ZCMD_PLAYERMENU_FRIEND_PROMOTE:
	{
		OutputDebugString("ZPlayerMenu > ZCMD_PLAYERMENU_FRIEND_PROMOTE - NOT IMPLEMENTED YET \n");
	}
	return true;
	case ZCMD_PLAYERMENU_FRIEND_DEMOTE:
	{
		OutputDebugString("ZPlayerMenu > ZCMD_PLAYERMENU_FRIEND_DEMOTE - NOT IMPLEMENTED YET \n");
	}
	return true;
	case ZCMD_PLAYERMENU_FRIEND_FOLLOW:
	{
#ifdef _VOTESETTING
		if (GunzState == GUNZ_LOBBY) {
			ZPostStageFollow(pMenu->GetTargetName());
		}
#endif
	}
	return true;

	case ZCMD_PLAYERMENU_CLAN_INVITE:
		ZPostRequestJoinClan(ZGetMyUID(), ZGetMyInfo()->GetClanName(), pMenu->GetTargetName());
		return true;

	case ZCMD_PLAYERMENU_CLAN_KICK:
		ZPostRequestExpelClanMember(ZGetMyUID(), pMenu->GetTargetName());
		return true;

	case ZCMD_PLAYERMENU_CLAN_GRADE_MASTER:
		ZPostRequestChangeClanGrade(ZGetMyUID(), pMenu->GetTargetName(), MCG_MASTER);
		return true;

	case ZCMD_PLAYERMENU_CLAN_GRADE_ADMIN:
		ZPostRequestChangeClanGrade(ZGetMyUID(), pMenu->GetTargetName(), MCG_ADMIN);
		return true;

	case ZCMD_PLAYERMENU_CLAN_GRADE_MEMBER:
		ZPostRequestChangeClanGrade(ZGetMyUID(), pMenu->GetTargetName(), MCG_MEMBER);
		return true;

	case ZCMD_PLAYERMENU_CLAN_LEAVE:
	{
		MWidget* pWidget = (MWidget*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ConfirmLeaveClan");
		if (pWidget)
			pWidget->Show(true, true);
	}
	return true;

	case ZCMD_PLAYERMENU_CLAN_FOLLOW:
	{
#ifdef _VOTESETTING
		if (GunzState == GUNZ_LOBBY) {
			ZPostStageFollow(pMenu->GetTargetName());
		}
#endif
	}
	return true;
	};
	return false;
}