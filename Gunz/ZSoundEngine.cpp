#include "stdafx.h"

#include "ZGameInterface.h"
#include "ZGame.h"
#include "ZSoundEngine.h"
#include "MDebug.h"
#include "RealSpace2.h"
#include "MMatchItem.h"
#include "ZConfiguration.h"
#include "Physics.h"
#include "Fileinfo.h"
#include "ZApplication.h"
#include "ZSoundFMod.h"
#include "MMath.h"
#include "ZInitialLoading.h"

_USING_NAMESPACE_REALSPACE2

constexpr auto ZDEF_MINDISTANCE = 300.0f;					//	3m
constexpr auto ZDEF_MAXDISTANCE = 4000.f;				//	40m
constexpr auto ZDEF_MAXDISTANCESQ = 16000000.F;
constexpr auto ZDEF_MAX_DISTANCE_PRIORITY = 100;
constexpr auto ZDEF_ROLLFACTOR = 1.0f;

// Sound FX
constexpr auto SOUNDEFFECT_DIR = "sound/effect/";
constexpr auto SOUNDEFFECT_EXT = ".wav";
constexpr auto SOUND_OPTION_FILE_NAME = "SoundOption.xml";
constexpr auto SOUNDEFFECT_XML = "sound/effect/effect.xml";
constexpr auto SOUNDNPC_DIR = "sound/npc/";
constexpr auto DEFAULT_SOUND_FRAME = 30;

FSOUND_SAMPLE* ZSoundEngine::GetFS(const char* szName, bool bHero)
{
	SoundSource* pSS = GetSoundSource(szName, bHero);
	if (pSS != NULL) return pSS->pFS;
	return NULL;
}

SoundSource* ZSoundEngine::GetSoundSource(const char* szName, bool bHero)
{
	SESMAP::iterator i;
	if (!bHero)
	{
		i = m_SoundEffectSource.find(string(szName));
		if (i == m_SoundEffectSource.end())
		{
			i = m_SoundEffectSource2D.find(string(szName));
			if (i == m_SoundEffectSource2D.end()) return NULL;
		}
	}
	else
	{
		i = m_SoundEffectSource2D.find(string(szName));
		if (i == m_SoundEffectSource2D.end())
		{
			i = m_SoundEffectSource.find(string(szName));
			if (i == m_SoundEffectSource.end()) return NULL;
		}
	}

	SoundSource* pRetSource = (SoundSource*)(i->second);
	return pRetSource;
}

ZSoundEngine::ZSoundEngine()
{
	m_pMusicBuffer = NULL;

	m_fEffectVolume = 1.0f;
	m_fMusicVolume = 0.0f;

	m_bEffectMute = false;
	m_szOpenedMusicName[0] = 0;
	m_bSoundEnable = false;
	m_b3DSound = true;
	m_b3DSoundUpdate = false;
	m_bHWMixing = true;

	m_ListenerPos = rvector(0, 0, 0);

	m_bEffectVolControl = false;
	m_bBGMVolControl = false;
	m_fEffectVolFactor = 0;
	m_fEffectVolEnd = 0;
	m_fBGMVolFactor = 0;
	m_fBGMVolEnd = 0;

	m_bBattleMusic = false;
	m_pfs = NULL;
}

ZSoundEngine::~ZSoundEngine()
{
}

void ZSoundEngine::SetEffectVolume(float fVolume)
{
	m_fEffectVolume = fVolume;
	ZGetSoundFMod()->SetVolume(m_fEffectVolume * 255);
}

void ZSoundEngine::SetEffectVolume(int iChnnel, float fVolume)
{
	ZGetSoundFMod()->SetVolume(iChnnel, (int)(fVolume * 255));
}

bool ZSoundEngine::Create(HWND hwnd, bool bHWMixing, ZLoadingProgress* pLoadingProgress)
{
	m_bSoundEnable = ZGetSoundFMod()->Create(hwnd, FSOUND_OUTPUT_DSOUND, 44100, bHWMixing ? 16 : 1024, bHWMixing ? 16 : 0, bHWMixing ? 16 : 32, 0);
	if (!m_bSoundEnable && bHWMixing)
	{
		m_bSoundEnable = ZGetSoundFMod()->Create(hwnd, FSOUND_OUTPUT_DSOUND, 44100, 0, 0, 32, 0);
		if (!m_bSoundEnable)
		{
			mlog("Fail to Create Sound Engine..\n");
			return false;
		}
		else
		{
			mlog("Create SoundEngine with Software mode after failed with hardware mode..\n");
			m_bHWMixing = false;
			Z_AUDIO_HWMIXING = false;
		}
	}
	m_b8Bits = Z_AUDIO_8BITSOUND;

	if (!LoadResource(const_cast<char*>(SOUNDEFFECT_XML), pLoadingProgress))
	{
		mlog("fail to load sound effect\n");
	}

	ZGetSoundFMod()->SetRollFactor(3.f);
	ZGetSoundFMod()->SetDistanceFactor(100.f);
	ZGetSoundFMod()->SetDopplerFactor(1.f);

	ZGetSoundFMod()->SetMusicEndCallback(MusicEndCallback, this);

	SetEffectVolume(m_fEffectVolume);
	SetMusicVolume(m_fMusicVolume);
	ZGetSoundFMod()->SetMute(Z_AUDIO_EFFECT_MUTE);
	ZGetSoundFMod()->SetMusicMute(Z_AUDIO_BGM_MUTE);

	SetFramePerSecond(DEFAULT_SOUND_FRAME);
	SetInverseSound(Z_AUDIO_INVERSE);

	m_bHWMixing = Z_AUDIO_HWMIXING;

	return true;
}

