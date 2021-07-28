#include "stdafx.h"

#include "ZObserver.h"
#include "ZGame.h"
#include "ZCharacter.h"
#include "ZCharacterManager.h"
#include "ZCamera.h"
#include "ZGameInterface.h"
#include "MLabel.h"
#include "ZCombatInterface.h"
#include "ZApplication.h"
#include "ZMyInfo.h"
#include "ZGameClient.h"
#include "ZRuleDuel.h"
#include "ZRuleDuelTournament.h"

bool ZObserverQuickTarget::ConvertKeyToIndex(char nKey, int* nIndex)
{
	int nVal = nKey - '0';
	if (0 <= nVal && nVal <= 9) {
		*nIndex = nVal;
		return true;
	}
	else {
		return false;
	}
}

void ZObserverQuickTarget::StoreTarget(int nIndex, MUID uidChar)
{
	m_arrayPlayers[nIndex] = uidChar;
}
MUID ZObserverQuickTarget::GetTarget(int nIndex)
{
	return m_arrayPlayers[nIndex];
}

bool ZObserver::OnKeyEvent(bool bCtrl, char nKey)
{
	int nIndex = 0;
	if (m_QuickTarget.ConvertKeyToIndex(nKey, &nIndex) == false)
		return false;

	if (bCtrl) {
		m_QuickTarget.StoreTarget(nIndex, GetTargetCharacter()->GetUID());
	}
	else {
		MUID uidTarget = m_QuickTarget.GetTarget(nIndex);

		// 태거지정 특수키 사용 체크
		if ((uidTarget == MUID(0, 0)) && (nKey == OBSERVER_QUICK_TAGGER_TARGET_KEY))
		{
			for (ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.begin();
				itor != ZGetGame()->m_CharacterManager.end(); ++itor)
			{
				ZCharacter* pChar = (*itor).second;
				if (pChar->IsTagger())
				{
					uidTarget = pChar->GetUID();
					break;
				}
			}
		}

		ZCharacter* pCharacter = ZGetGame()->m_CharacterManager.Find(uidTarget);
		if (pCharacter && pCharacter->IsDie() == false)
			SetTarget(uidTarget);
	}
	return true;
}
/////////////////////////////////////////////////

ZObserver::ZObserver()
{
	m_pTargetCharacter = NULL;
	m_bVisible = false;
	m_pIDLResource = NULL;
	//	m_nLookType = ZOLM_BACKVIEW;
	m_FreeLookTarget = rvector(0.0f, 0.0f, 0.0f);
	m_nType = ZOM_ANYONE;
}
ZObserver::~ZObserver()
{
	Destroy();
}
bool ZObserver::Create(ZCamera* pCamera, ZIDLResource* pIDLResource)
{
	m_pCamera = pCamera;
	m_pIDLResource = pIDLResource;
	//	m_nLookType = ZOLM_BACKVIEW;

	return true;
}
void ZObserver::Destroy()
{
	ShowInfo(false);
}

void ZObserver::Show(bool bVisible)
{
	if (m_bVisible == bVisible) return;

	m_pCamera->SetLookMode(ZCAMERA_DEFAULT);

	if (bVisible)
	{
		if (ZGetGame()->GetMatch()->IsTeamPlay()) {
			if (ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_BLUE)
				m_nType = ZOM_BLUE;
			else
				m_nType = ZOM_RED;
		}
		else
			m_nType = ZOM_ANYONE;

		if (SetFirstTarget())
		{
			m_fDelay = ZOBSERVER_DEFAULT_DELAY_TIME;
			ShowInfo(true);
			m_bVisible = true;
			//			ZApplication::GetGameInterface()->SetCursorEnable(true);

			return;
		}
	}

	// 해제
	m_pTargetCharacter = NULL;
	ShowInfo(false);
	m_bVisible = false;
	//	ZApplication::GetGameInterface()->SetCursorEnable(false);

	ZGetGame()->ReleaseObserver();	// Observe Command Queue 청소
}

