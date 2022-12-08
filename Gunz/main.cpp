#include "stdafx.h"

#include "ZPrerequisites.h"
#include "ZConfiguration.h"
#include "ZGameClient.h"
#include <windows.h>
#include <wingdi.h>
#include <mmsystem.h>
#include <shlwapi.h>
#include <shellapi.h>

#include "dxerr.h"

#include "main.h"
#include "resource.h"
#include "VersionNo.h"

#include "Mint4R2.h"
#include "ZApplication.h"
#include "MDebug.h"
#include "ZMessages.h"
#include "MMatchNotify.h"
#include "RealSpace2.h"
#include "Mint.h"
#include "ZGameInterface.h"
#include "RFrameWork.h"
#include "ZButton.h"
#include "ZDirectInput.h"
#include "ZActionDef.h"
#include "MRegistry.h"
#include "ZInitialLoading.h"
#include "MDebug.h"
#include "MCrashDump.h"
#include "ZEffectFlashBang.h"
#include "ZMsgBox.h"
#include "ZStencilLight.h"
#include "ZReplay.h"
#include "ZUtil.h"
#include "ZOptionInterface.h"
#include "HMAC_SHA1.h"

#ifdef USING_VERTEX_SHADER
#include "RShaderMgr.h"
#endif

#include "RLenzFlare.h"
#include "ZLocale.h"
#include "MSysInfo.h"

#include "MTraceMemory.h"
#include "ZInput.h"
#include "Mint4Gunz.h"
#include "SecurityTest.h"
#include "CheckReturnCallStack.h"

#include "MFile.h"
#include "RGMain.h"
#include "RGGlobal.h"

RMODEPARAMS	g_ModeParams = { 800,600,true,D3DFMT_R5G6B5 };

#ifndef _DEBUG
#define SUPPORT_EXCEPTIONHANDLING
#endif

void AntiShotbotLogger();
RRESULT RenderScene(void* pParam);

#define RD_STRING_LENGTH 512
char cstrReleaseDate[512];

ZApplication	g_App;
MDrawContextR2* g_pDC = NULL;
MFontR2* g_pDefFont = NULL;
ZDirectInput	g_DInput;
ZInput* g_pInput = NULL;
Mint4Gunz		g_Mint;

HRESULT GetDirectXVersionViaDxDiag(DWORD* pdwDirectXVersionMajor, DWORD* pdwDirectXVersionMinor, TCHAR* pcDirectXVersionLetter);

void zexit(int returnCode)
{
#ifdef _GAMEGUARD
	GetZGameguard().Release();
#endif
	exit(returnCode);
}

void CrcFailExitApp() {
#ifdef _PUBLISH
	PostMessage(g_hWnd, WM_CLOSE, 0, 0);
#else
	int* crash = NULL;
	*crash = 0;
#endif
}

void _ZChangeGameState(int nIndex)
{
	GunzState state = GunzState(nIndex);

	if (ZApplication::GetGameInterface())
	{
		ZApplication::GetGameInterface()->SetState(state);
	}
}

