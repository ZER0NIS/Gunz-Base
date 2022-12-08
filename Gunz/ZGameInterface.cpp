#include "stdafx.h"

#include "ZGameInterface.h"
#include "ZApplication.h"
#include "ZPost.h"
#include "UPnP.h"
#include "ZConsole.h"
#include "MCommandLogFrame.h"
#include "ZConfiguration.h"
#include "FileInfo.h"
#include "ZInterfaceItem.h"
#include "MPicture.h"
#include "ZInterfaceListener.h"
#include "ZEffectSmoke.h"
#include "ZEffectLightTracer.h"
#include "MProfiler.h"
#include "ZActionDef.h"
#include "MSlider.h"
#include "ZMsgBox.h"
#include "MDebug.h"
#include "MMatchTransDataType.h"
#include "MBlobArray.h"
#include "MListBox.h"
#include "MTextArea.h"
#include "MTabCtrl.h"
#include "MComboBox.h"
#include "ZInterfaceBackground.h"
#include "ZCharacterSelectView.h"
#include "ZCharacterViewList.h"
#include "ZCharacterView.h"
#include "MMatchStage.h"
#include "ZScreenEffectManager.h"
#include "RShaderMgr.h"
#include "ZShopEquipInterface.h"
#include "ZShopEquipListbox.h"
#include "ZMyItemList.h"
#include "ZMyInfo.h"
#include "ZStageSetting.h"
#include "RealSoundEffect.h"
#include "ZInitialLoading.h"
#include "RShaderMgr.h"
#include "zeffectflashbang.h"
#include "MToolTip.h"
#include "ZRoomListbox.h"
#include "ZPlayerListBox.h"
#include "MMatchNotify.h"
#include "ZMapListBox.h"
#include "ZDuelTournamentRankingListBox.h"
#include "ZToolTip.h"
#include "ZCanvas.h"
#include "ZCrossHair.h"
#include "ZPlayerMenu.h"
#include "ZItemMenu.h"
#include "MPanel.h"
#include "ZNetRepository.h"
#include "ZStencilLight.h"
#include "MUtil.h"
#include "ZMap.h"
#include "ZBmNumLabel.h"
#include "ZItemSlotView.h"
#include "ZMapDesc.h"
#include "MStringTable.h"

#include "ZReplay.h"
#include "MFileDialog.h"
#include "ZServerView.h"
#include "ZLocale.h"
#include "ZLocatorList.h"
#include "ZInput.h"
#include "ZActionKey.h"
#include "ZMonsterBookInterface.h"
#include "ZGameInput.h"
#include "ZOptionInterface.h"
#include "ZPlayerManager.h"
#include "ZShop.h"
#include "ZGambleItemDefine.h"

#include "defer.h"
#include "MFile.h"

extern MCommandLogFrame* m_pLogFrame;

static int g_debug_tex_update_cnt;
bool ZGameInterface::m_bSkipGlobalEvent = false;

bool ZGameInterface::m_sbRemainClientConnectionForResetApp = false;
ZGameClient* ZGameInterface::m_spGameClient = NULL;

void ZChangeGameState(GunzState state)
{
	PostMessage(g_hWnd, WM_CHANGE_GAMESTATE, int(state), 0);
}

void ZEmptyBitmap()
{
	MBitmapManager::Destroy();
	MBitmapManager::DestroyAniBitmap();
}

static StringView GetAliasName(const StringView& Path, bool AddDir)
{
	if (AddDir)
		return Path;

	auto SlashIndex = Path.find_last_of("/\\");
	if (SlashIndex == Path.npos)
		return Path;

	return Path.substr(SlashIndex + 1);
}

static void AddBitmap(const StringView& Path, bool AddDirToAliasName)
{
#if defined(_PUBLISH) && defined(ONLY_LOAD_MRS_FILES)
	MZFile::SetReadMode(MZIPREADFLAG_ZIP | MZIPREADFLAG_MRS | MZIPREADFLAG_MRS2 | MZIPREADFLAG_FILE);
#endif

	auto MBitmapR2Create = MBeginProfile("ZGameInterface::LoadBitmaps - MBitmapR2::Create");

	MBitmapR2* pBitmap = new MBitmapR2;
	if (pBitmap->Create(GetAliasName(Path, AddDirToAliasName), RGetDevice(), Path) == true)
		MBitmapManager::Add(pBitmap);
	else
		delete pBitmap;

	MEndProfile(MBitmapR2Create);

#if defined(_PUBLISH) && defined(ONLY_LOAD_MRS_FILES)
	MZFile::SetReadMode(MZIPREADFLAG_MRS2);
#endif
}

void ZGameInterface::LoadBitmaps(const char* szDir, ZLoadingProgress* pLoadingProgress)
{
	mlog("START ZGameInterface::LoadBitmaps\n");

	static const StringView BitmapExtensions[3] = { ".png", ".bmp", ".tga" };

	auto HasBitmapExtension = [&](auto&& Path) {
		auto ExtIndex = Path.find_last_of('.');
		if (ExtIndex == Path.npos)
			return false;

		const auto Ext = Path.substr(ExtIndex);
		auto EqualsExt = [&](auto&& x) { return x == Ext; };
		return std::any_of(std::begin(BitmapExtensions), std::end(BitmapExtensions), EqualsExt);
	};

	auto& FS = *ZGetFileSystem();

	auto AddBitmapsIn = [&](auto&& Path, bool AddDirToAliasName)
	{
		auto DirNode = FS.GetDirectory(Path);
		if (!DirNode)
			return;

		for (auto&& File : FilesInDirRecursive(*DirNode))
		{
			if (HasBitmapExtension(File.Path))
				AddBitmap(File.Path, AddDirToAliasName);
		}
	};

	AddBitmapsIn(szDir, false);
	AddBitmapsIn(PATH_CUSTOM_CROSSHAIR, true);

	mlog("END ZGameInterface::LoadBitmaps\n");
}

void AddListItem(MListBox* pList, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	class MDragableListItem : public MDefaultListItem {
		char m_szDragItemString[256];
	public:
		MDragableListItem(MBitmap* pBitmap, const char* szText, const char* szItemString)
			: MDefaultListItem(pBitmap, szText) {
			if (szItemString != NULL) strcpy(m_szDragItemString, szItemString);
			else m_szDragItemString[0] = 0;
		}
		virtual bool GetDragItem(MBitmap** ppDragBitmap, char* szDragString, char* szDragItemString) {
			*ppDragBitmap = GetBitmap(0);
			if (GetString(1) != NULL) strcpy(szDragString, GetString(1));
			else szDragString[0] = 0;
			strcpy(szDragItemString, m_szDragItemString);
			return true;
		};
	};
	MDefaultListItem* pNew = new MDragableListItem(pBitmap, szString, szItemString);
	pList->Add(pNew);
}

bool InitSkillList(MWidget* pWidget)
{
	if (pWidget == NULL) return false;

	if (strcmp(pWidget->GetClassName(), MINT_LISTBOX) != 0) return false;
	MListBox* pList = (MListBox*)pWidget;

	pList->SetItemHeight(32);
	pList->SetVisibleHeader(false);

	pList->AddField("Icon", 32);
	pList->AddField("Name", 600);
	AddListItem(pList, MBitmapManager::Get("skill000.png"), "Fire-Ball", "Object.Skill $player $target 0");
	AddListItem(pList, MBitmapManager::Get("skill001.png"), "Bull-Shit", "Object.Skill $player $target 1");
	return true;
}

bool InitItemList(MWidget* pWidget)
{
	if (pWidget == NULL) return false;

	if (strcmp(pWidget->GetClassName(), MINT_LISTBOX) != 0) return false;
	MListBox* pList = (MListBox*)pWidget;

	pList->SetVisibleHeader(false);
	pList->SetItemHeight(40);

	pList->AddField("Icon", 42);
	pList->AddField("Name", 600);
	for (int i = 0; i < 30; i++)
	{
		char szName[256], szItem[256];
		int d = i % 6;
		sprintf(szItem, "item%03d.png", d);
		sprintf(szName, "나무블레이드%d", i);
		AddListItem(pList, MBitmapManager::Get(szItem), szName, "Command Something");
	}

	return true;
}

#define DEFAULT_SLIDER_MAX			10000

ZGameInterface::ZGameInterface(const char* szName, MWidget* pParent, MListener* pListener) : ZInterface(szName, pParent, pListener)
{
	MSetString(1, ZMsg(MSG_MENUITEM_OK));
	MSetString(2, ZMsg(MSG_MENUITEM_CANCEL));
	MSetString(3, ZMsg(MSG_MENUITEM_YES));
	MSetString(4, ZMsg(MSG_MENUITEM_NO));
	MSetString(5, ZMsg(MSG_MENUITEM_MESSAGE));

	m_pShopEquipInterface = new ZShopEquipInterface;

	m_bShowInterface = true;
	m_bViewUI = true;
	m_bWaitingArrangedGame = false;

	m_pMyCharacter = NULL;

	SetBounds(0, 0, MGetWorkspaceWidth(), MGetWorkspaceHeight());

	m_pGame = NULL;
	m_pCombatInterface = NULL;
	m_pLoadingInterface = NULL;

	m_dwFrameMoveClock = 0;

	m_nInitialState = GUNZ_LOGIN;
	m_nPreviousState = GUNZ_LOGIN;

	m_nState = GUNZ_NA;

	m_bCursor = true;
	MCursorSystem::Show(m_bCursor);

	m_bLogin = false;
	m_bLoading = false;

	m_pRoomListFrame = NULL;
	m_pDuelTournamentLobbyFrame = NULL;
	m_pBottomFrame = NULL;
	m_pClanInfoBg = NULL;
	m_pDuelTournamentInfoBg = NULL;
	m_pDuelTournamentRankingLabel = NULL;

	m_pMsgBox = new ZMsgBox("", Mint::GetInstance()->GetMainFrame(), this, MT_OK);
	m_pConfirmMsgBox = new ZMsgBox("", Mint::GetInstance()->GetMainFrame(), this, MT_YESNO);

	m_pMonsterBookInterface = new ZMonsterBookInterface();

	m_pMapThumbnail = NULL;

	m_pCharacterSelectView = NULL;
	m_pBackground = new ZInterfaceBackground();

	m_pCursorSurface = NULL;

	if (!m_sbRemainClientConnectionForResetApp)
		m_spGameClient = NULL;

	m_nDrawCount = 0;

	m_bTeenVersion = true;
	m_pScreenEffectManager = NULL;
	m_pEffectManager = NULL;
	m_pGameInput = NULL;

	m_bReservedWeapon = false;
	m_ReservedWeapon = ZCWT_NONE;

	m_bLeaveBattleReserved = false;
	m_bLeaveStageReserved = false;

	Mint::GetInstance()->SetGlobalEvent(ZGameInterface::OnGlobalEvent);
	ZGetInput()->SetEventListener(ZGameInterface::OnGlobalEvent);

	m_pPlayerMenu = NULL;
	m_pMiniMap = NULL;

	m_bOnEndOfReplay = false;
	m_nLevelPercentCache = 0;

	m_pLoginBG = NULL;
	m_pLoginPanel = NULL;

	m_nLoginState = LOGINSTATE_STANDBY;
	m_dwLoginTimer = 0;

	m_nLocServ = 0;

	m_dwHourCount = 0;
	m_dwTimeCount = timeGetTime() + 3600000;

	if (m_pRoomListFrame != NULL)
	{
		delete m_pRoomListFrame;
		m_pRoomListFrame = NULL;
	}
	if (m_pDuelTournamentLobbyFrame != NULL)
	{
		delete m_pDuelTournamentLobbyFrame;
		m_pDuelTournamentLobbyFrame = NULL;
	}
	if (m_pBottomFrame != NULL)
	{
		delete m_pBottomFrame;
		m_pBottomFrame = NULL;
	}
	if (m_pClanInfoBg != NULL)
	{
		delete m_pClanInfoBg;
		m_pClanInfoBg = NULL;
	}
	if (m_pDuelTournamentInfoBg != NULL)
	{
		delete m_pDuelTournamentInfoBg;
		m_pDuelTournamentInfoBg = NULL;
	}
	if (m_pDuelTournamentRankingLabel != NULL)
	{
		delete m_pDuelTournamentRankingLabel;
		m_pDuelTournamentRankingLabel = NULL;
	}

	if (m_pLoginBG != NULL)
	{
		delete m_pLoginBG;
		m_pLoginBG = NULL;
	}

	if (m_pLoginPanel != NULL)
	{
		delete m_pLoginPanel;
		m_pLoginPanel = NULL;
	}

	m_dwRefreshTime = 0;

	m_pLocatorList = ZGetConfiguration()->GetLocatorList();
	m_pTLocatorList = ZGetConfiguration()->GetTLocatorList();

	m_dwVoiceTime = 0;
	m_szCurrVoice[0] = 0;
	m_szNextVoice[0] = 0;
	m_dwNextVoiceTime = 0;

	m_bReservedQuit = false;
	m_bReserveResetApp = false;

	m_dErrMaxPalyerDelayTime = 0;
	m_bErrMaxPalyer = false;

	m_bGameFinishLeaveBattle = true;

	m_MyPort = 0;
}

ZGameInterface::~ZGameInterface()
{
	ZEmptyBitmap();

	OnDestroy();

	SAFE_DELETE(m_pMiniMap);
	SAFE_RELEASE(m_pCursorSurface);
	SAFE_DELETE(m_pScreenEffectManager);
	SAFE_DELETE(m_pEffectManager);
	SAFE_DELETE(m_pCharacterSelectView);
	SAFE_DELETE(m_pBackground);
	SAFE_DELETE(m_pMsgBox);
	SAFE_DELETE(m_pConfirmMsgBox);
	SAFE_DELETE(m_pMonsterBookInterface);

	SAFE_DELETE(m_pRoomListFrame);
	SAFE_DELETE(m_pDuelTournamentLobbyFrame);
	SAFE_DELETE(m_pBottomFrame);
	SAFE_DELETE(m_pClanInfoBg);
	SAFE_DELETE(m_pDuelTournamentInfoBg);
	SAFE_DELETE(m_pDuelTournamentRankingLabel);

	SAFE_DELETE(m_pLoginPanel);

	SAFE_DELETE(m_pShopEquipInterface);
}

bool ZGameInterface::InitInterface(const char* szSkinName, ZLoadingProgress* pLoadingProgress)
{
	DWORD _begin_time, _end_time;
#define BEGIN_ { _begin_time = timeGetTime(); }
#define END_(x) { _end_time = timeGetTime(); float f_time = (_end_time - _begin_time) / 1000.f; mlog("%s : %f \n", x,f_time ); }

	SetObjectTextureLevel(ZGetConfiguration()->GetVideo()->nCharTexLevel);
	SetMapTextureLevel(ZGetConfiguration()->GetVideo()->nMapTexLevel);
	SetEffectLevel(ZGetConfiguration()->GetVideo()->nEffectLevel);
	SetTextureFormat(ZGetConfiguration()->GetVideo()->nTextureFormat);
#ifndef _DEBUG
	SetBandiCaptureConfig(ZGetConfiguration()->GetMovingPicture()->iResolution);
	SetBandiCaptureFileSize(ZGetConfiguration()->GetMovingPicture()->iFileSize);
#endif

	bool bRet = true;
	char szPath[256];
	char szSubPath[256] = "interface/grm/default/";
	char szFileName[256];
	char a_szSkinName[256];
	strcpy(a_szSkinName, szSkinName);

	ZGetInterfaceSkinPath(szPath, a_szSkinName);
	ZGetInterfaceSkinPathSubLanguage(szSubPath, a_szSkinName);
	sprintf(szFileName, "%s%s", szPath, FILENAME_INTERFACE_MAIN);

	ZEmptyBitmap();

	ZLoadingProgress pictureProgress("pictures", pLoadingProgress, .7f);
	BEGIN_;
	LoadBitmaps(szPath, &pictureProgress);

	END_("loading pictures");

	BEGIN_;
	if (!m_IDLResource.LoadFromFile(szFileName, this, ZGetFileSystem()))
	{
		strcpy(a_szSkinName, DEFAULT_INTERFACE_SKIN);
		ZGetInterfaceSkinPath(szPath, a_szSkinName);
		sprintf(szFileName, "%s%s", szPath, FILENAME_INTERFACE_MAIN);
		LoadBitmaps(szPath, &pictureProgress);

		if (m_IDLResource.LoadFromFile(szFileName, this, ZGetFileSystem()))
		{
			mlog("IDLResource Loading Success!!\n");
		}
		else
		{
			mlog("IDLResource Loading Failed!!\n");
		}
		bRet = false;
	}
	else
	{
		mlog("IDLResource Loading Success!!\n");
	}
	END_("IDL resources");

	MBFrameLook* pFrameLook = (MBFrameLook*)m_IDLResource.FindFrameLook("DefaultFrameLook");
	if (pFrameLook != NULL)
	{
		m_pMsgBox->ChangeCustomLook((MFrameLook*)pFrameLook);
		m_pConfirmMsgBox->ChangeCustomLook((MFrameLook*)pFrameLook);
	}
	else
	{
	}

	ZStageSetting::InitStageSettingDialog();

	ZShopEquipListbox* pShopEquipListBox = (ZShopEquipListbox*)m_IDLResource.FindWidget("AllEquipmentList");
	if (pShopEquipListBox)
	{
		pShopEquipListBox->SetOnDropCallback(ShopPurchaseItemListBoxOnDrop);
	}
	pShopEquipListBox = (ZShopEquipListbox*)m_IDLResource.FindWidget("MyAllEquipmentList");
	if (pShopEquipListBox)
	{
		pShopEquipListBox->SetOnDropCallback(ShopSaleItemListBoxOnDrop);
	}

	pShopEquipListBox = (ZShopEquipListbox*)m_IDLResource.FindWidget("EquipmentList");
	if (pShopEquipListBox)
	{
		pShopEquipListBox->SetOnDropCallback(CharacterEquipmentItemListBoxOnDrop);
	}

	pShopEquipListBox = (ZShopEquipListbox*)m_IDLResource.FindWidget("AccountItemList");
	if (pShopEquipListBox)
	{
	}

	m_textAreaLookItemDesc.SetBgColor(MCOLOR(10, 10, 10, 220));

	MTextArea* pTextArea = (MTextArea*)m_IDLResource.FindWidget("Shop_ItemDescription");
	if (pTextArea) pTextArea->ChangeCustomLook(&m_textAreaLookItemDesc);
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("Equip_ItemDescription");
	if (pTextArea) pTextArea->ChangeCustomLook(&m_textAreaLookItemDesc);
	InitInterfaceListener();

#define CENTERMESSAGE	"CenterMessage"
	BEGIN_WIDGETLIST(CENTERMESSAGE, &m_IDLResource, MLabel*, pWidget);
	pWidget->SetAlignment(MAM_HCENTER);
	END_WIDGETLIST();

	ZGetOptionInterface()->InitInterfaceOption();

#ifdef _FASTDEBUG
	m_IDLResource.FindWidget("LoginPassword")->SetText("1111");
#endif
	InitMaps(m_IDLResource.FindWidget("MapSelection"));

	m_pPlayerMenu = new ZPlayerMenu("PlayerMenu", this, this, MPMT_VERTICAL);
	((MPopupMenu*)m_pPlayerMenu)->Show(false);

	return true;
}

