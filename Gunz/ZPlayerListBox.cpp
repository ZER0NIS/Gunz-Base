#include "stdafx.h"

#include "zapplication.h"
#include "MMatchObjCache.h"
#include "ZPlayerlistbox.h"
#include "MBitmap.h"
#include "MListBox.h"
#include "zpost.h"
#include "ZCharacterView.h"
#include "ZPlayerMenu.h"
#include "MToolTip.h"
#include "ZButton.h"
#include "ZMyInfo.h"

#define PLAYER_SLOT_GAP 1

#define PLAYERLIST_ITEM_HEIGHT	23

bool GetUserInfoUID(MUID uid, MCOLOR& _color, MMatchUserGradeID& gid);

#define COLORTEXT_SUPPORT // Custom: Added this to support colours in player list box

void ZPlayerListBoxLook::OnItemDraw2(MDrawContext* pDC, MRECT& r, const char* szText, MCOLOR color, bool bSelected, bool bFocus, int nAdjustWidth)
{
	if (szText == NULL) return;

	pDC->SetColor(color);

	MRECT rtemp, rtemp2;
	rtemp2 = rtemp = pDC->GetClipRect();
	rtemp2.x += r.x;
	rtemp2.y += r.y;
	rtemp2.w = r.w;
	rtemp2.h = r.h;
	pDC->SetClipRect(rtemp2);
#ifdef COLORTEXT_SUPPORT
	// Custom: Centre playerlist colour
	pDC->TextMultiLine(MRECT(r.x, r.y + (r.h - pDC->GetFont()->GetHeight()) / 2, r.w, r.h), szText, 0, false);
#else
	pDC->Text(r.x, r.y + (r.h - pDC->GetFont()->GetHeight()) / 2, szText);
#endif
	pDC->SetClipRect(rtemp);
}

void ZPlayerListBoxLook::OnItemDraw2(MDrawContext* pDC, MRECT& r, MBitmap* pBitmap, bool bSelected, bool bFocus, int nAdjustWidth)
{
	if (pBitmap == NULL) return;

	MRECT rtemp, rtemp2;
	rtemp2 = rtemp = pDC->GetClipRect();
	rtemp2.w -= nAdjustWidth;
	pDC->SetClipRect(rtemp2);

	pDC->SetBitmap(pBitmap);
	pDC->Draw(r, MRECT(0, 0, pBitmap->GetWidth(), pBitmap->GetHeight()));

	pDC->SetClipRect(rtemp);
}

MRECT ZPlayerListBoxLook::GetClientRect(MListBox* pListBox, MRECT& r)
{
	return r;
}

// Custom: Zeronis Fix Resolution
float GetF(float _new)
{
	float adjustWidth = _new / 800.f;
	if (RGetWidthScreen() != 0.75f)
	{
		if (RGetIsWidthScreen())
			adjustWidth = _new / 960.f;
		if (RGetIs16x9())
			adjustWidth = _new / 1024.f;
	}
	return adjustWidth;
}

void ZPlayerListBoxLook::OnDraw(MListBox* pListBox, MDrawContext* pDC)
{
	((ZPlayerListBox*)pListBox)->UpdateList(((ZPlayerListBox*)pListBox)->GetPlayerListMode());

	int newW = RGetScreenWidth();
	float fA = GetF(newW);

	m_SelectedPlaneColor = MCOLOR(180, 220, 180);

	int nItemHeight = 23 * fA;
	int nShowCount = 0;

	MRECT r = pListBox->GetClientRect();
	MPOINT pos = pListBox->GetPosition();

	int nHeaderHeight = nItemHeight;

	MRECT rr = pListBox->m_Rect;
	rr.x = 0;
	rr.y = 0;

	bool bShowClanCreateFrame =
		((ZPlayerListBox*)pListBox)->GetMode() == ZPlayerListBox::PLAYERLISTMODE_CHANNEL_CLAN
		&& !ZGetMyInfo()->IsClanJoined();

	pDC->SetColor(10, 10, 10);

	ZPlayerListBox::PLAYERLISTMODE pm = ((ZPlayerListBox*)pListBox)->GetPlayerListMode();

	int nMode = 0;

	if (pm == ZPlayerListBox::PLAYERLISTMODE_STAGE || pm == ZPlayerListBox::PLAYERLISTMODE_STAGE_FRIEND)
		nMode = 1;

	MBitmap* pBaseBmp = NULL;

	for (int i = pListBox->GetStartItem(); i < pListBox->GetCount(); i++) {
		MPOINT p;
		p.x = r.x;
		p.y = r.y + nHeaderHeight + nItemHeight * nShowCount;
		MListItem* pItem = pListBox->Get(i);
		bool bSelected = pItem->m_bSelected;
		bool bFocused = (pListBox->IsFocus());

		int nFieldStartX = 0;

		if (bSelected) {
			pDC->SetColor(130, 130, 130);
			pDC->Rectangle(MRECT(p.x + 2, p.y + 5, r.x + r.w - 8, nItemHeight - 4));
		}

		for (int j = 0; j < max(pListBox->GetFieldCount(), 1); j++) {
			int nTabSize = r.w;
			if (j < pListBox->GetFieldCount()) nTabSize = pListBox->GetField(j)->nTabSize;

			int nWidth = min(nTabSize, r.w - nFieldStartX);
			if (pListBox->m_bAbsoulteTabSpacing == false) nWidth = r.w * nTabSize / 100;

			int nAdjustWidth = 0;

			if (pListBox->GetScrollBar()->IsVisible()) {
				nAdjustWidth = pListBox->GetScrollBar()->GetRect().w + pListBox->GetScrollBar()->GetRect().w / 2;
			}

			MRECT ir(p.x + nFieldStartX + 5, p.y + 7, nWidth - 7, nItemHeight - 7);
			MRECT irt(p.x + nFieldStartX, p.y + 5, nWidth, nItemHeight);

			const char* szText = pItem->GetString(j);
			MBitmap* pBitmap = pItem->GetBitmap(j);
			MCOLOR color = pItem->GetColor();

			if (pBitmap != NULL)
				OnItemDraw2(pDC, ir, pBitmap, bSelected, bFocused, nAdjustWidth);

			if (szText != NULL)
			{
				OnItemDraw2(pDC, irt, szText, color, bSelected, bFocused, nAdjustWidth);
			}

			nFieldStartX += nWidth;
			if (nFieldStartX >= r.w) break;
		}

		nShowCount++;

		if (nShowCount >= pListBox->GetShowItemCount()) break;
	}
}

