#include "stdafx.h"

#include "ZGameClient.h"
#include "ZCombatInterface.h"
#include "ZGameInterface.h"
#include "ZInterfaceItem.h"
#include "ZInterfaceListener.h"
#include "ZApplication.h"
#include "ZCharacter.h"
#include "ZCharacterManager.h"
#include "RealSpace2.h"
#include "MComboBox.h"
#include "RTypes.h"
#include "ZScreenEffectManager.h"
#include "ZActionDef.h"
#include "ZEffectFlashBang.h"
#include "ZConfiguration.h"
#include "ZPost.h"
#include "ZWeaponScreenEffect.h"
#include "MemPool.h"
#include "ZMyInfo.h"
#include "ZCharacterView.h"
#include "ZItemSlotView.h"
#include <algorithm>
#include <Sensapi.h>
#pragma comment(lib, "Sensapi.lib")

#include "ZApplication.h"
#include "ZCombatQuestScreen.h"
#include "ZBmNumLabel.h"
#include "ZModule_QuestStatus.h"
#include "ZLocale.h"

#include "ZRuleDuel.h"
#include "ZRuleDuelTournament.h"
#include "ZInput.h"

using namespace std;

bool GetUserInfoUID(MUID uid, MCOLOR& _color, MMatchUserGradeID& gid);

#define BACKGROUND_COLOR1					0xff202020
#define BACKGROUND_COLOR2					0xff000000
#define BACKGROUND_COLOR_MYCHAR_DEATH_MATCH	MINT_ARGB(255*40/100,140,180,255)
#define BACKGROUND_COLOR_MYCHAR_RED_TEAM	MINT_ARGB(255*40/100,255,50,50)
#define BACKGROUND_COLOR_MYCHAR_BLUE_TEAM	MINT_ARGB(255*40/100,50,50,255)

#define BACKGROUND_COLOR_COMMANDER			MINT_ARGB(255*40/100,255,88,255)

#define TEXT_COLOR_TITLE			0xFFAAAAAA
#define TEXT_COLOR_DEATH_MATCH		0xfffff696
#define TEXT_COLOR_DEATH_MATCH_DEAD	0xff807b4b
#define TEXT_COLOR_BLUE_TEAM		0xff8080ff
#define TEXT_COLOR_BLUE_TEAM_DEAD	0xff606080
#define TEXT_COLOR_RED_TEAM			0xffff8080
#define TEXT_COLOR_RED_TEAM_DEAD	0xff806060
#define TEXT_COLOR_SPECTATOR		0xff808080
#define TEXT_COLOR_CLAN_NAME		0xffffffff

struct ZScoreBoardItem : public CMemPoolSm<ZScoreBoardItem> {
	MUID uidUID;
	char szLevel[16];
	char szName[64];
	char szClan[CLAN_NAME_LENGTH];
	int nDuelQueueIdx;
	int	nClanID;
	int nTeam;
	bool bDeath;
	int nExp;
	int nKills;
	int nDeaths;
	int nPing;
	int nDTLastWeekGrade;
	bool bMyChar;
	bool bCommander;
	bool bGameRoomUser;

	MCOLOR SpColor;
	bool  bSpColor;

	ZScoreBoardItem(const MUID& _uidUID, char* _szLevel, char* _szName, char* _szClan, int _nTeam, bool _bDeath, int _nExp, int _nKills, int _nDeaths, int _nPing, int _nDTLastWeekGrade, bool _bMyChar, bool _bGameRoomUser, bool _bCommander = false)
	{
		uidUID = _uidUID;
		strcpy(szLevel, _szLevel);
		strcpy(szName, _szName);
		strcpy(szClan, _szClan);
		nTeam = _nTeam;
		bDeath = _bDeath;
		nExp = _nExp;
		nKills = _nKills;
		nDeaths = _nDeaths;
		nPing = _nPing;
		bMyChar = _bMyChar;
		bCommander = _bCommander;
		bSpColor = false;
		SpColor = MCOLOR(0, 0, 0);
		bGameRoomUser = _bGameRoomUser;
		nDTLastWeekGrade = _nDTLastWeekGrade;
	}
	ZScoreBoardItem() {
		bSpColor = false;
		SpColor = MCOLOR(0, 0, 0);
	}

	void SetColor(MCOLOR c) {
		SpColor = c;
		bSpColor = true;
	}

	MCOLOR GetColor() {
		return SpColor;
	}
};

ZCombatInterface::ZCombatInterface(const char* szName, MWidget* pParent, MListener* pListener)
	: ZInterface(szName, pParent, pListener)
{
	m_fElapsed = 0;

	m_nBulletSpare = 0;
	m_nBulletCurrMagazine = 0;
	m_nMagazine = 0;
	memset(m_szItemName, 0, sizeof(m_szItemName));

	m_pIDLResource = ZApplication::GetGameInterface()->GetIDLResource();

	m_bMenuVisible = false;

	m_bPickTarget = false;
	m_pLastItemDesc = NULL;

	m_bReserveFinish = false;

	m_pTargetLabel = NULL;
	m_szTargetName[0] = 0;

	m_nBulletImageIndex = 0;
	m_nMagazineImageIndex = 0;

	m_nReserveFinishTime = 0;

	m_pWeaponScreenEffect = NULL;

	m_pResultPanel = NULL;
	m_pResultPanel_Team = NULL;
	m_pResultLeft = NULL;
	m_pResultRight = NULL;

	m_pQuestScreen = NULL;

	m_nClanIDRed = 0;
	m_nClanIDBlue = 0;
	m_szRedClanName[0] = 0;
	m_szBlueClanName[0] = 0;

	m_bNetworkAlive = true;
	m_dLastTimeTick = 0;
	m_dAbuseHandicapTick = 0;

	m_bSkipUIDrawByRule = false;
}

ZCombatInterface::~ZCombatInterface()
{
	OnDestroy();
}

bool ZCombatInterface::OnCreate()
{
	ZGetGame()->m_pMyCharacter->EnableAccumulationDamage(false);

	m_Observer.Create(ZApplication::GetGameInterface()->GetCamera(),
		ZApplication::GetGameInterface()->GetIDLResource());

	m_pTargetLabel = new MLabel("", this, this);
	m_pTargetLabel->SetTextColor(0xffff0000);
	m_pTargetLabel->SetSize(100, 30);

	ShowInfo(true);

	m_pResultBgImg = NULL;
	m_bDrawScoreBoard = false;

	EnableInputChat(false);

	m_Chat.Create("CombatChatOutput", true);
	m_Chat.ShowOutput(ZGetConfiguration()->GetViewGameChat());
	m_Chat.m_pChattingOutput->ReleaseFocus();

	m_AdminMsg.Create("CombatChatOutputAdmin", false);
	MFont* pFont = MFontManager::Get("FONTb11b");
	m_AdminMsg.SetFont(pFont);
	m_AdminMsg.m_pChattingOutput->ReleaseFocus();

	if (ZGetMyInfo()->IsAdminGrade()) {
		MMatchObjCache* pCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
		if (pCache && pCache->GetUGrade() == MMUG_EVENTMASTER && pCache->CheckFlag(MTD_PlayerFlags_AdminHide)) {
			ShowChatOutput(false);
		}
	}

	m_ppIcons[0] = MBitmapManager::Get("medal_A.tga");
	m_ppIcons[1] = MBitmapManager::Get("medal_U.tga");
	m_ppIcons[2] = MBitmapManager::Get("medal_E.tga");
	m_ppIcons[3] = MBitmapManager::Get("medal_F.tga");
	m_ppIcons[4] = MBitmapManager::Get("medal_H.tga");

	m_CrossHair.Create();
	m_CrossHair.ChangeFromOption();

	m_pWeaponScreenEffect = new ZWeaponScreenEffect;
	m_pWeaponScreenEffect->Create();

	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGame()->GetMatch()->GetMatchType()))
	{
		m_pQuestScreen = new ZCombatQuestScreen();
	}

	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatTDMInfo");
	if (pWidget)
	{
		if (ZGetGameTypeManager()->IsTeamExtremeGame(ZGetGame()->GetMatch()->GetMatchType()))
		{
			int nMargin[BMNUM_NUMOFCHARSET] = { 13,9,13,13,13,13,13,13,13,13,8,10,8 };

			ZBmNumLabel* pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("TDM_Score_Blue");
			if (pBmNumLabel)
			{
				pBmNumLabel->SetAlignmentMode(MAM_HCENTER);
				pBmNumLabel->SetCharMargin(nMargin);
				pBmNumLabel->SetNumber(0);
			}

			pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("TDM_Score_Red");
			if (pBmNumLabel)
			{
				pBmNumLabel->SetAlignmentMode(MAM_HCENTER);
				pBmNumLabel->SetIndexOffset(16);
				pBmNumLabel->SetCharMargin(nMargin);
				pBmNumLabel->SetNumber(0);
			}

			pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("TDM_Score_Max");
			if (pBmNumLabel)
			{
				pBmNumLabel->SetAlignmentMode(MAM_HCENTER);
				pBmNumLabel->SetIndexOffset(32);
				int nMargin2[BMNUM_NUMOFCHARSET] = { 18,12,18,18,18,18,18,18,18,18,18,18,18 };
				pBmNumLabel->SetCharMargin(nMargin2);
				pBmNumLabel->SetNumber(0);
			}

			pWidget->Show(true);

			MWidget* pPicture = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("TDM_RedWin");
			if (pPicture)
				pPicture->Show(true);
			pPicture = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("TDM_BlueWin");
			if (pPicture)
				pPicture->Show(true);
		}
		else
		{
			pWidget->Show(false);

			MWidget* pPicture = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("TDM_RedWin");
			if (pPicture)
				pPicture->Show(false);
			pPicture = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("TDM_BlueWin");
			if (pPicture)
				pPicture->Show(false);
		}
	}

	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
	{
		MWidget* pPicture = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("TDM_Score_TimeBack");
		if (pPicture)
			pPicture->Show(true);

		ZBmNumLabel* pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("DT_RemainTime");

		if (pBmNumLabel)
		{
			pBmNumLabel->SetAlignmentMode(MAM_HCENTER);
			pBmNumLabel->SetIndexOffset(32);
			int nMargin[BMNUM_NUMOFCHARSET] = { 18,12,18,18,18,18,18,18,18,18,18,18,18 };
			pBmNumLabel->SetCharMargin(nMargin);
			pBmNumLabel->SetNumber(0);
		}

		pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatDTInfo");
		if (pWidget)
			pWidget->Show(true);

		MWidget* pFrame = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatDT_CharacterInfo");
		if (pFrame)
		{
			int numChild = pFrame->GetChildCount();
			MWidget* pChild = NULL;
			for (int i = 0; i < numChild; ++i)
			{
				pChild = pFrame->GetChild(i);
				if (pChild &&
					strcmp(pChild->GetClassName(), MINT_LABEL) == 0 &&
					strstr(pChild->m_szIDLName, "CombatDT_PlayerInfo_"))
				{
					if (strstr(pChild->m_szIDLName, "Left"))
						((MLabel*)pChild)->SetAlignment(MAM_RIGHT | MAM_VCENTER);
					else if (strstr(pChild->m_szIDLName, "Right"))
						((MLabel*)pChild)->SetAlignment(MAM_LEFT | MAM_VCENTER);
					else
						((MLabel*)pChild)->SetAlignment(MAM_HCENTER | MAM_VCENTER);
				}
			}
		}
		MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatDT_MatchLevel");
		if (pLabel)
			pLabel->SetAlignment(MAM_HCENTER | MAM_VCENTER);

		GetWidgetCharViewLeft()->SetEnableRotateZoom(false, false);
		GetWidgetCharViewRight()->SetEnableRotateZoom(false, false);
		GetWidgetCharViewResult()->SetEnableRotateZoom(false, false);

		ZGetGame()->m_pMyCharacter->EnableAccumulationDamage(true);
	}

	MButton* pExitConfirmButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageExit");
	if (pExitConfirmButton) {
		char szConfirmMsg[256];
		if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
		{
			ZTransMsg(szConfirmMsg, MSG_GAME_DUELTOURNAMENT_MATCH_EXITSTAGE_CONFIRM);
			pExitConfirmButton->SetAlterableConfirmMessage(szConfirmMsg);
		}
		else
		{
			pExitConfirmButton->RestoreIDLConfirmMessage();
		}
	}

	ZGetSoundEngine()->Set3DSoundUpdate(true);

	m_bOnFinish = false;
	m_bShowResult = false;
	m_bIsShowUI = true;

	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Option");
	if (pWidget)
		pWidget->Show(false);

	return true;
}

void ZCombatInterface::OnDestroy()
{
	if (m_nClanIDBlue) {
		ZGetEmblemInterface()->DeleteClanInfo(m_nClanIDBlue);
		m_nClanIDBlue = 0;
	}
	if (m_nClanIDRed) {
		ZGetEmblemInterface()->DeleteClanInfo(m_nClanIDRed);
		m_nClanIDRed = 0;
	}

	if (m_pQuestScreen) { delete m_pQuestScreen; m_pQuestScreen = NULL; }

	m_Observer.Destroy();

	m_ResultItems.Destroy();
	SAFE_DELETE(m_pResultPanel);
	SAFE_DELETE(m_pResultPanel_Team);
	SAFE_DELETE(m_pResultLeft);
	SAFE_DELETE(m_pResultRight);

	EnableInputChat(false);

	m_Chat.Destroy();
	m_AdminMsg.Destroy();

	m_CrossHair.Destroy();

	if (m_pTargetLabel)
	{
		delete m_pTargetLabel;
		m_pTargetLabel = NULL;
	}
	ShowInfo(false);

#ifdef _BIRDSOUND

#else
	ZGetSoundEngine()->Set3DSoundUpdate(false);
#endif

	MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("GameResult_Background");
	if (pPicture)
		pPicture->SetBitmap(NULL);

	if (m_pResultBgImg != NULL)
	{
		delete m_pResultBgImg;
		m_pResultBgImg = NULL;
	}

	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatTDMInfo");
	if (pWidget)
		pWidget->Show(false);

	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatDTInfo");
	if (pWidget)
		pWidget->Show(false);

	ZCharacterView* pCharView = GetWidgetCharViewLeft();
	if (pCharView)
		pCharView->SetCharacter(MUID(0, 0));

	pCharView = GetWidgetCharViewRight();
	if (pCharView)
		pCharView->SetCharacter(MUID(0, 0));

	pCharView = GetWidgetCharViewResult();
	if (pCharView)
		pCharView->SetCharacter(MUID(0, 0));

	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatDT_CharacterInfo");
	if (pWidget)
		pWidget->Show(false);

	// Custom: Fixed calling destructor of ZWeaponScreenEffect
	if (m_pWeaponScreenEffect)
		m_pWeaponScreenEffect->Destroy();

	SAFE_DELETE(m_pWeaponScreenEffect);

	ZScoreBoardItem::Release();
}

void TextRelative(MDrawContext* pDC, float x, float y, const char* szText, bool bCenter)
{
	if (RGetIsWidthScreen())
		x = (x * 800 + 80) / 960.f;

	int screenx = x * MGetWorkspaceWidth();
	if (bCenter)
	{
		MFont* pFont = pDC->GetFont();
		screenx -= pFont->GetWidth(szText) / 2;
	}

	pDC->Text(screenx, y * MGetWorkspaceHeight(), szText);
}

void BitmapRelative(MDrawContext* pDC, float x, float y, float w, float h, MBitmap* pBitmap, bool bCenter = false)
{
	pDC->SetBitmap(pBitmap);

	if (RGetIsWidthScreen())
		x = (x * 800 + 80) / 960.f;

	int screenx = x * MGetWorkspaceWidth();
	if (bCenter)
	{
		MFont* pFont = pDC->GetFont();
		screenx -= w / 2;
	}

	pDC->Draw(screenx, y * MGetWorkspaceHeight(), w, h);
}

