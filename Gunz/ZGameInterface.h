#ifndef _ZGAMEINTERFACE_H
#define _ZGAMEINTERFACE_H

#include "ZPrerequisites.h"
#include "ZInterface.h"
#include "ZCamera.h"
#include "ZChat.h"
#include "ZQuest.h"
#include "ZSurvival.h"
#include "ZGameType.h"
#include "ZTips.h"
#include "ZCombatMenu.h"
#include "ZMyCharacter.h"
#include "ZBandiCapturer.h"
#include "ZBitmapManager.h"

#define LOGINSTATE_FADEIN				0
#define LOGINSTATE_SHOWLOGINFRAME		1
#define LOGINSTATE_STANDBY				2
#define LOGINSTATE_LOGINCOMPLETE		3
#define LOGINSTATE_FADEOUT				4

class ZLocatorList;
class ZGameInput;
class ZMonsterBookInterface;
class ZShopEquipInterface;

enum ZChangeWeaponType;

class MUserDataListItem : public MDefaultListItem {
	int m_nUserData;
public:
	MUserDataListItem(const char* szText, int nUserData)
		: MDefaultListItem(szText) {
		m_nUserData = nUserData;
	}

	int GetUserData() { return m_nUserData; }
};

class ZGameInterface : public ZInterface {
public:
	GunzState			m_nInitialState;
	bool				m_bTeenVersion;
	bool				m_bViewUI;
	bool				m_bTeamPlay;

	bool				m_bLoginTimeout;
	DWORD				m_dwLoginTimeout;

	MTextAreaLook		m_textAreaLookItemDesc;

	list<MCommand*>	m_listDelayedGameCmd;

protected:
	ZScreenEffectManager* m_pScreenEffectManager;
	ZEffectManager* m_pEffectManager;

	GunzState			m_nPreviousState;

	ZCombatInterface* m_pCombatInterface;
	ZShopEquipInterface* m_pShopEquipInterface;
	ZGameInput* m_pGameInput;
	ZLoading* m_pLoadingInterface;
	ZPlayerMenu* m_pPlayerMenu;

	static ZGameClient* m_spGameClient;
	ZGame* m_pGame;
	ZCamera				m_Camera;
	ZChat				m_Chat;
	ZQuest				m_Quest;
	ZSurvival			m_Survival;
	ZGameTypeManager	m_GameTypeManager;
	ZMiniMap* m_pMiniMap;
	ZTips				m_Tips;

	ZBandiCapturer* m_Capture;

	ZCombatMenu			m_CombatMenu;

	ZMyCharacter* m_pMyCharacter;

	ZMonsterBookInterface* m_pMonsterBookInterface;

	bool				m_bShowInterface;

	bool				m_bCursor;
	LPDIRECT3DSURFACE9	m_pCursorSurface;

	DWORD				m_dwFrameMoveClock;

	ZIDLResource		m_IDLResource;

	GunzState			m_nState;
	bool				m_bLogin;

	bool				m_bLoading;
	bool				m_bWaitingArrangedGame;

	MBitmap* m_pMapThumbnail;

	ZMsgBox* m_pMsgBox;
	ZMsgBox* m_pConfirmMsgBox;
	ZInterfaceBackground* m_pBackground;
	ZCharacterSelectView* m_pCharacterSelectView;

	bool				m_bOnEndOfReplay;
	int					m_nLevelPercentCache;
	unsigned long int	m_nDrawCount;

	bool			m_bReservedWeapon;
	ZChangeWeaponType m_ReservedWeapon;

	bool			m_bLeaveBattleReserved;
	bool			m_bLeaveStageReserved;
	DWORD			m_dwLeaveBattleTime;

	int				m_nLoginState;
	DWORD			m_dwLoginTimer;
	DWORD			m_dwRefreshTime;
	int				m_nLocServ;

	MBitmapR2* m_pRoomListFrame;
	MBitmapR2* m_pDuelTournamentLobbyFrame;
	MBitmapR2* m_pBottomFrame;
	MBitmapR2* m_pClanInfoBg;
	MBitmapR2* m_pDuelTournamentInfoBg;
	MBitmapR2* m_pDuelTournamentRankingLabel;
	MBitmapR2* m_pLoginBG;
	MBitmapR2* m_pLoginPanel;

	ZBitmapManager<int> m_ItemThumbnailMgr;
	ZLocatorList* m_pLocatorList;
	ZLocatorList* m_pTLocatorList;

	DWORD			m_dwTimeCount;
	DWORD			m_dwHourCount;

	DWORD			m_dwVoiceTime;
	char			m_szCurrVoice[256];
	char			m_szNextVoice[256];
	DWORD			m_dwNextVoiceTime;