IMPLEMENT_LOOK(ZPlayerListBox, ZPlayerListBoxLook)

ZPlayerListBox::ZPlayerListBox(const char* szName, MWidget* pParent, MListener* pListener)
	: MListBox(szName, pParent, pListener)
{
	LOOK_IN_CONSTRUCTOR()
		SetVisibleHeader(false);

	m_bAbsoulteTabSpacing = true;

	m_nTotalPlayerCount = 0;
	m_nPage = 0;

	m_bVisible = true;
	m_bVisibleHeader = true;

	SetItemHeight(PLAYERLIST_ITEM_HEIGHT);

	mSelectedPlayer = 0;
	mStartToDisplay = 0;
	m_bAlwaysVisibleScrollbar = false;
	m_bHideScrollBar = true;
	m_pScrollBar->SetVisible(false);
	m_pScrollBar->Enable(false);

	m_pScrollBar->m_nDebugType = 3;
	m_nDebugType = 2;

	m_nOldW = RGetScreenWidth();

	SetListener(this);

	m_pButton = new ZBmButton(NULL, this, this);
	m_pButton->SetStretch(true);

	m_nMode = PLAYERLISTMODE_CHANNEL;
	InitUI(m_nMode);
}

ZPlayerListBox::~ZPlayerListBox(void)
{
	SAFE_DELETE(m_pButton);
}

void ZPlayerListBox::SetupButton(const char* szOn, const char* szOff)
{
	m_pButton->SetUpBitmap(MBitmapManager::Get(szOff));
	m_pButton->SetDownBitmap(MBitmapManager::Get(szOn));
	m_pButton->SetOverBitmap(MBitmapManager::Get(szOff));
}

void ZPlayerListBox::InitUI(PLAYERLISTMODE nMode)
{
	int newW = RGetScreenWidth();
	float fA = GetF(newW);

	m_nMode = nMode;
	RemoveAllField();

	const int nFields[PLAYERLISTMODE_END] = { 6,6,2,2,4,4 };
	for (int i = 0; i < nFields[nMode]; i++) {
		AddField("", 10);
	}
	OnSize(0, 0);

	bool bShowClanCreateFrame = false;

	switch (m_nMode) {
	case PLAYERLISTMODE_CHANNEL:	SetupButton("pltab_lobby_on.tga", "pltab_lobby_off.tga"); break;

	case PLAYERLISTMODE_STAGE: 		SetupButton("pltab_game_on.tga", "pltab_game_off.tga"); break;

	case PLAYERLISTMODE_CHANNEL_FRIEND:
	case PLAYERLISTMODE_STAGE_FRIEND: 	SetupButton("pltab_friends_on.tga", "pltab_friends_off.tga"); break;

	case PLAYERLISTMODE_CHANNEL_CLAN:
	case PLAYERLISTMODE_STAGE_CLAN:
	{
		SetupButton("pltab_clan_on.tga", "pltab_clan_off.tga");
		bShowClanCreateFrame = !ZGetMyInfo()->IsClanJoined();
	}break;
	}

	MWidget* pFrame = ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyPlayerListClanCreateFrame");
	MButton* pButtonUp = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerListPrev");
	MButton* pButtonDn = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerListNext");
	if (pFrame)
	{
		pFrame->Show(bShowClanCreateFrame);
		pButtonUp->Show(!bShowClanCreateFrame);
		pButtonDn->Show(!bShowClanCreateFrame);
	}
}