RRESULT OnCreate(void* pParam)
{
	g_App.PreCheckArguments();

	string strFileLenzFlare("System/LenzFlare.xml");
#ifndef _DEBUG
	strFileLenzFlare += "";
#endif
	RCreateLenzFlare(strFileLenzFlare.c_str());
	RGetLenzFlare()->Initialize();

	mlog("main : RGetLenzFlare()->Initialize() \n");

	RBspObject::CreateShadeMap("sfx/water_splash.bmp");

	sprintf_safe(cstrReleaseDate, "Version : (" __DATE__ " " __TIME__ ")");
	g_DInput.Create(g_hWnd, FALSE, FALSE);
	g_pInput = new ZInput(&g_DInput);

	RSetGammaRamp(Z_VIDEO_GAMMA_VALUE);
	RSetRenderFlags(RRENDER_CLEAR_BACKBUFFER);

	ZGetInitialLoading()->Initialize(1, 0, 0, RGetScreenWidth(), RGetScreenHeight(), 0, 0, 1024, 768);

	mlog("InitialLoading success.\n");
	struct _finddata_t c_file;
	intptr_t hFile;
	char szFileName[256];
#define FONT_DIR	"Font/"
#define FONT_EXT	"ttf"
	if ((hFile = _findfirst(FONT_DIR "*." FONT_EXT, &c_file)) != -1L) {
		do {
			strcpy(szFileName, FONT_DIR);
			strcat(szFileName, c_file.name);
			AddFontResource(szFileName);
		} while (_findnext(hFile, &c_file) == 0);
		_findclose(hFile);
	}

	g_pDefFont = new MFontR2;

	if (!g_pDefFont->Create("Default", Z_LOCALE_DEFAULT_FONT, DEFAULT_FONT_HEIGHT, 1.0f))
	{
		mlog("Fail to Create defualt font : MFontR2 / main.cpp.. onCreate\n");
		g_pDefFont->Destroy();
		SAFE_DELETE(g_pDefFont);
		g_pDefFont = NULL;
	}

	g_pDC = new MDrawContextR2(RGetDevice());

#ifndef _FASTDEBUG
	if (ZGetInitialLoading()->IsUseEnable())
	{
		char szFileName[256];
		int nBitmap = rand() % 3;
		switch (nBitmap)
		{
		case (0):
			strcpy_safe(szFileName, "interface/Loading/loading_1.jpg");
			break;
		case (1):
			strcpy_safe(szFileName, "interface/Loading/loading_2.jpg");
			break;
		case (2):
			strcpy_safe(szFileName, "interface/Loading/loading_3.jpg");
			break;
		case (3):
			strcpy_safe(szFileName, "interface/Loading/loading_4.jpg");
			break;
		}

		ZGetInitialLoading()->AddBitmapGrade("interface/Loading/loading_grade_fifteen.jpg");
		ZGetInitialLoading()->AddBitmap(0, szFileName);
		ZGetInitialLoading()->AddBitmapBar("interface/Loading/loading.bmp");
		ZGetInitialLoading()->SetText(g_pDefFont, 10, 30, cstrReleaseDate);
		ZGetInitialLoading()->SetPercentage(0.0f);
		ZGetInitialLoading()->Draw(MODE_FADEIN, 0, true);
	}
#endif

	g_Mint.Initialize(800, 600, g_pDC, g_pDefFont);
	Mint::GetInstance()->SetHWND(RealSpace2::g_hWnd);

	mlog("interface Initialize success\n");

	ZLoadingProgress appLoading("application");
	if (!g_App.OnCreate(&appLoading))
	{
		ZGetInitialLoading()->Release();
		return R_ERROR_LOADING;
	}

	ZGetSoundEngine()->SetEffectVolume(Z_AUDIO_EFFECT_VOLUME);
	ZGetSoundEngine()->SetMusicVolume(Z_AUDIO_BGM_VOLUME);
	ZGetSoundEngine()->SetEffectMute(Z_AUDIO_EFFECT_MUTE);
	ZGetSoundEngine()->SetMusicMute(Z_AUDIO_BGM_MUTE);

	g_Mint.SetWorkspaceSize(g_ModeParams.nWidth, g_ModeParams.nHeight);
	g_Mint.GetMainFrame()->SetSize(g_ModeParams.nWidth, g_ModeParams.nHeight);
	ZGetOptionInterface()->Resize(g_ModeParams.nWidth, g_ModeParams.nHeight);

	for (int i = 0; i < ZACTION_COUNT; i++)
	{
		ZACTIONKEYDESCRIPTION& keyDesc = ZGetConfiguration()->GetKeyboard()->ActionKeys[i];
		g_pInput->RegisterActionKey(i, keyDesc.nVirtualKey);
		if (keyDesc.nVirtualKeyAlt != -1)
			g_pInput->RegisterActionKey(i, keyDesc.nVirtualKeyAlt);
	}

	g_App.SetInitialState();

	ZGetFlashBangEffect()->SetDrawCopyScreen(true);

	static const char* szDone = "Done.";
	ZGetInitialLoading()->SetLoadingStr(szDone);
	if (ZGetInitialLoading()->IsUseEnable())
	{
#ifndef _FASTDEBUG
		ZGetInitialLoading()->SetPercentage(100.f);
		ZGetInitialLoading()->Draw(MODE_FADEOUT, 0, true);
#endif
		ZGetInitialLoading()->Release();
	}

	ChangeAA(ZGetConfiguration()->GetVideo()->nAntiAlias);
	RMODEPARAMS ModeParams = { RGetScreenWidth(),RGetScreenHeight(),RIsFullScreen(),RGetPixelFormat() };
	RResetDevice(&ModeParams);

	mlog("main : OnCreate() done\n");

	SetFocus(g_hWnd);

	return R_OK;
}