void MatchOrderRelative(MDrawContext* pDC, float x, float y, float fHalfGrid, int nMatchCount, int nCouple, bool bBlink)
{
	float screenx = x * MGetWorkspaceWidth();
	float screeny = y * MGetWorkspaceHeight();

	float fRectX = screenx - 2;
	float fRectY = screeny - 4;
	float fRectWidth = 0.138f * MGetWorkspaceWidth();
	float fRectHeight = 0.04f * MGetWorkspaceHeight();
	pDC->Rectangle(fRectX, fRectY, fRectWidth, fRectHeight);

	if (bBlink)
		return;
	float fLineWidth = 0.032f * MGetWorkspaceWidth();
	float fLineHeight = 0.038f * MGetWorkspaceHeight();

	MDUELTOURNAMENTTYPE eDTType = ZApplication::GetGameInterface()->GetDuelTournamentType();
	switch (eDTType)
	{
	case MDUELTOURNAMENTTYPE_FINAL:
	{
		fLineWidth = 0.145f * MGetWorkspaceWidth();
		if (nCouple != 1)
			pDC->HLine(fRectX + fRectWidth, fRectY + fRectHeight / 2, fLineWidth);
		return;
	}
	break;
	case MDUELTOURNAMENTTYPE_SEMIFINAL:
	{
		fLineWidth = 0.082f * MGetWorkspaceWidth();
		fLineHeight = 0.049f * MGetWorkspaceHeight();
	}
	break;
	case MDUELTOURNAMENTTYPE_QUATERFINAL:
	{
		fLineWidth = 0.032f * MGetWorkspaceWidth();
		fLineHeight = 0.038f * MGetWorkspaceHeight();
	}
	break;
	}

	fLineHeight *= nMatchCount + 0.9f;
	float fHalf = fHalfGrid * MGetWorkspaceHeight();
	if (nCouple)
	{
		fLineHeight = -fLineHeight;
		fHalf = -fHalf;
	}
	pDC->HLine(fRectX + fRectWidth, fRectY + fRectHeight / 2, fLineWidth);
	pDC->VLine(fRectX + fRectWidth + fLineWidth, fRectY + fRectHeight / 2, fLineHeight);
	if (nMatchCount != eDTType + 1)
		pDC->HLine(fRectX + fRectWidth + fLineWidth, fRectY + fRectHeight / 2 + fHalf, fLineWidth);
}

void ZCombatInterface::DrawNPCName(MDrawContext* pDC)
{
	for (ZObjectManager::iterator itor = ZGetObjectManager()->begin();
		itor != ZGetObjectManager()->end(); ++itor)
	{
		rvector pos, screen_pos;
		ZObject* pObject = (*itor).second;
		if (!pObject->IsVisible()) continue;
		if (pObject->IsDie()) continue;
		if (!pObject->IsNPC()) continue;

		ZActor* pActor = (ZActor*)pObject;

		pos = pObject->GetPosition();
		RVisualMesh* pVMesh = pObject->m_pVMesh;
		RealSpace2::rboundingbox box;

		if (pVMesh == NULL) continue;

		box.vmax = pos + rvector(50.f, 50.f, 190.f);
		box.vmin = pos + rvector(-50.f, -50.f, 0.f);

		if (isInViewFrustum(&box, RGetViewFrustum()))
		{
			screen_pos = RGetTransformCoord(pObject->GetPosition() + rvector(0, 0, 100.f));

			MFont* pFont = NULL;
			pFont = pActor->CheckFlag(AF_MY_CONTROL) ? MFontManager::Get("FONTa12_O1Blr") : MFontManager::Get("FONTa12_O1Red");
			pDC->SetColor(MCOLOR(0xFF00FF00));
			pDC->SetBitmap(NULL);
			pDC->SetFont(pFont);

			int x = screen_pos.x - pDC->GetFont()->GetWidth(pActor->m_szOwner) / 2;
			pDC->Text(x, screen_pos.y - 12, pActor->m_szOwner);

			float fElapsedTime = ZGetGame()->GetTime() - pActor->m_fLastBasicInfo;
			if (!pActor->CheckFlag(AF_MY_CONTROL) && fElapsedTime > .2f) {
				int y = screen_pos.y;
				y += pFont->GetHeight();
				char temp[256];
				sprintf(temp, "%2.2f", fElapsedTime);
				x = screen_pos.x - pDC->GetFont()->GetWidth(temp) / 2;
				pDC->Text(x, y - 12, temp);
			}
		}
	}
}

void ZCombatInterface::DrawHPAPNPC(MDrawContext* pDC)
{
	for (ZObjectManager::iterator itor = ZGetObjectManager()->begin();
		itor != ZGetObjectManager()->end(); ++itor)
	{
		rvector pos, screen_pos;
		ZObject* pObject = (*itor).second;
		ZActor* pActor = (ZActor*)pObject;
		if (!pObject->IsVisible()) continue;
		if (pObject->IsDie()) continue;
		if (!pObject->IsNPC()) continue;

		pos = pObject->GetPosition();
		RVisualMesh* pVMesh = pObject->m_pVMesh;
		RealSpace2::rboundingbox box;

		if (pVMesh == NULL) continue;

		box.vmax = pos + rvector(50.f, 50.f, 190.f);
		box.vmin = pos + rvector(-50.f, -50.f, 0.f);

		if (isInViewFrustum(&box, RGetViewFrustum()))
		{
			screen_pos = RGetTransformCoord(pVMesh->GetHeadPosition() + rvector(0, 0, 35.f));

			MFont* pFont = NULL;
			pFont = pActor->CheckFlag(AF_MY_CONTROL) ? MFontManager::Get("FONTa12_O1Blr") : MFontManager::Get("FONTa12_O1Red");
			pDC->SetColor(MCOLOR(0xFFFF0000));
			pDC->SetBitmap(NULL);
			pDC->SetFont(pFont);

			int x = screen_pos.x - pDC->GetFont()->GetWidth(pActor->m_szOwner) / 2;

			MCOLOR color;
			const int BAR_HEIGHT = 4;
			const int BAR_WIDTH = 70;

			int nMax = pActor->GetActualMaxHP();
			int nCurr = min(pActor->GetActualHP(), nMax);

			int nHP = nCurr;
			if (nHP == 0 && !pActor->IsDie())
				nHP += 1;
			int bar_hp_width = (int)(BAR_WIDTH * ((int)nHP / nMax));
			color = MCOLOR(0xFF0091FF);
			int screeny = screen_pos.y - 50;
			int screenx = x;
			int bar_y = screeny + pFont->GetHeight() + 2;
			pDC->SetColor(color);
			pDC->FillRectangle(screenx, bar_y, bar_hp_width, BAR_HEIGHT);
			pDC->Rectangle(screenx, bar_y, BAR_WIDTH, BAR_HEIGHT);
		}
	}
}

void ZCombatInterface::DrawTDMScore(MDrawContext* pDC)
{
	int nBlueKills = ZGetGame()->GetMatch()->GetTeamKills(MMT_BLUE);
	int nRedKills = ZGetGame()->GetMatch()->GetTeamKills(MMT_RED);
	int nTargetKills = ZGetGameClient()->GetMatchStageSetting()->GetRoundMax();

	ZBmNumLabel* pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("TDM_Score_Blue");
	if (pBmNumLabel)
		pBmNumLabel->SetNumber(nBlueKills);

	pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("TDM_Score_Red");
	if (pBmNumLabel)
		pBmNumLabel->SetNumber(nRedKills);

	pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("TDM_Score_Max");
	pBmNumLabel->SetNumber(nTargetKills);

	MWidget* pRed = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("TDM_RedWin");
	MWidget* pBlue = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("TDM_BlueWin");

	if (pRed && pBlue)
	{
		int nTime[] = { 1, 1400, 1400, 900, 900, 200 };
		int nDiff = min(abs(nBlueKills - nRedKills), 5);
		int nCurrTime = timeGetTime() % nTime[nDiff];

		if (nDiff == 0)
		{
			pRed->Show(false);
			pBlue->Show(false);

			return;
		}
		if ((nDiff == 1) || (nDiff == 2) || (nDiff >= 5))
		{
			if (nCurrTime > 100)
			{
				pRed->Show(false);
				pBlue->Show(false);

				return;
			}
		}
		else if ((nDiff == 3) || (nDiff == 4))
		{
			if (((nCurrTime > 100) && (nCurrTime < 200)) || (nCurrTime > 300))
			{
				pRed->Show(false);
				pBlue->Show(false);

				return;
			}
		}

		if (nRedKills > nBlueKills)
		{
			pRed->Show(true);
			pBlue->Show(false);
		}
		else if (nRedKills < nBlueKills)
		{
			pRed->Show(false);
			pBlue->Show(true);
		}
		else
		{
			pRed->Show(false);
			pBlue->Show(false);
		}
	}
}

void ZCombatInterface::UpdateNetworkAlive(MDrawContext* pDC)
{
	DWORD dw;
	BOOL bIsNetworkAlive = IsNetworkAlive(&dw);
	if (!bIsNetworkAlive)
	{
		m_dAbuseHandicapTick = timeGetTime() + 2000;
		m_bNetworkAlive = false;
	}
	if (bIsNetworkAlive && timeGetTime() > m_dAbuseHandicapTick)
	{
		m_bNetworkAlive = true;
	}
	return;
}

void ZCombatInterface::OnDraw(MDrawContext* pDC)
{
#ifdef LOCALE_KOREA
	if (timeGetTime() - m_dLastTimeTick > 500)
	{
		UpdateNetworkAlive(pDC);
		m_dLastTimeTick = timeGetTime();
	}
#endif

	if (m_bShowResult)
		return;

	bool bDrawAllPlayerName = false;

	if (ZGetGame()->m_pMyCharacter->IsAdminHide()
		&& MEvent::GetAltState() && ZGetCamera()->GetLookMode() != ZCAMERA_MINIMAP)
		bDrawAllPlayerName = true;

	if (ZGetCamera()->GetLookMode() == ZCAMERA_FREELOOK || bDrawAllPlayerName)
		DrawAllPlayerName(pDC);
	else
	{
		if (!ZGetGameInterface()->IsMiniMapEnable())
		{
			DrawFriendName(pDC);
			DrawEnemyName(pDC);
		}
	}

	GetVoteInterface()->DrawVoteTargetlist(pDC);
	GetVoteInterface()->DrawVoteMessage(pDC);

	ZGetScreenEffectManager()->Draw();

	if (IsShowUI())
	{
		m_Chat.OnDraw(pDC);

		if (!m_bSkipUIDrawByRule)
		{
			m_pWeaponScreenEffect->Draw(pDC);

			ZGetScreenEffectManager()->DrawMyHPPanal(pDC);
			ZGetScreenEffectManager()->DrawMyWeaponImage();
			ZGetScreenEffectManager()->DrawMyBuffImage();

			if (m_pQuestScreen) m_pQuestScreen->OnDraw(pDC);

			DrawSoloSpawnTimeMessage(pDC);

			DrawPont(pDC);
			DrawMyNamePont(pDC);
			DrawMyWeaponPont(pDC);
			DrawHPAPNPC(pDC);

			if (ZGetGameInterface()->GetCamera()->GetLookMode() == ZCAMERA_DEFAULT)
				m_CrossHair.Draw(pDC);
		}

		DrawBuffStatus(pDC);
		DrawScore(pDC);

		if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
		{
			((ZRuleDuelTournament*)ZGetGame()->GetMatch()->GetRule())->OnDraw(pDC);
		}
	}

	if (ZGetGame())
	{
		ZGetGame()->m_HelpScreen.DrawHelpScreen();
	}

	if (ZGetGameInterface()->GetBandiCapturer() != NULL)
		ZGetGameInterface()->GetBandiCapturer()->DrawCapture(pDC);
}

void ZCombatInterface::DrawMyNamePont(MDrawContext* pDC)
{
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
		return;

	ZCharacter* pCharacter = GetTargetCharacter();
	if (pCharacter == NULL) return;
	if (m_Observer.IsVisible())
		return;

	MFont* pFont = GetGameFont();

	pDC->SetFont(pFont);
	pDC->SetColor(MCOLOR(0xFFFFFFFF));

	char buffer[256];

	sprintf(buffer, "%d  %s", pCharacter->GetProperty()->nLevel, pCharacter->GetProperty()->GetName());

	if ((ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL) || (!pCharacter->IsObserverTarget()))
	{
		float fCenterVert = 0.018f - (float)pFont->GetHeight() / (float)RGetScreenHeight() / 2;
		TextRelative(pDC, 100.f / 800.f, fCenterVert, buffer);
	}
}
void ZCombatInterface::DrawMyWeaponPont(MDrawContext* pDC)
{
	ZCharacter* pCharacter = GetTargetCharacter();
	if (pCharacter == NULL) return;
	if (m_Observer.IsVisible()) return;

	MFont* pFont = GetGameFont();

	pDC->SetFont(pFont);
	pDC->SetColor(MCOLOR(0xFFFFFFFF));

	char buffer[256];

	TextRelative(pDC, 660.f / 800.f, 510.f / 600.f, m_szItemName);

	MMatchCharItemParts nParts = pCharacter->GetItems()->GetSelectedWeaponParts();
	if (nParts != MMCIP_MELEE && nParts < MMCIP_END)
	{
		sprintf(buffer, "%d / %d", m_nBulletCurrMagazine, m_nBulletSpare);
		TextRelative(pDC, 720.f / 800.f, 585.f / 600.f, buffer);
	}
}

void ZCombatInterface::DrawPont(MDrawContext* pDC)
{
	ZCharacter* pCharacter = GetTargetCharacter();
	if (pCharacter == NULL) return;

	if (m_Observer.IsVisible())
		return;

	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
	{
		char	charName[3][32];
		charName[0][0] = charName[1][0] = charName[2][0] = 0;
		float fRx = (float)MGetWorkspaceWidth() / 800.0f;
		float fRy = (float)MGetWorkspaceHeight() / 600.0f;

		MFont* pFont = MFontManager::Get("FONTa10_O2Wht");
		if (pFont == NULL)
			pDC->SetFont(pFont);
		pDC->SetColor(MCOLOR(0xFFFFFFFF));

		bool bIsChallengerDie = false;
		int nMyChar = -1;

		ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
		if (pDuel)
		{
			for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin(); itor != ZGetCharacterManager()->end(); ++itor)
			{
				ZCharacter* pCharacter = (*itor).second;

				if (pCharacter->GetUID() == pDuel->QInfo.m_uidChampion)
				{
					if (ZGetMyUID() == pDuel->QInfo.m_uidChampion)
					{
						ZGetCombatInterface()->DrawVictory(pDC, 210, 86, pDuel->QInfo.m_nVictory);
					}
					else
					{
						sprintf(charName[0], "%s%d  %s", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetProperty()->nLevel, pCharacter->GetUserName());

						if ((ZGetMyUID() == pDuel->QInfo.m_uidChampion) || (ZGetMyUID() == pDuel->QInfo.m_uidChallenger))
						{
							int nTextWidth = pFont->GetWidth(charName[0]);
							int nWidth = ZGetCombatInterface()->DrawVictory(pDC, 162, 300, pDuel->QInfo.m_nVictory, true);
							ZGetCombatInterface()->DrawVictory(pDC, 43 + nTextWidth + nWidth, 157, pDuel->QInfo.m_nVictory);
						}
					}
				}

				else if (pCharacter->GetUID() == pDuel->QInfo.m_uidChallenger)
				{
					if (ZGetMyUID() != pDuel->QInfo.m_uidChallenger)
						sprintf(charName[0], "%s%d  %s", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetProperty()->nLevel, pCharacter->GetUserName());

					bIsChallengerDie = pCharacter->IsDie();
				}

				else if (pCharacter->GetUID() == pDuel->QInfo.m_WaitQueue[0])
					sprintf(charName[1], "%s%d  %s", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetProperty()->nLevel, pCharacter->GetUserName());

				else if (pCharacter->GetUID() == pDuel->QInfo.m_WaitQueue[1])
					sprintf(charName[2], "%s%d  %s", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetProperty()->nLevel, pCharacter->GetUserName());
			}
		}

		MBitmap* pBitmap = MBitmapManager::Get("duel-mode.tga");
		if (pBitmap)
		{
			pDC->SetBitmap(pBitmap);

			int nIcon = 50.0f * fRx;
			pDC->Draw(8.0f * fRx, 153.0f * fRy, nIcon, nIcon);
		}

		pBitmap = MBitmapManager::Get("icon_play.tga");
		if (pBitmap && (charName[1][0] != 0))
		{
			pDC->SetBitmap(pBitmap);

			int nIcon = 22.0f * fRx;
			pDC->Draw(60.0f * fRx, 175.0f * fRy, nIcon, nIcon);
			pDC->Draw(53.0f * fRx, 175.0f * fRy, nIcon, nIcon);
		}

		int nTime = timeGetTime() % 200;
		if (nTime < 100)
			pDC->SetColor(MCOLOR(0xFFFFFF00));
		else
			pDC->SetColor(MCOLOR(0xFFA0A0A0));

		if (bIsChallengerDie)
			pDC->SetColor(MCOLOR(0xFF808080));

		int nPosY = 160.0f * fRy;
		pDC->Text(60.0f * fRx, nPosY, charName[0]);

		pDC->SetColor(MCOLOR(0xFF808080));
		nPosY += 20;
		pDC->Text(80.0f * fRx, nPosY, charName[1]);
		nPosY += 15;
	}
}

