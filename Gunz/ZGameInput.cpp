#include "stdafx.h"

#include "ZGameInput.h"
#include "ZApplication.h"
#include "ZGameInterface.h"
#include "ZGame.h"
#include "ZConfiguration.h"
#include "ZActionDef.h"
#include "Mint.h"
#include "MEvent.h"
#include "MWidget.h"
#include "ZGameClient.h"
#include "ZCombatInterface.h"
#include "ZConsole.h"
#include "ZPost.h"
#include "ZScreenEffectManager.h"
#include "ZMyInfo.h"
#include "ZMinimap.h"
#include "ZInput.h"
#include "ZBandiCapturer.h"

#undef _DONOTUSE_DINPUT_MOUSE

ZGameInput* ZGameInput::m_pInstance = NULL;
static DWORD g_dwMacroTime = 0;
ZGameInput::ZGameInput()
{
	m_pInstance = this;
	m_bCTOff = false;

	static ZKEYSEQUENCEITEM action_ftumble[] = { {true,ZACTION_FORWARD}, {false,ZACTION_FORWARD} , {true,ZACTION_FORWARD} };
	static ZKEYSEQUENCEITEM action_btumble[] = { {true,ZACTION_BACK}, {false,ZACTION_BACK} , {true,ZACTION_BACK} };
	static ZKEYSEQUENCEITEM action_rtumble[] = { {true,ZACTION_RIGHT}, {false,ZACTION_RIGHT} , {true,ZACTION_RIGHT} };
	static ZKEYSEQUENCEITEM action_ltumble[] = { {true,ZACTION_LEFT}, {false,ZACTION_LEFT} , {true,ZACTION_LEFT} };

#define ADDKEYSEQUENCE(time,x) m_SequenceActions.push_back(ZKEYSEQUENCEACTION(time,sizeof(x)/sizeof(ZKEYSEQUENCEITEM),x));

	const float DASH_SEQUENCE_TIME = 0.2f;
	ADDKEYSEQUENCE(DASH_SEQUENCE_TIME, action_ftumble);
	ADDKEYSEQUENCE(DASH_SEQUENCE_TIME, action_btumble);
	ADDKEYSEQUENCE(DASH_SEQUENCE_TIME, action_rtumble);
	ADDKEYSEQUENCE(DASH_SEQUENCE_TIME, action_ltumble);
}

ZGameInput::~ZGameInput()
{
	m_pInstance = NULL;
}