bool CheckDll(char* fileName, BYTE* SHA1_Value)
{
	BYTE digest[20];
	BYTE Key[GUNZ_HMAC_KEY_LENGTH];

	memset(Key, 0, 20);
	memcpy(Key, GUNZ_HMAC_KEY, strlen(GUNZ_HMAC_KEY));

	CHMAC_SHA1 HMAC_SHA1;
	HMAC_SHA1.HMAC_SHA1_file(fileName, Key, GUNZ_HMAC_KEY_LENGTH, digest);

	if (memcmp(digest, SHA1_Value, 20) == 0)
	{
		return true;
	}

	return false;
}

RRESULT OnDestroy(void* pParam)
{
	mlog("Destroy gunz\n");

	g_App.OnDestroy();

	SAFE_DELETE(g_pDefFont);

	g_Mint.Finalize();

	mlog("interface finalize.\n");

	SAFE_DELETE(g_pInput);
	g_DInput.Destroy();

	mlog("game input destroy.\n");

	RGetShaderMgr()->Release();

	ZGetConfiguration()->Destroy();

	mlog("game gonfiguration destroy.\n");

	delete g_pDC;

	struct _finddata_t c_file;
	intptr_t hFile;
	char szFileName[256];
#define FONT_DIR	"Font/"
#define FONT_EXT	"ttf"
	if ((hFile = _findfirst(FONT_DIR "*." FONT_EXT, &c_file)) != -1L) {
		do {
			strcpy(szFileName, FONT_DIR);
			strcat(szFileName, c_file.name);
			RemoveFontResource(szFileName);
		} while (_findnext(hFile, &c_file) == 0);
		_findclose(hFile);
	}

	MFontManager::Destroy();
	MBitmapManager::Destroy();
	MBitmapManager::DestroyAniBitmap();

	mlog("Bitmap manager destroy Animation bitmap.\n");

	ZBasicInfoItem::Release();
	ZGetStencilLight()->Destroy();
	LightSource::Release();

	RBspObject::DestroyShadeMap();
	RDestroyLenzFlare();
	RAnimationFileMgr::GetInstance()->Destroy();

	ZStringResManager::ResetInstance();

	GetMeshManager()->Destroy();

	mlog("destroy gunz finish.\n");

	return R_OK;
}

RRESULT OnUpdate(void* pParam)
{
	auto prof = MBeginProfile("main::OnUpdate");

	g_pInput->Update();

	g_App.OnUpdate();

	const DWORD dwCurrUpdateTime = timeGetTime();

	__EP(100);

	return R_OK;
}

RRESULT OnRender(void* pParam)
{
	auto mainOnRender = MBeginProfile("main::OnRender");

	if (!RIsActive() && RIsFullScreen())
		return R_NOTREADY;

	g_App.OnDraw();

	MEndProfile(mainOnRender);

	return R_OK;
}

RRESULT OnInvalidate(void* pParam)
{
	MBitmapR2::m_dwStateBlock = NULL;

	g_App.OnInvalidate();

	return R_OK;
}

RRESULT OnRestore(void* pParam)
{
	for (int i = 0; i < MBitmapManager::GetCount(); i++) {
		MBitmapR2* pBitmap = (MBitmapR2*)MBitmapManager::Get(i);
		pBitmap->OnLostDevice();
	}

	g_App.OnRestore();

	return R_OK;
}

RRESULT OnActivate(void* pParam)
{
	if (ZGetGameInterface() && ZGetGameClient() && Z_ETC_BOOST)
		ZGetGameClient()->PriorityBoost(true);
	return R_OK;
}

RRESULT OnDeActivate(void* pParam)
{
	if (ZGetGameInterface() && ZGetGameClient())
		ZGetGameClient()->PriorityBoost(false);
	return R_OK;
}