	int				m_nRetryCount;

	bool			m_bReservedQuit;
	DWORD			m_dwReservedQuitTimer;

	bool			m_bReserveResetApp;

	static bool		m_bSkipGlobalEvent;

	DWORD			m_MyPort;

	DWORD			m_dErrMaxPalyerDelayTime;
	DWORD			m_bErrMaxPalyer;

	bool			m_bGameFinishLeaveBattle;

	vector<DTPlayerInfo> m_vecDTPlayerInfo;
	MDUELTOURNAMENTTYPE m_eDuelTournamentType;

protected:
	static bool		OnGlobalEvent(MEvent* pEvent);
	virtual bool	OnEvent(MEvent* pEvent, MListener* pListener);
	bool			OnDebugEvent(MEvent* pEvent, MListener* pListener);
	virtual bool	OnCommand(MWidget* pWidget, const char* szMessage);
	static bool		OnCommand(MCommand* pCommand);

	bool ResizeWidget(const char* szName, int w, int h);
	bool ResizeWidgetRecursive(MWidget* pWidget, int w, int h);
	void SetListenerWidget(const char* szName, MListener* pListener);

	void UpdateCursorEnable();
	void UpdateDuelTournamentWaitMsgDots();

	bool InitInterface(const char* szSkinName, ZLoadingProgress* pLoadingProgress = NULL);
	bool InitInterfaceListener();
	void FinalInterface();

	void LoadBitmaps(const char* szDir, ZLoadingProgress* pLoadingProgress = NULL);

	void LeaveBattle();

	void OnGreeterCreate(void);
	void OnGreeterDestroy(void);

	void OnLoginCreate(void);
	void OnLoginDestroy(void);

	void OnDirectLoginCreate(void);
	void OnDirectLoginDestroy(void);

	void OnGameOnLoginCreate(void);
	void OnGameOnLoginDestroy(void);

	void OnLobbyCreate(void);
	void OnLobbyDestroy(void);

	void OnStageCreate(void);
	void OnStageDestroy(void);

	void OnCharSelectionCreate(void);
	void OnCharSelectionDestroy(void);

	void OnCharCreationCreate(void);
	void OnCharCreationDestroy(void);

	void OnShutdownState();

#ifdef _BIRDTEST
	void OnBirdTestCreate();
	void OnBirdTestDestroy();
	void OnBirdTestUpdate();
	void OnBirdTestDraw();
	void OnBirdTestCommand(MCommand* pCmd);
#endif

	void OnUpdateGameMessage(void);

	void HideAllWidgets();

	void OnResponseShopItemList(const vector< MTD_ShopItemInfo*>& vShopItemList, const vector<MTD_GambleItemNode*>& vGItemList);
	void OnResponseCharacterItemList(MUID* puidEquipItem
		, MTD_ItemNode* pItemNodes
		, int nItemCount
		, MTD_GambleItemNode* pGItemNodes
		, int nGItemCount);

	void OnSendGambleItemList(void* pGItemArray, const DWORD dwCount);

	void OnDrawStateGame(MDrawContext* pDC);
	void OnDrawStateLogin(MDrawContext* pDC);
	void OnDrawStateLobbyNStage(MDrawContext* pDC);
	void OnDrawStateCharSelection(MDrawContext* pDC);

#ifdef _QUEST_ITEM
	void OnResponseCharacterItemList_QuestItem(MTD_QuestItemNode* pQuestItemNode, int nQuestItemCount);
	void OnResponseBuyQuestItem(const int nResult, const int nBP);
	void OnResponseSellQuestItem(const int nResult, const int nBP);
#endif

	void OnResponseServerStatusInfoList(const int nListCount, void* pBlob);
	void OnResponseBlockCountryCodeIP(const char* pszBlockCountryCode, const char* pszRoutingURL);

	void RequestServerStatusListInfo();

public:
	ZGameInterface(const char* szName = NULL, MWidget* pParent = NULL, MListener* pListener = NULL);
	~ZGameInterface();

	static bool m_sbRemainClientConnectionForResetApp;

	bool OnCreate(ZLoadingProgress* pLoadingProgress);
	void OnDestroy();

	void OnInvalidate();
	void OnRestore();

	bool Update(float fElapsed);
	void OnDraw(MDrawContext* pDC);

	void SetCursorEnable(bool bEnable);
	void OnResetCursor();
	bool IsCursorEnable() { return m_bCursor; }

	bool SetState(GunzState nState);
	GunzState GetState(void) { return m_nState; }

	void UpdateBlueRedTeam(void);

	void ChangeToCharSelection(void);

