#include "stdafx.h"

#include "ZApplication.h"
#include "ZGameInterface.h"
#include "MCommandLogFrame.h"
#include "ZConsole.h"
#include "ZInterface.h"
#include "Config.h"
#include "MDebug.h"
#include "RMeshMgr.h"
#include "RShadermgr.h"
#include "ZConfiguration.h"
#include "MProfiler.h"
#include "MChattingFilter.h"
#include "ZInitialLoading.h"
#include "ZWorldItem.h"
#include "MMatchWorlditemdesc.h"
#include "MMatchQuestMonsterGroup.h"
#include "ZReplay.h"
#include "ZGameClient.h"
#include "MRegistry.h"
#include "CGLEncription.h"
#include "ZLocale.h"
#include "ZUtil.h"
#include "ZStringResManager.h"
#include "ZFile.h"
#include "ZActionKey.h"
#include "ZInput.h"
#include "ZOptionInterface.h"
#include "PrivateKey.h"

#include "RGMain.h"
#include "MeshManager.h"

#ifdef _ZPROFILER
#include "ZProfiler.h"
#endif

ZApplication* ZApplication::m_pInstance = NULL;
MZFileSystem		ZApplication::m_FileSystem;
ZSoundEngine		ZApplication::m_SoundEngine;
RMeshMgr			ZApplication::m_NPCMeshMgr;
RMeshMgr			ZApplication::m_MeshMgr;
RMeshMgr			ZApplication::m_WeaponMeshMgr;
RAniEventMgr   	    ZApplication::m_AniEventMgr;
ZTimer				ZApplication::m_Timer;
ZEmblemInterface	ZApplication::m_EmblemInterface;
ZSkillManager		ZApplication::m_SkillManager;

MCommandLogFrame* m_pLogFrame = NULL;

ZApplication::ZApplication()
{
	m_nTimerRes = 0;
	m_pInstance = this;

	m_pGameInterface = NULL;
	m_pStageInterface = NULL;

	m_nInitialState = GUNZ_LOGIN;

	m_bLaunchDevelop = false;
	m_bLaunchTest = false;

	SetLaunchMode(ZLAUNCH_MODE_DEBUG);

#ifdef _ZPROFILER
	m_pProfiler = new ZProfiler;
#endif
}

ZApplication::~ZApplication()
{
#ifdef _ZPROFILER
	SAFE_DELETE(m_pProfiler);
#endif

	m_pInstance = NULL;
}

bool GetNextName(char* szBuffer, int nBufferCount, const char* szSource)
{
	while (*szSource == ' ' || *szSource == '\t') szSource++;

	const char* end = NULL;
	if (szSource[0] == '"')
	{
		end = strchr(szSource + 1, '"');
		szSource++;
		end--;
	}
	else
	{
		end = strchr(szSource, ' ');
		if (NULL == end) end = strchr(szSource, '\t');
	}

	if (end)
	{
		int nCount = end - szSource;
		if (nCount == 0 || nCount >= nBufferCount) return false;

		strncpy_safe(szBuffer, nBufferCount, szSource, nCount);
		szBuffer[nCount] = 0;
	}
	else
	{
		int nCount = (int)strlen(szSource);
		if (nCount == 0 || nCount >= nBufferCount) return false;

		strcpy_safe(szBuffer, nBufferCount, szSource);
	}

	return true;
}