void ZPlayerListBox::RefreshUI()
{
	InitUI(GetMode());
}

void ZPlayerListBox::SetMode(PLAYERLISTMODE nMode)
{
	ZPlayerListBox* pWidget = (ZPlayerListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerList");
	if (pWidget) {
		pWidget->RemoveAll();
	}

	InitUI(nMode);

	if (!ZGetGameClient()) return;
	if (nMode == PLAYERLISTMODE_CHANNEL) {
		if (ZGetGameClient()->IsConnected()) {
			if (pWidget) {
				int nPage = pWidget->m_nPage;
				ZPostRequestChannelPlayerList(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetChannelUID(), nPage);
			}
		}
	}
	else if (nMode == PLAYERLISTMODE_STAGE) {
		if (ZGetGameClient()->IsConnected())
			ZPostRequestStagePlayerList(ZGetGameClient()->GetStageUID());
	}
	else if (nMode == PLAYERLISTMODE_CHANNEL_FRIEND || nMode == PLAYERLISTMODE_STAGE_FRIEND) {
		if (ZGetGameClient()->IsConnected())
			ZPostFriendList();
	}
	else if (nMode == PLAYERLISTMODE_CHANNEL_CLAN || nMode == PLAYERLISTMODE_STAGE_CLAN) {
		if (ZGetGameClient()->IsConnected())
			ZPostRequestClanMemberList(ZGetGameClient()->GetPlayerUID());
	}
}
void ZPlayerListBox::AddTestItems()
{
}

void GetRectMul(MRECT* rect, MRECT* org_rect, float f)
{
	rect->x = org_rect->x * f;
	rect->y = org_rect->y * f;
	rect->w = org_rect->w * f;
	rect->h = org_rect->h * f;
}

void ZPlayerListBox::OnSize(int w, int h)
{
	if (m_Fields.GetCount() == 0) return;

	int newW = RGetScreenWidth();
	float fA = GetF(newW);

	m_nItemHeight = PLAYERLIST_ITEM_HEIGHT * fA;

	m_pButton->SetBounds(0, 0, m_Rect.w, (int)(28.0 * fA));

	switch (m_nMode) {
	case PLAYERLISTMODE_CHANNEL:
	{
		m_Fields.Get(0)->nTabSize = 23 * fA;
		m_Fields.Get(1)->nTabSize = 16 * fA;
		m_Fields.Get(2)->nTabSize = 23 * fA;
		m_Fields.Get(3)->nTabSize = 94 * fA;
		m_Fields.Get(4)->nTabSize = 23 * fA;
		m_Fields.Get(5)->nTabSize = 94 * fA;
	}
	break;
	case PLAYERLISTMODE_STAGE:
	{
		m_Fields.Get(0)->nTabSize = 23 * fA;
		m_Fields.Get(1)->nTabSize = 16 * fA;
		m_Fields.Get(2)->nTabSize = 23 * fA;
		m_Fields.Get(3)->nTabSize = 94 * fA;
		m_Fields.Get(4)->nTabSize = 23 * fA;
		m_Fields.Get(5)->nTabSize = 94 * fA;
	}
	break;
	case PLAYERLISTMODE_STAGE_FRIEND:
	case PLAYERLISTMODE_CHANNEL_FRIEND:
	{
		m_Fields.Get(0)->nTabSize = 23 * fA;
		m_Fields.Get(1)->nTabSize = 72 * fA;
	}
	break;
	case PLAYERLISTMODE_CHANNEL_CLAN:
	case PLAYERLISTMODE_STAGE_CLAN:
	{
		m_Fields.Get(0)->nTabSize = 23 * fA;
		m_Fields.Get(1)->nTabSize = 85 * fA;
		m_Fields.Get(2)->nTabSize = 23 * fA;
		m_Fields.Get(3)->nTabSize = 90 * fA;
	}
	break;
	};
	RecalcList();
}
// Channel PlayerList
void ZPlayerListBox::AddPlayer(MUID& puid, ePlayerState state, int  nLevel, char* szName, char* szClanName, unsigned int nClanID, MMatchUserGradeID nGrade, int duelTournamentGrade)
{
	auto len = strlen(szName);
	if (len == 0)
		return;

	char szFileName[64] = "";
	char szLevel[64] = "";

	char* szRefName = NULL;
	MCOLOR _color;

	bool bSpUser = false;

	if (IsAdminGrade(nGrade))
		sprintf(szLevel, "--");
	else
		sprintf(szLevel, "%2d", nLevel);

	szRefName = szName;

	bSpUser = ZGetGame()->GetUserGradeIDColor(nGrade, _color);

	switch (state) {
	case PS_FIGHT: strcpy(szFileName, "player_status_player.tga");	break;
	case PS_WAIT: strcpy(szFileName, "player_status_game.tga");		break;
	case PS_LOBBY: strcpy(szFileName, "player_status_lobby.tga");	break;
	}

	char szDTGradeIconFileName[64];
	GetDuelTournamentGradeIconFileName(szDTGradeIconFileName, duelTournamentGrade);
	MBitmap* pBmpDTGradeIcon = MBitmapManager::Get(szDTGradeIconFileName);

	ZLobbyPlayerListItem* pItem = new ZLobbyPlayerListItem(puid, MBitmapManager::Get(szFileName), nClanID, szLevel, szRefName, szClanName, state, nGrade, pBmpDTGradeIcon);

	if (bSpUser)
		pItem->SetColor(_color);

	MListBox::Add(pItem);
}

// Stage PlayerList
void ZPlayerListBox::AddPlayer(MUID& puid, MMatchObjectStageState state, int nLevel, char* szName, char* szClanName, unsigned int nClanID, bool isMaster, MMatchTeam nTeam, int duelTournamentGrade)
{
	auto len = strlen(szName);
	if (len == 0)
		return;

	char szFileName[64] = "";
	char szFileNameState[64] = "";
	char szLevel[64] = "";

	char* szRefName = NULL;

	MCOLOR _color;
	bool bSpUser = false;

	MMatchUserGradeID gid = MMUG_FREE;

	MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(puid);
	if (IsAdminGrade(pObjCache->GetUGrade()))
		sprintf(szLevel, "--");
	else
		sprintf(szLevel, "%2d", nLevel);
	szRefName = szName;
	bSpUser = GetUserInfoUID(puid, _color, gid);

	MBitmap* pBitmap = NULL;
	if (isMaster) {
		switch (state) {
		case MOSS_NONREADY:
			if (nTeam == MMT_RED)			strcpy(szFileName, "stg_status_master_red.tga");
			else if (nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_master_blue.tga");
			else if (nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_master_observer.tga");
			else 							strcpy(szFileName, "stg_status_master_normal.tga");
			break;
		case MOSS_READY:
			if (nTeam == MMT_RED)			strcpy(szFileName, "stg_status_master_red_ready.tga");
			else if (nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_master_blue_ready.tga");
			else if (nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_master_observer.tga");
			else 							strcpy(szFileName, "stg_status_master_normal_ready.tga");
			break;
		case MOSS_EQUIPMENT:
			if (nTeam == MMT_RED)			strcpy(szFileName, "stg_status_master_red_equip.tga");
			else if (nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_master_blue_equip.tga");
			else if (nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_master_observer.tga");
			else 							strcpy(szFileName, "stg_status_master_normal_equip.tga");
			break;
		default:
			strcpy(szFileName, " ");
			break;
		}
	}
	else {
		switch (state) {
		case MOSS_NONREADY:
			if (nTeam == MMT_RED)			strcpy(szFileName, "stg_status_member_red.tga");
			else if (nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_member_blue.tga");
			else if (nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_member_observer.tga");
			else							strcpy(szFileName, "stg_status_member_normal.tga");
			break;
		case MOSS_READY:
			if (nTeam == MMT_RED)			strcpy(szFileName, "stg_status_member_red_ready.tga");
			else if (nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_member_blue_ready.tga");
			else if (nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_member_observer.tga");
			else							strcpy(szFileName, "stg_status_member_normal_ready.tga");
			break;
		case MOSS_EQUIPMENT:
			if (nTeam == MMT_RED)			strcpy(szFileName, "stg_status_member_red_equip.tga");
			else if (nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_member_blue_equip.tga");
			else if (nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_member_observer.tga");
			else							strcpy(szFileName, "stg_status_member_normal_equip.tga");
			break;
		}
	}
	pBitmap = MBitmapManager::Get(szFileName);

	char szDTGradeIconFileName[64];
	GetDuelTournamentGradeIconFileName(szDTGradeIconFileName, duelTournamentGrade);
	MBitmap* pBmpDTGradeIcon = MBitmapManager::Get(szDTGradeIconFileName);

	ZStagePlayerListItem* pItem = new ZStagePlayerListItem(puid, pBitmap, nClanID, szRefName, szClanName, szLevel, gid, pBmpDTGradeIcon);

	if (bSpUser)
		pItem->SetColor(_color);

	MListBox::Add(pItem);

	if (ZGetMyUID() == puid)
	{
		bool bBlue, bRed;
		bBlue = bRed = false;
		if (nTeam == MMT_BLUE)	bBlue = true;
		if (nTeam == MMT_RED)	bRed = true;

		MButton* pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageTeamBlue");
		pButton->SetCheck(bBlue);

		pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageTeamRed");
		pButton->SetCheck(bRed);
	}
}

void ZPlayerListBox::AddPlayer(ePlayerState state, char* szName, char* szLocation)
{
	if ((int)strlen(szName) == 0)
		return;

	char szFileName[64] = "";
	switch (state) {
	case PS_FIGHT: strcpy(szFileName, "player_status_player.tga");	break;
	case PS_WAIT: strcpy(szFileName, "player_status_game.tga");		break;
	case PS_LOBBY: strcpy(szFileName, "player_status_lobby.tga");	break;
		// Custom: Added custom player status image
	case PS_LOGOUT: strcpy(szFileName, "player_status_logout.tga");  break;
	}

	MListBox::Add(new ZFriendPlayerListItem(MUID(0, 0), MBitmapManager::Get(szFileName), szName, NULL, szLocation, state, MMUG_FREE));
}

void ZPlayerListBox::AddPlayer(MUID& puid, ePlayerState state, char* szName, int nLevel, MMatchClanGrade nGrade)
{
	auto len = strlen(szName);
	if (len == 0)
		return;

	char szFileName[64] = "";
	char szGradeName[64];

	char* szRefName = NULL;

	MCOLOR _color = MCOLOR(240, 64, 64);
	bool bSpUser = false;

	switch (nGrade)
	{
	case MCG_MASTER:
		sprintf(szGradeName, ZMsg(MSG_WORD_CLAN_MASTER));
		bSpUser = true;
		break;
	case MCG_ADMIN:
		sprintf(szGradeName, ZMsg(MSG_WORD_CLAN_ADMIN));
		bSpUser = true;
		break;
	default: sprintf(szGradeName, ZMsg(MSG_WORD_CLAN_MEMBER));
		break;
	}
	szRefName = szName;

	switch (state) {
	case PS_FIGHT: strcpy(szFileName, "player_status_player.tga");	break;
	case PS_WAIT: strcpy(szFileName, "player_status_game.tga");		break;
	case PS_LOBBY: strcpy(szFileName, "player_status_lobby.tga");	break;
		// Custom: Added custom player status image
	case PS_LOGOUT: strcpy(szFileName, "player_status_logout.tga");   break;
	}

	ZClanPlayerListItem* pItem = new ZClanPlayerListItem(puid, MBitmapManager::Get(szFileName), szRefName, szGradeName, NULL, state, nGrade);

	if (bSpUser)
		pItem->SetColor(_color);

	MListBox::Add(pItem);
}

void ZPlayerListBox::DelPlayer(MUID& puid)
{
	ZPlayerListItem* pItem = NULL;

	for (int i = 0; i < GetCount(); i++) {
		pItem = (ZPlayerListItem*)Get(i);
		if (pItem->m_PlayerUID == puid) {
			Remove(i);
			return;
		}
	}
}

ZPlayerListItem* ZPlayerListBox::GetUID(MUID uid)
{
	ZPlayerListItem* pItem = NULL;

	for (int i = 0; i < GetCount(); i++) {
		pItem = (ZPlayerListItem*)Get(i);
		if (pItem->m_PlayerUID == uid)
			return pItem;
	}
	return NULL;
}

const char* ZPlayerListBox::GetPlayerName(int nIndex)
{
	ZPlayerListItem* pItem = (ZPlayerListItem*)Get(nIndex);

	if (!pItem)
		return NULL;

	return pItem->m_szName;
}

static DWORD g_zplayer_list_update_time = 0;

#define ZPLAYERLIST_UPDATE_TIME 2000

void ZPlayerListBox::UpdateList(int mode)
{
	if (ZGetGameClient()->IsConnected() == false) return;

	DWORD this_time = timeGetTime();

	if (this_time < g_zplayer_list_update_time + ZPLAYERLIST_UPDATE_TIME)
		return;

	if (mode == PLAYERLISTMODE_CHANNEL) {
	}
	else if (mode == PLAYERLISTMODE_STAGE) {
	}
	else if (mode == PLAYERLISTMODE_CHANNEL_CLAN) {
	}

	g_zplayer_list_update_time = this_time;
}

void ZPlayerListBox::UpdatePlayer(MUID& puid, MMatchObjectStageState state, bool isMaster, MMatchTeam nTeam)
{
	ZStagePlayerListItem* pItem = (ZStagePlayerListItem*)GetUID(puid);

	if (pItem) {
		char szFileName[64] = "";
		char szFileNameState[64] = "";

		MBitmap* pBitmap = NULL;
		MBitmap* pBitmapState = NULL;

		const MSTAGE_SETTING_NODE* pStageSetting = ZGetGameClient()->GetMatchStageSetting()->GetStageSetting();

		if ((nTeam != MMT_SPECTATOR) && (ZGetGameTypeManager()->IsTeamGame(pStageSetting->nGameType) == false))
		{
			nTeam = MMT_ALL;
		}

		if (isMaster) {
			switch (state) {
			case MOSS_NONREADY:
				if (nTeam == MMT_RED) { strcpy(szFileName, "stg_status_master_red.tga");				pItem->m_nTeam = 1; }
				else if (nTeam == MMT_BLUE) { strcpy(szFileName, "stg_status_master_blue.tga");				pItem->m_nTeam = 2; }
				else if (nTeam == MMT_SPECTATOR) { strcpy(szFileName, "stg_status_master_observer.tga");			pItem->m_nTeam = 3; }
				else { strcpy(szFileName, "stg_status_master_normal.tga");			pItem->m_nTeam = 0; }
				break;
			case MOSS_READY:
				if (nTeam == MMT_RED) { strcpy(szFileName, "stg_status_master_red_ready.tga");		pItem->m_nTeam = 1; }
				else if (nTeam == MMT_BLUE) { strcpy(szFileName, "stg_status_master_blue_ready.tga");		pItem->m_nTeam = 2; }
				else if (nTeam == MMT_SPECTATOR) { strcpy(szFileName, "stg_status_master_observer.tga");			pItem->m_nTeam = 3; }
				else { strcpy(szFileName, "stg_status_master_normal_ready.tga");		pItem->m_nTeam = 0; }
				break;
			case MOSS_EQUIPMENT:
				if (nTeam == MMT_RED) { strcpy(szFileName, "stg_status_master_red_equip.tga");		pItem->m_nTeam = 1; }
				else if (nTeam == MMT_BLUE) { strcpy(szFileName, "stg_status_master_blue_equip.tga");		pItem->m_nTeam = 2; }
				else if (nTeam == MMT_SPECTATOR) { strcpy(szFileName, "stg_status_master_observer.tga");			pItem->m_nTeam = 3; }
				else { strcpy(szFileName, "stg_status_master_normal_equip.tga");		pItem->m_nTeam = 0; }
				break;
			default:
				strcpy(szFileName, " ");
				break;
			}
		}
		else {
			switch (state) {
			case MOSS_NONREADY:
				if (nTeam == MMT_RED) { strcpy(szFileName, "stg_status_member_red.tga");				pItem->m_nTeam = 1; }
				else if (nTeam == MMT_BLUE) { strcpy(szFileName, "stg_status_member_blue.tga");				pItem->m_nTeam = 2; }
				else if (nTeam == MMT_SPECTATOR) { strcpy(szFileName, "stg_status_member_observer.tga");			pItem->m_nTeam = 3; }
				else { strcpy(szFileName, "stg_status_member_normal.tga");			pItem->m_nTeam = 0; }
				break;
			case MOSS_READY:
				if (nTeam == MMT_RED) { strcpy(szFileName, "stg_status_member_red_ready.tga");		pItem->m_nTeam = 1; }
				else if (nTeam == MMT_BLUE) { strcpy(szFileName, "stg_status_member_blue_ready.tga");		pItem->m_nTeam = 2; }
				else if (nTeam == MMT_SPECTATOR) { strcpy(szFileName, "stg_status_member_observer.tga");			pItem->m_nTeam = 3; }
				else { strcpy(szFileName, "stg_status_member_normal_ready.tga");		pItem->m_nTeam = 0; }
				break;
			case MOSS_EQUIPMENT:
				if (nTeam == MMT_RED) { strcpy(szFileName, "stg_status_member_red_equip.tga");		pItem->m_nTeam = 1; }
				else if (nTeam == MMT_BLUE) { strcpy(szFileName, "stg_status_member_blue_equip.tga");		pItem->m_nTeam = 2; }
				else if (nTeam == MMT_SPECTATOR) { strcpy(szFileName, "stg_status_member_observer.tga");			pItem->m_nTeam = 3; }
				else { strcpy(szFileName, "stg_status_member_normal_equip.tga");		pItem->m_nTeam = 0; }
				break;
			}
		}

		pBitmap = MBitmapManager::Get(szFileName);
		pItem->m_pBitmap = pBitmap;
	}

	ZCharacterView* pCharView = (ZCharacterView*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_Charviewer_");
	if (pCharView != 0 && puid == pCharView->m_Info.UID)
	{
		pCharView->m_Info.bMaster = isMaster;
		pCharView->m_Info.m_pMnTeam->Set_CheckCrc((int)nTeam);
		pCharView->m_Info.nStageState = state;
	}

	if ((nTeam != MMT_SPECTATOR) && (ZGetMyUID() == puid))
	{
		bool bBlue, bRed;
		bBlue = bRed = false;
		if (nTeam == MMT_BLUE)	bBlue = true;
		if (nTeam == MMT_RED)	bRed = true;

		MButton* pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageTeamBlue");
		pButton->SetCheck(bBlue);

		pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageTeamRed");
		pButton->SetCheck(bRed);
	}
}

void ZPlayerListBox::UpdatePlayer(MUID& puid, MMatchObjectStageState state, char* szName, int  nLevel, bool isMaster, MMatchTeam nTeam)
{
	return;

	ZStagePlayerListItem* pItem = (ZStagePlayerListItem*)GetUID(puid);
	if (pItem) {
		char szFileName[64] = "";
		char szFileNameState[64] = "";
		char szLevel[64];

		MBitmap* pBitmap = NULL;
		MBitmap* pBitmapState = NULL;

		if (isMaster) {
			switch (state) {
			case MOSS_NONREADY:
				if (nTeam == MMT_RED)			strcpy(szFileName, "stg_status_master_red.tga");
				else if (nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_master_blue.tga");
				else if (nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_master_observer.tga");
				else 							strcpy(szFileName, "stg_status_master_normal.tga");
				break;
			case MOSS_READY:
				if (nTeam == MMT_RED)			strcpy(szFileName, "stg_status_master_red_ready.tga");
				else if (nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_master_blue_ready.tga");
				else if (nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_master_observer.tga");
				else 							strcpy(szFileName, "stg_status_master_normal_ready.tga");
				break;
			case MOSS_EQUIPMENT:
				if (nTeam == MMT_RED)			strcpy(szFileName, "stg_status_master_red_equip.tga");
				else if (nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_master_blue_equip.tga");
				else if (nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_master_observer.tga");
				else 							strcpy(szFileName, "stg_status_master_normal_equip.tga");
				break;
			default:
				strcpy(szFileName, " ");
				break;
			}
		}
		else {
			switch (state) {
			case MOSS_NONREADY:
				if (nTeam == MMT_RED)			strcpy(szFileName, "stg_status_member_red.tga");
				else if (nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_member_blue.tga");
				else if (nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_member_observer.tga");
				else 							strcpy(szFileName, "stg_status_member_normal.tga");
				break;
			case MOSS_READY:
				if (nTeam == MMT_RED)			strcpy(szFileName, "stg_status_member_red_ready.tga");
				else if (nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_member_blue_ready.tga");
				else if (nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_member_observer.tga");
				else 							strcpy(szFileName, "stg_status_member_normal_ready.tga");
				break;
			case MOSS_EQUIPMENT:
				if (nTeam == MMT_RED)			strcpy(szFileName, "stg_status_member_red_equip.tga");
				else if (nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_member_blue_equip.tga");
				else if (nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_member_observer.tga");
				else 							strcpy(szFileName, "stg_status_member_normal_equip.tga");
				break;
			}
		}
		pBitmap = MBitmapManager::Get(szFileName);
		pItem->m_pBitmap = pBitmap;
		sprintf(szLevel, "Lv %2d", nLevel);
		strcpy(pItem->m_szLevel, szLevel);
		strcpy(pItem->m_szName, szName);
	}

	ZCharacterView* pCharView = (ZCharacterView*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_Charviewer_");
	if (pCharView != 0 && puid == pCharView->m_Info.UID)
	{
		pCharView->m_Info.bMaster = isMaster;
		pCharView->m_Info.m_pMnTeam->Set_CheckCrc((int)nTeam);
		pCharView->m_Info.nStageState = state;
		pCharView->m_Info.nLevel = nLevel;
		pCharView->SetText(szName);
	}

	if (ZGetMyUID() == puid)
	{
		bool bBlue, bRed;
		bBlue = bRed = false;
		if (nTeam == MMT_BLUE)	bBlue = true;
		if (nTeam == MMT_RED)	bRed = true;

		MButton* pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageTeamBlue");
		pButton->SetCheck(bBlue);

		pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageTeamRed");
		pButton->SetCheck(bRed);
	}
}

bool ZPlayerListBox::OnCommand(MWidget* pWidget, const char* szMessage)
{
	if (pWidget == m_pButton) {
		if (strcmp(szMessage, MBTN_CLK_MSG) == 0) {
			switch (GetMode()) {
			case PLAYERLISTMODE_CHANNEL:
				SetMode(PLAYERLISTMODE_CHANNEL_FRIEND);
				break;
			case PLAYERLISTMODE_CHANNEL_FRIEND:
				SetMode(PLAYERLISTMODE_CHANNEL_CLAN);
				break;
			case PLAYERLISTMODE_CHANNEL_CLAN:
				SetMode(PLAYERLISTMODE_CHANNEL);
				break;
			case PLAYERLISTMODE_STAGE:
				break;
			case PLAYERLISTMODE_STAGE_FRIEND:
				break;
			case PLAYERLISTMODE_STAGE_CLAN:
				break;
			default:
				SetMode(PLAYERLISTMODE_CHANNEL);
				break;
			}
			return true;
		}
	}
	else
		if (strcmp(szMessage, "selected") == 0) {
			if (m_nSelItem != -1) {
				ZStagePlayerListItem* pItem = (ZStagePlayerListItem*)Get(m_nSelItem);
				if (pItem) {
					MUID uid = pItem->m_PlayerUID;

					ZCharacterView* pCharView = (ZCharacterView*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_Charviewer");
					if (pCharView != 0) pCharView->SetCharacter(uid);
				}
			}
			return true;
		}
		else if (strcmp(szMessage, "selected2") == 0) {
			if ((GetKeyState(VK_MENU) & 0x8000) != 0) {
				if (m_nSelItem != -1) {
					ZStagePlayerListItem* pItem = (ZStagePlayerListItem*)Get(m_nSelItem);
					if (pItem) {
						char temp[1024];
						sprintf(temp, "/kick %s", pItem->m_szName);
						ZPostStageChat(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), temp);
					}
				}
			}
		}

	return true;
}
bool ZPlayerListBox::OnEvent(MEvent* pEvent, MListener* pListener)
{
	MRECT rtClient = GetClientRect();

	if (pEvent->nMessage == MWM_RBUTTONDOWN) {
		if (rtClient.InPoint(pEvent->Pos) == true) {
			int nSelItem = FindItem(pEvent->Pos);
			if (nSelItem != -1) {
				ZLobbyPlayerListItem* pItem = (ZLobbyPlayerListItem*)Get(nSelItem);

				bool bShow = true;

				MCOLOR _color;
				MMatchUserGradeID gid;

				if (pItem->GetUID() == ZGetMyUID() &&
					GetMode() != PLAYERLISTMODE_CHANNEL_CLAN)
					bShow = false;

				GetUserInfoUID(pItem->GetUID(), _color, gid);

				if ((gid == MMUG_DEVELOPER) || (gid == MMUG_ADMIN)) {
					bShow = false;
				}

				SetSelIndex(nSelItem);
				if (bShow) {
					ZPlayerMenu* pMenu = ZApplication::GetGameInterface()->GetPlayerMenu();
					pMenu->SetTargetName(pItem->GetString());
					pMenu->SetTargetUID(pItem->GetUID());

					switch (GetMode()) {
					case PLAYERLISTMODE_CHANNEL:
						pMenu->SetupMenu(ZPLAYERMENU_SET_LOBBY);
						break;
					case PLAYERLISTMODE_STAGE:
						pMenu->SetupMenu(ZPLAYERMENU_SET_STAGE);
						break;
					case PLAYERLISTMODE_CHANNEL_FRIEND:
						pMenu->SetupMenu(ZPLAYERMENU_SET_FRIEND);
						break;
					case PLAYERLISTMODE_STAGE_FRIEND:
						pMenu->SetupMenu(ZPLAYERMENU_SET_FRIEND);
						break;
					case PLAYERLISTMODE_CHANNEL_CLAN:
						if (pItem->GetUID() == ZGetMyUID())
							pMenu->SetupMenu(ZPLAYERMENU_SET_CLAN_ME);
						else
							pMenu->SetupMenu(ZPLAYERMENU_SET_CLAN);
						break;
					default:
						break;
					};

					MPOINT posItem;
					posItem = pEvent->Pos;
					MPOINT posMenu = MClientToScreen(this, posItem);

					if ((posMenu.x + pMenu->GetClientRect().w) > (MGetWorkspaceWidth() - 5))
						posMenu.x = MGetWorkspaceWidth() - pMenu->GetClientRect().w - 5;

					if ((posMenu.y + pMenu->GetClientRect().h) > (MGetWorkspaceHeight() - 5))
						posMenu.y = MGetWorkspaceHeight() - pMenu->GetClientRect().h - 5;

					pMenu->Show(posMenu.x, posMenu.y);
				}
				return true;
			}
		}
	}

	else if (pEvent->nMessage == MWM_LBUTTONDBLCLK)
	{
		if (rtClient.InPoint(pEvent->Pos) == true)
		{
			int nSelItem = FindItem(pEvent->Pos);

			if (nSelItem != -1)
			{
#ifdef _DEBUG
				ZLobbyPlayerListItem* pItem = (ZLobbyPlayerListItem*)Get(nSelItem);
				ZPostRequestCharInfoDetail(ZGetMyUID(), pItem->m_szName);
#endif
			}
		}
	}

	return MListBox::OnEvent(pEvent, pListener);;
}

MUID ZPlayerListBox::GetSelectedPlayerUID()
{
	ZLobbyPlayerListItem* pItem = (ZLobbyPlayerListItem*)GetSelItem();
	if (!pItem) return MUID(0, 0);

	return pItem->GetUID();
}

void ZPlayerListBox::SelectPlayer(MUID uid)
{
	for (int i = 0; i < GetCount(); i++)
	{
		ZLobbyPlayerListItem* pItem = (ZLobbyPlayerListItem*)Get(i);
		if (pItem->GetUID() == uid) {
			SetSelIndex(i);
			return;
		}
	}
}

void ZPlayerListBox::MultiplySize(float byIDLWidth, float byIDLHeight, float byCurrWidth, float byCurrHeight)
{
	MListBox::MultiplySize(byIDLWidth, byIDLHeight, byCurrWidth, byCurrHeight);

	OnSize(GetRect().w, GetRect().h);
}