void ZCombatInterface::DrawScore(MDrawContext* pDC)
{
	m_bDrawScoreBoard = false;
	if (ZIsActionKeyPressed(ZACTION_SCORE) == true) {
		if (m_Chat.IsShow() == false)
			m_bDrawScoreBoard = true;
	}
	else if (ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PREPARE) {
		int cur_round = ZGetGame()->GetMatch()->GetCurrRound();
		if (cur_round == 0) {
			m_bDrawScoreBoard = true;
		}
	}

	if (ZGetGameTypeManager()->IsTeamExtremeGame(ZGetGame()->GetMatch()->GetMatchType()))
	{
		DrawTDMScore(pDC);
	}

	if (m_bDrawScoreBoard) {
		if (ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUELTOURNAMENT)
			DrawScoreBoard(pDC);
		else
			DrawDuelTournamentScoreBoard(pDC);
	}
}

void ZCombatInterface::DrawBuffStatus(MDrawContext* pDC)
{
}

void ZCombatInterface::DrawFinish()
{
	if (!m_bShowResult && IsFinish())
	{
		float fVolume;
		DWORD dwClock = timeGetTime() - m_nReserveFinishTime;
		if (dwClock > 4000)
			fVolume = 0.0f;
		else
			fVolume = (float)(4000 - dwClock) / 4000.0f * m_fOrgMusicVolume;

		ZApplication::GetSoundEngine()->SetMusicVolume(fVolume);

		if (timeGetTime() >= m_nReservedOutTime)
		{
			MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Option");
			if (pWidget)
				pWidget->Show(false);
			pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatMenuFrame");
			if (pWidget)
				pWidget->Show(false);
			MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatChatInput");
			if (pLabel)
				pLabel->SetText("");
			ZGetCombatInterface()->EnableInputChat(false);

			if (ZGetGameTypeManager()->IsQuestOnly(ZGetGame()->GetMatch()->GetMatchType()))
			{
				if (!ZGetQuest()->IsQuestComplete())
				{
					ZChangeGameState(GUNZ_STAGE);
					m_bShowResult = true;

					return;
				}
			}

			GetResultInfo();

			pWidget = ZGetGameInterface()->GetIDLResource()->FindWidget("GameResult");
			if (pWidget)
				pWidget->Show(true, true);

			ZApplication::GetSoundEngine()->SetMusicVolume(m_fOrgMusicVolume);
			ZApplication::GetSoundEngine()->OpenMusic(BGMID_FIN, ZApplication::GetFileSystem());
			ZApplication::GetSoundEngine()->PlayMusic(false);
			m_nReservedOutTime = timeGetTime() + 15000;
			m_bShowResult = true;
		}
	}
}

int ZCombatInterface::DrawVictory(MDrawContext* pDC, int x, int y, int nWinCount, bool bGetWidth)
{
	if (bGetWidth)
	{
		int nWidth = 0;

		int nNum = nWinCount % 5;
		if (nNum)
			nWidth += 17.0f + 17.0f * 0.63f * (nNum - 1);

		if ((nWinCount % 10) >= 5)
			nWidth += 19.0f * 0.2f + 19.0f * 1.1f;
		else
			nWidth += 19.0f * 0.5f;

		nNum = nWinCount / 10;
		if (nNum)
			nWidth += 22.0f + 22.0f * 0.5f * (nNum - 1);

		return nWidth;
	}

	MBitmap* pBitmap = MBitmapManager::Get("killstone.tga");
	if (!pBitmap)
		return 0;

	pDC->SetBitmap(pBitmap);

	float fRx = (float)MGetWorkspaceWidth() / 800.0f;
	float fRy = (float)MGetWorkspaceHeight() / 600.0f;

	int nImage = ((timeGetTime() / 100) % 20);
	if (nImage > 10)
		nImage = 0;
	nImage *= 32;
	nImage = ((timeGetTime() / 100) % 20);
	if (nImage > 10)
		nImage = 0;
	nImage *= 32;

	int nPosX = x * fRx;
	int nPosY = y * fRy;
	int nSize = 17.0f * fRx;
	for (int i = 0; i < (nWinCount % 5); i++)
	{
		pDC->Draw(nPosX, nPosY, nSize, nSize, nImage, 0, 32, 32);
		nPosX -= nSize * 0.63f;
	}

	nSize = 19.0f * fRx;
	nPosY = (y - 2) * fRy;
	if ((nWinCount % 10) >= 5)
	{
		nPosX -= nSize * 0.2f;
		pDC->Draw(nPosX, nPosY, nSize, nSize, nImage, 64, 32, 32);
		nPosX -= nSize * 1.1f;
	}
	else
		nPosX -= nSize * 0.5f;

	nSize = 22.0f * fRx;
	nPosY = (y - 5) * fRy;
	for (int i = 0; i < (nWinCount / 10); i++)
	{
		pDC->Draw(nPosX, nPosY, nSize, nSize, nImage, 32, 32, 32);
		nPosX -= nSize * 0.5f;
	}

	return 0;
}

void ZCombatInterface::OnDrawCustom(MDrawContext* pDC)
{
	if (m_bShowResult)
	{
		if (ZGetGameTypeManager()->IsQuestOnly(ZGetGame()->GetMatch()->GetMatchType()))
		{
			int nNumCount = (timeGetTime() - (m_nReservedOutTime - 15000)) * 3.6418424f;
			ZBmNumLabel* pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("QuestResult_GetPlusXP");
			if (pBmNumLabel)
			{
				if (nNumCount < ZGetQuest()->GetRewardXP())
					pBmNumLabel->SetNumber(nNumCount, false);
				else
					pBmNumLabel->SetNumber(ZGetQuest()->GetRewardXP(), false);
			}
			pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("QuestResult_GetMinusXP");
			if (pBmNumLabel)
				pBmNumLabel->SetNumber(0, false);
			pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("QuestResult_GetTotalXP");
			if (pBmNumLabel)
			{
				if (nNumCount < ZGetQuest()->GetRewardXP())
					pBmNumLabel->SetNumber(nNumCount, false);
				else
					pBmNumLabel->SetNumber(ZGetQuest()->GetRewardXP(), false);
			}
			pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("QuestResult_GetBounty");
			if (pBmNumLabel)
			{
				if (nNumCount < ZGetQuest()->GetRewardBP())
					pBmNumLabel->SetNumber(nNumCount, false);
				else
					pBmNumLabel->SetNumber(ZGetQuest()->GetRewardBP(), false);
			}
		}
		else if (ZGetGameTypeManager()->IsSurvivalOnly(ZGetGame()->GetMatch()->GetMatchType()))
		{
			int nNumCount = (timeGetTime() - (m_nReservedOutTime - 15000)) * 3.6418424f;

			ZBmNumLabel* pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SurvivalResult_GetReachedRound");
			if (pBmNumLabel)
				pBmNumLabel->SetNumber(static_cast<ZSurvival*>(ZGetQuest())->GetReachedRound(), false);

			pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SurvivalResult_GetPoint");
			if (pBmNumLabel)
			{
				if (nNumCount < ZGetQuest()->GetRewardXP())
					pBmNumLabel->SetNumber(nNumCount, false);
				else
					pBmNumLabel->SetNumber(static_cast<ZSurvival*>(ZGetQuest())->GetPoint(), false);
			}

			pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SurvivalResult_GetXP");
			if (pBmNumLabel)
			{
				if (nNumCount < ZGetQuest()->GetRewardXP())
					pBmNumLabel->SetNumber(nNumCount, false);
				else
					pBmNumLabel->SetNumber(ZGetQuest()->GetRewardXP(), false);
			}

			pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SurvivalResult_GetBounty");
			if (pBmNumLabel)
			{
				if (nNumCount < ZGetQuest()->GetRewardBP())
					pBmNumLabel->SetNumber(nNumCount, false);
				else
					pBmNumLabel->SetNumber(ZGetQuest()->GetRewardBP(), false);
			}
		}

		MLabel* pLabel = (MLabel*)ZGetGameInterface()->GetIDLResource()->FindWidget("GameResult_RemaindTime");
		if (pLabel)
		{
			char szRemaindTime[100];
			sprintf(szRemaindTime, "%d", (m_nReservedOutTime - timeGetTime()) / 1000);
			char szText[100];
			ZTransMsg(szText, MSG_GAME_EXIT_N_MIN_AFTER, 1, szRemaindTime);

			pLabel->SetAlignment(MAM_HCENTER);
			pLabel->SetText(szText);
		}

		return;
	}

	if (m_Observer.IsVisible())
	{
		if (!ZGetGameInterface()->IsMiniMapEnable())
		{
			if (!ZGetGame()->IsReplay() || ZGetGame()->IsShowReplayInfo())
				ZGetScreenEffectManager()->DrawSpectator();
		}

		m_Observer.OnDraw(pDC);
	}

	if (m_bDrawLeaveBattle)
		DrawLeaveBattleTimeMessage(pDC);
}

void ZCombatInterface::DrawSoloSpawnTimeMessage(MDrawContext* pDC)
{
	if (ZGetGame()->m_pMyCharacter->IsAdminHide()) return;

	ZMatch* pMatch = ZGetGame()->GetMatch();
	if (pMatch->GetRoundState() == MMATCH_ROUNDSTATE_PLAY)
	{
		if (!pMatch->IsWaitForRoundEnd())
		{
			if (ZGetGame()->m_pMyCharacter && ZGetGame()->m_pMyCharacter->IsDie())
			{
				char szMsg[128] = "";
				int nRemainTime = pMatch->GetRemainedSpawnTime();
				if ((nRemainTime > 0) && (nRemainTime <= 5))
				{
					char temp[4];
					sprintf(temp, "%d", nRemainTime);
					ZTransMsg(szMsg, MSG_GAME_WAIT_N_MIN, 1, temp);
				}
				else if ((nRemainTime == 0) && (!ZGetGame()->GetSpawnRequested()))
				{
					sprintf(szMsg, ZMsg(MSG_GAME_CLICK_FIRE));
				}

				MFont* pFont = GetGameFont();
				pDC->SetFont(pFont);
				pDC->SetColor(MCOLOR(0xFFFFFFFF));
				TextRelative(pDC, 400.f / 800.f, 400.f / 600.f, szMsg, true);
			}
		}
	}
}

void ZCombatInterface::DrawLeaveBattleTimeMessage(MDrawContext* pDC)
{
	char szMsg[128] = "";
	char temp[4];
	sprintf(temp, "%d", m_nDrawLeaveBattleSeconds);
	ZTransMsg(szMsg, MSG_GAME_EXIT_N_MIN_AFTER, 1, temp);

	MFont* pFont = GetGameFont();
	pDC->SetFont(pFont);
	pDC->SetColor(MCOLOR(0xFFFFFFFF));
	TextRelative(pDC, 400.f / 800.f, 350.f / 600.f, szMsg, true);
}

bool ZCombatInterface::IsDone()
{
	return false;
}

bool ZCombatInterface::OnEvent(MEvent* pEvent, MListener* pListener)
{
	return false;
}

void ZCombatInterface::Resize(int w, int h)
{
	SetSize(w, h);
}

void ZCombatInterface::ShowMenu(bool bVisible)
{
	if (m_bMenuVisible == bVisible) return;

	m_bMenuVisible = bVisible;
}

void ZCombatInterface::EnableInputChat(bool bInput, bool bTeamChat)
{
	m_Chat.EnableInput(bInput, bTeamChat);
}

void ZCombatInterface::OutputChatMsg(const char* szMsg)
{
	m_Chat.OutputChatMsg(szMsg);
}

void ZCombatInterface::OutputChatMsg(MCOLOR color, const char* szMsg)
{
	m_Chat.OutputChatMsg(color, szMsg);
}

void ZCombatInterface::SetItemName(const char* szName)
{
	if (!strcmp(m_szItemName, szName)) return;

	strcpy(m_szItemName, szName);
}

void ZCombatInterface::ShowInfo(bool bVisible)
{
	MWidget* pWidget;
	char szTemp[256];
	for (int i = 0; i < 9; i++)
	{
		sprintf(szTemp, "%s%d", ZIITEM_COMBAT_INFO, i);
		pWidget = m_pIDLResource->FindWidget(szTemp);
		if (pWidget != NULL)
		{
			pWidget->Show(bVisible);
		}
	}
	pWidget = m_pIDLResource->FindWidget(ZIITEM_COMBAT_CHATFRAME);
	if (pWidget != NULL)
	{
		pWidget->Show(bVisible);
	}
}

void ZCombatInterface::Update(float fElapsed)
{
	DrawFinish();

	if (m_bShowResult)
	{
		if (timeGetTime() > m_nReservedOutTime)
		{
			if (ZGetGameClient()->IsLadderGame() || ZGetGameClient()->IsDuelTournamentGame())
				ZChangeGameState(GUNZ_LOBBY);
			else
				ZChangeGameState(GUNZ_STAGE);
		}
	}

	m_fElapsed = fElapsed;

	if (m_bReserveFinish) {
		if ((timeGetTime() - m_nReserveFinishTime) > 1000) {
			OnFinish();
			m_bReserveFinish = false;
		}
	}

	ZCharacter* pCharacter = GetTargetCharacter();
	if (pCharacter == NULL)				return;
	if (!pCharacter->GetInitialized())	return;
	if (ZGetScreenEffectManager() == NULL) return;
	if (pCharacter->GetProperty() == NULL) return;

	float fGauge = 100.f;
	float fCur, fMax;
	if (ZGetGame() && ZGetGame()->GetMatch())
	{
		fMax = (float)pCharacter->GetMaxHP();
		fCur = (float)pCharacter->GetHP();

		if (fCur != 0.f && fMax != 0.f)	fGauge = fCur / fMax;
		else							fGauge = 0.f;

		if (ZGetGame()->GetMatch()->GetCurrRound() == 0 &&
			ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PREPARE)
			fGauge = 100.f;

		ZGetScreenEffectManager()->SetGauge_HP(fGauge);

		fMax = (float)pCharacter->GetMaxAP();
		fCur = (float)pCharacter->GetAP();

		if (fCur != 0.f && fMax != 0.f)	fGauge = fCur / fMax;
		else							fGauge = 0.f;

		ZGetScreenEffectManager()->SetGauge_AP(fGauge);
	}
	else
	{
		ZGetScreenEffectManager()->SetGauge_HP(fGauge);
		ZGetScreenEffectManager()->SetGauge_AP(fGauge);
	}

	MMatchWeaponType wtype = MWT_NONE;

	ZItem* pSItem = pCharacter->GetItems()->GetSelectedWeapon();

	MMatchItemDesc* pSelectedItemDesc = NULL;

	if (pSItem) {
		pSelectedItemDesc = pSItem->GetDesc();

		m_nBulletSpare = pSItem->GetBulletSpare();
		m_nBulletCurrMagazine = pSItem->GetBulletCurrMagazine();
	}

	if (pSelectedItemDesc) {
		wtype = pSelectedItemDesc->m_nWeaponType.Ref();
	}

	ZGetScreenEffectManager()->SetWeapon(wtype, pSelectedItemDesc);

	if ((pSelectedItemDesc) && (m_pLastItemDesc != pSelectedItemDesc)) {
		SetItemName(pSelectedItemDesc->m_pMItemName->Ref().m_szItemName);
	}

	UpdateCombo(pCharacter);

	m_Chat.Update();
	m_AdminMsg.Update();

	if (pCharacter->GetItems()->GetSelectedWeaponParts() == MMCIP_MELEE) {
		ShowCrossHair(false);
	}
	else {
		ShowCrossHair(true);
	}

	GameCheckPickCharacter();
}