bool ZApplication::ParseArguments(const char* pszArgs)
{
	m_szCmdLine = pszArgs;

	if (pszArgs[0] == '"')
	{
		m_szFileName = pszArgs + 1;
		if (m_szFileName[m_szFileName.length() - 1] == '"')
			m_szFileName.resize(m_szFileName.length() - 1);
	}
	else
	{
		m_szFileName = pszArgs;
	}

	auto&& ZTOKEN_ASSETSDIR = "assetsdir";

	auto* str = strstr(pszArgs, ZTOKEN_ASSETSDIR);
	char buffer[256];

	if (str && GetNextName(buffer, sizeof(buffer), str + strlen(ZTOKEN_ASSETSDIR)))
	{
		AssetsDir = buffer;
	}

	if (_stricmp(m_szFileName.c_str() + m_szFileName.length() - strlen(GUNZ_REC_FILE_EXT),
		GUNZ_REC_FILE_EXT) == 0) {
		SetLaunchMode(ZLAUNCH_MODE_STANDALONE_REPLAY);
		m_nInitialState = GUNZ_GAME;
		return true;
	}

	if (pszArgs[0] == '/')
	{
#ifndef _PUBLISH
		if (strstr(pszArgs, "launchdevelop") != NULL)
		{
			SetLaunchMode(ZLAUNCH_MODE_STANDALONE_DEVELOP);
			m_bLaunchDevelop = true;

			return true;
		}
		else if (strstr(pszArgs, "launch") != NULL)
		{
			SetLaunchMode(ZLAUNCH_MODE_STANDALONE);
			return true;
		}
#endif
	}

#ifndef _PUBLISH
	{
		SetLaunchMode(ZLAUNCH_MODE_STANDALONE_DEVELOP);
		m_bLaunchDevelop = true;
		return true;
	}
#endif

	SetLaunchMode(ZLAUNCH_MODE_STANDALONE);

	return true;
}

void ZApplication::CheckSound()
{
#ifdef _BIRDSOUND

#else
	int size = m_MeshMgr.m_id_last;
	int ani_size = 0;

	RMesh* pMesh = NULL;
	RAnimationMgr* pAniMgr = NULL;
	RAnimation* pAni = NULL;

	for (int i = 0; i < size; i++) {
		pMesh = m_MeshMgr.GetFast(i);
		if (pMesh) {
			pAniMgr = &pMesh->m_ani_mgr;
			if (pAniMgr) {
				ani_size = pAniMgr->m_id_last;
				for (int j = 0; j < ani_size; j++) {
					pAni = pAniMgr->m_node_table[j];
					if (pAni) {
						if (m_SoundEngine.isPlayAbleMtrl(pAni->m_sound_name) == false) {
							pAni->ClearSoundFile();
						}
						else {
							int ok = 0;
						}
					}
				}
			}
		}
	}
#endif
}

void RegisterForbidKey()
{
	ZActionKey::RegisterForbidKey(0x3b);
	ZActionKey::RegisterForbidKey(0x3c);
	ZActionKey::RegisterForbidKey(0x3d);
	ZActionKey::RegisterForbidKey(0x3e);
	ZActionKey::RegisterForbidKey(0x3f);
	ZActionKey::RegisterForbidKey(0x40);
	ZActionKey::RegisterForbidKey(0x41);
	ZActionKey::RegisterForbidKey(0x42);
}

void ZProgressCallBack(void* pUserParam, float fProgress)
{
	ZLoadingProgress* pLoadingProgress = (ZLoadingProgress*)pUserParam;
	pLoadingProgress->UpdateAndDraw(fProgress);
}

namespace RealSpace2
{
	extern bool DynamicResourceLoading;
}

#define STARTUP_CACHE_FILES

#ifdef TIMESCALE
unsigned long long GetGlobalTimeMSOverride()
{
	if (!ZApplication::GetInstance())
		return timeGetTime();

	return ZApplication::GetInstance()->GetTime();
}
#endif