bool ZSoundEngine::Reset(HWND hwnd, bool bHWMixing)
{
	if (m_bHWMixing == bHWMixing) return false;
	m_bHWMixing = bHWMixing;

	char szBuffer[128];
	strcpy(szBuffer, m_szOpenedMusicName);

	Destroy();
	m_bSoundEnable = Create(hwnd, bHWMixing);
	if (!m_bSoundEnable)	return false;

	if (ZGetGame() != NULL)
	{
		list<AmbSndInfo*> aslist = ZGetGame()->GetWorld()->GetBsp()->GetAmbSndList();
		for (list<AmbSndInfo*>::iterator iter = aslist.begin(); iter != aslist.end(); ++iter)
		{
			AmbSndInfo* pAS = *iter;
			if (pAS->itype & AS_AABB)
				ZGetSoundEngine()->SetAmbientSoundBox(pAS->szSoundName, pAS->min, pAS->max, (pAS->itype & AS_2D) ? true : false);
			else if (pAS->itype & AS_SPHERE)
				ZGetSoundEngine()->SetAmbientSoundSphere(pAS->szSoundName, pAS->center, pAS->radius, (pAS->itype & AS_2D) ? true : false);
		}
	}

	OpenMusic(szBuffer, ZGetFileSystem());
	PlayMusic();
	return true;
}

void ZSoundEngine::Destroy()
{
	mlog("Destroy sound engine.\n");
	for (SESMAP::iterator iter = m_SoundEffectSource.begin(); iter != m_SoundEffectSource.end(); ++iter)
	{
		SoundSource* pSS = iter->second;
		if (pSS != NULL && pSS->pFS != NULL)
			FSOUND_Sample_Free(pSS->pFS);
		SAFE_DELETE(pSS);
	}
	m_SoundEffectSource.clear();

	for (SESMAP::iterator iter = m_SoundEffectSource2D.begin(); iter != m_SoundEffectSource2D.end(); ++iter)
	{
		SoundSource* pSS = iter->second;
		if (pSS != NULL && pSS->pFS != NULL)
			FSOUND_Sample_Free(pSS->pFS);
		SAFE_DELETE(pSS);
	}
	m_SoundEffectSource2D.clear();
	m_DelaySoundList.clear();
	ClearAmbientSound();

	StopMusic();
	CloseMusic();

	ZGetSoundFMod()->Close();

	mlog("Destroy sound engine. success\n");
}

bool ZSoundEngine::SetSamplingBits(bool b8Bits)
{
	if (b8Bits == m_b8Bits) return true;
	m_b8Bits = b8Bits;
	return Reload();
}

bool ZSoundEngine::OpenMusic(const char* szFileName, MZFileSystem* pfs)
{
	if (!m_bSoundEnable) return false;

	m_pfs = pfs;
	if (!strcmp(m_szOpenedMusicName, szFileName)) return false;
	if (m_szOpenedMusicName[0] != 0) CloseMusic();
	strcpy(m_szOpenedMusicName, szFileName);

	MZFile mzf;
	mzf.SetReadMode(MZIPREADFLAG_ZIP | MZIPREADFLAG_MRS | MZIPREADFLAG_MRS2 | MZIPREADFLAG_FILE);
	if (!mzf.Open(szFileName, pfs)) return false;
	m_pMusicBuffer = new char[mzf.GetLength() + 1];
	mzf.Read(m_pMusicBuffer, mzf.GetLength());
	m_pMusicBuffer[mzf.GetLength()] = 0;

	int len = mzf.GetLength();

	return ZGetSoundFMod()->OpenStream(m_pMusicBuffer, len);
}

void ZSoundEngine::CloseMusic()
{
	if (!m_bSoundEnable) return;

	if (m_szOpenedMusicName[0] == 0) return;
	m_szOpenedMusicName[0] = 0;

	ZGetSoundFMod()->CloseMusic();

	if (m_pMusicBuffer)
	{
		delete m_pMusicBuffer; m_pMusicBuffer = NULL;
	}
}

void ZSoundEngine::SetMusicVolume(float fVolume)
{
	if (!m_bSoundEnable) return;

	m_fMusicVolume = fVolume;
	ZGetSoundFMod()->SetMusicVolume(fVolume * 255);
}

float ZSoundEngine::GetMusicVolume(void)
{
	return m_fMusicVolume;
}

void ZSoundEngine::MusicEndCallback(void* pCallbackContext)
{
	ZSoundEngine* pSoundEngine = (ZSoundEngine*)pCallbackContext;
	if (pSoundEngine->m_bBattleMusic)
	{
		pSoundEngine->OpenMusic(BGMID_BATTLE, pSoundEngine->m_pfs);
		pSoundEngine->PlayMusic();
	}
}

void ZSoundEngine::PlayMusic(bool bLoop)
{
	if (!m_bSoundEnable || m_bMusicMute) return;

	if (m_bBattleMusic)
	{
		ZGetSoundFMod()->PlayMusic(false);
	}
	else
	{
		ZGetSoundFMod()->PlayMusic(bLoop);
	}

	SetMusicVolume(m_fMusicVolume);
}