bool ZGameInterface::InitInterfaceListener()
{
	m_pMsgBox->SetListener(ZGetMsgBoxListener());
	m_pConfirmMsgBox->SetListener(ZGetConfirmMsgBoxListener());

	SetListenerWidget("VideoGamma", ZGetOptionGammaSliderChangeListener());
	SetListenerWidget("ResizeCancel", ZGetCancelResizeConfirmListener());
	SetListenerWidget("ResizeRequest", ZGetRequestResizeListener());
	SetListenerWidget("ResizeReject", ZGetViewConfirmCancelListener());
	SetListenerWidget("ResizeAccept", ZGetViewConfrimAcceptListener());
	SetListenerWidget("16BitSound", ZGet16BitSoundListener());
	SetListenerWidget("8BitSound", ZGet8BitSoundListener());

	SetListenerWidget("NetworkPortChangeRestart", ZGetNetworkPortChangeRestartListener());
	SetListenerWidget("NetworkPortChangeCancel", ZGetNetworkPortChangeCancelListener());

	SetListenerWidget("MouseSensitivitySlider", ZGetMouseSensitivitySliderListener());
	SetListenerWidget("MouseSensitivityEdit", ZGetMouseSensitivityEditListener());

	SetListenerWidget("Exit", ZGetExitListener());
	SetListenerWidget("LoginOK", ZGetLoginListener());
	SetListenerWidget("OptionFrame", ZGetOptionFrameButtonListener());
	SetListenerWidget("Register", ZGetRegisterListener());
	SetListenerWidget("Stage_OptionFrame", ZGetOptionFrameButtonListener());
	SetListenerWidget("LobbyOptionFrame", ZGetOptionFrameButtonListener());

	SetListenerWidget("WarningExit", ZGetExitListener());

	SetListenerWidget("Logout", ZGetLogoutListener());
	SetListenerWidget("ChannelChattingInput", ZGetChannelChatInputListener());
	SetListenerWidget("GameStart", ZGetGameStartListener());
	SetListenerWidget("ChatInput", ZGetChatInputListener());
	SetListenerWidget("ParentClose", ZGetParentCloseListener());
	SetListenerWidget("ChannelListFrameCaller", ZGetChannelListFrameCallerListener());
	SetListenerWidget("MapList", ZGetMapListListener());
	SetListenerWidget("Lobby_StageExit", ZGetLobbyListener());
	SetListenerWidget("LoginState", ZGetLoginStateButtonListener());
	SetListenerWidget("GreeterState", ZGetGreeterStateButtonListener());
	SetListenerWidget("CombatMenuClose", ZGetCombatMenuCloseButtonListener());
	SetListenerWidget("PreviousState", ZGetPreviousStateButtonListener());
	SetListenerWidget("QuickJoin", ZGetQuickJoinButtonListener());
	SetListenerWidget("QuickJoin2", ZGetQuickJoinButtonListener());
	SetListenerWidget("Lobby_Charviewer_info", ZGetLobbyCharInfoCallerButtonListener());
	SetListenerWidget("StageBeforeBtn", ZGetLobbyPrevRoomListButtonListener());
	SetListenerWidget("StageAfterBtn", ZGetLobbyNextRoomListPrevButtonListener());
	SetListenerWidget("Lobby_RoomNo1", ZGetLobbyNextRoomNoButtonListener());
	SetListenerWidget("Lobby_RoomNo2", ZGetLobbyNextRoomNoButtonListener());
	SetListenerWidget("Lobby_RoomNo3", ZGetLobbyNextRoomNoButtonListener());
	SetListenerWidget("Lobby_RoomNo4", ZGetLobbyNextRoomNoButtonListener());
	SetListenerWidget("Lobby_RoomNo5", ZGetLobbyNextRoomNoButtonListener());
	SetListenerWidget("Lobby_RoomNo6", ZGetLobbyNextRoomNoButtonListener());

	SetListenerWidget("LobbyChannelPlayerListPrev", ZGetPlayerListPrevListener());
	SetListenerWidget("LobbyChannelPlayerListNext", ZGetPlayerListNextListener());

	SetListenerWidget("StagePlayerListPrev", ZGetStagePlayerListPrevListener());
	SetListenerWidget("StagePlayerListNext", ZGetStagePlayerListNextListener());

	SetListenerWidget("RequestLadderRejoin", ZGetRequestLadderRejoinGameListener());

	SetListenerWidget("PlayerWarsVoteButton0", ZGetPlayerWarsVote0());
	SetListenerWidget("PlayerWarsVoteButton1", ZGetPlayerWarsVote1());
	SetListenerWidget("PlayerWarsVoteButton2", ZGetPlayerWarsVote2());

	SetListenerWidget("ArrangedTeamGame", ZGetArrangedTeamGameListener());
	SetListenerWidget("ArrangedTeamDialogOk", ZGetArrangedTeamDialogOkListener());
	SetListenerWidget("ArrangedTeamDialogClose", ZGetArrangedTeamDialogCloseListener());
	SetListenerWidget("ArrangedTeamGame_Cancel", ZGetArrangedTeamGame_CancelListener());

	SetListenerWidget("LeaveClanOK", ZGetLeaveClanOKListener());
	SetListenerWidget("LeaveClanCancel", ZGetLeaveClanCancelListener());

	SetListenerWidget("DuelTournamentGame", ZGetDuelTournamentGameButtonListener());
	SetListenerWidget("DuelTournamentGame_2Test", ZGetDuelTournamentGame2TestButtonListener());
	SetListenerWidget("DuelTournamentGame_4Test", ZGetDuelTournamentGame4TestButtonListener());
	SetListenerWidget("DuelTournamentWaitMatchCancel", ZGetDuelTournamentWaitCancelButtonListener());

	SetListenerWidget("ChannelJoin", ZGetChannelListJoinButtonListener());
	SetListenerWidget("ChannelListClose", ZGetChannelListCloseButtonListener());
	SetListenerWidget("ChannelList", ZGetChannelListListener());
	SetListenerWidget("PrivateChannelEnter", ZGetPrivateChannelEnterListener());

	SetListenerWidget("ChannelList_Normal", ZGetChannelList());
	SetListenerWidget("ChannelList_Private", ZGetChannelList());
	SetListenerWidget("ChannelList_Clan", ZGetChannelList());
	SetListenerWidget("ChannelList_DuelTournament", ZGetChannelList());
	SetListenerWidget("MyClanChannel", ZGetMyClanChannel());

	SetListenerWidget("StageCreateFrameCaller", ZGetStageCreateFrameCallerListener());

	SetListenerWidget("StageListFrameCaller", ZGetStageListFrameCallerListener());
	SetListenerWidget("StageCreateBtn", ZGetStageCreateBtnListener());
	SetListenerWidget("PrivateStageJoinBtn", ZGetPrivateStageJoinBtnListener());
	SetListenerWidget("StageJoin", ZGetStageJoinListener());
	SetListenerWidget("StageChattingInput", ZGetStageChatInputListener());
	SetListenerWidget("StageSettingCaller", ZGetStageSettingCallerListener());
	SetListenerWidget("StageType", ZGetStageSettingStageTypeListener());
	SetListenerWidget("StageMaxPlayer", ZGetStageSettingChangedComboboxListener());
	SetListenerWidget("StageRoundCount", ZGetStageSettingChangedComboboxListener());
	SetListenerWidget("StageLimitTime", ZGetStageSettingChangedComboboxListener());
	SetListenerWidget("StageIntrude", ZGetStageSettingChangedComboboxListener());
	SetListenerWidget("StageTeamRed", ZGetStageTeamRedListener());
	SetListenerWidget("StageTeamRed2", ZGetStageTeamRedListener());
	SetListenerWidget("StageTeamBlue", ZGetStageTeamBlueListener());
	SetListenerWidget("StageTeamBlue2", ZGetStageTeamBlueListener());
	SetListenerWidget("StageObserverBtn", ZGetStageObserverBtnListener());
	SetListenerWidget("StageReady", ZGetStageReadyListener());
	SetListenerWidget("StageSettingApplyBtn", ZGetStageSettingApplyBtnListener());
	SetListenerWidget("BattleExit", ZGetBattleExitButtonListener());
	SetListenerWidget("StageExit", ZGetStageExitButtonListener());

	SetListenerWidget("MapChange", ZGetMapChangeListener());
	SetListenerWidget("MapSelect", ZGetMapSelectListener());

	SetListenerWidget("MapList", ZGetStageMapListSelectionListener());
	SetListenerWidget("Stage_SacrificeItemButton0", ZStageSacrificeItem0());
	SetListenerWidget("Stage_SacrificeItemButton1", ZStageSacrificeItem1());
	SetListenerWidget("Stage_PutSacrificeItem", ZStagePutSacrificeItem());
	SetListenerWidget("Stage_SacrificeItemBoxOpen", ZStageSacrificeItemBoxOpen());
	SetListenerWidget("Stage_SacrificeItemBoxClose", ZStageSacrificeItemBoxClose());
	SetListenerWidget("Stage_SacrificeItemListbox", ZGetSacrificeItemListBoxListener());
	SetListenerWidget("MonsterBookCaller", ZGetMonsterBookCaller());

	SetListenerWidget("MapSelection", ZGetMapComboListener());

	SetListenerWidget("Stage_RelayMapType", ZGetRelayMapTypeListener());
	SetListenerWidget("Stage_RelayMapRepeatCount", ZGetRelayMapTurnCountListener());
	SetListenerWidget("Stage_RelayMapListbox", ZGetRelayMapListBoxListener());
	SetListenerWidget("Stage_MapListbox", ZGetMapListBoxListener());
	SetListenerWidget("Stage_RelayMap_OK_Button", ZGetRelayMapOKButtonListener());
	SetListenerWidget("Stage_RelayMapBoxOpen", ZStageRelayMapBoxOpen());
	SetListenerWidget("Stage_RelayMapBoxClose", ZStageRelayMapBoxClose());

	SetListenerWidget("SaveOptionButton", ZGetSaveOptionButtonListener());
	SetListenerWidget("CancelOptionButton", ZGetCancelOptionButtonListener());

	SetListenerWidget("DefaultSettingLoad", ZGetLoadDefaultKeySettingListener());
	SetListenerWidget("Optimization", ZSetOptimizationListener());

	SetListenerWidget("ShopCaller", ZGetShopCallerButtonListener());
	SetListenerWidget("ShopClose", ZGetShopCloseButtonListener());

	SetListenerWidget("EquipmentCaller", ZGetEquipmentCallerButtonListener());
	SetListenerWidget("EquipmentClose", ZGetEquipmentCloseButtonListener());

	SetListenerWidget("CharSelectionCaller", ZGetCharSelectionCallerButtonListener());

	SetListenerWidget("BuyConfirmCaller", ZGetBuyButtonListener());

	SetListenerWidget("SellConfirmCaller", ZGetSellButtonListener());
	SetListenerWidget("TradeCountableItem_Cancel", ZGetCountableItemTradeDlgCloseListener());
	SetListenerWidget("TradeCountableItem_Ok", ZGetCountableItemTradeDlgOkButtonListener());
	SetListenerWidget("TradeCountableItem_CountUp", ZGetCountableItemTradeDlgCountUpButtonListener());
	SetListenerWidget("TradeCountableItem_CountDn", ZGetCountableItemTradeDlgCountDnButtonListener());
	SetListenerWidget("TradeCountableItem_CountNum", ZGetCountableItemTradeDlgCountChangeListener());
	SetListenerWidget("SellCashItemConfirmFrame_Sell", ZGetSellCashItemConfirmDlgOkButtonListener());
	SetListenerWidget("SellCashItemConfirmFrame_Cancel", ZGetSellCashItemConfirmDlgCancelListener());
	SetListenerWidget("Equip", ZGetEquipButtonListener());
	SetListenerWidget("EquipmentSearch", ZGetEquipmentSearchButtonListener());
	SetListenerWidget("ForcedEntryToGame", ZGetStageForcedEntryToGameListener());
	SetListenerWidget("ForcedEntryToGame2", ZGetStageForcedEntryToGameListener());
	SetListenerWidget("AllEquipmentListCaller", ZGetAllEquipmentListCallerButtonListener());
	SetListenerWidget("MyAllEquipmentListCaller", ZGetMyAllEquipmentListCallerButtonListener());
	SetListenerWidget("CashEquipmentListCaller", ZGetCashEquipmentListCallerButtonListener());
	SetListenerWidget("CashRecharge", ZGetShopCachRechargeButtonListener());
	SetListenerWidget("ShopSearchFrameCaller", ZGetShopSearchCallerButtonListener());

	SetListenerWidget("AllEquipmentList", ZGetShopPurchaseItemListBoxListener());
	SetListenerWidget("MyAllEquipmentList", ZGetShopSellItemListBoxListener());
	SetListenerWidget("Shop_AllEquipmentFilter", ZGetShopListFilterListener());
	SetListenerWidget("Equip_AllEquipmentFilter", ZGetEquipListFilterListener());
	SetListenerWidget("Shop_to_Equipment", ZGetShopEquipmentCallerButtonListener());

	SetListenerWidget("Shop_ArmorEquipmentTab", ZShopItemEquipmentTabOpen());
	SetListenerWidget("Shop_WeaponEquipmentTab", ZShopWeaponEquipmentTabOpen());
	SetListenerWidget("Shop_EquipListFrameCloseButton", ZShopListFrameClose());
	SetListenerWidget("Shop_EquipListFrameOpenButton", ZShopListFrameOpen());

	SetListenerWidget("Equipment_CharacterTab", ZGetEquipmentCharacterTabButtonListener());
	SetListenerWidget("Equipment_AccountTab", ZGetEquipmentAccountTabButtonListener());
	SetListenerWidget("EquipmentList", ZGetEquipmentMyItemListBoxListener());
	SetListenerWidget("AccountItemList", ZGetAccountItemListBoxListener());
	SetListenerWidget("Equipment_to_Shop", ZGetEquipmentShopCallerButtonListener());
	SetListenerWidget("Equip_ArmorEquipmentTab", ZEquipItemEquipmentTabOpen());
	SetListenerWidget("Equip_WeaponEquipmentTab", ZEquipWeaponEquipmentTabOpen());
	SetListenerWidget("Equip_EquipListFrameOpenButton", ZEquipListFrameOpen());
	SetListenerWidget("Equip_EquipListFrameCloseButton", ZEquipListFrameClose());
	SetListenerWidget("Equipment_CharacterRotate", ZEquipmetRotateBtn());

	SetListenerWidget("BringAccountItemBtn", ZGetBringAccountItemButtonListener());
	SetListenerWidget("SendAccountItemBtn", ZGetSendAccountItemButtonListener());

	SetListenerWidget("CS_SelectChar", ZGetSelectCharacterButtonListener());
	SetListenerWidget("CS_SelectCharDefKey", ZGetSelectCharacterButtonListener());
	SetListenerWidget("CS_CreateChar", ZGetShowCreateCharacterButtonListener());
	SetListenerWidget("CS_DeleteChar", ZGetDeleteCharacterButtonListener());
	SetListenerWidget("CS_Prev", ZGetLogoutListener());
	SetListenerWidget("CharSel_SelectBtn0", ZGetSelectCharacterButtonListener0());
	SetListenerWidget("CharSel_SelectBtn1", ZGetSelectCharacterButtonListener1());
	SetListenerWidget("CharSel_SelectBtn2", ZGetSelectCharacterButtonListener2());
	SetListenerWidget("CharSel_SelectBtn3", ZGetSelectCharacterButtonListener3());
	SetListenerWidget("ShowChar_infoGroup", ZGetShowCharInfoGroupListener());
	SetListenerWidget("ShowEquip_InfoGroup", ZGetShowEquipInfoGroupListener());

	SetListenerWidget("CC_CreateChar", ZGetCreateCharacterButtonListener());
	SetListenerWidget("CC_Cancel", ZGetCancelCreateCharacterButtonListener());
	SetListenerWidget("CC_Sex", ZChangeCreateCharInfoListener());
	SetListenerWidget("CC_Hair", ZChangeCreateCharInfoListener());
	SetListenerWidget("CC_Face", ZChangeCreateCharInfoListener());
	SetListenerWidget("CC_Face", ZChangeCreateCharInfoListener());
	SetListenerWidget("CC_Costume", ZChangeCreateCharInfoListener());
	SetListenerWidget("Charviewer_Rotate_L", ZGetCreateCharacterLeftButtonListener());
	SetListenerWidget("Charviewer_Rotate_R", ZGetCreateCharacterRightButtonListener());

	SetListenerWidget("Lobby_Charviewer_Rotate_L", ZGetSelectCameraLeftButtonListener());
	SetListenerWidget("Lobby_Charviewer_Rotate_R", ZGetSelectCameraRightButtonListener());

	SetListenerWidget("CS_DeleteCharButton", ZGetConfirmDeleteCharacterButtonListener());
	SetListenerWidget("CS_CloseConfirmDeleteCharButton", ZGetCloseConfirmDeleteCharButtonListener());

	SetListenerWidget("MonsterBook_PrevPageButton", ZGetMonsterInterfacePrevPage());
	SetListenerWidget("MonsterBook_NextPageButton", ZGetMonsterInterfaceNextPage());
	SetListenerWidget("MonsterBook_Close", ZGetMonsterInterfaceQuit());
	SetListenerWidget("MonsterBook_Close2", ZGetMonsterInterfaceQuit());

	SetListenerWidget("GameResult_ButtonQuit", ZGetGameResultQuit());

	SetListenerWidget("112_ConfirmEdit", ZGet112ConfirmEditListener());
	SetListenerWidget("112_ConfirmOKButton", ZGet112ConfirmOKButtonListener());
	SetListenerWidget("112_ConfirmCancelButton", ZGet112ConfirmCancelButtonListener());

	SetListenerWidget("ClanSponsorAgreementConfirm_OK", ZGetClanSponsorAgreementConfirm_OKButtonListener());
	SetListenerWidget("ClanSponsorAgreementConfirm_Cancel", ZGetClanSponsorAgreementConfirm_CancelButtonListener());
	SetListenerWidget("ClanSponsorAgreementWait_Cancel", ZGetClanSponsorAgreementWait_CancelButtonListener());
	SetListenerWidget("ClanJoinerAgreementConfirm_OK", ZGetClanJoinerAgreementConfirm_OKButtonListener());
	SetListenerWidget("ClanJoinerAgreementConfirm_Cancel", ZGetClanJoinerAgreementConfirm_CancelButtonListener());
	SetListenerWidget("ClanJoinerAgreementWait_Cancel", ZGetClanJoinerAgreementWait_CancelButtonListener());

	SetListenerWidget("LobbyPlayerListTabClanCreateButton", ZGetLobbyPlayerListTabClanCreateButtonListener());
	SetListenerWidget("ClanCreateDialogOk", ZGetClanCreateDialogOk());
	SetListenerWidget("ClanCreateDialogClose", ZGetClanCreateDialogClose());

	SetListenerWidget("ProposalAgreementWait_Cancel", ZGetProposalAgreementWait_CancelButtonListener());
	SetListenerWidget("ProposalAgreementConfirm_OK", ZGetProposalAgreementConfirm_OKButtonListener());
	SetListenerWidget("ProposalAgreementConfirm_Cancel", ZGetProposalAgreementConfirm_CancelButtonListener());

	SetListenerWidget("ReplayOkButton", ZReplayOk());
	SetListenerWidget("ReplayCaller", ZGetReplayCallerButtonListener());
	SetListenerWidget("Replay_View", ZGetReplayViewButtonListener());
	SetListenerWidget("ReplayClose", ZGetReplayExitButtonListener());
	SetListenerWidget("Replay_FileList", ZGetReplayFileListBoxListener());

	SetListenerWidget("PlayerWarsMapVoteWidget", ZGetArrangedPlayerWarsListenerOpen());
	SetListenerWidget("ClosePlayerWarsMapVote", ZGetArrangedPlayerWarsListenerCloser());

	MTabCtrl* pTab = (MTabCtrl*)m_IDLResource.FindWidget("PlayerListControl");
	if (pTab) pTab->UpdateListeners();

	return true;
}

void ZGameInterface::FinalInterface()
{
	SAFE_DELETE(m_pPlayerMenu);
	m_IDLResource.Clear();
	mlog("m_IDLResource.Clear() End : \n");
	SetCursor(NULL);
	mlog("ZGameInterface::FinalInterface() End: \n");
}

bool ZGameInterface::ChangeInterfaceSkin(const char* szNewSkinName)
{
	char szPath[256];
	char szFileName[256];
	ZGetInterfaceSkinPath(szPath, szNewSkinName);
	sprintf(szFileName, "%s%s", szPath, FILENAME_INTERFACE_MAIN);

	FinalInterface();
	bool bSuccess = InitInterface(szNewSkinName);

	if (bSuccess)
	{
		switch (m_nState)
		{
		case GUNZ_LOGIN:	ShowWidget("Login", true); break;
		case GUNZ_LOBBY:	ShowWidget("Lobby", true); 	break;
		case GUNZ_STAGE: 	ShowWidget("Stage", true); 	break;
		case GUNZ_CHARSELECTION:
			if (m_bShowInterface)
			{
				ShowWidget("CharSelection", true);
			}break;
		case GUNZ_CHARCREATION: ShowWidget("CharCreation", true); break;
		}
		ZGetOptionInterface()->Resize(MGetWorkspaceWidth(), MGetWorkspaceHeight());
	}

	return bSuccess;
}

static bool g_parts[10];
static bool g_parts_change;

bool ZGameInterface::ShowWidget(const char* szName, bool bVisible, bool bModal)
{
	MWidget* pWidget = m_IDLResource.FindWidget(szName);

	if (pWidget == NULL)
		return false;

	if (strcmp(szName, "Lobby") == 0)
	{
		pWidget->Show(bVisible, bModal);

		pWidget = m_IDLResource.FindWidget("Shop");
		pWidget->Show(false);
		pWidget = m_IDLResource.FindWidget("Equipment");
		pWidget->Show(false);
	}
	else
		pWidget->Show(bVisible, bModal);

	return true;
}

void ZGameInterface::SetListenerWidget(const char* szName, MListener* pListener)
{
	BEGIN_WIDGETLIST(szName, &m_IDLResource, MWidget*, pWidget);
	pWidget->SetListener(pListener);
	END_WIDGETLIST();
}

void ZGameInterface::EnableWidget(const char* szName, bool bEnable)
{
	MWidget* pWidget = m_IDLResource.FindWidget(szName);
	if (pWidget) pWidget->Enable(bEnable);
}

void ZGameInterface::SetTextWidget(const char* szName, const char* szText)
{
	BEGIN_WIDGETLIST(szName, &m_IDLResource, MWidget*, pWidget);
	pWidget->SetText(szText);
	END_WIDGETLIST();
}

bool ZGameInterface::OnGameCreate(void)
{
	HideAllWidgets();

	GetItemThumbnailMgr()->UnloadTextureTemporarily();

	m_Camera.Init();
	ClearMapThumbnail();

	g_parts[6] = true;
	ZApplication::GetSoundEngine()->CloseMusic();
	m_bLoading = true;

	ZLoadingProgress gameLoading("Game");

	ZGetInitialLoading()->Initialize(1, 0, 0, RGetScreenWidth(), RGetScreenHeight(), 0, 0, 1024, 768, true);

	char szFileName[256];
	int nBitmap = rand() % 3;
	switch (nBitmap)
	{
	case (0):
		strcpy(szFileName, "interface/Loading/loading_1.jpg");
		break;
	case (1):
		strcpy(szFileName, "interface/Loading/loading_2.jpg");
		break;
	case (2):
		strcpy(szFileName, "interface/Loading/loading_3.jpg");
		break;
	case (3):
		strcpy(szFileName, "interface/Loading/loading_4.jpg");
		break;
	case (4):
		strcpy(szFileName, "interface/Loading/loading_5.jpg");
		break;
	case (5):
		strcpy(szFileName, "interface/Loading/loading_6.jpg");
		break;
	case (6):
		strcpy(szFileName, "interface/Loading/loading_7.jpg");
		break;
	case (7):
		strcpy(szFileName, "interface/Loading/loading_8.jpg");
		break;
	case (8):
		strcpy(szFileName, "interface/Loading/loading_9.jpg");
		break;
	}

	ZGetInitialLoading()->AddBitmap(0, szFileName);
	ZGetInitialLoading()->AddBitmapBar("interface/Loading/loading.bmp");
	ZGetInitialLoading()->SetPercentage(0.0f);
	ZGetInitialLoading()->Draw(MODE_FADEIN, 0, true);

	m_pGame = new ZGame;
	if (!m_pGame->Create(ZApplication::GetFileSystem(), &gameLoading))
	{
		mlog("ZGame 생성 실패\n");
		SAFE_DELETE(m_pGame);
		m_bLoading = false;
		ZPostStageLeave(ZGetGameClient()->GetPlayerUID());
		ZApplication::GetGameInterface()->SetState(GUNZ_LOBBY);

		ZGetInitialLoading()->Release();

		return false;
	}

	m_pMyCharacter = (ZMyCharacter*)ZGetGame()->m_pMyCharacter;

	SetFocus();

	m_pGameInput = new ZGameInput();

	m_pCombatInterface = new ZCombatInterface("combatinterface", this, this);
	m_pCombatInterface->SetBounds(GetRect());
	m_pCombatInterface->OnCreate();

	MWidget* pWidget = m_IDLResource.FindWidget("SkillFrame");
	if (pWidget != NULL) pWidget->Show(true);

	InitSkillList(m_IDLResource.FindWidget("SkillList"));

	pWidget = m_IDLResource.FindWidget("InventoryFrame");
	if (pWidget != NULL) pWidget->Show(true);

	InitItemList(m_IDLResource.FindWidget("ItemList"));

	SetCursorEnable(false);

	m_bLoading = false;

#ifndef _FASTDEBUG
	ZGetInitialLoading()->SetPercentage(100.0f);
	ZGetInitialLoading()->Draw(MODE_FADEOUT, 0, true);
#endif
	ZGetInitialLoading()->Release();

	if ((ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_REPLAY) ||
		(ZGetGameClient()->IsLadderGame()) ||
		ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()) ||
		ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
	{
		m_CombatMenu.EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, false);
	}
	else
	{
		m_CombatMenu.EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, true);
	}

#ifndef _DEBUG
	m_Capture = new ZBandiCapturer;
	m_Capture->Init(g_hWnd, (LPDIRECT3DDEVICE9)RGetDevice());
#endif

	return true;
}

void ZGameInterface::OnGameDestroy(void)
{
	mlog("ZGameInterface::OnGameDestroy - Started\n");

	MPicture* pPicture;
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_ClanBitmap1");
	if (pPicture) pPicture->SetBitmap(NULL);
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_ClanBitmap2");
	if (pPicture) pPicture->SetBitmap(NULL);

	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("GameResult");
	if (pWidget) {
		MFrame* pFrame = (MFrame*)pWidget;
		pFrame->MFrame::Show(false);
	}

	ZGetGameClient()->RequestOnGameDestroyed();

	SAFE_DELETE(m_pMiniMap);

	if (m_pGameInput)
	{
		delete m_pGameInput; m_pGameInput = NULL;
	}

	if (m_pCombatInterface)
	{
		m_pCombatInterface->OnDestroy();
		delete m_pCombatInterface;
		m_pCombatInterface = NULL;
	}

	ShowWidget(CENTERMESSAGE, false);

	if (ZGetGame() != NULL) {
		ZGetGame()->Destroy();
		SAFE_DELETE(m_pGame);
	}

	SetCursorEnable(true);
	m_bLeaveBattleReserved = false;
	m_bLeaveStageReserved = false;

#ifndef _DEBUG
	SAFE_DELETE(m_Capture);
#endif

	mlog("ZGameInterface::OnGameDestroy - Finished\n");
}

void ZGameInterface::OnGreeterCreate(void)
{
	ShowWidget("Greeter", true);

	if (m_pBackground)
		m_pBackground->SetScene(LOGIN_SCENE_FIXEDSKY);

	ZApplication::GetSoundEngine()->StopMusic();
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_INTRO, ZApplication::GetFileSystem());
}

void ZGameInterface::OnGreeterDestroy(void)
{
	ShowWidget("Greeter", false);

	if (m_pBackground)
		m_pBackground->SetScene(LOGIN_SCENE_FALLDOWN);
}

void ZGameInterface::OnLoginCreate(void)
{
	m_bLoginTimeout = false;
	m_nLoginState = LOGINSTATE_FADEIN;
	m_dwLoginTimer = timeGetTime();

	if (m_pLoginBG != NULL)
	{
		delete m_pLoginBG;
		m_pLoginBG = NULL;
	}
	m_pLoginBG = new MBitmapR2;
	bool bRead = false;

	bRead = m_pLoginBG->Create("loginbg.png", RGetDevice(), "wallpaper.jpg", false);

	if (bRead)
	{
		if ((m_pLoginBG->GetWidth() > 1024) || (m_pLoginBG->GetHeight() > 768))
			bRead = false;
	}

	if (!bRead)
		bRead = m_pLoginBG->Create("loginbg.png", RGetDevice(), "interface/default/loginbg.jpg");

	if (bRead && m_pLoginBG)
	{
		MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget("Login_BackgrdImg");
		if (pPicture)
			pPicture->SetBitmap(m_pLoginBG->GetSourceBitmap());
	}

	if (m_pLoginPanel != NULL)
	{
		delete m_pLoginPanel;
		m_pLoginPanel = NULL;
	}

	m_pLoginPanel = new MBitmapR2;
	((MBitmapR2*)m_pLoginPanel)->Create("loginpanel.png", RGetDevice(), "interface/default/loginpanel.tga");
	if (m_pLoginPanel)
	{
		MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget("Login_Panel");
		if (pPicture)
			pPicture->SetBitmap(m_pLoginPanel->GetSourceBitmap());
	}

	MButton* pLoginOk = (MButton*)m_IDLResource.FindWidget("LoginOK");
	if (pLoginOk)
		pLoginOk->Enable(true);

	MWidget* pLoginFrame = m_IDLResource.FindWidget("LoginFrame");
	MWidget* pLoginBG = m_IDLResource.FindWidget("Login_BackgrdImg");
	if (pLoginFrame)
	{
		if (pLoginBG)
			pLoginFrame->Show(false);
		else
			pLoginFrame->Show(true);
	}

	pLoginFrame = m_IDLResource.FindWidget("Login_ConnectingMsg");
	if (pLoginFrame)
		pLoginFrame->Show(false);

	MLabel* pErrorLabel = (MLabel*)m_IDLResource.FindWidget("LoginError");
	if (pErrorLabel)
		pErrorLabel->SetText("");

	MLabel* pPasswd = (MLabel*)m_IDLResource.FindWidget("LoginPassword");
	if (pPasswd)
		pPasswd->SetText("");

	HideAllWidgets();

	ShowWidget("Login", true);

	ZServerView* pServerList = (ZServerView*)m_IDLResource.FindWidget("SelectedServer");
	if (pServerList)
	{
		pServerList->ClearServerList();
		if (strcmp(Z_LOCALE_DEFAULT_FONT, "Arial") == 0)
			pServerList->SetTextOffset(-2);

		if (ZApplication::GetInstance()->IsLaunchTest())
		{
			if (m_pTLocatorList && (m_pTLocatorList->GetSize() > 0))
				m_nLocServ = rand() % m_pTLocatorList->GetSize();
		}
		else
		{
			if (m_pLocatorList && (m_pLocatorList->GetSize() > 0))
				m_nLocServ = rand() % m_pLocatorList->GetSize();
		}

		RequestServerStatusListInfo();
	}

	MWidget* pWidget = m_IDLResource.FindWidget("LoginID");
	if (pWidget)
	{
		char buffer[256];
		if (ZGetApplication()->GetSystemValue("LoginID", buffer))
			pWidget->SetText(buffer);
	}

	pWidget = m_IDLResource.FindWidget("ServerAddress");
	if (pWidget)
	{
		pWidget->SetText(ZGetConfiguration()->GetServerIP());
	}
	pWidget = m_IDLResource.FindWidget("ServerPort");
	if (pWidget)
	{
		char szText[25];
		sprintf(szText, "%d", ZGetConfiguration()->GetServerPort());
		pWidget->SetText(szText);
	}

	if (m_pBackground)
	{
		m_pBackground->LoadMesh();
		m_pBackground->SetScene(LOGIN_SCENE_FIXEDSKY);
	}

	ZApplication::GetSoundEngine()->StopMusic();
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_INTRO, ZApplication::GetFileSystem());

	if (m_spGameClient->IsConnected())
	{
		ZPostDisconnect();
	}

	ShowWidget("Login_Logo", false);
	ShowWidget("Label_ID", true);
	ShowWidget("LoginID", true);
	ShowWidget("Label_Password", true);
	ShowWidget("LoginPassword", true);
}