void ZCombatInterface::SetPickTarget(bool bPick, ZCharacter* pCharacter)
{
	bool bFriend = false;
	if (bPick)
	{
		if (pCharacter == NULL) return;

		if (ZGetGame()->GetMatch()->IsTeamPlay())
		{
			ZCharacter* pTargetCharacter = GetTargetCharacter();
			if (pTargetCharacter && pTargetCharacter->GetTeamID() == pCharacter->GetTeamID())
			{
				bFriend = true;
			}
		}

		if (bFriend == false)
		{
			m_pTargetLabel->SetTextColor(0xffff0000);
		}

		if (pCharacter->IsAdminName())
		{
			MCOLOR gmColor;
			char szEmpty[4];
			memset(szEmpty, 0, sizeof(szEmpty));

			if (ZGetGame()->GetUserGradeIDColor(pCharacter->GetUserGrade(), gmColor))
				m_pTargetLabel->SetTextColor(gmColor);
			else
				m_pTargetLabel->SetTextColor(ZCOLOR_ADMIN_NAME);
		}
		if (!bFriend == true && !pCharacter->IsDie())
		{
			strcpy(m_szTargetName, pCharacter->GetUserName());
			m_pTargetLabel->SetText(m_szTargetName);
		}

		int nCrosshairHeight = m_CrossHair.GetHeight();

		int nLen = m_pTargetLabel->GetRect().w;
		m_pTargetLabel->SetPosition(((MGetWorkspaceWidth() - m_pTargetLabel->GetRect().w) / 2), (MGetWorkspaceHeight() / 2) - nCrosshairHeight);
		m_pTargetLabel->SetAlignment(MAM_HCENTER);
	}
	else
	{
		m_CrossHair.SetState(ZCS_NORMAL);
		memset(m_szTargetName, 0, sizeof(m_szTargetName));
		m_pTargetLabel->Show(false);
	}

	m_bPickTarget = bPick;
}

void ZCombatInterface::SetItemImageIndex(int nIndex)
{
	char szTemp[256];
	sprintf(szTemp, "item%02d.png", nIndex);
	BEGIN_WIDGETLIST("CombatItemPic", ZApplication::GetGameInterface()->GetIDLResource(),
		MPicture*, pPicture);

	pPicture->SetBitmap(MBitmapManager::Get(szTemp));

	END_WIDGETLIST();
}

void ZCombatInterface::UpdateCombo(ZCharacter* pCharacter)
{
	if (pCharacter == NULL) return;

	static int nComboX = -999, nComboY = -999;
	static int nLastCombo = 0;

	int nCurCombo = pCharacter->GetStatus().Ref().nCombo;

	if (nCurCombo != nLastCombo)
	{
		nLastCombo = nCurCombo;
		ZGetScreenEffectManager()->SetCombo(nLastCombo);
	}
	else if (nCurCombo != 0)
	{
	}
}

void ZCombatInterface::DrawFriendName(MDrawContext* pDC)
{
	if (ZGetGame()->m_pMyCharacter == NULL) return;

	if (ZGetGame()->GetMatch()->IsTeamPlay())
	{
		ZCharacter* pTargetCharacter = GetTargetCharacter();
		if (pTargetCharacter == NULL) return;

		for (ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.begin();
			itor != ZGetGame()->m_CharacterManager.end(); ++itor)
		{
			rvector pos, screen_pos;
			ZCharacter* pCharacter = (*itor).second;
			if (!pCharacter->IsVisible()) continue;
			if (pCharacter->IsDie()) continue;
			if (pCharacter->GetTeamID() != pTargetCharacter->GetTeamID()) continue;
			if (pCharacter == pTargetCharacter) continue;

			pos = pCharacter->GetPosition();
			RVisualMesh* pVMesh = pCharacter->m_pVMesh;
			RealSpace2::rboundingbox box;

			if (pVMesh == NULL) continue;

			box.vmax = pos + rvector(50.f, 50.f, 190.f);
			box.vmin = pos + rvector(-50.f, -50.f, 0.f);

			if (isInViewFrustum(&box, RGetViewFrustum()))
			{
				screen_pos = RGetTransformCoord(pCharacter->GetVisualMesh()->GetHeadPosition() + rvector(0, 0, 30.f));

				MFont* pFont = NULL;

				if (pCharacter->IsAdminName()) {
					pFont = MFontManager::Get("FONTa12_O1Org");
					pDC->SetColor(MCOLOR(ZCOLOR_ADMIN_NAME));
				}
				else {
					pFont = MFontManager::Get("FONTa12_O1Blr");
					pDC->SetColor(MCOLOR(0xFF00FF00));
				}

				pDC->SetBitmap(NULL);

				pDC->SetFont(pFont);
				int x = screen_pos.x - pDC->GetFont()->GetWidth(pCharacter->GetUserName()) / 2;

				pDC->Text(x, screen_pos.y - 12, pCharacter->GetUserName());
			}
		}
	}
}

void ZCombatInterface::DrawEnemyName(MDrawContext* pDC)
{
	MPOINT Cp = GetCrosshairPoint();

	ZPICKINFO pickinfo;

	rvector pos, dir;
	if (!RGetScreenLine(Cp.x, Cp.y, &pos, &dir))
		return;

	ZCharacter* pTargetCharacter = GetTargetCharacter();

	if (ZGetGame()->Pick(pTargetCharacter, pos, dir, &pickinfo))
	{
		if (pickinfo.pObject) {
			if (!IsPlayerObject(pickinfo.pObject)) return;
			if (pickinfo.pObject->IsDie()) return;

			ZCharacter* pPickedCharacter = (ZCharacter*)pickinfo.pObject;

			bool bFriend = false;
			if (ZGetGame()->GetMatch()->IsTeamPlay()) {
				if (pTargetCharacter && pPickedCharacter->GetTeamID() == pTargetCharacter->GetTeamID())
					bFriend = true;
			}

			if (bFriend == false) {
				MFont* pFont = NULL;

				if (pPickedCharacter->IsAdminName()) {
					pDC->SetColor(MCOLOR(ZCOLOR_ADMIN_NAME));
					pFont = MFontManager::Get("FONTa12_O1Org");
				}
				else {
					pFont = MFontManager::Get("FONTa12_O1Red");
				}

				pDC->SetFont(pFont);

				int x = Cp.x - pDC->GetFont()->GetWidth(pPickedCharacter->GetUserName()) / 2;
				pDC->Text(x, Cp.y - pDC->GetFont()->GetHeight() - 10, pPickedCharacter->GetUserName());
			}
		}
	}
}

void ZCombatInterface::DrawAllPlayerName(MDrawContext* pDC)
{
	for (ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.begin();
		itor != ZGetGame()->m_CharacterManager.end(); ++itor)
	{
		rvector pos, screen_pos;
		ZCharacter* pCharacter = (*itor).second;
		if (!pCharacter->IsVisible()) continue;
		if (pCharacter->IsDie()) continue;

		pos = pCharacter->GetPosition();
		RVisualMesh* pVMesh = pCharacter->m_pVMesh;
		RealSpace2::rboundingbox box;

		if (pVMesh == NULL) continue;

		box.vmax = pos + rvector(50.f, 50.f, 190.f);
		box.vmin = pos + rvector(-50.f, -50.f, 0.f);

		if (isInViewFrustum(&box, RGetViewFrustum()))
		{
			if (ZGetCamera()->GetLookMode() == ZCAMERA_MINIMAP) {
				rvector pos = pCharacter->GetPosition();
				pos.z = 0;
				screen_pos = RGetTransformCoord(pos);
			}
			else
				screen_pos = RGetTransformCoord(pCharacter->GetVisualMesh()->GetHeadPosition() + rvector(0, 0, 30.f));

			MFont* pFont = NULL;

			if (pCharacter->IsAdminName()) {
				pFont = MFontManager::Get("FONTa12_O1Org");
				pDC->SetColor(MCOLOR(ZCOLOR_ADMIN_NAME));
			}
			else {
				if (pCharacter->GetTeamID() == MMT_RED)
					pFont = MFontManager::Get("FONTa12_O1Red");
				else if (pCharacter->GetTeamID() == MMT_BLUE)
					pFont = MFontManager::Get("FONTa12_O1Blr");
				else
					pFont = MFontManager::Get("FONTa12_O1Blr");

				pDC->SetColor(MCOLOR(0xFF00FF00));
			}

			pDC->SetBitmap(NULL);

			pDC->SetFont(pFont);
			int x = screen_pos.x - pDC->GetFont()->GetWidth(pCharacter->GetUserName()) / 2;

			pDC->Text(x, screen_pos.y - 12, pCharacter->GetUserName());
		}
	}
}

MFont* ZCombatInterface::GetGameFont()
{
	MFont* pFont = MFontManager::Get("FONTa10_O2Wht");
	return pFont;
}

bool CompareZScoreBoardItem(ZScoreBoardItem* a, ZScoreBoardItem* b) {
	if (a->nDuelQueueIdx < b->nDuelQueueIdx) return true;
	if (a->nDuelQueueIdx > b->nDuelQueueIdx) return false;

	if (a->nTeam < b->nTeam) return true;
	if (a->nTeam > b->nTeam) return false;

	if (a->nExp > b->nExp) return true;
	if (a->nExp < b->nExp) return false;

	if (a->nKills > b->nKills) return true;
	if (a->nKills < b->nKills) return false;
	return false;
}

void ZCombatInterface::DrawDuelTournamentScoreBoard(MDrawContext* pDC)
{
	ZGetScreenEffectManager()->DrawScoreBoard();

	MFont* pFont = GetGameFont();
	pDC->SetFont(pFont);
	pFont = pDC->GetFont();
	pDC->SetColor(MCOLOR(TEXT_COLOR_TITLE));

	char szText[256];

	sprintf(szText, "(%03d) %s", ZGetGameClient()->GetStageNumber(), ZGetGameClient()->GetStageName());
	TextRelative(pDC, 0.26f, 0.22f, szText);

	float x = 0.27f;
	float y = 0.284f;
	float linespace2 = 0.071f / 3.f;

	strcpy(szText, ZGetGameClient()->GetMatchStageSetting()->GetMapName());
	TextRelative(pDC, x, y, szText);

	y -= linespace2;
	sprintf(szText, "%s", ZGetGameTypeManager()->GetGameTypeStr(ZGetGame()->GetMatch()->GetMatchType()));
	TextRelative(pDC, x, y, szText);

	x = 0.70f;
	y = 0.284f;

	DrawPlayTime(pDC, x, y);
	y -= linespace2;

	((ZRuleDuelTournament*)ZGetGame()->GetMatch()->GetRule())->ShowMatchOrder(pDC, false, m_fElapsed);
}

void ZCombatInterface::DrawPlayTime(MDrawContext* pDC, float xPos, float yPos)
{
	char szText[256];
	if (ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PLAY)
	{
		DWORD dwTime = ZGetGame()->GetMatch()->GetRemaindTime();
		DWORD dwLimitTime = ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->nLimitTime;
		if (dwLimitTime != 99999)
		{
			dwLimitTime *= 60000;
			if (dwTime <= dwLimitTime)
			{
				dwTime = (dwLimitTime - dwTime) / 1000;
				sprintf(szText, "%s : %d:%02d", ZMsg(MSG_WORD_REMAINTIME), (dwTime / 60), (dwTime % 60));
			}
			else
				sprintf(szText, "%s :", ZMsg(MSG_WORD_REMAINTIME));
		}
		else
			sprintf(szText, "%s : ---", ZMsg(MSG_WORD_REMAINTIME));
	}
	else
		sprintf(szText, "%s : ---", ZMsg(MSG_WORD_REMAINTIME));

	TextRelative(pDC, xPos, yPos, szText);
}

int ZCombatInterface::GetPlayTime()
{
	if (ZGetGame()->GetMatch()->GetRoundState() != MMATCH_ROUNDSTATE_PLAY)
		return -1;

	DWORD dwLimitTime = ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->nLimitTime;
	if (dwLimitTime == 99999 || dwLimitTime == 0)
		return -2;

	dwLimitTime *= 60000;
	DWORD dwTime = ZGetGame()->GetMatch()->GetRemaindTime();
	if (dwTime > dwLimitTime)
		return -1;

	return (dwLimitTime - dwTime) / 1000;
}