void ZSoundEngine::StopMusic()
{
	if (!m_bSoundEnable) return;
	ZGetSoundFMod()->StopMusic();
}

void ZSoundEngine::PlaySoundBladeConcrete(MMatchItemDesc* pDesc, rvector pos)
{
	if (pDesc != nullptr && strcmp("rubber", pDesc->m_szWeaponByFiber) == 0)
	{
		PlaySound("rubber_concrete", pos);
		return;
	}

	PlaySound("blade_concrete", pos);
}
void ZSoundEngine::PlaySoundBladeDamage(MMatchItemDesc* pDesc, rvector& pos)
{
	if (pDesc != nullptr && strcmp("rubber", pDesc->m_szWeaponByFiber) == 0)
	{
		PlaySound("rubber_damage", pos);
		return;
	}

	PlaySound("blade_damage", pos);
}
void ZSoundEngine::PlaySoundHangOnWall(MMatchItemDesc* pDesc, rvector& pos)
{
	if (pDesc != nullptr && strcmp("rubber", pDesc->m_szWeaponByFiber) == 0)
	{
		PlaySound("rubber_hangonwall", pos);
		return;
	}

	PlaySound("hangonwall", pos);
}
void ZSoundEngine::PlaySoundChargeComplete(MMatchItemDesc* pDesc, const rvector& pos)
{
	if (pDesc != nullptr && strcmp("rubber", pDesc->m_szWeaponByFiber) == 0)
	{
		PlaySound("fx2/FX_rubber_ChargeComplete", pos);
		return;
	}

	PlaySound("fx2/FX_ChargeComplete", pos);
}
void ZSoundEngine::PlaySoundSmash(MMatchItemDesc* pDesc, rvector& pos, bool bObserverTarget)
{
	if (pDesc != nullptr && strcmp("rubber", pDesc->m_szWeaponByFiber) == 0)
	{
		PlaySound(bObserverTarget ? "we_rubber_smash_2d" : "we_rubber_smash", pos);
		return;
	}

	PlaySound(bObserverTarget ? "we_smash_2d" : "we_smash", pos);
}

void ZSoundEngine::PlaySoundSheath(MMatchItemDesc* pDesc, const rvector& pos, bool bObserverTarget)
{
	if (pDesc != nullptr && strcmp("rubber", pDesc->m_szWeaponByFiber) == 0)
	{
		PlaySound("fx_rubber_sheath", pos, bObserverTarget);
		return;
	}

	PlaySound("fx_blade_sheath", pos, bObserverTarget);
}

void ZSoundEngine::PlaySEFire(MMatchItemDesc* pDesc, float x, float y, float z, bool bPlayer)
{
	if (!m_bSoundEnable || !pDesc)	return;

	if (pDesc->m_nType.Ref() == MMIT_RANGE || pDesc->m_nType.Ref() == MMIT_CUSTOM)
	{
		char* szSndName = pDesc->m_szFireSndName;
		if (bPlayer)
		{
			char szBuffer[64];
			sprintf(szBuffer, "%s_2d", szSndName);
#ifdef _SOUND_LOG
			mlog("%s stereo 2d sound is played..\n", szBuffer);
#endif
			char* szDefault;
			if (pDesc->m_nType.Ref() == MMIT_RANGE)
				szDefault = "we_rifle_fire_2d";
			else
				szDefault = szSndName;

			PlaySoundElseDefault(szBuffer, szDefault, rvector(x, y, z), bPlayer);
			return;
		}
		PlaySoundElseDefault(szSndName, "we_rifle_fire", rvector(x, y, z), bPlayer);
	}
}

void ZSoundEngine::PlaySEDryFire(MMatchItemDesc* pDesc, float x, float y, float z, bool bPlayer)
{
	if (!m_bSoundEnable || !pDesc)	return;

	if (pDesc->m_nType.Ref() == MMIT_RANGE || pDesc->m_nType.Ref() == MMIT_CUSTOM)
	{
		char* szSndName = pDesc->m_szDryfireSndName;
		SoundSource* pSS = GetSoundSource(szSndName, bPlayer);
		if (pSS == 0)
			PlaySound("762arifle_dryfire", rvector(x, y, z), bPlayer, false);
		else
			PlaySound(szSndName, rvector(x, y, z), bPlayer, false);
	}
}

void ZSoundEngine::PlaySEReload(MMatchItemDesc* pDesc, float x, float y, float z, bool bPlayer)
{
	if (!m_bSoundEnable || !pDesc)	return;

	if (pDesc->m_nType.Ref() == MMIT_RANGE)
	{
		char* szSndName = pDesc->m_szReloadSndName;
		if (bPlayer)
		{
			char szBuffer[64];
			sprintf(szBuffer, "%s_2d", szSndName);
#ifdef _SOUND_LOG
			mlog("%s stereo 2d sound is played..\n", szBuffer);
#endif
			PlaySoundElseDefault(szBuffer, "we_rifle_reload_2d", rvector(x, y, z), bPlayer);
		}
		PlaySoundElseDefault(szSndName, "we_rifle_reload", rvector(x, y, z), bPlayer);
	}
}

void ZSoundEngine::PlaySERicochet(float x, float y, float z)
{
	if (!m_bSoundEnable)	return;
	PlaySound("ricochet_concrete01", rvector(x, y, z), false, false);
}