	bool ChangeInterfaceSkin(const char* szNewSkinName);

	bool ShowWidget(const char* szName, bool bVisible, bool bModal = false);
	void SetTextWidget(const char* szName, const char* szText);
	void EnableWidget(const char* szName, bool bEnable);

	void TestChangeParts(int mode);
	void TestChangePartsAll();
	void TestChangeWeapon(RVisualMesh* pVMesh = NULL);
	void TestToggleCharacter();

	void ChangeParts(int mode);
	void ChangeWeapon(ZChangeWeaponType nType);

	void Reload();

	void RespawnMyCharacter();

	void ReserveLeaveStage();
	void ReserveLeaveBattle();
	void FinishGame(void);
	bool IsLeaveBattleReserved() { return m_bLeaveBattleReserved; }

	void ReserveResetApp(bool b) { m_bReserveResetApp = b; }
	bool IsReservedResetApp() { return m_bReserveResetApp; }

	void SaveScreenShot();

	void ShowMessage(const char* szText, MListener* pCustomListenter = NULL, int nMessageID = 0);
	void ShowConfirmMessage(const char* szText, MListener* pCustomListenter = NULL);
	void ShowMessage(int nMessageID);
	void ShowErrorMessage(int nErrorID);
	void ShowErrorMessage(const char* szErrorMsg, int nErrorID);

	void ShowInterface(bool bShowInterface);
	bool IsShowInterface() { return m_bShowInterface; }

	void SetTeenVersion(bool bt) { m_bTeenVersion = bt; }
	bool GetTeenVersion() { return m_bTeenVersion; }

	void OnCharSelect(void);

	bool OnGameCreate(void);
	void OnGameDestroy(void);
	void OnGameUpdate(float fElapsed);

	void OnArrangedTeamGameUI(bool bFinding, bool isvote = false);
	void OnDuelTournamentGameUI(bool bWaiting);
	void OnPlayerWarsShowerOpen(bool bShowMe);
	void OnPlayerWarsShower(bool bShowMe);
	void InitLobbyUIByChannelType();

	void InitLadderUI(bool bLadderEnable);
	void InitClanLobbyUI(bool bClanBattleEnable);
	void InitDuelTournamentLobbyUI(bool bEnableDuelTournamentUI);
	void InitChannelFrame(MCHANNEL_TYPE nChannelType);

	void SetMapThumbnail(const char* szMapName);
	void ClearMapThumbnail();
	void SerializeStageInterface();

	void EnableLobbyInterface(bool bEnable);
	void EnableStageInterface(bool bEnable);
	void ShowPrivateStageJoinFrame(const char* szStageName);

	void SetRoomNoLight(int d);

	void ShowEquipmentDialog(bool bShow = true);
	void ShowShopDialog(bool bShow = true);

	void ChangeSelectedChar(int nNum);

	void ShowReplayDialog(bool bShow);
	void ViewReplay(void);

	void ShowMenu(bool bEnable);
	void Show112Dialog(bool bShow);
	bool IsMenuVisible();

	bool OpenMiniMap();
	bool IsMiniMapEnable();

	void RequestQuickJoin();

	void EnableCharSelectionInterface(bool bEnable);

public:

	bool IsReadyToPropose();

	void OnReplay();

	void OnRequestXTrapSeedKey(unsigned char* pComBuf);

	void OnDisconnectMsg(const DWORD dwMsgID);
	void ShowDisconnectMsg(DWORD errStrID, DWORD delayTime);

	void OnAnnounceDeleteClan(const string& strAnnounce);

	MBitmap* GetQuestItemIcon(int nItemID, bool bSmallIcon);

	static bool CheckSkipGlobalEvent() { return m_bSkipGlobalEvent; }
	void SetSkipGlobalEvent(bool bSkip) { m_bSkipGlobalEvent = bSkip; }

	void OnVoiceSound();
	void PlayVoiceSound(char* pszSoundName, DWORD time = 0);

	void SetAgentPing(DWORD nIP, DWORD nTimeStamp);

	void OnRequestGameguardAuth(const DWORD dwIndex, const DWORD dwValue1, const DWORD dwValue2, const DWORD dwValue3);

	void SetErrMaxPlayerDelayTime(DWORD dDelayTime) { m_dErrMaxPalyerDelayTime = dDelayTime; }
	DWORD GetErrMaxPlayerDelayTime() { return m_dErrMaxPalyerDelayTime; }
	void SetErrMaxPlayer(bool bErrMaxPalyer) { m_bErrMaxPalyer = bErrMaxPalyer; }
	bool IsErrMaxPlayer() { return m_bErrMaxPalyer == 0 ? false : true; }