void ZGameInterface::OnLoginDestroy(void)
{
	ShowWidget("Login", false);

	MWidget* pWidget = m_IDLResource.FindWidget("LoginID");
	if (pWidget)
	{
		ZGetApplication()->SetSystemValue("LoginID", pWidget->GetText());

		if (m_pBackground)
			m_pBackground->SetScene(LOGIN_SCENE_FALLDOWN);
	}

	if (m_pLoginBG != NULL)
	{
		MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget("Login_BackgrdImg");
		if (pPicture)
			pPicture->SetBitmap(NULL);

		delete m_pLoginBG;
		m_pLoginBG = NULL;
	}

	if (m_pLoginPanel != NULL)
	{
		MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget("Login_Panel");
		if (pPicture)
			pPicture->SetBitmap(NULL);

		delete m_pLoginPanel;
		m_pLoginPanel = NULL;
	}

	ZGetShop()->Destroy();
}

void ZGameInterface::OnDirectLoginCreate(void)
{
#ifdef LOCALE_KOREA

#endif
}

void ZGameInterface::OnDirectLoginDestroy(void)
{
#ifdef LOCALE_KOREA

#endif
}

void ZGameInterface::OnGameOnLoginCreate(void)
{
	if (m_pBackground)
	{
		m_pBackground->LoadMesh();
		m_pBackground->SetScene(LOGIN_SCENE_FIXEDSKY);
	}

	ZApplication::GetSoundEngine()->StopMusic();
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_INTRO, ZApplication::GetFileSystem());

	if (m_spGameClient->IsConnected())
	{
		ZPostDisconnect();
	}

	HideAllWidgets();
	ShowWidget("NetmarbleLogin", true);

	ZBaseAuthInfo* pAuthInfo = ZGetLocale()->GetAuthInfo();
	if (pAuthInfo)
		ZPostConnect(pAuthInfo->GetServerIP(), pAuthInfo->GetServerPort());
}

void ZGameInterface::OnGameOnLoginDestroy(void)
{
	if (m_pBackground)
		m_pBackground->SetScene(LOGIN_SCENE_FALLDOWN);
}

void ZGameInterface::OnLobbyCreate(void)
{
	if (m_bOnEndOfReplay)
	{
		m_bOnEndOfReplay = false;
		ZGetMyInfo()->SetLevelPercent(m_nLevelPercentCache);
	}

	if (m_bOnEndOfReplay)
	{
		m_bOnEndOfReplay = false;
		ZGetMyInfo()->SetLevelPercent(m_nLevelPercentCache);
	}

	if (m_pBackground != 0)
		m_pBackground->Free();

	if (m_spGameClient)
	{
		m_spGameClient->ClearPeers();
		m_spGameClient->ClearStageSetting();
	}

	SetRoomNoLight(1);
	ZGetGameClient()->RequestOnLobbyCreated();

	ShowWidget("CombatMenuFrame", false);
	ShowWidget("Lobby", true);
	EnableLobbyInterface(true);

	MWidget* pWidget = m_IDLResource.FindWidget("StageName");
	if (pWidget) {
		char buffer[256];
		if (ZGetApplication()->GetSystemValue("StageName", buffer))
			pWidget->SetText(buffer);
	}

	ZRoomListBox* pRoomList = (ZRoomListBox*)m_IDLResource.FindWidget("Lobby_StageList");
	if (pRoomList) pRoomList->Clear();

	ShowWidget("Lobby_StageList", true);
	MPicture* pPicture = 0;
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_StripBottom");
	if (pPicture != NULL)	pPicture->SetAnimation(0, 1000.0f);
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_StripTop");
	if (pPicture != NULL)	pPicture->SetAnimation(1, 1000.0f);

	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_RoomListBG");
	if (pPicture)
	{
		m_pRoomListFrame = new MBitmapR2;
		((MBitmapR2*)m_pRoomListFrame)->Create("gamelist_panel.png", RGetDevice(), "interface/default/gamelist_panel.png");

		if (m_pRoomListFrame != NULL)
			pPicture->SetBitmap(m_pRoomListFrame->GetSourceBitmap());

		m_pDuelTournamentLobbyFrame = new MBitmapR2;
		((MBitmapR2*)m_pDuelTournamentLobbyFrame)->Create("dueltournament_lobby_panel.png", RGetDevice(), "interface/default/dueltournament_lobby_panel.png");
	}
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_BottomBG");
	if (pPicture)
	{
		m_pBottomFrame = new MBitmapR2;
		((MBitmapR2*)m_pBottomFrame)->Create("bottom_panel.png", RGetDevice(), "interface/default/bottom_panel.png");

		if (m_pBottomFrame != NULL)
			pPicture->SetBitmap(m_pBottomFrame->GetSourceBitmap());
	}

	m_pClanInfoBg = new MBitmapR2;
	((MBitmapR2*)m_pClanInfoBg)->Create("claninfo_panel.tga", RGetDevice(), "interface/default/claninfo_panel.tga");
	if (m_pClanInfoBg != NULL)
	{
		pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_ClanInfoBG");
		if (pPicture)	pPicture->SetBitmap(m_pClanInfoBg->GetSourceBitmap());
	}

	m_pDuelTournamentInfoBg = new MBitmapR2;
	((MBitmapR2*)m_pDuelTournamentInfoBg)->Create("dueltournamentinfo_panel.tga", RGetDevice(), "interface/default/dueltournamentinfo_panel.tga");
	if (m_pDuelTournamentInfoBg != NULL)
	{
		pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_DuelTournamentInfoBG");
		if (pPicture)	pPicture->SetBitmap(m_pDuelTournamentInfoBg->GetSourceBitmap());
	}

	m_pDuelTournamentRankingLabel = new MBitmapR2;
	((MBitmapR2*)m_pDuelTournamentRankingLabel)->Create("DuelTournamentRankingLabel.png", RGetDevice(), "interface/default/DuelTournamentRankingLabel.png");
	if (m_pDuelTournamentRankingLabel != NULL)
	{
		pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_DuelTournamentRankingListLabel");
		if (pPicture)	pPicture->SetBitmap(m_pDuelTournamentRankingLabel->GetSourceBitmap());
	}

	ZDuelTournamentRankingListBox* pDTRankingListBox = (ZDuelTournamentRankingListBox*)m_IDLResource.FindWidget("Lobby_DuelTournamentRankingList");
	if (pDTRankingListBox) pDTRankingListBox->LoadInterfaceImgs();

#ifdef _BIRDSOUND
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_LOBBY);
	ZApplication::GetSoundEngine()->PlayMusic(true);
#else
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_LOBBY, ZApplication::GetFileSystem());
	ZApplication::GetSoundEngine()->PlayMusic(true);
#endif

	ZPlayerListBox* pPlayerListBox = (ZPlayerListBox*)m_IDLResource.FindWidget("LobbyChannelPlayerList");
	if (pPlayerListBox)
		pPlayerListBox->SetMode(ZPlayerListBox::PLAYERLISTMODE_CHANNEL);

	pWidget = m_IDLResource.FindWidget("ChannelChattingInput");
	if (pWidget) pWidget->SetFocus();

	if (m_pBackground)
		m_pBackground->SetScene(LOGIN_SCENE_FIXEDCHAR);

	bool bEnable = ZGetGameClient()->GetEnableInterface();

	pWidget = m_IDLResource.FindWidget("StageJoin");
	if (pWidget)		pWidget->Enable(bEnable);

	pWidget = m_IDLResource.FindWidget("StageCreateFrameCaller");
	if (pWidget)		pWidget->Enable(bEnable);

	pWidget = m_IDLResource.FindWidget("QuickJoin");
	if (pWidget)		pWidget->Enable(bEnable);

	pWidget = m_IDLResource.FindWidget("QuickJoin2");
	if (pWidget)		pWidget->Enable(bEnable);

	pWidget = m_IDLResource.FindWidget("ArrangedTeamGame");
	if (pWidget)		pWidget->Enable(bEnable);

	pWidget = m_IDLResource.FindWidget("ChannelChattingInput");
	if (pWidget)		pWidget->Enable(bEnable);

	pWidget = m_IDLResource.FindWidget("Lobby_StageList");
	if (pWidget)		pWidget->Enable(bEnable);

	pWidget = m_IDLResource.FindWidget("RequestLadderRejoin");
	if (pWidget)		pWidget->Enable(bEnable);

	pWidget = m_IDLResource.FindWidget("PlayerWarsMapVoteWidget");
	if (pWidget)		pWidget->Enable(bEnable);

	InitLobbyUIByChannelType();
}

void ZGameInterface::InitLobbyUIByChannelType()
{
	bool bClanBattleUI = ((ZGetGameClient()->GetServerMode() == MSM_CLAN) && (ZGetGameClient()->GetChannelType() == MCHANNEL_TYPE_CLAN));
	bool bDuelTournamentUI = (ZGetGameClient()->GetChannelType() == MCHANNEL_TYPE_DUELTOURNAMENT);

	if (bClanBattleUI)
	{
		ZGetGameInterface()->InitDuelTournamentLobbyUI(false);
		ZGetGameInterface()->InitClanLobbyUI(true);
	}
	else if (bDuelTournamentUI)
	{
		ZGetGameInterface()->InitClanLobbyUI(false);
		ZGetGameInterface()->InitDuelTournamentLobbyUI(true);

		ZPostDuelTournamentRequestSideRankingInfo(ZGetMyUID());
	}
	else
	{
		ZGetGameInterface()->InitClanLobbyUI(false);
		ZGetGameInterface()->InitDuelTournamentLobbyUI(false);
	}

	if (bDuelTournamentUI)
		ZPostDuelTournamentRequestSideRankingInfo(ZGetMyUID());
}

void ZGameInterface::OnLobbyDestroy(void)
{
	ShowWidget("Lobby", false);

	MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_RoomListBG");
	if (pPicture)
		pPicture->SetBitmap(NULL);

	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_BottomBG");
	if (pPicture)
		pPicture->SetBitmap(NULL);

	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_ClanInfoBG");
	if (pPicture)
		pPicture->SetBitmap(NULL);

	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_DuelTournamentInfoBG");
	if (pPicture)
		pPicture->SetBitmap(NULL);

	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_DuelTournamentRankingListLabel");
	if (pPicture)
		pPicture->SetBitmap(NULL);

	SAFE_DELETE(m_pRoomListFrame);
	SAFE_DELETE(m_pDuelTournamentLobbyFrame);
	SAFE_DELETE(m_pBottomFrame);
	SAFE_DELETE(m_pClanInfoBg);
	SAFE_DELETE(m_pDuelTournamentInfoBg);
	SAFE_DELETE(m_pDuelTournamentRankingLabel);

	ZDuelTournamentRankingListBox* pDTRankingListBox = (ZDuelTournamentRankingListBox*)m_IDLResource.FindWidget("Lobby_DuelTournamentRankingList");
	if (pDTRankingListBox) {
		pDTRankingListBox->UnloadInterfaceImgs();
		pDTRankingListBox->SetVisible(false);
	}

	MWidget* pWidget = m_IDLResource.FindWidget("StageName");
	if (pWidget) ZGetApplication()->SetSystemValue("StageName", pWidget->GetText());
}

void ZGameInterface::OnStageCreate(void)
{
	mlog("StageCreated\n");

	if (m_spGameClient)
	{
		m_spGameClient->ClearPeers();
	}

	ShowWidget("Shop", false);
	ShowWidget("Equipment", false);
	ShowWidget("Stage", true);
	EnableStageInterface(true);
	MButton* pObserverBtn = (MButton*)m_IDLResource.FindWidget("StageObserverBtn");
	if (pObserverBtn)
		pObserverBtn->SetCheck(false);

	ZCharacterView* pCharView = (ZCharacterView*)m_IDLResource.FindWidget("Stage_Charviewer");

	if (pCharView != NULL)
	{
		pCharView->SetCharacter(ZGetMyUID());
	}

	MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_StripBottom");
	if (pPicture != NULL)	pPicture->SetBitmapColor(0xFFFFFFFF);
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_StripTop");
	if (pPicture != NULL)	pPicture->SetBitmapColor(0xFFFFFFFF);

	ZPostRequestStageSetting(ZGetGameClient()->GetStageUID());
	SerializeStageInterface();

#ifdef _BIRDSOUND
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_LOBBY);
	ZApplication::GetSoundEngine()->PlayMusic(true);
#else
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_LOBBY, ZApplication::GetFileSystem());
	ZApplication::GetSoundEngine()->PlayMusic(true);
#endif
	ZApplication::GetStageInterface()->OnCreate();
}

void ZGameInterface::OnStageDestroy(void)
{
	ZApplication::GetStageInterface()->OnDestroy();
}

char* GetItemSlotName(const char* szName, int nItem)
{
	static char szTemp[256];
	strcpy(szTemp, szName);

	switch (nItem)
	{
	case MMCIP_HEAD:		strcat(szTemp, "_EquipmentSlot_Head");			break;
	case MMCIP_CHEST:		strcat(szTemp, "_EquipmentSlot_Chest");		break;
	case MMCIP_HANDS:		strcat(szTemp, "_EquipmentSlot_Hands");		break;
	case MMCIP_LEGS:		strcat(szTemp, "_EquipmentSlot_Legs");			break;
	case MMCIP_FEET:		strcat(szTemp, "_EquipmentSlot_Feet");			break;
	case MMCIP_FINGERL:	strcat(szTemp, "_EquipmentSlot_FingerL");		break;
	case MMCIP_FINGERR:	strcat(szTemp, "_EquipmentSlot_FingerR");		break;
	case MMCIP_AVATAR:		strcat(szTemp, "_EquipmentSlot_Avatar");		break;
	case MMCIP_MELEE:		strcat(szTemp, "_EquipmentSlot_Melee");		break;
	case MMCIP_PRIMARY:	strcat(szTemp, "_EquipmentSlot_Primary");		break;
	case MMCIP_SECONDARY:	strcat(szTemp, "_EquipmentSlot_Secondary");	break;
	case MMCIP_CUSTOM1:	strcat(szTemp, "_EquipmentSlot_Custom1");		break;
	case MMCIP_CUSTOM2:	strcat(szTemp, "_EquipmentSlot_Custom2");		break;
	case MMCIP_COMMUNITY1:	strcat(szTemp, "_EquipmentSlot_Community1");	break;
	case MMCIP_COMMUNITY2:	strcat(szTemp, "_EquipmentSlot_Community2");	break;
	case MMCIP_LONGBUFF1:	strcat(szTemp, "_EquipmentSlot_LongBuff1");	break;
	case MMCIP_LONGBUFF2:	strcat(szTemp, "_EquipmentSlot_LongBuff2");	break;

	default: break;
	}

	return szTemp;
}

bool ZGameInterface::OnCreate(ZLoadingProgress* pLoadingProgress)
{
	if (!m_sbRemainClientConnectionForResetApp)
		m_spGameClient = new ZGameClient();

	if (!m_Tips.Initialize(ZApplication::GetFileSystem(), ZGetLocale()->GetLanguage())) {
		mlog("Check tips.xml\n");
		return false;
	}

	auto ZGameInterfaceInitInterface = MBeginProfile("ZGameInterface::InitInterface");
	ZLoadingProgress interfaceProgress("Interface", pLoadingProgress, .7f);
	if (!InitInterface(ZGetConfiguration()->GetInterfaceSkinName(), &interfaceProgress))
	{
		mlog("ZGameInterface::OnCreate: Failed InitInterface\n");
		return false;
	}
	MEndProfile(ZGameInterfaceInitInterface);

	interfaceProgress.UpdateAndDraw(1.f);

	auto ZScreenEffectManagerCreate = MBeginProfile("ZGameInterface - ZScreenEffectManager::Create");
	m_pScreenEffectManager = new ZScreenEffectManager;
	if (!m_pScreenEffectManager->Create())
		return false;
	MEndProfile(ZScreenEffectManagerCreate);

	auto ZEffectManagerCreate = MBeginProfile("ZGameInterface - ZEffectManager::Create");
	m_pEffectManager = new ZEffectManager;
	if (!m_pEffectManager->Create())
		return false;
	MEndProfile(ZEffectManagerCreate);

	auto ZGameClientCreate = MBeginProfile("ZGameInterface - ZGameClient::Create");

	SetTeenVersion(ZGetLocale()->IsTeenMode());

	if (!m_sbRemainClientConnectionForResetApp)
	{
		int nNetworkPort = RandomNumber(ZGetConfiguration()->GetEtc()->nNetworkPort1, ZGetConfiguration()->GetEtc()->nNetworkPort2);
		if (m_spGameClient->Create(nNetworkPort) == false) {
			string strMsg = "Unknown Network Error";
			if (GetLastError() == WSAEADDRINUSE)
				NotifyMessage(MATCHNOTIFY_NETWORK_PORTINUSE, &strMsg);
			else
				NotifyMessage(MATCHNOTIFY_NETWORK_CREATE_FAILED, &strMsg);
			ShowMessage(strMsg.c_str());
		}
		m_spGameClient->SetOnCommandCallback(OnCommand);
		m_spGameClient->CreateUPnP(nNetworkPort);
		m_MyPort = nNetworkPort;

		mlog("Client create success.\n");
	}

	MEndProfile(ZGameClientCreate);

	auto Etc = MBeginProfile("ZGameInterface - Etc.");

	ZItemSlotView* itemSlot;
	for (int i = 0; i < MMCIP_END; i++)
	{
		itemSlot = (ZItemSlotView*)m_IDLResource.FindWidget(GetItemSlotName("Shop", i));
		if (itemSlot)
		{
			strcpy(itemSlot->m_szItemSlotPlace, "Shop");

			switch (itemSlot->GetParts())
			{
			case MMCIP_HEAD:		itemSlot->SetText(ZMsg(MSG_WORD_HEAD));				break;
			case MMCIP_CHEST:		itemSlot->SetText(ZMsg(MSG_WORD_CHEST));				break;
			case MMCIP_HANDS:		itemSlot->SetText(ZMsg(MSG_WORD_HANDS));				break;
			case MMCIP_LEGS:		itemSlot->SetText(ZMsg(MSG_WORD_LEGS));				break;
			case MMCIP_FEET:		itemSlot->SetText(ZMsg(MSG_WORD_FEET));				break;
			case MMCIP_FINGERL:		itemSlot->SetText(ZMsg(MSG_WORD_LFINGER));			break;
			case MMCIP_FINGERR:		itemSlot->SetText(ZMsg(MSG_WORD_RFINGER));			break;
			case MMCIP_AVATAR:		itemSlot->SetText(ZMsg(MSG_WORD_AVATAR));				break;
			case MMCIP_MELEE:		itemSlot->SetText(ZMsg(MSG_WORD_MELEE));				break;
			case MMCIP_PRIMARY:		itemSlot->SetText(ZMsg(MSG_WORD_WEAPON1));			break;
			case MMCIP_SECONDARY:	itemSlot->SetText(ZMsg(MSG_WORD_WEAPON2));			break;
			case MMCIP_CUSTOM1:		itemSlot->SetText(ZMsg(MSG_WORD_ITEM1));				break;
			case MMCIP_CUSTOM2:		itemSlot->SetText(ZMsg(MSG_WORD_ITEM2));				break;
			case MMCIP_COMMUNITY1:	itemSlot->SetText(ZMsg(MSG_WORD_COMMUNITY_ITEM1));	break;
			case MMCIP_COMMUNITY2:	itemSlot->SetText(ZMsg(MSG_WORD_COMMUNITY_ITEM2));	break;
			case MMCIP_LONGBUFF1:	itemSlot->SetText(ZMsg(MSG_WORD_LONGBUFF_ITEM1));		break;
			case MMCIP_LONGBUFF2:	itemSlot->SetText(ZMsg(MSG_WORD_LONGBUFF_ITEM2));		break;
			default:				itemSlot->SetText("");									break;
			}
		}
	}
	for (int i = 0; i < MMCIP_END; i++)
	{
		itemSlot = (ZItemSlotView*)m_IDLResource.FindWidget(GetItemSlotName("Equip", i));
		if (itemSlot)
		{
			strcpy(itemSlot->m_szItemSlotPlace, "Equip");

			switch (itemSlot->GetParts())
			{
			case MMCIP_HEAD:		itemSlot->SetText(ZMsg(MSG_WORD_HEAD));				break;
			case MMCIP_CHEST:		itemSlot->SetText(ZMsg(MSG_WORD_CHEST));				break;
			case MMCIP_HANDS:		itemSlot->SetText(ZMsg(MSG_WORD_HANDS));				break;
			case MMCIP_LEGS:		itemSlot->SetText(ZMsg(MSG_WORD_LEGS));				break;
			case MMCIP_FEET:		itemSlot->SetText(ZMsg(MSG_WORD_FEET));				break;
			case MMCIP_FINGERL:		itemSlot->SetText(ZMsg(MSG_WORD_LFINGER));			break;
			case MMCIP_FINGERR:		itemSlot->SetText(ZMsg(MSG_WORD_RFINGER));			break;
			case MMCIP_AVATAR:		itemSlot->SetText(ZMsg(MSG_WORD_AVATAR));				break;
			case MMCIP_MELEE:		itemSlot->SetText(ZMsg(MSG_WORD_MELEE));				break;
			case MMCIP_PRIMARY:		itemSlot->SetText(ZMsg(MSG_WORD_WEAPON1));			break;
			case MMCIP_SECONDARY:	itemSlot->SetText(ZMsg(MSG_WORD_WEAPON2));			break;
			case MMCIP_CUSTOM1:		itemSlot->SetText(ZMsg(MSG_WORD_ITEM1));				break;
			case MMCIP_CUSTOM2:		itemSlot->SetText(ZMsg(MSG_WORD_ITEM2));				break;
			case MMCIP_COMMUNITY1:	itemSlot->SetText(ZMsg(MSG_WORD_COMMUNITY_ITEM1));	break;
			case MMCIP_COMMUNITY2:	itemSlot->SetText(ZMsg(MSG_WORD_COMMUNITY_ITEM2));	break;
			case MMCIP_LONGBUFF1:	itemSlot->SetText(ZMsg(MSG_WORD_LONGBUFF_ITEM1));		break;
			case MMCIP_LONGBUFF2:	itemSlot->SetText(ZMsg(MSG_WORD_LONGBUFF_ITEM2));		break;
			default:				itemSlot->SetText("");									break;
			}
		}
	}
	ZShopEquipListbox* pItemListBox = (ZShopEquipListbox*)m_IDLResource.FindWidget("EquipmentList");
	if (pItemListBox)
	{
		pItemListBox->EnableDragAndDrop(true);
	}

	MListBox* pReplayBox = (MListBox*)m_IDLResource.FindWidget("Replay_FileList");
	if (pReplayBox)
	{
		pReplayBox->m_FontAlign = MAM_VCENTER;
		pReplayBox->SetVisibleHeader(false);
		pReplayBox->AddField("NAME", 92);
		pReplayBox->AddField("VERSION", 8);
		pReplayBox->m_bAbsoulteTabSpacing = false;
	}

	if (Z_ETC_BOOST && RIsActive())
		m_spGameClient->PriorityBoost(true);

	m_spGameClient->SetRejectNormalChat(Z_ETC_REJECT_NORMALCHAT);
	m_spGameClient->SetRejectTeamChat(Z_ETC_REJECT_TEAMCHAT);
	m_spGameClient->SetRejectClanChat(Z_ETC_REJECT_CLANCHAT);
	m_spGameClient->SetRejectWhisper(Z_ETC_REJECT_WHISPER);
	m_spGameClient->SetRejectInvite(Z_ETC_REJECT_INVITE);

	int nLineHeightTextArea = int(18 / 600.f * MGetWorkspaceHeight());
	MTextArea* pTextArea = (MTextArea*)m_IDLResource.FindWidget("CombatResult_PlayerNameList");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetCustomLineHeight(nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("CombatResult_PlayerKillList");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetCustomLineHeight(nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("CombatResult_PlayerDeathList");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetCustomLineHeight(nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("ClanResult_PlayerNameList1");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetCustomLineHeight(nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("ClanResult_PlayerKillList1");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetCustomLineHeight(nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("ClanResult_PlayerDeathList1");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetCustomLineHeight(nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("ClanResult_PlayerNameList2");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetCustomLineHeight(nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("ClanResult_PlayerKillList2");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetCustomLineHeight(nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("ClanResult_PlayerDeathList2");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetCustomLineHeight(nLineHeightTextArea);
	}
	MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget("CombatResult_Header");
	if (pPicture)
		pPicture->SetOpacity(80);
	pPicture = (MPicture*)m_IDLResource.FindWidget("ClanResult_Header1");
	if (pPicture)
		pPicture->SetOpacity(80);
	pPicture = (MPicture*)m_IDLResource.FindWidget("ClanResult_Header2");
	if (pPicture)
		pPicture->SetOpacity(80);

	{
		MComboBox* pCombo = (MComboBox*)m_IDLResource.FindWidget("Stage_RelayMapType");
		if (pCombo)
		{
			pCombo->RemoveAll();
			pCombo->Add(ZMsg(MSG_GAME_RELAYMAP_TURN));
			pCombo->Add(ZMsg(MSG_GAME_RELAYMAP_RANDOM));
			pCombo->SetSelIndex(0);
		}
		pCombo = (MComboBox*)m_IDLResource.FindWidget("Stage_RelayMapRepeatCount");
		if (pCombo)
		{
			pCombo->RemoveAll();
			pCombo->Add(ZMsg(MSG_GAME_RELAYMAP_1ROUND));
			pCombo->Add(ZMsg(MSG_GAME_RELAYMAP_2ROUND));
			pCombo->Add(ZMsg(MSG_GAME_RELAYMAP_3ROUND));
			pCombo->Add(ZMsg(MSG_GAME_RELAYMAP_4ROUND));
			pCombo->Add(ZMsg(MSG_GAME_RELAYMAP_5ROUND));
			pCombo->SetSelIndex(2);
		}

		MListBox* pMapListBox = (MListBox*)m_IDLResource.FindWidget("Stage_MapListbox");
		if (pMapListBox)
		{
			pMapListBox->m_FontAlign = MAM_LEFT;
			pMapListBox->AddField("ICON", 23);
			pMapListBox->AddField("NAME", 170);
			pMapListBox->SetItemHeight(23);
			pMapListBox->SetVisibleHeader(false);
			pMapListBox->m_bNullFrame = true;
		}
		MListBox* pRelayMapListBox = (MListBox*)m_IDLResource.FindWidget("Stage_RelayMapListbox");
		if (pRelayMapListBox)
		{
			pRelayMapListBox->m_FontAlign = MAM_LEFT;
			pRelayMapListBox->AddField("ICON", 23);
			pRelayMapListBox->AddField("NAME", 170);
			pRelayMapListBox->SetItemHeight(23);
			pRelayMapListBox->SetVisibleHeader(false);
			pRelayMapListBox->m_bNullFrame = true;
		}
	}

	MListBox* pListBox = (MListBox*)m_IDLResource.FindWidget("Stage_SacrificeItemListbox");
	if (pListBox)
	{
		pListBox->m_FontAlign = MAM_VCENTER;
		pListBox->AddField("ICON", 32);
		pListBox->AddField("NAME", 170);
		pListBox->SetItemHeight(32);
		pListBox->SetVisibleHeader(false);
		pListBox->m_bNullFrame = true;
		pListBox->EnableDragAndDrop(true);
		pListBox->SetOnDropCallback(OnDropCallbackRemoveSacrificeItem);
	}

	itemSlot = (ZItemSlotView*)m_IDLResource.FindWidget("Stage_SacrificeItemButton0");
	if (itemSlot)
	{
		itemSlot->EnableDragAndDrop(true);
		strcpy(itemSlot->m_szItemSlotPlace, "SACRIFICE0");
	}
	itemSlot = (ZItemSlotView*)m_IDLResource.FindWidget("Stage_SacrificeItemButton1");
	if (itemSlot)
	{
		itemSlot->EnableDragAndDrop(true);
		strcpy(itemSlot->m_szItemSlotPlace, "SACRIFICE1");
	}

	pListBox = (MListBox*)m_IDLResource.FindWidget("QuestResult_ItemListbox");
	if (pListBox)
	{
		pListBox->m_FontAlign = MAM_VCENTER;
		pListBox->AddField("ICON", 35);
		pListBox->AddField("NAME", 300);
		pListBox->SetItemHeight(32);
		pListBox->SetVisibleHeader(false);
		pListBox->SetFont(MFontManager::Get("FONTa10b"));
		pListBox->m_FontColor = MCOLOR(0xFFFFF794);
		pListBox->m_bNullFrame = true;
	}

	pTextArea = (MTextArea*)m_IDLResource.FindWidget("SurvivalResult_MyRank");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetCustomLineHeight(nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("SurvivalResult_MyName");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetCustomLineHeight(nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("SurvivalResult_MyRankPoint");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetCustomLineHeight(nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("SurvivalResult_PlayerRankList");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetCustomLineHeight(nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("SurvivalResult_PlayerNameList");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetCustomLineHeight(nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("SurvivalResult_PlayerRankPointList");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetCustomLineHeight(nLineHeightTextArea);
	}

	int nMargin[BMNUM_NUMOFCHARSET] = { 15,15,15,15,15,15,15,15,15,15,8,10,8 };
	int nMarginNarrow[BMNUM_NUMOFCHARSET] = { 12,12,12,12,12,12,12,12,12,12,8,10,8 };
	ZBmNumLabel* pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("Lobby_ClanInfoWinLose");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("Lobby_ClanInfoPoints");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("Lobby_ClanInfoTotalPoints");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("Lobby_ClanInfoRanking");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("Lobby_DuelTournamentInfoWinLose");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMarginNarrow);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("Lobby_DuelTournamentInfoPoints");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMarginNarrow);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("Lobby_DuelTournamentInfoWinners");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMarginNarrow);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("Lobby_DuelTournamentInfoRanking");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMarginNarrow);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("Lobby_DuelTournamentInfoWinLosePrev");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMarginNarrow);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("Lobby_DuelTournamentInfoPointsPrev");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMarginNarrow);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("Lobby_DuelTournamentInfoWinnersPrev");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMarginNarrow);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("Lobby_DuelTournamentInfoRankingPrev");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMarginNarrow);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("QuestResult_GetPlusXP");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("QuestResult_GetMinusXP");
	if (pBmNumLabel)
	{
		pBmNumLabel->SetCharMargin(nMargin);
		pBmNumLabel->SetIndexOffset(16);
	}
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("QuestResult_GetTotalXP");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("QuestResult_GetBounty");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("SurvivalResult_GetReachedRound");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("SurvivalResult_GetPoint");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("SurvivalResult_GetXP");
	if (pBmNumLabel)
	{
		pBmNumLabel->SetCharMargin(nMargin);
		pBmNumLabel->SetIndexOffset(16);
	}
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("SurvivalResult_GetBounty");
	if (pBmNumLabel)
	{
		pBmNumLabel->SetCharMargin(nMargin);
		pBmNumLabel->SetIndexOffset(16);
	}

	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_DEATHMATCH_SOLO, ZMsg(MSG_MT_DEATHMATCH_SOLO));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_DEATHMATCH_TEAM, ZMsg(MSG_MT_DEATHMATCH_TEAM));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_GLADIATOR_SOLO, ZMsg(MSG_MT_GLADIATOR_SOLO));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_GLADIATOR_TEAM, ZMsg(MSG_MT_GLADIATOR_TEAM));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_ASSASSINATE, ZMsg(MSG_MT_ASSASSINATE));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_TRAINING, ZMsg(MSG_MT_TRAINING));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_SURVIVAL, ZMsg(MSG_MT_SURVIVAL));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_QUEST, ZMsg(MSG_MT_QUEST));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_BERSERKER, ZMsg(MSG_MT_BERSERKER));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_DEATHMATCH_TEAM2, ZMsg(MSG_MT_DEATHMATCH_TEAM2));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_DUEL, ZMsg(MSG_MT_DUEL));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_DUELTOURNAMENT, ZMsg(MSG_MT_DUELTOURNAMENT));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_CTF, ZMsg(MSG_MT_CTF));

