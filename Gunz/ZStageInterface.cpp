/***********************************************************************
  ZStageInterface.cpp
  
  용  도 : 스테이지 인터페이스를 관리하는 클래스. 코드 관리상의 편리를 위해
           분리했음(사실 아직 완전히 다 분리 못했음. -_-;).
  작성일 : 11, MAR, 2004
  작성자 : 임동환
************************************************************************/


#include "stdafx.h"							// Include stdafx.h
#include "ZStageInterface.h"				// Include ZStageInterface.h
#include "ZStageSetting.h"					// Include ZStageSetting.h
#include "ZGameInterface.h"
#include "ZPlayerListBox.h"
#include "ZCombatMenu.h"
#include "ZShopEquipListbox.h"
#include "ZMyItemList.h"
#include "ZItemSlotView.h"
#include "ZMessages.h"
#include "ZLanguageConf.h"


/* 해야할 것들...

 1. ZStageSetting 관련 루틴을 여기로 옮겨와야 하는디...  -_-;
 2. 버튼 UI쪽도 역시 여기로 옮겨와야 하는데 졸라 꼬여있다...  -_-;
*/


/***********************************************************************
  ZStageInterface : public
  
  desc : 생성자
************************************************************************/
ZStageInterface::ZStageInterface( void)
{
	m_bPrevQuest = false;
	m_bDrawStartMovieOfQuest = false;
	m_pTopBgImg = NULL;
	m_pStageFrameImg = NULL;
	m_pItemListFrameImg = NULL;
	m_nListFramePos = 0;
	m_nStateSacrificeItemBox = 0;
	m_pRelayMapListFrameImg = NULL;
	m_nRelayMapListFramePos = 0;
	m_bRelayMapRegisterComplete = true;
	m_bEnableWidgetByRelayMap = true;
}


/***********************************************************************
  ~ZStageInterface : public
  
  desc : 소멸자
************************************************************************/
ZStageInterface::~ZStageInterface( void)
{
	if ( m_pTopBgImg != NULL)
	{
		delete m_pTopBgImg;
		m_pTopBgImg = NULL;
	}

	if ( m_pStageFrameImg != NULL)
	{
		delete m_pStageFrameImg;
		m_pStageFrameImg = NULL;
	}
}


/***********************************************************************
  Create : public
  
  desc : 초기화
************************************************************************/
void ZStageInterface::OnCreate( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	// 초기화 해주고
	m_bPrevQuest = false;
	m_bDrawStartMovieOfQuest = false;
	m_nStateSacrificeItemBox = 0;		// Hide
	m_nGameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
	m_SacrificeItem[ SACRIFICEITEM_SLOT0].RemoveItem();
	m_SacrificeItem[ SACRIFICEITEM_SLOT1].RemoveItem();

	ReadSenarioNameXML();

	MPicture* pPicture = (MPicture*)pResource->FindWidget( "Stage_SacrificeItemImage0");
	if ( pPicture)
		pPicture->SetOpacity( 255);
	pPicture = (MPicture*)pResource->FindWidget( "Stage_SacrificeItemImage1");
	if ( pPicture)
		pPicture->SetOpacity( 255);

	pPicture = (MPicture*)pResource->FindWidget( "Stage_MainBGTop");
	if ( pPicture)
		pPicture->SetBitmap( MBitmapManager::Get( "main_bg_t.png"));
	pPicture = (MPicture*)pResource->FindWidget( "Stage_FrameBG");
	if ( pPicture)
	{
		m_pStageFrameImg = new MBitmapR2;
		((MBitmapR2*)m_pStageFrameImg)->Create( "stage_frame.png", RGetDevice(), "interface/default/stage_frame.png");

		if ( m_pStageFrameImg != NULL)
			pPicture->SetBitmap( m_pStageFrameImg->GetSourceBitmap());
	}
	pPicture = (MPicture*)pResource->FindWidget( "Stage_ItemListBG");
	if ( pPicture)
	{
		m_pItemListFrameImg = new MBitmapR2;
		((MBitmapR2*)m_pItemListFrameImg)->Create( "itemlistframe.tga", RGetDevice(), "interface/default/itemlistframe.tga");

		if ( m_pItemListFrameImg != NULL)
			pPicture->SetBitmap( m_pItemListFrameImg->GetSourceBitmap());
	}
	MWidget* pWidget = (MWidget*)pResource->FindWidget( "Stage_ItemListView");
	if ( pWidget)
	{
		MRECT rect;
		rect = pWidget->GetRect();
		rect.x = -rect.w;
		m_nListFramePos = rect.x;
		pWidget->SetBounds( rect);
	}
	MLabel* pLabel = (MLabel*)pResource->FindWidget( "Stage_SenarioName");
	if ( pLabel)
		pLabel->SetText( "");
	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_SenarioNameImg");
	if ( pWidget)
		pWidget->Show( false);
	MListBox* pListBox = (MListBox*)pResource->FindWidget( "Stage_SacrificeItemListbox");
	if ( pListBox)
		pListBox->RemoveAll();
	MTextArea* pDesc = (MTextArea*)pResource->FindWidget( "Stage_ItemDesc");
	if ( pDesc)
	{
		pDesc->SetTextColor( MCOLOR(0xFF808080));
		//pDesc->SetText( "아이템을 화면 중앙에 있는 두개의 제단에 끌어놓음으로써 게임 레벨을 조정할 수 있습니다.");
		char szText[256];
		sprintf(szText, ZMsg( MSG_QUESTITEM_USE_DESCRIPTION ));
		pDesc->SetText(szText);
	}

	{ // 릴레이맵 "RelayMap" 초기화
		pPicture = (MPicture*)pResource->FindWidget( "Stage_RelayMapListBG" );
		if(pPicture)
		{ // f릴레이맵 리스트 배경 이미지
			m_pRelayMapListFrameImg = new MBitmapR2;
			((MBitmapR2*)m_pRelayMapListFrameImg)->Create( "relaymaplistframe.tga", RGetDevice(), "interface/default/relaymaplistframe.tga");

			if(m_pRelayMapListFrameImg != NULL)
				pPicture->SetBitmap(m_pRelayMapListFrameImg->GetSourceBitmap());
		}
		MWidget* pWidget = (MWidget*)pResource->FindWidget( "Stage_RelayMapListView");
		if ( pWidget)
		{
			MRECT rect;
			rect = pWidget->GetRect();
			rect.x = -rect.w;
			m_nRelayMapListFramePos = rect.x;
			pWidget->SetBounds( rect);
		}

		MComboBox* pCombo = (MComboBox*)pResource->FindWidget("Stage_RelayMapType");			
		if ( pCombo)
			pCombo->CloseComboBoxList();
		pCombo = (MComboBox*)pResource->FindWidget("Stage_RelayMapRepeatCount");					
		if ( pCombo)
			pCombo->CloseComboBoxList();

		MListBox* pListBox = (MListBox*)pResource->FindWidget( "Stage_RelayMapListbox");
		if ( pListBox)
			pListBox->RemoveAll();
		pListBox = (MListBox*)pResource->FindWidget( "Stage_MapListbox");
		if ( pListBox)
			pListBox->RemoveAll();
	}

	ZApplication::GetGameInterface()->ShowWidget( "Stage_Flame0", false);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_Flame1", false);

	// 게임 방식인 채로 방 나갔다가 다른 방 들어가면 버그 생기는거 수정 
	MComboBox* pCombo = (MComboBox*)pResource->FindWidget("StageType");			
	if ( pCombo)
		pCombo->CloseComboBoxList();

	// 맵리스트 연 채로 방 나갔다가 다른 방 들어가면 방장이 아닌데도 맵 바꿔지는 버그 수정
	pCombo = (MComboBox*)pResource->FindWidget("MapSelection");					
	if ( pCombo)
		pCombo->CloseComboBoxList();


	// 채널 리스트 박스는 닫아버림
	pWidget = (MWidget*)pResource->FindWidget( "ChannelListFrame");
	if ( pWidget)
		pWidget->Show( false);


	// 화면 업데 한번 해주삼~
	UpdateSacrificeItem();
	SerializeSacrificeItemListBox();

	// QL 초기화
	OnResponseQL( 0);


	ZApplication::GetGameInterface()->ShowWidget( "Stage_Lights0", false);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_Lights1", false);
}