typedef list<ZScoreBoardItem*> ZSCOREBOARDITEMLIST;
void ZCombatInterface::DrawScoreBoard(MDrawContext* pDC)
{
	bool bClanGame = ZGetGameClient()->IsLadderGame();

	ZSCOREBOARDITEMLIST items;

	ZGetScreenEffectManager()->DrawScoreBoard();

	MFont* pFont = GetGameFont();
	pDC->SetFont(pFont);
	pFont = pDC->GetFont();
	pDC->SetColor(MCOLOR(TEXT_COLOR_TITLE));

	char szText[256];

	if (bClanGame)
	{
		int nRed = 0, nBlue = 0;

		for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin();
			itor != ZGetCharacterManager()->end(); ++itor)
		{
			ZCharacter* pCharacter = (*itor).second;

			if (pCharacter->GetTeamID() == MMT_BLUE) nBlue++;
			if (pCharacter->GetTeamID() == MMT_RED) nRed++;
		}

		char nvsn[32];
		sprintf(nvsn, "%d:%d", nRed, nBlue);
		ZTransMsg(szText, MSG_GAME_SCORESCREEN_STAGENAME, 3, nvsn, m_szRedClanName, m_szBlueClanName);
	}
	else
	{
		sprintf(szText, "(%03d) %s", ZGetGameClient()->GetStageNumber(), ZGetGameClient()->GetStageName());
	}
	TextRelative(pDC, 0.26f, 0.22f, szText);

	float x = 0.27f;
	float y = 0.284f;
	float linespace2 = 0.071f / 3.f;

	if (ZGetGame()->GetMatch()->IsTeamPlay())
	{
		if (bClanGame)
		{
			sprintf(szText, "%d (%s)  VS  %d (%s)",
				ZGetGame()->GetMatch()->GetTeamScore(MMT_RED), m_szRedClanName,
				ZGetGame()->GetMatch()->GetTeamScore(MMT_BLUE), m_szBlueClanName);
		}
		else
		{
			if (ZGetGameTypeManager()->IsTeamExtremeGame(ZGetGame()->GetMatch()->GetMatchType()))
				sprintf(szText, "%s : %d(Red) vs %d(Blue)", ZGetGameTypeManager()->GetGameTypeStr(ZGetGame()->GetMatch()->GetMatchType()),
					ZGetGame()->GetMatch()->GetTeamKills(MMT_RED),
					ZGetGame()->GetMatch()->GetTeamKills(MMT_BLUE));
			else
				sprintf(szText, "%s : %d(Red) vs %d(Blue)", ZGetGameTypeManager()->GetGameTypeStr(ZGetGame()->GetMatch()->GetMatchType()),
					ZGetGame()->GetMatch()->GetTeamScore(MMT_RED),
					ZGetGame()->GetMatch()->GetTeamScore(MMT_BLUE));
		}
	}
	else
		sprintf(szText, "%s", ZGetGameTypeManager()->GetGameTypeStr(ZGetGame()->GetMatch()->GetMatchType()));
	TextRelative(pDC, x, y, szText);
	y -= linespace2;

	strcpy(szText, ZGetGameClient()->GetMatchStageSetting()->GetMapName());
	if (ZGetGameTypeManager()->IsQuestOnly(ZGetGame()->GetMatch()->GetMatchType()))
	{
		sprintf(szText, "%s (%s %d)", szText, ZMsg(MSG_CHARINFO_LEVELMARKER), ZGetQuest()->GetGameInfo()->GetQuestLevel());
	}
	TextRelative(pDC, x, y, szText);

	x = 0.70f;
	y = 0.284f;

	if (ZGetGameTypeManager()->IsQuestOnly(ZGetGame()->GetMatch()->GetMatchType()))
	{
		sprintf(szText, "%s : %d", ZMsg(MSG_WORD_GETITEMQTY), ZGetQuest()->GetGameInfo()->GetNumOfObtainQuestItem());
		TextRelative(pDC, x, y, szText);
		y -= linespace2;
	}

	if (ZGetGameTypeManager()->IsQuestOnly(ZGetGame()->GetMatch()->GetMatchType()))
	{
		int nNPCKilled = ZGetQuest()->GetGameInfo()->GetNPCCount() - ZGetQuest()->GetGameInfo()->GetNPCKilled();
		if (nNPCKilled < 0)
			nNPCKilled = 0;

		MUID uidBoss = ZGetQuest()->GetGameInfo()->GetBoss();

		if (uidBoss != MUID(0, 0))
			sprintf(szText, "%s : -", ZMsg(MSG_WORD_REMAINNPC));
		else
			sprintf(szText, "%s : %d", ZMsg(MSG_WORD_REMAINNPC), nNPCKilled);
		TextRelative(pDC, x, y, szText);
		y -= linespace2;
	}
	else if (ZGetGameTypeManager()->IsSurvivalOnly(ZGetGame()->GetMatch()->GetMatchType()))
	{
		int nNPCKilled = ZGetQuest()->GetGameInfo()->GetNPCCount() - ZGetQuest()->GetGameInfo()->GetNPCKilled();
		if (nNPCKilled < 0)
			nNPCKilled = 0;

		MUID uidBoss = ZGetQuest()->GetGameInfo()->GetBoss();

		sprintf(szText, "%s : %d", ZMsg(MSG_WORD_REMAINNPC), nNPCKilled);
		TextRelative(pDC, x, y, szText);
		y -= linespace2;
	}

	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
	{
		if (ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PLAY)
		{
			DWORD dwTime = ZGetGame()->GetMatch()->GetRemaindTime();
			DWORD dwLimitTime = ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->nLimitTime;
			if (dwLimitTime != 99999)
			{
				dwLimitTime *= 60000;
				if (dwTime <= dwLimitTime)
				{
					dwTime = (dwLimitTime - dwTime) / 1000;
					sprintf(szText, "%s : %d:%02d", ZMsg(MSG_WORD_REMAINTIME), (dwTime / 60), (dwTime % 60));
				}
				else
					sprintf(szText, "%s :", ZMsg(MSG_WORD_REMAINTIME));
			}
			else
				sprintf(szText, "%s : ---", ZMsg(MSG_WORD_REMAINTIME));
		}
		else
			sprintf(szText, "%s : ---", ZMsg(MSG_WORD_REMAINTIME));

		TextRelative(pDC, x, y, szText);
		y -= linespace2;

		sprintf(szText, "%s : %d", ZMsg(MSG_WORD_ENDKILL), ZGetGame()->GetMatch()->GetRoundCount());
	}

	else if (ZGetGame()->GetMatch()->IsWaitForRoundEnd() && !bClanGame)
	{
		if (ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PLAY)
		{
			DWORD dwTime = ZGetGame()->GetMatch()->GetRemaindTime();
			DWORD dwLimitTime = ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->nLimitTime;
			if (dwLimitTime != 99999)
			{
				dwLimitTime *= 60000;
				if (dwTime <= dwLimitTime)
				{
					dwTime = (dwLimitTime - dwTime) / 1000;
					sprintf(szText, "%s : %d:%02d", ZMsg(MSG_WORD_REMAINTIME), (dwTime / 60), (dwTime % 60));
				}
				else
					sprintf(szText, "%s :", ZMsg(MSG_WORD_REMAINTIME));
			}
			else
				sprintf(szText, "%s : ---", ZMsg(MSG_WORD_REMAINTIME));
		}
		else
			sprintf(szText, "%s : ---", ZMsg(MSG_WORD_REMAINTIME));

		TextRelative(pDC, x, y, szText);
		y -= linespace2;

		sprintf(szText, "%s : %d / %d %s", ZMsg(MSG_WORD_RPROGRESS), ZGetGame()->GetMatch()->GetCurrRound() + 1, ZGetGame()->GetMatch()->GetRoundCount(), ZMsg(MSG_WORD_ROUND));
	}
	else if (ZGetGameTypeManager()->IsQuestOnly(ZGetGame()->GetMatch()->GetMatchType()))
	{
		sprintf(szText, "%s : %d / %d", ZMsg(MSG_WORD_RPROGRESS), ZGetQuest()->GetGameInfo()->GetCurrSectorIndex() + 1, ZGetQuest()->GetGameInfo()->GetMapSectorCount());
	}
	else if (ZGetGameTypeManager()->IsSurvivalOnly(ZGetGame()->GetMatch()->GetMatchType()))
	{
		int currSector = ZGetQuest()->GetGameInfo()->GetCurrSectorIndex() + 1;
		int sectorCount = ZGetQuest()->GetGameInfo()->GetMapSectorCount();
		int repeatCount = ZGetQuest()->GetGameInfo()->GetRepeatCount();

		currSector += ZGetQuest()->GetGameInfo()->GetCurrRepeatIndex() * sectorCount;
		sectorCount *= repeatCount;
		sprintf(szText, "%s : %d / %d", ZMsg(MSG_WORD_RPROGRESS), currSector, sectorCount);
	}

	else if (!bClanGame)
	{
		if (ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PLAY)
		{
			DWORD dwTime = ZGetGame()->GetMatch()->GetRemaindTime();
			DWORD dwLimitTime = ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->nLimitTime;
			if (dwLimitTime != 99999)
			{
				dwLimitTime *= 60000;
				if (dwTime <= dwLimitTime)
				{
					dwTime = (dwLimitTime - dwTime) / 1000;
					sprintf(szText, "%s : %d:%02d", ZMsg(MSG_WORD_REMAINTIME), (dwTime / 60), (dwTime % 60));
				}
				else
					sprintf(szText, "%s :", ZMsg(MSG_WORD_REMAINTIME));
			}
			else
				sprintf(szText, "%s : ---", ZMsg(MSG_WORD_REMAINTIME));
		}
		else
			sprintf(szText, "%s : ---", ZMsg(MSG_WORD_REMAINTIME));

		TextRelative(pDC, x, y, szText);
		y -= linespace2;

		if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_CTF)

			sprintf(szText, "%s : %d", "Captures", ZGetGame()->GetMatch()->GetRoundCount());
		else
			sprintf(szText, "%s : %d", ZMsg(MSG_WORD_ENDKILL), ZGetGame()->GetMatch()->GetRoundCount());
	}
	TextRelative(pDC, x, y, szText);

	const float normalXPOS[] = { 0.26f, 0.47f, 0.67f, 0.78f, 0.84f, 0.93f, 0.334f, 0.311f };
	const float clanXPOS[] = { 0.44f, 0.24f, 0.67f, 0.76f, 0.82f, 0.91f, 0.514f, 0.491f };

	const float* ITEM_XPOS = bClanGame ? clanXPOS : normalXPOS;

	y = 0.343f;
	const float fHeight = 0.578f;

	char szBuff[25];
	pDC->SetColor(MCOLOR(TEXT_COLOR_TITLE));
	x = ITEM_XPOS[0];
	sprintf(szBuff, "%s", ZMsg(MSG_CHARINFO_LEVEL));
	TextRelative(pDC, x, y, szBuff);
	x = ITEM_XPOS[6];
	sprintf(szBuff, "%s", ZMsg(MSG_CHARINFO_NAME));
	TextRelative(pDC, x, y, szBuff);
	x = ITEM_XPOS[1] + .02f;;
	TextRelative(pDC, x, y, ZMsg(MSG_CHARINFO_CLAN));
	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGame()->GetMatch()->GetMatchType()))
	{
		x = ITEM_XPOS[2];
		sprintf(szBuff, "%s/%s", ZMsg(MSG_CHARINFO_HP), ZMsg(MSG_CHARINFO_AP));
		TextRelative(pDC, x, y, szBuff);
	}
	else
	{
		x = ITEM_XPOS[2] - .01f;
		TextRelative(pDC, x, y, ZMsg(MSG_WORD_EXP));
	}
	x = ITEM_XPOS[3] - .01f;
	TextRelative(pDC, x, y, ZMsg(MSG_WORD_KILL));
	x = ITEM_XPOS[4] - .01f;
	TextRelative(pDC, x, y, ZMsg(MSG_WORD_DEATH));
	x = ITEM_XPOS[5] - .01f;
	TextRelative(pDC, x, y, ZMsg(MSG_WORD_PING));

	float fTitleHeight = (float)pFont->GetHeight() * 1.1f / (float)RGetScreenHeight();
	y += fTitleHeight;

	int nMaxLineCount = 16;

	float linespace = (fHeight - fTitleHeight) / (float)nMaxLineCount;

	if (bClanGame)
	{
		for (int i = 0; i < 2; i++)
		{
			MMatchTeam nTeam = (i == 0) ? MMT_RED : MMT_BLUE;
			char* szClanName = (i == 0) ? m_szRedClanName : m_szBlueClanName;
			int nClanID = (i == 0) ? m_nClanIDRed : m_nClanIDBlue;

			MFont* pClanFont = MFontManager::Get("FONTb11b");
			pDC->SetFont(pClanFont);
			pDC->SetColor(MCOLOR(TEXT_COLOR_CLAN_NAME));

			float clancenter = .5f * (ITEM_XPOS[0] - ITEM_XPOS[1]) + ITEM_XPOS[1];
			float clanx = clancenter - .5f * ((float)pClanFont->GetWidth(szClanName) / (float)MGetWorkspaceWidth());
			float clany = y + linespace * ((nTeam == MMT_RED) ? .5f : 8.5f);

			MBitmap* pbmp = ZGetEmblemInterface()->GetClanEmblem(nClanID);
#ifdef TEST_CLAN_SCOREBOARD
			pbmp = MBitmapManager::Get("btntxtr_gnd_on.png");
#endif
			if (pbmp) {
				pDC->SetBitmap(pbmp);

				const float fIconSize = .1f;
				int nIconSize = fIconSize * MGetWorkspaceWidth();

				int screenx = (clancenter - .5f * fIconSize) * MGetWorkspaceWidth();
				int screeny = (clany)*MGetWorkspaceHeight();

				pDC->Draw(screenx, screeny, nIconSize, nIconSize);

				clany += fIconSize + 1.2 * linespace;
			}

			TextRelative(pDC, clanx, clany, szClanName);

			sprintf(szText, "%d", ZGetGame()->GetMatch()->GetTeamScore(nTeam));
			clanx = clancenter - .5f * ((float)pClanFont->GetWidth(szText) / (float)MGetWorkspaceWidth());
			clany += 1.f * linespace;
			TextRelative(pDC, clanx, clany, szText);
		}
	}

	ZCharacterManager::iterator itor;
	for (itor = ZGetCharacterManager()->begin();
		itor != ZGetCharacterManager()->end(); ++itor)
	{
		ZCharacter* pCharacter = (*itor).second;

		if (pCharacter->GetTeamID() == MMT_SPECTATOR) continue;

		if (pCharacter->IsAdminHide()) continue;

		ZScoreBoardItem* pItem = new ZScoreBoardItem;

		if (pCharacter->IsAdminName()) {
			sprintf(pItem->szLevel, "--%s", ZMsg(MSG_CHARINFO_LEVELMARKER));
			pItem->SetColor(ZCOLOR_ADMIN_NAME);
		}
		else {
			sprintf(pItem->szLevel, "%d%s", pCharacter->GetProperty()->nLevel, ZMsg(MSG_CHARINFO_LEVELMARKER));
		}

		sprintf(pItem->szName, "%s", pCharacter->GetUserName());

		memcpy(pItem->szClan, pCharacter->GetProperty()->GetClanName(), CLAN_NAME_LENGTH);

		pItem->nClanID = pCharacter->GetClanID();
		pItem->nTeam = ZGetGame()->GetMatch()->IsTeamPlay() ? pCharacter->GetTeamID() : MMT_END;
		pItem->bDeath = pCharacter->IsDie();
		if (ZGetGameTypeManager()->IsQuestDerived(ZGetGame()->GetMatch()->GetMatchType()))
			pItem->nExp = pCharacter->GetStatus().Ref().nKills * 100;
		else
			pItem->nExp = pCharacter->GetStatus().Ref().nExp;
		pItem->nKills = pCharacter->GetStatus().Ref().nKills;
		pItem->nDeaths = pCharacter->GetStatus().Ref().nDeaths;
		pItem->uidUID = pCharacter->GetUID();

		int nPing = (pCharacter->GetUID() == ZGetGameClient()->GetPlayerUID() ? 0 : MAX_PING);
		MMatchPeerInfo* pPeer = ZGetGameClient()->FindPeer(pCharacter->GetUID());
		if (pPeer) {
			if (ZGetGame()->IsReplay())
				nPing = 0;
			else
				nPing = pPeer->GetPing(ZGetGame()->GetTickTime());
		}
		pItem->nPing = nPing;
		pItem->bMyChar = pCharacter->IsHero();

		if (ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_SPECTATOR &&
			m_Observer.IsVisible() && m_Observer.GetTargetCharacter() == pCharacter)
			pItem->bMyChar = true;

		if (pCharacter->GetTeamID() == ZGetGame()->m_pMyCharacter->GetTeamID() && pCharacter->m_dwStatusBitPackingValue.Ref().m_bCommander)
			pItem->bCommander = true;
		else
			pItem->bCommander = false;

		if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_BERSERKER)
		{
			if (pCharacter->IsTagger()) pItem->bCommander = true;
		}

		if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
		{
			ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGame()->GetMatch()->GetRule();
			pItem->nDuelQueueIdx = pDuel->GetQueueIdx(pCharacter->GetUID());
		}
		else
			pItem->nDuelQueueIdx = 0;

		MMatchObjCache* pCache = ZGetGameClient()->FindObjCache(pCharacter->GetUID());
		if (pCache)
			pItem->bGameRoomUser = (pCache->GetPGrade() == MMPG_PREMIUM_IP) ? true : false;
		else
			pItem->bGameRoomUser = false;

		pItem->nDTLastWeekGrade = pCharacter->GetDTLastWeekGrade();

		items.push_back(pItem);
	}

	items.sort(CompareZScoreBoardItem);
	ZSCOREBOARDITEMLIST::iterator i;

	int nCurrentTeamIndex;
	if (ZGetGame()->GetMatch()->IsTeamPlay())
		nCurrentTeamIndex = MMT_RED;
	else
	{
		if (items.size() > 0)
			nCurrentTeamIndex = (*items.begin())->nTeam;
	}

	int nCount = 0;

	for (i = items.begin(); i != items.end(); i++)
	{
		ZScoreBoardItem* pItem = *i;

		if (nCurrentTeamIndex != pItem->nTeam)
		{
			nCurrentTeamIndex = pItem->nTeam;
			nCount = max(nCount, min(8, nMaxLineCount - ((int)items.size() - nCount)));
		}

		float itemy = y + linespace * nCount;

		nCount++;

		if (nCount > nMaxLineCount) break;

		MCOLOR backgroundcolor;
		if (pItem->bMyChar) {
			backgroundcolor = BACKGROUND_COLOR_MYCHAR_DEATH_MATCH;
			if (!bClanGame) {
				backgroundcolor =
					(pItem->nTeam == MMT_RED) ? BACKGROUND_COLOR_MYCHAR_RED_TEAM :
					(pItem->nTeam == MMT_BLUE) ? BACKGROUND_COLOR_MYCHAR_BLUE_TEAM :
					BACKGROUND_COLOR_MYCHAR_DEATH_MATCH;
			}
		}

		if (pItem->bCommander) backgroundcolor = BACKGROUND_COLOR_COMMANDER;

		if (pItem->bMyChar || pItem->bCommander)
		{
			int y1 = itemy * MGetWorkspaceHeight();
			int y2 = (y + linespace * nCount) * MGetWorkspaceHeight();

			int x1 = bClanGame ? 0.43 * MGetWorkspaceWidth() : 0.255 * MGetWorkspaceWidth();
			int x2 = (0.715 + 0.26) * MGetWorkspaceWidth();

			pDC->SetColor(backgroundcolor);
			pDC->FillRectangleW(x1, y1, x2 - x1, y2 - y1);
		}

		if (pItem->bGameRoomUser)
		{
			int nIconSize = .8f * linespace * (float)MGetWorkspaceHeight();
			float icony = itemy + (linespace - (float)nIconSize / (float)MGetWorkspaceHeight()) * .5f;
			BitmapRelative(pDC, ITEM_XPOS[0] - 0.043f, icony, nIconSize + 4, nIconSize, MBitmapManager::Get("icon_gameroom_s.tga"));
		}

		{
			int nIconSize = .8f * linespace * (float)MGetWorkspaceHeight();
			float icony = itemy + (linespace - (float)nIconSize / (float)MGetWorkspaceHeight()) * .5f;

			char szDTGradeIconFileName[64];
			GetDuelTournamentGradeIconFileName(szDTGradeIconFileName, pItem->nDTLastWeekGrade);
			MBitmap* pBmpDTGradeIcon = MBitmapManager::Get(szDTGradeIconFileName);

			BitmapRelative(pDC, ITEM_XPOS[7], icony, nIconSize, nIconSize, MBitmapManager::Get(szDTGradeIconFileName));
		}

		MCOLOR textcolor = pItem->bDeath ? TEXT_COLOR_DEATH_MATCH_DEAD : TEXT_COLOR_DEATH_MATCH;

		if (!bClanGame)
		{
			if (pItem->nTeam == MMT_RED)
				textcolor = pItem->bDeath ? TEXT_COLOR_RED_TEAM_DEAD : TEXT_COLOR_RED_TEAM;
			else
				if (pItem->nTeam == MMT_BLUE)
					textcolor = pItem->bDeath ? TEXT_COLOR_BLUE_TEAM_DEAD : TEXT_COLOR_BLUE_TEAM;
				else
					if (pItem->nTeam == MMT_SPECTATOR)
						textcolor = TEXT_COLOR_SPECTATOR;
		}

		if (pItem->bSpColor)
		{
			if (!pItem->bDeath)
				textcolor = pItem->GetColor();
			else
				textcolor = 0xff402010;
		}

		pDC->SetColor(textcolor);
		pDC->SetFont(pFont);

		float texty = itemy + (linespace - (float)pFont->GetHeight() / (float)MGetWorkspaceHeight()) * .5f;
		x = ITEM_XPOS[0];
		TextRelative(pDC, x, texty, pItem->szLevel);

		x = ITEM_XPOS[6];
		TextRelative(pDC, x, texty, pItem->szName);

		if (!bClanGame)
		{
			x = ITEM_XPOS[1];

			int nIconSize = .8f * linespace * (float)MGetWorkspaceHeight();
			float icony = itemy + (linespace - (float)nIconSize / (float)MGetWorkspaceHeight()) * .5f;

			if (pItem->szClan[0]) {
				MBitmap* pbmp = ZGetEmblemInterface()->GetClanEmblem(pItem->nClanID);
				if (pbmp) {
					pDC->SetBitmap(pbmp);
					int screenx = x * MGetWorkspaceWidth();
					int screeny = icony * MGetWorkspaceHeight();

					pDC->Draw(screenx, screeny, nIconSize, nIconSize);
				}
			}
			x += (float)nIconSize / (float)MGetWorkspaceWidth() + 0.005f;
			TextRelative(pDC, x, texty, pItem->szClan);
		}

		if (ZGetGameTypeManager()->IsQuestDerived(ZGetGame()->GetMatch()->GetMatchType()))
		{
			bool bDraw = m_Observer.IsVisible();

			ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.find(pItem->uidUID);
			if (itor != ZGetGame()->m_CharacterManager.end())
			{
				ZCharacter* pQuestPlayerInfo = (*itor).second;

				MCOLOR tmpColor = pDC->GetColor();

				x = ITEM_XPOS[2];

				if (bDraw)
					pDC->SetColor(MCOLOR(0x40FF0000));
				else
					pDC->SetColor(MCOLOR(0x30000000));
				pDC->FillRectangleW((x * MGetWorkspaceWidth()), texty * MGetWorkspaceHeight() + 1, 0.08 * MGetWorkspaceWidth(), 7);

				if (bDraw)
				{
					float nValue = 0.08 * pQuestPlayerInfo->GetHP() / pQuestPlayerInfo->GetMaxHP();
					pDC->SetColor(MCOLOR(0x90FF0000));
					pDC->FillRectangleW((x * MGetWorkspaceWidth()), texty * MGetWorkspaceHeight() + 1, nValue * MGetWorkspaceWidth(), 7);
				}

				if (bDraw)
					pDC->SetColor(MCOLOR(0x4000FF00));
				else
					pDC->SetColor(MCOLOR(0x30000000));
				pDC->FillRectangleW((x * MGetWorkspaceWidth()), texty * MGetWorkspaceHeight() + 9, 0.08 * MGetWorkspaceWidth(), 3);
				if (bDraw)
				{
					float nValue = 0.08 * pQuestPlayerInfo->GetAP() / pQuestPlayerInfo->GetMaxAP();
					pDC->SetColor(MCOLOR(0x9000FF00));
					pDC->FillRectangleW((x * MGetWorkspaceWidth()), texty * MGetWorkspaceHeight() + 9, nValue * MGetWorkspaceWidth(), 3);
				}

				pDC->SetColor(tmpColor);

				x = ITEM_XPOS[3];
				int nKills = 0;
				ZModule_QuestStatus* pMod = (ZModule_QuestStatus*)pQuestPlayerInfo->GetModule(ZMID_QUESTSTATUS);
				if (pMod)
					nKills = pMod->GetKills();
				sprintf(szText, "%d", nKills);
				TextRelative(pDC, x, texty, szText, true);
			}
		}
		else
		{
			x = ITEM_XPOS[2];
			sprintf(szText, "%d", pItem->nExp);
			TextRelative(pDC, x, texty, szText, true);

			MCOLOR color = pDC->GetColor();

			if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
			{
				if (!pItem->bDeath)
					pDC->SetColor(200, 0, 0);
				else
					pDC->SetColor(120, 0, 0);
			}

			x = ITEM_XPOS[3];
			sprintf(szText, "%d", pItem->nKills);
			TextRelative(pDC, x, texty, szText, true);

			pDC->SetColor(color);
		}

		x = ITEM_XPOS[4];
		sprintf(szText, "%d", pItem->nDeaths);
		TextRelative(pDC, x, texty, szText, true);

		x = ITEM_XPOS[5];
		sprintf(szText, "%d", pItem->nPing);
		if (pItem->nPing == 0 || pItem->nPing > 250)
			pDC->SetColor(255, 0, 0);
		else if (pItem->nPing > 1)
			pDC->SetColor(50, 205, 50);
		else if (pItem->nPing > 150)
			pDC->SetColor(250, 250, 210);
		TextRelative(pDC, x, texty, szText, true);
	}

	while (!items.empty())
	{
		delete* items.begin();
		items.erase(items.begin());
	}
}