	virtual void MultiplySize(float byIDLWidth, float byIDLHeight, float byCurrWidth, float byCurrHeight);

	void SetDuelTournamentCharacterList(MDUELTOURNAMENTTYPE nType, const vector<DTPlayerInfo>& vecDTPlayerInfo);
	const vector<DTPlayerInfo>& GetVectorDTPlayerInfo() { return m_vecDTPlayerInfo; }
	void SetDuelTournamantType(MDUELTOURNAMENTTYPE eType) { m_eDuelTournamentType = eType; }
	MDUELTOURNAMENTTYPE GetDuelTournamentType() { return m_eDuelTournamentType; }

	void UpdateDuelTournamantMyCharInfoUI();
	void UpdateDuelTournamantMyCharInfoPreviousUI();

	bool GetIsGameFinishLeaveBattle() { return m_bGameFinishLeaveBattle; }

	__forceinline ZGameClient* GetGameClient(void) { return m_spGameClient; }
	__forceinline ZGame* GetGame(void) { return m_pGame; }
	__forceinline ZCombatInterface* GetCombatInterface(void) { return m_pCombatInterface; }
	__forceinline ZShopEquipInterface* GetShopEquipInterface(void) { return m_pShopEquipInterface; }
	__forceinline ZCamera* GetCamera() { return &m_Camera; }
	__forceinline ZCharacter* GetMyCharacter() { return (ZCharacter*)m_pMyCharacter; }
	__forceinline ZBaseQuest* GetQuest();
	__forceinline ZQuest* GetQuestExactly() { return &m_Quest; }
	__forceinline ZSurvival* GetSurvivalExactly() { return &m_Survival; }
	__forceinline ZChat* GetChat() { return &m_Chat; }
	__forceinline ZGameTypeManager* GetGameTypeManager() { return &m_GameTypeManager; }

	ZScreenEffectManager* GetScreenEffectManager() { return m_pScreenEffectManager; }
	ZEffectManager* GetEffectManager() { return m_pEffectManager; }
	void SetGameClient(ZGameClient* pGameClient) { m_spGameClient = pGameClient; }

	ZCharacterSelectView* GetCharacterSelectView() { return m_pCharacterSelectView; }
	ZIDLResource* GetIDLResource(void) { return &m_IDLResource; }
	ZPlayerMenu* GetPlayerMenu() { return m_pPlayerMenu; }
	ZMiniMap* GetMiniMap() { return m_pMiniMap; }

	ZTips* GetTips() { return &m_Tips; }
	ZBandiCapturer* GetBandiCapturer() { return m_Capture; }
	ZCombatMenu* GetCombatMenu() { return &m_CombatMenu; }
	ZMonsterBookInterface* GetMonsterBookInterface() { return m_pMonsterBookInterface; }

	ZBitmapManager<int>* GetItemThumbnailMgr() { return &m_ItemThumbnailMgr; }
};

__forceinline ZBaseQuest* ZGameInterface::GetQuest()
{
	if (m_pGame && m_pGame->GetMatch())
	{
		MMATCH_GAMETYPE gameType = ZGetGame()->GetMatch()->GetMatchType();

		if (m_GameTypeManager.IsQuestOnly(gameType))
			return static_cast<ZBaseQuest*>(&m_Quest);
		else if (m_GameTypeManager.IsSurvivalOnly(gameType))
			return static_cast<ZBaseQuest*>(&m_Survival);
	}

	return static_cast<ZBaseQuest*>(&m_Quest);
}

#define BEGIN_WIDGETLIST(_ITEM, _IDLRESPTR, _CLASS, _INSTANCE)								\
{																							\
	MWidgetList WidgetList;																	\
	(_IDLRESPTR)->FindWidgets(WidgetList, _ITEM);											\
	for (MWidgetList::iterator itor = WidgetList.begin(); itor != WidgetList.end(); ++itor) \
{																							\
	if ((*itor) != NULL)																	\
{																							\
	_CLASS _INSTANCE = ((_CLASS)(*itor));

#define END_WIDGETLIST()		}}}

#define DEFAULT_INTERFACE_SKIN "Default"

#define WM_CHANGE_GAMESTATE		(WM_USER + 25)
void ZChangeGameState(GunzState state);

inline void GetDuelTournamentGradeIconFileName(char* out_sz, int grade)
{
	sprintf(out_sz, "dt_grade%d.png", grade);
}

char* GetItemSlotName(const char* szName, int nItem);
bool SetWidgetToolTipText(char* szWidget, const char* szToolTipText, MAlignmentMode mam = MAM_LEFT | MAM_TOP);

#endif