void ZSoundEngine::PlaySEHitObject(float x, float y, float z, RBSPPICKINFO& info_)
{
	if (!m_bSoundEnable)	return;

	static const char* base_snd_name = "fx_bullethit_mt_";
	FSOUND_SAMPLE* pFS = NULL;
	static char	buffer[256];

	if (info_.pNode == NULL) {
		return;
	}
	RMATERIAL* material_info = ZGetGame()->GetWorld()->GetBsp()->GetMaterial(info_.pNode, info_.nIndex);

	if (material_info == NULL) {
		return;
	}

	const char* temp = material_info->Name.c_str();
	size_t	size = strlen(temp);

	int index = (int)(size - 1);
	while (index >= 2)
	{
		if (temp[index--] == 't' && temp[index--] == 'm')
		{
			index += 4;
			break;
		}
	}

	if (index <= 2)
	{
		PlaySound("fx_bullethit_mt_con", rvector(x, y, z), false, false);
		return;
	}

	if (strcpy(buffer, base_snd_name) == NULL)
	{
		PlaySound("fx_bullethit_mt_con", rvector(x, y, z), false, false);
	}
	if (strncat(buffer, temp + index, size - index) == NULL)
	{
		PlaySound("fx_bullethit_mt_con", rvector(x, y, z), false, false);
	}

	PlaySoundElseDefault(buffer, "fx_bullethit_mt_con", rvector(x, y, z));
}

void ZSoundEngine::PlaySEHitBody(float x, float y, float z)
{
	if (!m_bSoundEnable)	return;
	PlaySound("fx_bullethit_mt_fsh", rvector(x, y, z), false, false);
}

bool ZSoundEngine::isPlayAble(char* name)
{
	if (!m_bSoundEnable)	return false;

	SESMAP::iterator i = m_SoundEffectSource.find(name);

	if (i == m_SoundEffectSource.end())
	{
		i = m_SoundEffectSource2D.find(name);
		if (i == m_SoundEffectSource2D.end())
			return false;
	}
	return true;
}

bool ZSoundEngine::isPlayAbleMtrl(char* name)
{
	if (!m_bSoundEnable)	return false;

	if (!name)		return false;
	if (!name[0])	return false;

	int len = (int)strlen(name);

	SESMAP::iterator node;
	FSOUND_SAMPLE* pFS = NULL;
	char filename[256];

	for (node = m_SoundEffectSource.begin(); node != m_SoundEffectSource.end(); ++node)
	{
		strcpy(filename, ((string)((*node).first)).c_str());

		if (strncmp(filename, name, len) == 0)
			return true;
	}

	for (node = m_SoundEffectSource2D.begin(); node != m_SoundEffectSource2D.end(); ++node)
	{
		strcpy(filename, ((string)((*node).first)).c_str());

		if (strncmp(filename, name, len) == 0)
			return true;
	}

	return false;
}

int ZSoundEngine::PlaySound(char* Name, const rvector& pos, bool bHero, bool bLoop, DWORD dwDelay)
{
	if (!m_bSoundEnable)	return 0;
	if (!m_b3DSoundUpdate) return 0;

	SoundSource* pSS = GetSoundSource(Name, bHero);
	if (pSS == 0)
	{
#ifdef _SOUND_LOG
		mlog("No %sSound Source[%s]\n", bHero ? "2d" : "3d", Name);
#endif
		return 0;
	}

	int priority = 0;
	if (!CheckCulling(Name, pSS, pos, bHero, &priority)) return 0;

#ifdef _DEBUG
#endif

	if (dwDelay > 0)
	{
		DelaySound DS;
		DS.dwDelay = dwDelay + timeGetTime();
		DS.pSS = pSS;
		DS.pos = pos;
		DS.priority = priority;
		DS.bPlayer = bHero;
		m_DelaySoundList.push_back(DS);
		return 0;
	}

	FSOUND_SAMPLE* pFS = pSS->pFS;
	if (pFS == NULL)
	{
#ifdef _SOUND_LOG
		mlog("FSOUND_SAMPLE is Null for Sound Source[%s]\n", Name);
#endif
		return 0;
	}
	return PlaySE(pFS, pos, priority, bHero, bLoop);
}

void ZSoundEngine::PlaySoundElseDefault(char* Name, char* NameDefault, rvector& pos, bool bHero, bool bLoop, DWORD dwDelay)
{
	if (!m_bSoundEnable)	return;
	if (!m_b3DSoundUpdate) return;

	SoundSource* pSS = GetSoundSource(Name, bHero);
	if (pSS == 0)
	{
		pSS = GetSoundSource(NameDefault, bHero);
		if (pSS == 0)
		{
#ifdef _SOUND_LOG
			mlog("No %sSound Source[%s] even Default Sound Source[%s]\n", bHero ? "2d" : "3d", Name, NameDefault);
#endif
			return;
		}
#ifdef _SOUND_LOG
		mlog("No %sSound Source[%s] so Use Default Sound Source[%s]\n", bHero ? "2d" : "3d", Name, NameDefault);
#endif
	}

	int priority = 0;
	if (!CheckCulling(Name, pSS, pos, bHero, &priority)) return;

	if (dwDelay > 0)
	{
		DelaySound DS;
		DS.dwDelay = dwDelay + timeGetTime();
		DS.pSS = pSS;
		DS.pos = pos;
		DS.priority = priority;
		DS.bPlayer = bHero;
		m_DelaySoundList.push_back(DS);
		return;
	}

	FSOUND_SAMPLE* pFS = pSS->pFS;
	if (pFS == NULL)
	{
#ifdef _SOUND_LOG
		mlog("FSOUND_SAMPLE is Null for Sound Source[%s]\n", Name);
#endif
		return;
	}

	PlaySE(pFS, pos, priority, bHero, bLoop);
}

