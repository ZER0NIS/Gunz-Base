#include "stdafx.h"
#include "ZRuleDuelTournament.h"
#include "ZScreenEffectManager.h"
#include "ZCharacterView.h"
#include "ZItemSlotView.h"
#include "ZBmNumLabel.h"

//# 룰 클래스에 UI관련 기능이 있는것이 이상합니다만, 적어도 ZCombatInterface가 계속 비대해지는 것보단 나을 것 같습니다.

ZRuleDuelTournament::ZRuleDuelTournament(ZMatch* pMatch) : ZRule(pMatch)
{
	memset(&m_DTGameInfo, 0, sizeof(m_DTGameInfo));
	memset(&m_prevPlayerInfo, 0, sizeof(m_prevPlayerInfo));
	memset(&m_nextPlayerInfo, 0, sizeof(m_nextPlayerInfo));

	m_nDTRoundCount = 0;
	m_nDTPlayCount= 1;

	InitCharacterList();

//#ifdef _DUELTOURNAMENT
	m_fBlinkOpacity = 255;

	m_bSlideToCenter = true;
	m_fSlideElapsedTime = -1;
//#endif

	m_bFirstPreCountdown = true;
	m_dwTimeEnterPreCountDown = 0;
	m_ePreCountdownDetail = PCD_NOT_PRECOUNTDOWN;
}

ZRuleDuelTournament::~ZRuleDuelTournament()
{
}


bool ZRuleDuelTournament::OnCommand(MCommand* pCommand)
{
	//char buf[256];
	//sprintf(buf,"ZRuleDuelTournament[ID:%d]\n", pCommand->GetID());
	//mlog(buf);

	if (!ZGetGame()) return false;

	switch (pCommand->GetID())
	{
	case MC_MATCH_DUELTOURNAMENT_GAME_NEXT_MATCH_PLYAERINFO:
		m_prevPlayerInfo = m_nextPlayerInfo;
		pCommand->GetParameter(&m_nextPlayerInfo,	0, MPT_BLOB);
		return true;

	case MC_MATCH_DUELTOURNAMENT_GAME_INFO:
		{
			pCommand->GetParameter(&m_DTGameInfo,	0, MPT_BLOB);

			if (m_DTGameInfo.bIsRoundEnd)
			{
				rvector pos = ZGetGame()->m_pMyCharacter->GetPosition();
				rvector dir = ZGetGame()->m_pMyCharacter->m_DirectionLower;

				if ((m_DTGameInfo.uidPlayer1 == ZGetMyUID()) || (m_DTGameInfo.uidPlayer2 == ZGetMyUID())){
					ZMapSpawnData* pSpawnData = ZGetGame()->GetMapDesc()->GetSpawnManager()->GetData( m_DTGameInfo.uidPlayer1 == ZGetMyUID() ? 0 : 1);

					if( pSpawnData != NULL ){
						ZPostRequestSpawn(ZGetMyUID(), pSpawnData->m_Pos, pSpawnData->m_Dir);
						ZGetGame()->SetSpawnRequested(true);
					}
				} else {
					ZCharacter* cha = ZGetCharacterManager()->Find(m_DTGameInfo.uidPlayer1);
					if (cha != NULL) cha->Revival();

					cha = ZGetCharacterManager()->Find(m_DTGameInfo.uidPlayer2);
					if (cha != NULL) cha->Revival();

					ZGetCombatInterface()->SetObserverMode(true);
				}
			}

			if ((m_DTGameInfo.uidPlayer1 != ZGetGame()->m_pMyCharacter->GetUID()) 
				&& (m_DTGameInfo.uidPlayer2 != ZGetGame()->m_pMyCharacter->GetUID())) 
			{
				ZGetCombatInterface()->SetObserverMode(true);

				ZCharacter *pCharacter = ZGetCharacterManager()->Find(ZGetMyUID());
				if (pCharacter)
				{
					pCharacter->SetVisible(false);
					pCharacter->ForceDie();
				}
			}
		}
		return true;

	case MC_MATCH_DUELTOURNAMENT_GAME_ROUND_RESULT_INFO :
		{
			MTD_DuelTournamentRoundResultInfo DTGameResultInfo;
			pCommand->GetParameter(&DTGameResultInfo,	0, MPT_BLOB);

			SetRoundResultInfo(DTGameResultInfo);
		}
		return true;

	case MC_MATCH_DUELTOURNAMENT_GAME_MATCH_RESULT_INFO :
		{
			MTD_DuelTournamentMatchResultInfo DTGameResultInfo;
			pCommand->GetParameter(&DTGameResultInfo,	0, MPT_BLOB);

			//////////////////////////////////////////////////// LOG ////////////////////////////////////////////////////
#ifdef _DUELTOURNAMENT_LOG_ENABLE_
			char szbuf[32] = {0, };
			switch((MDUELTOURNAMENTROUNDSTATE)DTGameResultInfo.nMatchType)
			{
			case MDUELTOURNAMENTROUNDSTATE_FINAL:		sprintf(szbuf, "ROUND_FINAL \n");		break;
			case MDUELTOURNAMENTROUNDSTATE_SEMIFINAL:	sprintf(szbuf, "ROUND_SEMIFINAL \n");	break;
			case MDUELTOURNAMENTROUNDSTATE_QUATERFINAL:	sprintf(szbuf, "ROUND_QUATERFINAL \n");	break;
			default:									sprintf(szbuf, "ROUND_FAIL \n");		break;
			}
			mlog("[GAME_MATCH_RESULT] RoundType:%s, MatchNumber:%d, Winner(%d:%d), Loser(%d:%d), GainTP:%d, LoseTP:%d \n", 
				szbuf, DTGameResultInfo.nMatchNumber, DTGameResultInfo.uidWinnerPlayer.High, DTGameResultInfo.uidWinnerPlayer.Low,
				DTGameResultInfo.uidLoserPlayer.High, DTGameResultInfo.uidLoserPlayer.Low, DTGameResultInfo.nGainTP, DTGameResultInfo.nLoseTP);
#endif
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////

			SetMatchResultInfo(DTGameResultInfo);
		}
		return true;
	}

	return false;
}

void ZRuleDuelTournament::AfterCommandProcessed( MCommand* pCommand )
{
	switch (pCommand->GetID())
	{
	case MC_PEER_OPENED:
		{
			// ZGame에서 MC_PEER_OPENED를 처리할 때 해당 피어의 캐릭터를 SetVisible(true) 해버린다
			// 듀얼토너먼트 시작시 모든 캐릭터를 SetVisible(false)로 만드는데, MC_PEER_OPENED가 약간 늦게 들어온 경우
			// 캐릭터가 맵 중앙에 멍하니 떠 있게 된다. 이 현상을 없애기 위해 ZGame의 커맨드 처리 후에 다시 강제로 캐릭터를 숨겨준다
			MUID uidChar;
			pCommand->GetParameter(&uidChar, 0, MPT_UID);

			if (!ZGetCharacterManager()) break;
			ZCharacter* pCharacter = ZGetCharacterManager()->Find(uidChar);
			if (!pCharacter) break;

			// 게임중일땐 이 피어가 현재 출전자가 아니어야만 숨긴다
			if (ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PLAY)
			{
				if (uidChar != m_nextPlayerInfo.uidPlayer1 &&
					uidChar != m_nextPlayerInfo.uidPlayer2)
					pCharacter->SetVisible(false);
			}
			else
			{
				pCharacter->SetVisible(false);
			}
		}
		break;
		
	}
}