bool ZApplication::OnCreate(ZLoadingProgress* pLoadingProgress)
{
	MInitProfile();

#ifdef TIMESCALE
	GetGlobalTimeMS = GetGlobalTimeMSOverride;
#endif
	string strFileNameZItem(FILENAME_ZITEM_DESC);
	string strFileNameZItemLocale(FILENAME_ZITEM_DESC_LOCALE);
	string strFileNameZBuff(FILENAME_BUFF_DESC);
	string strFileNameWorlditem(FILENAME_WORLDITEM);
	string strFileNameAbuse(FILENAME_ABUSE);

	TIMECAPS tc;

	mlog("ZApplication::OnCreate : begin\n");

	ZLoadingProgress InitialLoading("Initializing", pLoadingProgress, 0.05f);
	InitialLoading.UpdateAndDraw(0);

#ifdef STARTUP_CACHE_FILES
	auto CacheArchives = MBeginProfile("Cache archives");
	const char* CachedFileNames[] = { "system", "model/woman", "model/man", "model/weapon", "model/NPC", "sfx",
		"interface/default", "interface/iconos","interface/iconos", "interface/login","interface/loading",
		"sound/bgm", "sound/effect", };
	for (auto&& File : CachedFileNames)
		m_FileSystem.CacheArchive(File);
	MEndProfile(CacheArchives);
#endif

	InitialLoading.UpdateAndDraw(0.5f);

	[&]
	{
		if (!IsDynamicResourceLoad())
			return;

		auto Fail = [&]()
		{
			MLog("Failed to load parts index! Turning off dynamic resource loading\n");
			RealSpace2::DynamicResourceLoading = false;
		};

		auto ret = ReadMZFile("system/parts_index.xml");
		if (!ret.first)
			return Fail();

		if (!GetMeshManager()->LoadParts(ret.second))
			return Fail();
	}();

	__BP(2000, "ZApplication::OnCreate");

#define MMTIMER_RESOLUTION	1
	if (TIMERR_NOERROR == timeGetDevCaps(&tc, sizeof(TIMECAPS)))
	{
		m_nTimerRes = min(max(tc.wPeriodMin, MMTIMER_RESOLUTION), tc.wPeriodMax);
		timeBeginPeriod(m_nTimerRes);
	}

	if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_NETMARBLE)
		m_nInitialState = GUNZ_DIRECTLOGIN;

	if (ZGameInterface::m_sbRemainClientConnectionForResetApp == true)
		m_nInitialState = GUNZ_LOBBY;

	DWORD _begin_time, _end_time;
#define BEGIN_ { _begin_time = timeGetTime(); }
#define END_(x) { _end_time = timeGetTime(); float f_time = (_end_time - _begin_time) / 1000.f; mlog("\n-------------------> %s : %f \n\n", x,f_time ); }

	__BP(2001, "m_SoundEngine.Create");

	ZLoadingProgress soundLoading("Sound", pLoadingProgress, .12f);
	BEGIN_;
#ifdef _BIRDSOUND
	m_SoundEngine.Create(RealSpace2::g_hWnd, 44100, Z_AUDIO_HWMIXING, GetFileSystem());
#else
	m_SoundEngine.Create(RealSpace2::g_hWnd, Z_AUDIO_HWMIXING, &soundLoading);
#endif
	END_("Sound Engine Create");
	soundLoading.UpdateAndDraw(1.f);

	__EP(2001);

	mlog("sound engine create.\n");

	RegisterForbidKey();

	__BP(2002, "m_pInterface->OnCreate()");

	ZLoadingProgress giLoading("GameInterface", pLoadingProgress, .35f);

	BEGIN_;
	m_pGameInterface = new ZGameInterface("Game Interface", Mint::GetInstance()->GetMainFrame(), Mint::GetInstance()->GetMainFrame());
	m_pGameInterface->m_nInitialState = m_nInitialState;
	if (!m_pGameInterface->OnCreate(&giLoading))
	{
		mlog("Failed: ZGameInterface OnCreate\n");
		SAFE_DELETE(m_pGameInterface);
		return false;
	}

	m_pGameInterface->SetBounds(0, 0, MGetWorkspaceWidth(), MGetWorkspaceHeight());
	END_("GameInterface Create");

	giLoading.UpdateAndDraw(1.f);

	m_pStageInterface = new ZStageInterface();
	m_pOptionInterface = new ZOptionInterface;

	__EP(2002);

#ifdef _BIRDTEST
	goto BirdGo;
#endif

	__BP(2003, "Character Loading");

	ZLoadingProgress meshLoading("Mesh", pLoadingProgress, .41f);
	BEGIN_;
	if (m_MeshMgr.LoadXmlList("system/model_character.xml", ZProgressCallBack, &meshLoading) == -1)
		return false;

	mlog("Load character.xml success,\n");

	END_("Character Loading");
	meshLoading.UpdateAndDraw(1.f);