int ZSoundEngine::PlaySound(char* Name, bool bLoop, DWORD dwDelay)
{
	if (!m_bSoundEnable)	return 0;

	SoundSource* pSS = GetSoundSource(Name, true);
	if (pSS == 0)
	{
#ifdef _SOUND_LOG
		mlog("No 2DSound Source[%s]\n", Name);
#endif
		return 0;
	}

	if (dwDelay > 0)
	{
		DelaySound DS;
		DS.dwDelay = dwDelay + timeGetTime();
		DS.pSS = pSS;
		DS.priority = 200;
		DS.bPlayer = true;
		m_DelaySoundList.push_back(DS);
		return 0;
	}

	FSOUND_SAMPLE* pFS = pSS->pFS;
	if (pFS == NULL)
	{
#ifdef _SOUND_LOG
		mlog("FSOUND_SAMPLE is Null for Sound Source[%s]\n", Name);
#endif
		return 0;
	}
	return PlaySE(pFS, rvector(0, 0, 0), 200, true, bLoop);
}

void ZSoundEngine::Run(void)
{
	DWORD currentTime = timeGetTime();
	if ((currentTime - m_Time) < m_DelayTime) return;
	m_Time = currentTime;

	auto ZSoundEngineRun = MBeginProfile("ZSoundEngine::Run");

	if (!m_bSoundEnable)	return;
	if (!m_b3DSoundUpdate)	return;

	if (ZGetGame())
	{
		rvector Pos = RCameraPosition;
		ZCharacter* pInterestCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
		if (pInterestCharacter != NULL)
		{
			Pos = pInterestCharacter->GetPosition();
			Pos.z += 170.f;
		}

		rvector Orientation = Pos - RCameraPosition;
		D3DXVec3Normalize(&Orientation, &Orientation);

		rvector right;
		D3DXVec3Cross(&right, &Orientation, &RCameraUp);

		UpdateAmbSound(Pos, right);

		for (DSLIST::iterator iter = m_DelaySoundList.begin(); iter != m_DelaySoundList.end();)
		{
			DelaySound DS = *iter;
			if (DS.dwDelay < m_Time)
			{
				PlaySE(DS.pSS->pFS, DS.pos, DS.priority, DS.bPlayer);
				iter = m_DelaySoundList.erase(iter);
				continue;
			}
			++iter;
		}

		m_ListenerPos = Pos;

		MBeginProfile(1004, "ZSoundEngine::Run : SetListener");
		if (m_bInverse)
			ZGetSoundFMod()->SetListener(&Pos, NULL, -Orientation.x, -Orientation.y, Orientation.z, 0, 0, 1);
		else
			ZGetSoundFMod()->SetListener(&Pos, NULL, Orientation.x, Orientation.y, Orientation.z, 0, 0, 1);

		MEndProfile(1004);
		MBeginProfile(33, "ZSoundEngine::Run : Update");
		ZGetSoundFMod()->Update();
		MEndProfile(33);
	}

	MEndProfile(ZSoundEngineRun);
}

const char* ZSoundEngine::GetBGMFileName(int nBgmIndex)
{
	static char m_stSndFileName[MAX_BGM][64] = { "Intro Retake2(D-R).ogg",
												"Theme Rock(D).ogg",
												"HardBgm3 Vanessa Retake(D).ogg",
												"HardBgm(D).ogg",
												"HardTech(D).ogg",
												"HardCore(D).ogg",
												"Ryswick style.ogg",
												"El-tracaz.ogg",
												"Industrial technolism.ogg",
												"TRANCE mission_tmix.ogg",
												"Vague words.ogg",
												"X-Fighter.ogg",
												"Fin.ogg" };

	static char szFileName[256] = "";
#define BGM_FOLDER		"Sound/BGM/"

	int nRealBgmIndex = nBgmIndex;
	if ((nBgmIndex >= BGMID_BATTLE) && (nBgmIndex < BGMID_FIN)) nRealBgmIndex = RandomNumber(BGMID_BATTLE, BGMID_FIN - 1);
	sprintf(szFileName, "%s%s", BGM_FOLDER, m_stSndFileName[nRealBgmIndex]);

	return szFileName;
}

bool ZSoundEngine::OpenMusic(int nBgmIndex, MZFileSystem* pfs)
{
	if (!m_bSoundEnable) return false;

	m_pfs = pfs;
	if (nBgmIndex == BGMID_BATTLE) m_bBattleMusic = true;
	else m_bBattleMusic = false;

	char szFileName[256];
	strcpy(szFileName, GetBGMFileName(nBgmIndex));

	return OpenMusic(szFileName, pfs);
}