void ZRuleDuelTournament::InitCharacterList()
{
	ZeroMemory(&m_QuaterFinalPlayer, sizeof(m_QuaterFinalPlayer));
	//ZeroMemory(&m_DTNextPlayer, sizeof(m_DTNextPlayer));
	ZeroMemory(&m_DTChampion, sizeof(m_DTChampion));

	MDUELTOURNAMENTTYPE eType = ZApplication::GetGameInterface()->GetDuelTournamentType();
	vector<DTPlayerInfo> vecPlayerInfo = ZApplication::GetGameInterface()->GetVectorDTPlayerInfo();
	if(vecPlayerInfo.size() <= 0)	return;
	if(vecPlayerInfo.size() != GetDTPlayerCount(eType)) { }

	vector<DTPlayerInfo>::iterator it = vecPlayerInfo.begin();
	for(int i=0; it != vecPlayerInfo.end(); ++it, ++i)	// 총 8명 세팅
	{
		strcpy(m_QuaterFinalPlayer[i].m_szCharName, (*it).m_szCharName);
		m_QuaterFinalPlayer[i].uidPlayer = (*it).uidPlayer;
		m_QuaterFinalPlayer[i].m_nTP = (*it).m_nTP;
		m_QuaterFinalPlayer[i].nNumber = i;

		switch(eType)
		{
		case MDUELTOURNAMENTTYPE_FINAL:			m_QuaterFinalPlayer[i].nMatchLevel = 2;		break;
		case MDUELTOURNAMENTTYPE_SEMIFINAL:		m_QuaterFinalPlayer[i].nMatchLevel = 1;		break;
		case MDUELTOURNAMENTTYPE_QUATERFINAL:	m_QuaterFinalPlayer[i].nMatchLevel = 0;		break;
		}
	}

	switch(eType)
	{
	case MDUELTOURNAMENTTYPE_FINAL:			m_eDTRoundState = MDUELTOURNAMENTROUNDSTATE_FINAL;			break;
	case MDUELTOURNAMENTTYPE_SEMIFINAL:		m_eDTRoundState = MDUELTOURNAMENTROUNDSTATE_SEMIFINAL;		break;
	case MDUELTOURNAMENTTYPE_QUATERFINAL:	m_eDTRoundState = MDUELTOURNAMENTROUNDSTATE_QUATERFINAL;	break;
	}
}