#ifndef _DEBUG
	MWidget* pWidget = m_IDLResource.FindWidget("MonsterBookCaller");
	if (pWidget)
		pWidget->Show(false);
#endif

#ifndef _QUEST_ITEM
	MComboBox* pComboBox = (MComboBox*)m_IDLResource.FindWidget("Shop_AllEquipmentFilter");
	if (pComboBox)
		pComboBox->Remove(10);
	pComboBox = (MComboBox*)m_IDLResource.FindWidget("Equip_AllEquipmentFilter");
	if (pComboBox)
		pComboBox->Remove(10);
#endif

	MComboBox* pCombo = (MComboBox*)m_IDLResource.FindWidget("Shop_AllEquipmentFilter");
	if (pCombo)
	{
		pCombo->SetAlignment(MAM_LEFT);
		pCombo->SetListboxAlignment(MAM_LEFT);
	}
	pCombo = (MComboBox*)m_IDLResource.FindWidget("Equip_AllEquipmentFilter");
	if (pCombo)
	{
		pCombo->SetAlignment(MAM_LEFT);
		pCombo->SetListboxAlignment(MAM_LEFT);
	}

	pCombo = (MComboBox*)m_IDLResource.FindWidget("StageType");
	if (pCombo)
		pCombo->SetListboxAlignment(MAM_LEFT);

	ZCharacterView* pCharView = (ZCharacterView*)m_IDLResource.FindWidget("EquipmentInformation");
	if (pCharView)
		pCharView->EnableAutoRotate(true);
	pCharView = (ZCharacterView*)m_IDLResource.FindWidget("EquipmentInformationShop");
	if (pCharView)
		pCharView->EnableAutoRotate(true);

	char szTooltip[256];
	ZTransMsg(szTooltip, MSG_LOBBY_DUELTOURNAMENT_RECORD_TOOPTIP_WINLOSE_PREV);
	SetWidgetToolTipText("Lobby_DuelTournamentInfoWinLosePrev", szTooltip, MAM_RIGHT | MAM_TOP);
	ZTransMsg(szTooltip, MSG_LOBBY_DUELTOURNAMENT_RECORD_TOOPTIP_TP_PREV);
	SetWidgetToolTipText("Lobby_DuelTournamentInfoPointsPrev", szTooltip, MAM_RIGHT | MAM_TOP);
	ZTransMsg(szTooltip, MSG_LOBBY_DUELTOURNAMENT_RECORD_TOOPTIP_WINNER_PREV);
	SetWidgetToolTipText("Lobby_DuelTournamentInfoWinnersPrev", szTooltip, MAM_RIGHT | MAM_TOP);
	ZTransMsg(szTooltip, MSG_LOBBY_DUELTOURNAMENT_RECORD_TOOPTIP_RANK_PREV);
	SetWidgetToolTipText("Lobby_DuelTournamentInfoRankingPrev", szTooltip, MAM_RIGHT | MAM_TOP);
	ZTransMsg(szTooltip, MSG_LOBBY_DUELTOURNAMENT_RECORD_TOOPTIP_WINLOSE);
	SetWidgetToolTipText("Lobby_DuelTournamentInfoWinLose", szTooltip, MAM_RIGHT | MAM_TOP);
	ZTransMsg(szTooltip, MSG_LOBBY_DUELTOURNAMENT_RECORD_TOOPTIP_TP);
	SetWidgetToolTipText("Lobby_DuelTournamentInfoPoints", szTooltip, MAM_RIGHT | MAM_TOP);
	ZTransMsg(szTooltip, MSG_LOBBY_DUELTOURNAMENT_RECORD_TOOPTIP_WINNER);
	SetWidgetToolTipText("Lobby_DuelTournamentInfoWinners", szTooltip, MAM_RIGHT | MAM_TOP);
	ZTransMsg(szTooltip, MSG_LOBBY_DUELTOURNAMENT_RECORD_TOOPTIP_RANK);
	SetWidgetToolTipText("Lobby_DuelTournamentInfoRanking", szTooltip, MAM_RIGHT | MAM_TOP);
	ZTransMsg(szTooltip, MSG_LOBBY_DUELTOURNAMENT_RECORD_TOOPTIP_SIDERANKLIST);
	SetWidgetToolTipText("Lobby_DuelTournamentRankingList", szTooltip, MAM_RIGHT | MAM_TOP);

	SetCursorEnable(true);

	MEndProfile(Etc);

	mlog("ZGameInterface::OnCreate : done \n");

	return true;
}

void ZGameInterface::OnDestroy()
{
	mlog("ZGameInterface::OnDestroy() : begin \n");

	MTextArea* pTextArea = (MTextArea*)m_IDLResource.FindWidget("Shop_ItemDescription");
	if (pTextArea) pTextArea->ChangeCustomLook(NULL);

	ZCharacterView* pCharView = (ZCharacterView*)m_IDLResource.FindWidget("Stage_Charviewer");
	if (pCharView != 0) pCharView->OnInvalidate();

	SetCursorEnable(false);

	SetState(GUNZ_NA);

	SAFE_DELETE(m_pMapThumbnail);

	if (!m_sbRemainClientConnectionForResetApp)
	{
		m_spGameClient->Destroy();
		mlog("ZGameInterface::OnDestroy() : g_pGameClient->Destroy() \n");

		SAFE_DELETE(m_spGameClient);
		mlog("SAFE_DELETE(g_pGameClient) : \n");
	}

	SAFE_DELETE(m_pLoginBG);

	m_Tips.Finalize();
	FinalInterface();

	mlog("ZGameInterface::OnDestroy() : done() \n");
}

void ZGameInterface::OnShutdownState()
{
	mlog("ZGameInterface::OnShutdown() : begin \n");

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MLabel* pLabel = (MLabel*)pResource->FindWidget("NetmarbleLoginMessage");
	pLabel->SetText(ZErrStr(MERR_CLIENT_DISCONNECTED));
	ZApplication::GetGameInterface()->ShowWidget("NetmarbleLogin", true);

	mlog("ZGameInterface::OnShutdown() : done() \n");
}

bool ZGameInterface::SetState(GunzState nState)
{
	if (m_nState == nState)
		return true;

	if (nState == GUNZ_PREVIOUS)
		nState = m_nPreviousState;

	if (nState == GUNZ_GAME)
	{
		if (ZApplication::GetStageInterface()->IsShowStartMovieOfQuest())
		{
			ZApplication::GetStageInterface()->ChangeStageEnableReady(true);
			MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageReady");
			if (pWidget)
				pWidget->Enable(false);
			ZApplication::GetStageInterface()->StartMovieOfQuest();
			return true;
		}
	}

	m_nPreviousState = m_nState;

	if (m_nState == GUNZ_GAME) OnGameDestroy();
	else if (m_nState == GUNZ_LOGIN)
	{
		MWidget* pWidget = m_IDLResource.FindWidget("Login_BackgrdImg");
		if (!pWidget)
			OnLoginDestroy();
	}
	else if (m_nState == GUNZ_DIRECTLOGIN) OnDirectLoginDestroy();
	else if (m_nState == GUNZ_LOBBY) OnLobbyDestroy();
	else if (m_nState == GUNZ_STAGE) OnStageDestroy();
	else if (m_nState == GUNZ_GREETER) OnGreeterDestroy();
	else if (m_nState == GUNZ_CHARSELECTION)
	{
		OnCharSelectionDestroy();

		if (nState == GUNZ_LOBBY)
		{
			ZGetMyInfo()->GetItemList()->QuestItemClear();
			ZPostRequestGetCharQuestItemInfo(ZGetGameClient()->GetPlayerUID());
		}
	}
	else if (m_nState == GUNZ_CHARCREATION) OnCharCreationDestroy();
#ifdef _BIRDTEST
	else if (m_nState == GUNZ_BIRDTEST) OnBirdTestDestroy();
#endif

	bool bStateChanged = true;
	if (nState == GUNZ_GAME) bStateChanged = OnGameCreate();
	else if (nState == GUNZ_LOGIN) OnLoginCreate();
	else if (nState == GUNZ_DIRECTLOGIN) OnDirectLoginCreate();
	else if (nState == GUNZ_LOBBY)	OnLobbyCreate();
	else if (nState == GUNZ_STAGE) OnStageCreate();
	else if (nState == GUNZ_GREETER) OnGreeterCreate();
	else if (nState == GUNZ_CHARSELECTION)
	{
		if (m_nPreviousState == GUNZ_LOGIN)
		{
			MWidget* pWidget = m_IDLResource.FindWidget("Login_BackgrdImg");
			if (!pWidget)
				OnCharSelectionCreate();
			else
			{
				m_nLoginState = LOGINSTATE_LOGINCOMPLETE;
				m_dwLoginTimer = timeGetTime() + 1000;
				return true;
			}
		}
		else
			OnCharSelectionCreate();
	}
	else if (nState == GUNZ_CHARCREATION) OnCharCreationCreate();
	else if (nState == GUNZ_SHUTDOWN) OnShutdownState();
#ifdef _BIRDTEST
	else if (nState == GUNZ_BIRDTEST) OnBirdTestCreate();
#endif

	if (bStateChanged == false) {
		m_pMsgBox->SetText("Error: Can't Create a Game!");
		m_pMsgBox->Show(true, true);
		SetState(GUNZ_PREVIOUS);
	}
	else {
		m_nState = nState;
	}

	m_nDrawCount = 0;
	return true;
}

_NAMESPACE_REALSPACE2_BEGIN
extern int g_nPoly, g_nCall;
extern int g_nPickCheckPolygon, g_nRealPickCheckPolygon;
_NAMESPACE_REALSPACE2_END

#ifndef _PUBLISH
#include "fmod.h"
#endif

void ZGameInterface::OnDrawStateGame(MDrawContext* pDC)
{
	if (m_pGame != NULL)
	{
		if (!IsMiniMapEnable())
			m_pGame->Draw();

		if (m_bViewUI) {
			if (m_bLeaveBattleReserved)
			{
				int nSeconds = (m_dwLeaveBattleTime - timeGetTime() + 999) / 1000;
				m_pCombatInterface->SetDrawLeaveBattle(m_bLeaveBattleReserved, nSeconds);
			}
			else
				m_pCombatInterface->SetDrawLeaveBattle(false, 0);

			if (GetCamera()->GetLookMode() == ZCAMERA_MINIMAP) {
				m_pMiniMap->OnDraw(pDC);
			}

			m_pCombatInterface->OnDrawCustom(pDC);
		}
	}
}

void ZGameInterface::OnDrawStateLogin(MDrawContext* pDC)
{
	MLabel* pConnectingLabel = (MLabel*)m_IDLResource.FindWidget("Login_ConnectingMsg");
	if (pConnectingLabel)
	{
		char szMsg[128];
		memset(szMsg, 0, sizeof(szMsg));
		int nCount = (timeGetTime() / 800) % 4;
		for (int i = 0; i < nCount; i++)
			szMsg[i] = '<';
		sprintf(szMsg, "%s %s ", szMsg, "Connecting");
		for (int i = 0; i < nCount; i++)
			strcat(szMsg, ">");

		pConnectingLabel->SetText(szMsg);
		pConnectingLabel->SetAlignment(MAM_HCENTER | MAM_VCENTER);
	}

	MWidget* pWidget = m_IDLResource.FindWidget("LoginFrame");
	MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget("Login_BackgrdImg");
	if (!pWidget || !pPicture)
		return;

	ZServerView* pServerList = (ZServerView*)m_IDLResource.FindWidget("SelectedServer");
	MEdit* pPassword = (MEdit*)m_IDLResource.FindWidget("LoginPassword");
	MWidget* pLogin = m_IDLResource.FindWidget("LoginOK");
	if (pServerList && pPassword && pLogin)
	{
		if (pServerList->IsSelected() && (int)strlen(pPassword->GetText()))
			pLogin->Enable(true);
		else
			pLogin->Enable(false);
	}

	DWORD dwCurrTime = timeGetTime();

	if (m_bLoginTimeout && (m_dwLoginTimeout <= dwCurrTime))
	{
		m_bLoginTimeout = false;

		MLabel* pLabel = (MLabel*)m_IDLResource.FindWidget("LoginError");
		if (pLabel)
			pLabel->SetText(ZErrStr(MERR_CLIENT_CONNECT_FAILED));

		MButton* pLoginOK = (MButton*)m_IDLResource.FindWidget("LoginOK");
		if (pLoginOK)
			pLoginOK->Enable(true);

		pWidget->Show(true);

		if (pConnectingLabel)
			pConnectingLabel->Show(false);
	}

	if (m_nLoginState == LOGINSTATE_FADEIN)
	{
		m_bLoginTimeout = false;

		if (dwCurrTime >= m_dwLoginTimer)
		{
			int nOpacity = pPicture->GetOpacity() + 3;
			if (nOpacity > 255)
				nOpacity = 255;

			pPicture->SetOpacity(nOpacity);

			m_dwLoginTimer = dwCurrTime + 9;
		}

		if (pPicture->GetOpacity() == 255)
		{
			m_dwLoginTimer = dwCurrTime + 1000;
			m_nLoginState = LOGINSTATE_SHOWLOGINFRAME;
		}
		else
			pWidget->Show(false);
	}
	else if (m_nLoginState == LOGINSTATE_SHOWLOGINFRAME)
	{
		m_bLoginTimeout = false;

		if (timeGetTime() > m_dwLoginTimer)
		{
			m_nLoginState = LOGINSTATE_STANDBY;
			pWidget->Show(true);
		}
	}
	else if (m_nLoginState == LOGINSTATE_STANDBY)
	{
#ifdef _LOCATOR
		if (timeGetTime() > m_dwRefreshTime)
			RequestServerStatusListInfo();
#endif
	}
	else if (m_nLoginState == LOGINSTATE_LOGINCOMPLETE)
	{
		m_bLoginTimeout = false;

		if (timeGetTime() > m_dwLoginTimer)
			m_nLoginState = LOGINSTATE_FADEOUT;

		if (pConnectingLabel)
			pConnectingLabel->Show(false);
	}
	else if (m_nLoginState == LOGINSTATE_FADEOUT)
	{
		m_bLoginTimeout = false;
		pWidget->Show(false);

		if (dwCurrTime >= m_dwLoginTimer)
		{
			int nOpacity = pPicture->GetOpacity() - 3;
			if (nOpacity < 0)
				nOpacity = 0;

			pPicture->SetOpacity(nOpacity);

			m_dwLoginTimer = dwCurrTime + 9;
		}

		if (pPicture->GetOpacity() == 0)
		{
			OnLoginDestroy();

			m_nLoginState = LOGINSTATE_STANDBY;
			m_nState = GUNZ_CHARSELECTION;
			OnCharSelectionCreate();
		}
	}

	if (IsErrMaxPlayer())
	{
		if (m_dErrMaxPalyerDelayTime <= dwCurrTime)
		{
			ZPostDisconnect();
			SetErrMaxPlayer(false);
			ZApplication::GetGameInterface()->ShowErrorMessage(10003);
		}
	}

#define CHECK_PING_TIME	(1 * 3 * 1000)
	static unsigned long tmAgentPingTestCheckTimer = dwCurrTime;
	if (dwCurrTime - tmAgentPingTestCheckTimer > CHECK_PING_TIME)
	{
		tmAgentPingTestCheckTimer = dwCurrTime;

		if (pServerList)
		{
			SERVERLIST cServerList = pServerList->GetServerList();

			for (SERVERLIST::iterator itr = cServerList.begin(); itr != cServerList.end(); itr++)
			{
				ServerInfo* serverInfo = (*itr);

				MCommand* pCmd = GetGameClient()->CreateCommand(MC_UDP_PING, MUID(0, 0));
				unsigned int nIP = (unsigned int)inet_addr(m_spGameClient->GetUDPInfo()->GetAddress());
				pCmd->AddParameter(new MCmdParamUInt(dwCurrTime));
				GetGameClient()->SendCommandByUDP(pCmd, serverInfo->szAgentIP, 7778);
				delete pCmd;
			}
		}
	}
}