#ifdef _QUEST
	if (m_NPCMeshMgr.LoadXmlList("system/model_npc.xml") == -1)
		return false;
#endif

	__EP(2003);

	__BP(2004, "WeaponMesh Loading");

	BEGIN_;

	string strFileNameWeapon("system/model_weapon.xml");
#ifndef _DEBUG
	strFileNameWeapon += "";
#endif
	if (m_WeaponMeshMgr.LoadXmlList((char*)strFileNameWeapon.c_str()) == -1)
		return false;

	END_("WeaponMesh Loading");

	__EP(2004);

	__BP(2005, "Worlditem Loading");

	ZLoadingProgress etcLoading("etc", pLoadingProgress, .02f);
	BEGIN_;

#ifdef	_WORLD_ITEM_
	m_MeshMgr.LoadXmlList((char*)strFileNameWorlditem.c_str());
#endif

	mlog("Load weapon.xml success. \n");

	END_("Worlditem Loading");
	__EP(2005);

	__BP(2006, "ETC .. XML");

	BEGIN_;
	CreateConsole(ZGetGameClient()->GetCommandManager());

	m_pLogFrame = new MCommandLogFrame("Command Log", Mint::GetInstance()->GetMainFrame(), Mint::GetInstance()->GetMainFrame());
	int nHeight = MGetWorkspaceHeight() / 3;
	m_pLogFrame->SetBounds(0, MGetWorkspaceHeight() - nHeight - 1, MGetWorkspaceWidth() - 1, nHeight);
	m_pLogFrame->Show(false);

	m_pGameInterface->SetFocusEnable(true);
	m_pGameInterface->SetFocus();
	m_pGameInterface->Show(true);

	if (!MGetMatchItemDescMgr()->ReadCache())
	{
		if (!MGetMatchItemDescMgr()->ReadXml(GetFileSystem(), strFileNameZItem.c_str()))
		{
			MLog("Error while Read Item Descriptor %s\n", strFileNameZItem.c_str());
		}
		if (!MGetMatchItemDescMgr()->ReadXml(GetFileSystem(), strFileNameZItemLocale.c_str()))
		{
			MLog("Error while Read Item Descriptor %s\n", strFileNameZItemLocale.c_str());
		}

		MGetMatchItemDescMgr()->WriteCache();
	}
	mlog("Load zitem info success.\n");

	if (!MGetMatchBuffDescMgr()->ReadXml(GetFileSystem(), strFileNameZBuff.c_str()))
	{
		MLog("Error while Read Buff Descriptor %s\n", strFileNameZBuff.c_str());
	}
	mlog("Load zBuff info success.\n");

	if (!MGetMatchWorldItemDescMgr()->ReadXml(GetFileSystem(), strFileNameWorlditem.c_str()))
	{
		MLog("Error while Read Item Descriptor %s\n", strFileNameWorlditem.c_str());
	}
	mlog("Init world item manager success.\n");

	if (!MGetMapDescMgr()->Initialize(GetFileSystem(), "system/map.xml"))
	{
		MLog("Error while Read map Descriptor %s\n", "system/map.xml");
	}
	mlog("Init map Descriptor success.\n");

	string strFileChannelRule("system/channelrule.xml");
#ifndef _DEBUG
	strFileChannelRule += "";
#endif
	if (!ZGetChannelRuleMgr()->ReadXml(GetFileSystem(), strFileChannelRule.c_str()))
	{
		MLog("Error while Read Item Descriptor %s\n", strFileChannelRule.c_str());
	}
	mlog("Init channel rule manager success.\n");
	bool bSucceedLoadAbuse = MGetChattingFilter()->LoadFromFile(GetFileSystem(), strFileNameAbuse.c_str());
	if (!bSucceedLoadAbuse || MGetChattingFilter()->GetNumAbuseWords() == 0)
	{
		MLog("Error while Read Abuse Filter %s\n", strFileNameAbuse.c_str());
		MessageBox(NULL, ZErrStr(MERR_FIND_INVALIDFILE), ZMsg(MSG_WARNING), MB_OK);
		return false;
	}
	mlog("Init abuse manager success.\n");