bool ZSoundEngine::LoadResource(char* pFileName_, ZLoadingProgress* pLoading)
{
	if (!m_bSoundEnable)
	{
		return false;
	}

	MXmlDocument Data;
	if (!Data.LoadFromFile(pFileName_, ZGetFileSystem()))
	{
		return false;
	}

	MXmlElement root, chr, attr;

	float fTemp;
	char szSoundName[256];
	char szSoundFileName[256];
	int iType = 0;

	root = Data.GetDocumentElement();
	int iCount = root.GetChildNodeCount();

	for (int i = 0; i < iCount; ++i)
	{
		if (pLoading && (i % 10 == 0)) pLoading->UpdateAndDraw(float(i) / float(iCount));
		chr = root.GetChildNode(i);
		chr.GetTagName(szSoundName);
		if (szSoundName[0] == '#')
		{
			continue;
		}
		chr.GetAttribute(szSoundName, "NAME");
		strcpy(szSoundFileName, SOUNDEFFECT_DIR);
		strcat(szSoundFileName, szSoundName);
		strcat(szSoundFileName, SOUNDEFFECT_EXT);

		chr.GetAttribute(&iType, "type", 0);

		FSOUND_SAMPLE* pFS = NULL;
		FSOUND_SAMPLE* pFS2 = NULL;

		int flag = FSOUND_SIGNED | FSOUND_MONO;

		// 8Bits
		if (m_b8Bits) flag |= FSOUND_8BITS;
		else flag |= FSOUND_16BITS;

		switch (iType)
		{
		case 0:

			if (m_bHWMixing)
				flag |= FSOUND_HW3D;
			pFS = ZGetSoundFMod()->LoadWave(szSoundFileName, flag);

			break;

		case 1:

			if (m_bHWMixing)
				flag |= FSOUND_HW2D;
			else flag |= FSOUND_2D;
			pFS = ZGetSoundFMod()->LoadWave(szSoundFileName, flag);

			break;

		case 2:

			if (m_bHWMixing)
				flag |= FSOUND_HW3D;
			pFS = ZGetSoundFMod()->LoadWave(szSoundFileName, flag);

			if (m_bHWMixing)
			{
				flag &= ~FSOUND_HW3D;
				flag |= FSOUND_HW2D;
			}
			else flag |= FSOUND_2D;
			pFS2 = ZGetSoundFMod()->LoadWave(szSoundFileName, flag);

			break;

		case 3:

			flag &= ~FSOUND_MONO;
			flag |= FSOUND_STEREO;

			if (m_bHWMixing) flag |= FSOUND_HW2D;
			else flag |= FSOUND_2D;

			pFS2 = ZGetSoundFMod()->LoadWave(szSoundFileName, flag);

			break;

		case 4:

			pFS2 = ZGetSoundFMod()->LoadWave(szSoundFileName, FSOUND_LOOP_NORMAL | FSOUND_NORMAL | FSOUND_2D);
			break;

		case 5:
			pFS2 = ZGetSoundFMod()->LoadWave(szSoundFileName, FSOUND_LOOP_NORMAL | FSOUND_SIGNED | FSOUND_STEREO | FSOUND_16BITS | FSOUND_2D);
			break;

		case 6:
			pFS = ZGetSoundFMod()->LoadWave(szSoundFileName, FSOUND_LOOP_NORMAL | FSOUND_NORMAL);
			break;
		}

		SoundSource* pSS = NULL;

		if (pFS != NULL)
		{
			float min = ZDEF_MINDISTANCE;
			float max = ZDEF_MAXDISTANCE;
			if (chr.GetAttribute(&fTemp, "MINDISTANCE"))
				min = fTemp;
			pSS = new SoundSource;
			pSS->pFS = pFS;
			pSS->fMaxDistance = max;
			ZGetSoundFMod()->SetMinMaxDistance(pFS, min, 1000000000.0f);

			m_SoundEffectSource.insert(SESMAP::value_type(szSoundName, pSS));
		}
		if (pFS2 != NULL)
		{
			pSS = new SoundSource;
			pSS->pFS = pFS2;

			m_SoundEffectSource2D.insert(SESMAP::value_type(szSoundName, pSS));
		}
	}
	strcpy(m_SoundFileName, pFileName_);
	return true;
}

int ZSoundEngine::GetEnumDeviceCount()
{
	return ZGetSoundFMod()->GetNumDriver();
}

const char* ZSoundEngine::GetDeviceDescription(int index)
{
	return ZGetSoundFMod()->GetDriverName(index);
}

void ZSoundEngine::SetMusicMute(bool b)
{
	if (b == m_bMusicMute) return;

	m_bMusicMute = b;

	if (!m_bSoundEnable) return;

	ZGetSoundFMod()->SetMusicMute(b);
	if (!b)	SetMusicVolume(m_fMusicVolume);
}

int ZSoundEngine::PlaySE(FSOUND_SAMPLE* pFS, const rvector& pos, int Priority, bool bPlayer, bool bLoop)
{
	if (!m_bSoundEnable || m_bEffectMute || pFS == NULL) return -1;
	return ZGetSoundFMod()->Play(pFS, &pos, NULL, m_fEffectVolume * 255, Priority, bPlayer, bLoop);
}

void ZSoundEngine::StopLoopSound()
{
}

void ZSoundEngine::StopSound(int iChannel)
{
	if (!m_bSoundEnable) return;
	ZGetSoundFMod()->StopSound(iChannel);
}

void ZSoundEngine::SetEffectMute(bool b)
{
	m_bEffectMute = b;
}

void ZSoundEngine::Set3DSoundUpdate(bool b)
{
	m_b3DSoundUpdate = b;
	if (!b)
	{
		ZGetSoundFMod()->StopSound();
	}
}