/***********************************************************************
  OnDestroy : public
  
  desc : 서버나 혹은 시스템의 요청으로부터 스테이지 화면을 새로 갱신하는 함수
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnDestroy( void)
{
	ZApplication::GetGameInterface()->ShowWidget( "Stage", false);

	MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_MainBGTop");
	if ( pPicture)
		pPicture->SetBitmap( MBitmapManager::Get( "main_bg_t.png"));
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_FrameBG");
	if ( pPicture)
		pPicture->SetBitmap( NULL);
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemListBG");
	if ( pPicture)
		pPicture->SetBitmap( NULL);

	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_RelayMapListBG");
	if ( pPicture)
		pPicture->SetBitmap( NULL);

	if ( m_pTopBgImg != NULL)
	{
		delete m_pTopBgImg;
		m_pTopBgImg = NULL;
	}
	if ( m_pStageFrameImg != NULL)
	{
		delete m_pStageFrameImg;
		m_pStageFrameImg = NULL;
	}
	if ( m_pItemListFrameImg != NULL)
	{
		delete m_pItemListFrameImg;
		m_pItemListFrameImg = NULL;
	}
	if (m_pRelayMapListFrameImg != NULL)
	{
		delete m_pRelayMapListFrameImg;
		m_pRelayMapListFrameImg = NULL;
	}

	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_CharacterInfo");
	if ( pWidget)
		pWidget->Enable( true);

//	m_SenarioDesc.clear();
}

void ZStageInterface::OnStageCharListSettup()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	for ( MStageCharSettingList::iterator itor = ZGetGameClient()->GetMatchStageSetting()->m_CharSettingList.begin();
		itor != ZGetGameClient()->GetMatchStageSetting()->m_CharSettingList.end();  ++itor) 
	{
		MSTAGE_CHAR_SETTING_NODE* pCharNode = (*itor);

		ZPlayerListBox* pItem = (ZPlayerListBox*)pResource->FindWidget( "StagePlayerList_");
		if ( pItem)
		{
			bool bMaster = false;

			if ( ZGetGameClient()->GetMatchStageSetting()->GetMasterUID() == pCharNode->uidChar)
				bMaster = true;

			pItem->UpdatePlayer( pCharNode->uidChar,(MMatchObjectStageState)pCharNode->nState,bMaster,MMatchTeam(pCharNode->nTeam));
		}
	}
}


/***********************************************************************
  OnStageInterfaceSettup : public
  
  desc : 서버나 혹은 시스템의 요청으로부터 스테이지 화면을 새로 갱신하는 함수
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnStageInterfaceSettup( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	ZStageSetting::InitStageSettingGameType();
/*
	// 맵 종류를 설정한다.
	MComboBox* pCB = (MComboBox*)pResource->FindWidget( "MapSelection");
	if ( pCB)
	{
		int nSelected = pCB->GetSelIndex();

		InitMaps( pCB);

		if ( nSelected >= pCB->GetCount())
			nSelected = pCB->GetCount() - 1;

		pCB->SetSelIndex( nSelected);
	}
*/

	// CharListView의 Add, Remove, Update는 ZGameClient::OnObjectCache 에서 관리한다.
	MSTAGE_CHAR_SETTING_NODE* pMyCharNode = NULL;
	bool bMyReady = false;		// Ready 상태인지 아닌지...
	for ( MStageCharSettingList::iterator itor = ZGetGameClient()->GetMatchStageSetting()->m_CharSettingList.begin();
		itor != ZGetGameClient()->GetMatchStageSetting()->m_CharSettingList.end();  ++itor) 
	{
		MSTAGE_CHAR_SETTING_NODE* pCharNode = (*itor);

		// 나 자신일 경우
		if ( pCharNode->uidChar == ZGetGameClient()->GetPlayerUID()) 
		{
			pMyCharNode = pCharNode;
			if (pMyCharNode->nState == MOSS_READY)
				bMyReady = true;
			else
				bMyReady = false;
		}

		ZPlayerListBox* pItem = (ZPlayerListBox*)pResource->FindWidget( "StagePlayerList_");
		if ( pItem)
		{
			bool bMaster = false;

			if ( ZGetGameClient()->GetMatchStageSetting()->GetMasterUID() == pCharNode->uidChar)
				bMaster = true;
			
			pItem->UpdatePlayer( pCharNode->uidChar,(MMatchObjectStageState)pCharNode->nState,bMaster,MMatchTeam(pCharNode->nTeam));
		}
	}

	// 스테이지의 버튼 상태(게임시작, 난입, 준비완료)를 설정한다.
	ChangeStageButtons( ZGetGameClient()->IsForcedEntry(), ZGetGameClient()->AmIStageMaster(), bMyReady);

	// 스테이지의...
	ChangeStageGameSetting( ZGetGameClient()->GetMatchStageSetting()->GetStageSetting());
	
	// 난입 멤버일 경우에...
	if ( !ZGetGameClient()->AmIStageMaster() && ( ZGetGameClient()->IsForcedEntry()))
	{
		if ( pMyCharNode != NULL)
			ChangeStageEnableReady( bMyReady);
	}

	// 만약 난입으로 들어왔는데 다른 사람 다 나가서 내가 방장이 되었다면 난입모드 해제
	if ( (ZGetGameClient()->AmIStageMaster() == true) && ( ZGetGameClient()->IsForcedEntry()))
	{
		if ( ZGetGameClient()->GetMatchStageSetting()->GetStageState() == STAGE_STATE_STANDBY)
		{
			ZGetGameClient()->ReleaseForcedEntry();

			// 인터페이스관련
			ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller", true);	// 방설정 버튼
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", true);			// 맵선택 콤보박스
			ZApplication::GetGameInterface()->EnableWidget( "StageType", true);				// 게임방식 콤보박스
			ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", true);		// 최대인원 콤보박스
			ZApplication::GetGameInterface()->EnableWidget( "StageRoundCount", true);		// 경기횟수 콤보박스
		}
		else	// 마스터인데 다른사람들 게임중이면 인터페이스 Disable
		{
			// 인터페이스관련
			ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller", false);	// 방설정 버튼
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", false);			// 맵선택 콤보박스
			ZApplication::GetGameInterface()->EnableWidget( "StageType", false);			// 게임방식 콤보박스
			ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", false);		// 최대인원 콤보박스
			ZApplication::GetGameInterface()->EnableWidget( "StageRoundCount", false);		// 경기횟수 콤보박스
		}
	}

	/// 릴레이맵이면서 다음맵으로 계속 진행중일때 시작 버튼을 모두 OFF 시켜준다.
	if(	ZGetGameClient()->AmIStageMaster() && 
		!m_bEnableWidgetByRelayMap && 
		ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->bIsRelayMap)
	{
		ZApplication::GetGameInterface()->EnableWidget( "GameStart", false);
		// ZApplication::GetGameInterface()->EnableWidget( "StageReady", false);	// 준비완료버튼(기획에 따라 변경가능)

		// 인터페이스관련
		ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller",	false);		// 방설정 버튼
		ZApplication::GetGameInterface()->EnableWidget( "MapSelection",			false);		// 맵선택 콤보박스
		ZApplication::GetGameInterface()->EnableWidget( "StageType",			false);		// 게임방식 콤보박스
		ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer",		false);		// 최대인원 콤보박스
		ZApplication::GetGameInterface()->EnableWidget( "StageRoundCount",		false);		// 경기횟수 콤보박스

		ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller",		false);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMap_OK_Button", false);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMapType",		false);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMapRepeatCount", false);
	}


	// 화면 상단의 맵 이미지 설정하기
	MPicture* pPicture = 0;
	MBitmap* pBitmap = 0;
	char szMapName[256];
 	pPicture = (MPicture*)pResource->FindWidget( "Stage_MainBGTop");
	if ( pPicture)
	{
		if( 0 == strcmp(MMATCH_MAPNAME_RELAYMAP, ZGetGameClient()->GetMatchStageSetting()->GetMapName()))
		{
			sprintf( szMapName, "interface/default/%s", MGetMapDescMgr()->GetMapImageName( MMATCH_DEFAULT_STAGESETTING_MAPNAME));
		}
		else
			sprintf( szMapName, "interface/default/%s", MGetMapDescMgr()->GetMapImageName( ZGetGameClient()->GetMatchStageSetting()->GetMapName()));

		if ( m_pTopBgImg != NULL)
		{
			delete m_pTopBgImg;
			m_pTopBgImg = NULL;
		}

		m_pTopBgImg = new MBitmapR2;
		((MBitmapR2*)m_pTopBgImg)->Create( "TopBgImg.png", RGetDevice(), szMapName);

		if ( m_pTopBgImg != NULL)
			pPicture->SetBitmap( m_pTopBgImg->GetSourceBitmap());
	}
	
	// 정보창에 게임방제 설정하기
	MLabel* pLabel = (MLabel*)pResource->FindWidget( "StageNameLabel");
	if ( pLabel != 0)
	{
		char szStr[ 256];
		sprintf( szStr, "%s > %s > %03d:%s", ZGetGameClient()->GetServerName(), ZMsg( MSG_WORD_STAGE), ZGetGameClient()->GetStageNumber(), ZGetGameClient()->GetStageName());
		pLabel->SetText( szStr);
	}

	// 상하단 스트라이프의 색상 바꾸기
#define SDM_COLOR			MCOLOR(255,0,0)
#define TDM_COLOR			MCOLOR(0,255,0)
#define SGD_COLOR			MCOLOR(0,0,255)
#define TGD_COLOR			MCOLOR(255,255,0)
#define ASSASIN_COLOR		MCOLOR(255,0,255)
#define TRAINING_COLOR		MCOLOR(0,255,255)
#define QUEST_COLOR			MCOLOR(255,255,255)
#define SURVIVAL_COLOR		MCOLOR(255,255,255)

	MCOLOR color;
	switch ( ZGetGameClient()->GetMatchStageSetting()->GetGameType() )
	{	
		case MMATCH_GAMETYPE_ASSASSINATE:
			color = ASSASIN_COLOR;
			break;

		case MMATCH_GAMETYPE_DEATHMATCH_SOLO:
			color = SDM_COLOR;
			break;

		case MMATCH_GAMETYPE_DEATHMATCH_TEAM:
		case MMATCH_GAMETYPE_DEATHMATCH_TEAM2:
		case MMATCH_GAMETYPE_CTF:
			color = TDM_COLOR;
			break;

		case MMATCH_GAMETYPE_GLADIATOR_SOLO:
			color = SGD_COLOR;
			break;

		case MMATCH_GAMETYPE_GLADIATOR_TEAM:
			color = TGD_COLOR;
			break;

		case MMATCH_GAMETYPE_TRAINING:
			color = TRAINING_COLOR;
			break;

#ifdef _QUEST
		case MMATCH_GAMETYPE_SURVIVAL:
			color = QUEST_COLOR;
			break;

		case MMATCH_GAMETYPE_QUEST:
			color = SURVIVAL_COLOR;
			break;
#endif
		case MMATCH_GAMETYPE_BERSERKER:
			color = SDM_COLOR;
			break;

		case MMATCH_GAMETYPE_DUEL:
			color = SDM_COLOR;
			break;
		case MMATCH_GAMETYPE_DUELTOURNAMENT:
			color = SDM_COLOR;
			break;

		default:
			//_ASSERT(0);
			color = MCOLOR(255,255,255,255);
	}
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_StripBottom");
	if(pPicture != NULL && !pPicture->IsAnim())
	{		
        pPicture->SetBitmapColor( color );
		if(!(pPicture->GetBitmapColor().GetARGB() == pPicture->GetReservedBitmapColor().GetARGB()))
			pPicture->SetAnimation( 2, 700.0f);
	}
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_StripTop");
	if(pPicture != NULL && !pPicture->IsAnim())	
	{
		pPicture->SetBitmapColor( color );
		if(!(pPicture->GetBitmapColor().GetARGB() == pPicture->GetReservedBitmapColor().GetARGB()))
			pPicture->SetAnimation( 3, 700.0f);		
	}
}