bool CompareZResultBoardItem(ZResultBoardItem* a, ZResultBoardItem* b) {
	if (a->nScore > b->nScore) return true;
	else if (a->nScore < b->nScore) return false;

	if (a->nKills > b->nKills) return true;
	else if (a->nKills < b->nKills) return false;

	if (a->nDeaths < b->nDeaths) return true;
	else if (a->nDeaths > b->nDeaths) return false;

	return false;
}

void AddCombatResultInfo(const char* szName, int nScore, int nKill, int nDeath, int bMyChar, bool bGameRoomUser)
{
	char szText[128];

	MTextArea* pWidget = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatResult_PlayerNameList");
	if (pWidget)
		pWidget->AddText(szName, (bMyChar ? MCOLOR(0xFFFFF794) : MCOLOR(0xFFFFF794)));

	for (int i = 0; i < 16; i++)
	{
		char szWidget[128];
		sprintf(szWidget, "CombatResult_PlayerScore%d", i);
		MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szWidget);
		if (pLabel)
		{
			if (strcmp(pLabel->GetText(), "") == 0)
			{
				sprintf(szText, "%d", nScore);
				pLabel->SetText(szText);
				pLabel->SetAlignment(MAM_RIGHT);

				sprintf(szWidget, "CombatResult_GameRoomImg%02d", i);
				MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szWidget);
				if (pWidget)
					pWidget->Show(bGameRoomUser);

				break;
			}
		}
	}

	pWidget = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatResult_PlayerKillList");
	if (pWidget)
	{
		sprintf(szText, "%d", nKill);
		pWidget->AddText(szText, MCOLOR(0xFFFFF794));
	}

	pWidget = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatResult_PlayerDeathList");
	if (pWidget)
	{
		sprintf(szText, "%d", nDeath);
		pWidget->AddText(szText, MCOLOR(0xFFFFF794));
	}
}

void AddClanResultInfoWin(const char* szName, int nScore, int nKill, int nDeath, int bMyChar, bool bGameRoomUser)
{
	char szText[125];

	MTextArea* pWidget = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_PlayerNameList1");
	if (pWidget)
		pWidget->AddText(szName, (bMyChar ? MCOLOR(0xFFFFF794) : MCOLOR(0xFFFFF794)));

	for (int i = 0; i < 4; i++)
	{
		char szWidget[128];
		sprintf(szWidget, "ClanResult_PlayerScore1%d", i);
		MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szWidget);
		if (pLabel)
		{
			if (strcmp(pLabel->GetText(), "") == 0)
			{
				sprintf(szText, "%d", nScore);
				pLabel->SetText(szText);
				pLabel->SetAlignment(MAM_RIGHT);

				sprintf(szWidget, "ClanResult_GameRoomImg1%d", i);
				MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szWidget);
				if (pWidget)
					pWidget->Show(bGameRoomUser);

				break;
			}
		}
	}

	pWidget = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_PlayerKillList1");
	if (pWidget)
	{
		sprintf(szText, "%d", nKill);
		pWidget->AddText(szText, MCOLOR(0xFFFFF794));
	}

	pWidget = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_PlayerDeathList1");
	if (pWidget)
	{
		sprintf(szText, "%d", nDeath);
		pWidget->AddText(szText, MCOLOR(0xFFFFF794));
	}
}

void AddClanResultInfoLose(const char* szName, int nScore, int nKill, int nDeath, int bMyChar, bool bGameRoomUser)
{
	char szText[125];

	MTextArea* pWidget = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_PlayerNameList2");
	if (pWidget)
		pWidget->AddText(szName, (bMyChar ? MCOLOR(0xFFFFF794) : MCOLOR(0xFFFFF794)));

	for (int i = 0; i < 4; i++)
	{
		char szWidget[128];
		sprintf(szWidget, "ClanResult_PlayerScore2%d", i);
		MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szWidget);
		if (pLabel)
		{
			if (strcmp(pLabel->GetText(), "") == 0)
			{
				sprintf(szText, "%d", nScore);
				pLabel->SetText(szText);
				pLabel->SetAlignment(MAM_RIGHT);

				sprintf(szWidget, "ClanResult_GameRoomImg2%d", i);
				MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szWidget);
				if (pWidget)
					pWidget->Show(bGameRoomUser);

				break;
			}
		}
	}

	pWidget = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_PlayerKillList2");
	if (pWidget)
	{
		sprintf(szText, "%d", nKill);
		pWidget->AddText(szText, MCOLOR(0xFFFFF794));
	}

	pWidget = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_PlayerDeathList2");
	if (pWidget)
	{
		sprintf(szText, "%d", nDeath);
		pWidget->AddText(szText, MCOLOR(0xFFFFF794));
	}
}