void ZRuleDuelTournament::SetRoundResultInfo(MTD_DuelTournamentRoundResultInfo& DTGameResultInfo)
{
	///////////////////////////////////////////////////////////
	// Notice 
	// bDraw는 비겼을 때, True이다.
	// bIsMatchFinish는 Match가 종료되었을 때, True이다.
	// 다음과 같은 경우의 수가 존재할 수 있다.
	// bDraw = true,  bIsMatchFinish = true  => 플레이어 두명이 모두 이탈했을 경우
	// bDraw = true,  bIsMatchFinish = false => 동반 자살로 인하여 비겼을 경우
	// bDraw = false, bIsMatchFinish = true  => 그냥 한명이 이겼을 경우
	// bDraw = false, bIsMatchFinish = false => 그냥 한명이 이겼는데, 매치가 계속 진행되야될 경우(4강, 결승)
	///////////////////////////////////////////////////////////
	DuelTournamentPlayer* pPlayer1 = GetPlayer(m_nextPlayerInfo.uidPlayer1);
	DuelTournamentPlayer* pPlayer2 = GetPlayer(m_nextPlayerInfo.uidPlayer2);

	if(DTGameResultInfo.bDraw)
	{	// 드로우
		if(DTGameResultInfo.bIsMatchFinish)
		{	// 두명 모두 이탈
		}
		else
		{	// 동반 자살
			if(pPlayer1 && pPlayer2)
			{
				if(pPlayer1->nVictory == 1 && pPlayer2->nVictory == 1)
				{}	// 둘다 1승일때 동반 자살이면 카운트 해주지 않는다.(무효게임 처리)
				else
				{
					++pPlayer1->nVictory;
					++pPlayer2->nVictory;
				}
			}
		}
	}
	else
	{	// 승패가 갈림
		DuelTournamentPlayer* pPlayer = GetPlayer(DTGameResultInfo.uidWinnerPlayer);
		if(pPlayer)
		{
			++pPlayer->nVictory;

			if(DTGameResultInfo.bIsTimeOut)
			{ //if(만약 KO없이 시간제로 끝났다면)
				char szTemp[256];
				ZTransMsg(szTemp, MSG_GAME_DUELTOURNAMENT_MATCH_VICTORY, 1, pPlayer->m_szCharName);
				ZChatOutput(MCOLOR(255, 200, 200), szTemp);
			}
		}
		else
		{
#ifdef _DUELTOURNAMENT_LOG_ENABLE_
			mlog("WINNER NOT FIND [player1(%d:%d)%s][player2(%d:%d)%s] Winner(%d:%d)\n", 
				pPlayer1->uidPlayer.High, pPlayer1->uidPlayer.Low, pPlayer1->m_szCharName, 
				pPlayer2->uidPlayer.High, pPlayer2->uidPlayer.Low, pPlayer2->m_szCharName, 
				DTGameResultInfo.uidWinnerPlayer.High, DTGameResultInfo.uidWinnerPlayer.Low);
#endif
		}

		if(DTGameResultInfo.bIsMatchFinish){}// 승패 갈림(게임 끝)// 여기에 오면 무조건 챔피언
		else{}// 승패 갈림(다음 경기 있음)
	}

	// 듀얼 토너먼트가 끝날때마다(승,패,드로우) 처리해준다.
	++m_nDTPlayCount;

	//////////////////////////////////////////////////// LOG ////////////////////////////////////////////////////
#ifdef _DUELTOURNAMENT_LOG_ENABLE_
	mlog("[GAME_ROUND_RESULT] Draw:%s, MatchFinish:%s, Winner(%d:%d), Loser(%d:%d)\n", DTGameResultInfo.bDraw?"true":"false", DTGameResultInfo.bIsMatchFinish?"true":"false", 
		DTGameResultInfo.uidWinnerPlayer.High, DTGameResultInfo.uidWinnerPlayer.Low, DTGameResultInfo.uidLoserPlayer.High, DTGameResultInfo.uidLoserPlayer.Low);
#endif
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void ZRuleDuelTournament::SetMatchResultInfo(MTD_DuelTournamentMatchResultInfo& DTGameResultInfo)
{
	DuelTournamentPlayer winnerPlayer;
	bool bFindPlayer = false;
	for(int i=0; i<8; ++i)
	{
		if(m_QuaterFinalPlayer[i].uidPlayer == DTGameResultInfo.uidWinnerPlayer)
		{
			winnerPlayer = m_QuaterFinalPlayer[i];
			m_DTChampion = winnerPlayer;
			bFindPlayer = true;
		}
		// 졌으면 낙오자(죽음)로 처리
		if(m_QuaterFinalPlayer[i].uidPlayer == DTGameResultInfo.uidLoserPlayer)
		{
			if (ZGetMyUID() == DTGameResultInfo.uidLoserPlayer)
			{
				RestoreStageExitButtonConfirmMessage();
			}
		}
	}
#ifndef _PUBLISH
	if(bFindPlayer == false)
	{
		char buf[256] = {0, };
		sprintf(buf, "QuaterFinalPlayer 리스트에서 승리자(%d:%d)를 못찾음", DTGameResultInfo.uidWinnerPlayer.High, DTGameResultInfo.uidWinnerPlayer.Low);
		mlog(buf);
		ZApplication::GetGameInterface()->ShowMessage(buf);
	}
#endif

	int nMatchLevel = 0;
	switch(m_eDTRoundState)
	{
	case MDUELTOURNAMENTROUNDSTATE_FINAL:				nMatchLevel = 3;	break;
	case MDUELTOURNAMENTROUNDSTATE_SEMIFINAL:			nMatchLevel = 2;	break;
	case MDUELTOURNAMENTROUNDSTATE_QUATERFINAL:			nMatchLevel = 1;	break;
	}

	bool bMatchResult = false;
	// 승리자 세팅(여기서 걸리는게 없으면 에러)
	for(int i=0; i<8; ++i)
	{
		if(m_QuaterFinalPlayer[i].uidPlayer == winnerPlayer.uidPlayer)
		{
			m_QuaterFinalPlayer[i] = winnerPlayer;
			m_QuaterFinalPlayer[i].nMatchLevel = nMatchLevel;
			m_QuaterFinalPlayer[i].nVictory = 0;	// 매치가 끝나면 다음 매치를 위해 승리포인트 초기화
			bMatchResult = true; break;
		}
	}

	m_nDTPlayCount = 1;			// 라운드가 끝났으면 다시 1경기부터 시작
	++m_nDTRoundCount; // 다음 승부
	if(m_nDTRoundCount == GetDTRoundCount(m_eDTRoundState))
	{	// 현재 라운드 마지막 승부까지 갔으면, 다음 라운드로 변경해준다.
		if((MDUELTOURNAMENTROUNDSTATE)DTGameResultInfo.nMatchType == MDUELTOURNAMENTROUNDSTATE_FINAL)
			m_eDTRoundState = MDUELTOURNAMENTROUNDSTATE_MAX;
		else
			m_eDTRoundState = (MDUELTOURNAMENTROUNDSTATE)--DTGameResultInfo.nMatchType;
		m_nDTRoundCount = 0;
		++nMatchLevel;
	}

	if(!bMatchResult)
	{	// 만약 대전 결과가 없다면
#ifdef _DUELTOURNAMENT_LOG_ENABLE_
		char szbufName[512] = {0, };
		sprintf(szbufName, "QUATERFINAL ");
		for(int i=0; i<8; ++i)
			sprintf(szbufName, "[%dTh %s(%d:%d), Number:%d TP:%d, MatchLevel:%d, ]", 
			i, m_QuaterFinalPlayer[i].m_szCharName, m_QuaterFinalPlayer[i].uidPlayer.High, m_QuaterFinalPlayer[i].uidPlayer.Low,  
			m_QuaterFinalPlayer[i].nNumber, m_QuaterFinalPlayer[i].m_nTP, m_QuaterFinalPlayer[i].nMatchLevel);
		mlog(szbufName);

		char buf[256] = {0, };
		sprintf(buf, "클라이언트 리스트에서 승리자(%s)를 못찾음", winnerPlayer.m_szCharName);
		mlog(buf);
		//__asm int 3;
#endif
		//_ASSERT(0);
	}

	ShowWinLoseScreenEffect(DTGameResultInfo);
}


int ZRuleDuelTournament::GetQueueIdx(const MUID& uidChar)
{
	if (uidChar == m_DTGameInfo.uidPlayer1 || uidChar == m_DTGameInfo.uidPlayer2 ) return -1;
	for (int i = 0; i < m_DTGameInfo.nWaitPlayerListLength; i++)
		if (m_DTGameInfo.WaitPlayerList[i] == uidChar) return i;

	return -100;
}

void ZRuleDuelTournament::OnDraw(MDrawContext* pDC)
{
	// 남은 시간표시
	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDTInfo");
	ZBmNumLabel* pBmNumLabel = (ZBmNumLabel*) ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "DT_RemainTime");
	MWidget *pPicture = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "DM_Infinity_Time");
	if (pWidget && pBmNumLabel && pPicture)
	{
		DWORD dwTime = ZGetCombatInterface()->GetPlayTime();
		if (dwTime == -1)
			pWidget->Show(false);
		else if (dwTime == -2)
		{
			pWidget->Show(true);		// 시간에대한 프레임은 그려준다.
			pBmNumLabel->Show(false);	// 숫자는 그리지 않는다.
			pPicture->Show(true);		// 무한대 표시한다.
		}
		else 
		{
			pBmNumLabel->SetNumber( dwTime);
			pWidget->Show(true);
			pBmNumLabel->Show(true);
			pPicture->Show(false);		// 무한대 표시하지 않는다.
		}
	}
}

void ZRuleDuelTournament::OnUpdate( float fDelta )
{
	if (ZGetGame()->IsReplay())
		RestoreStageExitButtonConfirmMessage();

	if (m_ePreCountdownDetail == PCD_WINLOSE)
	{
		DWORD nowTime = timeGetTime();
		DWORD elapsedTime = nowTime - m_dwTimeEnterPreCountDown;

		DWORD timeToWinLoseClose = DUELTOURNAMENT_PRECOUNTDOWN_WINLOSE_SHOWTIME - 500; //0.5초
		DWORD timeToNextMatchOpen = DUELTOURNAMENT_PRECOUNTDOWN_WINLOSE_SHOWTIME + 500;
		
		// 승자패자 보여줄 시간이 다 찼을때즈음 UI를 닫았다가 nextMatch때 다시 열리도록 한다
		if (timeToWinLoseClose < elapsedTime && elapsedTime < timeToNextMatchOpen)
		{
			BeginPlayerInfoUISlideAni(false);
			ShowMatchPlayerInfoUI(false);
		}
		else if (timeToNextMatchOpen < elapsedTime)
		{
			SetPreCountdownDetail(PCD_NEXTMATCH);
		}
	}

	UpdateUISlideAni(fDelta);

	if (ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PLAY)
		ShowMatchPlayerInfoUI(false);	// 컴터에 부하가 걸린 경우 플레이때도 대전자정보 UI가 닫히지 않는 경우가 생기는 현상 억제;;
}