void ZObserver::ShowInfo(bool bShow)
{
	if (m_pTargetCharacter == NULL) return;
	/*
		MWidget* pWidget = m_pIDLResource->FindWidget("ObserverInfoLabel");

		if (pWidget!=NULL)
		{
			pWidget->Show(bShow);

			if (bShow)
			{
				char szTemp[128];
				sprintf(szName, "%s (HP:%d, AP:%d)", m_pTargetCharacter->GetUserName(), (int)m_pTargetCharacter->GetHP(), (int)m_pTargetCharacter->GetAP());
				pWidget->SetText(szTemp);

				((MLabel*)pWidget)->SetAlignment(MAM_HCENTER);
			}
		}
	*/
}

void ZObserver::ChangeToNextTarget()
{
	if (ZGetGame() == NULL) return;
	if (m_pTargetCharacter == NULL) return;

	ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.find(m_pTargetCharacter->GetUID());
	ZCharacter* pCharacter = NULL;
	bool bFlag = false;
	if (itor != ZGetGame()->m_CharacterManager.end())
	{
		do
		{
			itor++;

			if (itor == ZGetGame()->m_CharacterManager.end())
			{
				if (bFlag)
				{
					if (!ZGetMyInfo()->IsAdminGrade())
						Show(false);
					return;		// 두번 루프를 돌면 타겟이 아무도 없는 것이니 옵져버 해제
				}

				itor = ZGetGame()->m_CharacterManager.begin();
				bFlag = true;
			}
			pCharacter = (*itor).second;
		} while (!IsVisibleSetTarget(pCharacter));

		SetTarget(pCharacter);
	}
}

void ZObserver::SetTarget(ZCharacter* pCharacter)
{
	m_pTargetCharacter = pCharacter;
	if (m_pTargetCharacter)
	{
		m_FreeLookTarget = m_pTargetCharacter->m_TargetDir;
	}

	ShowInfo(true);
}

void ZObserver::SetType(ZObserverType nType)
{
	m_nType = nType;
}

bool ZObserver::SetFirstTarget()
{
	for (ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.begin();
		itor != ZGetGame()->m_CharacterManager.end(); ++itor)
	{
		ZCharacter* pCharacter = (*itor).second;
		if (IsVisibleSetTarget(pCharacter))
		{
			SetTarget(pCharacter);
			return true;
		}
	}

	// 타겟으로 할만한 사람이 없으면 옵져버 모드 해제
	Show(false);

	return false;
}

bool ZObserver::IsVisibleSetTarget(ZCharacter* pCharacter)
{
	if (pCharacter->IsDie()) return false;

	if (ZGetGame()->IsReplay()) return true;
	/*
		if(ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUEL)				// 듀얼일때 챔피언도 도전자도 아니면 옵저브 할 수 없다.
		{
			ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
			if (pDuel->GetQueueIdx(pCharacter->GetUID()) <= 1)	return true;
			else												return false;
		}*/

		// 듀얼 토너먼트일 때, 참가자면 옵저브 할 수 없다.
	if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUEL)
	{
		ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
		if (pDuel->GetQueueIdx(pCharacter->GetUID()) <= 1)	return true;
		else												return false;
	}

	// 퀘스트 모드일때는 visible 이 아니면 포탈로 들어간것이다. 옵저브 할수없다고 간주.
	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType())) {
		if (!pCharacter->IsVisible()) return false;
	}

	if (m_nType == ZOM_ANYONE) return true;

	if (m_nType == ZOM_RED && pCharacter->GetTeamID() == MMT_RED)
		return true;

	if (m_nType == ZOM_BLUE && pCharacter->GetTeamID() == MMT_BLUE)
		return true;

	/*
	// AdminHide 처리
	MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
	if (pObjCache && pObjCache->CheckFlag(MTD_PlayerFlags_AdminHide))
		return true;

	// 디버그 버젼에서는 아무나 볼수 있다
#ifndef _PUBLISH
	return true;
#endif

	if (pCharacter != g_pGame->m_pMyCharacter)
	{
		if (g_pGame->GetMatch()->IsTeamPlay())
		{
			if (pCharacter->GetTeamID() == g_pGame->m_pMyCharacter->GetTeamID())
			{
				return true;
			}
		}
	}
	*/

	return false;
}

bool GetUserInfoUID(MUID uid, MCOLOR& _color, MMatchUserGradeID& gid);