bool ZGameInput::OnEvent(MEvent* pEvent)
{
	int sel = 0;

	if ((ZGetGameInterface()->GetState() != GUNZ_GAME)) return false;
	if (ZGetGameInterface()->GetGame() == NULL) return false;

	MWidget* pMenuWidget = ZGetGameInterface()->GetIDLResource()->FindWidget("CombatMenuFrame");
	if ((pMenuWidget) && (pMenuWidget->IsVisible())) return false;
	MWidget* pChatWidget = ZGetGameInterface()->GetIDLResource()->FindWidget("CombatChatInput");
	if ((pChatWidget) && (pChatWidget->IsVisible())) return false;
	MWidget* p112ConfirmWidget = ZGetGameInterface()->GetIDLResource()->FindWidget("112Confirm");
	if (p112ConfirmWidget->IsVisible()) return false;

#ifndef _PUBLISH
	if (m_pInstance) {
		if (m_pInstance->OnDebugEvent(pEvent) == true)
			return true;
	}
#endif

	ZMyCharacter* pMyCharacter = ZGetGameInterface()->GetGame()->m_pMyCharacter;
	if ((!pMyCharacter) || (!pMyCharacter->GetInitialized())) return false;

	switch (pEvent->nMessage) {
	case MWM_HOTKEY:
	{
		int nKey = pEvent->nKey;
		ZHOTKEY* hk = ZGetConfiguration()->GetHotkey(nKey);
		char buffer[256];
		strcpy(buffer, hk->command.c_str());
		ZApplication::GetGameInterface()->GetChat()->Input(buffer);
	}break;

	case MWM_LBUTTONDOWN:
	{
		ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();

		if (ZGetCombatInterface()->IsShowResult())
		{
			if (((ZGetCombatInterface()->m_nReservedOutTime - timeGetTime()) / 1000) < 13)
			{
				if (ZGetGameClient()->IsLadderGame() || ZGetGameClient()->IsDuelTournamentGame())
					ZChangeGameState(GUNZ_LOBBY);
				else
					ZChangeGameState(GUNZ_STAGE);

				return true;
			}
		}

		if (pCombatInterface->IsChat())
		{
			pCombatInterface->EnableInputChat(false);
		}

		if (pCombatInterface->GetObserver()->IsVisible())
		{
			pCombatInterface->GetObserver()->ChangeToNextTarget();
			return true;
		}

		if (ZGetGameInterface()->IsCursorEnable())
			return false;
	}
	return true;
	case MWM_RBUTTONDOWN:
	{
		if (ZGetGameInterface()->GetCombatInterface()->IsChat())
		{
			ZGetGameInterface()->GetCombatInterface()->EnableInputChat(false);
		}

		ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
		if (pCombatInterface->GetObserver()->IsVisible())
		{
			pCombatInterface->GetObserver()->NextLookMode();
		}
	}
	return true;
	case MWM_MBUTTONDOWN:
		if (ZGetGameInterface()->GetCombatInterface()->IsChat())
		{
			ZGetGameInterface()->GetCombatInterface()->EnableInputChat(false);
		}
		return true;
	case MWM_ACTIONRELEASED:
	{
		switch (pEvent->nKey) {
		case ZACTION_FORWARD:
		case ZACTION_BACK:
		case ZACTION_LEFT:
		case ZACTION_RIGHT:
			if (m_pInstance)
				m_pInstance->m_ActionKeyHistory.push_back(ZACTIONKEYITEM(ZGetGame()->GetTime(), false, pEvent->nKey));
			return true;

		case ZACTION_DEFENCE:
		{
			if (ZGetGame()->m_pMyCharacter)
				ZGetGame()->m_pMyCharacter->m_statusFlags.Ref().m_bGuardKey = false;
		}
		return true;
		}
	}break;
	case MWM_ACTIONPRESSED:
		if (!ZGetGame()->IsReservedSuicide())
		{
			switch (pEvent->nKey) {
			case ZACTION_FORWARD:
			case ZACTION_BACK:
			case ZACTION_LEFT:
			case ZACTION_RIGHT:
			case ZACTION_JUMP:
				if (m_pInstance)
					m_pInstance->m_ActionKeyHistory.push_back(ZACTIONKEYITEM(ZGetGame()->GetTime(), true, pEvent->nKey));
				return true;
			case ZACTION_MELEE_WEAPON:
			{
				if (!ZGetGame()->IsReplay())
					ZGetGameInterface()->ChangeWeapon(ZCWT_MELEE);
			}
			return true;
			case ZACTION_PRIMARY_WEAPON:
			{
				if (!ZGetGame()->IsReplay())
					ZGetGameInterface()->ChangeWeapon(ZCWT_PRIMARY);
			}
			return true;
			case ZACTION_SECONDARY_WEAPON:
			{
				if (!ZGetGame()->IsReplay())
					ZGetGameInterface()->ChangeWeapon(ZCWT_SECONDARY);
			}
			return true;
			case ZACTION_ITEM1:
			case ZACTION_ITEM2:
			{
				int nIndex = pEvent->nKey - ZACTION_ITEM1 + ZCWT_CUSTOM1;
				if (!ZGetGame()->IsReplay()) {
					ZGetGameInterface()->ChangeWeapon(ZChangeWeaponType(nIndex));
				}
			}
			return true;
			case ZACTION_COMMUNITYITEM1:	mlog("Community Item1 Selected!\n"); return true;
			case ZACTION_COMMUNITYITEM2:	mlog("Community Item2 Selected!\n"); return true;
			case ZACTION_PREV_WEAPON:
			{
				if (!ZGetGame()->IsReplay())
					ZGetGameInterface()->ChangeWeapon(ZCWT_PREV);
			}
			return true;
			case ZACTION_NEXT_WEAPON:
			{
				if (!ZGetGame()->IsReplay())
					ZGetGameInterface()->ChangeWeapon(ZCWT_NEXT);
			}
			return true;
			case ZACTION_RELOAD:
			{
				if (!ZGetGame()->IsReplay())
					ZGetGameInterface()->Reload();
			}
			return true;
			case ZACTION_DEFENCE:
			{
				if (ZGetGame()->m_pMyCharacter && !ZGetGame()->IsReplay())
					ZGetGame()->m_pMyCharacter->m_statusFlags.Ref().m_bGuardKey = true;
			}
			return true;

			case ZACTION_TAUNT:
			case ZACTION_BOW:
			case ZACTION_WAVE:
			case ZACTION_LAUGH:
			case ZACTION_CRY:
			case ZACTION_DANCE:
			{
				if (ZGetGame()->IsReplay())
					break;
				if (MEvent::GetShiftState())
					break;
				if (ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
					break;

				ZC_SPMOTION_TYPE mtype;

				if (pEvent->nKey == ZACTION_TAUNT) mtype = ZC_SPMOTION_TAUNT;
				else if (pEvent->nKey == ZACTION_BOW) mtype = ZC_SPMOTION_BOW;
				else if (pEvent->nKey == ZACTION_WAVE) mtype = ZC_SPMOTION_WAVE;
				else if (pEvent->nKey == ZACTION_LAUGH) mtype = ZC_SPMOTION_LAUGH;
				else if (pEvent->nKey == ZACTION_CRY) mtype = ZC_SPMOTION_CRY;
				else if (pEvent->nKey == ZACTION_DANCE) mtype = ZC_SPMOTION_DANCE;
				else
					return true;

				if (ZGetGame())
					ZGetGame()->PostSpMotion(mtype);
			}
			return true;

			case ZACTION_RECORD:
			{
				if (ZGetGame() && !ZGetGame()->IsReplay())
					ZGetGame()->ToggleRecording();
			}
			return true;
			case ZACTION_MOVING_PICTURE:
			{
				if (ZGetGameInterface()->GetBandiCapturer() != NULL)
					ZGetGameInterface()->GetBandiCapturer()->ToggleStart();
			}
			return true;
			case ZACTION_TOGGLE_CHAT:
			{
				if (ZGetCombatInterface()->IsShowUI())
				{
					if (ZGetGame())
					{
						ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
						ZGetSoundEngine()->PlaySound("if_error");
						pCombatInterface->ShowChatOutput(!ZGetConfiguration()->GetViewGameChat());
					}
				}
			}
			return true;
			case ZACTION_USE_WEAPON:
			case ZACTION_USE_WEAPON2:
			{
				return true;
			}

			case ZACTION_SENSITIVITY_INC:
			case ZACTION_SENSITIVITY_DEC:
			{
				int nPrev = ZGetConfiguration()->GetMouseSensitivityInInt();
				float senstivity = Z_MOUSE_SENSITIVITY;

				if (pEvent->nKey == ZACTION_SENSITIVITY_INC)
					senstivity += 0.01f;
				else
					senstivity -= 0.01f;

				ZGetConfiguration()->SetMouseSensitivityInFloat(senstivity);

				int nNew = ZGetConfiguration()->GetMouseSensitivityInInt();

				ZGetConfiguration()->ReserveSave();
				ZChatOutputMouseSensitivityChanged(nPrev, nNew);
				return true;
			}
			}
		}
		break;

	case MWM_KEYDOWN:
	{
		ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();

		switch (pEvent->nKey)
		{
		case VK_F1:
		case VK_F2:
		case VK_F3:
		case VK_F4:
		case VK_F5:
		case VK_F6:
		case VK_F7:
		case VK_F8:

			if (pEvent->nKey == VK_F1) sel = 0;
			else if (pEvent->nKey == VK_F2) sel = 1;
			else if (pEvent->nKey == VK_F3) sel = 2;
			else if (pEvent->nKey == VK_F4) sel = 3;
			else if (pEvent->nKey == VK_F5) sel = 4;
			else if (pEvent->nKey == VK_F6) sel = 5;
			else if (pEvent->nKey == VK_F7) sel = 6;
			else if (pEvent->nKey == VK_F8) sel = 7;

			if (ZGetConfiguration()) {
				char* str = ZGetConfiguration()->GetMacro()->GetString(sel);

				if (str) {
					if (ZApplication::GetGameInterface())
						if (ZApplication::GetGameInterface()->GetChat())
						{
							// Custom: Macro spam fix
							if (MEvent::GetShiftState())
								break;

							if (timeGetTime() - g_dwMacroTime > 2000) //Custom: Macro Spam Delay here
							{
								ZApplication::GetGameInterface()->GetChat()->Input(str);
								g_dwMacroTime = timeGetTime();
							}
							else
							{
								break; //not sure if this was neccessary but put it in.
							}
						}
				}
			}
			return true;

		case VK_F9:
		{
			char szName[128];
			{
				if (ZGetGame()->GetMatch()->IsTeamPlay())
				{
					sprintf(szName, "!(HP: %d / %d AP: %d / %d)", (int)ZGetGame()->m_pMyCharacter->GetHP(), (int)ZGetGame()->m_pMyCharacter->GetMaxHP(), (int)ZGetGame()->m_pMyCharacter->GetAP(), (int)ZGetGame()->m_pMyCharacter->GetMaxAP());
					ZApplication::GetGameInterface()->GetChat()->Input(szName);
					return true;
				}
				else
				{
					sprintf(szName, "(HP: %d / %d AP: %d / %d)", (int)ZGetGame()->m_pMyCharacter->GetHP(), (int)ZGetGame()->m_pMyCharacter->GetMaxHP(), (int)ZGetGame()->m_pMyCharacter->GetAP(), (int)ZGetGame()->m_pMyCharacter->GetMaxAP());
					ZApplication::GetGameInterface()->GetChat()->Input(szName);
					return true;
				}
			}
		}
		return true;

		case VK_RETURN:
		case VK_OEM_2:
		{
			if (!ShowCombatInputChat()) return false;
		}
		return true;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':

		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'Y':
		case 'N':
			if (pCombatInterface->GetObserver()->IsVisible())
				pCombatInterface->GetObserver()->OnKeyEvent(pEvent->bCtrl, pEvent->nKey);

			if (ZGetGameClient()->CanVote() ||
				ZGetGameInterface()->GetCombatInterface()->GetVoteInterface()->GetShowTargetList())
			{
				ZGetGameInterface()->GetCombatInterface()->GetVoteInterface()->VoteInput(pEvent->nKey);
			}
			break;
		case VK_ESCAPE:
			if (ZGetGameInterface()->GetCombatInterface()->GetVoteInterface()->GetShowTargetList()) {
				ZGetGameInterface()->GetCombatInterface()->GetVoteInterface()->CancelVote();
			}
			else {
				ZGetGameInterface()->ShowMenu(!ZGetGameInterface()->IsMenuVisible());
				ZGetGameInterface()->Show112Dialog(false);
			}

			return true;
		case 'M':
			if (ZGetGame()->IsReplay() && pCombatInterface->GetObserver()->IsVisible())
			{
				if (ZGetGameInterface()->GetCamera()->GetLookMode() == ZCAMERA_FREELOOK)
					ZGetGameInterface()->GetCamera()->SetLookMode(ZCAMERA_MINIMAP);
				else
					ZGetGameInterface()->GetCamera()->SetLookMode(ZCAMERA_FREELOOK);
			}
			break;
		case 'T':
			if (ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_SPECTATOR &&
				ZGetGame()->GetMatch()->IsTeamPlay() &&
				pCombatInterface->GetObserver()->IsVisible()) {
				ZObserver* pObserver = pCombatInterface->GetObserver();
				pObserver->SetType(pObserver->GetType() == ZOM_BLUE ? ZOM_RED : ZOM_BLUE);
				pObserver->ChangeToNextTarget();
			}
		case 'H':
			if (ZGetGame()->IsReplay() && pCombatInterface->GetObserver()->IsVisible())
			{
				if (ZGetGame()->IsShowReplayInfo())
					ZGetGame()->ShowReplayInfo(false);
				else
					ZGetGame()->ShowReplayInfo(true);
			}
			break;
		}
	}
	break;

	case MWM_CHAR:
	{
		ZMatch* pMatch = ZGetGame()->GetMatch();
		if (pMatch->IsTeamPlay()) {
			switch (pEvent->nKey) {
			case '\'':
			case '\"':
			{
				ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
				pCombatInterface->EnableInputChat(true, true);
			}
			return true;
			};
		}

		if (pEvent->nKey == '/') {
			if (!ShowCombatInputChat()) return false;
		}
	}
	break;

	case MWM_SYSKEYDOWN:
	{
		if (pEvent->nKey == 90) {
			ZGetCombatInterface()->SetIsShowUI(!ZGetCombatInterface()->IsShowUI());
			if (ZGetGame())
			{
				ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
				ZGetSoundEngine()->PlaySound("if_error");
				pCombatInterface->ShowChatOutput(ZGetCombatInterface()->IsShowUI());
			}
		}
	}
	break;

	case MWM_MOUSEWHEEL:
	{
		if (ZGetGame()->IsReplay())
			break;

		int nDelta = pEvent->nDelta;

		if ((ZGetMyInfo()->IsAdminGrade() && ZGetCombatInterface()->GetObserver()->IsVisible()) ||
			(!ZGetGameInterface()->m_bViewUI))
		{
			ZCamera* pCamera = ZGetGameInterface()->GetCamera();
			pCamera->m_fDist += -(float)nDelta;
			pCamera->m_fDist = max(CAMERA_DIST_MIN, pCamera->m_fDist);
			pCamera->m_fDist = min(CAMERA_DIST_MAX, pCamera->m_fDist);
			break;
		}
	}break;

	case MWM_MOUSEMOVE:
	{
		if (ZGetGameInterface()->IsCursorEnable() == false)
		{
			return true;
		}
	}
	break;
	}

	return false;
}

bool ZGameInput::ShowCombatInputChat()
{
	if (ZGetCombatInterface()->IsShowUI() && !ZGetCombatInterface()->IsShowResult())
	{
		ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
		if ((pCombatInterface) && (!pCombatInterface->IsChat()) && !ZGetGame()->IsReplay())
		{
			MWidget* pWidget = ZGetGameInterface()->GetIDLResource()->FindWidget("112Confirm");
			if (pWidget && pWidget->IsVisible()) return false;
			pCombatInterface->EnableInputChat(true);
		}
	}
	return true;
}

#include "MTextArea.h"

void ZGameInput::Update(float fElapsed)
{
	const ZCharaterStatusBitPacking& uStatus = ZGetGame()->m_pMyCharacter->m_dwStatusBitPackingValue.Ref();
	ZMyCharaterStatusBitPacking& zStatus = ZGetGame()->m_pMyCharacter->m_statusFlags.Ref();

	if (RIsActive())
	{
		ZCamera* pCamera = ZGetGameInterface()->GetCamera();
		ZMyCharacter* pMyCharacter = ZGetGame()->m_pMyCharacter;
		if ((!pMyCharacter) || (!pMyCharacter->GetInitialized())) return;

		if (!ZGetGameInterface()->IsCursorEnable())
		{
			{
				float fRotateX = 0;
				float fRotateY = 0;

#ifdef _DONOTUSE_DINPUT_MOUSE
				int iDeltaX, iDeltaY;

				POINT pt;
				GetCursorPos(&pt);
				ScreenToClient(g_hWnd, &pt);
				iDeltaX = pt.x - RGetScreenWidth() / 2;
				iDeltaY = pt.y - RGetScreenHeight() / 2;

				float fRotateStep = 0.0005f * Z_MOUSE_SENSITIVITY * 10.0f;
				fRotateX = (iDeltaX * fRotateStep);
				fRotateY = (iDeltaY * fRotateStep);

#else
				ZGetInput()->GetRotation(&fRotateX, &fRotateY);
#endif

				bool bRotateEnable = false;
				if (!zStatus.m_bSkill && !uStatus.m_bWallJump && !uStatus.m_bWallJump2 && !zStatus.m_bWallHang &&
					!uStatus.m_bTumble && !uStatus.m_bBlast && !uStatus.m_bBlastStand && !uStatus.m_bBlastDrop)
					bRotateEnable = true;
				if (pMyCharacter->IsDie()) bRotateEnable = true;

				if (RIsActive())
				{
					ZCamera* pCamera = ZGetGameInterface()->GetCamera();

					pCamera->m_fAngleX += fRotateY;
					pCamera->m_fAngleZ += fRotateX;

					if (pCamera->GetLookMode() == ZCAMERA_MINIMAP) {
						pCamera->m_fAngleX = max(pi / 2 + .1f, pCamera->m_fAngleX);
						pCamera->m_fAngleX = min(pi - 0.1f, pCamera->m_fAngleX);
					}
					else {
						static float lastanglex, lastanglez;
						if (bRotateEnable)
						{
							pCamera->m_fAngleZ = fmod(pCamera->m_fAngleZ, 2 * PI);
							pCamera->m_fAngleX = fmod(pCamera->m_fAngleX, 2 * PI);

							pCamera->m_fAngleX = max(CAMERA_ANGLEX_MIN, pCamera->m_fAngleX);
							pCamera->m_fAngleX = min(CAMERA_ANGLEX_MAX, pCamera->m_fAngleX);

							lastanglex = pCamera->m_fAngleX;
							lastanglez = pCamera->m_fAngleZ;
						}
						else
						{
							pCamera->m_fAngleX = max(CAMERA_ANGLEX_MIN, pCamera->m_fAngleX);
							pCamera->m_fAngleX = min(CAMERA_ANGLEX_MAX, pCamera->m_fAngleX);

							pCamera->m_fAngleX = max(lastanglex - pi / 4.f, pCamera->m_fAngleX);
							pCamera->m_fAngleX = min(lastanglex + pi / 4.f, pCamera->m_fAngleX);

							pCamera->m_fAngleZ = max(lastanglez - pi / 4.f, pCamera->m_fAngleZ);
							pCamera->m_fAngleZ = min(lastanglez + pi / 4.f, pCamera->m_fAngleZ);
						}
					}

					ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
					if (pCombatInterface && !pCombatInterface->IsChat() &&
						(pCamera->GetLookMode() == ZCAMERA_FREELOOK || pCamera->GetLookMode() == ZCAMERA_MINIMAP))
					{
						rvector right;
						rvector forward = RCameraDirection;
						CrossProduct(&right, rvector(0, 0, 1), forward);
						Normalize(right);
						const rvector up = rvector(0, 0, 1);

						rvector accel = rvector(0, 0, 0);

						if (ZIsActionKeyPressed(ZACTION_FORWARD) == true)	accel += forward;
						if (ZIsActionKeyPressed(ZACTION_BACK) == true)		accel -= forward;
						if (ZIsActionKeyPressed(ZACTION_LEFT) == true)		accel -= right;
						if (ZIsActionKeyPressed(ZACTION_RIGHT) == true)	accel += right;
						if (ZIsActionKeyPressed(ZACTION_JUMP) == true)		accel += up;
						if (ZIsActionKeyPressed(ZACTION_USE_WEAPON) == true)			accel -= up;

						rvector cameraMove =
							(pCamera->GetLookMode() == ZCAMERA_FREELOOK ? 1000.f : 10000.f)
							* fElapsed * accel;

						rvector targetPos = pCamera->GetPosition() + cameraMove;

						if (pCamera->GetLookMode() == ZCAMERA_FREELOOK)
							ZGetGame()->GetWorld()->GetBsp()->CheckWall(pCamera->GetPosition(), targetPos, ZFREEOBSERVER_RADIUS, 0.f, RCW_SPHERE);
						else
						{
							rboundingbox* pbb = &ZGetGame()->GetWorld()->GetBsp()->GetRootNode()->bbTree;
							targetPos.x = max(min(targetPos.x, pbb->maxx), pbb->minx);
							targetPos.y = max(min(targetPos.y, pbb->maxy), pbb->miny);

							ZMiniMap* pMinimap = ZGetGameInterface()->GetMiniMap();
							if (pMinimap)
								targetPos.z = max(min(targetPos.z, pMinimap->GetHeightMax()), pMinimap->GetHeightMin());
							else
								targetPos.z = max(min(targetPos.z, 7000), 2000);
						}

						pCamera->SetPosition(targetPos);
					}
					else if (!ZGetGame()->IsReplay())
					{
						pMyCharacter->ProcessInput(fElapsed);
					}
				}
			}
			POINT pt = { RGetScreenWidth() / 2,RGetScreenHeight() / 2 };
			ClientToScreen(g_hWnd, &pt);
			SetCursorPos(pt.x, pt.y);

			GameCheckSequenceKeyCommand();
		}
		else
			pMyCharacter->ReleaseButtonState();
	}
}

#define MAX_KEY_SEQUENCE_TIME	2.f

void ZGameInput::GameCheckSequenceKeyCommand()
{
	while (m_ActionKeyHistory.size() > 0 && (ZGetGame()->GetTime() - (*m_ActionKeyHistory.begin()).fTime > MAX_KEY_SEQUENCE_TIME))
	{
		m_ActionKeyHistory.erase(m_ActionKeyHistory.begin());
	}

	// Custom: Iterator decrement fix
	if (m_ActionKeyHistory.size())
	{
		for (int ai = 0; ai < (int)m_SequenceActions.size(); ai++)
		{
			ZKEYSEQUENCEACTION action = m_SequenceActions.at(ai);

			list<ZACTIONKEYITEM>::reverse_iterator itr = m_ActionKeyHistory.rbegin();

			bool bAction = true;
			for (int i = action.nKeyCount - 1; i >= 0; --i)
			{
				// fix
				if (itr == m_ActionKeyHistory.rend())
				{
					bAction = false;
					break;
				}

				ZACTIONKEYITEM itm = *itr;

				if (i == 0)
				{
					if (ZGetGame()->GetTime() - itm.fTime > action.fTotalTime)
					{
						bAction = false;
						break;
					}
				}
				if (itm.nActionKey != action.pKeys[i].nActionKey || itm.bPressed != action.pKeys[i].bPressed)
				{
					bAction = false;
					break;
				}
				if (i != 0 && itr == m_ActionKeyHistory.rend())
				{
					bAction = false;
					break;
				}

				++itr;
			}

			if (bAction)
			{
				if (m_ActionKeyHistory.size())
					m_ActionKeyHistory.clear();

				if (ai >= 0 && ai <= 3)
					ZGetGame()->m_pMyCharacter->OnTumble(ai);
			}
		}
	}
}