void ZCombatInterface::GetResultInfo(void)
{
	m_ResultItems.sort(CompareZResultBoardItem);

	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatResult");
	if (pWidget)  pWidget->Show(false);
	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult");
	if (pWidget)  pWidget->Show(false);
	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("QuestResult");
	if (pWidget)  pWidget->Show(false);
	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SurvivalResult");
	if (pWidget)  pWidget->Show(false);
	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("DuelTournamentResult");
	if (pWidget)  pWidget->Show(false);

	const int _H18 = CONVERT600(18);
	const int _H2 = CONVERT600(2);
	const int _H21 = CONVERT600(21);
	const int _W17 = CONVERT800(17);
	const int _W21 = CONVERT800(21);
	const int _W20 = CONVERT800(20);

	MTextArea* pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatResult_PlayerNameList");
	if (pTextArea) {
		pTextArea->Clear();
		pTextArea->SetCustomLineHeight(_H18);
	}
	for (int i = 0; i < 16; i++)
	{
		char szWidget[128];
		sprintf(szWidget, "CombatResult_PlayerScore%d", i);
		MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szWidget);
		if (pLabel)
		{
			MRECT rect;
			rect = pLabel->GetRect();
			rect.y = pTextArea->GetRect().y + _H18 * i - _H2;
			rect.h = _H21;
			pLabel->SetBounds(rect);

			pLabel->SetText("");
			pLabel->SetAlignment(MAM_LEFT | MAM_TOP);
		}

		sprintf(szWidget, "CombatResult_GameRoomImg%02d", i);
		MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szWidget);
		if (pWidget)
			pWidget->Show(false);
	}
	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatResult_PlayerKillList");
	if (pTextArea) {
		pTextArea->Clear();
		pTextArea->SetCustomLineHeight(_H18);
	}
	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatResult_PlayerDeathList");
	if (pTextArea) {
		pTextArea->Clear();
		pTextArea->SetCustomLineHeight(_H18);
	}
	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_PlayerNameList1");
	if (pTextArea) {
		pTextArea->Clear();
		pTextArea->SetCustomLineHeight(_H18);
	}
	for (int i = 0; i < 4; i++)
	{
		char szWidget[128];
		sprintf(szWidget, "ClanResult_PlayerScore1%d", i);
		MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szWidget);
		if (pLabel)
		{
			MRECT rect;
			rect = pLabel->GetRect();
			rect.y = pTextArea->GetRect().y + _H18 * i - _H2;
			rect.h = _H21;
			pLabel->SetBounds(rect);

			pLabel->SetText("");
			pLabel->SetAlignment(MAM_LEFT | MAM_TOP);
		}

		sprintf(szWidget, "ClanResult_GameRoomImg1%d", i);
		MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szWidget);
		if (pWidget)
			pWidget->Show(false);
	}
	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_PlayerKillList1");
	if (pTextArea) {
		pTextArea->Clear();
		pTextArea->SetCustomLineHeight(_H18);
	}
	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_PlayerDeathList1");
	if (pTextArea) {
		pTextArea->Clear();
		pTextArea->SetCustomLineHeight(_H18);
	}
	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_PlayerNameList2");
	if (pTextArea) {
		pTextArea->Clear();
		pTextArea->SetCustomLineHeight(_H18);
	}
	for (int i = 0; i < 4; i++)
	{
		char szWidget[128];
		sprintf(szWidget, "ClanResult_PlayerScore2%d", i);
		MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szWidget);
		if (pLabel)
		{
			MRECT rect;
			rect = pLabel->GetRect();
			rect.y = pTextArea->GetRect().y + _H18 * i - _H2;
			rect.h = _H21;
			pLabel->SetBounds(rect);

			pLabel->SetText("");
			pLabel->SetAlignment(MAM_LEFT | MAM_TOP);
		}

		sprintf(szWidget, "ClanResult_GameRoomImg2%d", i);
		MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szWidget);
		if (pWidget)
			pWidget->Show(false);
	}
	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_PlayerKillList2");
	if (pTextArea) {
		pTextArea->Clear();
		pTextArea->SetCustomLineHeight(_H18);
	}
	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_PlayerDeathList2");
	if (pTextArea) {
		pTextArea->Clear();
		pTextArea->SetCustomLineHeight(_H18);
	}

	char szFileName[256];
	szFileName[0] = 0;

	if (ZGetGameTypeManager()->IsQuestOnly(ZGetGame()->GetMatch()->GetMatchType()))
	{
		strcpy(szFileName, "interface/default/rstbg_quest.jpg");
		pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("QuestResult");
		if (pWidget)
			pWidget->Show(true);

		ZBmNumLabel* pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("QuestResult_GetPlusXP");
		if (pBmNumLabel)
			pBmNumLabel->SetNumber(0, false);
		pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("QuestResult_GetMinusXP");
		if (pBmNumLabel)
			pBmNumLabel->SetNumber(0, false);
		pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("QuestResult_GetTotalXP");
		if (pBmNumLabel)
			pBmNumLabel->SetNumber(0, false);
		pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("QuestResult_GetBounty");
		if (pBmNumLabel)
			pBmNumLabel->SetNumber(0, false);
	}

	else if (ZGetGameTypeManager()->IsSurvivalOnly(ZGetGame()->GetMatch()->GetMatchType()))
	{
		strcpy(szFileName, "interface/default/rstbg_survival.jpg");
		pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SurvivalResult");
		if (pWidget)
			pWidget->Show(true);

		ZBmNumLabel* pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SurvivalResult_GetReachedRound");
		if (pBmNumLabel)
			pBmNumLabel->SetNumber(0, false);
		pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SurvivalResult_GetPoint");
		if (pBmNumLabel)
			pBmNumLabel->SetNumber(0, false);
		pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SurvivalResult_GetXP");
		if (pBmNumLabel)
			pBmNumLabel->SetNumber(0, false);
		pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SurvivalResult_GetBounty");
		if (pBmNumLabel)
			pBmNumLabel->SetNumber(0, false);
	}

	else if (ZGetGameClient()->IsLadderGame())
	{
		strcpy(szFileName, "interface/default/rstbg_clan.jpg");
		pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult");
		if (pWidget)
			pWidget->Show(true);

		int nWinnerTeam;
		if (ZGetGame()->GetMatch()->GetTeamScore(MMT_RED) == ZGetGame()->GetMatch()->GetTeamScore(MMT_BLUE))
		{
			MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_Win");
			if (pPicture) 	pPicture->SetBitmap(MBitmapManager::Get("result_draw.tga"));

			pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_Lose");
			if (pPicture) 	pPicture->SetBitmap(MBitmapManager::Get("result_draw.tga"));
		}
		else
		{
			MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_Win");
			if (pPicture) 	pPicture->SetBitmap(MBitmapManager::Get("result_win.tga"));

			pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_Lose");
			if (pPicture) 	pPicture->SetBitmap(MBitmapManager::Get("result_lose.tga"));
		}

		if (ZGetGame()->GetMatch()->GetTeamScore(MMT_RED) > ZGetGame()->GetMatch()->GetTeamScore(MMT_BLUE))
			nWinnerTeam = MMT_RED;
		else
			nWinnerTeam = MMT_BLUE;

		for (int i = 0; i < 2; i++)
		{
			MMatchTeam nTeam = (i == 0) ? MMT_RED : MMT_BLUE;
			char* szClanName = (i == 0) ? m_szRedClanName : m_szBlueClanName;
			int nClanID = (i == 0) ? m_nClanIDRed : m_nClanIDBlue;

			MPicture* pPicture;
			if (nWinnerTeam == nTeam)
				pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_ClanBitmap1");
			else
				pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_ClanBitmap2");
			if (pPicture)
			{
				MBitmap* pBitmap = ZGetEmblemInterface()->GetClanEmblem2(nClanID);
				if (pBitmap)
				{
					pPicture->SetBitmap(pBitmap);
					pPicture->Show(true);
				}
			}

			MLabel* pLabel;
			if (nWinnerTeam == nTeam)
				pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_PlayerNameListLabel1");
			else
				pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_PlayerNameListLabel2");
			if (pLabel)
			{
				pLabel->SetText(szClanName);
				pLabel->Show(true);
			}

			int nStartX = 0;
			int nStartY = 0;
			char szName[256];
			sprintf(szName, "ClanResult_PlayerNameList%d", i + 1);
			pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szName);
			if (pWidget)
			{
				nStartX = pWidget->GetRect().x;
				nStartY = pWidget->GetRect().y;
			}

			for (int j = 0; j < 4; j++)
			{
				char szName[256];
				sprintf(szName, "ClanResult_GameRoomImg%d%d", i + 1, j);
				pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szName);
				if (pWidget)
				{
					MRECT rect2;
					rect2.x = nStartX - _W17;
					rect2.y = _H18 * j + nStartY;
					rect2.w = _W21;
					rect2.h = _H18;

					pWidget->SetBounds(rect2);
				}
			}
		}

		for (ZResultBoardList::iterator i = m_ResultItems.begin(); i != m_ResultItems.end(); i++)
		{
			ZResultBoardItem* pItem = *i;

			if ((pItem->nTeam != MMT_RED) && (pItem->nTeam != MMT_BLUE))
				continue;

			if (nWinnerTeam == pItem->nTeam)
				AddClanResultInfoWin(pItem->szName, pItem->nScore, pItem->nKills, pItem->nDeaths, pItem->bMyChar, pItem->bGameRoomUser);
			else
				AddClanResultInfoLose(pItem->szName, pItem->nScore, pItem->nKills, pItem->nDeaths, pItem->bMyChar, pItem->bGameRoomUser);
		}
	}
	else if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
	{
		strcpy(szFileName, "interface/default/rstbg_deathmatch.jpg");
		pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("DuelTournamentResult");
		if (pWidget)
			pWidget->Show(true);

		ZCharacterView* pCharView = GetWidgetCharViewResult();
		if (pCharView) {
			MUID uidChampion = ((ZRuleDuelTournament*)ZGetGame()->GetMatch()->GetRule())->GetChampion();
			pCharView->SetCharacter(uidChampion);
		}
	}
	else
	{
		if ((ZGetLocale()->GetCountry() == MC_US) || (ZGetLocale()->GetCountry() == MC_BRAZIL) || (ZGetLocale()->GetCountry() == MC_INDIA))
		{
			if ((rand() % 2))
				strcpy(szFileName, "interface/default/rstbg_deathmatch.jpg");
			else
				strcpy(szFileName, "interface/default/rstbg_clan.jpg");
		}
		else
			strcpy(szFileName, "interface/default/rstbg_deathmatch.jpg");

		pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatResult");
		if (pWidget)
			pWidget->Show(true);

		int nStartY = 0;
		pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatResult_PlayerNameList");
		if (pWidget)
			nStartY = pWidget->GetRect().y;

		ZResultBoardList::iterator itrList = m_ResultItems.begin();
		for (int i = 0; i < 16; i++)
		{
			int nTeam = 0;

			if (itrList != m_ResultItems.end())
			{
				ZResultBoardItem* pItem = *itrList;

				if ((pItem->nTeam == MMT_RED) || (pItem->nTeam == MMT_BLUE) || (pItem->nTeam == 4))
					AddCombatResultInfo(pItem->szName, pItem->nScore, pItem->nKills, pItem->nDeaths, pItem->bMyChar, pItem->bGameRoomUser);

				nTeam = pItem->nTeam;
				itrList++;
			}

			for (int j = MMT_RED; j <= MMT_BLUE; j++)
			{
				char szName[128];
				sprintf(szName, "CombatResult_%sTeamBG%02d", ((j == MMT_RED) ? "Red" : "Blue"), i);

				pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szName);
				if (pWidget)
				{
					MRECT rect;
					rect = pWidget->GetRect();
					rect.y = _H18 * i + nStartY;
					rect.h = _H18;

					pWidget->SetBounds(rect);

					if (nTeam == j)
						pWidget->Show(true);
					else
						pWidget->Show(false);

					pWidget->SetOpacity(110);

					sprintf(szName, "CombatResult_GameRoomImg%02d", i);
					pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szName);
					if (pWidget)
					{
						MRECT rect2;
						rect2 = pWidget->GetRect();
						rect2.x = rect.x - _W20;
						rect2.y = _H18 * i + nStartY;
						rect2.w = _W21;
						rect2.h = _H18;

						pWidget->SetBounds(rect2);
					}
				}
			}
		}

		MPicture* pPicture;
		if (ZGetGameTypeManager()->IsTeamExtremeGame(ZGetGame()->GetMatch()->GetMatchType()))
		{
			pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatResult_Finish");
			if (pPicture)
				pPicture->Show(false);

			pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatResult_WinLoseDraw");
			if (pPicture)
			{
				if (ZGetGame()->GetMatch()->GetTeamKills(MMT_RED) == ZGetGame()->GetMatch()->GetTeamKills(MMT_BLUE))
					pPicture->SetBitmap(MBitmapManager::Get("result_draw.tga"));
				else
				{
					if (ZGetGame()->GetMatch()->GetTeamKills(MMT_RED) > ZGetGame()->GetMatch()->GetTeamKills(MMT_BLUE))
					{
						if (ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_RED)
							pPicture->SetBitmap(MBitmapManager::Get("result_win.tga"));
						else
							pPicture->SetBitmap(MBitmapManager::Get("result_lose.tga"));
					}
					else
					{
						if (ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_BLUE)
							pPicture->SetBitmap(MBitmapManager::Get("result_win.tga"));
						else
							pPicture->SetBitmap(MBitmapManager::Get("result_lose.tga"));
					}
				}

				pPicture->Show(true);
			}
		}
		else if (ZGetGameInterface()->m_bTeamPlay)
		{
			pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatResult_Finish");
			if (pPicture)
				pPicture->Show(false);

			pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatResult_WinLoseDraw");
			if (pPicture)
			{
				if (ZGetGame()->GetMatch()->GetTeamScore(MMT_RED) == ZGetGame()->GetMatch()->GetTeamScore(MMT_BLUE))
					pPicture->SetBitmap(MBitmapManager::Get("result_draw.tga"));
				else
				{
					if (ZGetGame()->GetMatch()->GetTeamScore(MMT_RED) > ZGetGame()->GetMatch()->GetTeamScore(MMT_BLUE))
					{
						if (ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_RED)
							pPicture->SetBitmap(MBitmapManager::Get("result_win.tga"));
						else
							pPicture->SetBitmap(MBitmapManager::Get("result_lose.tga"));
					}
					else
					{
						if (ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_BLUE)
							pPicture->SetBitmap(MBitmapManager::Get("result_win.tga"));
						else
							pPicture->SetBitmap(MBitmapManager::Get("result_lose.tga"));
					}
				}

				pPicture->Show(true);
			}
		}
		else
		{
			pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatResult_Finish");
			if (pPicture)
				pPicture->Show(true);

			pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatResult_WinLoseDraw");
			if (pPicture)
				pPicture->Show(false);
		}
	}

	m_pResultBgImg = new MBitmapR2;
	((MBitmapR2*)m_pResultBgImg)->Create("resultbackground.png", RGetDevice(), szFileName);
	if (m_pResultBgImg != NULL)
	{
		MPicture* pBgImage = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("GameResult_Background");
		if (pBgImage)
			pBgImage->SetBitmap(m_pResultBgImg->GetSourceBitmap());
	}
}