RRESULT OnError(void* pParam)
{
	mlog("RealSpace::OnError(%d) \n", RGetLastError());

	switch (RGetLastError())
	{
	case RERROR_INVALID_DEVICE:
	{
		D3DADAPTER_IDENTIFIER9* ai = RGetAdapterID();
		char szLog[512];
		ZTransMsg(szLog, MSG_DONOTSUPPORT_GPCARD, 1, ai->Description);

		int ret = MessageBox(NULL, szLog, ZMsg(MSG_WARNING), MB_YESNO);
		if (ret != IDYES)
			return R_UNKNOWN;
	}
	break;
	case RERROR_CANNOT_CREATE_D3D:
	{
		ShowCursor(TRUE);

		char szLog[512];
		sprintf(szLog, ZMsg(MSG_DIRECTX_NOT_INSTALL));

		int ret = MessageBox(NULL, szLog, ZMsg(MSG_WARNING), MB_YESNO);
		if (ret == IDYES)
		{
			ShellExecute(g_hWnd, "open", ZMsg(MSG_DIRECTX_DOWNLOAD_URL), NULL, NULL, SW_SHOWNORMAL);
		}
	}
	break;
	};

	return R_OK;
}

void SetModeParams()
{
	g_ModeParams.bFullScreen = ZGetConfiguration()->GetVideo()->bFullScreen;
	g_ModeParams.nWidth = ZGetConfiguration()->GetVideo()->nWidth;
	g_ModeParams.nHeight = ZGetConfiguration()->GetVideo()->nHeight;
	ZGetConfiguration()->GetVideo()->nColorBits == 32 ?
		g_ModeParams.PixelFormat = D3DFMT_X8R8G8B8 : g_ModeParams.PixelFormat = D3DFMT_R5G6B5;
}

void ResetAppResource()
{
#ifdef LOCALE_NHNUSAA
	ZNHN_USAAuthInfo* pUSAAuthInfo = (ZNHN_USAAuthInfo*)ZGetLocale()->GetAuthInfo();
	string strUserID = pUSAAuthInfo->GetUserID();
#endif

	ZGetGameInterface()->m_sbRemainClientConnectionForResetApp = true;
	ZGetGameInterface()->GetGameClient()->Destroy();

	OnDestroy(0);

	ZGetConfiguration()->Destroy();
	ZGetConfiguration()->Load();

	SetModeParams();

	if (!ZApplication::GetInstance()->InitLocale())
		MLog("In changing language... InitLocale error !!!\n");

	ZGetConfiguration()->Load_StringResDependent();

	OnCreate(0);
	RGetParticleSystem()->Restore();
	OnRestore(0);

	ZGetGameInterface()->m_sbRemainClientConnectionForResetApp = false;

	ZPostRequestCharacterItemListForce(ZGetGameClient()->GetPlayerUID());

	ZGetGameInterface()->UpdateDuelTournamantMyCharInfoUI();
	ZGetGameInterface()->UpdateDuelTournamantMyCharInfoPreviousUI();
}

int FindStringPos(char* str, char* word)
{
	if (!str || str[0] == 0)	return -1;
	if (!word || word[0] == 0)	return -1;

	int str_len = (int)strlen(str);
	int word_len = (int)strlen(word);

	char c;
	bool bCheck = false;

	for (int i = 0; i < str_len; i++) {
		c = str[i];
		if (c == word[0]) {
			bCheck = true;

			for (int j = 1; j < word_len; j++) {
				if (str[i + j] != word[j]) {
					bCheck = false;
					break;
				}
			}

			if (bCheck) {
				return i;
			}
		}
	}
	return -1;
}

bool FindCrashFunc(char* pName)
{
	if (!pName) return false;

	FILE* fp;
	fp = fopen("mlog.txt", "r");
	if (fp == NULL)  return false;

	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char* pBuffer = new char[size];

	fread(pBuffer, 1, size, fp);

	fclose(fp);

	int posSource = FindStringPos(pBuffer, "ublish");
	if (posSource == -1) return false;

	int posA = FindStringPos(pBuffer + posSource, "Function Name");
	int posB = posA + FindStringPos(pBuffer + posSource + posA, "\n");

	if (posA == -1) return false;
	if (posB == -1) return false;

	posA += 16;

	int memsize = posB - posA;
	memcpy(pName, &pBuffer[posA + posSource], memsize);

	pName[memsize] = 0;

	delete[] pBuffer;

	for (int i = 0; i < memsize; i++) {
		if (pName[i] == ':') {
			pName[i] = '-';
		}
	}

	return true;
}

