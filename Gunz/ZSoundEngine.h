#ifndef _ZSOUNDENGINE_H
#define _ZSOUNDENGINE_H

class ZLoadingProgress;

#include "MZFileSystem.h"
#include "ZActorSound.h"
#include "MQuestNPC.h"

#include <map>
#include <string>
using namespace std;

#pragma comment(lib, "dxguid.lib")

struct MMatchItemDesc;
struct FSOUND_SAMPLE;

struct SoundSource
{
	FSOUND_SAMPLE* pFS;
	float fMaxDistance;
	unsigned long int	nLastPlayedTime;
	SoundSource()
	{
		pFS = 0;
		fMaxDistance = 1000000000.0f;
		nLastPlayedTime = 0;
	}
};

struct DelaySound
{
	SoundSource* pSS;
	DWORD dwDelay;
	rvector pos;
	int priority;
	bool bPlayer;
	DelaySound()
	{
		pSS = 0;
		dwDelay = 0;
		priority = 0;
		bPlayer = false;
	}
};

struct AmbSound
{
	int type;
	SoundSource* pSS;
	rvector pos[2];
	float radius;
	rvector center;
	int iChannel;
	float dx, dy, dz;
	char szSoundName[64];
	AmbSound()
	{
		type = 0;
		pSS = NULL;
		iChannel = -1;
		dx = dy = dz = 0;
		szSoundName[0] = 0;
	}
};

typedef std::map<std::string, SoundSource*> SESMAP;
typedef std::list<DelaySound> DSLIST;
typedef std::list<AmbSound> ASLIST;

class ZSoundEngine
{
private:
	char					m_SoundFileName[256];
	ZActorSoundManager		m_ASManager;
protected:
	char* m_pMusicBuffer;
	char			m_szOpenedMusicName[256];

	SESMAP			m_SoundEffectSource;
	SESMAP			m_SoundEffectSource2D;
	DSLIST			m_DelaySoundList;
	ASLIST			m_AmbientSoundList;

	float			m_fEffectVolume;
	float			m_fMusicVolume;

	bool			m_bEffectMute;
	bool			m_bMusicMute;
	bool			m_bSoundEnable;
	bool			m_b3DSound;
	bool			m_b3DSoundUpdate;
	rvector			m_ListenerPos;
	bool			m_bInverse;
	bool			m_b8Bits;
	bool			m_bHWMixing;

	DWORD			m_Time;
	DWORD			m_DelayTime;

	bool			m_bEffectVolControl;
	float			m_fEffectVolFactor;
	float			m_fEffectVolEnd;

	bool			m_bBGMVolControl;
	float			m_fBGMVolFactor;
	float			m_fBGMVolEnd;

	MZFileSystem* m_pfs;
	bool			m_bBattleMusic;
	const char* GetBGMFileName(int nBgmIndex);
	bool OpenMusic(const char* szFileName, MZFileSystem* pfs);
	bool CheckCulling(const char* szName, SoundSource* pSS, const rvector& vSoundPos, bool bHero, int* pnoutPriority = NULL);
	static void MusicEndCallback(void* pCallbackContext);
public:
	ZSoundEngine();
	virtual ~ZSoundEngine();
	bool Create(HWND hwnd, bool bHWMixing = false, ZLoadingProgress* pLoading = NULL);
	bool Reset(HWND hwnd, bool bHWMixing);
	void Destroy();
	bool LoadResource(char* pFileName_, ZLoadingProgress* pLoading = NULL);
	bool Reload();

	bool OpenMusic(int nBgmIndex, MZFileSystem* pfs);
	void CloseMusic();
	void PlayMusic(bool bLoop = true);
	void StopMusic();
	void SetMusicVolume(float fVolume);
	float GetMusicVolume(void);
	void SetMusicMute(bool b);

	void SetEffectVolume(float fVolume);
	void SetEffectVolume(int iChannel, float fVolume);
	void StopLoopSound();
	void StopSound(int iChannel);
	void SetEffectMute(bool b);
	bool SetSamplingBits(bool b8bit);
	void SetInverseSound(bool bInverse) { m_bInverse = bInverse; }

	int PlaySE(FSOUND_SAMPLE* pFS, const rvector& pos, int Priority, bool bPlayer = false, bool bLoop = false);

	void PlaySoundBladeConcrete(MMatchItemDesc* pDesc, rvector pos);
	void PlaySoundBladeDamage(MMatchItemDesc* pDesc, rvector& pos);
	void PlaySoundHangOnWall(MMatchItemDesc* pDesc, rvector& pos);
	void PlaySoundChargeComplete(MMatchItemDesc* pDesc, const rvector& pos);
	void PlaySoundSmash(MMatchItemDesc* pDesc, rvector& pos, bool bObserverTarget);
	void PlaySoundSheath(MMatchItemDesc* pDesc, const rvector& pos, bool bObserverTarget);

	void PlaySEFire(MMatchItemDesc* pDesc, float x, float y, float z, bool bPlayer = false);
	void PlaySEDryFire(MMatchItemDesc* pDesc, float x, float y, float z, bool bPlayer = false);
	void PlaySEReload(MMatchItemDesc* pDesc, float x, float y, float z, bool bPlayer = false);