void ZGameInterface::OnDrawStateLobbyNStage(MDrawContext* pDC)
{
	ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();

	DWORD dwClock = timeGetTime();
	if ((dwClock - m_dwFrameMoveClock) < 30)
		return;
	m_dwFrameMoveClock = dwClock;

	if (GetState() == GUNZ_LOBBY)
	{
		char buf[512];
		MLabel* pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerName");
		if (pLabel)
		{
			sprintf(buf, "%s", ZGetMyInfo()->GetCharName());
			pLabel->SetText(buf);
		}
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecClan");
		sprintf(buf, "%s : %s", ZMsg(MSG_CHARINFO_CLAN), ZGetMyInfo()->GetClanName());
		if (pLabel) pLabel->SetText(buf);
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecLevel");
		sprintf(buf, "%s : %d %s", ZMsg(MSG_CHARINFO_LEVEL), ZGetMyInfo()->GetLevel(), ZMsg(MSG_CHARINFO_LEVELMARKER));
		if (pLabel) pLabel->SetText(buf);
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecXP");
		sprintf(buf, "%s : %d%%", ZMsg(MSG_CHARINFO_XP), ZGetMyInfo()->GetLevelPercent());
		if (pLabel) pLabel->SetText(buf);
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecBP");
		sprintf(buf, "%s : %d", ZMsg(MSG_CHARINFO_BOUNTY), ZGetMyInfo()->GetBP());
		if (pLabel) pLabel->SetText(buf);
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecHP");
		sprintf(buf, "%s : %d", ZMsg(MSG_CHARINFO_HP), ZGetMyInfo()->GetHP());
		if (pLabel) pLabel->SetText(buf);
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecAP");
		sprintf(buf, "%s : %d", ZMsg(MSG_CHARINFO_AP), ZGetMyInfo()->GetAP());
		if (pLabel) pLabel->SetText(buf);
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecWT");
		ZMyItemList* pItems = ZGetMyInfo()->GetItemList();
		sprintf(buf, "%s : %d/%d", ZMsg(MSG_CHARINFO_WEIGHT), pItems->GetEquipedTotalWeight(), pItems->GetMaxWeight());
		if (pLabel) pLabel->SetText(buf);

		pLabel = (MLabel*)pRes->FindWidget("Lobby_ChannelName");
		sprintf(buf, "%s > %s > %s", ZGetGameClient()->GetServerName(), ZMsg(MSG_WORD_LOBBY), ZGetGameClient()->GetChannelName());
		if (pLabel)
			pLabel->SetText(buf);
	}

	else if (GetState() == GUNZ_STAGE)
	{
		char buf[512];
		MListBox* pListBox = (MListBox*)pRes->FindWidget("StagePlayerList_");
		bool bShowMe = true;
		if (pListBox)
		{
			ZStagePlayerListItem* pItem = NULL;
			if (pListBox->GetSelIndex() < pListBox->GetCount())
			{
				if (pListBox->GetSelIndex() >= 0)
					pItem = (ZStagePlayerListItem*)pListBox->Get(pListBox->GetSelIndex());

				if ((pListBox->GetSelIndex() != -1) && (strcmp(ZGetMyInfo()->GetCharName(), pItem->GetString(3)) != 0))
					bShowMe = false;
			}

			ZPlayerInfo* pInfo = NULL;
			if (bShowMe)
				pInfo = ZGetPlayerManager()->Find(ZGetMyUID());
			else if (pItem != NULL)
				pInfo = ZGetPlayerManager()->Find(pItem->GetUID());

			MLabel* pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerName");
			if (pLabel)
			{
				if (bShowMe)
					sprintf(buf, "%s", ZGetMyInfo()->GetCharName());
				else
					sprintf(buf, "%s", pItem->GetString(3));
				pLabel->SetText(buf);
			}

			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecClan");
			if (pLabel)
			{
				if (bShowMe)
					sprintf(buf, "%s : %s", ZMsg(MSG_CHARINFO_CLAN), ZGetMyInfo()->GetClanName());
				else
				{
					if (strcmp(pItem->GetString(5), "") == 0)
						sprintf(buf, "%s :", ZMsg(MSG_CHARINFO_CLAN));
					else
						sprintf(buf, "%s : %s", ZMsg(MSG_CHARINFO_CLAN), pItem->GetString(5));
				}
				pLabel->SetText(buf);
			}

			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecLevel");
			if (pLabel)
			{
				if (bShowMe)
					sprintf(buf, "%s : %d %s", ZMsg(MSG_CHARINFO_LEVEL), ZGetMyInfo()->GetLevel(), ZMsg(MSG_CHARINFO_LEVELMARKER));
				else
					sprintf(buf, "%s : %s %s", ZMsg(MSG_CHARINFO_LEVEL), pItem->GetString(1), ZMsg(MSG_CHARINFO_LEVELMARKER));
				pLabel->SetText(buf);
			}

			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecXP");
			if (pLabel)
			{
				if (bShowMe)
					sprintf(buf, "%s : %d%%", ZMsg(MSG_CHARINFO_XP), ZGetMyInfo()->GetLevelPercent());
				else
					sprintf(buf, "%s : -", ZMsg(MSG_CHARINFO_XP));
				pLabel->SetText(buf);
				pLabel->Enable(bShowMe);
			}

			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecBP");
			if (pLabel)
			{
				if (bShowMe)
					sprintf(buf, "%s : %d", ZMsg(MSG_CHARINFO_BOUNTY), ZGetMyInfo()->GetBP());
				else
					sprintf(buf, "%s : -", ZMsg(MSG_CHARINFO_BOUNTY));
				pLabel->SetText(buf);
				pLabel->Enable(bShowMe);
			}

			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecHP");
			if (pLabel)
			{
				if (bShowMe)
					sprintf(buf, "%s : %d", ZMsg(MSG_CHARINFO_HP), ZGetMyInfo()->GetHP());
				else
					sprintf(buf, "%s : -", ZMsg(MSG_CHARINFO_HP));
				pLabel->SetText(buf);
				pLabel->Enable(bShowMe);
			}

			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecAP");
			if (pLabel)
			{
				if (bShowMe)
					sprintf(buf, "%s : %d", ZMsg(MSG_CHARINFO_AP), ZGetMyInfo()->GetAP());
				else
					sprintf(buf, "%s : -", ZMsg(MSG_CHARINFO_AP));
				pLabel->SetText(buf);
				pLabel->Enable(bShowMe);
			}

			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecWT");
			if (pLabel)
			{
				ZMyItemList* pItems = ZGetMyInfo()->GetItemList();
				if (bShowMe)
					sprintf(buf, "%s : %d/%d", ZMsg(MSG_CHARINFO_WEIGHT), pItems->GetEquipedTotalWeight(), pItems->GetMaxWeight());
				else
					sprintf(buf, "%s : -", ZMsg(MSG_CHARINFO_WEIGHT));
				pLabel->SetText(buf);
				pLabel->Enable(bShowMe);
			}

			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecRanking");
			if (pLabel && pInfo)
			{
				if (pInfo->GetRank() == 0)
					sprintf(buf, "%s : -", ZMsg(MSG_CHARINFO_RANKING));
				else
					sprintf(buf, "%s : %d", ZMsg(MSG_CHARINFO_RANKING), pInfo->GetRank());
				pLabel->SetText(buf);
			}

			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecKill");
			if (pLabel && pInfo)
			{
				sprintf(buf, "%s : %d", ZMsg(MSG_CHARINFO_KILL), pInfo->GetKill());
				pLabel->SetText(buf);
			}

			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecDeath");
			if (pLabel && pInfo)
			{
				sprintf(buf, "%s : %d", ZMsg(MSG_CHARINFO_DEATH), pInfo->GetDeath());
				pLabel->SetText(buf);
			}

			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecWinning");
			if (pLabel && pInfo)
			{
				sprintf(buf, "%s : %.1f%%", ZMsg(MSG_CHARINFO_WINNING), pInfo->GetWinningRatio());
				pLabel->SetText(buf);
			}

			if (bShowMe)
			{
				ZCharacterView* pCharView = (ZCharacterView*)pRes->FindWidget("Stage_Charviewer");
				if (pCharView)
					pCharView->SetCharacter(ZGetMyUID());
			}
		}

		ZApplication::GetStageInterface()->OnDrawStartMovieOfQuest();

		int nOpacity = 90.0f * (sin(timeGetTime() * 0.003f) + 1) + 75;

		MLabel* pLabel = (MLabel*)pRes->FindWidget("Stage_SenarioName");
		MPicture* pPicture = (MPicture*)pRes->FindWidget("Stage_Lights0");
		if (pPicture)
		{
			pPicture->SetOpacity(nOpacity);
		}
		pPicture = (MPicture*)pRes->FindWidget("Stage_Lights1");
		if (pPicture)
		{
			pPicture->SetOpacity(nOpacity);
		}

		MWidget* pWidget = pRes->FindWidget("Stage_ItemListView");
		if (!pWidget)
			return;

		int nEndPos = ZApplication::GetStageInterface()->m_nListFramePos;
		MRECT rect = pWidget->GetRect();
		if (rect.x != nEndPos)
		{
			int nNewPos = rect.x + (nEndPos - rect.x) * 0.25;
			if (nNewPos == rect.x)
				rect.x = nEndPos;
			else
				rect.x = nNewPos;

			pWidget->SetBounds(rect);

			if (rect.x == 0)
			{
				pWidget = pRes->FindWidget("Stage_CharacterInfo");
				if (pWidget)
					pWidget->Enable(false);
			}
		}

		pWidget = pRes->FindWidget("Stage_RelayMapListView");
		if (!pWidget)
			return;

		nEndPos = ZApplication::GetStageInterface()->m_nRelayMapListFramePos;
		rect = pWidget->GetRect();
		if (rect.x != nEndPos)
		{
			int nNewPos = rect.x + (nEndPos - rect.x) * 0.25;
			if (nNewPos == rect.x)
				rect.x = nEndPos;
			else
				rect.x = nNewPos;

			pWidget->SetBounds(rect);

			if (rect.x == 0)
			{
				pWidget = pRes->FindWidget("Stage_CharacterInfo");
				if (pWidget)
					pWidget->Enable(false);
			}
		}
	}
}

void ZGameInterface::OnDrawStateCharSelection(MDrawContext* pDC)
{
	if (m_pBackground && m_pCharacterSelectView)
	{
		m_pBackground->LoadMesh();
		m_pBackground->Draw();
		m_pCharacterSelectView->Draw();

		ZGetEffectManager()->Draw(timeGetTime());
	}
}

void ZGameInterface::OnDraw(MDrawContext* pDC)
{
	m_nDrawCount++;

	__BP(11, "ZGameInterface::OnDraw");

	if (m_bLoading)
	{
		__EP(11);
		return;
	}

#ifdef _BIRDTEST
	if (GetState() == GUNZ_BIRDTEST)
	{
		OnBirdTestDraw();
		__EP(11);
		return;
	}
#endif

	switch (GetState())
	{
	case GUNZ_GAME:
	{
		OnDrawStateGame(pDC);
	}
	break;
	case GUNZ_LOGIN:
	case GUNZ_DIRECTLOGIN:
	{
		OnDrawStateLogin(pDC);
	}
	break;
	case GUNZ_LOBBY:
	case GUNZ_STAGE:
	{
		OnDrawStateLobbyNStage(pDC);
	}
	break;
	case GUNZ_CHARSELECTION:
	case GUNZ_CHARCREATION:
	{
		OnDrawStateCharSelection(pDC);
	}
	break;
	}

#ifdef LOCALE_KOREA
	if (timeGetTime() >= m_dwTimeCount)
	{
		m_dwTimeCount += 3600000;
		m_dwHourCount++;

		char szText[256];
		if (m_dwHourCount > 3)
			sprintf(szText, "%d 시간이 경과했습니다. 잠시 휴식을 취하시기 바랍니다.", m_dwHourCount);
		else
			sprintf(szText, "%d 시간이 경과 하였습니다.", m_dwHourCount);
		ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szText);

		ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), "본 게임은 15세 이용가로서 만 15세 미만은 이용할 수 없습니다.");
	}
#endif

	__EP(11);
}

void ZGameInterface::TestChangePartsAll()
{
}

void ZGameInterface::TestChangeParts(int mode) {
#ifndef _PUBLISH
	RMeshPartsType ptype = eq_parts_etc;

	if (mode == 0) { ptype = eq_parts_chest; }
	else if (mode == 1) { ptype = eq_parts_head; }
	else if (mode == 2) { ptype = eq_parts_hands; }
	else if (mode == 3) { ptype = eq_parts_legs; }
	else if (mode == 4) { ptype = eq_parts_feet; }
	else if (mode == 5) { ptype = eq_parts_face; }

	ZPostChangeParts(ptype, 1);

#endif
}
void ZGameInterface::TestToggleCharacter()
{
	ZPostChangeCharacter();
}

void ZGameInterface::TestChangeWeapon(RVisualMesh* pVMesh)
{
	static int nWeaponIndex = 0;

	int nItemID = 0;
	switch (nWeaponIndex)
	{
	case 0:
		nItemID = 1;
		break;
	case 1:
		nItemID = 5;
		break;
	case 2:
		nItemID = 2;
		break;
	case 3:
		nItemID = 3;
		break;
	case 4:
		nItemID = 6;
		break;
	case 5:
		nItemID = 7;
		break;
	case 6:
		nItemID = 4;
		break;
	}

	if (GetState() == GUNZ_GAME)
	{
		if (m_pMyCharacter == NULL) return;

		switch (nWeaponIndex)
		{
		case 0:
		case 1:
			m_pMyCharacter->GetItems()->EquipItem(MMCIP_MELEE, nItemID);
			m_pMyCharacter->ChangeWeapon(MMCIP_MELEE);
			break;
		case 2:
		case 3:
		case 4:
		case 5:
			m_pMyCharacter->GetItems()->EquipItem(MMCIP_PRIMARY, nItemID);
			m_pMyCharacter->ChangeWeapon(MMCIP_PRIMARY);
			break;
		case 6:
			m_pMyCharacter->GetItems()->EquipItem(MMCIP_CUSTOM1, nItemID);
			m_pMyCharacter->ChangeWeapon(MMCIP_CUSTOM1);
			break;
		}
	}
	else if (GetState() == GUNZ_CHARSELECTION)
	{
		if (pVMesh != NULL)
		{
			ZChangeCharWeaponMesh(pVMesh, nItemID);
			pVMesh->SetAnimation("login_intro");
			pVMesh->GetFrameInfo(ani_mode_lower)->m_nFrame = 0;
		}
	}
	else if (GetState() == GUNZ_LOBBY)
	{
		if (pVMesh != NULL)
		{
			ZChangeCharWeaponMesh(pVMesh, nItemID);
		}
	}

	nWeaponIndex++;
	if (nWeaponIndex >= 7) nWeaponIndex = 0;
}

void ZGameInterface::RespawnMyCharacter()
{
	if (ZGetGame() == NULL) return;

	m_pMyCharacter->Revival();
	rvector pos = rvector(0, 0, 0), dir = rvector(0, 1, 0);

	ZMapSpawnData* pSpawnData = ZGetGame()->GetMapDesc()->GetSpawnManager()->GetSoloRandomData();
	if (pSpawnData != NULL)
	{
		pos = pSpawnData->m_Pos;
		dir = pSpawnData->m_Dir;
	}

	m_pMyCharacter->SetPosition(pos);
	m_pMyCharacter->SetDirection(dir);
}

bool ZGameInterface::OnGlobalEvent(MEvent* pEvent)
{
	if (ZGameInterface::CheckSkipGlobalEvent() == true)
		return true;

	if ((ZGetGameInterface()->GetState() == GUNZ_GAME))
		return ZGameInput::OnEvent(pEvent);

#ifndef _PUBLISH
	switch (pEvent->nMessage) {
	case MWM_CHAR:
	{
		switch (pEvent->nKey) {
		case '`':
			if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_DEVELOP)
			{
				ZGetConsole()->Show(!ZGetConsole()->IsVisible());
				ZGetConsole()->SetZOrder(MZ_TOP);
				return true;
			}
			break;
		}
	}break;

	case MWM_KEYDOWN:
	{
		if (pEvent->GetCtrlState() && pEvent->nKey == 'R') {
			GunzState state = ZGetGameInterface()->GetState();
			m_sbRemainClientConnectionForResetApp = true;
			ZLoadingProgress progress("");
			ZGetGameInterface()->OnDestroy();
			ZGetGameInterface()->OnCreate(&progress);
			ZGetGameInterface()->SetState(state);
			m_sbRemainClientConnectionForResetApp = false;
		}
	}
	}
#endif
	return false;
	}

bool ZGameInterface::OnDebugEvent(MEvent* pEvent, MListener* pListener)
{
	switch (pEvent->nMessage) {
	case MWM_KEYDOWN:
	{
		switch (pEvent->nKey)
		{
		case VK_F10:
			m_pLogFrame->Show(!m_pLogFrame->IsVisible());
			return true;
		case VK_NUMPAD8:
		{
			if (GetState() == GUNZ_CHARSELECTION)
			{
				if (m_pCharacterSelectView != NULL)
				{
				}
			}
			else if (GetState() == GUNZ_LOBBY)
			{
				if (ZGetCharacterViewList(GUNZ_LOBBY) != NULL)
				{
					RVisualMesh* pVMesh =
						ZGetCharacterViewList(GUNZ_LOBBY)->Get(ZGetGameClient()->GetPlayerUID())->GetVisualMesh();

					TestChangeWeapon(pVMesh);
				}
			}
		}
		break;
		}
	}
	break;
	}
	return false;
}

bool ZGameInterface::OnEvent(MEvent* pEvent, MListener* pListener)
{
#ifndef _PUBLISH
	if (OnDebugEvent(pEvent, pListener)) return true;
#endif

	return false;
}

bool ZGameInterface::OnCommand(MWidget* pWidget, const char* szMessage)
{
	if (pWidget == m_pPlayerMenu) {
		MMenuItem* pItem = (MMenuItem*)pWidget;

		OutputDebugString("PLAYERMENU");
	}
	return false;
}

void ZGameInterface::ChangeParts(int key)
{
}
void ZGameInterface::ChangeWeapon(ZChangeWeaponType nType)
{
	ZMyCharacter* pChar = ZGetGame()->m_pMyCharacter;

	if (pChar->m_pVMesh == NULL) return;
	if (pChar->IsDie()) return;
	if (m_pGame->GetMatch()->IsRuleGladiator()) return;

	int nParts = -1;

	bool bWheel = false;
	if ((nType == ZCWT_PREV) || (nType == ZCWT_NEXT))
	{
		bWheel = true;

		int nHasItemCount = 0;
		int nPos = -1;
		int ItemQueue[MMCIP_END];

		for (int i = MMCIP_MELEE; i < MMCIP_CUSTOM2 + 1; i++)
		{
			if (!pChar->GetItems()->GetItem((MMatchCharItemParts)i)->IsEmpty())
			{
				if (pChar->GetItems()->GetSelectedWeaponParts() == i)
					nPos = nHasItemCount;

				ItemQueue[nHasItemCount++] = i;
			}
		}

		if (nPos < 0) return;

		if (nType == ZCWT_PREV)
		{
			if (nHasItemCount <= 1) return;

			int nNewPos = nPos - 1;
			int nCount = 0;
			if (nNewPos < 0) nNewPos = nHasItemCount - 1;
			while (nNewPos != nPos)
			{
				MMatchCharItemParts nPart = (MMatchCharItemParts)ItemQueue[nNewPos];
				if ((nPart == MMCIP_CUSTOM1) || (nPart == MMCIP_CUSTOM2)) {
					ZItem* pItem = ZGetGame()->m_pMyCharacter->GetItems()->GetItem(nPart);
					if (pItem && pItem->GetBulletCurrMagazine() > 0) { break; }
				}
				else { break; }

				nNewPos = nNewPos - 1;
				if (nNewPos < 0) nNewPos = nHasItemCount - 1;

				nCount++;
				if (nCount > (nHasItemCount * 2))
					return;
			}
			nPos = nNewPos;
		}
		else if (nType == ZCWT_NEXT)
		{
			if (nHasItemCount <= 1) return;

			int nNewPos = nPos + 1;
			if (nNewPos >= nHasItemCount) nNewPos = 0;
			while (nNewPos != nPos)
			{
				MMatchCharItemParts nPart = (MMatchCharItemParts)ItemQueue[nNewPos];
				if ((nPart == MMCIP_CUSTOM1) || (nPart == MMCIP_CUSTOM2)) {
					ZItem* pItem = ZGetGame()->m_pMyCharacter->GetItems()->GetItem(nPart);
					if (pItem && pItem->GetBulletCurrMagazine() > 0) { break; }
				}
				else { break; }

				nNewPos = nNewPos + 1;
				if (nNewPos >= nHasItemCount) nNewPos = 0;
			}
			nPos = nNewPos;
		}

		nParts = ItemQueue[nPos];
	}
	else
	{
		switch (nType)
		{
		case ZCWT_MELEE:	nParts = (int)(MMCIP_MELEE);	break;
		case ZCWT_PRIMARY:	nParts = (int)(MMCIP_PRIMARY);	break;
		case ZCWT_SECONDARY:nParts = (int)(MMCIP_SECONDARY); break;
		case ZCWT_CUSTOM1:	nParts = (int)(MMCIP_CUSTOM1);	break;
		case ZCWT_CUSTOM2:	nParts = (int)(MMCIP_CUSTOM2);	break;
		default: break;
		}

		if ((nParts == MMCIP_CUSTOM1) || (nParts == MMCIP_CUSTOM2))
		{
			ZItem* pItem = pChar->GetItems()->GetItem((MMatchCharItemParts)nParts);
			if (pItem->GetBulletCurrMagazine() <= 0) return;
		}
	}

	if (nParts < 0) return;
	if (pChar->GetItems()->GetSelectedWeaponParts() == nParts) return;

	if (bWheel && (pChar->GetStateUpper() == ZC_STATE_UPPER_LOAD && pChar->IsUpperPlayDone() == false))
		return;

	const ZCharaterStatusBitPacking& uStatus = pChar->m_dwStatusBitPackingValue.Ref();
	ZMyCharaterStatusBitPacking& zStatus = pChar->m_statusFlags.Ref();

	if (zStatus.m_bWallHang || zStatus.m_bShot || zStatus.m_bShotReturn || uStatus.m_bTumble
		|| zStatus.m_bSkill || zStatus.m_bGuard || uStatus.m_bBlast || uStatus.m_bBlastFall
		|| uStatus.m_bBlastDrop || uStatus.m_bBlastStand || uStatus.m_bBlastAirmove
		|| zStatus.m_bSlash || zStatus.m_bJumpSlash || zStatus.m_bJumpSlashLanding

		|| (pChar->GetStateUpper() == ZC_STATE_UPPER_SHOT && pChar->IsUpperPlayDone() == false)

		)
	{
		m_bReservedWeapon = true;
		m_ReservedWeapon = nType;
		return;
	}

	m_bReservedWeapon = false;
	ZPostChangeWeapon(nParts);

	m_pMyCharacter->m_dwStatusBitPackingValue.Ref().m_bSpMotion = false;
	m_pMyCharacter->ChangeWeapon((MMatchCharItemParts)nParts);
}

void ZGameInterface::OnGameUpdate(float fElapsed)
{
	__BP(12, "ZGameInterface::OnGameUpdate");
	if (m_pGame == NULL) return;
	if (m_pGameInput) m_pGameInput->Update(fElapsed);

	m_pGame->Update(fElapsed);

	if (m_pCombatInterface) m_pCombatInterface->Update(fElapsed);

	if (m_bReservedWeapon)
		ChangeWeapon(m_ReservedWeapon);

	__EP(12);
}

extern bool g_bTestFromReplay;

void ZGameInterface::OnReplay()
{
	ShowWidget("ReplayConfirm", false);

	CreateReplayGame(NULL);
}

bool ZGameInterface::Update(float fElapsed)
{
	if (m_bReservedQuit) {
		if (timeGetTime() > m_dwReservedQuitTimer) {
			ZGetApplication()->Exit();
		}
	}

	if (m_pBackground && ((GetState() == GUNZ_CHARSELECTION) || (GetState() == GUNZ_CHARCREATION)))
		m_pBackground->OnUpdate(fElapsed);

	OnVoiceSound();

	if (GetState() == GUNZ_LOBBY)
	{
		if (g_bTestFromReplay == true)
		{
			ShowWidget("Lobby", false);
			ShowWidget("ReplayConfirm", true);
			return false;
		}
	}

	__BP(13, "ZGameInterface::Update");

	ZGetOptionInterface()->Update();

	GetShopEquipInterface()->Update();

	__BP(14, "ZGameInterface::GameClient Run & update");
	if (m_spGameClient != NULL) m_spGameClient->Run();
	m_spGameClient->Tick();
	__EP(14);

	if (!m_bLoading) {
		if (GetState() == GUNZ_GAME) {
			OnGameUpdate(fElapsed);
		}
		else {
#ifdef _BIRDTEST
			if (GetState() == GUNZ_BIRDTEST) OnBirdTestUpdate();
#endif
		}
		}

	if (GetState() == GUNZ_LOBBY && m_bWaitingArrangedGame) {
		MLabel* pLabel = (MLabel*)m_IDLResource.FindWidget("LobbyWaitingArrangedGameLabel");
		if (pLabel) {
			int nCount = (timeGetTime() / 500) % 5;
			char dots[10];
			for (int i = 0; i < nCount; i++) {
				dots[i] = '.';
			}
			dots[nCount] = 0;

			char szBuffer[256];
			sprintf(szBuffer, "%s%s", ZMsg(MSG_WORD_FINDTEAM), dots);
			pLabel->SetText(szBuffer);
		}
	}

	MWidget* pDTWaitMatchDlg = m_IDLResource.FindWidget("DuelTournamentWaitMatchDialog");
	if (pDTWaitMatchDlg && pDTWaitMatchDlg->IsVisible() && GetState() != GUNZ_LOBBY)
		OnDuelTournamentGameUI(false);

	UpdateDuelTournamentWaitMsgDots();

	UpdateCursorEnable();

	if (ZGetGame() != NULL && m_bLeaveBattleReserved && (m_dwLeaveBattleTime < timeGetTime()))
		LeaveBattle();

	__EP(13);

	return true;
	}