void ZStageInterface::SetStageRelayMapImage()
{
	MPicture* pPicture = 0;
	MBitmap* pBitmap = 0;
	char szMapName[256];
	pPicture = (MPicture*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Stage_MainBGTop");
	if(!pPicture) return;
	MListBox* pRelayMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_RelayMapListbox");
	if(pRelayMapListBox == NULL) return;
	if( 0 < pRelayMapListBox->GetCount())
	{
		sprintf( szMapName, "interface/default/%s", MGetMapDescMgr()->GetMapImageName( pRelayMapListBox->GetString(pRelayMapListBox->GetStartItem())));
		if ( m_pTopBgImg != NULL)
		{
			delete m_pTopBgImg;
			m_pTopBgImg = NULL;
		}

		m_pTopBgImg = new MBitmapR2;
		((MBitmapR2*)m_pTopBgImg)->Create( "TopBgImg.png", RGetDevice(), szMapName);

		if ( m_pTopBgImg != NULL)
			pPicture->SetBitmap( m_pTopBgImg->GetSourceBitmap());
	}
}

void ZStageInterface::SetEnableWidgetByRelayMap(bool b)
{
	if(!ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->bIsRelayMap)
	{	// 릴레이맵이 아니면 릴레이맵용 위젯 비활성화 처리를 할 필요 없다.
		m_bEnableWidgetByRelayMap = true;
		return;
	}

	m_bEnableWidgetByRelayMap = b;
}

/***********************************************************************
  ChangeStageGameSetting : public
  
  desc : 이것도 게임 관련 인터페이스를 수정하는거 같은데... 왜이렇게 많이 나눠놓은거지? -_-;
         주로 화면의 전체적인 UI를 설정한다.
  arg  : pSetting = 스테이지 설정 정보
  ret  : none
************************************************************************/
void ZStageInterface::ChangeStageGameSetting( const MSTAGE_SETTING_NODE* pSetting)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	m_nGameType = pSetting->nGameType;

	// Set map name
	SetMapName( pSetting->szMapName);

	// Is team game?
	ZApplication::GetGameInterface()->m_bTeamPlay = ZGetGameTypeManager()->IsTeamGame( pSetting->nGameType);


	// 관전 허용 여부 확인
	MComboBox* pCombo = (MComboBox*)pResource->FindWidget( "StageObserver");
	MButton* pObserverBtn = (MButton*)pResource->FindWidget( "StageObserverBtn");
	MLabel* pObserverLabel = (MLabel*)pResource->FindWidget( "StageObserverLabel");
	if ( pCombo && pObserverBtn && pObserverLabel)
	{
		if ( pCombo->GetSelIndex() == 1)
		{
			pObserverBtn->SetCheck( false);
			pObserverBtn->Enable( false);
			pObserverLabel->Enable( false);
		}
		else
		{
			pObserverBtn->Enable( true);
			pObserverLabel->Enable( true);
		}
	}

	// 청팀, 홍팀 상태 설정
	ZApplication::GetGameInterface()->UpdateBlueRedTeam();

	// 게임 방식에 따라서 UI를 변경한다
	MAnimation* pAniMapImg = (MAnimation*)pResource->FindWidget( "Stage_MapNameBG");
	bool bQuestUI = false;
	if ( (pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_SOLO) ||			// 데쓰매치 개인전이거나...
		 (pSetting->nGameType == MMATCH_GAMETYPE_GLADIATOR_SOLO) ||				// 칼전 개인전이거나...
		 (pSetting->nGameType == MMATCH_GAMETYPE_BERSERKER) ||					// 버서커모드이거나...
		 (pSetting->nGameType == MMATCH_GAMETYPE_TRAINING) ||					// 트레이닝이거나...
		 (pSetting->nGameType == MMATCH_GAMETYPE_DUEL))							// 듀얼모드 이면...
	{
		// 맵 이름 배경 이미지 변환
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame( 0);

		// 퀘스트 UI 감춤
		bQuestUI = false;
	}
	else if ( (pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM) ||		// 데쓰매치 팀전이거나...
		(pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM2) ||			// 무한데스매치 팀전이거나...
		 (pSetting->nGameType == MMATCH_GAMETYPE_GLADIATOR_TEAM) ||				// 칼전 팀전이거나...
		 (pSetting->nGameType == MMATCH_GAMETYPE_ASSASSINATE) ||
		 (pSetting->nGameType == MMATCH_GAMETYPE_CTF))					// 암살전 이면...
	{
		// 맵 이름 배경 이미지 변환
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame( 1);

		// 퀘스트 UI 감춤
		bQuestUI = false;
	}
	else if ( pSetting->nGameType == MMATCH_GAMETYPE_SURVIVAL)					// 서바이벌 모드이면...
	{
		// 맵 이름 배경 이미지 변환
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame( 0);

		// 퀘스트 UI 감춤
		bQuestUI = false;
	}
	else if ( pSetting->nGameType == MMATCH_GAMETYPE_QUEST)						// 퀘스트 모드이면...
	{
		// 맵 이름 배경 이미지 변환
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame( 2);

		// 퀘스트 UI 보임
		bQuestUI = true;
	}

	// 맵선택시 릴레이맵이면 릴레이맵 리스트 박스를 열어준다.
	 if(pSetting->bIsRelayMap)
		OpenRelayMapBox();
	else
		HideRelayMapBox();

	// 퀘스트 UI 설정
	ZApplication::GetGameInterface()->ShowWidget( "Stage_SacrificeItemImage0", bQuestUI);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_SacrificeItemImage1", bQuestUI);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_QuestLevel", bQuestUI);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_QuestLevelBG", bQuestUI);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_SacrificeItemButton0", bQuestUI);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_SacrificeItemButton1", bQuestUI);

	if ( m_bPrevQuest != bQuestUI)
	{
		ZApplication::GetGameInterface()->ShowWidget( "Stage_Lights0", bQuestUI);
		ZApplication::GetGameInterface()->ShowWidget( "Stage_Lights1", bQuestUI);

		m_SacrificeItem[ SACRIFICEITEM_SLOT0].RemoveItem();
		m_SacrificeItem[ SACRIFICEITEM_SLOT1].RemoveItem();

		UpdateSacrificeItem();

		if ( bQuestUI)
		{
//			ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", false);
			ZPostRequestSacrificeSlotInfo( ZGetGameClient()->GetPlayerUID());
			ZPostRequestQL( ZGetGameClient()->GetPlayerUID());
			OpenSacrificeItemBox();
		}
		else
		{
			MLabel* pLabel = (MLabel*)pResource->FindWidget( "Stage_SenarioName");
			if ( pLabel)
				pLabel->SetText( "");
			ZApplication::GetGameInterface()->ShowWidget( "Stage_SenarioNameImg", false);
//			if (ZGetGameClient()->AmIStageMaster())
//				ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", true);

			HideSacrificeItemBox();
		}

		m_bPrevQuest = !m_bPrevQuest;
	}

	if ( (pSetting->nGameType == MMATCH_GAMETYPE_SURVIVAL) || (pSetting->nGameType == MMATCH_GAMETYPE_QUEST))
		ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller", false);


	// 라운드 선택 콤보박스 보이기
//	bool bShowRound = true;
//	if ( ( pSetting->nGameType == MMATCH_GAMETYPE_SURVIVAL) || ( pSetting->nGameType == MMATCH_GAMETYPE_QUEST))
//		bShowRound = false;
	
//	ZApplication::GetGameInterface()->ShowWidget( "StageRoundCountLabelBG", bShowRound);
//	ZApplication::GetGameInterface()->ShowWidget( "StageRoundCountLabel", bShowRound);
//	ZApplication::GetGameInterface()->ShowWidget( "StageRoundCount", bShowRound);


	// 라운드 or Kill
	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "StageRoundCountLabel");
	if ( pWidget)
	{
		if ((pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_SOLO) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM2) ||		// 팀데스매치 익스트림일 때에도 Kill로 표시.... by kammir 20081117
			(pSetting->nGameType == MMATCH_GAMETYPE_GLADIATOR_SOLO) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_TRAINING) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_BERSERKER) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_DUEL))
			pWidget->SetText( ZMsg(MSG_WORD_KILL));

		else if(pSetting->nGameType == MMATCH_GAMETYPE_CTF)
			pWidget->SetText("Captures");
		else
			pWidget->SetText( ZMsg(MSG_WORD_ROUND));
	}


	// 콤포넌트 업데이트
	ZStageSetting::ShowStageSettingDialog( pSetting, false);


	// 게임중 메뉴 수정 - 머하는 부분인지 알수 없음...
	// 게임중 메뉴에서 퀘스트및 서바이벌일때 대기방으로 나가기 버튼 비활성화 시킴
#ifdef _QUEST
	if ( ZGetGameTypeManager()->IsQuestDerived( pSetting->nGameType) || ZGetGameClient()->IsLadderGame())
		ZApplication::GetGameInterface()->GetCombatMenu()->EnableItem( ZCombatMenu::ZCMI_BATTLE_EXIT, false);
	else
		ZApplication::GetGameInterface()->GetCombatMenu()->EnableItem( ZCombatMenu::ZCMI_BATTLE_EXIT, true);