void ZRuleDuelTournament::OnSetRoundState( MMATCH_ROUNDSTATE roundState )
{
	switch (roundState)
	{
	case MMATCH_ROUNDSTATE_PRE_COUNTDOWN:
		{
			// 프리카운트다운때 승패표시 후 다음매치 소개로 넘어가는데 이게 첫매치라면 승패표시는 건너뛴다
			if (m_bFirstPreCountdown)
				SetPreCountdownDetail(PCD_NEXTMATCH);
			else
				SetPreCountdownDetail(PCD_WINLOSE);

			// 모든 캐릭터를 숨기고 조작불가 상태로
			for (ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.begin();
				itor != ZGetGame()->m_CharacterManager.end(); ++itor)
			{
				ZCharacter* pCharacter = (*itor).second;
				pCharacter->SetVisible(false);
			}

			m_bFirstPreCountdown = false;
		}
		break;
	case MMATCH_ROUNDSTATE_COUNTDOWN:
		{
			SetPreCountdownDetail(PCD_NOT_PRECOUNTDOWN);
		}
		break;
	case MMATCH_ROUNDSTATE_EXIT:
		{
			// 결승전이 끝났을땐 PreCountdown으로 안들어가므로 finish때 대전 플레이어의 정보를 보여준다
			BeginPlayerInfoUISlideAni(true);
			ShowMatchPlayerInfoUI(true);
			ShowMatchPlayerInfoUI_OnlyNextMatch(false);
		}
		break;
	}
}

void ZRuleDuelTournament::SetPreCountdownDetail(PRECOUNTDOWN_DETAIL eDetail)
{
	// 프리카운트다운 상태로 진입한 시각을 기록
	if (m_ePreCountdownDetail == PCD_NOT_PRECOUNTDOWN &&
		eDetail != PCD_NOT_PRECOUNTDOWN)
		m_dwTimeEnterPreCountDown = timeGetTime();

	m_ePreCountdownDetail = eDetail;


	switch(eDetail)
	{
	case PCD_NOT_PRECOUNTDOWN:
		BeginPlayerInfoUISlideAni(false);
		ShowMatchPlayerInfoUI(false);
		break;

	case PCD_WINLOSE:
		{
			SetMatchPlayerInfoUI(m_prevPlayerInfo.uidPlayer1, m_prevPlayerInfo.uidPlayer2);
			BeginPlayerInfoUISlideAni(true);
			ShowMatchPlayerInfoUI(true);
			ShowMatchPlayerInfoUI_OnlyNextMatch(false);
		}
		break;
	case PCD_NEXTMATCH:
		{
			SetMatchPlayerInfoUI(m_nextPlayerInfo.uidPlayer1, m_nextPlayerInfo.uidPlayer2);
			
			BeginPlayerInfoUISlideAni(true);
			ShowMatchPlayerInfoUI(true);
			ShowMatchPlayerInfoUI_OnlyNextMatch(true);
		}
		break;
	}
}

void ZRuleDuelTournament::ShowWinLoseScreenEffect(MTD_DuelTournamentMatchResultInfo& DTGameResultInfo)
{
	bool bLeftPlayerWin;
/*		 if (m_nextPlayerInfo.uidPlayer1 == DTGameResultInfo.uidLoserPlayer &&
			 m_nextPlayerInfo.uidPlayer2 == DTGameResultInfo.uidWinnerPlayer)
			bLeftPlayerWin = false;
	else if (m_nextPlayerInfo.uidPlayer1 == DTGameResultInfo.uidWinnerPlayer &&
			 m_nextPlayerInfo.uidPlayer2 == DTGameResultInfo.uidLoserPlayer)
			bLeftPlayerWin = true;
	else
		return;*/

		 if (m_nextPlayerInfo.uidPlayer2 == DTGameResultInfo.uidWinnerPlayer)
			 bLeftPlayerWin = false;
	else if (m_nextPlayerInfo.uidPlayer1 == DTGameResultInfo.uidWinnerPlayer)
			 bLeftPlayerWin = true;
	else
		return;

	// UI에 좌우 캐릭터가 표시되어 있고 각 캐릭터 위에 WIN/LOSE 이펙트를 올린다
	float winOffsetX = 200.f;
	float loseOffsetX = winOffsetX;

	if (bLeftPlayerWin)
		winOffsetX *= -1;
	else
		loseOffsetX *= -1;

	ZGetScreenEffectManager()->AddScreenEffect("win", rvector(winOffsetX, 0, 0));
	ZGetScreenEffectManager()->AddScreenEffect("lose", rvector(loseOffsetX, 0, 0));
}

void ZRuleDuelTournament::ShowMatchPlayerInfoUI_OnlyNextMatch(bool bShow)
{
	// VS 이미지와 몇강인지 표시는 다음 매치 예고때만 보여준다
	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDT_CharviewerVs");
	if (pWidget)
		pWidget->Show(bShow);

	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDT_MatchLevel");
	if (pWidget)
		pWidget->Show(bShow);

	char sz[128];
	char szRoundCount[64];
	sprintf(szRoundCount, "%d", m_nDTRoundCount+1);
	MDUELTOURNAMENTTYPE eType = ZApplication::GetGameInterface()->GetDuelTournamentType();
	switch(m_eDTRoundState)
	{
	case MDUELTOURNAMENTROUNDSTATE_FINAL:		ZTransMsg(sz, MSG_GAME_DUELTOURNAMENT_MATCH_FINAL);		break;
	case MDUELTOURNAMENTROUNDSTATE_SEMIFINAL:	ZTransMsg(sz, MSG_GAME_DUELTOURNAMENT_MATCH_SEMIFINAL, 1, szRoundCount);	break;
	case MDUELTOURNAMENTROUNDSTATE_QUATERFINAL:	ZTransMsg(sz, MSG_GAME_DUELTOURNAMENT_MATCH_QUATERFINAL, 1, szRoundCount);	break;
	default: sprintf(sz, ""); break;
	}

	if (pWidget)
		pWidget->SetText(sz);
}

void ZRuleDuelTournament::ShowMatchPlayerInfoUI(bool bShow)
{
	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDT_CharacterInfo");
	if (pWidget)
		pWidget->Show(bShow);

	// 대전자 정보를 보여줄땐 크로스헤어, 스크린이펙트, 기타 UI들을 그리지 않는다
	ZGetCombatInterface()->SetSkipUIDraw(bShow);
}

void ZRuleDuelTournament::SetVisiblePlayerInfoUI(bool bLeft, bool bShow)
{
	char szLeftRight[32] = "Left";
	if (!bLeft)
		strcpy(szLeftRight, "Right");

	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatDT_CharacterInfo");
	if (pWidget)
	{
		MWidget* pChild;
		int numChild = pWidget->GetChildCount();
		for (int i=0; i<numChild; ++i)
		{
			pChild = pWidget->GetChild(i);

			if (pChild && strstr(pChild->m_szIDLName, "CombatDT_") && strstr(pChild->m_szIDLName, szLeftRight))
				pChild->SetVisible(bShow);
		}
	}
}