void ZGameInterface::OnResetCursor()
{
	SetCursorEnable(m_bCursor);
}

void ZGameInterface::SetCursorEnable(bool bEnable)
{
	if (m_bCursor == bEnable) return;

	m_bCursor = bEnable;
	MCursorSystem::Show(bEnable);
}

void ZGameInterface::UpdateCursorEnable()
{
	if (GetState() != GUNZ_GAME ||
		(GetCombatInterface() && GetCombatInterface()->IsShowResult()) ||
		IsMenuVisible() ||
		m_pMsgBox->IsVisible() ||
		m_pConfirmMsgBox->IsVisible())
		SetCursorEnable(true);
	else
	{
		MWidget* pWidget = m_IDLResource.FindWidget("112Confirm");
		if (pWidget && pWidget->IsVisible())
			SetCursorEnable(true);

		else
			SetCursorEnable(false);
	}
}

void ZGameInterface::UpdateDuelTournamentWaitMsgDots()
{
	if (GetState() != GUNZ_LOBBY) return;

	MLabel* pLabel = (MLabel*)m_IDLResource.FindWidget("DuelTournamentWaitMatchLabel");
	if (pLabel == NULL || !pLabel->IsVisible()) return;

	const char* szText = pLabel->GetText();
	size_t len = strlen(szText);
	if (len == 0) return;

	int nCount = (timeGetTime() / 500) % 5 + 1;
	char dots[10];
	for (int i = 0; i < nCount; i++) {
		dots[i] = '.';
	}
	dots[nCount] = 0;

	int msgEndPosExceptDots = 0;
	for (size_t pos = len - 1; pos >= 0; --pos) {
		if (szText[pos] != '.') {
			msgEndPosExceptDots = (int)pos;
			break;
		}
	}

	char szBuffer[256];
	strncpy(szBuffer, szText, msgEndPosExceptDots + 1);
	szBuffer[msgEndPosExceptDots + 1] = 0;
	strcat(szBuffer, dots);
	pLabel->SetText(szBuffer);
}

void ZGameInterface::SetMapThumbnail(const char* szMapName)
{
	SAFE_DELETE(m_pMapThumbnail);

	char szThumbnail[256];
	sprintf(szThumbnail, "maps/%s/%s.rs.bmp", szMapName, szMapName);

	m_pMapThumbnail = Mint::GetInstance()->OpenBitmap(szThumbnail);
	if (!m_pMapThumbnail)
	{
		sprintf(szThumbnail, "maps/%s/%s.bmp", szMapName, szMapName);
		m_pMapThumbnail = Mint::GetInstance()->OpenBitmap(szThumbnail);
	}
}

void ZGameInterface::ClearMapThumbnail()
{
	SAFE_DELETE(m_pMapThumbnail);
}

void ZGameInterface::Reload()
{
	if (!ZGetGame()->m_pMyCharacter->GetItems()->GetSelectedWeapon()) return;
	MMatchItemDesc* pSelectedItemDesc = ZGetGame()->m_pMyCharacter->GetItems()->GetSelectedWeapon()->GetDesc();

	if (pSelectedItemDesc == NULL) return;

	if (pSelectedItemDesc->m_nType.Ref() != MMIT_RANGE)  return;

	if (ZGetGame()->m_pMyCharacter->GetItems()->GetSelectedWeapon()->isReloadable() == false) return;

	ZMyCharacter* pChar = ZGetGame()->m_pMyCharacter;

	ZCharaterStatusBitPacking& uStatus = pChar->m_dwStatusBitPackingValue.Ref();

	if (uStatus.m_bBlast ||
		uStatus.m_bBlastFall ||
		uStatus.m_bBlastDrop ||
		uStatus.m_bBlastStand ||
		uStatus.m_bBlastAirmove)
		return;

	if (pChar->GetStateUpper() == ZC_STATE_UPPER_RELOAD) return;

	uStatus.m_bSpMotion = false;

	ZPostReload();
}

void ZGameInterface::SaveScreenShot()
{
	static unsigned long int st_nLastTime = 0;
	unsigned long int nNowTime = timeGetTime();
#define SCREENSHOT_DELAY		2000

	if ((nNowTime - st_nLastTime) < SCREENSHOT_DELAY)	return;
	st_nLastTime = nNowTime;

	char szPath[_MAX_PATH];
	char szFilename[_MAX_PATH];
	char szFilenameSafe[_MAX_PATH];

	TCHAR szMyDocPath[MAX_PATH];
	if (GetMyDocumentsPath(szMyDocPath)) {
		strcpy(szPath, szMyDocPath);
		strcat(szPath, GUNZ_FOLDER);
		CreatePath(szPath);
		strcat(szPath, SCREENSHOT_FOLDER);
		CreatePath(szPath);
		strcat(szPath, "/");
	}

	SYSTEMTIME t;
	GetLocalTime(&t);
	char szCharName[MATCHOBJECT_NAME_LENGTH];
	ValidateFilename(szCharName, ZGetMyInfo()->GetCharName(), '_');

	sprintf(szFilename, "%s_%4d%02d%02d_%02d%02d%02d",
		szCharName, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);

	sprintf(szFilenameSafe, "nocharname_%4d%02d%02d_%02d%02d%02d",
		t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);

	LPBYTE data = NULL;
	LPDIRECT3DSURFACE9 frontbuffer = NULL;

	{
		HRESULT hr;
		LPDIRECT3DDEVICE9 pDevice = RGetDevice();

		D3DDISPLAYMODE d3ddm;
		pDevice->GetDisplayMode(0, &d3ddm);

		hr = pDevice->CreateOffscreenPlainSurface(d3ddm.Width, d3ddm.Height, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &frontbuffer, NULL);
		hr = pDevice->GetFrontBufferData(0, frontbuffer);

		RECT rt;
		GetWindowRect(g_hWnd, &rt);

		D3DLOCKED_RECT LRECT;
		if (frontbuffer->LockRect(&LRECT, &rt, NULL) == D3D_OK)
		{
			int nWidth = rt.right - rt.left;
			int nHeight = rt.bottom - rt.top;

			int pitch = LRECT.Pitch;
			LPBYTE source = (LPBYTE)LRECT.pBits;

			data = new BYTE[nWidth * nHeight * 4];
			for (int i = 0; i < nHeight; i++)
			{
				memcpy(data + i * nWidth * 4, source + pitch * i, nWidth * 4);
			}

			char szFullPath[_MAX_PATH];
			char szFullPathToNotify[_MAX_PATH];

			sprintf(szFullPath, "%s%s", szPath, szFilename);
			sprintf(szFullPathToNotify, GUNZ_FOLDER SCREENSHOT_FOLDER"/""%s.jpg", szFilename);
			bool bSuccess = RScreenShot(nWidth, nHeight, data, szFullPath);

			if (!bSuccess) {
				sprintf(szFullPath, "%s%s", szPath, szFilenameSafe);
				sprintf(szFullPathToNotify, GUNZ_FOLDER SCREENSHOT_FOLDER"/""%s.jpg", szFilenameSafe);
				bSuccess = RScreenShot(nWidth, nHeight, data, szFullPath);

				if (!bSuccess) goto SCREENSHOTERROR;
			}

			char szOutput[_MAX_PATH * 2];
			ZTransMsg(szOutput, MSG_SCREENSHOT_SAVED, 1, szFullPathToNotify);
			ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szOutput);

			delete[] data;
		}
		SAFE_RELEASE(frontbuffer);
	}
	return;

SCREENSHOTERROR:
	delete[] data;
	SAFE_RELEASE(frontbuffer);

	ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM),
		ZMsg(MSG_SCREENSHOT_CANT_SAVE));

	return;
}

void ZGameInterface::ShowMessage(const char* szText, MListener* pCustomListenter, int nMessageID)
{
	if (pCustomListenter)
		m_pMsgBox->SetCustomListener(pCustomListenter);

	char text[1024] = "";

	if (nMessageID != 0)
	{
		sprintf(text, "%s (M%d)", szText, nMessageID);
	}
	else
	{
		strcpy(text, szText);
	}

	m_pMsgBox->SetText(text);
	m_pMsgBox->Show(true, true);
}

void ZGameInterface::ShowConfirmMessage(const char* szText, MListener* pCustomListenter)
{
	if (pCustomListenter)
		m_pConfirmMsgBox->SetCustomListener(pCustomListenter);

	m_pConfirmMsgBox->SetText(szText);
	m_pConfirmMsgBox->Show(true, true);
}

void ZGameInterface::ShowMessage(int nMessageID)
{
	const char* str = ZMsg(nMessageID);
	if (str)
	{
		char text[1024];
		sprintf(text, "%s (M%d)", str, nMessageID);
		ShowMessage(text);
	}
}

void ZGameInterface::ShowErrorMessage(int nErrorID)
{
	const char* str = ZErrStr(nErrorID);
	if (str)
	{
		char text[1024];
		sprintf(text, "%s (E%d)", str, nErrorID);
		ShowMessage(text);
	}
}

void ZGameInterface::ShowErrorMessage(const char* szErrorMsg, int nErrorID)
{
	char szErrorID[64];
	sprintf(szErrorID, " (E%d)", nErrorID);

	char text[1024];
	strcpy(text, szErrorMsg);
	strcat(text, szErrorID);

	ShowMessage(text);
}

void ZGameInterface::ChangeSelectedChar(int nNum)
{
	bool bRequested = false;
	if ((!ZCharacterSelectView::m_CharInfo[nNum].m_bLoaded) && (!ZCharacterSelectView::m_CharInfo[nNum].m_bRequested))
	{
		ZPostAccountCharInfo(nNum);
		GetCharacterSelectView()->UpdateInterface(nNum);
		ZCharacterSelectView::m_CharInfo[nNum].m_bRequested = true;
		bRequested = true;
	}

	if ((!bRequested) && (GetCharacterSelectView() != NULL))
	{
		GetCharacterSelectView()->SelectChar(nNum);
	}
}

void ZGameInterface::OnCharSelectionCreate(void)
{
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_INTRO, ZApplication::GetFileSystem());

	EnableCharSelectionInterface(true);

	if (m_pCharacterSelectView != NULL) SAFE_DELETE(m_pCharacterSelectView);
	m_pCharacterSelectView = new ZCharacterSelectView();
	m_pCharacterSelectView->SetBackground(m_pBackground);
	m_pCharacterSelectView->SelectChar(ZCharacterSelectView::GetSelectedCharacter());

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MWidget* pWidget = pResource->FindWidget("CS_SelectCharDefKey");
	if (pWidget)
		pWidget->Enable(true);

	pWidget = pResource->FindWidget("CharSel_GameRoomUser");
	if (pWidget)
	{
		if (ZGetMyInfo()->GetPGradeID() == MMPG_PREMIUM_IP)
			pWidget->Show(true);
		else
			pWidget->Show(false);
	}

#ifdef LOCALE_NHNUSAA
	GetNHNUSAReport().ReportDisplaySelCharacter();
#endif
}

void ZGameInterface::OnCharSelect(void)
{
	if (m_pCharacterSelectView->SelectMyCharacter() == false)
		return;

	EnableCharSelectionInterface(false);

	if (m_pBackground)
		m_pBackground->SetScene(LOGIN_SCENE_SELECTCHAR);

#ifdef LOCALE_NHNUSAA
	GetNHNUSAReport().ReportSelectCharacter();
#endif
}

void ZGameInterface::OnCharSelectionDestroy(void)
{
	ShowWidget("CharSelection", false);
	if (m_pCharacterSelectView != NULL) SAFE_DELETE(m_pCharacterSelectView);
}

void ZGameInterface::OnCharCreationCreate(void)
{
	ShowWidget("CharSelection", false);
	ShowWidget("CharCreation", true);

	if (m_pCharacterSelectView != NULL) SAFE_DELETE(m_pCharacterSelectView);
	m_pCharacterSelectView = new ZCharacterSelectView();
	m_pCharacterSelectView->SetBackground(m_pBackground);
	m_pCharacterSelectView->SetState(ZCharacterSelectView::ZCSVS_CREATE);
	m_pCharacterSelectView->OnChangedCharCostume();

#ifdef LOCALE_NHNUSAA
	GetNHNUSAReport().ReportDisplayCreateCharacter();
#endif
}

void ZGameInterface::OnCharCreationDestroy(void)
{
	ShowWidget("CharCreation", false);
	ShowWidget("CharSelection", true);

	if (m_pCharacterSelectView != NULL) SAFE_DELETE(m_pCharacterSelectView);
}

void ZGameInterface::ChangeToCharSelection(void)
{
	ZCharacterSelectView::ClearCharInfo();
	ZPostAccountCharList(ZGetMyInfo()->GetSystemInfo()->pbyGuidAckMsg);
}

void ZGameInterface::OnInvalidate()
{
	ZGetWorldManager()->OnInvalidate();
	if (m_pGame)
		m_pGame->OnInvalidate();
	if (m_pBackground)
		m_pBackground->OnInvalidate();
	if (m_pCharacterSelectView)
		m_pCharacterSelectView->OnInvalidate();
	if (ZGetEffectManager())
		ZGetEffectManager()->OnInvalidate();

	ZCharacterView* pCharView = (ZCharacterView*)m_IDLResource.FindWidget("Stage_Charviewer");
	if (pCharView != 0) pCharView->OnInvalidate();
	pCharView = (ZCharacterView*)m_IDLResource.FindWidget("EquipmentInformation");
	if (pCharView != 0) pCharView->OnInvalidate();
	pCharView = (ZCharacterView*)m_IDLResource.FindWidget("EquipmentInformationShop");
	if (pCharView != 0) pCharView->OnInvalidate();

	if (m_pCombatInterface)
		m_pCombatInterface->OnInvalidate();
}

void ZGameInterface::OnRestore()
{
	ZGetWorldManager()->OnRestore();
	if (m_pGame)
		m_pGame->OnRestore();
	if (m_pBackground)
		m_pBackground->OnRestore();
	if (m_pCharacterSelectView)
		m_pCharacterSelectView->OnRestore();
	if (ZGetEffectManager())
		ZGetEffectManager()->OnRestore();

	ZCharacterView* pCharView = (ZCharacterView*)m_IDLResource.FindWidget("Stage_Charviewer");
	if (pCharView != 0) pCharView->OnRestore();
	pCharView = (ZCharacterView*)m_IDLResource.FindWidget("EquipmentInformation");
	if (pCharView != 0) pCharView->OnRestore();
	pCharView = (ZCharacterView*)m_IDLResource.FindWidget("EquipmentInformationShop");
	if (pCharView != 0) pCharView->OnRestore();

	if (m_pCombatInterface)
		m_pCombatInterface->OnRestore();
}

void ZGameInterface::UpdateBlueRedTeam(void)
{
	MButton* pBlueTeamBtn = (MButton*)m_IDLResource.FindWidget("StageTeamBlue");
	MButton* pBlueTeamBtn2 = (MButton*)m_IDLResource.FindWidget("StageTeamBlue2");
	MButton* pRedTeamBtn = (MButton*)m_IDLResource.FindWidget("StageTeamRed");
	MButton* pRedTeamBtn2 = (MButton*)m_IDLResource.FindWidget("StageTeamRed2");
	if ((pRedTeamBtn == NULL) || (pBlueTeamBtn == NULL) || (pRedTeamBtn2 == NULL) || (pBlueTeamBtn2 == NULL))
		return;

	if (m_bTeamPlay)
	{
		pRedTeamBtn->Show(true);
		pRedTeamBtn2->Show(true);
		pBlueTeamBtn->Show(true);
		pBlueTeamBtn2->Show(true);

		ZPlayerListBox* pListBox = (ZPlayerListBox*)m_IDLResource.FindWidget("StagePlayerList_");

		if (pListBox)
		{
			int nR = 0, nB = 0;

			for (int i = 0; i < pListBox->GetCount(); i++)
			{
				ZStagePlayerListItem* pSItem = (ZStagePlayerListItem*)pListBox->Get(i);
				if (pSItem->m_nTeam == 1)		nR++;
				else if (pSItem->m_nTeam == 2)	nB++;
			}

			char buffer[64];
			ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();
			ZBmNumLabel* pNumLabel;

			sprintf(buffer, "%s:%d", ZMsg(MSG_WORD_BLUETEAM), nB);
			pNumLabel = (ZBmNumLabel*)pRes->FindWidget("StageNumOfBlueTeam");
			if (pNumLabel)
			{
				pNumLabel->SetText(buffer);
				pNumLabel->Show(true);
			}
			MButton* pButton = (MButton*)pRes->FindWidget("StageTeamBlue");
			if (pButton)
				pButton->SetText(buffer);

			sprintf(buffer, "%s:%d", ZMsg(MSG_WORD_REDTEAM), nR);
			pNumLabel = (ZBmNumLabel*)pRes->FindWidget("StageNumOfRedTeam");
			if (pNumLabel)
			{
				pNumLabel->SetIndexOffset(16);
				pNumLabel->SetText(buffer);
				pNumLabel->Show(true);
			}
			pButton = (MButton*)pRes->FindWidget("StageTeamRed");
			if (pButton)
				pButton->SetText(buffer);

			sprintf(buffer, "%s : %d", ZMsg(MSG_WORD_PLAYERS), nB + nR);
			MLabel* pLabel = (MLabel*)pRes->FindWidget("Stage_NumOfPerson");
			if (pLabel)
				pLabel->SetText(buffer);
		}
	}
	else
	{
		pRedTeamBtn->Show(false);
		pRedTeamBtn2->Show(false);
		pBlueTeamBtn->Show(false);
		pBlueTeamBtn2->Show(false);

		ZPlayerListBox* pListBox = (ZPlayerListBox*)m_IDLResource.FindWidget("StagePlayerList_");

		if (pListBox)
		{
			int nPlayerNum = 0;

			for (int i = 0; i < pListBox->GetCount(); i++)
			{
				ZStagePlayerListItem* pSItem = (ZStagePlayerListItem*)pListBox->Get(i);
				if (pSItem->m_nTeam == 0)
					nPlayerNum++;
			}

			ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();
			ZBmNumLabel* pNumLabel;

			char buffer[64];

			pNumLabel = (ZBmNumLabel*)pRes->FindWidget("StageNumOfBlueTeam");
			if (pNumLabel)
				pNumLabel->Show(false);

			pNumLabel = (ZBmNumLabel*)pRes->FindWidget("StageNumOfRedTeam");
			if (pNumLabel)
				pNumLabel->Show(false);

			sprintf(buffer, "%s : %d", ZMsg(MSG_WORD_PLAYERS), nPlayerNum);
			MLabel* pLabel = (MLabel*)pRes->FindWidget("Stage_NumOfPerson");
			if (pLabel)
				pLabel->SetText(buffer);
		}
	}
}

void ZGameInterface::ShowInterface(bool bShowInterface)
{
	m_bShowInterface = bShowInterface;

	if (m_nState != GUNZ_GAME)
	{
		SetCursorEnable(bShowInterface);
	}

	if (m_nState == GUNZ_LOGIN)
	{
		ShowWidget("Login", m_bShowInterface);
	}
	else if (m_nState == GUNZ_CHARSELECTION)
	{
		ShowWidget("CharSelection", m_bShowInterface);
	}
	else if (m_nState == GUNZ_GAME)
	{
		bool bConsole = ZGetConsole()->IsVisible();
		bool bLogFrame = m_pLogFrame->IsVisible();

		m_pLogFrame->Show(m_bShowInterface);
		ZGetConsole()->Show(m_bShowInterface);
		ShowWidget("CombatInfo1", m_bShowInterface);
		ShowWidget("CombatInfo2", m_bShowInterface);
		ShowWidget("Time", m_bShowInterface);
		ZGetConsole()->Show(bConsole);
		m_pLogFrame->Show(bLogFrame);
	}
}

void ZGameInterface::OnResponseShopItemList(const vector< MTD_ShopItemInfo*>& vShopItemList, const vector<MTD_GambleItemNode*>& vGItemList)
{
	ZGetShop()->SetItemsAll(vShopItemList);
	ZGetShop()->SetItemsGamble(vGItemList);
	ZGetShop()->Serialize();
}

void ZGameInterface::OnResponseCharacterItemList(MUID* puidEquipItem
	, MTD_ItemNode* pItemNodes
	, int nItemCount
	, MTD_GambleItemNode* pGItemNodes
	, int nGItemCount)
{
	ZGetMyInfo()->GetItemList()->SetItemsAll(pItemNodes, nItemCount);
	ZGetMyInfo()->GetItemList()->SetGambleItemAll(pGItemNodes, nGItemCount);
	ZGetMyInfo()->GetItemList()->SetEquipItemsAll(puidEquipItem);
	ZGetMyInfo()->GetItemList()->Serialize();

	GetShopEquipInterface()->DrawCharInfoText();
}

void ZGameInterface::FinishGame()
{
	ZGetGame()->StopRecording();
	ZGetGameInterface()->GetCombatInterface()->Finish();
	m_bGameFinishLeaveBattle = true;
}

void ZGameInterface::SerializeStageInterface()
{
	MComboBox* pCombo = (MComboBox*)m_IDLResource.FindWidget("MapSelection");
	if (pCombo)
		InitMaps(pCombo);

	ZApplication::GetStageInterface()->OnStageInterfaceSettup();
}

void ZGameInterface::HideAllWidgets()
{
	ShowWidget("Login", false);
	ShowWidget("Lobby", false);
	ShowWidget("Stage", false);
	ShowWidget("Game", false);
	ShowWidget("Option", false);
	ShowWidget("CharSelection", false);
	ShowWidget("CharCreation", false);
	ShowWidget("Shop", false);
	ShowWidget("StageSettingFrame", false);
	ShowWidget("BuyConfirm", false);
	ShowWidget("Equipment", false);
	ShowWidget("StageCreateFrame", false);
	ShowWidget("PrivateStageJoinFrame", false);
}

bool SetWidgetToolTipText(char* szWidget, const char* szToolTipText, MAlignmentMode mam) {
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if (pResource == NULL)		return false;
	if (!szToolTipText)		return false;

	MWidget* pWidget = pResource->FindWidget(szWidget);

	if (pWidget) {
		if (!szToolTipText[0]) {
			pWidget->DetachToolTip();
		}
		else {
			pWidget->AttachToolTip(new ZToolTip(szToolTipText, pWidget, mam));
		}
	}
	return false;
}

bool GetItemDescName(string& str, DWORD nItemID)
{
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);

	if (pItemDesc == NULL)
	{
		str.clear();
		return false;
	}

	str = (string)pItemDesc->m_pMItemName->Ref().m_szItemName;
	return false;
}

void ZGameInterface::ShowEquipmentDialog(bool bShow)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if (bShow)
	{
		ShowWidget("Lobby", false);
		ShowWidget("Stage", false);
		ShowWidget("Shop", false);

		MButton* pButton = (MButton*)pResource->FindWidget("Equipment_to_Shop");
		MLabel* pLabel = (MLabel*)pResource->FindWidget("Equip_Message");
		if (pButton && pLabel)
		{
			char buf[256];
			if (ZApplication::GetGameInterface()->GetState() == GUNZ_STAGE)
			{
				pButton->Show(false);
				sprintf(buf, "%s > %s > %s", ZGetGameClient()->GetServerName(), ZMsg(MSG_WORD_STAGE), ZMsg(MSG_WORD_EQUIPMENT));
				pLabel->SetText(buf);
			}
			else
			{
				pButton->Show(true);
				sprintf(buf, "%s > %s > %s", ZGetGameClient()->GetServerName(), ZMsg(MSG_WORD_LOBBY), ZMsg(MSG_WORD_EQUIPMENT));
				pLabel->SetText(buf);
			}
		}

		MWidget* pWidget = pResource->FindWidget("Equipment");
		if (pWidget != NULL) pWidget->Show(true, true);

		BEGIN_WIDGETLIST("EquipmentInformation", pResource, ZCharacterView*, pCharacterView);
		ZMyInfo* pmi = ZGetMyInfo();
		ZMyItemList* pil = ZGetMyInfo()->GetItemList();

		unsigned long int nEquipedItemID[MMCIP_END];

		for (int i = 0; i < MMCIP_END; i++)
		{
			nEquipedItemID[i] = pil->GetEquipedItemID(MMatchCharItemParts(i));
		}
		pCharacterView->InitCharParts(pmi->GetSex(), pmi->GetHair(), pmi->GetFace(), nEquipedItemID);
		END_WIDGETLIST();

		GetShopEquipInterface()->SelectEquipmentTab(0);

		ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());
		ZPostStageState(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MOSS_EQUIPMENT);

		MPicture* pPicture = 0;
		pPicture = (MPicture*)pResource->FindWidget("Equip_StripBottom");
		if (pPicture != NULL)
			pPicture->SetAnimation(0, 1000.0f);
		pPicture = (MPicture*)pResource->FindWidget("Equip_StripTop");
		if (pPicture != NULL)
			pPicture->SetAnimation(1, 1000.0f);

		pWidget = pResource->FindWidget("Equip");
		if (pWidget != NULL)
			pWidget->Enable(false);
		pWidget = pResource->FindWidget("SendAccountItemBtn");
		if (pWidget != NULL)
			pWidget->Enable(false);
		pWidget = pResource->FindWidget("BringAccountItemBtn");
		if (pWidget != NULL)
			pWidget->Enable(false);
		pWidget = pResource->FindWidget("BringAccountSpendableItemConfirmOpen");
		if (pWidget != NULL)
			pWidget->Enable(false);

		MTextArea* pTextArea = (MTextArea*)pResource->FindWidget("Equip_ItemDescription");
		if (pTextArea)	pTextArea->SetText("");

		GetShopEquipInterface()->DrawCharInfoText();
	}
	else
	{
		ZPostStageState(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MOSS_NONREADY);

		MWidget* pWidget = pResource->FindWidget("Equipment");
		if (pWidget != NULL)
			pWidget->Show(false);

		if (ZApplication::GetGameInterface()->GetState() == GUNZ_STAGE)
			ShowWidget("Stage", true);
		else
			ShowWidget("Lobby", true);
		GetShopEquipInterface()->DrawCharInfoText();

		GetItemThumbnailMgr()->UnloadTextureTemporarily();
	}
}