#endif
}


/***********************************************************************
  ChangeStageButtons : public
  
  desc : 스테이지 내의 버튼들(게임시작, 난입, 준비완료)의 상태를 설정한다.
         게임 설정과 관련된 위젯의 UI를 설정한다.
  arg  : bForcedEntry = 난입 여부(true or false)
         bMaster = 방장 여부(true or false)
		 bReady = 준비 완료 여부(true or false)
  ret  : none
************************************************************************/
void ZStageInterface::ChangeStageButtons( bool bForcedEntry, bool bMaster, bool bReady)
{
	if ( bForcedEntry)	// 난입 모드
	{
		ZApplication::GetGameInterface()->ShowWidget( "GameStart", false);
		ZApplication::GetGameInterface()->ShowWidget( "StageReady", false);

		ZApplication::GetGameInterface()->ShowWidget( "ForcedEntryToGame", true);
		ZApplication::GetGameInterface()->ShowWidget( "ForcedEntryToGame2", true);

		ChangeStageEnableReady( false);
	}
	else
	{
		ZApplication::GetGameInterface()->ShowWidget( "ForcedEntryToGame", false);
		ZApplication::GetGameInterface()->ShowWidget( "ForcedEntryToGame2", false);

		ZApplication::GetGameInterface()->ShowWidget( "GameStart", bMaster);
		ZApplication::GetGameInterface()->ShowWidget( "StageReady", !bMaster);

		if ( bMaster)
		{
			// 보통 방장이라면 방세팅을 바꿀 수 있는 권한을 가진다 (UI enable).
			// 그러나 그래서는 안되는 경우가 있는데 그걸 체크한다.
			bool bMustDisableUI_despiteMaster = false;
			
			// 퀘스트모드에서 방장이 게임시작을 누르고 희생아이템 불타는 애니메이션 도중 방장이 스스로를 kick해서
			// 다른 사람이 방장 권한을 얻은 경우, 내가 방장일지라도 더이상 방세팅을 바꿀 수는 없다
			if ( m_bDrawStartMovieOfQuest)
				bMustDisableUI_despiteMaster = true;


			if (bMustDisableUI_despiteMaster)
				ChangeStageEnableReady( true);	// UI disable
			else
				ChangeStageEnableReady( false);	// UI enable
		}
		else
			ChangeStageEnableReady( bReady);
	}
}


/***********************************************************************
  ChangeStageEnableReady : public
  
  desc : 스테이지의 인터페이스(버튼 enable등)를 켜거나 끄는 함수
  arg  : true(=interface enable) or false(=interface disable)
  ret  : none
************************************************************************/
void ZStageInterface::ChangeStageEnableReady( bool bReady)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	ZApplication::GetGameInterface()->EnableWidget( "GameStart", !bReady);

	ZApplication::GetGameInterface()->EnableWidget( "StageTeamBlue",  !bReady);
	ZApplication::GetGameInterface()->EnableWidget( "StageTeamBlue2", !bReady);
	ZApplication::GetGameInterface()->EnableWidget( "StageTeamRed",  !bReady);
	ZApplication::GetGameInterface()->EnableWidget( "StageTeamRed2", !bReady);
	ZApplication::GetGameInterface()->EnableWidget( "Lobby_StageExit", !bReady);

	if ( (m_nGameType == MMATCH_GAMETYPE_SURVIVAL) || (m_nGameType == MMATCH_GAMETYPE_QUEST))
	{
		ZApplication::GetGameInterface()->EnableWidget( "Stage_SacrificeItemListbox", !bReady);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_PutSacrificeItem",     !bReady);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_SacrificeItemButton0", !bReady);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_SacrificeItemButton1", !bReady);
		if ( ZGetGameClient()->AmIStageMaster())
		{
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageType", !bReady);
		}
//		ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", false);
		ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller", false);
	}
	else
	{
		if ( ZGetGameClient()->AmIStageMaster())
		{
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageType", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageRoundCount", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMap_OK_Button", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMapType", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMapRepeatCount", !bReady);
		}
		else
		{
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", false);
			ZApplication::GetGameInterface()->EnableWidget( "StageType", false);
			ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", false);
			ZApplication::GetGameInterface()->EnableWidget( "StageRoundCount", false);
			ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller", false);
			ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMap_OK_Button", false);
			ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMapType", false);
			ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMapRepeatCount", false);
		}
	}
    
	BEGIN_WIDGETLIST( "Stage_OptionFrame", pResource, MButton*, pButton);
	pButton->Enable( !bReady);
	END_WIDGETLIST();

	BEGIN_WIDGETLIST( "EquipmentCaller", pResource, MButton*, pButton);
	pButton->Enable( !bReady);
	END_WIDGETLIST();

	// Custom: Show shop in stage
	BEGIN_WIDGETLIST("ShopCaller", pResource, MButton*, pButton);
	pButton->Enable(!bReady);
	END_WIDGETLIST();

	ZApplication::GetGameInterface()->EnableWidget( "StagePlayerList_", !bReady);
}


/***********************************************************************
  SetMapName : public
  
  desc : 맵 선택 콤보박스에 맵 이름을 하나씩 등록시킨다.
  arg  : szMapName = 등록시킬 맵 이름
  ret  : none
************************************************************************/
void ZStageInterface::SetMapName( const char* szMapName)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if ( szMapName == NULL)
		return;

	MComboBox* pMapCombo = (MComboBox*)pResource->FindWidget( "MapSelection");
	if ( pMapCombo)
	{
		// 일단 임시 하드코딩(우에엥~ ㅠ.ㅠ)
//		if ( m_nGameType == MMATCH_GAMETYPE_QUEST)
//			pMapCombo->SetText( "Mansion");
//		else
			pMapCombo->SetText( szMapName);
	}
}




/*
	여기서부터 새로 추가된 내용...

	왠만한건 리스너나 다른 곳에서 자동으로 호출되도록 해놨으나 아직 테스트인 관계로
	완벽한건 아님...  -_-;
*/