#ifdef _QUEST_ITEM
	if (!GetQuestItemDescMgr().ReadXml(GetFileSystem(), FILENAME_QUESTITEM_DESC))
	{
		MLog("Error while read quest tiem descrition xml file.\n");
	}
#endif

	mlog("Init chatting filter. success\n");

	if (!m_SkillManager.Create()) {
		MLog("Error while create skill manager\n");
	}

	END_("ETC ..");

#ifndef _BIRDTEST
	etcLoading.UpdateAndDraw(1.f);
#endif

	ZGetEmblemInterface()->Create();

	__EP(2006);

	__EP(2000);

	__SAVEPROFILE("profile_loading.txt");

#ifdef STARTUP_CACHE_FILES
	m_FileSystem.ReleaseCachedArchives();
#endif

	return true;

#undef BEGIN_
#undef END_
}

void ZApplication::OnDestroy()
{
	m_WorldManager.Destroy();
	ZGetEmblemInterface()->Destroy();

	MGetMatchWorldItemDescMgr()->Clear();

	m_SoundEngine.Destroy();
	DestroyConsole();

	mlog("Destroy console.\n");

	SAFE_DELETE(m_pLogFrame);
	SAFE_DELETE(m_pGameInterface);
	SAFE_DELETE(m_pStageInterface);
	SAFE_DELETE(m_pOptionInterface);

	m_NPCMeshMgr.DelAll();

	m_MeshMgr.DelAll();
	mlog("Destroy mesh manager.\n");

	m_WeaponMeshMgr.DelAll();
	mlog("Destroy weapon mesh manager.\n");

	m_SkillManager.Destroy();
	mlog("Clear SkillManager.\n");

#ifdef _QUEST_ITEM
	GetQuestItemDescMgr().Clear();
	mlog("Clear QuestItemDescMgr.\n");
#endif

	MGetMatchItemDescMgr()->Clear();
	mlog("Clear MatchItemDescMgr.\n");

	MGetChattingFilter()->Clear();
	mlog("Clear ChattingFilter.\n");

	ZGetChannelRuleMgr()->Clear();
	mlog("Clear ChannelRuleMgr.\n");

	if (m_nTimerRes != 0)
	{
		timeEndPeriod(m_nTimerRes);
		m_nTimerRes = 0;
	}

	RGetParticleSystem()->Destroy();

	// Custom: Destroy FS
	m_FileSystem.Destroy();

	mlog("destroy game application done.\n");
}

void ZApplication::ResetTimer()
{
	m_Timer.ResetFrame();
}

void ZApplication::OnUpdate()
{
	auto prof = MBeginProfile("ZApplication::OnUpdate");

	[&]
	{
		static u64 LastRealTime = timeGetTime();
		auto CurRealTime = timeGetTime();
		if (Timescale == 1.f)
		{
			Time += CurRealTime - LastRealTime;
		}
		else
		{
			auto Delta = double(CurRealTime - LastRealTime);
			Time += Delta * Timescale;
		}
		LastRealTime = CurRealTime;
	}();

	auto ElapsedTime = m_Timer.UpdateFrame();

	if (Timescale != 1.f)
		ElapsedTime *= Timescale;

	GetRGMain().OnUpdate(ElapsedTime);

	__BP(1, "ZApplication::OnUpdate::m_pInterface->Update");
	if (m_pGameInterface) m_pGameInterface->Update(ElapsedTime);
	__EP(1);

	__BP(2, "ZApplication::OnUpdate::SoundEngineRun");

	m_SoundEngine.Run();

	__EP(2);

	if (ZIsActionKeyPressed(ZACTION_SCREENSHOT)) {
		if (m_pGameInterface)
			m_pGameInterface->SaveScreenShot();
	}

	MGetMatchItemDescMgr()->ShiftMemoryGradually();

	__EP(0);
}

bool g_bProfile = false;

#define PROFILE_FILENAME	"profile.txt"