	void PlaySERicochet(float x, float y, float z);
	void PlaySEHitObject(float x, float y, float z, RBSPPICKINFO& info_);

	void PlaySEHitBody(float x, float y, float z);

#undef PlaySound

	int PlaySound(const char* Name, const rvector& pos, bool bHero = false, bool bLoop = false, DWORD dwDelay = 0);
	void PlaySoundElseDefault(const char* Name, const char* NameDefault, const rvector& pos, bool bHero = false, bool bLoop = false, DWORD dwDelay = 0);
	int PlaySound(const char* Name, bool bLoop = false, DWORD dwDelay = 0);

	bool isPlayAble(char* name);
	bool isPlayAbleMtrl(char* name);

	void Run(void);
	void UpdateAmbSound(rvector& Pos, rvector& Ori);
	float GetArea(rvector& Pos, AmbSound& a);

	int GetEnumDeviceCount();
	const char* GetDeviceDescription(int index);

	FSOUND_SAMPLE* GetFS(const char* szName, bool bHero = false);
	SoundSource* GetSoundSource(const char* szName, bool bHero);

	void SetFramePerSecond(int n) { m_DelayTime = 1000 / n; }
	void Set3DSoundUpdate(bool b);
	bool Get3DSoundUpdate() const { return m_b3DSoundUpdate; }

	void SetAmbientSoundBox(char* Name, rvector& pos1, rvector& pos2, bool b2d = true);
	void SetAmbientSoundSphere(char* Name, rvector& pos, float radius, bool b2d = true);
	void ClearAmbientSound();

	void PlayVoiceSound(char* szName);

	void SetVolumeControlwithDuration(float fStartPercent, float fEndPercent, DWORD dwDuration, bool bEffect, bool bBGM);

	bool LoadNPCResource(MQUEST_NPC nNPC, ZLoadingProgress* pLoading = NULL);
	void ReleaseNPCResources();
	void PlayNPCSound(MQUEST_NPC nNPC, MQUEST_NPC_SOUND nSound, rvector& pos, bool bMyKill = true);
};

#define BGMID_INTRO				0
#define BGMID_LOBBY				1
#define BGMID_BATTLE			2
#define BGMID_FIN				12

#define MAX_BGM	13

#define VOICE_COOL					"nar/NAR01"
#define VOICE_NICE					"nar/NAR02"
#define VOICE_GREAT					"nar/NAR03"
#define VOICE_WONDERFUL				"nar/NAR04"
#define VOICE_KILLEDALL				"nar/NAR05"
#define VOICE_HEADSHOT				"nar/NAR06"
#define VOICE_FANTASTIC				"nar/NAR07"
#define VOICE_EXCELLENT				"nar/NAR08"
#define VOICE_UNBELIEVABLE			"nar/NAR09"
#define VOICE_GET_READY				"nar/NAR10"
#define VOICE_LETS_ROCK				"nar/NAR11"
#define VOICE_FINAL_ROUND			"nar/NAR27"
#define VOICE_YOU_WON				"nar/NAR12"
#define VOICE_YOU_LOSE				"nar/NAR13"
#define VOICE_RED_TEAM_WON			"nar/NAR14"
#define VOICE_BLUE_TEAM_WON			"nar/NAR15"
#define VOICE_DRAW_GAME				"nar/NAR16"
#define VOICE_REDTEAM_BOSS_DOWN		"nar/NAR19"
#define VOICE_BLUETEAM_BOSS_DOWN	"nar/NAR20"
#define VOICE_PLAYER_NOT_READY		"nar/NAR26"
#define VOICE_BERSERKER_DOWN		"nar/NAR28"
#define VOICE_GOT_BERSERKER			"nar/NAR29"
#define VOICE_QUEST_START_FAIL		"nar/NAR22"
#define VOICE_CTF					"nar/NAR30"
#define VOICE_NEW_INTRUDER			"nar/NAR17"
#define VOICE_NEW_CHALLENGER		"nar/NAR18"
#define VOICE_RED_HAS_FLAG			"nar/NAR32"
#define VOICE_BLUE_HAS_FLAG			"nar/NAR31"
#define VOICE_RED_FLAG_RETURN		"nar/NAR34"
#define VOICE_BLUE_FLAG_RETURN		"nar/NAR33"
#define VOICE_RED_TEAM_SCORE		"nar/NAR36"
#define VOICE_BLUE_TEAM_SCORE		"nar/NAR35"
#define VOICE_GO_BACK				""
#define VOICE_FOLLOW_ME				""
#define VOICE_BACK_ME_UP			""
#define VOICE_COVER_ME				""
#define VOICE_ENEMY_IN_SIGHT		""
#define VOICE_THANK_YOU				""
#define VOICE_SORRY					""
#define VOICE_HAHAHA				""
#define VOICE_OOPS					""
#define VOICE_UH_OH					""
#define VOICE_YES					""
#define VOICE_NO					""
#define VOICE_HEAL_ME_PLEASE		""
#define VOICE_MEDIC					""
#define VOICE_HOLD					""
#define VOICE_DEFEND				""
#define VOICE_NEGATIVE				""
#define VOICE_AFFIRMATIVE			""

#define VOICE_MAX					45

#endif