/***********************************************************************
  OpenSacrificeItemBox : public
  
  desc : 희생 아이템 선택 창 열기
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OpenSacrificeItemBox( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxOpen");
	if ( pButton)
		pButton->Show( false);
	pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxClose");
	if ( pButton)
		pButton->Show( true);

	m_nStateSacrificeItemBox = 2;
	GetSacrificeItemBoxPos();
}


/***********************************************************************
  CloseSacrificeItemBox : public
  
  desc : 희생 아이템 선택 창 닫기
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::CloseSacrificeItemBox( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxClose");
	if ( pButton)
		pButton->Show( false);
	pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxOpen");
	if ( pButton)
		pButton->Show( true);

	MWidget* pWidget = pResource->FindWidget( "Stage_CharacterInfo");
	if ( pWidget)
		pWidget->Enable( true);

	m_nStateSacrificeItemBox = 1;
	GetSacrificeItemBoxPos();
}


/***********************************************************************
  HideSacrificeItemBox : public
  
  desc : 희생 아이템 선택 창 감추기
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::HideSacrificeItemBox( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxClose");
	if ( pButton)
		pButton->Show( false);
	pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxOpen");
	if ( pButton)
		pButton->Show( true);

	MWidget* pWidget = pResource->FindWidget( "Stage_CharacterInfo");
	if ( pWidget)
		pWidget->Enable( true);

	m_nStateSacrificeItemBox = 0;
	GetSacrificeItemBoxPos();
}


/***********************************************************************
  HideSacrificeItemBox : public
  
  desc : 희생 아이템 선택 창 위치 구하기
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::GetSacrificeItemBoxPos( void)
{
//#ifdef _DEBUG
//	m_nListFramePos = 0;
//	return;
//#endif

	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemListView");
	if ( pWidget)
	{
		MRECT rect;

		switch ( m_nStateSacrificeItemBox)
		{
			case 0 :		// Hide
				rect = pWidget->GetRect();
				m_nListFramePos = -rect.w;
				break;

			case 1 :		// Close
				rect = pWidget->GetRect();
				m_nListFramePos = -rect.w + ( rect.w * 0.14);
				break;

			case 2 :		// Open
				m_nListFramePos = 0;
				break;
		}
	}
}


/***********************************************************************
  OnSacrificeItem0 : public
  
  desc : 희생 아이템0을 눌렀을 때
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnSacrificeItem0( void)
{
}


/***********************************************************************
  OnSacrificeItem1 : public
  
  desc : 희생 아이템1를 눌렀을 때
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnSacrificeItem1( void)
{
}


/***********************************************************************
  UpdateSacrificeItem : protected
  
  desc : 변경된 희생 아이템 이미지, 정보등을 업데이트 함.
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::UpdateSacrificeItem( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	// 스테이지 영역에 있는 희생 아이템 이미지 수정
	for ( int i = SACRIFICEITEM_SLOT0;  i <= SACRIFICEITEM_SLOT1;  i++)
	{
		char szWidgetNameItem[ 128];
		sprintf( szWidgetNameItem, "Stage_SacrificeItemImage%d", i);
		MPicture* pPicture = (MPicture*)pResource->FindWidget( szWidgetNameItem);
		if ( pPicture)
		{
			if ( m_SacrificeItem[ i].IsExist())
			{
				pPicture->SetBitmap( m_SacrificeItem[ i].GetIconBitmap());
				char szMsg[ 128];
				MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache( m_SacrificeItem[ i].GetUID());
				if ( pObjCache)
					sprintf( szMsg, "%s (%s)", m_SacrificeItem[ i].GetName(), pObjCache->GetName());
				else
					strcpy( szMsg, m_SacrificeItem[ i].GetName());
				pPicture->AttachToolTip( szMsg);
			}
			else
			{
				pPicture->SetBitmap( NULL);
				pPicture->DetachToolTip();
			}
		}
	}
}


/***********************************************************************
  SerializeSacrificeItemListBox : public
  
  desc : 희생 아이템 리스트 박스에 자료를 받는다.
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::SerializeSacrificeItemListBox( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MListBox* pListBox = (MListBox*)pResource->FindWidget( "Stage_SacrificeItemListbox");
	if ( !pListBox)
		return;

	int nStartIndex  = pListBox->GetStartItem();
	int nSelectIndex = pListBox->GetSelIndex();
	pListBox->RemoveAll();

	// 리스트에 추가
	for ( MQUESTITEMNODEMAP::iterator questitem_itor = ZGetMyInfo()->GetItemList()->GetQuestItemMap().begin();
		  questitem_itor != ZGetMyInfo()->GetItemList()->GetQuestItemMap().end();
		  questitem_itor++)
	{
		ZMyQuestItemNode* pItemNode = (*questitem_itor).second;
		MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc( pItemNode->GetItemID());
		if ( pItemDesc)
		{
			int nCount = pItemNode->m_nCount;
			if ( m_SacrificeItem[ SACRIFICEITEM_SLOT0].IsExist() &&
				 (m_SacrificeItem[ SACRIFICEITEM_SLOT0].GetUID() == ZGetGameClient()->GetPlayerUID()) &&
				 (pItemDesc->m_nItemID == m_SacrificeItem[ SACRIFICEITEM_SLOT0].GetItemID()))
				nCount--;
			if ( m_SacrificeItem[ SACRIFICEITEM_SLOT1].IsExist() &&
				 (m_SacrificeItem[ SACRIFICEITEM_SLOT1].GetUID() == ZGetGameClient()->GetPlayerUID()) &&
				 (pItemDesc->m_nItemID == m_SacrificeItem[ SACRIFICEITEM_SLOT1].GetItemID()))
				nCount--;

			if ( pItemDesc->m_bSecrifice && (nCount > 0))		// 희생 아이템만 추가
			{
				pListBox->Add( new SacrificeItemListBoxItem( pItemDesc->m_nItemID,
															 ZApplication::GetGameInterface()->GetQuestItemIcon( pItemDesc->m_nItemID, true),
															 pItemDesc->m_szQuestItemName,
															 nCount,
															 pItemDesc->m_szDesc));
			}
		}
	}

	MWidget* pWidget = pResource->FindWidget( "Stage_NoItemLabel");
	if ( pWidget)
	{
		if ( pListBox->GetCount() > 0)
			pWidget->Show( false);
		else
			pWidget->Show( true);
	}

	pListBox->SetStartItem( nStartIndex);
	pListBox->SetSelIndex( min( (pListBox->GetCount() - 1), nSelectIndex));
}


/***********************************************************************
  OnDropSacrificeItem : public
  
  desc : 희생 아이템 놓기
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnDropSacrificeItem( int nSlotNum)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pReadyBtn = (MButton*)pResource->FindWidget("StageReady");
	if(pReadyBtn) 
		if(pReadyBtn->GetCheck()) 
		{	// 준비 완료버튼이 눌려있다면
			ZApplication::GetStageInterface()->ChangeStageEnableReady(pReadyBtn->GetCheck());	// 다른 UI를 꺼준다.
			return;	// 희생아이템을 슬롯에 올리지 않는다.
		}

	MListBox* pListBox = (MListBox*)pResource->FindWidget( "Stage_SacrificeItemListbox");
	if ( !pListBox || (pListBox->GetSelIndex() < 0))
		return;

	SacrificeItemListBoxItem* pItemDesc = (SacrificeItemListBoxItem*)pListBox->Get( pListBox->GetSelIndex());
	if ( pItemDesc)
	{
		MTextArea* pDesc = (MTextArea*)pResource->FindWidget( "Stage_ItemDesc");

		// 슬롯이 비어있으면 무조건 올림
		if ( ! m_SacrificeItem[ nSlotNum].IsExist())
		{
			ZPostRequestDropSacrificeItem( ZGetGameClient()->GetPlayerUID(), nSlotNum, pItemDesc->GetItemID());
			char szText[256];
			sprintf(szText, ZMsg( MSG_QUESTITEM_USE_DESCRIPTION ));
			pDesc->SetText(szText);
			//if ( pDesc)
			//	pDesc->Clear();
		}

		// 슬롯이 비어있지 않으면...
		else
		{
			if ( (m_SacrificeItem[ nSlotNum].GetUID()    != ZGetGameClient()->GetPlayerUID()) ||
				 (m_SacrificeItem[ nSlotNum].GetItemID() != pItemDesc->GetItemID()))
				ZPostRequestDropSacrificeItem( ZGetGameClient()->GetPlayerUID(), nSlotNum, pItemDesc->GetItemID());

			char szText[256];
			sprintf(szText, ZMsg( MSG_QUESTITEM_USE_DESCRIPTION ));
			pDesc->SetText(szText);
			//if ( pDesc)
			//	pDesc->Clear();
		}
	}
}


/***********************************************************************
  OnRemoveSacrificeItem : public
  
  desc : 희생 아이템 빼기
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnRemoveSacrificeItem( int nSlotNum)
{
	if ( !m_SacrificeItem[ nSlotNum].IsExist())
		return;

	ZPostRequestCallbackSacrificeItem( ZGetGameClient()->GetPlayerUID(),
									   nSlotNum,
									   m_SacrificeItem[ nSlotNum].GetItemID());

	MTextArea* pDesc = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemDesc");
	char szText[256];
	sprintf(szText, ZMsg( MSG_QUESTITEM_USE_DESCRIPTION ));
	pDesc->SetText(szText);
	//if ( pDesc)
	//	pDesc->Clear();
}


/***********************************************************************
  MSacrificeItemListBoxListener
  
  desc : 희생 아이템 리스트 박스 리스너
************************************************************************/
class MSacrificeItemListBoxListener : public MListener
{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		// On select
		if ( MWidget::IsMsg( szMessage, MLB_ITEM_SEL) == true)
		{
			MListBox* pListBox = (MListBox*)pWidget;

			// 아이템 설명 업데이트
			SacrificeItemListBoxItem* pItemDesc = (SacrificeItemListBoxItem*)pListBox->GetSelItem();
			MTextArea* pDesc = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemDesc");
			if ( pItemDesc && pDesc)
			{
				char szCount[ 128];
				sprintf( szCount, "%s : %d", ZMsg( MSG_WORD_QUANTITY), pItemDesc->GetItemCount());
				pDesc->SetTextColor( MCOLOR( 0xFFD0D0D0));
				pDesc->SetText( szCount);
				pDesc->AddText( "\n");
				pDesc->AddText( pItemDesc->GetItemDesc(), 0xFF808080);
			}

			return true;
		}


		// On double click
		else if ( MWidget::IsMsg( szMessage, MLB_ITEM_DBLCLK) == true)
		{
			// Put item
			if ( !ZApplication::GetStageInterface()->m_SacrificeItem[ 0].IsExist())
				ZApplication::GetStageInterface()->OnDropSacrificeItem( 0);
			else if ( !ZApplication::GetStageInterface()->m_SacrificeItem[ 1].IsExist())
				ZApplication::GetStageInterface()->OnDropSacrificeItem( 1);
		
			return true;
		}

		return false;
	}
};

MSacrificeItemListBoxListener g_SacrificeItemListBoxListener;

MListener* ZGetSacrificeItemListBoxListener( void)
{
	return &g_SacrificeItemListBoxListener;
}


/***********************************************************************
  OnDropCallbackRemoveSacrificeItem
  
  desc : 희생 아이템 제거
************************************************************************/
void OnDropCallbackRemoveSacrificeItem( void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	if ( (pSender == NULL) || (strcmp(pSender->GetClassName(), MINT_ITEMSLOTVIEW)))
		return;

	ZItemSlotView* pItemSlotView = (ZItemSlotView*)pSender;
	ZApplication::GetStageInterface()->OnRemoveSacrificeItem( (strcmp( pItemSlotView->m_szItemSlotPlace, "SACRIFICE0") == 0) ? 0 : 1);
}


// 릴레이맵 인터페이스
/***********************************************************************
OpenRelayMapBox : public

desc : 릴레이맵 선택 창 열기
arg  : none
ret  : none
************************************************************************/
void ZStageInterface::OpenRelayMapBox( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget( "Stage_RelayMapBoxOpen");
	if ( pButton)
		pButton->Show( false);
	pButton = (MButton*)pResource->FindWidget( "Stage_RelayMapBoxClose");
	if ( pButton)
		pButton->Show( true);

	SetRelayMapBoxPos(2);

	// 리플레이 박스에 맵리스트를 생성
	ZApplication::GetStageInterface()->RelayMapCreateMapList();
}

/***********************************************************************
CloseRelayMapBox : public

desc : 릴레이맵 선택 창 닫기
arg  : none
ret  : none
************************************************************************/
void ZStageInterface::CloseRelayMapBox( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget( "Stage_RelayMapBoxClose");
	if ( pButton)
		pButton->Show( false);
	pButton = (MButton*)pResource->FindWidget( "Stage_RelayMapBoxOpen");
	if ( pButton)
		pButton->Show( true);

	MWidget* pWidget = pResource->FindWidget( "Stage_CharacterInfo");
	if ( pWidget)
		pWidget->Enable( true);

	SetRelayMapBoxPos(1);
}

/***********************************************************************
HideRelayMapBox : public

desc : 릴레이맵 선택 창 감추기
arg  : none
ret  : none
************************************************************************/
void ZStageInterface::HideRelayMapBox( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget( "Stage_RelayMapBoxClose");
	if ( pButton)
		pButton->Show( false);
	pButton = (MButton*)pResource->FindWidget( "Stage_RelayMapBoxOpen");
	if ( pButton)
		pButton->Show( true);

	MWidget* pWidget = pResource->FindWidget( "Stage_CharacterInfo");
	if ( pWidget)
		pWidget->Enable( true);

	SetRelayMapBoxPos(0);
}