bool ZApplication::OnDraw()
{
	static bool currentprofile = false;
	if (g_bProfile && !currentprofile)
	{
		currentprofile = true;
		MInitProfile();
	}

	if (!g_bProfile && currentprofile)
	{
		currentprofile = false;
		MSaveProfile(PROFILE_FILENAME);
	}

	__BP(3, "ZApplication::Draw");

	__BP(4, "ZApplication::Draw::Mint::Run");
	if (ZGetGameInterface()->GetState() != GUNZ_GAME)
	{
		Mint::GetInstance()->Run();
	}
	__EP(4);

	__BP(5, "ZApplication::Draw::Mint::Draw");

	Mint::GetInstance()->Draw();

	__EP(5);

	__EP(3);

#ifdef _ZPROFILER
	m_pProfiler->Update();
	m_pProfiler->Render();
#endif

	return m_pGameInterface->IsDone();
}

ZSoundEngine* ZApplication::GetSoundEngine(void)
{
	return &m_SoundEngine;
}

void ZApplication::OnInvalidate()
{
	RGetShaderMgr()->Release();
	if (m_pGameInterface)
		m_pGameInterface->OnInvalidate();

	if (IsRGMainAlive())
		GetRGMain().OnInvalidate();
}

void ZApplication::OnRestore()
{
	if (m_pGameInterface)
		m_pGameInterface->OnRestore();
	if (ZGetConfiguration()->GetVideo()->bShader)
	{
		RMesh::mHardwareAccellated = true;
		if (!RGetShaderMgr()->SetEnable())
		{
			RGetShaderMgr()->SetDisable();
		}
	}

	if (IsRGMainAlive())
		GetRGMain().OnRestore();
}

void ZApplication::Exit()
{
	PostMessage(g_hWnd, WM_CLOSE, 0, 0);
}

#define ZTOKEN_GAME				"game"
#define ZTOKEN_REPLAY			"replay"
#define ZTOKEN_GAME_CHARDUMMY	"dummy"
#define ZTOKEN_GAME_AI			"ai"
#define ZTOKEN_QUEST			"quest"
#define ZTOKEN_FAST_LOADING		"fast"

void ZApplication::PreCheckArguments()
{
	if (strstr(m_szCmdLine.c_str(), ZTOKEN_FAST_LOADING)) {
		RMesh::SetPartsMeshLoadingSkip(1);
	}
}

void ZApplication::ParseStandAloneArguments(const char* pszArgs)
{
}

void ZApplication::SetInitialState()
{
	if (GetLaunchMode() == ZLAUNCH_MODE_STANDALONE_REPLAY) {
		CreateReplayGame(m_szFileName.c_str());
		return;
	}

	ParseStandAloneArguments(m_szCmdLine.c_str());

	ZGetGameInterface()->SetState(m_nInitialState);
}

bool ZApplication::InitLocale()
{
	ZGetLocale()->Init(GetCountryID(ZGetConfiguration()->GetLocale()->strCountry.c_str()));

	char szPath[MAX_PATH] = "system/";

	if (!ZGetConfiguration()->IsUsingDefaultLanguage())
	{
		const char* szSelectedLanguage = ZGetConfiguration()->GetSelectedLanguage();

		ZGetLocale()->SetLanguage(GetLanguageID(szSelectedLanguage));

		strcat(szPath, szSelectedLanguage);
		strcat(szPath, "/");
	}

	ZGetStringResManager()->Init(szPath, ZGetLocale()->GetLanguage(), GetFileSystem());

	return true;
}

bool ZApplication::GetSystemValue(const char* szField, char* szData, int maxlen)
{
	return MRegistry::Read(HKEY_CURRENT_USER, szField, szData, maxlen);
}

void ZApplication::SetSystemValue(const char* szField, const char* szData)
{
	MRegistry::Write(HKEY_CURRENT_USER, szField, szData);
}

void ZApplication::InitFileSystem()
{
	m_FileSystem.Create(AssetsDir);
	RSetFileSystem(ZApplication::GetFileSystem());
}