void ZGameInterface::ShowShopDialog(bool bShow)
{
	if (bShow)
	{
		ShowWidget("Lobby", false);
		ShowWidget("Stage", false);
		ShowWidget("Equipment", false);

		ZGetShop()->Create();
		ZGetShop()->Serialize();

		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

		MPicture* pPicture = 0;
		pPicture = (MPicture*)pResource->FindWidget("Shop_StripBottom");
		if (pPicture != NULL)
			pPicture->SetAnimation(0, 1000.0f);
		pPicture = (MPicture*)pResource->FindWidget("Shop_StripTop");
		if (pPicture != NULL)
			pPicture->SetAnimation(1, 1000.0f);

		MTextArea* pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Shop_ItemDescription");
		if (pTextArea)	pTextArea->SetText("");

		char buf[256];
		MLabel* pLabel = (MLabel*)pResource->FindWidget("Shop_Message");
		sprintf(buf, "%s > %s > %s", ZGetGameClient()->GetServerName(), ZMsg(MSG_WORD_LOBBY), ZMsg(MSG_WORD_SHOP));
		if (pLabel)
			pLabel->SetText(buf);

		MWidget* pWidget = pResource->FindWidget("Shop");
		if (pWidget != NULL)
			pWidget->Show(true, true);
		pWidget = pResource->FindWidget("BuyConfirmCaller");
		if (pWidget != NULL)
			pWidget->Enable(false);
		pWidget = pResource->FindWidget("SellConfirmCaller");
		if (pWidget != NULL)
			pWidget->Enable(false);

		GetShopEquipInterface()->SelectShopTab(0);

		ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());

		GetShopEquipInterface()->DrawCharInfoText();
	}
	else
	{
		ZPostStageState(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MOSS_NONREADY);

		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		MWidget* pWidget = pResource->FindWidget("Shop");
		if (pWidget != NULL) pWidget->Show(false);

		ShowWidget("Lobby", true);

		GetShopEquipInterface()->DrawCharInfoText();

		GetItemThumbnailMgr()->UnloadTextureTemporarily();
	}
}

void ZGameInterface::EnableCharSelectionInterface(bool bEnable)
{
	MWidget* pWidget;

	pWidget = m_IDLResource.FindWidget("CS_SelectChar");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_DeleteChar");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_CreateChar");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_Prev");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_Name_Pre");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_Name_Next");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_Name");
	if (pWidget) pWidget->Enable(bEnable);

	if (!bEnable)
	{
		for (int i = 0; i < MAX_CHAR_COUNT; i++)
		{
			char szName[256];
			sprintf(szName, "CharSel_SelectBtn%d", i);
			pWidget = m_IDLResource.FindWidget(szName);
			if (pWidget)
				pWidget->Show(false);
		}
	}

	if (bEnable && (ZCharacterSelectView::GetNumOfCharacter() == 0))
	{
		pWidget = m_IDLResource.FindWidget("CS_SelectChar");
		if (pWidget) pWidget->Enable(false);
		pWidget = m_IDLResource.FindWidget("CS_DeleteChar");
		if (pWidget) pWidget->Enable(false);
	}

	if (bEnable && (ZCharacterSelectView::GetNumOfCharacter() >= MAX_CHAR_COUNT))
	{
		pWidget = m_IDLResource.FindWidget("CS_CreateChar");
		if (pWidget) pWidget->Enable(false);
	}
}

void ZGameInterface::EnableLobbyInterface(bool bEnable)
{
	EnableWidget("LobbyOptionFrame", bEnable);
	EnableWidget("Lobby_Charviewer_info", bEnable);
	EnableWidget("StageJoin", bEnable);
	EnableWidget("StageCreateFrameCaller", bEnable);
	EnableWidget("ShopCaller", bEnable);
	EnableWidget("EquipmentCaller", bEnable);
	EnableWidget("ReplayCaller", bEnable);
	EnableWidget("CharSelectionCaller", bEnable);
	EnableWidget("Logout", bEnable);
	EnableWidget("QuickJoin", bEnable);
	EnableWidget("QuickJoin2", bEnable);
	EnableWidget("ChannelListFrameCaller", bEnable);
	EnableWidget("StageList", bEnable);
	EnableWidget("Lobby_StageList", bEnable);
	EnableWidget("LobbyChannelPlayerList", bEnable);
	EnableWidget("ChannelChattingOutput", bEnable);
	EnableWidget("ChannelChattingInput", bEnable);

	if (bEnable)
	{
		MMatchServerMode nCurrentServerMode = ZGetGameClient()->GetServerMode();
		MCHANNEL_TYPE nCurrentChannelType = ZGetGameClient()->GetChannelType();
		bool bClanBattleUI = (nCurrentServerMode == MSM_CLAN) && (nCurrentChannelType == MCHANNEL_TYPE_CLAN);
		ZGetGameInterface()->InitClanLobbyUI(bClanBattleUI);
	}

	ZApplication::GetStageInterface()->SetEnableWidgetByRelayMap(!bEnable);
}

void ZGameInterface::EnableStageInterface(bool bEnable)
{
	EnableWidget("Stage_Charviewer_info", bEnable);
	EnableWidget("StagePlayerNameInput_combo", bEnable);
	EnableWidget("GameStart", bEnable);
	MButton* pButton = (MButton*)m_IDLResource.FindWidget("StageReady");
	if (pButton)
	{
		pButton->Enable(bEnable);
		pButton->SetCheck(false);
	}
	EnableWidget("ForcedEntryToGame", bEnable);
	EnableWidget("ForcedEntryToGame2", bEnable);
	EnableWidget("StageTeamBlue", bEnable);
	EnableWidget("StageTeamBlue2", bEnable);
	EnableWidget("StageTeamRed", bEnable);
	EnableWidget("StageTeamRed2", bEnable);
	EnableWidget("ShopCaller", bEnable);
	EnableWidget("EquipmentCaller", bEnable);
	EnableWidget("StageSettingCaller", bEnable);
	EnableWidget("StageObserverBtn", bEnable);
	EnableWidget("Lobby_StageExit", bEnable);

	EnableWidget("MapSelection", bEnable);

	if (ZGetGameClient()->AmIStageMaster() == true)
		EnableWidget("StageType", true);
	else
		EnableWidget("StageType", false);

	EnableWidget("StageMaxPlayer", bEnable);
	EnableWidget("StageRoundCount", bEnable);

	EnableWidget("StagePlayerList_", bEnable);
}

void ZGameInterface::SetRoomNoLight(int d)
{
	char szBuffer[64];
	sprintf(szBuffer, "Lobby_RoomNo%d", d);
	MButton* pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szBuffer);
	if (pButton)
		pButton->SetCheck(true);
}

void ZGameInterface::ShowPrivateStageJoinFrame(const char* szStageName)
{
	MEdit* pStageNameEdit = (MEdit*)m_IDLResource.FindWidget("PrivateStageName");
	if (pStageNameEdit)
	{
		pStageNameEdit->SetText(szStageName);
		pStageNameEdit->Enable(false);
	}
	MEdit* pPassEdit = (MEdit*)m_IDLResource.FindWidget("PrivateStagePassword");
	if (pPassEdit != NULL)
	{
		pPassEdit->SetMaxLength(STAGEPASSWD_LENGTH);
		pPassEdit->SetText("");
		pPassEdit->SetFocus();
	}

	MWidget* pPrivateStageJoinFrame = m_IDLResource.FindWidget("PrivateStageJoinFrame");
	if (pPrivateStageJoinFrame)
	{
		pPrivateStageJoinFrame->Show(true, true);
	}
}

void ZGameInterface::LeaveBattle()
{
	ShowMenu(false);
	m_bGameFinishLeaveBattle = false;

	ZPostStageLeaveBattle(ZGetGameClient()->GetPlayerUID(), m_bGameFinishLeaveBattle);
	if (m_bLeaveStageReserved) {
		ZPostStageLeave(ZGetGameClient()->GetPlayerUID());
		ZApplication::GetGameInterface()->SetState(GUNZ_LOBBY);
	}
	else {
		ZApplication::GetGameInterface()->SetState(GUNZ_STAGE);
	}

	ZGetGameInterface()->SetCursorEnable(true);
	m_bLeaveBattleReserved = false;
	m_bLeaveStageReserved = false;
}

void ZGameInterface::ReserveLeaveStage()
{
	m_bLeaveStageReserved = true;
	ReserveLeaveBattle();
}

void ZGameInterface::ReserveLeaveBattle()
{
	bool bLeaveImmediately = false;
#ifndef _PUBLISH
	bLeaveImmediately = true;
#endif

	if (!m_pGame) return;

	if (m_pCombatInterface->GetObserver()->IsVisible())
		bLeaveImmediately = true;

	if (bLeaveImmediately)
	{
		LeaveBattle();
		return;
	}

	m_bLeaveBattleReserved = true;
	m_dwLeaveBattleTime = timeGetTime() + 5000;
}

void ZGameInterface::ShowMenu(bool bEnable)
{
	if (!GetCombatInterface()->IsShowResult())
		m_CombatMenu.ShowModal(bEnable);
	ZGetGameInterface()->SetCursorEnable(bEnable);
}

bool ZGameInterface::IsMenuVisible()
{
	return m_CombatMenu.IsVisible();
}

void ZGameInterface::Show112Dialog(bool bShow)
{
	MWidget* pWidget = m_IDLResource.FindWidget("112Confirm");
	if (!pWidget)
		return;

	if (pWidget->IsVisible() == bShow)
		return;

	pWidget->Show(bShow, true);
	pWidget->SetFocus();

	if (!bShow)
		return;

	MComboBox* pCombo1 = (MComboBox*)m_IDLResource.FindWidget("112_ConfirmID");
	MComboBox* pCombo2 = (MComboBox*)m_IDLResource.FindWidget("112_ConfirmReason");

	if (!pCombo1 || !pCombo2)
		return;

	pCombo1->RemoveAll();
	pCombo2->SetSelIndex(0);

	switch (m_nState)
	{
	case GUNZ_LOBBY:
	{
		ZPlayerListBox* pPlayerListBox = (ZPlayerListBox*)m_IDLResource.FindWidget("LobbyChannelPlayerList");
		if (pPlayerListBox)
		{
			for (int i = 0; i < pPlayerListBox->GetCount(); i++)
				pCombo1->Add(pPlayerListBox->GetPlayerName(i));
		}
	}
	break;

	case GUNZ_STAGE:
	{
		ZPlayerListBox* pPlayerListBox = (ZPlayerListBox*)m_IDLResource.FindWidget("StagePlayerList_");
		if (pPlayerListBox)
		{
			for (int i = 0; i < pPlayerListBox->GetCount(); i++)
				pCombo1->Add(pPlayerListBox->GetPlayerName(i));
		}
	}
	break;

	case GUNZ_GAME:
	{
		for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin(); itor != ZGetCharacterManager()->end(); itor++)
			pCombo1->Add((*itor).second->GetUserName());
	}
	break;
	}

	pCombo1->SetSelIndex(0);
}

void ZGameInterface::RequestQuickJoin()
{
	MTD_QuickJoinParam	quick_join_param;

	quick_join_param.nMapEnum = 0xFFFFFFFF;

	quick_join_param.nModeEnum = 0;
	SetBitSet(quick_join_param.nModeEnum, MMATCH_GAMETYPE_DEATHMATCH_SOLO);
	SetBitSet(quick_join_param.nModeEnum, MMATCH_GAMETYPE_DEATHMATCH_TEAM);
	SetBitSet(quick_join_param.nModeEnum, MMATCH_GAMETYPE_ASSASSINATE);

	ZPostRequestQuickJoin(ZGetGameClient()->GetPlayerUID(), &quick_join_param);
}

void ZGameInterface::InitClanLobbyUI(bool bClanBattleEnable)
{
	OnArrangedTeamGameUI(false);

	MWidget* pWidget;

	pWidget = m_IDLResource.FindWidget("StageJoin");
	if (pWidget) pWidget->Show(!bClanBattleEnable);
	pWidget = m_IDLResource.FindWidget("StageCreateFrameCaller");
	if (pWidget) pWidget->Show(!bClanBattleEnable);

	pWidget = m_IDLResource.FindWidget("ArrangedTeamGame");
	if (pWidget) pWidget->Show(bClanBattleEnable);

	m_CombatMenu.EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, !bClanBattleEnable);
	pWidget = m_IDLResource.FindWidget("QuickJoin");
	if (pWidget) pWidget->Show(!bClanBattleEnable);

	pWidget = m_IDLResource.FindWidget("QuickJoin2");
	if (pWidget) pWidget->Show(!bClanBattleEnable);

	bool bClanServer = ZGetGameClient()->GetServerMode() == MSM_CLAN;

	pWidget = m_IDLResource.FindWidget("PrivateChannelInput");
	if (pWidget) pWidget->Show(bClanServer);

	pWidget = m_IDLResource.FindWidget("PrivateChannelEnter");
	if (pWidget) pWidget->Show(bClanServer);

	pWidget = m_IDLResource.FindWidget("Lobby_ClanInfoBG");
	if (pWidget) pWidget->Show(bClanBattleEnable);

	pWidget = m_IDLResource.FindWidget("Lobby_ClanList");
	if (pWidget) pWidget->Show(bClanBattleEnable);

	pWidget = m_IDLResource.FindWidget("RequestLadderRejoin");
	if (pWidget) pWidget->Show(bClanBattleEnable);

	pWidget = m_IDLResource.FindWidget("PlayerWarsMapVoteWidget");
	if (pWidget) pWidget->Show(bClanBattleEnable);
}

void ZGameInterface::InitDuelTournamentLobbyUI(bool bEnableDuelTournamentUI)
{
	MWidget* pWidget;

	pWidget = m_IDLResource.FindWidget("Lobby_RoomListBG");
	if (pWidget) {
		if (!bEnableDuelTournamentUI) ((MPicture*)pWidget)->SetBitmap(m_pRoomListFrame);
		else ((MPicture*)pWidget)->SetBitmap(m_pDuelTournamentLobbyFrame);
	}

	pWidget = m_IDLResource.FindWidget("StageBeforeBtn");
	if (pWidget) pWidget->Show(!bEnableDuelTournamentUI);
	pWidget = m_IDLResource.FindWidget("StageAfterBtn");
	if (pWidget) pWidget->Show(!bEnableDuelTournamentUI);
	pWidget = m_IDLResource.FindWidget("Lobby_RoomNo1");
	if (pWidget) pWidget->Show(!bEnableDuelTournamentUI);
	pWidget = m_IDLResource.FindWidget("Lobby_RoomNo2");
	if (pWidget) pWidget->Show(!bEnableDuelTournamentUI);
	pWidget = m_IDLResource.FindWidget("Lobby_RoomNo3");
	if (pWidget) pWidget->Show(!bEnableDuelTournamentUI);
	pWidget = m_IDLResource.FindWidget("Lobby_RoomNo4");
	if (pWidget) pWidget->Show(!bEnableDuelTournamentUI);
	pWidget = m_IDLResource.FindWidget("Lobby_RoomNo5");
	if (pWidget) pWidget->Show(!bEnableDuelTournamentUI);
	pWidget = m_IDLResource.FindWidget("Lobby_RoomNo6");
	if (pWidget) pWidget->Show(!bEnableDuelTournamentUI);

	pWidget = m_IDLResource.FindWidget("StageJoin");
	if (pWidget) pWidget->Show(!bEnableDuelTournamentUI);
	pWidget = m_IDLResource.FindWidget("StageCreateFrameCaller");
	if (pWidget) pWidget->Show(!bEnableDuelTournamentUI);
	pWidget = m_IDLResource.FindWidget("QuickJoin");
	if (pWidget) pWidget->Show(!bEnableDuelTournamentUI);

	pWidget = m_IDLResource.FindWidget("DuelTournamentGame");
	if (pWidget) pWidget->Show(bEnableDuelTournamentUI);

#ifdef _PUBLISH
	pWidget = m_IDLResource.FindWidget("DuelTournamentGame_2Test");
	if (pWidget) pWidget->Show(false);
	pWidget = m_IDLResource.FindWidget("DuelTournamentGame_4Test");
	if (pWidget) pWidget->Show(false);
#else
	pWidget = m_IDLResource.FindWidget("DuelTournamentGame_2Test");
	if (pWidget) pWidget->Show(bEnableDuelTournamentUI);
	pWidget = m_IDLResource.FindWidget("DuelTournamentGame_4Test");
	if (pWidget) pWidget->Show(bEnableDuelTournamentUI);
#endif

	pWidget = m_IDLResource.FindWidget("Lobby_DuelTournamentInfoBG");
	if (pWidget) pWidget->Show(bEnableDuelTournamentUI);
	pWidget = m_IDLResource.FindWidget("Lobby_DuelTournamentRankingList");
	if (pWidget) pWidget->Show(bEnableDuelTournamentUI);

	if (bEnableDuelTournamentUI) {
		pWidget = m_IDLResource.FindWidget("Lobby_DuelTournamentInfoName");
		if (pWidget) pWidget->SetText(ZGetMyInfo()->GetCharName());
	}

	pWidget = m_IDLResource.FindWidget("Lobby_DuelTournamentRankingListLabel");
	if (pWidget) pWidget->Show(bEnableDuelTournamentUI);

	pWidget = m_IDLResource.FindWidget("Lobby_DuelTournamentNeedPointNextRank");
	if (pWidget) pWidget->Show(bEnableDuelTournamentUI);
}

void ZGameInterface::InitChannelFrame(MCHANNEL_TYPE nChannelType)
{
	MWidget* pWidget;

	pWidget = m_IDLResource.FindWidget("PrivateChannelInput");
	if (pWidget) pWidget->Show(nChannelType == MCHANNEL_TYPE_USER);
	pWidget = m_IDLResource.FindWidget("PrivateChannelEnter");
	if (pWidget) pWidget->Show(nChannelType == MCHANNEL_TYPE_USER);
	pWidget = m_IDLResource.FindWidget("MyClanChannel");
	if (pWidget) pWidget->Show(nChannelType == MCHANNEL_TYPE_CLAN);

	MListBox* pListBox = (MListBox*)m_IDLResource.FindWidget("ChannelList");
	if (pListBox) pListBox->RemoveAll();
}

void ZGameInterface::InitLadderUI(bool bLadderEnable)
{
	OnArrangedTeamGameUI(false);

	MWidget* pWidget;

	pWidget = m_IDLResource.FindWidget("StageJoin");
	if (pWidget) pWidget->Show(!bLadderEnable);
	pWidget = m_IDLResource.FindWidget("StageCreateFrameCaller");
	if (pWidget) pWidget->Show(!bLadderEnable);

	pWidget = m_IDLResource.FindWidget("ArrangedTeamGame");
	if (pWidget) pWidget->Show(bLadderEnable);

	m_CombatMenu.EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, !bLadderEnable);
	bool bLadderServer =
		ZGetGameClient()->GetServerMode() == MSM_CLAN ||
		ZGetGameClient()->GetServerMode() == MSM_LADDER ||
		ZGetGameClient()->GetServerMode() == MSM_EVENT;

	pWidget = m_IDLResource.FindWidget("PrivateChannelInput");
	if (pWidget) pWidget->Show(bLadderServer);

	pWidget = m_IDLResource.FindWidget("PrivateChannelEnter");
	if (pWidget) pWidget->Show(bLadderServer);
}

void ZGameInterface::OnPlayerWarsShowerOpen(bool bShowMe)
{
	MWidget* pWidget;
	pWidget = m_IDLResource.FindWidget("PlayerWarsMapVote");

	if (pWidget)
		pWidget->Show(bShowMe);
}

void ZGameInterface::OnPlayerWarsShower(bool bShowMe)
{
	MWidget* pWidget;
	pWidget = m_IDLResource.FindWidget("PlayerWarsMapVote");

	if (pWidget)
		pWidget->Show(bShowMe);
}

void ZGameInterface::OnArrangedTeamGameUI(bool bFinding, bool isvote)
{
	MWidget* pWidget;

	if (isvote == true)
	{
		pWidget = m_IDLResource.FindWidget("ArrangedTeamGame");
		if (pWidget) pWidget->Show(false);

		pWidget = m_IDLResource.FindWidget("LobbyFindClanTeam");
		if (pWidget != NULL) pWidget->Show(false);

		pWidget = m_IDLResource.FindWidget("PlayerWarsMapVote");
		if (pWidget)
		{
			pWidget->Show(bFinding);
			pWidget->SetText("Clan War Map Voter");
		}
	}
	else
	{
		pWidget = m_IDLResource.FindWidget("PlayerWarsMapVote");
		if (pWidget) pWidget->Show(false);

		pWidget = m_IDLResource.FindWidget("ArrangedTeamGame");
		if (pWidget) pWidget->Show(!bFinding);

		pWidget = m_IDLResource.FindWidget("LobbyFindClanTeam");
		if (pWidget != NULL) pWidget->Show(bFinding);
	}

#define SAFE_ENABLE(x,b) { pWidget= m_IDLResource.FindWidget( x ); if(pWidget) pWidget->Enable(!b); }

	SAFE_ENABLE("LobbyChannelPlayerList", bFinding);
	SAFE_ENABLE("ShopCaller", bFinding);
	SAFE_ENABLE("EquipmentCaller", bFinding);
	SAFE_ENABLE("ChannelListFrameCaller", bFinding);
	SAFE_ENABLE("LobbyOptionFrame", bFinding);
	SAFE_ENABLE("Logout", bFinding);
	SAFE_ENABLE("ReplayCaller", bFinding);
	SAFE_ENABLE("CharSelectionCaller", bFinding);
	SAFE_ENABLE("QuickJoin", bFinding);
	SAFE_ENABLE("QuickJoin2", bFinding);

	SAFE_ENABLE("RequestLadderRejoin", bFinding);

	m_bWaitingArrangedGame = bFinding;
}

void ZGameInterface::OnDuelTournamentGameUI(bool bWaiting)
{
	MWidget* pWidget;

	pWidget = m_IDLResource.FindWidget("DuelTournamentWaitMatchDialog");
	if (pWidget) pWidget->Show(bWaiting);

	SAFE_ENABLE("LobbyChannelPlayerList", bWaiting);
	SAFE_ENABLE("ShopCaller", bWaiting);
	SAFE_ENABLE("EquipmentCaller", bWaiting);
	SAFE_ENABLE("ChannelListFrameCaller", bWaiting);
	SAFE_ENABLE("LobbyOptionFrame", bWaiting);
	SAFE_ENABLE("Logout", bWaiting);
	SAFE_ENABLE("ReplayCaller", bWaiting);
	SAFE_ENABLE("CharSelectionCaller", bWaiting);
	SAFE_ENABLE("DuelTournamentGame", bWaiting);
	SAFE_ENABLE("DuelTournamentGame_2Test", bWaiting);
	SAFE_ENABLE("DuelTournamentGame_4Test", bWaiting);
}

bool ZGameInterface::IsReadyToPropose()
{
	if (GetState() != GUNZ_LOBBY)
		return false;

	if (m_bWaitingArrangedGame)
		return false;

	if (GetLatestExclusive() != NULL)
		return false;

	if (m_pMsgBox->IsVisible())
		return false;

	return true;
}

bool ZGameInterface::IsMiniMapEnable()
{
	return GetCamera()->GetLookMode() == ZCAMERA_MINIMAP;
}

bool ZGameInterface::OpenMiniMap()
{
	if (!m_pMiniMap) {
		m_pMiniMap = new ZMiniMap;
		if (!m_pMiniMap->Create(ZGetGameClient()->GetMatchStageSetting()->GetMapName()))
		{
			SAFE_DELETE(m_pMiniMap);
			return false;
		}
	}

	return true;
}