/***********************************************************************
GetRelayMapBoxPos : public

desc : 릴레이맵 선택 창 위치 구하기
arg  : none
ret  : none
************************************************************************/
void ZStageInterface::SetRelayMapBoxPos( int nBoxPos)
{
	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_RelayMapListView");
	if ( pWidget)
	{
		MRECT rect;

		switch ( nBoxPos)
		{
		case 0 :		// Hide
			rect = pWidget->GetRect();
			m_nRelayMapListFramePos = -rect.w;
			break;

		case 1 :		// Close
			rect = pWidget->GetRect();
			m_nRelayMapListFramePos = -rect.w + ( rect.w * 0.14);
			break;

		case 2 :		// Open
			m_nRelayMapListFramePos = 0;
			break;
		}
	}
}

/***********************************************************************
PostRelayMapType : public

desc : 릴레이맵 타입 업데이트 및 서버에 전송
arg  : none
ret  : none
************************************************************************/
void ZStageInterface::PostRelayMapElementUpdate( void)
{
	MComboBox* pCBRelayMapType = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Stage_RelayMapType" );
	if ( !pCBRelayMapType) return;
	MComboBox* pCBRelayMapTurnCount = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Stage_RelayMapRepeatCount" );
	if ( !pCBRelayMapTurnCount) return;
	ZPostStageRelayMapElementUpdate(ZGetGameClient()->GetStageUID(), pCBRelayMapType->GetSelIndex(), pCBRelayMapTurnCount->GetSelIndex());
}

/***********************************************************************
PostRelayMapListUpdate : public

desc : 릴레이맵 리스트 업데이트(서버에도 리스트를 보낸다.)
arg  : none
ret  : none
************************************************************************/
void ZStageInterface::PostRelayMapInfoUpdate( void)
{
	MComboBox* pCBRelayMapType = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Stage_RelayMapType" );
	if ( !pCBRelayMapType) return;
	MComboBox* pCBRelayMapRepeatCount = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Stage_RelayMapRepeatCount" );
	if ( !pCBRelayMapRepeatCount) return;

	MListBox* pRelayMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_RelayMapListbox");
	if(pRelayMapListBox == NULL) return;

	if(pRelayMapListBox->GetCount() <= 0)	
	{
		ZApplication::GetGameInterface()->ShowMessage(MSG_GAME_RELAYMAP_ONE_OR_MORE_MAP_SELECT);
		return;
	}
	
	//릴레이맵 리스트 전송
	void* pMapArray = MMakeBlobArray(sizeof(MTD_RelayMap), pRelayMapListBox->GetCount());
	int nMakeBlobCnt = 0;
	for(int i=0; i<pRelayMapListBox->GetCount(); i++)
	{
		MTD_RelayMap* pRelayMapNode = (MTD_RelayMap*)MGetBlobArrayElement(pMapArray, nMakeBlobCnt);
		for (int j = 0; j < MMATCH_MAP_COUNT; j++)
		{
			if(0 == strcmp(pRelayMapListBox->GetString(i), (char*)MGetMapDescMgr()->GetMapName(j)))
			{
				pRelayMapNode->nMapID = j;
				break;
			}
		}
		++nMakeBlobCnt;
	}

	ZPostStageRelayMapInfoUpdate(ZGetGameClient()->GetStageUID(), pCBRelayMapType->GetSelIndex(), pCBRelayMapRepeatCount->GetSelIndex(), pMapArray);
	MEraseBlobArray(pMapArray);

	ZApplication::GetStageInterface()->SetIsRelayMapRegisterComplete(true);
	ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMap_OK_Button", false);
}

/***********************************************************************
PostRelayMapList : public

desc : 릴레이맵 선택으로 맵 콤보박스 리스트를 만들어 준다.
arg  : none
ret  : none
************************************************************************/
void ZStageInterface::RelayMapCreateMapList()
{
	MComboBox* pCombo = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("MapSelection");
	if(pCombo == NULL) return;

	// 맵 리스트 만들어 주기
	MListBox* pMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_MapListbox");
	if(pMapListBox == NULL) return;

	pMapListBox->RemoveAll();	// 기존 릴레이맵 리스트를 모두 지워준다.
	for( int i = 0 ; i < pCombo->GetCount(); ++i )
	{
		if(strcmp(MMATCH_MAPNAME_RELAYMAP, pCombo->GetString(i)) == 0)
			continue;
		RelayMapList* pRelayMapList = new RelayMapList( pCombo->GetString(i), MBitmapManager::Get( "Mark_Arrow.bmp"));
		pMapListBox->Add( pRelayMapList);
	}


	// 방장이 릴레이맵 세팅중이면 릴레이맵 리스트를 업데이트 하지 않는다.
	if(!ZApplication::GetStageInterface()->GetIsRelayMapRegisterComplete())
		return;

	// 릴레이맵 리스트 만들어 주기
	MListBox* pRelaMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_RelayMapListbox");
	if(pRelaMapListBox == NULL) return;
	RelayMap arrayRelayMapList[MAX_RELAYMAP_LIST_COUNT];
	memcpy(arrayRelayMapList, ZGetGameClient()->GetMatchStageSetting()->GetRelayMapList(), sizeof(RelayMap)*MAX_RELAYMAP_LIST_COUNT);
	
	pRelaMapListBox->RemoveAll();	// 기존 릴레이맵 리스트를 모두 지워준다.
	for( int i = 0 ; i < ZGetGameClient()->GetMatchStageSetting()->GetRelayMapListCount(); ++i )
	{// 릴레이맵 리스트에 데이터를 추가해준다.
		int nMapID = arrayRelayMapList[i].nMapID;
		RelayMapList* pRelayMapList = new RelayMapList( MGetMapDescMgr()->GetMapName(MGetMapDescMgr()->GetMapID(nMapID)), MBitmapManager::Get( "Mark_X.bmp"));
		pRelaMapListBox->Add( pRelayMapList);
	}
}

/***********************************************************************
MMapListBoxListener

desc : 맵 리스트 박스 리스너
************************************************************************/
class MMapListBoxListener : public MListener
{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		// 스테이지 마스터가 아니면 맵 리스트를 컨트롤 할수없다.
		if(!ZGetGameClient()->AmIStageMaster())
			return false;
		// On select
		if ( MWidget::IsMsg( szMessage, MLB_ITEM_SEL) == true)
		{
			// 맵리스트에서 선택된 맵정보를 릴레이맵 리스트에 추가한다.
			MListBox* pMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_MapListbox");
			if(pMapListBox == NULL) return false;
			MListBox* pRelayMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_RelayMapListbox");
			if(pRelayMapListBox == NULL) return false;
			if(MAX_RELAYMAP_LIST_COUNT <= pRelayMapListBox->GetCount()) 
			{
				ZApplication::GetGameInterface()->ShowMessage( MSG_GAME_RELAYMAP_TOO_MANY_LIST_COUNT );
				return false;
			}

			RelayMapList* pMapList = (RelayMapList*)pMapListBox->GetSelItem();
			char szMapName[ MAPNAME_LENGTH ];
			strcpy(szMapName, pMapList->GetString());
			RelayMapList* pRelayMapList = new RelayMapList( szMapName, MBitmapManager::Get( "Mark_X.bmp"));

			pRelayMapListBox->Add( pRelayMapList);
			pRelayMapListBox->ShowItem(pRelayMapListBox->GetCount());	// 스크롤 위치 세팅

			// 릴레이 맵 수정 시작
			ZApplication::GetStageInterface()->SetIsRelayMapRegisterComplete(false);
			ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMap_OK_Button", true);

			return true;
		}

		// On double click
		else if ( MWidget::IsMsg( szMessage, MLB_ITEM_DBLCLK) == true)
		{
			return true;
		}

		return false;
	}
};
MMapListBoxListener g_MapListBoxListener;
MListener* ZGetMapListBoxListener( void)
{
	return &g_MapListBoxListener;
}

/***********************************************************************
MRelayMapListBoxListener

desc : 릴레이맵 리스트 박스 리스너
************************************************************************/
class MRelayMapListBoxListener : public MListener
{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		// 스테이지 마스터가 아니면 릴레이맵 리스트를 컨트롤 할수없다.
		if(!ZGetGameClient()->AmIStageMaster())
			return false;
		// On select
		if ( MWidget::IsMsg( szMessage, MLB_ITEM_SEL) == true)
		{
			MListBox* pRelayMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_RelayMapListbox");
			if(pRelayMapListBox == NULL) return false;
			if(pRelayMapListBox->GetCount() > 1)
			{	// 맵이 최소한 한개라도 있어야 한다.
				pRelayMapListBox->Remove(pRelayMapListBox->GetSelIndex());

				// 릴레이맵 리스트 스크롤 위치 세팅
				if(pRelayMapListBox->GetCount() <= pRelayMapListBox->GetShowItemCount())
					pRelayMapListBox->ShowItem(0);
				else
					pRelayMapListBox->SetStartItem(pRelayMapListBox->GetCount());
				
				// 릴레이 맵 수정 시작
				ZApplication::GetStageInterface()->SetIsRelayMapRegisterComplete(false);
				ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMap_OK_Button", true);

				return true;
			}
			else
			{
				ZApplication::GetGameInterface()->ShowMessage(MSG_GAME_RELAYMAP_ONE_OR_MORE_MAP_SELECT);
				return false;
			}
		}

		// On double click
		else if ( MWidget::IsMsg( szMessage, MLB_ITEM_DBLCLK) == true)
		{
			return true;
		}
		return false;
	}
};
MRelayMapListBoxListener g_RelayMapListBoxListener;
MListener* ZGetRelayMapListBoxListener( void)
{
	return &g_RelayMapListBoxListener;
}