long FAR PASCAL WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SYSCHAR:
		if (wParam == VK_RETURN)
		{
#ifndef _PUBLISH
			RFrame_ToggleFullScreen();
#endif
			return 0;
	}
		break;

	case WM_CREATE:
		if (strlen(Z_LOCALE_HOMEPAGE_TITLE) > 0)
		{
			ShowIExplorer(false, Z_LOCALE_HOMEPAGE_TITLE);
		}
		break;
	case WM_DESTROY:
		if (strlen(Z_LOCALE_HOMEPAGE_TITLE) > 0)
		{
			ShowIExplorer(true, Z_LOCALE_HOMEPAGE_TITLE);
		}
		break;
	case WM_SETCURSOR:
		if (ZApplication::GetGameInterface())
			ZApplication::GetGameInterface()->OnResetCursor();
		return TRUE;

	case WM_ENTERIDLE:
		RFrame_UpdateRender();
		break;

	case WM_KEYDOWN:
	{
		bool b = false;
	}
}

	if (Mint::GetInstance()->ProcessEvent(hWnd, message, wParam, lParam) == true)
	{
		if (ZGetGameInterface() && ZGetGameInterface()->IsReservedResetApp())
		{
			ZGetGameInterface()->ReserveResetApp(false);
			ResetAppResource();
		}

		return 0;
	}

	if (message == WM_CHANGE_GAMESTATE)
	{
		_ZChangeGameState(wParam);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

enum RBASE_FONT {
	RBASE_FONT_GULIM = 0,
	RBASE_FONT_BATANG = 1,

	RBASE_FONT_END
};

static int g_base_font[RBASE_FONT_END];
static char g_UserDefineFont[256];

bool _GetFileFontName(char* pUserDefineFont)
{
	if (pUserDefineFont == NULL) return false;

	FILE* fp = fopen("_Font", "rt");
	if (fp) {
		fgets(pUserDefineFont, 256, fp);
		fclose(fp);
		return true;
	}
	return false;
}

bool CheckFont()
{
	char FontPath[MAX_PATH];
	char FontNames[MAX_PATH + 100];

	::GetWindowsDirectory(FontPath, MAX_PATH);

	strcpy(FontNames, FontPath);
	strcat(FontNames, "\\Fonts\\gulim.ttc");

	if (_access(FontNames, 0) != -1) { g_base_font[RBASE_FONT_GULIM] = 1; }
	else { g_base_font[RBASE_FONT_GULIM] = 0; }

	strcpy(FontNames, FontPath);
	strcat(FontNames, "\\Fonts\\batang.ttc");

	if (_access(FontNames, 0) != -1) { g_base_font[RBASE_FONT_BATANG] = 1; }
	else { g_base_font[RBASE_FONT_BATANG] = 0; }

	if (g_base_font[RBASE_FONT_GULIM] == 0 && g_base_font[RBASE_FONT_BATANG] == 0) {
		if (_access("_Font", 0) != -1) {
			_GetFileFontName(g_UserDefineFont);
		}
		else {
			int hr = IDOK;

			if (hr == IDOK) {
				return true;
			}
			else {
				return false;
			}
		}
	}
	return true;
}

#include "shlobj.h"

void CheckFileAssociation()
{
#define GUNZ_REPLAY_CLASS_NAME	"GunzReplay"

	char szValue[256];
	if (!MRegistry::Read(HKEY_CLASSES_ROOT, "." GUNZ_REC_FILE_EXT, NULL, szValue))
	{
		MRegistry::Write(HKEY_CLASSES_ROOT, "." GUNZ_REC_FILE_EXT, NULL, GUNZ_REPLAY_CLASS_NAME);

		char szModuleFileName[_MAX_PATH] = { 0, };
		GetModuleFileName(NULL, szModuleFileName, _MAX_DIR);

		char szCommand[_MAX_PATH];
		sprintf(szCommand, "\"%s\" \"%%1\"", szModuleFileName);

		MRegistry::Write(HKEY_CLASSES_ROOT, GUNZ_REPLAY_CLASS_NAME"\\shell\\open\\command", NULL, szCommand);

		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSH, NULL, NULL);
	}
}