void ZObserver::DrawPlayerDuelHPAPBar(MDrawContext* pDC)
{
	char	charName[3][100];
	charName[0][0] = charName[1][0] = charName[2][0] = 0;
	float	fMaxHP[2] = { 0.0f, 0.0f }, fMaxAP[2] = { 0.0f, 0.0f };
	float	fHP[2] = { 0, 0 }, fAP[2] = { 0, 0 };
	bool	bExistNextChallenger = false;
	bool	bIsChampOserved = false;
	bool	bIsChlngOserved = false;
	MBitmap* pBitmap = NULL;

	MUID uidPlayer1, uidPlayer2, uidWaitQueue;
	uidWaitQueue.SetZero();

	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
	{
		ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
		uidPlayer1 = pDuel->QInfo.m_uidChampion;
		uidPlayer2 = pDuel->QInfo.m_uidChallenger;
		uidWaitQueue = pDuel->QInfo.m_WaitQueue[0];
		pBitmap = MBitmapManager::Get("duel_score.tga");
		ZGetCombatInterface()->DrawVictory(pDC, 162, 20, pDuel->QInfo.m_nVictory);	// 연승 마크 표시
	}
	else if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
	{
		ZRuleDuelTournament* pDuelTournament = (ZRuleDuelTournament*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
		uidPlayer1 = pDuelTournament->m_nextPlayerInfo.uidPlayer1;
		uidPlayer2 = pDuelTournament->m_nextPlayerInfo.uidPlayer2;
		pBitmap = MBitmapManager::Get("DuelTournament_score.tga");
		((ZRuleDuelTournament*)ZGetGame()->GetMatch()->GetRule())->DrawVictorySymbol(pDC, uidPlayer1, uidPlayer2);	// 승리 마크 표시
	}

	for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin(); itor != ZGetCharacterManager()->end(); ++itor)
	{
		ZCharacter* pCharacter = (*itor).second;

		// Champion
		if (pCharacter->GetUID() == uidPlayer1)
		{
			strcpy(charName[0], pCharacter->GetUserName());
			fMaxHP[0] = pCharacter->GetMaxHP();
			fMaxAP[0] = pCharacter->GetMaxAP();

			if (pCharacter->IsDie())
			{
				fHP[0] = 0;
				fAP[0] = 0;
			}
			else
			{
				if (ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
				{
					fHP[0] = pCharacter->GetHP();
					fAP[0] = pCharacter->GetAP();
				}
				else
				{
					ZRuleDuelTournament* pRule = ((ZRuleDuelTournament*)ZGetGame()->GetMatch()->GetRule());
					pRule->GetPlayerHpApForUI(uidPlayer1, &fMaxHP[0], &fMaxAP[0], &fHP[0], &fAP[0]);
				}
			}

			if (m_pTargetCharacter)
			{
				if (pCharacter->GetUID() == m_pTargetCharacter->GetUID())
					bIsChampOserved = true;
			}
		}

		// Challenger
		else if (pCharacter->GetUID() == uidPlayer2)
		{
			strcpy(charName[1], pCharacter->GetUserName());
			fMaxHP[1] = pCharacter->GetMaxHP();
			fMaxAP[1] = pCharacter->GetMaxAP();
			if (pCharacter->IsDie())
			{
				fHP[1] = 0;
				fAP[1] = 0;
			}
			else
			{
				if (ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
				{
					fHP[1] = pCharacter->GetHP();
					fAP[1] = pCharacter->GetAP();
				}
				else
				{
					ZRuleDuelTournament* pRule = ((ZRuleDuelTournament*)ZGetGame()->GetMatch()->GetRule());
					pRule->GetPlayerHpApForUI(uidPlayer2, &fMaxHP[1], &fMaxAP[1], &fHP[1], &fAP[1]);
				}
			}

			if (m_pTargetCharacter)
			{
				if (pCharacter->GetUID() == m_pTargetCharacter->GetUID())
					bIsChlngOserved = true;
			}
		}

		// Waiting
		else if (pCharacter->GetUID() == uidWaitQueue)
		{
			strcpy(charName[2], pCharacter->GetUserName());
			bExistNextChallenger = true;
		}
	}

	float fRx = (float)MGetWorkspaceWidth() / 800.0f;
	float fRy = (float)MGetWorkspaceHeight() / 600.0f;

	int nWidth;
	float fPosy;
	float fLength;
	float fHeight;

	// HP
	fPosy = 10.0f * fRy;
	fLength = 163.0f * fRx;
	fHeight = 23.0f * fRy;

	pDC->SetColor(255, 0, 0, 210);
	nWidth = (int)((float)fHP[0] / fMaxHP[0] * fLength);
	pDC->FillRectangle((193.0f + 163.0f) * fRx - nWidth, fPosy, nWidth, fHeight);

	nWidth = (int)((float)fHP[1] / fMaxHP[1] * fLength);
	pDC->FillRectangle(444.0f * fRx, fPosy, nWidth, fHeight);

	// AP
	pDC->SetColor(0, 50, 0, 128);
	pDC->FillRectangle(218.0f * fRx, 37.0f * fRy, 150.0f * fRx, 6.0f * fRy);
	pDC->FillRectangle(432.0f * fRx, 37.0f * fRy, 150.0f * fRx, 6.0f * fRy);

	pDC->SetColor(0, 180, 0, 200);
	nWidth = (int)((float)fAP[0] / fMaxAP[0] * 150.0f * fRx);
	pDC->FillRectangle((218.0f + 150.0f) * fRx - nWidth, 37.0f * fRy, nWidth, 5.0f * fRy);

	nWidth = (int)((float)fAP[1] / fMaxAP[1] * 150.0f * fRx);
	pDC->FillRectangle(432.0f * fRx, 37.0f * fRy, nWidth, 5.0f * fRy);

	if (pBitmap)
	{
		pDC->SetBitmap(pBitmap);
		pDC->Draw(167.0f * fRx, 0, 466.0f * fRx, 49.0f * fRx);
	}

	MFont* pFont = MFontManager::Get("FONTa10_O2Wht");
	if (pFont == NULL)
		//_ASSERT(0);
		pDC->SetFont(pFont);
	int nTime = timeGetTime() % 200;
	if (bIsChampOserved && (nTime < 100))
		pDC->SetColor(MCOLOR(0xFFFFFF00));
	else
		pDC->SetColor(MCOLOR(0xFFA0A0A0));
	TextRelative(pDC, 0.34f, 0.026f, charName[0], true);

	if (bIsChlngOserved && (nTime < 100))
		pDC->SetColor(MCOLOR(0xFFFFFF00));
	else
		pDC->SetColor(MCOLOR(0xFFA0A0A0));
	TextRelative(pDC, 0.66f, 0.026f, charName[1], true);

	if (bExistNextChallenger)
	{
		MBitmap* pBitmap = MBitmapManager::Get("icon_play.tga");
		if (pBitmap)
		{
			pDC->SetBitmap(pBitmap);

			int nIcon = 20.0f * fRx;
			pDC->Draw(646.0f * fRx, 0, nIcon, nIcon);
			pDC->Draw(640.0f * fRx, 0, nIcon, nIcon);
		}

		pDC->SetColor(MCOLOR(0xFF808080));
		TextRelative(pDC, 0.83f, 0.01f, charName[2], false);
	}

	// 현재 보고있는 캐릭터 이름 표시
	/*		if ( m_pTargetCharacter)
	{
	char szName[128];
	sprintf(szName, "%s (HP:%d, AP:%d)", m_pTargetCharacter->GetUserName(), (int)m_pTargetCharacter->GetHP(), (int)m_pTargetCharacter->GetAP());
	pDC->SetColor(MCOLOR(0xFFA0A0A0));
	TextRelative(pDC, 0.5f, 75.0f/800.0f, szName, true);
	}
	*/
}

void ZObserver::OnDraw(MDrawContext* pDC)
{
	if (ZGetGame()->IsReplay() && !ZGetGame()->IsShowReplayInfo())
		return;

	if (m_pTargetCharacter == NULL)
		return;

	if (ZGetCamera()->GetLookMode() == ZCAMERA_MINIMAP)
		return;

	// 운영자일 경우
	if (ZGetMyInfo()->IsAdminGrade())
	{
		MFont* pFont = MFontManager::Get("FONTb11b");
		if (pFont == NULL)
			//_ASSERT(0);
			pDC->SetFont(pFont);

		// 테두리
		MCOLOR backgroundcolor;
		if (m_pTargetCharacter->GetTeamID() == MMT_RED)
			backgroundcolor = MCOLOR(100, 0, 0, 150);
		else if (m_pTargetCharacter->GetTeamID() == MMT_BLUE)
			backgroundcolor = MCOLOR(0, 0, 100, 150);
		else
			backgroundcolor = MCOLOR(0, 0, 0, 150);

		pDC->SetColor(backgroundcolor);
		pDC->FillRectangle(MGetWorkspaceWidth() / 2 - 170, MGetWorkspaceHeight() * (650.0f / 800.0f) - 7, 340, 30);

		// 텍스트
		backgroundcolor = MCOLOR(255, 255, 255, 255);
		pDC->SetColor(backgroundcolor);

		char szName[128];
		sprintf(szName, "%s (HP:%d, AP:%d)", m_pTargetCharacter->GetUserName(), (int)m_pTargetCharacter->GetHP(), (int)m_pTargetCharacter->GetAP());
		TextRelative(pDC, 0.5f, 650.0f / 800.0f, szName, true);

		// 운영자계정, 듀얼토너먼트일때에 플레이어 게이지 출력
		if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
		{
			if (ZGetGame()->GetMatch()->GetRoundState() != MMATCH_ROUNDSTATE_PRE_COUNTDOWN)
				DrawPlayerDuelHPAPBar(pDC);
		}
	}

	// 듀얼 매치일 경우엔 게이지바를 표시한다
	else if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
	{
		DrawPlayerDuelHPAPBar(pDC);
	}
	else if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
	{
		if (ZGetGame()->GetMatch()->GetRoundState() != MMATCH_ROUNDSTATE_PRE_COUNTDOWN)
			DrawPlayerDuelHPAPBar(pDC);
	}
	else if (ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL)
	{
		char szName[128];
		sprintf(szName, "%s (HP:%d, AP:%d)", m_pTargetCharacter->GetUserName(), (int)m_pTargetCharacter->GetHP(), (int)m_pTargetCharacter->GetAP());
		if (m_pTargetCharacter->IsAdminName())
		{
			// Custom: Set custom colours
			MCOLOR gmColor;
			char szEmpty[4];
			memset(szEmpty, 0, sizeof(szEmpty));

			if (ZGetGame()->GetUserGradeIDColor(m_pTargetCharacter->GetUserGrade(), gmColor))
				pDC->SetColor(gmColor);
			else
				pDC->SetColor(MCOLOR(ZCOLOR_ADMIN_NAME));
		}
		else
			pDC->SetColor(MCOLOR(0xFFFFFFFF));

		MFont* pFont = MFontManager::Get("FONTb11b");
		if (pFont == NULL)
			//_ASSERT(0);
			pDC->SetFont(pFont);

		if (ZGetGameTypeManager()->IsTeamExtremeGame(ZGetGame()->GetMatch()->GetMatchType()))
			TextRelative(pDC, 0.5f, 75.0f / 800.0f, szName, true);
		else
			TextRelative(pDC, 0.5f, 50.0f / 800.0f, szName, true);
	}

	// 카메라 표시
	if (!ZGetMyInfo()->IsAdminGrade()) {
		ZCamera* pCamera = ZGetGameInterface()->GetCamera();

		const char* szModes[] = { "normal", "user", "free", "minimap" };
		char szFileName[50];
		sprintf(szFileName, "camera_%s.tga", szModes[pCamera->GetLookMode()]);
		pDC->SetBitmap(MBitmapManager::Get(szFileName));

		float fGain = (float)MGetWorkspaceWidth() / 800.0f;
		pDC->Draw((int)(720.0f * fGain), (int)(7.0f * fGain), (int)(64.0f * fGain), (int)(64.0f * fGain));
	}

	if (ZGetMyInfo()->IsAdminGrade() && !ZGetGameTypeManager()->IsTeamExtremeGame(ZGetGame()->GetMatch()->GetMatchType()))
	{
		int nNumOfTotal = 0, nNumOfRedTeam = 0, nNumOfBlueTeam = 0;
		ZCharacterManager::iterator itor;
		ZCharacter* pCharacter;
		for (itor = ZGetCharacterManager()->begin(); itor != ZGetCharacterManager()->end(); ++itor)
		{
			pCharacter = (*itor).second;

			if (pCharacter->GetTeamID() == MMT_SPECTATOR)		// 옵저버는 뺸다
				continue;

			if (pCharacter->IsAdminHide()) continue;

			if ((pCharacter->GetTeamID() == 4) && (!pCharacter->IsDie()))
				nNumOfTotal++;
			else if ((pCharacter->GetTeamID() == MMT_RED) && (!pCharacter->IsDie()))
				nNumOfRedTeam++;
			else if ((pCharacter->GetTeamID() == MMT_BLUE) && (!pCharacter->IsDie()))
				nNumOfBlueTeam++;
		}

		float sizex = MGetWorkspaceWidth() / 800.f;
		float sizey = MGetWorkspaceHeight() / 600.f;
		char szText[128];

		MCOLOR backgroundcolor;

		if (ZGetGame()->GetMatch()->IsTeamPlay())
		{
			backgroundcolor = MCOLOR(100, 0, 0, 150);
			pDC->SetColor(backgroundcolor);
			pDC->FillRectangle(700 * sizex, 37 * sizey, 85 * sizex, 22 * sizey);
			backgroundcolor = MCOLOR(0, 0, 100, 150);
			pDC->SetColor(backgroundcolor);
			pDC->FillRectangle(700 * sizex, 62 * sizey, 85 * sizex, 22 * sizey);

			// 인원수 표시
			backgroundcolor = MCOLOR(255, 180, 180, 255);
			pDC->SetColor(backgroundcolor);
			sprintf(szText, "%s:%d", ZMsg(MSG_WORD_REDTEAM), nNumOfRedTeam);
			TextRelative(pDC, 0.92f, 40.0f / 600.0f, szText, true);
			backgroundcolor = MCOLOR(180, 180, 255, 255);
			pDC->SetColor(backgroundcolor);
			sprintf(szText, "%s:%d", ZMsg(MSG_WORD_BLUETEAM), nNumOfBlueTeam);
			TextRelative(pDC, 0.92f, 65.0f / 600.0f, szText, true);
		}
		else
		{
		}
	}

	CheckDeadTarget();
}

void ZObserver::CheckDeadTarget()
{
	static unsigned long int nLastTime = timeGetTime();
	static unsigned long int st_nDeadTime = 0;

	unsigned long int nNowTime = timeGetTime();

	if (m_pTargetCharacter == NULL)
	{
		st_nDeadTime = 0;
		nLastTime = nNowTime;
		return;
	}

	if (m_pTargetCharacter->IsDie())
	{
		st_nDeadTime += nNowTime - nLastTime;
	}
	else
	{
		st_nDeadTime = 0;
	}

	if (st_nDeadTime > 3000)
	{
		st_nDeadTime = 0;
		ChangeToNextTarget();
	}

	nLastTime = nNowTime;
}

void ZObserver::SetTarget(MUID muid)
{
	ZCharacter* pCharacter = NULL;
	pCharacter = ZGetGame()->m_CharacterManager.Find(muid);
	if (pCharacter)
	{
		SetTarget(pCharacter);
	}
}

void ZObserver::NextLookMode()
{
	ZCamera* pCamera = ZGetGameInterface()->GetCamera();
	pCamera->SetNextLookMode();

	/*
	m_nLookType = ZObserverLookMode(m_nLookType +1);

	// 운영자 등급만 가능하다
	if(ZGetMyInfo()->IsAdminGrade())
	{
		if(m_nLookType==ZOLM_MAX)
			m_nLookType = ZOLM_BACKVIEW;
	}else
	{
		if(m_nLookType==ZOLM_FREELOOK)
			m_nLookType = ZOLM_BACKVIEW;
	}

	ZCamera *pCamera = ZGetGameInterface()->GetCamera();
	switch(m_nLookType) {
		case ZOLM_BACKVIEW:
			pCamera->SetLookMode(ZCAMERA_DEFAULT);break;
		case ZOLM_FREEANGLELOOK:
			pCamera->SetLookMode(ZCAMERA_FREEANGLE);break;
		case ZOLM_FREELOOK:
			pCamera->SetLookMode(ZCAMERA_FREELOOK);break;
		case ZOLM_MINIMAP:
			pCamera->SetLookMode(ZCAMERA_MINIMAP);break;
		default:
			//_ASSERT(FALSE);
	}
	*/
}