bool ZSoundEngine::Reload()
{
	mlog("Reload Sound Sources...\n");
	for (SESMAP::iterator iter = m_SoundEffectSource.begin(); iter != m_SoundEffectSource.end(); ++iter)
	{
		SoundSource* pSS = iter->second;
		SAFE_DELETE(pSS);
	}
	m_SoundEffectSource.clear();
	for (SESMAP::iterator iter = m_SoundEffectSource2D.begin(); iter != m_SoundEffectSource2D.end(); ++iter)
	{
		SoundSource* pSS = iter->second;
		SAFE_DELETE(pSS);
	}
	m_SoundEffectSource2D.clear();
	return LoadResource(m_SoundFileName);
}

void ZSoundEngine::SetAmbientSoundBox(char* Name, rvector& pos1, rvector& pos2, bool b2d)
{
	AmbSound AS;
	AS.type = AS_AABB;
	if (b2d) AS.type |= AS_2D;
	else AS.type |= AS_3D;
	AS.pSS = GetSoundSource(Name, b2d);
	if (AS.pSS == NULL)
	{
		return;
	}
	strcpy(AS.szSoundName, Name);
	AS.iChannel = -1;
	AS.pos[0] = pos1;
	AS.pos[1] = pos2;
	AS.center = (pos1 + pos2) * 0.5f;
	AS.dx = AS.pos[1].x - AS.center.x;
	AS.dy = AS.pos[1].y - AS.center.y;
	AS.dz = AS.pos[1].z - AS.center.z;
	m_AmbientSoundList.push_back(AS);
	if (!b2d)
	{
		float length = D3DXVec3Length(&rvector(AS.dx, AS.dy, AS.dz));
		FSOUND_Sample_SetMinMaxDistance(AS.pSS->pFS, length * 0.1f, length);
	}
}

void ZSoundEngine::SetAmbientSoundSphere(char* Name, rvector& pos, float radius, bool b2d)
{
	AmbSound AS;
	AS.type = AS_SPHERE;
	if (b2d) AS.type |= AS_2D;
	else AS.type |= AS_3D;
	AS.pSS = GetSoundSource(Name, b2d);
	if (AS.pSS == NULL)
	{
		return;
	}
	strcpy(AS.szSoundName, Name);
	AS.iChannel = -1;
	AS.radius = radius;
	AS.center = pos;
	m_AmbientSoundList.push_back(AS);
	if (!b2d) FSOUND_Sample_SetMinMaxDistance(AS.pSS->pFS, radius * 0.1f, radius);
}

void ZSoundEngine::ClearAmbientSound()
{
	for (ASLIST::iterator iter = m_AmbientSoundList.begin(); iter != m_AmbientSoundList.end(); )
	{
		AmbSound* AS = &(*iter);

		if (AS->iChannel != -1)
		{
			ZGetSoundFMod()->StopSound(AS->iChannel);
			SetEffectVolume(AS->iChannel, m_fEffectVolume);
			AS->iChannel = -1;
		}
		iter = m_AmbientSoundList.erase(iter);
	}
}

void ZSoundEngine::UpdateAmbSound(rvector& Pos, rvector& Ori)
{
	for (ASLIST::iterator iter = m_AmbientSoundList.begin(); iter != m_AmbientSoundList.end(); ++iter)
	{
		AmbSound* AS = &(*iter);

		if (AS == NULL) continue;

		float t = GetArea(Pos, *AS);

		if (t <= 0)
		{
			if (AS->iChannel != -1)
			{
				SetEffectVolume(AS->iChannel, m_fEffectVolume);
				StopSound(AS->iChannel);
				AS->iChannel = -1;
			}
			continue;
		}

		if (AS->iChannel == -1)
		{
			AS->iChannel = PlaySE(AS->pSS->pFS, AS->center, 150, true);
		}

		if (AS->iChannel != -1)
		{
			float vol = m_fEffectVolume * t;
			SetEffectVolume(AS->iChannel, vol);
		}
	}
}

#define AS_TA_ATTENUATION_RATIO_SQ 0.7f
#define AS_TB_ATTENUATION_RATIO_SQ 0.1f

#define AS_TA_AMP_COEFFICIENT 5.0f
#define AS_TB_AMP_COEFFICIENT 1.5f

float ZSoundEngine::GetArea(rvector& Pos, AmbSound& a)
{
	if (a.type & AS_AABB)
	{
		float dX = fabs(Pos.x - a.center.x);
		float dY = fabs(Pos.y - a.center.y);
		float dZ = fabs(Pos.z - a.center.z);
		if (dX < a.dx && dY < a.dy && dZ < a.dz)
		{
			return min((a.dx - dX) / a.dx * (a.dy - dY) / a.dy * (a.dz - dZ) / a.dz * ((a.type & AS_2D) ? AS_TA_AMP_COEFFICIENT : AS_TB_AMP_COEFFICIENT), 1.0f);
		}
		return -1;
	}
	else if (a.type & AS_SPHERE)
	{
		float length = D3DXVec3Length(&(a.center - Pos));
		float radius = a.radius;
		if (length >= radius) return -1;
		float sacred = radius * ((a.type & AS_2D) ? AS_TA_ATTENUATION_RATIO_SQ : AS_TB_ATTENUATION_RATIO_SQ);
		if (length <= sacred) return 1;
		return (radius - length) / (radius - sacred);
	}
	return -1;
}