/***********************************************************************
  StartMovieOfQuest : public
  
  desc : 퀘스트 모드로 시작할때 아이템 합쳐지는 무비를 시작함
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::StartMovieOfQuest( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	m_dwClockOfStartMovie = timeGetTime();

	// 화염 애니메이션 시작
	MAnimation* pAnimation = (MAnimation*)pResource->FindWidget( "Stage_Flame0");
	if ( pAnimation && m_SacrificeItem[ SACRIFICEITEM_SLOT0].IsExist())
	{
		pAnimation->SetCurrentFrame( 0);
		pAnimation->Show( true);
		pAnimation->SetRunAnimation( true);
	}
	pAnimation = (MAnimation*)pResource->FindWidget( "Stage_Flame1");
	if ( pAnimation && m_SacrificeItem[ SACRIFICEITEM_SLOT1].IsExist())
	{
		pAnimation->SetCurrentFrame( 0);
		pAnimation->Show( true);
		pAnimation->SetRunAnimation( true);
	}

	m_bDrawStartMovieOfQuest = true;
}


/***********************************************************************
  OnDrawStartMovieOfQuest : public
  
  desc : 퀘스트 모드로 시작할때 아이템 합쳐지는 무비를 그림
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnDrawStartMovieOfQuest( void)
{
	if ( !m_bDrawStartMovieOfQuest)
		return ;

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	// 경과 시간을 구한다.
	DWORD dwClock = timeGetTime() - m_dwClockOfStartMovie;

	// 희생 아이템 페이드 아웃
	int nOpacity = 255 - dwClock * 0.12f;
	if ( nOpacity < 0)
		nOpacity = 0;

	MPicture* pPicture = (MPicture*)pResource->FindWidget( "Stage_SacrificeItemImage0");
	if ( pPicture && m_SacrificeItem[ SACRIFICEITEM_SLOT0].IsExist())
		pPicture->SetOpacity( nOpacity);

	pPicture = (MPicture*)pResource->FindWidget( "Stage_SacrificeItemImage1");
	if ( pPicture && m_SacrificeItem[ SACRIFICEITEM_SLOT1].IsExist())
		pPicture->SetOpacity( nOpacity);

	// 종료 시간일 경우에...
	if ( dwClock > 3200)
	{
		m_bDrawStartMovieOfQuest = false;

		ZMyQuestItemMap::iterator itMyQItem;

		// 버그해결... 퀘스트 게임시작을 누르는 순간 해당 퀘스트 아이템이 감소된 상태(옮바른)의 갯수를 
		// DB에서 받아오기때문에 카운트 감소를 하면 잘못된값이 된다....20090318 by kammir
		// 여기서 슬롯에 자신의 아이템이 올려져 있으면 해당 아이템 카운트 감소.
		//if( ZGetGameClient()->GetUID() == m_SacrificeItem[ SACRIFICEITEM_SLOT0].GetUID() )
		//{
		//	itMyQItem = ZGetMyInfo()->GetItemList()->GetQuestItemMap().find( m_SacrificeItem[ SACRIFICEITEM_SLOT0].GetItemID() );
		//	itMyQItem->second->Decrease();
		//}
		//if( ZGetGameClient()->GetUID() == m_SacrificeItem[ SACRIFICEITEM_SLOT1].GetUID() )
		//{
		//	itMyQItem = ZGetMyInfo()->GetItemList()->GetQuestItemMap().find( m_SacrificeItem[ SACRIFICEITEM_SLOT1].GetItemID() );
		//	itMyQItem->second->Decrease();
		//}
		
		m_SacrificeItem[ SACRIFICEITEM_SLOT0].RemoveItem();
		m_SacrificeItem[ SACRIFICEITEM_SLOT1].RemoveItem();

		ZApplication::GetGameInterface()->SetState( GUNZ_GAME);
	}
}


/***********************************************************************
  IsShowStartMovieOfQuest : public
  
  desc : 퀘스트 모드로 시작할때 아이템 합쳐지는 무비를 보여줄지 여부를 결정.
  arg  : none
  ret  : true(=Quest mode start movie) or false(=none)
************************************************************************/
bool ZStageInterface::IsShowStartMovieOfQuest( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if ( m_nGameType == MMATCH_GAMETYPE_QUEST)
	{
		if ( m_SacrificeItem[ SACRIFICEITEM_SLOT0].IsExist() || m_SacrificeItem[ SACRIFICEITEM_SLOT1].IsExist())
			return true;
	}

	return false;
}


/***********************************************************************
  OnResponseDropSacrificeItemOnSlot : public
  
  desc : 희생 아이템이 올라갔을때
  arg  : none
  ret  : none
************************************************************************/
bool ZStageInterface::OnResponseDropSacrificeItemOnSlot( const int nResult, const MUID& uidRequester, const int nSlotIndex, const int nItemID )
{
#ifdef _QUEST_ITEM
	if( MOK == nResult)
	{
		MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc( nItemID);
		MBitmap* pIconBitmap = ZApplication::GetGameInterface()->GetQuestItemIcon( nItemID, false);

		m_SacrificeItem[ nSlotIndex].SetSacrificeItemSlot( uidRequester, nItemID, pIconBitmap, pItemDesc->m_szQuestItemName, pItemDesc->m_nLevel);
		SerializeSacrificeItemListBox();

		UpdateSacrificeItem();
	}
	else if( ITEM_TYPE_NOT_SACRIFICE == nResult)
	{
		// 희생 아이템이 아님.
		return false;
	}
	else if( NEED_MORE_QUEST_ITEM == nResult )
	{
		// 현제 가지고 있는 수량을 초과해서 올려 놓으려고 했을경우.
	}
	else if( MOK != nResult )
	{
		// 실패...
		return false;
	}
	else
	{
		// 정의되지 않은 error...
		ASSERT( 0 );
	}

#endif

	return true;
}


/***********************************************************************
  OnResponseCallbackSacrificeItem : public
  
  desc : 희생 아이템이 내려갔을때
  arg  : none
  ret  : none
************************************************************************/
bool ZStageInterface::OnResponseCallbackSacrificeItem( const int nResult, const MUID& uidRequester, const int nSlotIndex, const int nItemID )
{
#ifdef _QUEST_ITEM
	if( MOK == nResult )
	{
		m_SacrificeItem[ nSlotIndex].RemoveItem();
		SerializeSacrificeItemListBox();

		UpdateSacrificeItem();

		MTextArea* pDesc = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemDesc");
		char szText[256];
		sprintf(szText, ZMsg( MSG_QUESTITEM_USE_DESCRIPTION ));
		pDesc->SetText(szText);
		//if ( pDesc)
		//	pDesc->Clear();
	}
	else if( ERR_SACRIFICE_ITEM_INFO == nResult )
	{
		// 클라이언트에서 보낸 정보가 잘못된 정보. 따러 에러처리가 필요하면 여기서 해주면 됨.
	}

#endif

	return true;
}

#ifdef _QUEST_ITEM
///
// Fist : 추교성.
// Last : 추교성.
// 
// 서버로부터 QL의 정보를 받음.
///
bool ZStageInterface::OnResponseQL( const int nQL )
{
	ZGetQuest()->GetGameInfo()->SetQuestLevel( nQL);

	// 스테이지 영역에 있는 퀘스트 레벨 표시 수정
	MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_QuestLevel");
	if ( pLabel)
	{
		char szText[125];
		sprintf( szText, "%s %s : %d", ZMsg( MSG_WORD_QUEST), ZMsg( MSG_CHARINFO_LEVEL), nQL);
		pLabel->SetText( szText);
	}

	return true;
}

bool ZStageInterface::OnStageGameInfo( const int nQL, const int nMapsetID, const unsigned int nScenarioID )
{
	if (nScenarioID != 0)
	{
		ZGetQuest()->GetGameInfo()->SetQuestLevel( nQL );
	}
	else
	{
		// 시나리오가 없으면 그냥 0으로 보이게 한다.
		ZGetQuest()->GetGameInfo()->SetQuestLevel( 0 );
	}

	ZGetQuest()->GetGameInfo()->SetMapsetID( nMapsetID );
	ZGetQuest()->GetGameInfo()->SetSenarioID( nScenarioID );

	UpdateStageGameInfo(nQL, nMapsetID, nScenarioID);

	return true;
}

bool ZStageInterface::OnResponseSacrificeSlotInfo( const MUID& uidOwner1, const unsigned long int nItemID1, 
												   const MUID& uidOwner2, const unsigned long int nItemID2 )
{
	if ( (uidOwner1 != MUID(0,0)) && nItemID1)
	{
		MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc( nItemID1);
		MBitmap* pIconBitmap = ZApplication::GetGameInterface()->GetQuestItemIcon( nItemID1, false);
		m_SacrificeItem[ SACRIFICEITEM_SLOT0].SetSacrificeItemSlot( uidOwner1, nItemID1, pIconBitmap, pItemDesc->m_szQuestItemName, pItemDesc->m_nLevel);
	}
	else
		m_SacrificeItem[ SACRIFICEITEM_SLOT0].RemoveItem();

	if ( (uidOwner2 != MUID(0,0)) && nItemID2)
	{
		MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc( nItemID2);
		MBitmap* pIconBitmap = ZApplication::GetGameInterface()->GetQuestItemIcon( nItemID2, false);
		m_SacrificeItem[ SACRIFICEITEM_SLOT1].SetSacrificeItemSlot( uidOwner2, nItemID2, pIconBitmap, pItemDesc->m_szQuestItemName, pItemDesc->m_nLevel);
	}
	else
		m_SacrificeItem[ SACRIFICEITEM_SLOT1].RemoveItem();

	UpdateSacrificeItem();

	MTextArea* pDesc = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemDesc");
	//if ( pDesc)
	//	pDesc->Clear();

	return true;
}