HANDLE Mutex = 0;
DWORD g_dwMainThreadID;

int PASCAL WinMain(HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow)
{
	MCrashDump::SetCallback([](uintptr_t p) { MCrashDump::WriteDump(p, "Gunz.dmp"); });

	_set_invalid_parameter_handler([](const wchar_t* expression,
		const wchar_t* function,
		const wchar_t* file,
		unsigned int line,
		uintptr_t pReserved)
		{
			MLog("Invalid parameter detected in function %s.\n"
				"File: %s, line: %d.\nExpression: %s.\n",
				function, file, line, expression);
			assert(false);
		});

	g_dwMainThreadID = GetCurrentThreadId();

#ifdef _MTRACEMEMORY
	MInitTraceMemory();
#endif

	char szModuleFileName[_MAX_DIR] = { 0, };
	GetModuleFileName(NULL, szModuleFileName, _MAX_DIR);
	PathRemoveFileSpec(szModuleFileName);
	SetCurrentDirectory(szModuleFileName);

	InitLog(MLOGSTYLE_DEBUGSTRING | MLOGSTYLE_FILE);

	srand((unsigned int)time(nullptr));

	mlog("Gunz Online : " STRFILEVER " \n");
	mlog("Build : (" __DATE__ " " __TIME__ ")\n");
	char szDateRun[128] = "";
	char szTimeRun[128] = "";
	_strdate(szDateRun);
	_strtime(szTimeRun);
	mlog("Log time (%s %s)\n", szDateRun, szTimeRun);

#ifndef _PUBLISH
	mlog("cmdline = %s\n", cmdline);

#endif

	MSysInfoLog();

	CheckFileAssociation();

	MRegistry::szApplicationName = APPLICATION_NAME;

	g_App.InitFileSystem();

#if defined(_PUBLISH) && defined(ONLY_LOAD_MRS_FILES)
	MZFile::SetReadMode(MZIPREADFLAG_MRS2);
#endif

#ifdef LOCALE_NHNUSAA
	ZGetLanguageSetting_forNHNUSA()->SetLanguageIndexFromCmdLineStr(cmdline);
#endif

	ZGetConfiguration()->Load();

	CreateRGMain();

	ZStringResManager::MakeInstance();
	if (!ZApplication::GetInstance()->InitLocale())
	{
		MLog("Locale Init error !!!\n");
		return false;
	}

	ZGetConfiguration()->Load_StringResDependent();

	if (!ZGetConfiguration()->LateStringConvert())
	{
		MLog("main.cpp - Late string convert fale.\n");
		return false;
	}

	if (!InitializeNotify(ZApplication::GetFileSystem())) {
		MLog("Check notify.xml\n");
		return 0;
	}
	else
	{
		mlog("InitializeNotify ok.\n");
	}

	if (CheckFont() == false) {
		MLog("폰트가 없는 유저가 폰트 선택을 취소\n");
		return 0;
	}

	RSetFunction(RF_CREATE, OnCreate);
	RSetFunction(RF_RENDER, OnRender);
	RSetFunction(RF_UPDATE, OnUpdate);
	RSetFunction(RF_DESTROY, OnDestroy);
	RSetFunction(RF_INVALIDATE, OnInvalidate);
	RSetFunction(RF_RESTORE, OnRestore);
	RSetFunction(RF_ACTIVATE, OnActivate);
	RSetFunction(RF_DEACTIVATE, OnDeActivate);
	RSetFunction(RF_ERROR, OnError);

	SetModeParams();

	const int nRMainReturn = RMain(APPLICATION_NAME, this_inst, prev_inst, cmdline, cmdshow, &g_ModeParams, WndProc, IDI_ICON1);
	if (0 != nRMainReturn)
		return nRMainReturn;

	if (0 != RInitD3D(&g_ModeParams))
	{
		MessageBox(g_hWnd, "fail to initialize DirectX", NULL, MB_OK);
		mlog("error init RInitD3D\n");
		return 0;
	}

	const int nRRunReturn = RRun();

	ShowWindow(g_hWnd, SW_MINIMIZE);

	ZStringResManager::FreeInstance();

	return nRRunReturn;
}