void ZSoundEngine::SetVolumeControlwithDuration(float fStartPercent, float fEndPercent, DWORD dwDuration, bool bEffect, bool bBGM)
{
	m_bEffectVolControl = bEffect;
	m_bBGMVolControl = bBGM;

	DWORD currentTime = timeGetTime();
	DWORD endTime = currentTime + dwDuration;
	int nUpdate = (endTime - currentTime) / m_DelayTime;

	float startEffectVol, startBGMVol;

	if (bEffect && !m_bEffectMute)
	{
		startEffectVol = fStartPercent * m_fEffectVolume;
		m_fEffectVolEnd = fEndPercent * m_fEffectVolume;
		m_fEffectVolFactor = (m_fEffectVolEnd - startEffectVol) / nUpdate;
		m_fEffectVolume = startEffectVol;
	}

	if (bBGM && !m_bMusicMute)
	{
		startBGMVol = fStartPercent * m_fMusicVolume;
		m_fBGMVolEnd = fEndPercent * m_fMusicVolume;
		m_fBGMVolFactor = (m_fBGMVolEnd - startBGMVol) / nUpdate;
		m_fMusicVolume = startBGMVol;
	}
}

void ZSoundEngine::PlayVoiceSound(char* szName)
{
#ifndef _VOICE_EFFECT
	return;
#endif

	if (!m_bSoundEnable)	return;

	SoundSource* pSS = GetSoundSource(szName, true);
	if (pSS == 0)
	{
		return;
	}

	FSOUND_SAMPLE* pFS = pSS->pFS;
	if (pFS == NULL)
	{
		return;
	}
	PlaySE(pFS, rvector(0, 0, 0), 254, true, false);
}

bool ZSoundEngine::LoadNPCResource(MQUEST_NPC nNPC, ZLoadingProgress* pLoading)
{
	FSOUND_SAMPLE* pFS = NULL;
	FSOUND_SAMPLE* pFS2 = NULL;

	int flag = FSOUND_SIGNED | FSOUND_MONO | FSOUND_16BITS;
	if (m_bHWMixing) flag |= FSOUND_HW3D;

	for (int i = 0; i < NPC_SOUND_END; i++)
	{
		MQuestNPCInfo* pNPCInfo = ZGetQuest()->GetNPCCatalogue()->GetInfo(nNPC);
		if (pNPCInfo == NULL) return false;

		if (pNPCInfo->szSoundName[i][0] == 0) continue;
		char szSoundFileName[256] = "";

		strcpy(szSoundFileName, const_cast<char*>(SOUNDNPC_DIR));
		strcat(szSoundFileName, pNPCInfo->szSoundName[i]);
		strcat(szSoundFileName, const_cast<char*>(SOUNDEFFECT_EXT));

		pFS = ZGetSoundFMod()->LoadWave(szSoundFileName, flag);

		if (pFS != NULL)
		{
			float min = 500.0f;
			float max = ZDEF_MAXDISTANCE;

			SoundSource* pSS = new SoundSource;
			pSS->pFS = pFS;
			pSS->fMaxDistance = max;
			ZGetSoundFMod()->SetMinMaxDistance(pFS, min, 1000000000.0f);

			m_SoundEffectSource.insert(SESMAP::value_type(szSoundFileName, pSS));
		}
	}

	m_ASManager.insert(nNPC);

	return true;
}

void ZSoundEngine::ReleaseNPCResources()
{
}

void ZSoundEngine::PlayNPCSound(MQUEST_NPC nNPC, MQUEST_NPC_SOUND nSound, rvector& pos, bool bMyKill)
{
	MQuestNPCInfo* pNPCInfo = ZGetQuest()->GetNPCCatalogue()->GetInfo(nNPC);
	if (pNPCInfo == NULL) return;

	if (pNPCInfo->szSoundName[nSound][0] != 0)
	{
		char szSoundFileName[256] = "";
		strcpy(szSoundFileName, const_cast<char*>(SOUNDNPC_DIR));
		strcat(szSoundFileName, pNPCInfo->szSoundName[nSound]);
		strcat(szSoundFileName, const_cast<char*>(SOUNDEFFECT_EXT));

		int nChannel = PlaySound(szSoundFileName, pos, false, false);
		if (nChannel != 0)
		{
			if (bMyKill)
			{
				ZGetSoundFMod()->SetMinMaxDistance(nChannel, 3500.0f, ZDEF_MAXDISTANCE);
			}
			else
			{
				ZGetSoundFMod()->SetMinMaxDistance(nChannel, 500.0f, ZDEF_MAXDISTANCE);
			}
		}
	}
}

bool ZSoundEngine::CheckCulling(char* szName, SoundSource* pSS, const rvector& vSoundPos, bool bHero, int* pnoutPriority)
{
	float fDistSq = D3DXVec3LengthSq(&(vSoundPos - m_ListenerPos));
	if (!bHero)
	{
		if (fDistSq > (pSS->fMaxDistance * pSS->fMaxDistance))
		{
#ifdef _SOUND_LOG
			mlog("Cull by Distance[%s]\n", szName);
#endif
			return false;
	}
}

	unsigned long int nNowTime = timeGetTime();

	if ((nNowTime - pSS->nLastPlayedTime) < 10)
	{
		if (strncmp("fx_dash", szName, 7))
		{
#ifdef _DEBUG
#endif
			return false;
		}
	}

	pSS->nLastPlayedTime = nNowTime;

	if (pnoutPriority)
	{
		*pnoutPriority = ((1 - fDistSq / pSS->fMaxDistance) * ZDEF_MAX_DISTANCE_PRIORITY);
	}

	return true;
}