class ReplayListBoxItem : public MListItem
{
protected:
	char		m_szName[_MAX_PATH];
	char		m_szVersion[10];

public:
	ReplayListBoxItem(const char* szName, const char* szVersion)
	{
		strcpy(m_szName, szName);
		strcpy(m_szVersion, szVersion);
	}

	virtual const char* GetString(void)
	{
		return m_szName;
	}
	virtual const char* GetString(int i)
	{
		if (i == 0)
			return m_szName;
		else if (i == 1)
			return m_szVersion;

		return NULL;
	}
	virtual MBitmap* GetBitmap(int i)
	{
		return NULL;
	}
};

void ZGameInterface::ShowReplayDialog(bool bShow)
{
	if (bShow)
	{
		MWidget* pWidget;
		pWidget = (MWidget*)m_IDLResource.FindWidget("Replay");
		if (pWidget)
			pWidget->Show(true, true);

		pWidget = (MWidget*)m_IDLResource.FindWidget("Replay_View");
		if (pWidget)
			pWidget->Enable(false);

		MListBox* pListBox = (MListBox*)m_IDLResource.FindWidget("Replay_FileList");
		if (pListBox)
		{
			pListBox->RemoveAll();

			TCHAR szPath[MAX_PATH];
			if (GetMyDocumentsPath(szPath))
			{
				strcat(szPath, GUNZ_FOLDER);
				strcat(szPath, REPLAY_FOLDER);
				CreatePath(szPath);
			}
			TCHAR szFullPath[MAX_PATH];
			strcpy(szFullPath, szPath);

			strcat(szPath, "/*.gzr");

			struct _finddata_t c_file;
			long hFile;
			char szName[_MAX_PATH];
			char szFullPathName[_MAX_PATH];
			if ((hFile = _findfirst(szPath, &c_file)) != -1L)
			{
				do
				{
					strcpy(szName, c_file.name);

					strcpy(szFullPathName, szFullPath);
					strcat(szFullPathName, "/");
					strcat(szFullPathName, szName);

					DWORD dwFileVersion = 0;
					char szVersion[10];
					int nRead;
					DWORD header;
					ZFile* file = zfopen(szFullPathName);
					if (file)
					{
						nRead = zfread(&header, sizeof(header), 1, file);
						if ((nRead != 0) && (header == GUNZ_REC_FILE_ID))
						{
							zfread(&dwFileVersion, sizeof(dwFileVersion), 1, file);
							sprintf(szVersion, "v%d.0", dwFileVersion);
						}
						else
							strcpy(szVersion, "--");

						zfclose(file);
					}
					else
						strcpy(szVersion, "--");

					pListBox->Add(new ReplayListBoxItem(szName, szVersion));
				} while (_findnext(hFile, &c_file) == 0);

				_findclose(hFile);
			}

			pListBox->Sort();
		}
	}
	else
	{
		ShowWidget("Replay", false);
	}
}

void ZGameInterface::ViewReplay(void)
{
	ShowReplayDialog(false);

	MListBox* pListBox = (MListBox*)m_IDLResource.FindWidget("Replay_FileList");
	if (!pListBox)
		return;

	if (pListBox->GetSelItemString() == NULL)
		return;

	TCHAR szName[MAX_PATH];
	if (GetMyDocumentsPath(szName))
	{
		strcat(szName, GUNZ_FOLDER);
		strcat(szName, REPLAY_FOLDER);
		strcat(szName, "/");
		strcat(szName, pListBox->GetSelItemString());
	}

	m_bOnEndOfReplay = true;
	m_nLevelPercentCache = ZGetMyInfo()->GetLevelPercent();

	if (!CreateReplayGame(szName))
	{
		ZApplication::GetGameInterface()->ShowMessage("Can't Open Replay File");
	}

	m_CombatMenu.EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, false);
}

#ifdef _QUEST_ITEM
void ZGameInterface::OnResponseCharacterItemList_QuestItem(MTD_QuestItemNode* pQuestItemNode, int nQuestItemCount)
{
	if (0 == pQuestItemNode)
		return;

	ZGetMyInfo()->GetItemList()->SetQuestItemsAll(pQuestItemNode, nQuestItemCount);

	ZApplication::GetStageInterface()->SerializeSacrificeItemListBox();
	ZGetMyInfo()->GetItemList()->Serialize();

	GetShopEquipInterface()->DrawCharInfoText();
}

void ZGameInterface::OnResponseBuyQuestItem(const int nResult, const int nBP)
{
	if (MOK == nResult)
	{
		ZApplication::GetGameInterface()->ShowMessage(MSG_GAME_BUYITEM);

		ZGetMyInfo()->SetBP(nBP);
	}
	else if (MERR_TOO_MANY_ITEM == nResult)
	{
		ZApplication::GetGameInterface()->ShowErrorMessage(nResult);
	}
	else if (MERR_TOO_EXPENSIVE_BOUNTY == nResult)
	{
		ZApplication::GetGameInterface()->ShowErrorMessage(nResult);
	}
	else
	{
		mlog("ZGameInterface::OnCommand::MC_MATCH_RESPONSE_BUY_QUEST_ITEM - 정의되지 않은 결과처리.\n");
		ASSERT(0);
	}

	GetShopEquipInterface()->DrawCharInfoText();
}

void ZGameInterface::OnResponseSellQuestItem(const int nResult, const int nBP)
{
	if (MOK == nResult)
	{
		ZApplication::GetGameInterface()->ShowMessage(MSG_GAME_SELLITEM);

		ZGetMyInfo()->SetBP(nBP);
	}
	else
	{
	}

	GetShopEquipInterface()->DrawCharInfoText();
}
#endif

MBitmap* ZGameInterface::GetQuestItemIcon(int nItemID, bool bSmallIcon)
{
	char szFileName[64] = "";
	switch (nItemID)
	{
	case 200001:
	case 200002:
	case 200003:
	case 200004:
		strcpy(szFileName, "slot_icon_page");
		break;

	case 200005:
	case 200006:
	case 200007:
	case 200008:
	case 200009:
	case 200010:
		strcpy(szFileName, "slot_icon_skull");
		break;

	case 200011:
	case 200012:
	case 200013:
		strcpy(szFileName, "slot_icon_fresh");
		break;

	case 200014:
	case 200015:
	case 200016:
	case 200017:
		strcpy(szFileName, "slot_icon_ring");
		break;

	case 200018:
		strcpy(szFileName, "slot_icon_neck");
		break;

	case 200019:
	case 200020:
	case 200021:
	case 200022:
	case 200023:
	case 200024:
	case 200025:
	case 200026:
	case 200027:
		strcpy(szFileName, "slot_icon_doll");
		break;

	case 200028:
	case 200029:
	case 200030:
		strcpy(szFileName, "slot_icon_book");
		break;

	case 200031:
	case 200032:
	case 200033:
	case 200034:
	case 200035:
	case 200036:
	case 200037:
		strcpy(szFileName, "slot_icon_object");
		break;

	case 200038:
	case 200039:
	case 200040:
	case 200041:
	case 200042:
	case 200043:
	case 200044:
		strcpy(szFileName, "slot_icon_qsword");
		break;
	}

	strcat(szFileName, ".tga");

	return MBitmapManager::Get(szFileName);
}

void ZGameInterface::OnResponseServerStatusInfoList(const int nListCount, void* pBlob)
{
	ZServerView* pServerList = (ZServerView*)m_IDLResource.FindWidget("SelectedServer");
	if (!pServerList)
		return;

	int nCurrSel = pServerList->GetCurrSel2();
	pServerList->ClearServerList();

	if ((0 < nListCount) && (0 != pBlob))
	{
		for (int i = 0; i < nListCount; ++i)
		{
			MTD_ServerStatusInfo* pss = (MTD_ServerStatusInfo*)MGetBlobArrayElement(pBlob, i);
			if (0 == pss)
			{
				mlog("ZGameInterface::OnResponseServerStatusInfoList - %d번째에서 NULL포인터 발생.", i);
				continue;
			}

			in_addr inaddr;
			inaddr.S_un.S_addr = pss->m_dwIP;

			char* pszAddr = inet_ntoa(inaddr);
			char strAddrBuf[32];
			strcpy(strAddrBuf, pszAddr);

			char szServName[128] = { 0, };
			if (pss->m_nType == 1)
				sprintf(szServName, "%s %d", ZMsg(MSG_SERVER_NORMAL), pss->m_nServerID);
			else if (pss->m_nType == 2)
				sprintf(szServName, "%s %d", ZMsg(MSG_SERVER_MATCH), pss->m_nServerID);
			else if (pss->m_nType == 3)
				sprintf(szServName, "%s %d", ZMsg(MSG_SERVER_CLAN), pss->m_nServerID);
			else if (pss->m_nType == 4)
				sprintf(szServName, "%s %d", ZMsg(MSG_SERVER_QUEST), pss->m_nServerID);
			else if (pss->m_nType == 5)
				sprintf(szServName, "%s %d", ZMsg(MSG_SERVER_EVENT), pss->m_nServerID);
			else if (pss->m_nType == 6)
				sprintf(szServName, "Test Server %d", pss->m_nServerID);
			else if (pss->m_nType == 7)
				sprintf(szServName, "%s %d", ZMsg(MSG_SERVER_DEBUG), pss->m_nServerID);
			else
				continue;

			int nRow = 0;
			int nCol = 0;
			char* pszAgentIP = "";

			sprintf(szServName, "%s", pss->m_szServerName);

			if ((pss->m_nServerID % 50) >= 25)
				nRow = 1;

#ifdef LOCALE_NHNUSAA
			if (pss->m_nServerID > 200)
			{
				nCol = 1;
			}

			sockaddr_in Addr;
			Addr.sin_addr.S_un.S_addr = pss->m_dwAgentIP;
			pszAgentIP = inet_ntoa(Addr.sin_addr);

#else if defined(LOCALE_KOREA) || defined(_DEBUG)
			if (pss->m_nServerID < 51) { nCol = 0; nRow = 0; }
			else if (50 < pss->m_nServerID && pss->m_nServerID < 101) { nCol = 1; nRow = 0; }
			else if (100 < pss->m_nServerID && pss->m_nServerID < 151) { nCol = 0; nRow = 1; }
			else if (150 < pss->m_nServerID && pss->m_nServerID < 201) { nCol = 1; nRow = 1; }
			else if (200 < pss->m_nServerID && pss->m_nServerID < 251) { nCol = 2; nRow = 1; }
#endif
			pServerList->AddServer(szServName, strAddrBuf, pss->m_nPort, pss->m_nType, pss->m_nCurPlayer, pss->m_nMaxPlayer, pss->m_bIsLive, pszAgentIP, nRow, nCol);

#ifdef _DEBUG
			mlog("ServerList - ID:%d, Name:%s, IP:%s, Port:%d, Type:%d, (%d/%d)\n",
				pss->m_nServerID, szServName, strAddrBuf, pss->m_nPort, pss->m_nType, pss->m_nCurPlayer, pss->m_nMaxPlayer, "");
#endif
		}
		}

#ifdef LOCALE_NHNUSAA
	char szAuthString[NHN_AUTH_LENGTH] = { 0, };
	const int nStrRet = GetAuthString(szAuthString, NHN_AUTH_LENGTH);
	if (0 != nStrRet)
	{
		WriteNHNAuthLog(nStrRet, "GetAuthString");
	}
	else
	{
		((ZNHN_USAAuthInfo*)ZGetLocale()->GetAuthInfo())->SetAuthStr(szAuthString, strlen(szAuthString));
	}
#endif

	pServerList->SetCurrSel(nCurrSel);

	m_dwRefreshTime = timeGetTime() + 10000;

#ifdef LOCALE_NHNUSAA
	GetNHNUSAReport().ReportDisplayServerList();
#endif
}

void ZGameInterface::OnResponseBlockCountryCodeIP(const char* pszBlockCountryCode, const char* pszRoutingURL)
{
	if (0 != pszBlockCountryCode)
	{
		ShowMessage(pszRoutingURL);
	}
}

void ZGameInterface::RequestServerStatusListInfo()
{
	ZLocatorList* pLocatorList;

	if (ZApplication::GetInstance()->IsLaunchTest())
		pLocatorList = m_pTLocatorList;
	else
		pLocatorList = m_pLocatorList;

	if (0 == pLocatorList)
		return;

	if (pLocatorList->GetSize() < 1)
		return;

	MCommand* pCmd = ZNewCmd(MC_REQUEST_SERVER_LIST_INFO);
	if (0 != pCmd)
	{
		const string strIP = pLocatorList->GetIPByPos(m_nLocServ++);
		m_nLocServ %= pLocatorList->GetSize();

		GetGameClient()->SendCommandByUDP(pCmd, const_cast<char*>(strIP.c_str()), LOCATOR_PORT);
		delete pCmd;
	}

	m_dwRefreshTime = timeGetTime() + 1500;
}

void ZGameInterface::OnRequestXTrapSeedKey(unsigned char* pComBuf)
{
#ifdef _XTRAP
	unsigned char szSendBuf[128] = { 0, };
	int nRetValue = XTrap_CS_Step2(pComBuf, szSendBuf, XTRAP_PROTECT_PE | XTRAP_PROTECT_TEXT | XTRAP_PROTECT_EXCEPT_VIRUS);
	ZPostResponseXTrapSeedKey(szSendBuf);
	if (nRetValue != 0)
	{
		char szMsgBuf[500] = {
#ifdef LOCALE_KOREA
		"비정상적인 행위가 감지되었습니다. 게임을 종료합니다.\n"
#else
		"An abnormal behavior is detected. Terminating game.\n"
#endif
		};
		if (ZGetGameClient())
		{
			ZGetGameClient()->Disconnect();
			AfxMessageBox(szMsgBuf);
			mlog("Disconnect() called!\n");
		}
		else
		{
			OnDisconnectMsg(MERR_FIND_HACKER);
			mlog("OnDisconnectMsg() called!\n");
		}
		mlog(szMsgBuf);
		PostQuitMessage(0);
	}
#endif
}

void ZGameInterface::OnDisconnectMsg(const DWORD dwMsgID)
{
	MListBox* pListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("QuestResult_ItemListbox");
	if (0 != pListBox)
	{
		if ((dwMsgID == MERR_FIND_HACKER) || (dwMsgID == MERR_BLOCK_HACKER)
			|| (dwMsgID == MERR_BLOCK_BADUSER) || (dwMsgID == MERR_FIND_INVALIDFILE)
			|| (dwMsgID == MERR_FIND_FLOODING) || (dwMsgID == MERR_BLOCK_BY_ADMIN)
			|| (dwMsgID == MERR_BLOCK_SLEEP_ACCOUNT))
		{
			m_bReservedQuit = true;
			m_dwReservedQuitTimer = timeGetTime() + 5000;
		}

		ShowMessage(ZErrStr(dwMsgID));
	}
}

void ZGameInterface::ShowDisconnectMsg(DWORD errStrID, DWORD delayTime)
{
	m_bReservedQuit = true;
	m_dwReservedQuitTimer = timeGetTime() + delayTime;

	ShowMessage(ZErrStr(errStrID));
}

void ZGameInterface::OnAnnounceDeleteClan(const string& strAnnounce)
{
	char szMsg[128];

	sprintf(szMsg, MGetStringResManager()->GetErrorStr(MERR_CLAN_ANNOUNCE_DELETE), strAnnounce.c_str());
	ShowMessage(szMsg);
}

void ZGameInterface::OnVoiceSound()
{
	DWORD dwCurrTime = timeGetTime();

	if (dwCurrTime < m_dwVoiceTime)
		return;

	m_szCurrVoice[0] = 0;

	if (m_szNextVoice[0] == 0)
		return;

	ZApplication::GetSoundEngine()->PlaySound(m_szNextVoice);
	m_dwVoiceTime = dwCurrTime + m_dwNextVoiceTime;

	strcpy(m_szCurrVoice, m_szNextVoice);
	m_szNextVoice[0] = 0;
	m_dwNextVoiceTime = 0;
}

void ZGameInterface::PlayVoiceSound(char* pszSoundName, DWORD time)
{
	if (!Z_AUDIO_NARRATIONSOUND)
		return;

	if (strcmp(pszSoundName, m_szCurrVoice) != 0)
	{
		sprintf(m_szNextVoice, pszSoundName);
		m_dwNextVoiceTime = time;
	}

	if (timeGetTime() > m_dwVoiceTime)
		OnVoiceSound();
}

void ZGameInterface::OnRequestGameguardAuth(const DWORD dwIndex, const DWORD dwValue1, const DWORD dwValue2, const DWORD dwValue3)
{
#ifdef _GAMEGUARD
	GetZGameguard().SendToCallback(dwIndex, dwValue1, dwValue2, dwValue3);
#endif
}

void ZGameInterface::OnSendGambleItemList(void* pGItemArray, const DWORD dwCount)
{
	ZGetGambleItemDefineMgr().Release();

	MTD_DBGambleItmeNode* pGItem;
	ZGambleItemDefine* pZGItem;

	for (DWORD i = 0; i < dwCount; ++i)
	{
		pGItem = (MTD_DBGambleItmeNode*)MGetBlobArrayElement(pGItemArray, i);
		if (NULL == pGItem)
			return;

		pZGItem = new ZGambleItemDefine(pGItem->nItemID
			, pGItem->szName
			, pGItem->szDesc
			, pGItem->nBuyPrice
			, pGItem->bIsCash);
		if (NULL == pZGItem)
			return;

		if (!ZGetGambleItemDefineMgr().AddGambleItemDefine(pZGItem))
		{
		}
	}
}

void ZGameInterface::SetAgentPing(DWORD nIP, DWORD nTimeStamp)
{
	ZServerView* pServerList = (ZServerView*)m_IDLResource.FindWidget("SelectedServer");
	pServerList->SetAgentPing(nIP, nTimeStamp);
}

void ZGameInterface::MultiplySize(float byIDLWidth, float byIDLHeight, float byCurrWidth, float byCurrHeight)
{
	MWidget::MultiplySize(byIDLWidth, byIDLHeight, byCurrWidth, byCurrHeight);

	if (m_pMsgBox)
		m_pMsgBox->MultiplySize(byIDLWidth, byIDLHeight, byCurrWidth, byCurrHeight);
	if (m_pConfirmMsgBox)
		m_pConfirmMsgBox->MultiplySize(byIDLWidth, byIDLHeight, byCurrWidth, byCurrHeight);
}

void ZGameInterface::SetDuelTournamentCharacterList(MDUELTOURNAMENTTYPE nType, const vector<DTPlayerInfo>& vecDTPlayerInfo)
{
	m_eDuelTournamentType = nType;

	m_vecDTPlayerInfo.clear();
	m_vecDTPlayerInfo = vecDTPlayerInfo;
}

void ZGameInterface::UpdateDuelTournamantMyCharInfoUI()
{
	ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();
	if (!pRes) return;

	const ZGameClient::DTCHARINFO* pCharInfo = GetGameClient()->GetMyDuelTournamentCharInfo();
	if (!pCharInfo) return;

	char szOutput[256];
	sprintf(szOutput, "%d/%d", pCharInfo->wins, pCharInfo->losses);
	ZBmNumLabel* pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_DuelTournamentInfoWinLose");
	if (pNumLabel)
		pNumLabel->SetText(szOutput);
	sprintf(szOutput, "%d", pCharInfo->tournamentPoint);
	pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_DuelTournamentInfoPoints");
	if (pNumLabel)
		pNumLabel->SetText(szOutput);
	sprintf(szOutput, "%d", pCharInfo->winners);
	pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_DuelTournamentInfoWinners");
	if (pNumLabel)
		pNumLabel->SetText(szOutput);

	int ranking = (pCharInfo->ranking == -1) ? 0 : pCharInfo->ranking;
	sprintf(szOutput, "%d", ranking);
	pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_DuelTournamentInfoRanking");
	if (pNumLabel)
		pNumLabel->SetText(szOutput);

	GetDuelTournamentGradeIconFileName(szOutput, pCharInfo->lastWeekGrade);
	MPicture* pPicture = (MPicture*)pRes->FindWidget("Lobby_DuelTournamentInfoEmblem");
	if (pPicture) {
		pPicture->SetBitmap(MBitmapManager::Get(szOutput));

		char sz[32];
		char szTooltip[256];
		sprintf(sz, "%d", pCharInfo->lastWeekGrade);
		ZTransMsg(szTooltip, MSG_LOBBY_DUELTOURNAMENT_RECORD_TOOPTIP_GRADE, 1, sz);
		SetWidgetToolTipText("Lobby_DuelTournamentInfoEmblem", szTooltip, MAM_VCENTER);
	}
}

void ZGameInterface::UpdateDuelTournamantMyCharInfoPreviousUI()
{
	ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();
	if (!pRes) return;

	const ZGameClient::DTCHARINFO* pCharInfo = GetGameClient()->GetMyDuelTournamentCharInfoPrev();
	if (!pCharInfo) return;

	char szOutput[256];
	sprintf(szOutput, "%d/%d", pCharInfo->wins, pCharInfo->losses);
	ZBmNumLabel* pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_DuelTournamentInfoWinLosePrev");
	if (pNumLabel)
	{
		pNumLabel->SetIndexOffset(16);
		pNumLabel->SetText(szOutput);
	}
	sprintf(szOutput, "%d", pCharInfo->tournamentPoint);
	pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_DuelTournamentInfoPointsPrev");
	if (pNumLabel)
	{
		pNumLabel->SetIndexOffset(16);
		pNumLabel->SetText(szOutput);
	}
	sprintf(szOutput, "%d", pCharInfo->winners);
	pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_DuelTournamentInfoWinnersPrev");
	if (pNumLabel)
	{
		pNumLabel->SetIndexOffset(16);
		pNumLabel->SetText(szOutput);
	}

	int ranking = (pCharInfo->ranking == -1) ? 0 : pCharInfo->ranking;
	sprintf(szOutput, "%d", ranking);
	pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_DuelTournamentInfoRankingPrev");
	if (pNumLabel)
	{
		pNumLabel->SetIndexOffset(16);
		pNumLabel->SetText(szOutput);
	}
}

#include "MDrawContext.h"
#define MAX_CHAR_A_LINE		255

int MDrawContext::TextMultiLineChat(MRECT& r, const char* szText, int nLineGap, bool bAutoNextLine, int nIndentation, int nSkipLine, MPOINT* pPositions)
{
	bool bColorSupport = true;

	MBeginProfile(99, "MDrawContext::TextMultiLine");

	int nLine = 0;
	MFont* pFont = GetFont();

	int nLength = strlen(szText);

	int y = r.y;
	char* szCurrent = (char*)szText;
	MPOINT* pCurrentPos = pPositions;
	do {
		MCOLOR prevColor = GetColor();
		SetColor(20, 20, 20); //Edit your color
		char* szCurrentCpy = GetPureText(szCurrent);
		FillRectangle(r.x - 3, y + 2.7f, pFont->GetWidth(szCurrentCpy) + 5, pFont->GetHeight());
		pFont->GetWidth(szCurrentCpy), 200.f * 1.1 / 800.f * (float)MGetWorkspaceWidth();
		SetColor(prevColor);

		int nX = nLine == 0 ? 0 : nIndentation;
		int nOriginalCharCount = MMGetNextLinePos(pFont, szCurrent, r.w - nX, bAutoNextLine, true);

		if (nSkipLine <= nLine)
		{
			int nCharCount = min(nOriginalCharCount, MAX_CHAR_A_LINE);
			char buffer[256];
			if (bColorSupport) {
#define FLUSHPOS(_Pos)		if(pCurrentPos!=NULL){	\
								for(int i=0; buffer[i]!=NULL; i++){	\
									pCurrentPos[i+szCurrent-szText].x = _Pos+pFont->GetWidth(buffer, i);	\
									pCurrentPos[i+szCurrent-szText].y = y;	\
								}	\
							}

#define FLUSH				if(buffer[0]) { Text(r.x+nLastX, y, buffer); FLUSHPOS(r.x+nLastX); nLastX=nX; buffer[0]=0;pcurbuf=buffer; }

				int nLastX = nX;

				buffer[0] = 0;
				char* pcurbuf = buffer;
				for (int i = 0; i < nCharCount; i++) {
					unsigned char c = szCurrent[i], cc = szCurrent[i + 1];

					if (c == '^' && ('0' <= cc) && (cc <= '9'))
					{
						FLUSH;
						SetColor(MCOLOR(MMColorSet[cc - '0']));
						i++;
						continue;
					}

					int w;

					*(pcurbuf++) = c;
					if (c > 127 && i < nCharCount) {
						*(pcurbuf++) = cc;
						w = pFont->GetWidth(szCurrent + i, 2);
						i++;
					}
					else w = pFont->GetWidth(szCurrent + i, 1);

					*pcurbuf = 0;

					nX += w;
				}

				FLUSH;
			}
			else
			{
				strncpy(buffer, szCurrent, nCharCount);
				buffer[nCharCount] = 0;
				Text(r.x + nX, y, buffer);
				FLUSHPOS(r.x + nX);
			}
			y += pFont->GetHeight() + nLineGap;
		}

		szCurrent += nOriginalCharCount;
		nLine++;
		if (y >= r.y + r.h) break;
	} while (szCurrent < szText + nLength);

	MEndProfile(99);
	return nLine - nSkipLine;
}