void ZCombatInterface::DrawResultBoard(MDrawContext* pDC)
{
	bool bClanGame = ZGetGameClient()->IsLadderGame();
	if (!m_pResultPanel) return;

	m_pResultPanel->Draw(0);

	if (m_pResultPanel_Team)
		m_pResultPanel_Team->Draw(0);

#define FADE_START_FRAME	20000

	RVisualMesh* pvm = m_pResultPanel->GetVMesh();
	if (bClanGame && pvm->isOncePlayDone())
	{
		if (!m_pResultLeft)
		{
			char* szEffectNames[] = { "clan_win", "clan_draw", "clan_lose" };

			int nRed = ZGetGame()->GetMatch()->GetTeamScore(MMT_RED);
			int nBlue = ZGetGame()->GetMatch()->GetTeamScore(MMT_BLUE);
			int nLeft, nRight;

			if (ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_RED) {
				nLeft = (nRed == nBlue) ? 1 : (nRed > nBlue) ? 0 : 2;
			}
			else
				nLeft = (nRed == nBlue) ? 1 : (nRed < nBlue) ? 0 : 2;

			nRight = 2 - nLeft;

			m_pResultLeft = ZGetScreenEffectManager()->CreateScreenEffect(szEffectNames[nLeft],
				rvector(-240.f, -267.f, 0));
			m_pResultRight = ZGetScreenEffectManager()->CreateScreenEffect(szEffectNames[nRight],
				rvector(240.f, -267.f, 0));
		}

		m_pResultLeft->Draw(0);
		m_pResultRight->Draw(0);
	}

	int nFrame = pvm->GetFrameInfo(ani_mode_lower)->m_nFrame;

	float fOpacity = min(1.f, max(0, float(nFrame - FADE_START_FRAME)
		/ float(pvm->GetFrameInfo(ani_mode_lower)->m_pAniSet->GetMaxFrame() - FADE_START_FRAME)));

	MFont* pFont = GetGameFont();
	pDC->SetFont(pFont);
	pFont = pDC->GetFont();

	MCOLOR opacity = MCOLOR(0, 0, 0, 255 * fOpacity);
	pDC->SetOpacity(255 * fOpacity);

	float x, y;

	char szText[256];

	x = 0.026f;
	y = 0.107f;

	const float fHeight = 0.651f;

	int nMaxLineCount = 16;

	float linespace = fHeight / (float)nMaxLineCount;

	m_ResultItems.sort(CompareZResultBoardItem);

	if (bClanGame)
	{
		int nLeft = 0;
		int nRight = 0;

		y = 0.387f;

		for (ZResultBoardList::iterator i = m_ResultItems.begin(); i != m_ResultItems.end(); i++)
		{
			ZResultBoardItem* pItem = *i;

			int y1, y2;
			float itemy;

			float clancenter;
			bool bDrawClanName = false;

			MCOLOR backgroundcolor;

			if (pItem->nTeam == ZGetGame()->m_pMyCharacter->GetTeamID()) {
				x = 0.035f;
				itemy = y + linespace * nLeft;
				nLeft++;
				if (nLeft == 1)
				{
					bDrawClanName = true;
					clancenter = 0.25f;
				}
				backgroundcolor = (nLeft % 2 == 0) ? BACKGROUND_COLOR1 : BACKGROUND_COLOR2;
				y1 = itemy * MGetWorkspaceHeight();
				y2 = (y + linespace * nLeft) * MGetWorkspaceHeight();
			}
			else {
				x = 0.55f;
				itemy = y + linespace * nRight;
				nRight++;
				if (nRight == 1)
				{
					bDrawClanName = true;
					clancenter = 0.75f;
				}
				backgroundcolor = (nRight % 2 == 1) ? BACKGROUND_COLOR1 : BACKGROUND_COLOR2;
				y1 = itemy * MGetWorkspaceHeight();
				y2 = (y + linespace * nRight) * MGetWorkspaceHeight();
			}

			if (bDrawClanName)
			{
				MCOLOR textcolor = TEXT_COLOR_CLAN_NAME;
				textcolor.a = opacity.a;
				pDC->SetColor(textcolor);

				MFont* pClanFont = MFontManager::Get("FONTb11b");
				pDC->SetFont(pClanFont);

				float clanx = clancenter - .5f * (float)pClanFont->GetWidth(pItem->szClan) / (float)MGetWorkspaceWidth();
				TextRelative(pDC, clanx, 0.15, pItem->szClan);

				char szText[32];
				sprintf(szText, "%d", ZGetGame()->GetMatch()->GetTeamScore((MMatchTeam)pItem->nTeam));

				clanx = clancenter - .5f * (float)pClanFont->GetWidth(szText) / (float)MGetWorkspaceWidth();
				TextRelative(pDC, clanx, 0.2, szText);

				textcolor = TEXT_COLOR_TITLE;
				textcolor.a = opacity.a;
				pDC->SetColor(textcolor);
				float texty = itemy - linespace + (linespace - (float)pFont->GetHeight() / (float)RGetScreenHeight()) * .5f;
				TextRelative(pDC, x, texty, "Level Name");
				TextRelative(pDC, x + .25f, texty, "Exp", true);
				TextRelative(pDC, x + .32f, texty, "Kill", true);
				TextRelative(pDC, x + .39f, texty, "Death", true);
			}

			if (pItem->bMyChar)
				backgroundcolor = BACKGROUND_COLOR_MYCHAR_DEATH_MATCH;
			backgroundcolor.a = opacity.a >> 1;
			pDC->SetColor(backgroundcolor);
			pDC->FillRectangleW(
				(x - .01f) * MGetWorkspaceWidth(), y1,
				.44f * MGetWorkspaceWidth(), y2 - y1);

			MCOLOR textcolor = TEXT_COLOR_DEATH_MATCH;
			textcolor.a = opacity.a;
			pDC->SetColor(textcolor);
			pDC->SetFont(pFont);

			float texty = itemy + (linespace - (float)pFont->GetHeight() / (float)RGetScreenHeight()) * .5f;
			TextRelative(pDC, x, texty, pItem->szName);

			sprintf(szText, "%d", pItem->nScore);
			TextRelative(pDC, x + .25f, texty, szText, true);

			sprintf(szText, "%d", pItem->nKills);
			TextRelative(pDC, x + .32f, texty, szText, true);

			sprintf(szText, "%d", pItem->nDeaths);
			TextRelative(pDC, x + .39f, texty, szText, true);
		}
	}
	else
	{
		int nCount = 0;

		for (ZResultBoardList::iterator i = m_ResultItems.begin(); i != m_ResultItems.end(); i++)
		{
			ZResultBoardItem* pItem = *i;

			float itemy = y + linespace * nCount;

			nCount++;

			MCOLOR backgroundcolor = (nCount % 2 == 0) ? BACKGROUND_COLOR1 : BACKGROUND_COLOR2;
			if (pItem->bMyChar) backgroundcolor =
				(pItem->nTeam == MMT_RED) ? BACKGROUND_COLOR_MYCHAR_RED_TEAM :
				(pItem->nTeam == MMT_BLUE) ? BACKGROUND_COLOR_MYCHAR_BLUE_TEAM :
				BACKGROUND_COLOR_MYCHAR_DEATH_MATCH;

			backgroundcolor.a = opacity.a >> 1;
			pDC->SetColor(backgroundcolor);

			int y1 = itemy * MGetWorkspaceHeight();
			int y2 = (y + linespace * nCount) * MGetWorkspaceHeight();

			pDC->FillRectangleW(
				0.022f * MGetWorkspaceWidth(), y1,
				0.960f * MGetWorkspaceWidth(), y2 - y1);
			MCOLOR textcolor = TEXT_COLOR_DEATH_MATCH;

			if (pItem->nTeam == MMT_RED)
				textcolor = TEXT_COLOR_RED_TEAM;
			else
				if (pItem->nTeam == MMT_BLUE)
					textcolor = TEXT_COLOR_BLUE_TEAM;
				else
					if (pItem->nTeam == MMT_SPECTATOR)
						textcolor = TEXT_COLOR_SPECTATOR;

			textcolor.a = opacity.a;
			pDC->SetColor(textcolor);

			float texty = itemy + (linespace - (float)pFont->GetHeight() / (float)RGetScreenHeight()) * .5f;

			x = 0.025f;
			TextRelative(pDC, x, texty, pItem->szName);

			x = 0.43f;
			sprintf(szText, "%d", pItem->nScore);
			TextRelative(pDC, x, texty, szText, true);

			x = 0.52f;
			sprintf(szText, "%d", pItem->nKills);
			TextRelative(pDC, x, texty, szText, true);

			x = 0.61f;
			sprintf(szText, "%d", pItem->nDeaths);
			TextRelative(pDC, x, texty, szText, true);

			const float iconspace = 0.053f;

			x = 0.705f;

			pDC->SetBitmapColor(MCOLOR(255, 255, 255, 255 * fOpacity));

			IconRelative(pDC, x, texty, 0); x += iconspace;
			IconRelative(pDC, x, texty, 1); x += iconspace;
			IconRelative(pDC, x, texty, 2); x += iconspace;
			IconRelative(pDC, x, texty, 3); x += iconspace;
			IconRelative(pDC, x, texty, 4);

			pDC->SetBitmapColor(MCOLOR(255, 255, 255, 255));

			x = 0.705f + (float(pFont->GetHeight() * 1.3f) / MGetWorkspaceWidth());
			sprintf(szText, "%d", pItem->nAllKill);
			TextRelative(pDC, x, texty, szText); x += iconspace;
			sprintf(szText, "%d", pItem->nUnbelievable);
			TextRelative(pDC, x, texty, szText); x += iconspace;
			sprintf(szText, "%d", pItem->nExcellent);
			TextRelative(pDC, x, texty, szText); x += iconspace;
			sprintf(szText, "%d", pItem->nFantastic);
			TextRelative(pDC, x, texty, szText); x += iconspace;
			sprintf(szText, "%d", pItem->nHeadShot);
			TextRelative(pDC, x, texty, szText); x += iconspace;
		}
	}
}

void ZCombatInterface::IconRelative(MDrawContext* pDC, float x, float y, int nIcon)
{
	MBitmap* pbmp = m_ppIcons[nIcon];
	if (!pbmp) return;

	pDC->SetBitmap(pbmp);
	int screenx = x * MGetWorkspaceWidth();
	int screeny = y * MGetWorkspaceHeight();

	int nSize = pDC->GetFont()->GetHeight();
	pDC->Draw(screenx, screeny, nSize, nSize);
}

void ZCombatInterface::Finish()
{
	if (IsFinish())
		return;

	ZGetFlashBangEffect()->End();

	m_fOrgMusicVolume = ZApplication::GetSoundEngine()->GetMusicVolume();
	m_nReserveFinishTime = timeGetTime();
	m_bReserveFinish = true;

	m_CrossHair.Show(false);

	ZGetSoundEngine()->Set3DSoundUpdate(false);
}

bool ZCombatInterface::IsFinish()
{
	return m_bOnFinish;
}

void ZCombatInterface::OnFinish()
{
	if (m_pResultPanel) return;

	m_pResultLeft = NULL;
	m_pResultRight = NULL;

	ZGetScreenEffectManager()->AddRoundFinish();

	if (ZGetGame()->GetMatch()->IsTeamPlay() && !ZGetGameClient()->IsLadderGame())
	{
		int nRed = ZGetGame()->GetMatch()->GetTeamScore(MMT_RED), nBlue = ZGetGame()->GetMatch()->GetTeamScore(MMT_BLUE);
		if (nRed == nBlue)
			m_pResultPanel_Team = ZGetScreenEffectManager()->CreateScreenEffect("teamdraw");
		else
			if (nRed > nBlue)
				m_pResultPanel_Team = ZGetScreenEffectManager()->CreateScreenEffect("teamredwin");
			else
				m_pResultPanel_Team = ZGetScreenEffectManager()->CreateScreenEffect("teambluewin");
	}

	m_ResultItems.Destroy();

	ZCharacterManager::iterator itor;
	for (itor = ZGetCharacterManager()->begin();
		itor != ZGetCharacterManager()->end(); ++itor)
	{
		ZCharacter* pCharacter = (*itor).second;
		ZResultBoardItem* pItem = new ZResultBoardItem;

		if (pCharacter->IsAdminHide()) continue;

		if (pCharacter->IsAdminName()) {
			sprintf(pItem->szName, "--%s  %s", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
		}
		else {
			sprintf(pItem->szName, "%d%s %s", pCharacter->GetProperty()->nLevel, ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
		}

		strcpy(pItem->szClan, pCharacter->GetProperty()->GetClanName());
		pItem->nClanID = pCharacter->GetClanID();
		pItem->nTeam = ZGetGame()->GetMatch()->IsTeamPlay() ? pCharacter->GetTeamID() : MMT_END;
		pItem->nScore = pCharacter->GetStatus().Ref().nExp;
		pItem->nKills = pCharacter->GetStatus().Ref().nKills;
		pItem->nDeaths = pCharacter->GetStatus().Ref().nDeaths;

		pItem->nAllKill = pCharacter->GetStatus().Ref().nAllKill;
		pItem->nExcellent = pCharacter->GetStatus().Ref().nExcellent;
		pItem->nFantastic = pCharacter->GetStatus().Ref().nFantastic;
		pItem->nHeadShot = pCharacter->GetStatus().Ref().nHeadShot;
		pItem->nUnbelievable = pCharacter->GetStatus().Ref().nUnbelievable;

		pItem->bMyChar = pCharacter->IsHero();

		MMatchObjCache* pCache = ZGetGameClient()->FindObjCache(pCharacter->GetUID());
		if (pCache)
			pItem->bGameRoomUser = (pCache->GetPGrade() == MMPG_PREMIUM_IP) ? true : false;
		else
			pItem->bGameRoomUser = false;

		m_ResultItems.push_back(pItem);
	}

	m_Observer.Show(false);

	m_nReservedOutTime = timeGetTime() + 5000;
	m_bOnFinish = true;
}

void ZCombatInterface::SetObserverMode(bool bEnable)
{
	if (bEnable) ZGetScreenEffectManager()->ResetSpectator();
	m_Observer.Show(bEnable);
}

ZCharacter* ZCombatInterface::GetTargetCharacter()
{
	if (m_Observer.IsVisible())
	{
		return m_Observer.GetTargetCharacter();
	}

	return ZGetGame()->m_pMyCharacter;
}

MUID ZCombatInterface::GetTargetUID()
{
	return GetTargetCharacter()->GetUID();
}

void ZCombatInterface::GameCheckPickCharacter()
{
	MPOINT Cp = GetCrosshairPoint();

	ZPICKINFO pickinfo;

	rvector pos, dir;
	RGetScreenLine(Cp.x, Cp.y, &pos, &dir);

	ZMyCharacter* pMyChar = NULL;

	pMyChar = ZGetGame()->m_pMyCharacter;

	bool bPick = false;

	if (ZGetGame()->Pick(pMyChar, pos, dir, &pickinfo, RM_FLAG_ADDITIVE | RM_FLAG_HIDE, true)) {
		if (pickinfo.pObject) {
			if (pickinfo.info.parts == eq_parts_head) bPick = true;
			bPick = true;
		}
	}

	if (pMyChar && pMyChar->m_pVMesh) {
		RWeaponMotionType type = (RWeaponMotionType)pMyChar->m_pVMesh->GetSetectedWeaponMotionID();

		if ((type == eq_wd_katana) || (type == eq_wd_grenade) || (type == eq_ws_dagger) || (type == eq_wd_dagger)
			|| (type == eq_wd_item) || (type == eq_wd_sword) || (type == eq_wd_blade)) {
			bPick = false;
		}

		if (pMyChar->m_pVMesh->m_vRotXYZ.y > -20.f && pMyChar->m_pVMesh->m_vRotXYZ.y < 30.f) {
			bPick = false;
		}

		if (pMyChar->m_pVMesh->m_vRotXYZ.y < -25.f)
			bPick = true;

		if (pMyChar->IsMan()) {
			if (pMyChar->m_pVMesh->m_vRotXYZ.x < -20.f) {
				if (RCameraDirection.z < -0.2f)
					bPick = true;
			}
		}

		if ((pMyChar->m_AniState_Lower.Ref() == ZC_STATE_LOWER_TUMBLE_RIGHT) ||
			(pMyChar->m_AniState_Lower.Ref() == ZC_STATE_LOWER_TUMBLE_LEFT))
		{
			if (RCameraDirection.z < -0.3f)
				bPick = true;
		}

		if (RCameraDirection.z < -0.6f)
			bPick = true;

		if (bPick) {
			pMyChar->m_pVMesh->SetVisibility(0.4f);
		}
		else {
			pMyChar->m_pVMesh->SetVisibility(1.0f);
		}
	}

	if (ZGetGame()->Pick(pMyChar, pos, dir, &pickinfo))
	{
		if (pickinfo.pObject)
		{
			rvector v1;
			v1 = pickinfo.info.vOut;

			if (IsPlayerObject(pickinfo.pObject)) {
				SetPickTarget(true, (ZCharacter*)pickinfo.pObject);
			}
			else
			{
				m_CrossHair.SetState(ZCS_PICKENEMY);
			}
		}
		else
			SetPickTarget(false);
	}
}

void ZCombatInterface::OnGadget(MMatchWeaponType nWeaponType)
{
	if (m_pWeaponScreenEffect) m_pWeaponScreenEffect->OnGadget(nWeaponType);
	m_CrossHair.Show(false);
}

void ZCombatInterface::OnGadgetOff()
{
	if (m_pWeaponScreenEffect) m_pWeaponScreenEffect->OnGadgetOff();
	m_CrossHair.Show(true);
}

void ZCombatInterface::SetDrawLeaveBattle(bool bShow, int nSeconds)
{
	m_bDrawLeaveBattle = bShow;
	m_nDrawLeaveBattleSeconds = nSeconds;
}

void ZCombatInterface::OnAddCharacter(ZCharacter* pChar)
{
	bool bClanGame = ZGetGameClient()->IsLadderGame();
	if (bClanGame) {
		if (pChar->GetTeamID() == MMT_RED) {
			if (m_nClanIDRed == 0) {
				m_nClanIDRed = pChar->GetClanID();
				ZGetEmblemInterface()->AddClanInfo(m_nClanIDRed);
				strcpy(m_szRedClanName, pChar->GetProperty()->GetClanName());
			}
		}
		else if (pChar->GetTeamID() == MMT_BLUE) {
			if (m_nClanIDBlue == 0) {
				m_nClanIDBlue = pChar->GetClanID();
				ZGetEmblemInterface()->AddClanInfo(m_nClanIDBlue);
				strcpy(m_szBlueClanName, pChar->GetProperty()->GetClanName());
			}
		}
	}
}

void ZCombatInterface::ShowChatOutput(bool bShow)
{
	m_Chat.ShowOutput(bShow);
	ZGetConfiguration()->SetViewGameChat(bShow);
}

void ZCombatInterface::DrawAfterWidgets(MDrawContext* pDC)
{
	if (m_bShowResult)
		if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
			((ZRuleDuelTournament*)ZGetGame()->GetMatch()->GetRule())->ShowMatchOrder(pDC, true, m_fElapsed);
}

void ZCombatInterface::OnInvalidate()
{
	ZCharacterView* pCharView = GetWidgetCharViewLeft();
	if (pCharView)
		pCharView->OnInvalidate();
	pCharView = GetWidgetCharViewRight();
	if (pCharView)
		pCharView->OnInvalidate();
	pCharView = GetWidgetCharViewResult();
	if (pCharView)
		pCharView->OnInvalidate();
}

void ZCombatInterface::OnRestore()
{
	ZCharacterView* pCharView = GetWidgetCharViewLeft();
	if (pCharView)
		pCharView->OnRestore();
	pCharView = GetWidgetCharViewRight();
	if (pCharView)
		pCharView->OnRestore();
	pCharView = GetWidgetCharViewResult();
	if (pCharView)
		pCharView->OnRestore();
}