void ZRuleDuelTournament::SetMatchPlayerInfoUI(const MUID& uidPlayer1, const MUID& uidPlayer2)
{
	// 매치 시작 전에 대전자 2명의 정보를 보여주는 위젯들 세팅
	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDT_CharacterInfo");
	if (!pWidget)
		return;

	ZCharacterProperty emptyProp;
	MTD_CharInfo emptyInfo;

	const ZCharacterProperty* pCharProp[2]  = {&emptyProp, &emptyProp};
	const MTD_CharInfo* pCharInfo[2]		= {&emptyInfo, &emptyInfo};

	int ap[2] = {DEFAULT_CHAR_AP, DEFAULT_CHAR_AP};
	int hp[2] = {DEFAULT_CHAR_HP, DEFAULT_CHAR_HP};

	ZCharacter* pChar1 = ZGetCharacterManager()->Find(uidPlayer1);
	ZCharacter* pChar2 = ZGetCharacterManager()->Find(uidPlayer2);
	if (pChar1) {
		pCharProp[0] = pChar1->GetProperty();
		pCharInfo[0] = pChar1->GetCharInfo();

		hp[0] = pChar1->GetMaxHP();
		ap[0] = pChar1->GetMaxAP();		
	}
	if (pChar2) {
		pCharProp[1] = pChar2->GetProperty();
		pCharInfo[1] = pChar2->GetCharInfo();

		hp[1] = pChar2->GetMaxHP();
		ap[1] = pChar2->GetMaxAP();
	}

	if (pCharProp[0] != &emptyProp && pCharInfo[0] != &emptyInfo)		SetVisiblePlayerInfoUI(true, true);		// left, show
	else																SetVisiblePlayerInfoUI(true, false);	// left, hide

	if (pCharProp[1] != &emptyProp && pCharInfo[1] != &emptyInfo)		SetVisiblePlayerInfoUI(false, true);	// right, show
	else																SetVisiblePlayerInfoUI(false, false);	// right, hide


	ZCharacterView* pCharView = GetWidgetCharViewLeft();
	if (pCharView) {
		pCharView->SetCharacter( uidPlayer1);
		
		if (ZGetGame()->IsReplay())
			pCharView->InitCharParts(pCharProp[0]->nSex, pCharProp[0]->nHair, pCharProp[0]->nFace, pCharInfo[0]->nEquipedItemDesc);
	}

	pCharView = GetWidgetCharViewRight();
	if (pCharView) {
		pCharView->SetCharacter( uidPlayer2);

		if (ZGetGame()->IsReplay())
			pCharView->InitCharParts(pCharProp[1]->nSex, pCharProp[1]->nHair, pCharProp[1]->nFace, pCharInfo[1]->nEquipedItemDesc);
	}

	// TodoH(하) - Combatinterface_DuelTournament.xml 내에서 Custom1, Custom2를 없애줘야 되지 않을까?
	ZItemSlotView* itemSlot = NULL;
	MMatchCharItemParts partsToShow[] = { MMCIP_MELEE, MMCIP_PRIMARY, MMCIP_SECONDARY, MMCIP_CUSTOM1, MMCIP_CUSTOM2, MMCIP_LONGBUFF1, MMCIP_LONGBUFF2, MMCIP_END };
	for (int i=0; partsToShow[i]!=MMCIP_END; ++i)
	{
		MMatchCharItemParts parts = partsToShow[i];
		itemSlot = (ZItemSlotView*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( GetItemSlotName( "CombatDT_CharLeft", parts));
		if( itemSlot ) {
			itemSlot->SetItemID(pCharInfo[0]->nEquipedItemDesc[parts]);
			itemSlot->SetItemCount(pCharInfo[0]->nEquipedItemCount[parts]);
		}
		itemSlot = (ZItemSlotView*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( GetItemSlotName( "CombatDT_CharRight", parts));
		if( itemSlot ) {
			itemSlot->SetItemID(pCharInfo[1]->nEquipedItemDesc[parts]);
			itemSlot->SetItemCount(pCharInfo[1]->nEquipedItemCount[parts]);
		}
	}

	char szTemp[256];

	int idx;
	int tp[2] = {9999,9999};
	idx = GetPlayerInfoIndex(uidPlayer1);
	if (idx != -1) tp[0] = m_QuaterFinalPlayer[idx].m_nTP;
	idx = GetPlayerInfoIndex(uidPlayer2);
	if (idx != -1) tp[1] = m_QuaterFinalPlayer[idx].m_nTP;

	MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDT_PlayerInfo_Name_Left");
	if (pLabel)	pLabel->SetText(pCharInfo[0]->szName);
	pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDT_PlayerInfo_Clan_Left");
	if (pLabel)	pLabel->SetText(pCharInfo[0]->szClanName);
	pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDT_PlayerInfo_Level_Left");
	if (pLabel)	{
		sprintf(szTemp, "%d", pCharInfo[0]->nLevel);
		pLabel->SetText(szTemp);
	}
	pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDT_PlayerInfo_HP_Left");
	if (pLabel)	{
		sprintf(szTemp, "%d", hp[0]);
		pLabel->SetText(szTemp);
	}
	pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDT_PlayerInfo_AP_Left");
	if (pLabel)	{
		sprintf(szTemp, "%d", ap[0]);
		pLabel->SetText(szTemp);
	}
	pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDT_PlayerInfo_TP_Left");
	if (pLabel)	{
		sprintf(szTemp, "%d", tp[0]);
		pLabel->SetText(szTemp);
	}
	pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDT_PlayerInfo_Grade_Left");
	if (pLabel)	{
		char sz[32];
		sprintf(sz, "%d", pCharInfo[0]->nDTLastWeekGrade);
		ZTransMsg(szTemp, MSG_GAME_DUELTOURNAMENT_MATCH_NTH_GRADE, 1, sz);
		pLabel->SetText(szTemp);
	}

	pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDT_PlayerInfo_Name_Right");
	if (pLabel)	pLabel->SetText(pCharInfo[1]->szName);
	pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDT_PlayerInfo_Clan_Right");
	if (pLabel)	pLabel->SetText(pCharInfo[1]->szClanName);
	pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDT_PlayerInfo_Level_Right");
	if (pLabel)	{
		sprintf(szTemp, "%d", pCharInfo[1]->nLevel);
		pLabel->SetText(szTemp);
	}
	pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDT_PlayerInfo_HP_Right");
	if (pLabel)	{
		sprintf(szTemp, "%d", hp[1]);
		pLabel->SetText(szTemp);
	}
	pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDT_PlayerInfo_AP_Right");
	if (pLabel)	{
		sprintf(szTemp, "%d", ap[1]);
		pLabel->SetText(szTemp);
	}
	pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDT_PlayerInfo_TP_Right");
	if (pLabel)	{
		sprintf(szTemp, "%d", tp[1]);
		pLabel->SetText(szTemp);
	}
	pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDT_PlayerInfo_Grade_Right");
	if (pLabel)	{
		char sz[32];
		sprintf(sz, "%d", pCharInfo[1]->nDTLastWeekGrade);
		ZTransMsg(szTemp, MSG_GAME_DUELTOURNAMENT_MATCH_NTH_GRADE, 1, sz);
		pLabel->SetText(szTemp);
	}
}

// 플레이어 정보 위젯들에 슬라이딩 애니메이션 효과를 준다
void ZRuleDuelTournament::BeginPlayerInfoUISlideAni(bool bToCenter)
{
	m_fSlideElapsedTime = 0;
	m_bSlideToCenter = bToCenter;
}

void ZRuleDuelTournament::UpdateUISlideAni(float fElapsed)
{
	if (m_fSlideElapsedTime == -1)
		return;

	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatDT_CharacterInfo");
	if (!pWidget || !pWidget->IsVisible()) return;

	// 슬라이딩 시작위치 (오프셋: 왼쪽 위젯은 이만큼 왼쪽에서, 오른쪽 위젯은 이만큼 오른쪽에서부터)
	const float fSlideDistance = 300.f / 800.f * RGetScreenWidth();
	// 슬라이딩해서 원래 정의된 위치까지 도달하는데 걸릴 시간
	const float fSlidePeriod = 0.3f;

	//m_fSlideElapsedTime += float(int(fElapsed * 10)) / 10.f;
	m_fSlideElapsedTime += fElapsed;
	if (m_fSlideElapsedTime > fSlidePeriod) {
		m_fSlideElapsedTime = fSlidePeriod;
	}

	float f = m_fSlideElapsedTime / fSlidePeriod;	// 0.0 ~ 1.0
	
	if (!m_bSlideToCenter)	// 슬라이드 방향이 반대면 반대값으로
		f = 1.0f - f;

	float fs = f * (PI*0.5f);	// 0 ~ PI/2
	float fSlide = sinf(fs);	// 슬라이딩이 서서히 느려지도록 sin값 사용

	MWidget* pChild;
	int numChild = pWidget->GetChildCount();
	for (int i=0; i<numChild; ++i)
	{
		pChild = pWidget->GetChild(i);
		if (!pChild) continue;

		// 슬라이딩 대상은 장비 슬롯, 전적, 캐릭터뷰 위젯
		if (NULL == strstr(pChild->m_szIDLName, "CombatDT_"))
			continue;
		if (strstr(pChild->m_szIDLName, "Left"))
		{
			MRECT rc = pChild->GetIDLRect();
			MPOINT pt(rc.x, rc.y);
			pt.x = (pt.x - fSlideDistance) + (fSlideDistance * fSlide);
			
			pt.x = pt.x / 800.f * RGetScreenWidth();
			pt.y = pt.y / 600.f * RGetScreenHeight();
			pChild->SetPosition(pt);
		}
		else if (strstr(pChild->m_szIDLName, "Right"))
		{
			MRECT rc = pChild->GetIDLRect();
			MPOINT pt(rc.x, rc.y);
			pt.x = (pt.x + fSlideDistance) - (fSlideDistance * fSlide);
			
			pt.x = pt.x / 800.f * RGetScreenWidth();
			pt.y = pt.y / 600.f * RGetScreenHeight();
			pChild->SetPosition(pt);
		}
		else
			continue;
	}

	if (m_fSlideElapsedTime == fSlidePeriod)
		m_fSlideElapsedTime = -1;
}

static void ColorText(MDrawContext* pDC, int x, int y, const char* sz, MCOLOR color=0)
{
	// 색지정해서 텍스트 출력을 돕는 함수
	// color == 0이면 pDC의 현재 컬러로 출력
	MCOLOR oldColor = pDC->GetColor();
	
	if (color.GetARGB() != 0)
		pDC->SetColor(color);
	pDC->Text(x, y, sz);
	
	if (color.GetARGB() != 0)
		pDC->SetColor(oldColor);
}

void ZRuleDuelTournament::ShowMatchOrder(MDrawContext* pDC, bool isResult, float fElapsed)
{
	const float YPOS[4] = {0.023f, 0.156f, 0.343f, 0.475f};
	float fSceneWidth = (float)MGetWorkspaceWidth();
	float fSceneHeigth = (float)MGetWorkspaceHeight();
	float fStartX = 0.33f;
	float fStartY = 0.37f;
	if(isResult)
	{
		fStartX = 0.075f;
		fStartY = 0.33f;
	}
	float fRateX = 0.55f * fSceneWidth/800.f;
	float fRateY = 0.55f * fSceneHeigth/600.f;
	int screenx = fStartX * fSceneWidth;
	int screeny = fStartY * fSceneHeigth;

	char szTemp[256];
	float iconDTGradeSize = 28.f * fSceneHeigth/800.f;

	// 플레이어 이름 그리기 + 계급장
	for(int i=0; i<4; ++i)
	{
		// 이름 출력
		int y = screeny+(YPOS[i]*fSceneHeigth);
		int xLeft = screenx+(0.007f*fSceneWidth);
		int xRight = screenx+(0.42f*fSceneWidth);
		
		// 내 이름 바탕엔 파랑바탕색을 깐다
		MFont* pFont = pDC->GetFont();
		if (pFont)
		{
			if (ZGetMyUID() == m_QuaterFinalPlayer[i].uidPlayer)
			{
				int w = MMGetWidth(pFont, m_QuaterFinalPlayer[i].m_szCharName, (int)strlen(m_QuaterFinalPlayer[i].m_szCharName));
				MRECT rc(xLeft, y, w, pFont->GetHeight());
				DrawHighlight(pDC, rc);
			}
			else if (ZGetMyUID() == m_QuaterFinalPlayer[i+4].uidPlayer)
			{
				int w = MMGetWidth(pFont, m_QuaterFinalPlayer[i+4].m_szCharName, (int)strlen(m_QuaterFinalPlayer[i+4].m_szCharName));
				MRECT rc(xRight, y, w, pFont->GetHeight());
				DrawHighlight(pDC, rc);
			}
		}

		// 이름 스트링 출력
		MCOLOR color = 0;
		color = (isResult && m_DTChampion.uidPlayer == m_QuaterFinalPlayer[i].uidPlayer) ? 0xFFFFFF00 : 0;
		ColorText(pDC, xLeft, y, m_QuaterFinalPlayer[i].m_szCharName, color);

		color = (isResult && m_DTChampion.uidPlayer == m_QuaterFinalPlayer[i+4].uidPlayer) ? 0xFFFFFF00 : 0;
		ColorText(pDC, xRight, y, m_QuaterFinalPlayer[i+4].m_szCharName, color);
		
		// 계급장 표시
		MMatchObjCache* pPlayer1 = ZGetGameClient()->FindObjCache(m_QuaterFinalPlayer[i].uidPlayer);
		if (pPlayer1) {
			GetDuelTournamentGradeIconFileName(szTemp, pPlayer1->GetDTGrade());
			//{	GetDuelTournamentGradeIconFileName(szTemp, i+1);
			MBitmap* pBmpDTGradeIcon = MBitmapManager::Get( szTemp );
			pDC->SetBitmap(pBmpDTGradeIcon);
			pDC->Draw( screenx-iconDTGradeSize-(0.004f*fSceneWidth), y-iconDTGradeSize*0.25f, iconDTGradeSize, iconDTGradeSize);
		}

		MMatchObjCache* pPlayer2 = ZGetGameClient()->FindObjCache(m_QuaterFinalPlayer[i+4].uidPlayer);
		if (pPlayer2) {
			GetDuelTournamentGradeIconFileName(szTemp, pPlayer2->GetDTGrade());
			//{	GetDuelTournamentGradeIconFileName(szTemp, i+5);
			MBitmap* pBmpDTGradeIcon = MBitmapManager::Get( szTemp );
			pDC->SetBitmap(pBmpDTGradeIcon);
			pDC->Draw( screenx+(0.532f*fSceneWidth)+(0.004f*fSceneWidth), y-iconDTGradeSize*0.25f, iconDTGradeSize, iconDTGradeSize);
		}

		// Ping 값 출력하기
		if(pPlayer1)
		{
			char szText[32];
			sprintf(szText, "Ping : %d", GetPingValue(m_QuaterFinalPlayer[i].uidPlayer));
			ColorText(pDC, xLeft, y+(0.042*fSceneHeigth), szText, color);
		}
		if(pPlayer2)
		{
			char szText[32];
			sprintf(szText, "Ping : %d", GetPingValue(m_QuaterFinalPlayer[i+4].uidPlayer));
			ColorText(pDC, xRight, y+(0.042*fSceneHeigth), szText, color);
		}

	}

	// 대진표 이미지 그리기
	MBitmap* pBitmap = MBitmapManager::Get( "matchschedule.tga");
	if(pBitmap != NULL)
	{
		pDC->SetBitmap( pBitmap);
		float nMatchSizeX = fRateX * pBitmap->GetWidth();
		float nMatchSizeY = fRateY * pBitmap->GetHeight();
		pDC->Draw( screenx, screeny, nMatchSizeX, nMatchSizeY);

		if(isResult)
		{
			// 결과창 1등 마크 효과
			pBitmap = MBitmapManager::Get( "matchschedule_resultMark.tga");
			if(pBitmap != NULL)
			{
				pDC->SetBitmap( pBitmap);
				float nMatchSizeX = fRateX * pBitmap->GetWidth();
				float nMatchSizeY = fRateY * pBitmap->GetHeight();
				pDC->Draw( 0.282f*fSceneWidth, 0.4635f*fSceneHeigth, nMatchSizeX, nMatchSizeY);
			}
		}
	}

	// 대진표 진행상태 보여주는 라인 그리기
	pBitmap = MBitmapManager::Get( "matchschedule_line.tga");
	MBitmap* pBitmap2 = MBitmapManager::Get( "matchschedule_line2.tga");
	if(pBitmap != NULL && pBitmap2 != NULL)
	{
		const float LINE_YPOS[4] = {0.03f, 0.092f, 0.349f, 0.413f};
		const float LINE2_YPOS[2] = {0.097f, 0.35f};
		const int LINE_INVERSE[4] = {0, 1, 0, 1};
		const int LINE2_INVERSE[4] = {0, 1};

		float fPosX, fPosY, fPosX2, fPosY2;
		for(int i=0; i<4; ++i)
		{
			fPosY = screeny + LINE_YPOS[i]*fSceneHeigth;
			if(m_QuaterFinalPlayer[i].nMatchLevel >= 1)
			{
				fPosX = screenx + 0.125f*fSceneWidth;
				DrawInverse(pDC, pBitmap, fPosX, fPosY, fRateX, fRateY, LINE_INVERSE[i]);
				if(m_QuaterFinalPlayer[i].nMatchLevel >= 2)
				{
					fPosX2 = screenx + 0.17f*fSceneWidth;
					fPosY2 = screeny + LINE2_YPOS[i/2]*fSceneHeigth;
					DrawInverse(pDC, pBitmap2, fPosX2, fPosY2, fRateX, fRateY, LINE2_INVERSE[i/2]);
				}
			}
			if(m_QuaterFinalPlayer[i+4].nMatchLevel >= 1)
			{
				fPosX = screenx + 0.36f*fSceneWidth;
				DrawInverse(pDC, pBitmap, fPosX, fPosY, fRateX, fRateY, LINE_INVERSE[i]+2);
				if(m_QuaterFinalPlayer[i+4].nMatchLevel >= 2)
				{
					fPosX2 = screenx + 0.312f*fSceneWidth;
					fPosY2 = screeny + LINE2_YPOS[i/2]*fSceneHeigth;
					DrawInverse(pDC, pBitmap2, fPosX2, fPosY2, fRateX, fRateY, LINE2_INVERSE[i/2]+2);
				}
			}
		}
	}

	// 대전중인 플레이어 표현하는 노란 박스
	pBitmap = MBitmapManager::Get( "matchschedule_box.tga");
	if(pBitmap != NULL)
	{
		unsigned char prevOpcacity = pDC->GetOpacity();
		// 서서히 점멸하도록 알파를 조정한다
		float addOpacity = fElapsed * 300.f;
		m_fBlinkOpacity += addOpacity;
		m_fBlinkOpacity = fmod(m_fBlinkOpacity, 512.f);	// 값을 0~512 사이로 가둬둠
		unsigned char opacity;
		if (m_fBlinkOpacity >= 256.f)
			opacity = unsigned char(256.f-m_fBlinkOpacity);	// 256~512 까지는 알파가 줄어듬 (ex; 256-258 = -2 , uchar로 캐스팅하면 253)
		else
			opacity = unsigned char(m_fBlinkOpacity);

		pDC->SetOpacity(max(opacity, 20));	// 약간의 오퍼시티는 보장하자

		pDC->SetBitmap( pBitmap);
		float fBoxX = fRateX * pBitmap->GetWidth();
		float fBoxY = fRateY * pBitmap->GetHeight();

		float fPosY;
		DuelTournamentPlayer* pPlayer = GetPlayer(m_nextPlayerInfo.uidPlayer1);
		if(pPlayer)
		{
			if(pPlayer->nNumber < 4)
			{
				fPosY = YPOS[pPlayer->nNumber]-YPOS[0];
				pDC->Draw( screenx, screeny + fPosY*fSceneHeigth, fBoxX, fBoxY);
			}
			else
			{
				fPosY = YPOS[pPlayer->nNumber-4]-YPOS[0];
				pDC->Draw( screenx + (0.413f*fSceneWidth), screeny + fPosY*fSceneHeigth, fBoxX, fBoxY);
			}
		}
		pPlayer = GetPlayer(m_nextPlayerInfo.uidPlayer2);
		if(pPlayer)
		{
			if(pPlayer->nNumber < 4)
			{
				fPosY = YPOS[pPlayer->nNumber]-YPOS[0];
				pDC->Draw( screenx, screeny + fPosY*fSceneHeigth, fBoxX, fBoxY);
			}
			else
			{
				fPosY = YPOS[pPlayer->nNumber-4]-YPOS[0];
				pDC->Draw( screenx + (0.413f*fSceneWidth), screeny + fPosY*fSceneHeigth, fBoxX, fBoxY);
			}
		}


		pDC->SetOpacity(prevOpcacity);
	}
}
DuelTournamentPlayer* ZRuleDuelTournament::GetPlayer(const MUID& uid)
{
	for(int i=0; i<8; ++i)
	{
		if(m_QuaterFinalPlayer[i].uidPlayer == uid)
			return &m_QuaterFinalPlayer[i];
	}
	return NULL;
}

void ZRuleDuelTournament::DrawHighlight(MDrawContext* pDC, const MRECT& rc)
{
	MBitmapR2 *pBitmap=(MBitmapR2*)MBitmapManager::Get("button_glow.png");
	if(pBitmap) {
		DWORD defaultcolor = 0x3030F0;
		DWORD opacity=(DWORD)pDC->GetOpacity();
		MDrawEffect prevEffect = pDC->GetEffect();
		pDC->SetEffect(MDE_ADD);
		MCOLOR prevColor = pDC->GetBitmapColor();
		pDC->SetBitmapColor(MCOLOR(defaultcolor));
		unsigned char prevOpacity = pDC->GetOpacity();
		pDC->SetOpacity(opacity);
		pDC->SetBitmap(pBitmap);
		pDC->Draw(rc.x,rc.y,rc.w,rc.h,0,0,64,32);
		pDC->SetBitmapColor(prevColor);
		pDC->SetEffect(prevEffect);
		pDC->SetOpacity(prevOpacity);
	}
}

void ZRuleDuelTournament::DrawVictorySymbol(MDrawContext* pDC, MUID uidPlayer1, MUID uidPlayer2)
{
	if(MDUELTOURNAMENTROUNDSTATE_QUATERFINAL == m_eDTRoundState)
		return;	// 8강전일때는 승리마크가 필요없다.(표시도 안됨)

	float fSceneWidth = (float)MGetWorkspaceWidth();
	float fSceneHeigth = (float)MGetWorkspaceHeight();
	float fRx = fSceneWidth  / 800.0f;
	float fRy = fSceneHeigth / 600.0f;

	float fPosX[4] = {0.41f, 0.432f, 0.54f, 0.562f};
	float fPosY = 0.067f;
	float nSize = 23.f * fRx;

	MBitmap* pBitmap_Blank = MBitmapManager::Get( "DuelTournament_VictorySymbol_Blank.tga");
	if (pBitmap_Blank)
	{
		pDC->SetBitmap( pBitmap_Blank);

		if(MDUELTOURNAMENTROUNDSTATE_QUATERFINAL != m_eDTRoundState)
		{ // 8강이 아닐때는 승리마크를 1개만 보여준다.
			pDC->Draw( fPosX[0]*fSceneWidth, fPosY*fSceneHeigth, nSize, nSize, 0, 0, 23, 23);
			pDC->Draw( fPosX[3]*fSceneWidth, fPosY*fSceneHeigth, nSize, nSize, 0, 0, 23, 23);
		}
		pDC->Draw( fPosX[1]*fSceneWidth, fPosY*fSceneHeigth, nSize, nSize, 0, 0, 23, 23);
		pDC->Draw( fPosX[2]*fSceneWidth, fPosY*fSceneHeigth, nSize, nSize, 0, 0, 23, 23);
	}

	DuelTournamentPlayer* pPlayer1 = GetPlayer(m_nextPlayerInfo.uidPlayer1);
	DuelTournamentPlayer* pPlayer2 = GetPlayer(m_nextPlayerInfo.uidPlayer2);
	if(!pPlayer1 || !pPlayer2)
		return;

	if(pPlayer1->nVictory == 0 && pPlayer2->nVictory == 0)
		return;	// 둘다 승리가 하나도 없으면 그리지 않는다.

	int nVictory1, nVictory2;
	// 게임도중 1플레이어와 2플레이어 순서가 바뀌었을때 처리해준다.
	if(uidPlayer1 == pPlayer1->uidPlayer)
	{
		nVictory1 = pPlayer1->nVictory;
		nVictory2 = pPlayer2->nVictory;
	}
	else if(uidPlayer1 == pPlayer2->uidPlayer)
	{
		nVictory1 = pPlayer2->nVictory;
		nVictory2 = pPlayer1->nVictory;
	}
	else
		return;


	MBitmap* pBitmap = MBitmapManager::Get( "DuelTournament_VictorySymbol.tga");
	if ( !pBitmap)
		return ;
	pDC->SetBitmap( pBitmap);

	if(1 <= nVictory1)
	{
		pDC->Draw( fPosX[1]*fSceneWidth, fPosY*fSceneHeigth, nSize, nSize, 0, 0, 23, 23);
		if(2 <= nVictory1)
		{
			pDC->Draw( fPosX[0]*fSceneWidth, fPosY*fSceneHeigth, nSize, nSize, 0, 0, 23, 23);
		}
	}
	if(1 <= nVictory2)
	{
		pDC->Draw( fPosX[2]*fSceneWidth, fPosY*fSceneHeigth, nSize, nSize, 0, 0, 23, 23);
		if(2 <= nVictory2)
			pDC->Draw( fPosX[3]*fSceneWidth, fPosY*fSceneHeigth, nSize, nSize, 0, 0, 23, 23);
	}
}

void ZRuleDuelTournament::SetPlayerHpApForUI(const MUID& uidChar, float fmaxhp, float fmaxap, float fhp, float fap)
{
	int idx = GetPlayerInfoIndex(uidChar);
	if (idx != -1)
	{
		m_QuaterFinalPlayer[idx].fMaxHP = fmaxhp;
		m_QuaterFinalPlayer[idx].fMaxAP = fmaxap;
		m_QuaterFinalPlayer[idx].fHP = fhp;
		m_QuaterFinalPlayer[idx].fAP = fap;
	}
}

void ZRuleDuelTournament::GetPlayerHpApForUI(const MUID& uidChar, float* fmaxhp, float* fmaxap, float* fhp, float* fap)
{
	int idx = GetPlayerInfoIndex(uidChar);
	if (idx != -1)
	{
		*fmaxhp = m_QuaterFinalPlayer[idx].fMaxHP;
		*fmaxap = m_QuaterFinalPlayer[idx].fMaxAP;
		*fhp = m_QuaterFinalPlayer[idx].fHP;
		*fap = m_QuaterFinalPlayer[idx].fAP;
	}
}

int ZRuleDuelTournament::GetPlayerInfoIndex(const MUID& uidChar)
{
	for(int i=0; i < 8; ++i) {
		if(m_QuaterFinalPlayer[i].uidPlayer == uidChar)
			return i;
	}
	return -1;
}

int ZRuleDuelTournament::GetPingValue(MUID uiPlayer)
{
	int nPing = (uiPlayer == ZGetGameClient()->GetPlayerUID() ? 0 : MAX_PING);
	MMatchPeerInfo* pPeer = ZGetGameClient()->FindPeer(uiPlayer);
	if (pPeer) {
		if ( ZGetGame()->IsReplay())
			nPing = 0;
		else
			nPing = pPeer->GetPing(ZGetGame()->GetTickTime());
	}
	return nPing;
}
void ZRuleDuelTournament::DrawInverse(MDrawContext* pDC, MBitmap* pBitmap, float x, float y, float fRateX, float fRateY, int nMirror)
{
	pDC->SetBitmap(pBitmap);
	float w = fRateX * pBitmap->GetWidth();
	float h = fRateY * pBitmap->GetHeight();
	switch(nMirror)
	{
	case 0: pDC->DrawInverse(x, y, w, h, false,	false);	break;	// 정상
	case 1: pDC->DrawInverse(x, y, w, h, false, true);	break;	// X축 Inverse
	case 2: pDC->DrawInverse(x, y, w, h, true,	false);	break;	// Y축 Inverse
	case 3: pDC->DrawInverse(x, y, w, h, true,	true);	break;	// X, Y축 Inverse
	}
}

void ZRuleDuelTournament::RestoreStageExitButtonConfirmMessage()
{
	// 내가 출전할 경기가 남아있지 않으면 더이상 게임 나가기 버튼의 확인 대화상자에서 TP페널티 경고문을 보여줄 필요가 없으므로
	// 메시지를 원래대로 복원
	MButton* pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "StageExit");
	if (pButton) {
		pButton->RestoreIDLConfirmMessage();
	}
}