bool ZStageInterface::OnQuestStartFailed( const int nState )
{
	MTextArea* pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "StageChattingOutput");
	if ( pTextArea)
	{
		char text[256];
		sprintf(text, "^1%s", ZMsg(MSG_GANE_NO_QUEST_SCENARIO));
		pTextArea->AddText( text);
	}

/*
	if( MSQITRES_INV == nState )
	{
		// 해당 QL에대한 희생아이템 정보 테이블이 없음. 이경우는 맞지 않는 희생 아이템이 올려져 있을경우.
		MTextArea* pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "StageChattingOutput");
		if ( pTextArea)
			pTextArea->AddText( "^1현재 놓여있는 아이템은 조건에 맞지 않아 게임을 시작할 수 없습니다.");
	}
	else if( MSQITRES_DUP == nState )
	{
		// 양쪽 슬롯에 같은 아이템이 올려져 있음.
		MTextArea* pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "StageChattingOutput");
		if ( pTextArea)
			pTextArea->AddText( "^1같은 아이템 2개가 놓여있으므로 게임을 시작할 수 없습니다.");
	}
*/
	return true;
}


bool ZStageInterface::OnNotAllReady()
{
	return true;
}
#endif

void ZStageInterface::UpdateStageGameInfo(const int nQL, const int nMapsetID, const int nScenarioID)
{
	if (!ZGetGameTypeManager()->IsQuestOnly(ZGetGameClient()->GetMatchStageSetting()->GetGameType())) return;

	// 스테이지 영역에 있는 퀘스트 레벨 표시 수정
	MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_QuestLevel");
	if ( pLabel)
	{
		char szText[125];
		sprintf( szText, "%s %s : %d", ZMsg( MSG_WORD_QUEST), ZMsg( MSG_CHARINFO_LEVEL), nQL);
		pLabel->SetText( szText);
	}


#define		MAPSET_NORMAL		MCOLOR(0xFFFFFFFF)
#define		MAPSET_SPECIAL		MCOLOR(0xFFFFFF40)			// Green
//#define		MAPSET_SPECIAL		MCOLOR(0xFFFF2020)		// Red

	// 여기서 시나리오 이름을 보여준다.
	pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_SenarioName");
	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_SenarioNameImg");
	MPicture* pPictureL = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_Lights0");
	MPicture* pPictureR = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_Lights1");
	if ( pLabel)
	{
		if (nScenarioID == 0)
		{
			// 시나리오가 없는 경우
			pLabel->SetText( "");
			if ( pWidget)
				pWidget->Show( false);
			if ( pPictureL) {
				pPictureL->Show( false);
			}
			if ( pPictureR) {
				pPictureR->Show( false);
			}
		}
		else
		{
			pLabel->SetAlignment( MAM_HCENTER | MAM_VCENTER);

			LIST_SCENARIONAME::iterator itr =  m_SenarioDesc.find( nScenarioID);
			if ( itr != m_SenarioDesc.end())
			{
				pLabel->SetText( (*itr).second.m_szName);
				pLabel->SetTextColor(MCOLOR(0xFFFFFF00));
				if ( pWidget)
					pWidget->Show( true);

				if ( pPictureL) {
					pPictureL->Show( true);
					pPictureL->SetBitmapColor(MAPSET_SPECIAL);
				}
				if ( pPictureR) {
					pPictureR->Show( true);
					pPictureR->SetBitmapColor(MAPSET_SPECIAL);
				}
			}
			else
			{
				// 특별시나리오가 없을경우는 정규시나리오이다.
				pLabel->SetText("");
				pLabel->SetTextColor(MCOLOR(0xFFFFFFFF));
				if ( pWidget)
					pWidget->Show( false);

				if ( pPictureL) {
					pPictureL->Show( true);
					pPictureL->SetBitmapColor(MAPSET_NORMAL);
				}
				if ( pPictureR) {
					pPictureR->Show( true);
					pPictureR->SetBitmapColor(MAPSET_NORMAL);
				}

			}
		}
	}
}

/***********************************************************************
  SetSacrificeItemSlot : public
  
  desc : 희생 아이템 슬롯에 아이템 정보를 입력
  arg  : none
  ret  : none
************************************************************************/
void SacrificeItemSlotDesc::SetSacrificeItemSlot( const MUID& uidUserID, const unsigned long int nItemID, MBitmap* pBitmap, const char* szItemName, const int nQL)
{
	m_uidUserID = uidUserID;
	m_nItemID = nItemID;
	m_pIconBitmap = pBitmap;
	strcpy( m_szItemName, szItemName);
	m_nQL = nQL;
	m_bExist = true;
}


/***********************************************************************
  ReadSenarioNameXML : protected
  
  desc : 퀘스트 희생 아이템 XML을 읽는다
  arg  : none
  ret  : true(=success) or false(=fail)
************************************************************************/
bool ZStageInterface::ReadSenarioNameXML( void)
{

	MXmlDocument xmlQuestItemDesc;
	if (!xmlQuestItemDesc.LoadFromFile("System/scenario.xml", ZApplication::GetFileSystem()))
	{
		return false;
	}

	int nStdScenarioCount = 1000;

	MXmlElement rootElement = xmlQuestItemDesc.GetDocumentElement();
	for ( int i = 0;  i < rootElement.GetChildNodeCount();  i++)
	{
		MXmlElement chrElement = rootElement.GetChildNode( i);

		char szTagName[ 256];
		chrElement.GetTagName( szTagName);

		if ( szTagName[ 0] == '#')
			continue;


		char szAttrName[64];
		char szAttrValue[256];
		int nItemID = 0;
		MSenarioList SenarioMapList;

		bool bFind = false;


		// 정규 시나리오
		if ( !stricmp( szTagName, "STANDARD_SCENARIO"))			// 태그 시작
		{
			// Set Tag
			for ( int k = 0;  k < chrElement.GetAttributeCount();  k++)
			{
				chrElement.GetAttribute( k, szAttrName, szAttrValue);

				SenarioMapList.m_ScenarioType = ST_STANDARD;	// Type

				if ( !stricmp( szAttrName, "title"))			// Title
					strcpy( SenarioMapList.m_szName, szAttrValue);

				else if ( !stricmp( szAttrName, "mapset"))		// Map set
					strcpy( SenarioMapList.m_szMapSet, szAttrValue);
			}

			nItemID = nStdScenarioCount++;					// ID

			bFind = true;
		}

		// 특수 시나리오
		else if ( !stricmp( szTagName, "SPECIAL_SCENARIO"))			// 태그 시작
		{
			// Set Tag
			for ( int k = 0;  k < chrElement.GetAttributeCount();  k++)
			{
				chrElement.GetAttribute( k, szAttrName, szAttrValue);

				SenarioMapList.m_ScenarioType = ST_SPECIAL;		// Type

				if ( !stricmp( szAttrName, "id"))				// ID
					nItemID = atoi( szAttrValue);

				else if ( !stricmp( szAttrName, "title"))		// Title
					strcpy( SenarioMapList.m_szName, szAttrValue);

				else if ( !stricmp( szAttrName, "mapset"))		// Map set
					strcpy( SenarioMapList.m_szMapSet, szAttrValue);
			}

			bFind = true;
		}


		if ( bFind)
			m_SenarioDesc.insert( LIST_SCENARIONAME::value_type( nItemID, SenarioMapList));
	}

	xmlQuestItemDesc.Destroy();

	return true;
}

bool ZStageInterface::OnStopVote()
{
	ZGetGameClient()->SetVoteInProgress( false );
	ZGetGameClient()->SetCanVote( false );

#ifdef _DEBUG
	string str = ZMsg(MSG_VOTE_VOTE_STOP);
#endif

	ZChatOutput(ZMsg(MSG_VOTE_VOTE_STOP), ZChat::CMT_SYSTEM, ZChat::CL_CURRENT);
	return true;
}


void ZStageInterface::OnStartFail( const int nType, const MUID& uidParam )
{
	if( ALL_PLAYER_NOT_READY == nType )
	{
		// 모든 유저가 레디를 하지 않았음.
		ZGetGameInterface()->PlayVoiceSound( VOICE_PLAYER_NOT_READY, 1500);
	}
	else if( QUEST_START_FAILED_BY_SACRIFICE_SLOT == nType )
	{
		OnQuestStartFailed( uidParam.Low );

		ZGetGameInterface()->PlayVoiceSound( VOICE_QUEST_START_FAIL, 2800);
	}
	else if( INVALID_TACKET_USER == nType )
	{
		// 자기 자신이면 -_-;;
		//if( uidParam == ZGetMyUID() )
		//{
			char szMsg[ 128 ];

			sprintf( szMsg, MGetStringResManager()->GetErrorStr(MERR_CANNOT_START_NEED_TICKET), ZGetGameClient()->GetObjName(uidParam).c_str() );

			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			ZChatOutput(szMsg, ZChat::CMT_BROADCAST);
		//}

		// 입장권 누가 없는거야? -_-+ - by SungE
		//const MMatchPeerInfo* pPeer = ZGetGameClient()->GetPeers()->Find( uidParam );
		//if( 0 != pPeer )
		//{
		//	char szMsg[ 128 ];

		//	sprintf( szMsg, MGetStringResManager()->GetErrorStr(MERR_CANNOT_START_NEED_TICKET), pPeer->CharInfo.szName );

		//	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		//	ZChatOutput(szMsg, ZChat::CMT_BROADCAST);
		//}
	}
	else if( INVALID_MAP == nType )
	{
		char szMsg[ 128 ];

			sprintf( szMsg, "INVALID MAP!" );
				//MGetStringResManager()->GetErrorStr(MERR_CANNOT_START_NEED_TICKET), 
				//ZApplication::GetInstance()->GetGameClient()->GetObjName(uidParam).c_str() );

			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			ZChatOutput(szMsg, ZChat::CMT_BROADCAST);
	}

	// Stage UI Enable
	ChangeStageEnableReady( false);
}
