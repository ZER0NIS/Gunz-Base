#include "stdafx.h"

#include "ZGame.h"
#include <windows.h>
#include "MZFileSystem.h"
#include "RealSpace2.h"
#include "FileInfo.h"
#include "MDebug.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "ZConsole.h"
#include "MCommandLogFrame.h"
#include "ZInterface.h"
#include "ZGameInterface.h"
#include "ZApplication.h"
#include "ZCommandTable.h"
#include "ZPost.h"
#include "ZPostLocal.h"
#include "MStrEx.h"
#include "MMatchItem.h"
#include "ZEffectManager.h"
#include "ZEffectBillboard.h"
#include "Config.h"
#include "MProfiler.h"
#include "MMatchItem.h"
#include "ZScreenEffectManager.h"
#include "RParticleSystem.h"
#include "RDynamicLight.h"
#include "ZConfiguration.h"
#include "ZLoading.h"
#include "Physics.h"
#include "zeffectflashbang.h"
#include "ZInitialLoading.h"
#include "RealSoundEffect.h"
#include "RLenzFlare.h"
#include "ZWorldItem.h"
#include "ZMyInfo.h"
#include "ZNetCharacter.h"
#include "ZStencilLight.h"
#include "ZMap.h"
#include "ZEffectStaticMesh.h"
#include "ZEffectAniMesh.h"
#include "ZGameAction.h"
#include "ZSkyBox.h"
#include "ZFile.h"
#include "ZObject.h"
#include "ZCharacter.h"
#include "ZMapDesc.h"

#include "MXml.h"
#include "ZGameClient.h"
#include "MUtil.h"
#include "RMeshMgr.h"
#include "RVisualMeshMgr.h"
#include "RMaterialList.h"
#include "RAnimationMgr.h"
#include "ZCamera.h"
#include "Mint4R2.h"
#include "ZItemDesc.h"

#include "MMath.h"
#include "ZQuest.h"
#include "MMatchUtil.h"
#include "ZReplay.h"
#include "ZRuleBerserker.h"
#include "ZRuleDuelTournament.h"
#include "ZApplication.h"
#include "ZGameConst.h"

#include "ZRuleDuel.h"
#include "ZRuleDeathMatch.h"
#include "ZMyCharacter.h"
#include "MMatchCRC32XORCache.h"
#include "MMatchObjCache.h"

#include "ZModule_HealOverTime.h"

#ifdef LOCALE_NHNUSAA
#include "ZNHN_USA_Report.h"
#endif

_USING_NAMESPACE_REALSPACE2

#define ENABLE_CHARACTER_COLLISION
#define ENABLE_ADJUST_MY_DATA

void CheckMsgAboutChat(char* msg)
{
	int lenMsg = (int)strlen(msg);
	for (int i = 0; i < lenMsg; i++)
	{
		if (msg[i] == '\n' || msg[i] == '\r')
		{
			msg[i] = NULL;
			break;
		}
	}
}

struct RSnowParticle : public RParticle, CMemPoolSm<RSnowParticle>
{
	virtual bool Update(float fTimeElapsed);
};

bool RSnowParticle::Update(float fTimeElapsed)
{
	RParticle::Update(fTimeElapsed);

	if (position.z <= -1000.0f) return false;
	return true;
}

class ZSnowTownParticleSystem
{
private:
	RParticles* m_pParticles[3];
	bool IsSnowTownMap()
	{
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "snow", 4)) return true;

		return false;
	}
public:
	void Update(float fDeltaTime)
	{
		if (!IsSnowTownMap()) return;

#define SNOW_PARTICLE_COUNT_PER_SECOND		400

		int nSnowParticleCountPerSec = SNOW_PARTICLE_COUNT_PER_SECOND;
		switch (ZGetConfiguration()->GetVideo()->nEffectLevel)
		{
		case Z_VIDEO_EFFECT_HIGH:	break;
		case Z_VIDEO_EFFECT_NORMAL:	nSnowParticleCountPerSec = nSnowParticleCountPerSec / 4; break;
		case Z_VIDEO_EFFECT_LOW:	nSnowParticleCountPerSec = nSnowParticleCountPerSec / 8; break;
		default: nSnowParticleCountPerSec = 0; break;
		}

		int nCount = min(nSnowParticleCountPerSec * fDeltaTime, 20);
		for (int i = 0; i < nCount; i++)
		{
			RParticle* pp = new RSnowParticle();
			pp->ftime = 0;
			pp->position = rvector(RandomNumber(-8000.0f, 8000.0f), RandomNumber(-8000.0f, 8000.0f), 1500.0f);
			pp->velocity = rvector(RandomNumber(-40.0f, 40.0f), RandomNumber(-40.0f, 40.0f), RandomNumber(-150.0f, -250.0f));
			pp->accel = rvector(0, 0, -5.f);

			int particle_index = RandomNumber(0, 2);
			if (m_pParticles[particle_index]) m_pParticles[particle_index]->push_back(pp);
		}
	}
	void Create()
	{
		if (!IsSnowTownMap()) return;

		for (int i = 0; i < 3; i++)
		{
			m_pParticles[i] = NULL;
		}

		m_pParticles[0] = RGetParticleSystem()->AddParticles("sfx/water_splash.bmp", 25.0f);
		m_pParticles[1] = RGetParticleSystem()->AddParticles("sfx/water_splash.bmp", 10.0f);
		m_pParticles[2] = RGetParticleSystem()->AddParticles("sfx/water_splash.bmp", 5.0f);
	}
	void Destroy()
	{
		if (!IsSnowTownMap()) return;

		m_pParticles[0]->Clear();
		m_pParticles[1]->Clear();
		m_pParticles[2]->Clear();
	}
};

static ZSnowTownParticleSystem g_SnowTownParticleSystem;

float	g_fFOV = DEFAULT_FOV;
float	g_fFarZ = DEFAULT_FAR_Z;

extern sCharacterLight g_CharLightList[NUM_LIGHT_TYPE];

MUID tempUID(0, 0);
MUID g_MyChrUID(0, 0);

#define IsKeyDown(key) ((GetAsyncKeyState(key) & 0x8000)!=0)

void TestCreateEffect(int nEffIndex)
{
	float fDist = RandomNumber(0.0f, 100.0f);
	rvector vTar = rvector(RandomNumber(0.0f, 100.0f), RandomNumber(0.0f, 100.0f), RandomNumber(0.0f, 100.0f));
	rvector vPos = ZGetGame()->m_pMyCharacter->GetPosition();
	vPos.x += RandomNumber(0.0f, 100.0f);
	vPos.y += RandomNumber(0.0f, 100.0f);
	vPos.z += RandomNumber(0.0f, 100.0f);

	rvector vTarNormal = -RealSpace2::RCameraDirection;

	vTarNormal = rvector(RandomNumber(0.0f, 100.0f), RandomNumber(0.0f, 100.0f), RandomNumber(0.0f, 100.0f));

	ZCharacter* pCharacter = ZGetGame()->m_pMyCharacter;
	ZEffectManager* pEM = ZGetEffectManager();

	switch (nEffIndex)
	{
	case 0:
		pEM->AddReBirthEffect(vPos);
		break;
	case 1:
		pEM->AddLandingEffect(vPos, vTarNormal);
		break;
	case 2:
		pEM->AddGrenadeEffect(vPos, vTarNormal);
		break;
	case 3:
		pEM->AddRocketEffect(vPos, vTarNormal);
		break;
	case 4:
		pEM->AddRocketSmokeEffect(vPos);
		break;
	case 5:
		pEM->AddSwordDefenceEffect(vPos, -vTarNormal);
		break;
	case 6:
		pEM->AddSwordWaveEffect(vPos, 200, pCharacter);
		break;
	case 7:
		pEM->AddSwordUppercutDamageEffect(vPos, pCharacter->GetUID());
		break;
	case 8:
		pEM->AddBulletMark(vPos, vTarNormal);
		break;
	case 9:
		pEM->AddShotgunEffect(vPos, vPos, vTar, pCharacter);
		break;
	case 10:
		pEM->AddBloodEffect(vPos, vTarNormal);
		break;
	case 11:
		for (int i = 0; i < SEM_End; i++)
			pEM->AddSlashEffect(vPos, vTarNormal, i);
		break;
	case 12:
		pEM->AddSlashEffectWall(vPos, vTarNormal, 0);
		break;
	case 13:
		pEM->AddLightFragment(vPos, vTarNormal);
		break;
	case 14:
		pEM->AddDashEffect(vPos, vTarNormal, pCharacter);
		break;
	case 15:
		pEM->AddSmokeEffect(vPos);
		break;
	case 16:
		pEM->AddSmokeGrenadeEffect(vPos);
		break;
	case 17:
		pEM->AddGrenadeSmokeEffect(vPos, 1.0f, 0.5f, 123);
		break;
	case 18:
		pEM->AddWaterSplashEffect(vPos, vPos);
		break;
	case 19:
		pEM->AddWorldItemEatenEffect(vPos);
		break;
	case 20:
		pEM->AddCharacterIcon(pCharacter, 0);
		break;
	case 21:
		pEM->AddCommanderIcon(pCharacter, 0);
		break;
	case 22:
		pEM->AddChatIcon(pCharacter);
		break;
	case 23:
		pEM->AddLostConIcon(pCharacter);
		break;
	case 24:
		pEM->AddChargingEffect(pCharacter);
		break;
	case 25:
		pEM->AddChargedEffect(pCharacter);
		break;
	case 26:
		pEM->AddTrackFire(vPos);
		pEM->AddTrackFire(vPos);
		pEM->AddTrackFire(vPos);
		pEM->AddTrackFire(vPos);
		break;
	case 27:

		ZEffectWeaponEnchant * pEWE = pEM->GetWeaponEnchant(ZC_ENCHANT_FIRE);

		if (pEWE) {
			float fSIze = 105.f / 100.f;
			rvector vScale = rvector(0.6f * fSIze, 0.6f * fSIze, 0.9f * fSIze);
			pEWE->SetUid(pCharacter->GetUID());
			pEWE->SetAlignType(1);
			pEWE->SetScale(vScale);
			pEWE->Draw(timeGetTime());
		}

		break;
	}
}

float CalcActualDamage(ZObject* pAttacker, ZObject* pVictim, float fDamage)
{
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_BERSERKER)
	{
		ZRuleBerserker* pRule = (ZRuleBerserker*)ZGetGame()->GetMatch()->GetRule();
		if ((pAttacker) && (pAttacker != pVictim) && (pAttacker->GetUID() == pRule->GetBerserkerUID()))
		{
			return fDamage * BERSERKER_DAMAGE_RATIO;
		}
	}

	return fDamage;
}

void TestCreateEffects()
{
	int nCount = 100;

	for (int i = 0; i < nCount; i++)
	{
		int nEffIndex = RandomNumber(25, 28);
		TestCreateEffect(nEffIndex);
	}
}

ZGame::ZGame()
{
	m_bShowWireframe = false;
	m_pMyCharacter = NULL;

	m_bReserveObserver = false;

	for (int i = 0; i < ZLASTTIME_MAX; i++)
	{
		m_nLastTime[i] = timeGetTime();
	}

	m_fTime.Set_MakeCrc(0.0f);
	m_nReadyState = ZGAME_READYSTATE_INIT;
	m_pParticles = NULL;
	m_render_poly_cnt = 0;
	m_nReservedObserverTime = 0;
	m_nSpawnTime = 0;
	m_t = 0;

	m_bRecording = false;
	m_pReplayFile = NULL;

	m_bReplaying.Set_MakeCrc(false);
	m_bShowReplayInfo = true;
	m_bSpawnRequested = false;
	m_ReplayLogTime = 0;
	ZeroMemory(m_Replay_UseItem, sizeof(m_Replay_UseItem));

	m_pGameAction = new ZGameAction;

	CancelSuicide();
}

ZGame::~ZGame()
{
	delete m_pGameAction;
	g_SnowTownParticleSystem.Destroy();
	RSnowParticle::Release();
}

bool ZGame::Create(MZFileSystem* pfs, ZLoadingProgress* pLoading)
{
	ZGetGameClient()->CastStageBridgePeer(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());

	mlog("game create begin , type = %d\n", ZGetGameClient()->GetMatchStageSetting()->GetGameType());

	SetReadyState(ZGAME_READYSTATE_INIT);

#ifdef _QUEST
	{
		ZGetQuest()->OnGameCreate();
	}
#endif

	if (ZGetApplication()->GetLaunchMode() != ZApplication::ZLAUNCH_MODE_STANDALONE_AI &&
		ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType())) {
		for (int i = 0; i < ZGetQuest()->GetGameInfo()->GetMapSectorCount(); i++)
		{
			MQuestMapSectorInfo* pSecInfo = ZGetQuest()->GetSectorInfo(ZGetQuest()->GetGameInfo()->GetMapSectorID(i));
			ZGetWorldManager()->AddWorld(pSecInfo->szTitle);
		}
	}
	else {
		ZGetWorldManager()->AddWorld(ZGetGameClient()->GetMatchStageSetting()->GetMapName());
	}

	if (!ZGetWorldManager()->LoadAll(pLoading))
	{
		ZGetWorldManager()->Clear();
		return false;
	}

	ZGetWorldManager()->SetCurrent(0);

	mlog("ZGame::Create() :: ReloadAllAnimation Begin \n");
	ZGetMeshMgr()->ReloadAllAnimation();
	mlog("ZGame::Create() :: ReloadAllAnimation End \n");

	if (ZGetGameClient()->IsForcedEntry())
	{
		ZPostRequestPeerList(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
	}

	g_fFOV = DEFAULT_FOV;

	rvector dir = GetMapDesc()->GetWaitCamDir();
	rvector pos = GetMapDesc()->GetWaitCamPos();
	rvector up(0, 0, 1);
	RSetCamera(pos, pos + dir, up);

	int nModelID = -1;

	m_Match.Create();

	D3DMATERIAL9 mtrl;
	ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));

	mtrl.Diffuse.r = 1.0f;
	mtrl.Diffuse.g = 1.0f;
	mtrl.Diffuse.b = 1.0f;
	mtrl.Diffuse.a = 1.0f;

	mtrl.Ambient.r = 1.0f;
	mtrl.Ambient.g = 1.0f;
	mtrl.Ambient.b = 1.0f;
	mtrl.Ambient.a = 1.0f;

	RGetDevice()->SetMaterial(&mtrl);

	m_fTime.Set_CheckCrc(0.0f);
	m_bReserveObserver = false;

#ifdef _BIRDSOUND
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_BATTLE);
	ZApplication::GetSoundEngine()->PlayMusic();
#else
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_BATTLE, pfs);
	ZApplication::GetSoundEngine()->PlayMusic();
#endif

	m_CharacterManager.Clear();
	m_ObjectManager.Clear();

	m_pMyCharacter = (ZMyCharacter*)m_CharacterManager.Add(ZGetGameClient()->GetPlayerUID(), rvector(0.0f, 0.0f, 0.0f), true);

	{
		g_CharLightList[GUN].fLife = 300;
		g_CharLightList[GUN].fRange = 100;
		g_CharLightList[GUN].iType = GUN;
		g_CharLightList[GUN].vLightColor.x = 5.0f;
		g_CharLightList[GUN].vLightColor.y = 1.0f;
		g_CharLightList[GUN].vLightColor.z = 1.0f;

		g_CharLightList[SHOTGUN].fLife = 1000;
		g_CharLightList[SHOTGUN].fRange = 150;
		g_CharLightList[SHOTGUN].iType = SHOTGUN;
		g_CharLightList[SHOTGUN].vLightColor.x = 6.0f;
		g_CharLightList[SHOTGUN].vLightColor.y = 1.3f;
		g_CharLightList[SHOTGUN].vLightColor.z = 1.3f;

		g_CharLightList[CANNON].fLife = 1300;
		g_CharLightList[CANNON].fRange = 200;
		g_CharLightList[CANNON].iType = CANNON;
		g_CharLightList[CANNON].vLightColor.x = 7.0f;
		g_CharLightList[CANNON].vLightColor.y = 1.3f;
		g_CharLightList[CANNON].vLightColor.z = 1.3f;
	}

	ZGetFlashBangEffect()->SetBuffer();
	ZGetScreenEffectManager()->SetGaugeExpFromMyInfo();

#ifdef _BIRDSOUND

#else
	ZGetSoundEngine()->SetEffectVolume(Z_AUDIO_EFFECT_VOLUME);
	ZGetSoundEngine()->SetMusicVolume(Z_AUDIO_BGM_VOLUME);
#endif

	ZApplication::ResetTimer();
	m_GameTimer.Reset();

	ZGetInitialLoading()->SetPercentage(100.f);
	ZGetInitialLoading()->Draw(MODE_DEFAULT, 0, true);

#ifdef _BIRDSOUND

#else
	list<AmbSndInfo*> aslist = GetWorld()->GetBsp()->GetAmbSndList();
	for (list<AmbSndInfo*>::iterator iter = aslist.begin(); iter != aslist.end(); ++iter)
	{
		AmbSndInfo* pAS = *iter;
		if (pAS->itype & AS_AABB)
			ZGetSoundEngine()->SetAmbientSoundBox(pAS->szSoundName, pAS->min, pAS->max, (pAS->itype & AS_2D) ? true : false);
		else if (pAS->itype & AS_SPHERE)
			ZGetSoundEngine()->SetAmbientSoundSphere(pAS->szSoundName, pAS->center, pAS->radius, (pAS->itype & AS_2D) ? true : false);
	}
#endif

	MEMBER_SET_CHECKCRC(m_pMyCharacter->GetStatus(), nLoadingPercent, 100);
	ZPostLoadingComplete(ZGetGameClient()->GetPlayerUID(), 100);

	ZPostStageEnterBattle(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());

	char tmpbuf[128];
	_strtime(tmpbuf);

	mlog("game created ( %s )\n", tmpbuf);

	ZGetGameInterface()->GetCamera()->SetLookMode(ZCAMERA_DEFAULT);

	g_SnowTownParticleSystem.Create();

	return true;
}

void ZGame::Destroy()
{
	m_DataChecker.Clear();

	g_SnowTownParticleSystem.Destroy();

	SetClearColor(0);

	mlog("game destroy begin\n");

	ZGetGameClient()->AgentDisconnect();

	ZApplication::GetSoundEngine()->StopMusic();
	ZApplication::GetSoundEngine()->CloseMusic();

	mlog("Destroy sound engine()\n");

#ifdef _QUEST
	{
		ZGetQuest()->OnGameDestroy();
	}
#endif

	m_Match.Destroy();

	mlog("game destroy match destroy \n");

	if (ZGetGameClient()->IsForcedEntry())
	{
		ZGetGameClient()->ReleaseForcedEntry();

		ZGetGameInterface()->SerializeStageInterface();
	}

	ZPostStageLeaveBattle(ZGetGameClient()->GetPlayerUID(), ZGetGameInterface()->GetIsGameFinishLeaveBattle());

	ReleaseFlashBangEffect();

	RGetLenzFlare()->Clear();

	m_WeaponManager.Clear();
#ifdef _WORLD_ITEM_
	ZGetWorldItemManager()->Clear();
#endif

#ifdef _BIRDSOUND

#else
	ZGetSoundEngine()->ClearAmbientSound();
#endif

	m_ObserverCommandList.Destroy();
	m_ReplayCommandList.Destroy();
	m_DelayedCommandList.Destroy();

	ZGetEffectManager()->Clear();
	ZGetScreenEffectManager()->Clear();

	ZGetWorldManager()->Clear();

	ZGetCharacterManager()->Clear();

	char tmpbuf[128];
	_strtime(tmpbuf);

	mlog("game destroyed ( %s )\n", tmpbuf);
}

bool ZGame::CreateMyCharacter(MTD_CharInfo* pCharInfo)
{
	if (!m_pMyCharacter) return false;

	m_pMyCharacter->Create(pCharInfo);
	m_pMyCharacter->SetVisible(true);

	ZGetEffectManager()->AddBerserkerIcon(m_pMyCharacter);

	mlog("Create character : Name=%s Level=%d \n", pCharInfo->szName, pCharInfo->nLevel);
	return true;
}

bool ZGame::CheckGameReady()
{
	if (GetReadyState() == ZGAME_READYSTATE_RUN) {
		return true;
	}
	else if (GetReadyState() == ZGAME_READYSTATE_INIT) {
		SetReadyState(ZGAME_READYSTATE_WAITSYNC);
		ZPostRequestTimeSync(GetTickTime());
		return false;
	}
	else if (GetReadyState() == ZGAME_READYSTATE_WAITSYNC) {
		return false;
	}
	return false;
}

void ZGame::OnGameResponseTimeSync(unsigned int nLocalTimeStamp, unsigned int nGlobalTimeSync)
{
	ZGameTimer* pTimer = GetGameTimer();
	int nCurrentTick = pTimer->GetGlobalTick();
	int nDelay = (nCurrentTick - nLocalTimeStamp) / 2;
	int nOffset = (int)nGlobalTimeSync - (int)nCurrentTick + nDelay;

	pTimer->SetGlobalOffset(nOffset);

	SetReadyState(ZGAME_READYSTATE_RUN);
}

void ZGame::Update(float fElapsed)
{
	if (CheckGameReady() == false) {
		OnCameraUpdate(fElapsed);
		return;
	}

	GetWorld()->Update(fElapsed);

	ZGetEffectManager()->Update(fElapsed);
	ZGetScreenEffectManager()->UpdateEffects();

	m_GameTimer.UpdateTick(timeGetTime());
	m_fTime.Set_CheckCrc(m_fTime.Ref() + fElapsed);
	m_fTime.ShiftHeapPos();
	m_bReplaying.ShiftHeapPos_CheckCrc();

	m_ObjectManager.Update(fElapsed);

	if (m_pMyCharacter && !m_bReplaying.Ref())
	{
		PostBasicInfo();

		if (ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUELTOURNAMENT)
			PostHPAPInfo();
		else
			PostDuelTournamentHPAPInfo();

		PostPeerPingInfo();
		PostSyncReport();
	}

	CheckMyCharDead(fElapsed);
	if (m_bReserveObserver)
	{
		OnReserveObserver();
	}

	UpdateCombo();

	g_SnowTownParticleSystem.Update(fElapsed);

#ifdef _WORLD_ITEM_
	ZGetWorldItemManager()->update();
#endif
	m_Match.Update(fElapsed);

	if (m_bReplaying.Ref())
		OnReplayRun();
	if (ZGetGameInterface()->GetCombatInterface()->GetObserverMode() || m_bReplaying.Ref())
		OnObserverRun();

	ProcessDelayedCommand();

#ifdef _QUEST
	{
		ZGetQuest()->OnGameUpdate(fElapsed);
	}
#endif

	RGetParticleSystem()->Update(fElapsed);

	if (Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->Update();

	OnCameraUpdate(fElapsed);
	m_WeaponManager.Update();
}

void ZGame::OnCameraUpdate(float Elapsed)
{
	if (m_Match.GetRoundState() == MMATCH_ROUNDSTATE_PREPARE)
	{
		rvector dir = GetMapDesc()->GetWaitCamDir();
		rvector pos = GetMapDesc()->GetWaitCamPos();
		rvector up(0, 0, 1);

		RSetCamera(pos, pos + dir, up);
	}
	else
	{
		ZGetGameInterface()->GetCamera()->Update(Elapsed);
	}
}
void ZGame::CheckMyCharDeadByCriticalLine()
{
	MUID uidAttacker = MUID(0, 0);
	bool bReturnValue = m_pMyCharacter->GetPosition().z >= DIE_CRITICAL_LINE;
	if (m_pMyCharacter->GetPosition().z >= DIE_CRITICAL_LINE)
		PROTECT_DEBUG_REGISTER(bReturnValue)
		return;

	uidAttacker = m_pMyCharacter->GetLastThrower();

	ZObject* pAttacker = ZGetObjectManager()->GetObject(uidAttacker);
	if (pAttacker == NULL || !CanAttack(pAttacker, m_pMyCharacter))
	{
		uidAttacker = ZGetMyUID();
		pAttacker = m_pMyCharacter;
	}

	m_pMyCharacter->OnDamaged(pAttacker, m_pMyCharacter->GetPosition(), ZD_FALLING, MWT_NONE, m_pMyCharacter->GetHP());
	ZChatOutput(ZMsg(MSG_GAME_FALL_NARAK));
}
void ZGame::CheckMyCharDeadUnchecked()
{
	MUID uidAttacker = MUID(0, 0);
	bool bCheck = (m_pMyCharacter->IsDie() == true) | (m_pMyCharacter->GetHP() > 0);
	if ((m_pMyCharacter->IsDie() == true) || (m_pMyCharacter->GetHP() > 0))
		PROTECT_DEBUG_REGISTER(bCheck)
		return;

	if (uidAttacker == MUID(0, 0) && m_pMyCharacter->GetLastAttacker() != MUID(0, 0))
		uidAttacker = m_pMyCharacter->GetLastAttacker();

	if (GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_FINISH)
	{
		m_pMyCharacter->ActDead();
		m_pMyCharacter->Die();
		return;
	}

	ZPostDie(uidAttacker);

	if (!ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
	{
		ZPostGameKill(uidAttacker);
	}
	else
	{
		ZPostQuestGameKill();
	}

	if (ZApplication::GetGameInterface()->IsLeaveBattleReserved() == true)
		ZApplication::GetGameInterface()->ReserveLeaveBattle();
}

void ZGame::CheckMyCharDead(float fElapsed)
{
	bool bReturnValue = !m_pMyCharacter || m_pMyCharacter->IsDie();
	if (!m_pMyCharacter || m_pMyCharacter->IsDie())
		PROTECT_DEBUG_REGISTER(bReturnValue)
		return;

	CheckMyCharDeadByCriticalLine();
	CheckMyCharDeadUnchecked();
}

void ZGame::OnPreDraw()
{
	__BP(19, "ZGame::sub1");

	RSetProjection(g_fFOV, DEFAULT_NEAR_Z, g_fFarZ);

	bool bTrilinear = RIsTrilinear();

	RGetDevice()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, bTrilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE);
	RGetDevice()->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(1, D3DSAMP_MIPFILTER, bTrilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE);

	if (m_bShowWireframe) {
		RGetDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
		GetWorld()->SetFog(false);
	}
	else {
		RGetDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		GetWorld()->SetFog(true);
	}

	GetWorld()->GetBsp()->SetWireframeMode(m_bShowWireframe);

	rmatrix initmat;
	D3DXMatrixIdentity(&initmat);
	RGetDevice()->SetTransform(D3DTS_WORLD, &initmat);
	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, true);

	LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();
	pd3dDevice->SetTexture(0, NULL);
	pd3dDevice->SetTexture(1, NULL);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);

	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);

	if (m_pMyCharacter)
	{
		if (ZGetConfiguration()->GetVideo()->bDynamicLight)
		{
			rvector pos = m_pMyCharacter->GetPosition();
			RGetDynamicLightManager()->SetPosition(pos);
		}
	}

	__EP(19);
}

int g_debug_render_mode = 0;

extern MDrawContextR2* g_pDC;

void ZGame::Draw()
{
#ifdef _DEBUG
	if (GetAsyncKeyState(VK_UP))
	{
		ZPostSkill(ZC_SKILL_SPLASHSHOT, MMCIP_MELEE);
	}
#endif

	__BP(20, "ZGame::Draw");

	RRESULT isOK = RIsReadyToRender();

	if (isOK == R_NOTREADY)
	{
		__EP(20);
		return;
	}

	OnPreDraw();

	rmatrix _mat;
	RGetDevice()->GetTransform(D3DTS_WORLD, &_mat);

	__BP(21, "ZGame::Draw::DrawWorld");
	GetWorld()->Draw();
	__EP(21);

	ZMapDesc* pMapDesc = GetMapDesc();

	if (pMapDesc) {
		pMapDesc->DrawMapDesc();
	}

	if (m_Match.GetRoundState() != MMATCH_ROUNDSTATE_PREPARE)
	{
		__BP(22, "ZGame::Draw::DrawCharacters");

		m_ObjectManager.Draw();

		__EP(22);

		m_render_poly_cnt = RealSpace2::g_poly_render_cnt;
	}

	RGetDevice()->SetTransform(D3DTS_WORLD, &_mat);

	ZGetWorldItemManager()->Draw(0, GetWorld()->GetWaterHeight(), GetWorld()->IsWaterMap());

	m_WeaponManager.Render();

	__BP(50, "ZGame::DrawObjects");

	GetWorld()->GetBsp()->DrawObjects();

	__EP(50);

	__BP(17, "ZGame::Draw::Reflection");

	GetWorld()->GetWaters()->Render();

	__EP(17);

	if (m_Match.GetRoundState() != MMATCH_ROUNDSTATE_PREPARE)
	{
		__BP(23, "ZGame::Draw::DrawWeapons and effects");
#ifndef _PUBLISH
#endif

		ZGetEffectManager()->Draw(timeGetTime());

		__EP(23);
	}

#ifdef _WORLD_ITEM_
	__BP(34, "ZGame::Draw::ZGetWorldItemManager");

	ZGetWorldItemManager()->Draw(1, GetWorld()->GetWaterHeight(), GetWorld()->IsWaterMap());

	__EP(34);
#endif

	__BP(35, "ZGame::Draw::RGetParticleSystem");

	RGetParticleSystem()->Draw();

	__EP(35);

	__BP(36, "ZGame::Draw::LenzFlare");

	if (RReadyLenzFlare())
	{
		RGetLenzFlare()->Render(RCameraPosition, GetWorld()->GetBsp());
	}

	RSetProjection(DEFAULT_FOV, DEFAULT_NEAR_Z, g_fFarZ);
	RSetFog(FALSE);

	__EP(36);

	__BP(37, "ZGame::Draw::FlashBangEffect");

	if (IsActivatedFlashBangEffect())
	{
		ShowFlashBangEffect();
	}

	__BP(505, "ZGame::Draw::RenderStencilLight");
	if (Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->Render();
	__EP(505);

	__EP(37);

	__BP(38, "ZGame::Draw::DrawGameMessage");

	m_Match.OnDrawGameMessage();

	__EP(38);

	__EP(20);
}

void ZGame::DrawDebugInfo()
{
	char szTemp[256] = "";
	int n = 20;
	g_pDC->SetColor(MCOLOR(0xFFffffff));

	for (ZCharacterManager::iterator itor = m_CharacterManager.begin(); itor != m_CharacterManager.end(); ++itor)
	{
		ZCharacter* pCharacter = (*itor).second;
		sprintf(szTemp, "Pos = %6.3f %6.3f %6.3f  Dir = %6.3f %6.3f %6.3f", pCharacter->GetPosition().x,
			pCharacter->GetPosition().y, pCharacter->GetPosition().z,
			pCharacter->m_Direction.x, pCharacter->m_Direction.y, pCharacter->m_Direction.z);
		g_pDC->Text(20, n, szTemp);
		n += 15;

		RVisualMesh* pVMesh = pCharacter->m_pVMesh;

		AniFrameInfo* pAniLow = pVMesh->GetFrameInfo(ani_mode_lower);
		AniFrameInfo* pAniUp = pVMesh->GetFrameInfo(ani_mode_upper);

		sprintf(szTemp, "%s frame down %d / %d ", pAniLow->m_pAniSet->GetName(), pAniLow->m_nFrame, pAniLow->m_pAniSet->GetMaxFrame());
		g_pDC->Text(20, n, szTemp);
		n += 15;

		if (pAniUp->m_pAniSet)
		{
			sprintf(szTemp, "%s frame up %d / %d ", pAniUp->m_pAniSet->GetName(), pAniUp->m_nFrame, pAniUp->m_pAniSet->GetMaxFrame());
			g_pDC->Text(20, n, szTemp);
			n += 15;
		}
	}
}

void ZGame::Draw(MDrawContextR2& dc)
{
}

void ZGame::ParseReservedWord(char* pszDest, const char* pszSrc)
{
	char szSrc[256];
	char szWord[256];

	strcpy(szSrc, pszSrc);

	char szOut[256];	ZeroMemory(szOut, 256);
	int nOutOffset = 0;

	char* pszNext = szSrc;
	while (*pszNext != NULL) {
		pszNext = MStringCutter::GetOneArg(pszNext, szWord);

		if ((*szWord == '$') && (stricmp(szWord, "$player") == 0)) {
			sprintf(szWord, "%d %d", m_pMyCharacter->GetUID().High, m_pMyCharacter->GetUID().Low);
		}
		else if ((*szWord == '$') && (stricmp(szWord, "$target") == 0)) {
			sprintf(szWord, "%d %d", m_pMyCharacter->GetUID().High, m_pMyCharacter->GetUID().Low);
		}

		strcpy(szOut + nOutOffset, szWord);	nOutOffset += (int)strlen(szWord);
		if (*pszNext) {
			strcpy(szOut + nOutOffset, " ");
			nOutOffset++;
		}
	}
	strcpy(pszDest, szOut);
}

#include "ZMessages.h"

extern bool g_bProfile;

bool IsIgnoreObserverCommand(int nID)
{
	switch (nID) {
	case MC_PEER_PING:
	case MC_PEER_PONG:
	case MC_PEER_OPENED:
	case MC_MATCH_GAME_RESPONSE_TIMESYNC:
		return false;
	}
	return true;
}

void ZGame::OnCommand_Observer(MCommand* pCommand)
{
	if (!IsIgnoreObserverCommand(pCommand->GetID()))
	{
		OnCommand_Immidiate(pCommand);
		return;
	}

	ZObserverCommandItem* pZCommand = new ZObserverCommandItem;
	pZCommand->pCommand = pCommand->Clone();
	pZCommand->fTime = GetTime();
	m_ObserverCommandList.push_back(pZCommand);

#ifdef _LOG_ENABLE_OBSERVER_COMMAND_BUSH_
	if (pCommand->GetID() != 10012 && pCommand->GetID() != 10014)
	{
		char buf[256];
		sprintf(buf, "[OBSERVER_COMMAND_BUSH:%d]: %s\n", pCommand->GetID(), pCommand->GetDescription());
		OutputDebugString(buf);
	}
#endif

	if (pCommand->GetID() == MC_PEER_BASICINFO)
	{
		OnPeerBasicInfo(pCommand, true, false);
	}
}

void ZGame::ProcessDelayedCommand()
{
	for (ZObserverCommandList::iterator i = m_DelayedCommandList.begin(); i != m_DelayedCommandList.end(); i++)
	{
		ZObserverCommandItem* pItem = *i;

		if (GetTime() > pItem->fTime)
		{
			OnCommand_Immidiate(pItem->pCommand);
			i = m_DelayedCommandList.erase(i);
			delete pItem->pCommand;
			delete pItem;
		}
	}
}

void ZGame::OnReplayRun()
{
	if (m_ReplayCommandList.size() == 0 && m_bReplaying.Ref()) {
		m_bReplaying.Set_CheckCrc(false);
		EndReplay();
		return;
	}

	while (m_ReplayCommandList.size())
	{
		ZObserverCommandItem* pItem = *m_ReplayCommandList.begin();

#ifdef _REPLAY_TEST_LOG
		m_ReplayLogTime = pItem->fTime;
#else
		if (GetTime() < pItem->fTime)
			return;
#endif

		m_ReplayCommandList.erase(m_ReplayCommandList.begin());

		bool bSkip = false;
		switch (pItem->pCommand->GetID())
		{
		case MC_REQUEST_XTRAP_HASHVALUE:
		case MC_RESPONSE_XTRAP_HASHVALUE:
		case MC_REQUEST_XTRAP_SEEDKEY:
		case MC_RESPONSE_XTRAP_SEEDKEY:
		case MC_REQUEST_XTRAP_DETECTCRACK:
		case MC_REQUEST_GAMEGUARD_AUTH:
		case MC_RESPONSE_GAMEGUARD_AUTH:
		case MC_REQUEST_FIRST_GAMEGUARD_AUTH:
		case MC_RESPONSE_FIRST_GAMEGUARD_AUTH:
			bSkip = true;
		}

		if (bSkip == false)
			OnCommand_Observer(pItem->pCommand);

#ifdef _LOG_ENABLE_REPLAY_COMMAND_DELETE_
		if (pItem->pCommand->GetID() != 10012 && pItem->pCommand->GetID() != 10014)
		{
			char buf[256];
			sprintf(buf, "[REPLAY_COMMAND_DELETE:%d]: %s\n", pItem->pCommand->GetID(), pItem->pCommand->GetDescription());
			OutputDebugString(buf);
		}
#endif

#ifdef _REPLAY_TEST_LOG
		switch (pItem->pCommand->GetID())
		{
		case MC_MATCH_STAGE_ENTERBATTLE:
		{
			unsigned char nParam;
			pItem->pCommand->GetParameter(&nParam, 0, MPT_UCHAR);

			MCommandParameter* pParam = pItem->pCommand->GetParameter(1);
			if (pParam->GetType() != MPT_BLOB) break;
			void* pBlob = pParam->GetPointer();

			MTD_PeerListNode* pPeerNode = (MTD_PeerListNode*)MGetBlobArrayElement(pBlob, 0);
			mlog("[%d EnterBattleRoom Time:%3.3f]\n", pPeerNode->uidChar.Low, pItem->fTime);
		}
		break;
		case MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT:
		{
			MUID uidChar;
			pItem->pCommand->GetParameter(&uidChar, 0, MPT_UID);
			mlog("[%d LeaveBattleRoom Time:%3.3f]\n", uidChar.Low, m_ReplayLogTime);
		}
		break;
		}
#endif

		delete pItem->pCommand;
		delete pItem;
	}
}

void ZGame::OnObserverRun()
{
	while (m_ObserverCommandList.begin() != m_ObserverCommandList.end())
	{
		ZObserverCommandItem* pItem = *m_ObserverCommandList.begin();
		if (GetTime() - pItem->fTime < ZGetGameInterface()->GetCombatInterface()->GetObserver()->GetDelay())
			return;

		m_ObserverCommandList.erase(m_ObserverCommandList.begin());

		if (pItem->pCommand->GetID() == MC_PEER_BASICINFO)
			OnPeerBasicInfo(pItem->pCommand, false, true);
		else
		{
			OnCommand_Immidiate(pItem->pCommand);

#ifdef _LOG_ENABLE_OBSERVER_COMMAND_DELETE_
			char buf[256];
			sprintf(buf, "[OBSERVER_COMMAND_DELETE:%d]: %s\n", pItem->pCommand->GetID(), pItem->pCommand->GetDescription());
			OutputDebugString(buf);
#endif
		}

		delete pItem->pCommand;
		delete pItem;
	}

#ifdef _REPLAY_TEST_LOG
	for (int i = 0; i < 16; ++i)
	{
		if (m_Replay_UseItem[i].uid.Low == 0)
			break;
		for (int j = 0; j < 5; ++j)
		{
			if (m_Replay_UseItem[i].Item[j].Itemid == 0)
				break;
			MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(m_Replay_UseItem[i].Item[j].Itemid);

			mlog("[ uid:%d Item:%s(%d) UseCount:%d ]\n", m_Replay_UseItem[i].uid.Low, pItemDesc->m_pMItemName->Ref().m_szItemName, pItemDesc->m_nID, m_Replay_UseItem[i].Item[j].ItemUseCount);
		}
	}
	mlog("[Replay Playtime: %f]\n[End Replay]\n", m_ReplayLogTime);
#endif
}

void ZGame::FlushObserverCommands()
{
	while (m_ObserverCommandList.begin() != m_ObserverCommandList.end())
	{
		ZObserverCommandItem* pItem = *m_ObserverCommandList.begin();

		m_ObserverCommandList.erase(m_ObserverCommandList.begin());

		if (pItem->pCommand->GetID() != MC_PEER_BASICINFO)
			OnCommand_Immidiate(pItem->pCommand);

		delete pItem->pCommand;
		delete pItem;
	}
}

bool ZGame::OnCommand(MCommand* pCommand)
{
	if (m_bRecording)
	{
		ZObserverCommandItem* pItem = new ZObserverCommandItem;
		pItem->fTime = m_fTime.Ref();
		pItem->pCommand = pCommand->Clone();

		m_ReplayCommandList.push_back(pItem);
	}

	if (ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
	{
		OnCommand_Observer(pCommand);
		return true;
	}

	if (FilterDelayedCommand(pCommand))
	{
		return true;
	}

	return OnCommand_Immidiate(pCommand);
}

bool ZGame::GetUserGradeIDColor(MMatchUserGradeID gid, MCOLOR& UserNameColor)
{
	if (gid == MMUG_FREE) // W
	{
		UserNameColor = MCOLOR(255, 255, 255);
		return true;
	}
	else if (gid == MMUG_STAR)
	{
		UserNameColor = MCOLOR(255, 0, 0);
	}
	else if (gid == MMUG_EVENTMASTER)// Light Cyan
	{
		UserNameColor = MCOLOR(137, 201, 255);
		return true;
	}
	else if (gid == MMUG_DEVELOPER)// Light Red
	{
		UserNameColor = MCOLOR(255, 75, 75);
		return true;
	}

	else if (gid == MMUG_ADMIN)// Red
	{
		UserNameColor = MCOLOR(255, 0, 0);
		return true;
	}

	else if (gid == MMUG_DONATOR_1) // Blue
	{
		UserNameColor = MCOLOR(0, 102, 255);
		return true;
	}

	else if (gid == MMUG_DONATOR_2) // Green
	{
		UserNameColor = MCOLOR(0, 204, 0);
		return true;
	}

	else if (gid == MMUG_DONATOR_3)// Pink
	{
		UserNameColor = MCOLOR(204, 51, 204);
		return true;
	}

	else if (gid == MMUG_DONATOR_4)// Purple
	{
		UserNameColor = MCOLOR(153, 0, 255);
		return true;
	}

	else if (gid == MMUG_DONATOR_5)// Yellow
	{
		UserNameColor = MCOLOR(255, 215, 0);
		return true;
	}

	else if (gid == MMUG_DONATOR_6)// Gray
	{
		UserNameColor = MCOLOR(65, 73, 79);
		return true;
	}

	else if (gid == MMUG_DONATOR_7)// Light Green
	{
		UserNameColor = MCOLOR(153, 229, 151);
		return true;
	}

	else if (gid == MMUG_DONATOR_8)// Orange (peach color)
	{
		UserNameColor = MCOLOR(246, 170, 68);
		return true;
	}

	return false;
}

bool ZGame::GetUserNameColor(MUID uid, MCOLOR& UserNameColor)
{
	MMatchUserGradeID gid = MMUG_FREE;

	if (m_pMyCharacter->GetUID() == uid)
	{
		if (ZGetMyInfo())
		{
			gid = ZGetMyInfo()->GetUGradeID();
		}
		else {
			mlog("ZGame::GetUserNameColor ZGetMyInfo==NULL \n");
		}
	}
	else
	{
		MMatchPeerInfo* pPeer = ZGetGameClient()->FindPeer(uid);
		if (pPeer) {
			gid = pPeer->CharInfo.nUGradeID;
		}
	}

	return GetUserGradeIDColor(gid, UserNameColor);
}

void ZTranslateCommand(MCommand* pCmd, string& strLog)
{
	char szBuf[256] = "";

	unsigned long nGlobalClock = ZGetGame()->GetTickTime();
	itoa(nGlobalClock, szBuf, 10);
	strLog = szBuf;
	strLog += ": ";

	strLog += pCmd->m_pCommandDesc->GetName();

	string strPlayerName;
	MUID uid = pCmd->GetSenderUID();
	ZCharacter* pChar = ZGetCharacterManager()->Find(uid);
	if (pChar)
		strPlayerName = pChar->GetProperty()->GetName();
	else
		strPlayerName = "Unknown";

	strLog += " (";
	strLog += strPlayerName;
	strLog += ") ";

	string strParams;
	for (int i = 0; i < pCmd->GetParameterCount(); i++) {
		char szParam[256] = "";
		pCmd->GetParameter(i)->GetString(szParam);
		strParams += szParam;
		if (i < pCmd->GetParameterCount() - 1)
			strParams += ", ";
	}
	strLog += strParams;
}

void ZLogCommand(MCommand* pCmd)
{
	if (pCmd->GetID() == MC_AGENT_TUNNELING_UDP) {
		return;
	}

	string strCmd;
	ZTranslateCommand(pCmd, strCmd);

	OutputDebugString(strCmd.c_str());
	OutputDebugString("\n");
}

bool ZGame::OnCommand_Immidiate(MCommand* pCommand)
{
#ifdef _DEBUG
#endif

	if (m_pGameAction->OnCommand(pCommand))
	{
		return true;
	}

	if (OnRuleCommand(pCommand))
	{
		return true;
	}

	switch (pCommand->GetID())
	{
	case MC_GUNZ_ANTILEAD:
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);

		if (pParam->GetType() != MPT_BLOB)
			break;

		void* pBlob = pParam->GetPointer();
		int nSize = MGetBlobArrayCount(pBlob);

		for (int i = 0; i < nSize; ++i)
		{
			MTD_ShotInfo* pInfo = (MTD_ShotInfo*)MGetBlobArrayElement(pBlob, i);
			ZGetGameClient()->GetPeerPacketCrypter().Decrypt((char*)pInfo, sizeof(MTD_ShotInfo));

			if (m_pMyCharacter && ZGetGameClient()->GetPlayerUID() != pCommand->GetSenderUID())
			{
				ZCharacter* pCharacter = (ZCharacter*)ZGetCharacterManager()->Find(pCommand->GetSenderUID());
				rvector pos;
				pos.x = pInfo->fPosX;
				pos.y = pInfo->fPosY;
				pos.z = pInfo->fPosZ;
				if (pCharacter && pInfo->nLowId == ZGetGameClient()->GetPlayerUID().Low
					&& !ZGetGame()->GetMatch()->IsTeamPlay() || ((m_pMyCharacter->IsTeam(pCharacter) && ZGetGame()->GetMatch()->GetTeamKillEnabled()) || !m_pMyCharacter->IsTeam(pCharacter))
					)
				{
					m_pMyCharacter->OnDamaged(pCharacter, pos, (ZDAMAGETYPE)pInfo->nDamageType, (MMatchWeaponType)pInfo->nWeaponType, pInfo->fDamage, pInfo->fRatio);
				}
			}
			else
			{
				ZCharacter* pCharacter = (ZCharacter*)ZGetCharacterManager()->Find(pCommand->GetSenderUID());
				if (pCharacter != ZGetGame()->m_pMyCharacter)
				{
				}
				else
				{
				}
			}
		}
	}
	break;

	case MC_GUNZ_DAMAGECOUNTER:
	{
		int Damage;
		MUID AttackerUID;
		pCommand->GetParameter(&Damage, 0, MPT_INT);
		pCommand->GetParameter(&AttackerUID, 1, MPT_UID);
		ZCharacter* pCharacter = (ZCharacter*)ZGetCharacterManager()->Find(AttackerUID);

		pCharacter->GetStatus().CheckCrc();
		pCharacter->GetStatus().Ref().nDamageCaused += Damage;
		pCharacter->GetStatus().MakeCrc();
	}
	break;

	case MC_MATCH_STAGE_ENTERBATTLE:
	{
		unsigned char nParam;
		pCommand->GetParameter(&nParam, 0, MPT_UCHAR);

		MCommandParameter* pParam = pCommand->GetParameter(1);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();

		MTD_PeerListNode* pPeerNode = (MTD_PeerListNode*)MGetBlobArrayElement(pBlob, 0);

		OnStageEnterBattle(MCmdEnterBattleParam(nParam), pPeerNode);
	}
	break;
	case MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT:
	{
		MUID uidChar;
		bool bIsRelayMap;

		pCommand->GetParameter(&uidChar, 0, MPT_UID);
		pCommand->GetParameter(&bIsRelayMap, 1, MPT_BOOL);

		OnStageLeaveBattle(uidChar, bIsRelayMap);
	}
	break;
	case MC_MATCH_RESPONSE_PEERLIST:
	{
		MUID uidStage;
		pCommand->GetParameter(&uidStage, 0, MPT_UID);
		MCommandParameter* pParam = pCommand->GetParameter(1);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();
		int nCount = MGetBlobArrayCount(pBlob);
		OnPeerList(uidStage, pBlob, nCount);
	}
	break;
	case MC_MATCH_GAME_ROUNDSTATE:
	{
		MUID uidStage;
		int nRoundState, nRound, nArg;
		DWORD dwElapsed;

		pCommand->GetParameter(&uidStage, 0, MPT_UID);
		pCommand->GetParameter(&nRound, 1, MPT_INT);
		pCommand->GetParameter(&nRoundState, 2, MPT_INT);
		pCommand->GetParameter(&nArg, 3, MPT_INT);
		pCommand->GetParameter(&dwElapsed, 4, MPT_UINT);

		OnGameRoundState(uidStage, nRound, nRoundState, nArg);

		ZGetGame()->GetMatch()->SetRoundStartTime(dwElapsed);
	}
	break;
	case MC_MATCH_GAME_RESPONSE_TIMESYNC:
	{
		unsigned int nLocalTS, nGlobalTS;
		pCommand->GetParameter(&nLocalTS, 0, MPT_UINT);
		pCommand->GetParameter(&nGlobalTS, 1, MPT_UINT);

		OnGameResponseTimeSync(nLocalTS, nGlobalTS);
	}
	break;
	case MC_MATCH_RESPONSE_SUICIDE:
	{
		int nResult;
		MUID	uidChar;
		pCommand->GetParameter(&nResult, 0, MPT_INT);
		pCommand->GetParameter(&uidChar, 1, MPT_UID);

		if (nResult == MOK)
		{
			OnPeerDie(uidChar, uidChar);
			CancelSuicide();
		}
	}
	break;

	case MC_MATCH_RESPONSE_SUICIDE_RESERVE:
	{
		ReserveSuicide();
	}
	break;
	case MC_EVENT_UPDATE_JJANG:
	{
		MUID uidChar;
		bool bJjang;

		pCommand->GetParameter(&uidChar, 0, MPT_UID);
		pCommand->GetParameter(&bJjang, 1, MPT_BOOL);

		OnEventUpdateJjang(uidChar, bJjang);
	}
	break;
	case MC_PEER_CHAT:
	{
		int nTeam = MMT_ALL;
		char szMsg[CHAT_STRING_LEN];
		memset(szMsg, 0, sizeof(szMsg));

		pCommand->GetParameter(&nTeam, 0, MPT_INT);
		pCommand->GetParameter(szMsg, 1, MPT_STR, CHAT_STRING_LEN);
		CheckMsgAboutChat(szMsg);

		MCOLOR ChatColor = MCOLOR(0xFFD0D0D0);
		const MCOLOR TeamChatColor = MCOLOR(109, 207, 246);

		MUID uid = pCommand->GetSenderUID();
		ZCharacter* pChar = (ZCharacter*)ZGetCharacterManager()->Find(uid);

		MCOLOR UserNameColor = MCOLOR(190, 190, 0);

		bool bSpUser = GetUserNameColor(uid, UserNameColor);

		if (pChar)
		{
			int nMyTeam = ZGetGame()->m_pMyCharacter->GetTeamID();

			if ((nTeam == MMT_ALL) || (nTeam == MMT_SPECTATOR))
			{
				if (!ZGetGameClient()->GetRejectNormalChat() || (strcmp(pChar->GetUserName(), ZGetMyInfo()->GetCharName()) == 0))
				{
					ZGetSoundEngine()->PlaySound("if_error");
					char szTemp[sizeof(szMsg) + 64];

					if (bSpUser) {
						sprintf(szTemp, "%s : %s", pChar->GetProperty()->GetName(), szMsg);
						ZChatOutput(UserNameColor, szTemp);
					}
					else {
						sprintf(szTemp, "%s : %s", pChar->GetProperty()->GetName(), szMsg);
						ZChatOutput(ChatColor, szTemp);
					}
				}
			}

			else if (nTeam == nMyTeam)
			{
				if ((!ZGetGameClient()->IsLadderGame() && !ZGetGameClient()->GetRejectTeamChat()) ||
					(ZGetGameClient()->IsLadderGame() && !ZGetGameClient()->GetRejectClanChat()) ||
					(strcmp(pChar->GetUserName(), ZGetMyInfo()->GetCharName()) == 0))
				{
					ZGetSoundEngine()->PlaySound("if_error");
					char szTemp[256];

					if (bSpUser) {
						sprintf(szTemp, "(Team)%s : %s", pChar->GetProperty()->GetName(), szMsg);
						ZChatOutput(UserNameColor, szTemp);
					}
					else {
						sprintf(szTemp, "(Team)%s : %s", pChar->GetProperty()->GetName(), szMsg);
						ZChatOutput(TeamChatColor, szTemp);
					}
				}
			}
		}
	}
	break;

	case MC_PEER_CHAT_ICON:
	{
		bool bShow = false;
		pCommand->GetParameter(&bShow, 0, MPT_BOOL);

		MUID uid = pCommand->GetSenderUID();
		ZCharacter* pChar = ZGetCharacterManager()->Find(uid);
		if (pChar)
		{
			ZCharaterStatusBitPacking& uStatus = pChar->m_dwStatusBitPackingValue.Ref();
			if (bShow)
			{
				if (!uStatus.m_bChatEffect)
				{
					uStatus.m_bChatEffect = true;
					ZGetEffectManager()->AddChatIcon(pChar);
				}
			}
			else
				uStatus.m_bChatEffect = false;
		}
	}break;

	case MC_MATCH_OBTAIN_WORLDITEM:
	{
		if (!IsReplay()) break;

		MUID uidPlayer;
		int nItemUID;

		pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
		pCommand->GetParameter(&nItemUID, 1, MPT_INT);

		ZGetGameClient()->OnObtainWorldItem(uidPlayer, nItemUID);
	}
	break;
	case MC_MATCH_SPAWN_WORLDITEM:
	{
		if (!IsReplay()) break;

		MCommandParameter* pParam = pCommand->GetParameter(0);
		if (pParam->GetType() != MPT_BLOB) break;

		void* pSpawnInfoBlob = pParam->GetPointer();

		ZGetGameClient()->OnSpawnWorldItem(pSpawnInfoBlob);
	}
	break;
	case MC_MATCH_REMOVE_WORLDITEM:
	{
		if (!IsReplay()) break;

		int nItemUID;

		pCommand->GetParameter(&nItemUID, 0, MPT_INT);

		ZGetGameClient()->OnRemoveWorldItem(nItemUID);
	}
	break;
	case MC_MATCH_NOTIFY_ACTIATED_TRAPITEM_LIST:
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);
		if (pParam->GetType() != MPT_BLOB) break;

		void* pActiveTrapBlob = pParam->GetPointer();
		ZGetGameClient()->OnNotifyActivatedTrapItemList(pActiveTrapBlob);
	}
	break;

	case MC_PEER_BASICINFO: OnPeerBasicInfo(pCommand); break;
	case MC_PEER_HPINFO: OnPeerHPInfo(pCommand); break;
	case MC_PEER_HPAPINFO: OnPeerHPAPInfo(pCommand); break;
	case MC_PEER_DUELTOURNAMENT_HPAPINFO: OnPeerDuelTournamentHPAPInfo(pCommand); break;
	case MC_PEER_PING: OnPeerPing(pCommand); break;
	case MC_PEER_PONG: OnPeerPong(pCommand); break;
	case MC_PEER_OPENED: OnPeerOpened(pCommand); break;
	case MC_PEER_DASH: OnPeerDash(pCommand); break;
	case MC_PEER_SHOT:
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);
		if (pParam->GetType() != MPT_BLOB) break;

		ZPACKEDSHOTINFO* pinfo = (ZPACKEDSHOTINFO*)pParam->GetPointer();

		rvector pos = rvector(pinfo->posx, pinfo->posy, pinfo->posz);
		rvector to = rvector(pinfo->tox, pinfo->toy, pinfo->toz);

		OnPeerShot(pCommand->GetSenderUID(), pinfo->fTime, pos, to, (MMatchCharItemParts)pinfo->sel_type);
	}
	break;
	case MC_PEER_SHOT_MELEE:
	{
		float fShotTime;
		rvector pos, dir;

		pCommand->GetParameter(&fShotTime, 0, MPT_FLOAT);
		pCommand->GetParameter(&pos, 1, MPT_POS);

		OnPeerShot(pCommand->GetSenderUID(), fShotTime, pos, pos, MMCIP_MELEE);
	}
	break;

	case MC_PEER_SHOT_SP:
	{
		float fShotTime;
		rvector pos, dir;
		int sel_type, type;

		pCommand->GetParameter(&fShotTime, 0, MPT_FLOAT);
		pCommand->GetParameter(&pos, 1, MPT_POS);
		pCommand->GetParameter(&dir, 2, MPT_VECTOR);
		pCommand->GetParameter(&type, 3, MPT_INT);
		pCommand->GetParameter(&sel_type, 4, MPT_INT);

		OnPeerShotSp(pCommand->GetSenderUID(), fShotTime, pos, dir, type, (MMatchCharItemParts)sel_type);
	}
	break;

	case MC_PEER_RELOAD:
	{
		OnPeerReload(pCommand->GetSenderUID());
	}
	break;

	case MC_PEER_CHANGECHARACTER:
	{
		OnPeerChangeCharacter(pCommand->GetSenderUID());
	}
	break;

	case MC_PEER_DIE:
	{
		MUID	uid;
		pCommand->GetParameter(&uid, 0, MPT_UID);

		OnPeerDie(pCommand->GetSenderUID(), uid);
	}
	break;
	case MC_PEER_BUFF_INFO:
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();

		OnPeerBuffInfo(pCommand->GetSenderUID(), pBlob);
	}
	break;
	case MC_MATCH_GAME_DEAD:
	{
		MUID uidAttacker, uidVictim;
		unsigned long int nAttackerArg, nVictimArg;

		pCommand->GetParameter(&uidAttacker, 0, MPT_UID);
		pCommand->GetParameter(&nAttackerArg, 1, MPT_UINT);
		pCommand->GetParameter(&uidVictim, 2, MPT_UID);
		pCommand->GetParameter(&nVictimArg, 3, MPT_UINT);

		OnPeerDead(uidAttacker, nAttackerArg, uidVictim, nVictimArg);
	}
	break;
	case MC_MATCH_GAME_TEAMBONUS:
	{
		MUID uidChar;
		unsigned long int nExpArg;

		pCommand->GetParameter(&uidChar, 0, MPT_UID);
		pCommand->GetParameter(&nExpArg, 1, MPT_UINT);

		OnReceiveTeamBonus(uidChar, nExpArg);
	}
	break;
	case MC_PEER_SPAWN:
	{
		rvector pos, dir;
		pCommand->GetParameter(&pos, 0, MPT_POS);
		pCommand->GetParameter(&dir, 1, MPT_DIR);

		OnPeerSpawn(pCommand->GetSenderUID(), pos, dir);
	}
	break;
	case MC_MATCH_GAME_RESPONSE_SPAWN:
	{
		MUID uidChar;
		MShortVector s_pos, s_dir;

		pCommand->GetParameter(&uidChar, 0, MPT_UID);
		pCommand->GetParameter(&s_pos, 1, MPT_SVECTOR);
		pCommand->GetParameter(&s_dir, 2, MPT_SVECTOR);

		rvector pos, dir;
		pos = rvector((float)s_pos.x, (float)s_pos.y, (float)s_pos.z);
		dir = rvector(ShortToDirElement(s_dir.x), ShortToDirElement(s_dir.y), ShortToDirElement(s_dir.z));

		OnPeerSpawn(uidChar, pos, dir);
	}
	break;
	case MC_MATCH_SET_OBSERVER:
	{
		MUID uidChar;

		pCommand->GetParameter(&uidChar, 0, MPT_UID);

		OnSetObserver(uidChar);
	}
	break;
	case MC_PEER_CHANGE_WEAPON:
	{
		int nWeaponID;

		pCommand->GetParameter(&nWeaponID, 0, MPT_INT);

		OnChangeWeapon(pCommand->GetSenderUID(), MMatchCharItemParts(nWeaponID));
	}

	break;

	case MC_PEER_SPMOTION:
	{
		int nMotionType;

		pCommand->GetParameter(&nMotionType, 0, MPT_INT);

		OnPeerSpMotion(pCommand->GetSenderUID(), nMotionType);
	}
	break;

	case MC_PEER_CHANGE_PARTS:
	{
		int PartsType;
		int PartsID;

		pCommand->GetParameter(&PartsType, 0, MPT_INT);
		pCommand->GetParameter(&PartsID, 1, MPT_INT);

		OnChangeParts(pCommand->GetSenderUID(), PartsType, PartsID);
	}
	break;

	case MC_PEER_ATTACK:
	{
		int		type;
		rvector pos;

		pCommand->GetParameter(&type, 0, MPT_INT);
		pCommand->GetParameter(&pos, 1, MPT_POS);

		OnAttack(pCommand->GetSenderUID(), type, pos);
	}
	break;

	case MC_PEER_DAMAGE:
	{
		MUID	tuid;
		int		damage;

		pCommand->GetParameter(&tuid, 0, MPT_UID);
		pCommand->GetParameter(&damage, 1, MPT_INT);

		OnDamage(pCommand->GetSenderUID(), tuid, damage);
	}
	break;
	case MC_MATCH_RESET_TEAM_MEMBERS:
	{
		OnResetTeamMembers(pCommand);
	}
	break;

	case MC_REQUEST_XTRAP_HASHVALUE:
	{
	}
	break;

	case MC_MATCH_DISCONNMSG:
	{
		DWORD dwMsgID;
		pCommand->GetParameter(&dwMsgID, 0, MPT_UINT);

		ZApplication::GetGameInterface()->OnDisconnectMsg(dwMsgID);
	}
	break;

	case ZC_TEST_INFO:
	{
		OutputToConsole("Sync : %u", ZGetGameClient()->GetGlobalClockCount());

		rvector v;
		v = m_pMyCharacter->GetPosition();
		OutputToConsole("My Pos = %.2f %.2f %.2f", v.x, v.y, v.z);
	}
	break;
	case ZC_BEGIN_PROFILE:
		g_bProfile = true;
		break;
	case ZC_END_PROFILE:
		g_bProfile = false;
		break;
	case ZC_EVENT_OPTAIN_SPECIAL_WORLDITEM:
	{
		OnLocalOptainSpecialWorldItem(pCommand);
	}
	break;

#ifdef _GAMEGUARD
	case MC_REQUEST_GAMEGUARD_AUTH:
	{
		DWORD dwIndex;
		DWORD dwValue1;
		DWORD dwValue2;
		DWORD dwValue3;

		pCommand->GetParameter(&dwIndex, 0, MPT_UINT);
		pCommand->GetParameter(&dwValue1, 1, MPT_UINT);
		pCommand->GetParameter(&dwValue2, 2, MPT_UINT);
		pCommand->GetParameter(&dwValue3, 3, MPT_UINT);

		ZApplication::GetGameInterface()->OnRequestGameguardAuth(dwIndex, dwValue1, dwValue2, dwValue3);

#ifdef _DEBUG
		mlog("zgame recevie request gameguard auth. CmdID(%u) : %u, %u, %u, %u\n", pCommand->GetID(), dwIndex, dwValue1, dwValue2, dwValue3);
#endif
	}
	break;
#endif

#ifdef _XTRAP
	case MC_REQUEST_XTRAP_SEEDKEY:
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);
		if (pParam->GetType() != MPT_BLOB)
		{
			break;
		}
		void* pComBuf = pParam->GetPointer();
		unsigned char* szComBuf = (unsigned char*)MGetBlobArrayElement(pComBuf, 0);
		ZApplication::GetGameInterface()->OnRequestXTrapSeedKey(szComBuf);
	}
	break;
#endif
	case MC_MATCH_RESPONSE_USE_SPENDABLE_BUFF_ITEM:
	{
		MUID uidItem;
		int nResult;

		pCommand->GetParameter(&uidItem, 0, MPT_UID);
		pCommand->GetParameter(&nResult, 0, MPT_INT);

		OnResponseUseSpendableBuffItem(uidItem, nResult);
	}
	break;

	case MC_MATCH_SPENDABLE_BUFF_ITEM_STATUS:
	{
	}
	break;
	}

	ZRule* pRule = m_Match.GetRule();
	if (pRule) {
		pRule->AfterCommandProcessed(pCommand);
	}

	return false;
}

rvector ZGame::GetMyCharacterFirePosition(void)
{
	rvector p = ZGetGame()->m_pMyCharacter->GetPosition();
	p.z += 160.f;
	return p;
}

void ZGame::OnPeerBasicInfo(MCommand* pCommand, bool bAddHistory, bool bUpdate)
{
	MCommandParameter* pParam = pCommand->GetParameter(0);
	if (pParam->GetType() != MPT_BLOB) return;

	ZPACKEDBASICINFO* ppbi = (ZPACKEDBASICINFO*)pParam->GetPointer();

	ZBasicInfo bi;
	bi.position = rvector(Roundf(ppbi->posx), Roundf(ppbi->posy), Roundf(ppbi->posz));
	bi.velocity = rvector(ppbi->velx, ppbi->vely, ppbi->velz);
	bi.direction = 1.f / 32000.f * rvector(ppbi->dirx, ppbi->diry, ppbi->dirz);

	MUID uid = pCommand->GetSenderUID();

	MMatchPeerInfo* pPeer = ZGetGameClient()->FindPeer(uid);
	if (pPeer) {
		if (pPeer->IsOpened() == false) {
			MCommand* pCmd = ZGetGameClient()->CreateCommand(MC_PEER_OPENED, ZGetGameClient()->GetPlayerUID());
			pCmd->AddParameter(new MCmdParamUID(pPeer->uidChar));
			ZGetGameClient()->Post(pCmd);

			pPeer->SetOpened(true);
		}
	}

	ZCharacter* pCharacter = m_CharacterManager.Find(uid);
	if (!pCharacter) return;

	float fCurrentLocalTime = pCharacter->m_fTimeOffset + GetTime();

	float fTimeError = ppbi->fTime - fCurrentLocalTime;
	if (fabs(fTimeError) > TIME_ERROR_BETWEEN_RECIEVEDTIME_MYTIME) {
		pCharacter->m_fTimeOffset = ppbi->fTime - GetTime();
		pCharacter->m_fAccumulatedTimeError = 0;
		pCharacter->m_nTimeErrorCount = 0;
	}
	else
	{
		pCharacter->m_fAccumulatedTimeError += fTimeError;
		pCharacter->m_nTimeErrorCount++;
		if (pCharacter->m_nTimeErrorCount > 10) {
			pCharacter->m_fTimeOffset += .5f * pCharacter->m_fAccumulatedTimeError / 10.f;
			pCharacter->m_fAccumulatedTimeError = 0;
			pCharacter->m_nTimeErrorCount = 0;
		}
	}

	pCharacter->m_fLastReceivedTime = GetTime();

	if (bAddHistory)
	{
		ZBasicInfoItem* pitem = new ZBasicInfoItem;
		CopyMemory(&pitem->info, &bi, sizeof(ZBasicInfo));

		pitem->fReceivedTime = GetTime();

		pitem->fSendTime = ppbi->fTime - pCharacter->m_fTimeOffset;

		pCharacter->m_BasicHistory.push_back(pitem);

		while (pCharacter->m_BasicHistory.size() > CHARACTER_HISTROY_COUNT)
		{
			delete* pCharacter->m_BasicHistory.begin();
			pCharacter->m_BasicHistory.erase(pCharacter->m_BasicHistory.begin());
		}
	}

	if (bUpdate)
	{
		if (!IsReplay() && pCharacter->IsHero()) return;

		((ZNetCharacter*)(pCharacter))->SetNetPosition(bi.position, bi.velocity, bi.direction);

		pCharacter->SetAnimationLower((ZC_STATE_LOWER)ppbi->lowerstate);
		pCharacter->SetAnimationUpper((ZC_STATE_UPPER)ppbi->upperstate);

		if (pCharacter->GetItems()->GetSelectedWeaponParts() != ppbi->selweapon) {
			pCharacter->ChangeWeapon((MMatchCharItemParts)ppbi->selweapon);
		}
	}
}

void ZGame::OnPeerHPInfo(MCommand* pCommand)
{
	MUID uid = pCommand->GetSenderUID();
	ZCharacter* pCharacter = m_CharacterManager.Find(uid);
	if (!pCharacter) return;

	float fHP = 0.0f;
	pCommand->GetParameter(&fHP, 0, MPT_FLOAT);

	if (ZGetGameInterface()->GetCombatInterface()->GetObserverMode()) {
		pCharacter->SetHP(fHP);
	}
}

void ZGame::OnPeerHPAPInfo(MCommand* pCommand)
{
	MUID uid = pCommand->GetSenderUID();
	ZCharacter* pCharacter = m_CharacterManager.Find(uid);
	if (!pCharacter) return;

	float fHP = 0.0f;
	pCommand->GetParameter(&fHP, 0, MPT_FLOAT);
	float fAP = 0.0f;
	pCommand->GetParameter(&fAP, 1, MPT_FLOAT);

	if (ZGetGameInterface()->GetCombatInterface()->GetObserverMode()) {
		pCharacter->SetHP(fHP);
		pCharacter->SetAP(fAP);
	}

	if (pCharacter == m_pMyCharacter && fHP > 0 && !m_pMyCharacter->IsAlive())
		m_pMyCharacter->Revival();
}

void ZGame::OnPeerDuelTournamentHPAPInfo(MCommand* pCommand)
{
	MUID uid = pCommand->GetSenderUID();
	ZCharacter* pCharacter = m_CharacterManager.Find(uid);
	if (!pCharacter) return;

	BYTE MaxHP = 0;
	BYTE MaxAP = 0;
	BYTE HP = 0;
	BYTE AP = 0;

	pCommand->GetParameter(&MaxHP, 0, MPT_UCHAR);
	pCommand->GetParameter(&MaxAP, 1, MPT_UCHAR);

	pCommand->GetParameter(&HP, 2, MPT_UCHAR);
	pCommand->GetParameter(&AP, 3, MPT_UCHAR);

	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT) {
		((ZRuleDuelTournament*)m_Match.GetRule())->SetPlayerHpApForUI(uid, (float)MaxHP, (float)MaxAP, (float)HP, (float)AP);
	}

	if (ZGetGameInterface()->GetCombatInterface()->GetObserverMode()) {
		pCharacter->SetMaxHP((float)MaxHP);
		pCharacter->SetMaxAP((float)MaxAP);
		pCharacter->SetHP((float)HP);
		pCharacter->SetAP((float)AP);
	}
}

#ifdef _DEBUG
static int g_nPingCount = 0;
static int g_nPongCount = 0;
#endif

void ZGame::OnPeerPing(MCommand* pCommand)
{
	if (m_bReplaying.Ref()) return;

	unsigned int nTimeStamp;
	pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT);

	MCommandManager* MCmdMgr = ZGetGameClient()->GetCommandManager();
	MCommand* pCmd = new MCommand(MCmdMgr->GetCommandDescByID(MC_PEER_PONG),
		pCommand->GetSenderUID(), ZGetGameClient()->GetUID());
	pCmd->AddParameter(new MCmdParamUInt(nTimeStamp));
	ZGetGameClient()->Post(pCmd);
}

void ZGame::OnPeerPong(MCommand* pCommand)
{
	MMatchPeerInfo* pPeer = ZGetGameClient()->FindPeer(pCommand->GetSenderUID());
	if (pPeer == NULL)
		return;

	unsigned int nTimeStamp;
	pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT);

	int nPing = (GetTickTime() - nTimeStamp) / 2;
	pPeer->UpdatePing(GetTickTime(), nPing);

#ifdef _DEBUG
	g_nPongCount++;
#endif
}

void ZGame::OnPeerOpened(MCommand* pCommand)
{
	MUID uidChar;
	pCommand->GetParameter(&uidChar, 0, MPT_UID);

	ZCharacter* pCharacter = m_CharacterManager.Find(uidChar);
	if (pCharacter && pCharacter->IsDie() == false) {
		pCharacter->SetVisible(true);

		ZCharacter* pMyCharacter = ZGetGame()->m_pMyCharacter;
		if (pMyCharacter)
		{
			int nParts = ZGetGame()->m_pMyCharacter->GetItems()->GetSelectedWeaponParts();
			ZGetGame()->m_pMyCharacter->m_dwStatusBitPackingValue.Ref().m_bSpMotion = false;
			ZPostChangeWeapon(nParts);
		}
		PostMyBuffInfo();
	}

#ifdef _DEBUG
	char* pszName = "Unknown";
	char* pszNAT = "NoNAT";
	MMatchPeerInfo* pPeer = ZGetGameClient()->FindPeer(uidChar);
	if (pPeer) {
		pszName = pPeer->CharInfo.szName;
		if (pPeer->GetUDPTestResult() == false) pszNAT = "NAT";
	}

	char szBuf[64];
	sprintf(szBuf, "PEER_OPENED(%s) : %s(%d%d) \n", pszNAT, pszName, uidChar.High, uidChar.Low);
	OutputDebugString(szBuf);
#endif
}

void ZGame::OnChangeWeapon(MUID& uid, MMatchCharItemParts parts)
{
	ZCharacter* pCharacter = m_CharacterManager.Find(uid);
	if (pCharacter && pCharacter != m_pMyCharacter)
	{
		pCharacter->ChangeWeapon(parts);
	}
}

void ZGame::OnChangeParts(MUID& uid, int partstype, int PartsID)
{
	ZCharacter* pCharacter = m_CharacterManager.Find(uid);
	if (pCharacter) {
		pCharacter->OnChangeParts((RMeshPartsType)partstype, PartsID);
	}
}

void ZGame::OnAttack(MUID& uid, int type, rvector& pos)
{
	ZCharacter* pCharacter = m_CharacterManager.Find(uid);
	if (pCharacter) {
	}
}

void ZGame::OnDamage(MUID& uid, MUID& tuid, int damage)
{
}

void ZGame::OnPeerShotSp(MUID& uid, float fShotTime, rvector& pos, rvector& dir, int type, MMatchCharItemParts sel_type)
{
	ZCharacter* pOwnerCharacter = NULL;

	pOwnerCharacter = m_CharacterManager.Find(uid);
	if (pOwnerCharacter == NULL) return;
	if (!pOwnerCharacter->GetInitialized()) return;
	if (!pOwnerCharacter->IsVisible()) return;

	ZItem* pItem = pOwnerCharacter->GetItems()->GetItem(sel_type);
	if (!pItem) return;

	MMatchItemDesc* pDesc = pItem->GetDesc();
	if (pDesc == NULL) return;

	fShotTime -= pOwnerCharacter->m_fTimeOffset;

	if (sel_type != MMCIP_PRIMARY && sel_type != MMCIP_SECONDARY && sel_type != MMCIP_CUSTOM1 && sel_type != MMCIP_CUSTOM2)
		return;

	MMatchCharItemParts parts = (MMatchCharItemParts)sel_type;

	if (parts != pOwnerCharacter->GetItems()->GetSelectedWeaponParts()) {
		OnChangeWeapon(uid, parts);
	}

	MMatchWeaponType nType = pDesc->m_nWeaponType.Ref();
	if (nType == MWT_ROCKET) {
		if (type != ZC_WEAPON_SP_ROCKET) {
			return;
		}
	}
	else if (nType == MWT_MED_KIT || nType == MWT_REPAIR_KIT || nType == MWT_BULLET_KIT || nType == MWT_FOOD) {
		if (type != ZC_WEAPON_SP_ITEMKIT) {
			return;
		}
	}
	else if (nType == MWT_FLASH_BANG) {
		if (type != ZC_WEAPON_SP_FLASHBANG) {
			return;
		}
	}
	else if (nType == MWT_FRAGMENTATION) {
		if (type != ZC_WEAPON_SP_GRENADE) {
			return;
		}
	}
	else if (nType == MWT_SMOKE_GRENADE) {
		if (type != ZC_WEAPON_SP_SMOKE) {
			return;
		}
	}
	else if (nType == MWT_POTION) {
		if (type != ZC_WEAPON_SP_POTION) {
			return;
		}
	}
	else if (nType == MWT_TRAP) {
		if (type != ZC_WEAPON_SP_TRAP) {
			return;
		}
	}
	else if (nType == MWT_DYNAMITYE) {
		if (type != ZC_WEAPON_SP_DYNAMITE) {
			return;
		}
	}
	else {
		return;
	}

	if (pOwnerCharacter->CheckValidShotTime(pItem->GetDescID(), fShotTime, pItem)) {
		pOwnerCharacter->UpdateValidShotTime(pItem->GetDescID(), fShotTime);
	}
	else {
		return;
	}

	if (uid == ZGetMyUID()) {
		int nCurrMagazine = pItem->GetBulletCurrMagazine();
		if (!pItem->Shot()) return;

		if (!(pItem->GetBulletCurrMagazine() < nCurrMagazine)) {
			if (sel_type != MMCIP_MELEE) ZGetApplication()->Exit();
		}
	}
	else {
		if (!pItem->Shot()) return;
	}

	rvector velocity;
	rvector _pos;

	bool dLight = true;
	bool bSpend = false;

	switch (type)
	{
	case ZC_WEAPON_SP_GRENADE:
	{
		bSpend = true;

		velocity = pOwnerCharacter->GetVelocity() + pOwnerCharacter->m_TargetDir * 1200.f;
		velocity.z += 300.f;
		m_WeaponManager.AddGrenade(pos, velocity, pOwnerCharacter);
	}
	break;

	case ZC_WEAPON_SP_ROCKET:
	{
		m_WeaponManager.AddRocket(pos, dir, pOwnerCharacter);
		if (Z_VIDEO_DYNAMICLIGHT) {
			ZGetStencilLight()->AddLightSource(pos, 2.0f, 100);
		}
	}
	break;

	case ZC_WEAPON_SP_FLASHBANG:
	{
		bSpend = true;

		velocity = pOwnerCharacter->GetVelocity() + pOwnerCharacter->m_TargetDir * 1200.f;
		velocity.z += 300.0f;
		m_WeaponManager.AddFlashBang(pos, velocity, pOwnerCharacter);
		dLight = false;
	}
	break;

	case ZC_WEAPON_SP_SMOKE:
	{
		bSpend = true;

		velocity = pOwnerCharacter->GetVelocity() + pOwnerCharacter->m_TargetDir * 1200.f;
		velocity.z += 300.0f;
		m_WeaponManager.AddSmokeGrenade(pos, velocity, pOwnerCharacter);
		dLight = false;
	}
	break;

	case ZC_WEAPON_SP_TEAR_GAS:
	{
		bSpend = true;
		dLight = false;
	}
	break;

	case ZC_WEAPON_SP_ITEMKIT:
	{
		int nLinkedWorldItem = ZGetWorldItemManager()->GetLinkedWorldItemID(pItem->GetDesc());

		velocity = dir;
		_pos = pos;

		m_WeaponManager.AddKit(_pos, velocity, pOwnerCharacter, 0.2f, pItem->GetDesc()->m_pMItemName->Ref().m_szMeshName, nLinkedWorldItem);
		dLight = false;
	}
	break;

	case ZC_WEAPON_SP_POTION:
	{
		ApplyPotion(pItem->GetDescID(), pOwnerCharacter, 0);
	}
	break;

	case ZC_WEAPON_SP_TRAP:
	{
		OnUseTrap(pItem->GetDescID(), pOwnerCharacter, pos);
		dLight = true;
	}
	break;

	case ZC_WEAPON_SP_DYNAMITE:
	{
		OnUseDynamite(pItem->GetDescID(), pOwnerCharacter, pos);
		dLight = true;
	}
	break;

	default:
		break;
	}

#ifdef _REPLAY_TEST_LOG
	if (type == ZC_WEAPON_SP_POTION || type == ZC_WEAPON_SP_ITEMKIT)
	{
		for (int i = 0; i < 16; ++i)
		{
			if (m_Replay_UseItem[i].uid.Low == 0)
			{
				m_Replay_UseItem[i].uid = uid;
				m_Replay_UseItem[i].Item[0].Itemid = pDesc->m_nID;
				m_Replay_UseItem[i].Item[0].ItemUseCount++;
				break;
			}
			if (m_Replay_UseItem[i].uid == uid)
			{
				for (int j = 0; j < 5; ++j)
				{
					if (m_Replay_UseItem[i].Item[j].Itemid == 0)
					{
						m_Replay_UseItem[i].uid = uid;
						m_Replay_UseItem[i].Item[j].Itemid = pDesc->m_nID;
						m_Replay_UseItem[i].Item[j].ItemUseCount++;
						break;
					}
					if (m_Replay_UseItem[i].Item[j].Itemid == pDesc->m_nID)
					{
						m_Replay_UseItem[i].Item[j].ItemUseCount++;
						break;
					}
				}
				break;
			}
		}
	}
#endif

	if (type == ZC_WEAPON_SP_POTION)
	{
		if (pOwnerCharacter == ZGetGame()->m_pMyCharacter) {
			ZGetSoundEngine()->PlaySound("fx_itemget");
		}
	}
	else
	{
		ZApplication::GetSoundEngine()->PlaySEFire(pItem->GetDesc(), pos.x, pos.y, pos.z, (pOwnerCharacter == m_pMyCharacter));
	}

	if (dLight)
	{
		ZCharacter* pChar;

		if (ZGetConfiguration()->GetVideo()->bDynamicLight && pOwnerCharacter != NULL) {
			pChar = pOwnerCharacter;

			if (pChar->m_bDynamicLight) {
				pChar->m_vLightColor = g_CharLightList[CANNON].vLightColor;
				pChar->m_fLightLife = g_CharLightList[CANNON].fLife;
			}
			else {
				pChar->m_bDynamicLight = true;
				pChar->m_vLightColor = g_CharLightList[CANNON].vLightColor;
				pChar->m_vLightColor.x = 1.0f;
				pChar->m_iDLightType = CANNON;
				pChar->m_fLightLife = g_CharLightList[CANNON].fLife;
			}

			if (pOwnerCharacter->IsHero())
			{
				RGetDynamicLightManager()->AddLight(GUNFIRE, pos);
			}
		}
	}

	if (ZGetMyUID() == pOwnerCharacter->GetUID())
	{
		ZItem* pSelItem = pOwnerCharacter->GetItems()->GetSelectedWeapon();
		if (pSelItem && pSelItem->GetDesc() &&
			pSelItem->GetDesc()->IsSpendableItem())
		{
			ZMyItemNode* pItemNode = ZGetMyInfo()->GetItemList()->GetEquipedItem((MMatchCharItemParts)sel_type);
			if (pItemNode)
			{
				pItemNode->SetItemCount(pItemNode->GetItemCount() - 1);
				ZPostRequestUseSpendableNormalItem(pItemNode->GetUID());
			}
		}
	}
}

bool ZGame::CheckWall(ZObject* pObj1, ZObject* pObj2, bool bCoherentToPeer)
{
	if ((pObj1 == NULL) || (pObj2 == NULL))
		return false;

	if ((pObj1->GetVisualMesh() == NULL) || (pObj2->GetVisualMesh() == NULL))
		return false;

	rvector p1 = pObj1->GetPosition() + rvector(0.f, 0.f, 100.f);
	rvector p2 = pObj2->GetPosition() + rvector(0.f, 0.f, 100.f);

	if (bCoherentToPeer)
	{
		p1.x = short(p1.x);
		p1.y = short(p1.y);
		p1.z = short(p1.z);
		p2.x = short(p2.x);
		p2.y = short(p2.y);
		p2.z = short(p2.z);
	}

	rvector dir = p2 - p1;

	Normalize(dir);

	ZPICKINFO pickinfo;

	if (Pick(pObj1, p1, dir, &pickinfo)) {
		if (pickinfo.bBspPicked)
			return true;
	}

	return false;
}
bool ZGame::CheckWall(ZObject* pObj1, ZObject* pObj2, int& nDebugRegister, bool bCoherentToPeer)
{
	if ((pObj1 == NULL) || (pObj2 == NULL))
	{
		nDebugRegister = -10;
		return false;
	}

	if ((pObj1->GetVisualMesh() == NULL) || (pObj2->GetVisualMesh() == NULL))
	{
		nDebugRegister = -10;
		return false;
	}

	rvector p1 = pObj1->GetPosition() + rvector(0.f, 0.f, 100.f);
	rvector p2 = pObj2->GetPosition() + rvector(0.f, 0.f, 100.f);

	if (bCoherentToPeer)
	{
		p1.x = short(p1.x);
		p1.y = short(p1.y);
		p1.z = short(p1.z);
		p2.x = short(p2.x);
		p2.y = short(p2.y);
		p2.z = short(p2.z);
	}

	rvector dir = p2 - p1;

	Normalize(dir);

	ZPICKINFO pickinfo;

	if (Pick(pObj1, p1, dir, &pickinfo)) {
		if (pickinfo.bBspPicked)
		{
			nDebugRegister = FOR_DEBUG_REGISTER;
			return true;
		}
	}
	nDebugRegister = -10;
	return false;
}

void ZGame::OnExplosionGrenade(MUID uidOwner, rvector pos, float fDamage, float fRange, float fMinDamage, float fKnockBack, MMatchTeam nTeamID)
{
	ZObject* pTarget = NULL;

	float fDist, fDamageRange;

	for (ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor)
	{
		pTarget = (*itor).second;
		bool bReturnValue = !pTarget || pTarget->IsDie();
		if (!pTarget || pTarget->IsDie())
			PROTECT_DEBUG_REGISTER(bReturnValue)
			continue;

		fDist = Magnitude(pos - (pTarget->GetPosition() + rvector(0, 0, 80)));
		bReturnValue = fDist >= fRange;
		if (fDist >= fRange)
			PROTECT_DEBUG_REGISTER(bReturnValue)
			continue;

		rvector dir = pos - (pTarget->GetPosition() + rvector(0, 0, 80));
		Normalize(dir);

		if (GetDistance(pos, pTarget->GetPosition() + rvector(0, 0, 50), pTarget->GetPosition() + rvector(0, 0, 130)) < 50)
		{
			fDamageRange = 1.f;
		}
		else
		{
#define MAX_DMG_RANGE	50.f
			fDamageRange = 1.f - (1.f - fMinDamage) * (max(fDist - MAX_DMG_RANGE, 0) / (fRange - MAX_DMG_RANGE));
		}

		ZActor* pATarget = MDynamicCast(ZActor, pTarget);

		bool bPushSkip = false;

		if (pATarget)
		{
			bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;
		}

		if (bPushSkip == false)
		{
			pTarget->AddVelocity(fKnockBack * 7.f * (fRange - fDist) * -dir);
		}
		else
		{
			ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met");
		}

		ZCharacter* pOwnerCharacter = ZGetGame()->m_CharacterManager.Find(uidOwner);
		if (pOwnerCharacter)
		{
			CheckCombo(pOwnerCharacter, pTarget, !bPushSkip);
			CheckStylishAction(pOwnerCharacter);
		}

		float fActualDamage = fDamage * fDamageRange;
		float fRatio = ZItem::GetPiercingRatio(MWT_FRAGMENTATION, eq_parts_chest);
		pTarget->OnDamaged(pOwnerCharacter, pos, ZD_EXPLOSION, MWT_FRAGMENTATION, fActualDamage, fRatio);
	}

#define SHOCK_RANGE		1500.f

	ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	float fPower = (SHOCK_RANGE - Magnitude(pTargetCharacter->GetPosition() + rvector(0, 0, 50) - pos)) / SHOCK_RANGE;

	if (fPower > 0)
		ZGetGameInterface()->GetCamera()->Shock(fPower * 500.f, .5f, rvector(0.0f, 0.0f, -1.0f));

	GetWorld()->GetWaters()->CheckSpearing(pos, pos + rvector(0, 0, MAX_WATER_DEEP), 500, 0.8f);
}

void ZGame::OnExplosionMagic(ZWeaponMagic* pWeapon, MUID uidOwner, rvector pos, float fMinDamage, float fKnockBack, MMatchTeam nTeamID, bool bSkipNpc)
{
	ZObject* pTarget = NULL;

	float fRange = 100.f * pWeapon->GetDesc()->fEffectArea;
	float fDist, fDamageRange;

	for (ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor)
	{
		pTarget = (*itor).second;
		bool bForDebugRegister = !pTarget || pTarget->IsDie() || pTarget->IsNPC();
		if (!pTarget || pTarget->IsDie() || pTarget->IsNPC())
			PROTECT_DEBUG_REGISTER(bForDebugRegister)
			continue;
		bForDebugRegister = !pWeapon->GetDesc()->IsAreaTarget() && pWeapon->GetTarget() != pTarget->GetUID();
		if (!pWeapon->GetDesc()->IsAreaTarget() && pWeapon->GetTarget() != pTarget->GetUID())
			PROTECT_DEBUG_REGISTER(bForDebugRegister)
			continue;

		{
			fDist = Magnitude(pos - (pTarget->GetPosition() + rvector(0, 0, 80)));
			if (pWeapon->GetDesc()->IsAreaTarget())
			{
				PROTECT_DEBUG_REGISTER(fDist > fRange) continue;

				if (GetDistance(pos, pTarget->GetPosition() + rvector(0, 0, 50), pTarget->GetPosition() + rvector(0, 0, 130)) < 50)
				{
					fDamageRange = 1.f;
				}
				else
				{
#define MAX_DMG_RANGE	50.f

					fDamageRange = 1.f - (1.f - fMinDamage) * (max(fDist - MAX_DMG_RANGE, 0) / (fRange - MAX_DMG_RANGE));
				}
			}
			else
			{
				fDamageRange = 1.f;
			}

			float fDamage = pWeapon->GetDesc()->nModDamage;
			bForDebugRegister = pWeapon && pWeapon->GetDesc()->CheckResist(pTarget, &fDamage);
			if (!(pWeapon->GetDesc()->CheckResist(pTarget, &fDamage)))
				PROTECT_DEBUG_REGISTER(bForDebugRegister)
				continue;

			ZCharacter* pOwnerCharacter = ZGetGame()->m_CharacterManager.Find(uidOwner);
			if (pOwnerCharacter)
			{
				CheckCombo(pOwnerCharacter, pTarget, true);
				CheckStylishAction(pOwnerCharacter);
			}

			rvector dir = pos - (pTarget->GetPosition() + rvector(0, 0, 80));
			Normalize(dir);
			pTarget->AddVelocity(fKnockBack * 7.f * (fRange - fDist) * -dir);

			float fActualDamage = fDamage * fDamageRange;
			float fRatio = ZItem::GetPiercingRatio(MWT_FRAGMENTATION, eq_parts_chest);
			pTarget->OnDamaged(pOwnerCharacter, pos, ZD_MAGIC, MWT_FRAGMENTATION, fActualDamage, fRatio);
		}
	}

	if (pWeapon->GetDesc()->bCameraShock)
	{
		ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
		const float fDefaultPower = 500.0f;
		float fShockRange = pWeapon->GetDesc()->fCameraRange;
		float fDuration = pWeapon->GetDesc()->fCameraDuration;
		float fPower = (fShockRange - Magnitude(pTargetCharacter->GetPosition() + rvector(0, 0, 50) - pos)) / fShockRange;
		fPower *= pWeapon->GetDesc()->fCameraPower;

		if (fPower > 0)
		{
			ZGetGameInterface()->GetCamera()->Shock(fPower * fDefaultPower, fDuration, rvector(0.0f, 0.0f, -1.0f));
		}
	}

	GetWorld()->GetWaters()->CheckSpearing(pos, pos + rvector(0, 0, MAX_WATER_DEEP), 500, 0.8f);
}

void ZGame::OnExplosionMagicThrow(ZWeaponMagic* pWeapon, MUID uidOwner, rvector pos, float fMinDamage, float fKnockBack, MMatchTeam nTeamID, bool bSkipNpc, rvector from, rvector to)
{
	ZObject* pTarget = NULL;

	float fDist, fDamageRange;

	for (ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor)
	{
		pTarget = (*itor).second;
		bool bForDebugRegister = !pTarget || pTarget->IsDie() || pTarget->IsNPC();
		if (!pTarget || pTarget->IsDie() || pTarget->IsNPC())
			PROTECT_DEBUG_REGISTER(bForDebugRegister)
			continue;
		bForDebugRegister = !pWeapon->GetDesc()->IsAreaTarget() && pWeapon->GetTarget() != pTarget->GetUID();
		if (!pWeapon->GetDesc()->IsAreaTarget() && pWeapon->GetTarget() != pTarget->GetUID())
			PROTECT_DEBUG_REGISTER(bForDebugRegister)
			continue;

		fDist = GetDistance(pTarget->GetPosition() + rvector(0, 0, 80), from, to);

		if (fDist > pWeapon->GetDesc()->fColRadius + 100)
			continue;

		if (pWeapon->GetDesc()->IsAreaTarget())
		{
			if (fDist > pWeapon->GetDesc()->fColRadius + 80)
			{
				fDamageRange = 0.1f;
			}
			else
			{
				fDamageRange = 1.f - 0.9f * fDist / (pWeapon->GetDesc()->fColRadius + 80);
			}
		}
		else
		{
			fDamageRange = 1.f;
		}

		float fDamage = pWeapon->GetDesc()->nModDamage;
		bForDebugRegister = pWeapon && pWeapon->GetDesc()->CheckResist(pTarget, &fDamage);
		if (!(pWeapon->GetDesc()->CheckResist(pTarget, &fDamage)))
			PROTECT_DEBUG_REGISTER(bForDebugRegister)
			continue;

		ZCharacter* pOwnerCharacter = ZGetGame()->m_CharacterManager.Find(uidOwner);
		if (pOwnerCharacter)
		{
			CheckCombo(pOwnerCharacter, pTarget, true);
			CheckStylishAction(pOwnerCharacter);
		}

		float fActualDamage = fDamage * fDamageRange;
		float fRatio = ZItem::GetPiercingRatio(MWT_FRAGMENTATION, eq_parts_chest);
		pTarget->OnDamaged(pOwnerCharacter, pos, ZD_MAGIC, MWT_FRAGMENTATION, fActualDamage, fRatio);
	}

	if (pWeapon->GetDesc()->bCameraShock)
	{
		ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
		const float fDefaultPower = 500.0f;
		float fShockRange = pWeapon->GetDesc()->fCameraRange;
		float fDuration = pWeapon->GetDesc()->fCameraDuration;
		float fPower = (fShockRange - Magnitude(pTargetCharacter->GetPosition() + rvector(0, 0, 50) - pos)) / fShockRange;
		fPower *= pWeapon->GetDesc()->fCameraPower;

		if (fPower > 0)
		{
			ZGetGameInterface()->GetCamera()->Shock(fPower * fDefaultPower, fDuration, rvector(0.0f, 0.0f, -1.0f));
		}
	}

	GetWorld()->GetWaters()->CheckSpearing(pos, pos + rvector(0, 0, MAX_WATER_DEEP), 500, 0.8f);
}

void ZGame::OnExplosionMagicNonSplash(ZWeaponMagic* pWeapon, MUID uidOwner, MUID uidTarget, rvector pos, float fKnockBack)
{
	ZObject* pTarget = m_CharacterManager.Find(uidTarget);
	bool bForDebugRegister = pTarget == NULL || pTarget->IsNPC();
	if (pTarget == NULL || pTarget->IsNPC()) PROTECT_DEBUG_REGISTER(bForDebugRegister) return;

	bForDebugRegister = !pTarget || pTarget->IsDie();
	if (!pTarget || pTarget->IsDie())
		PROTECT_DEBUG_REGISTER(bForDebugRegister)
		return;

	float fDamage = pWeapon->GetDesc()->nModDamage;
	bForDebugRegister = pWeapon && pWeapon->GetDesc()->CheckResist(pTarget, &fDamage);
	if (!pWeapon->GetDesc()->CheckResist(pTarget, &fDamage))
		PROTECT_DEBUG_REGISTER(bForDebugRegister)
		return;

	ZCharacter* pOwnerCharacter = ZGetGame()->m_CharacterManager.Find(uidOwner);
	if (pOwnerCharacter)
	{
		CheckCombo(pOwnerCharacter, pTarget, true);
		CheckStylishAction(pOwnerCharacter);
	}

	rvector dir = pos - (pTarget->GetPosition() + rvector(0, 0, 80));
	Normalize(dir);
	pTarget->AddVelocity(fKnockBack * 7.f * -dir);

	float fRatio = ZItem::GetPiercingRatio(MWT_FRAGMENTATION, eq_parts_chest);
	pTarget->OnDamaged(pOwnerCharacter, pos, ZD_MAGIC, MWT_FRAGMENTATION, fDamage, fRatio);
}

int ZGame::SelectSlashEffectMotion(ZCharacter* pCharacter)
{
	if (pCharacter == NULL || pCharacter->GetSelectItemDesc() == NULL) return SEM_None;

	ZC_STATE_LOWER lower = pCharacter->m_AniState_Lower.Ref();

	int nAdd = 0;
	int ret = 0;

	MMatchWeaponType nType = pCharacter->GetSelectItemDesc()->m_nWeaponType.Ref();

	if (pCharacter->IsMan()) {
		if (lower == ZC_STATE_LOWER_ATTACK1) { nAdd = 0; }
		else if (lower == ZC_STATE_LOWER_ATTACK2) { nAdd = 1; }
		else if (lower == ZC_STATE_LOWER_ATTACK3) { nAdd = 2; }
		else if (lower == ZC_STATE_LOWER_ATTACK4) { nAdd = 3; }
		else if (lower == ZC_STATE_LOWER_ATTACK5) { nAdd = 4; }
		else if (lower == ZC_STATE_LOWER_UPPERCUT) { return SEM_ManUppercut; }

		if (nType == MWT_KATANA)		return SEM_ManSlash1 + nAdd;
		else if (nType == MWT_DOUBLE_KATANA)	return SEM_ManDoubleSlash1 + nAdd;
		else if (nType == MWT_GREAT_SWORD)	return SEM_ManGreatSwordSlash1 + nAdd;
	}
	else {
		if (lower == ZC_STATE_LOWER_ATTACK1) { nAdd = 0; }
		else if (lower == ZC_STATE_LOWER_ATTACK2) { nAdd = 1; }
		else if (lower == ZC_STATE_LOWER_ATTACK3) { nAdd = 2; }
		else if (lower == ZC_STATE_LOWER_ATTACK4) { nAdd = 3; }
		else if (lower == ZC_STATE_LOWER_ATTACK5) { nAdd = 4; }
		else if (lower == ZC_STATE_LOWER_UPPERCUT) { return SEM_WomanUppercut; }

		if (nType == MWT_KATANA)		return SEM_WomanSlash1 + nAdd;
		else if (nType == MWT_DOUBLE_KATANA)	return SEM_WomanDoubleSlash1 + nAdd;
		else if (nType == MWT_GREAT_SWORD)	return SEM_WomanGreatSwordSlash1 + nAdd;
	}

	return SEM_None;
}

void ZGame::OnPeerShot_Melee(const MUID& uidOwner, float fShotTime)
{
	ZObject* pAttacker = m_ObjectManager.GetObject(uidOwner);
	float time = fShotTime;
	bool bReturnValue = !pAttacker;
	if (!pAttacker)
		PROTECT_DEBUG_REGISTER(bReturnValue)
		return;

	ZItem* pItem = pAttacker->GetItems()->GetItem(MMCIP_MELEE);
	bReturnValue = !pItem;
	if (!pItem)
		PROTECT_DEBUG_REGISTER(bReturnValue)
		return;

	MMatchItemDesc* pSkillDesc = pItem->GetDesc();
	bReturnValue = !pSkillDesc;
	if (!pSkillDesc)
		PROTECT_DEBUG_REGISTER(bReturnValue)
	{
		return;
	}

	float fRange = pSkillDesc->m_nRange.Ref();
	if (fRange == 0)
		fRange = 150.f;

	float fAngle = cosf(ToRadian(pSkillDesc->m_nAngle.Ref() * 0.5f));

	if (pAttacker->IsNPC())
	{
		fRange += fRange * 0.2f;
		fAngle -= fAngle * 0.1f;
	}

	fShotTime = GetTime();

	rvector AttackerPos = pAttacker->GetPosition();
	rvector AttackerNorPos = AttackerPos;
	AttackerNorPos.z = 0;

	rvector AttackerDir = pAttacker->m_Direction;
	rvector AttackerNorDir = AttackerDir;
	AttackerNorDir.z = 0;
	Normalize(AttackerNorDir);

	int cm = 0;
	ZCharacter* pOwnerCharacter = m_CharacterManager.Find(uidOwner);
	if (pOwnerCharacter)
		cm = SelectSlashEffectMotion(pOwnerCharacter);

	bool bPlayer = false;
	rvector Pos = pAttacker->GetPosition();
	if (pAttacker == m_pMyCharacter)
	{
		Pos = RCameraPosition;
		bPlayer = true;
	}
	ZApplication::GetSoundEngine()->PlaySoundElseDefault("blade_swing", "blade_swing", rvector(Pos.x, Pos.y, Pos.z), bPlayer);

	bool bHit = false;

	for (ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor)
	{
		ZObject* pVictim = (*itor).second;

		ZModule_HPAP* pModule = (ZModule_HPAP*)pVictim->GetModule(ZMID_HPAP);
		if (pVictim->IsDie())
			PROTECT_DEBUG_REGISTER(pModule->GetHP() == 0)
			continue;

		bReturnValue = pAttacker == pVictim;
		if (pAttacker == pVictim)
			PROTECT_DEBUG_REGISTER(bReturnValue)
			continue;

		bool bRetVal = CanAttack(pAttacker, pVictim);
		if (!bRetVal)
			PROTECT_DEBUG_REGISTER(!CanAttack_DebugRegister(pAttacker, pVictim))
			continue;

		rvector VictimPos, VictimDir, VictimNorDir;
		rvector ZeroVector = rvector(0.0f, 0.0f, 0.0f);
		VictimPos = ZeroVector;
		bRetVal = pVictim->GetHistory(&VictimPos, &VictimDir, fShotTime);
		if (!bRetVal)
			PROTECT_DEBUG_REGISTER(VictimPos == ZeroVector)
			continue;

		if (!pAttacker->IsNPC())
		{
			rvector swordPos = AttackerPos + (AttackerNorDir * 100.f);
			swordPos.z += pAttacker->GetCollHeight() * .5f;
			float fDist = GetDistanceLineSegment(swordPos, VictimPos, VictimPos + rvector(0, 0, pVictim->GetCollHeight()));

			bReturnValue = fDist > fRange;
			if (fDist > fRange)
				PROTECT_DEBUG_REGISTER(bReturnValue)
				continue;

			rvector VictimNorDir = VictimPos - (AttackerPos - (AttackerNorDir * 50.f));
			Normalize(VictimNorDir);

			float fDot = D3DXVec3Dot(&AttackerNorDir, &VictimNorDir);
			bReturnValue = fDot < 0.5f;
			if (fDot < 0.5f)
				PROTECT_DEBUG_REGISTER(bReturnValue)
				continue;
		}

		else
		{
			rvector VictimNorPos = VictimPos;
			VictimNorPos.z = 0;

			rvector VictimNorDir = VictimPos - (AttackerPos - (AttackerNorDir * 50.f));
			VictimNorDir.z = 0;
			Normalize(VictimNorDir);

			float fDist = Magnitude(AttackerNorPos - VictimNorPos);
			bReturnValue = fDist > fRange;
			if (fDist > fRange)
				PROTECT_DEBUG_REGISTER(bReturnValue)
				continue;

			float fDot = D3DXVec3Dot(&AttackerNorDir, &VictimNorDir);
			bReturnValue = fDot < fAngle;
			if (fDot < fAngle)
				PROTECT_DEBUG_REGISTER(bReturnValue)
				continue;

			int nDebugRegister = 0;
			if (!InRanged(pAttacker, pVictim, nDebugRegister))
				PROTECT_DEBUG_REGISTER(nDebugRegister != FOR_DEBUG_REGISTER)
				continue;
		}

		int nDebugRegister = 0;
		bRetVal = CheckWall(pAttacker, pVictim, nDebugRegister, true);
		if (bRetVal)
			PROTECT_DEBUG_REGISTER(nDebugRegister == FOR_DEBUG_REGISTER)
			continue;

		bRetVal = pVictim->IsGuard() && (DotProduct(pVictim->m_Direction, AttackerNorDir) < 0);
		if (pVictim->IsGuard() && (DotProduct(pVictim->m_Direction, AttackerNorDir) < 0))
		{
			PROTECT_DEBUG_REGISTER(bRetVal)
			{
				rvector pos = pVictim->GetPosition();
				pos.z += 120.f;

				ZGetEffectManager()->AddSwordDefenceEffect(pos + (pVictim->m_Direction * 50.f), pVictim->m_Direction);
				pVictim->OnMeleeGuardSuccess();
				return;
			}
		}

		rvector pos = pVictim->GetPosition();
		pos.z += 130.f;
		pos -= AttackerDir * 50.f;

		ZGetEffectManager()->AddBloodEffect(pos, -VictimNorDir);
		ZGetEffectManager()->AddSlashEffect(pos, -VictimNorDir, cm);

		float fActualDamage = CalcActualDamage(pAttacker, pVictim, (float)pSkillDesc->m_nDamage.Ref());
		float fRatio = pItem->GetPiercingRatio(pSkillDesc->m_nWeaponType.Ref(), eq_parts_chest);
		pVictim->OnDamaged(pAttacker, pAttacker->GetPosition(), ZD_MELEE, pSkillDesc->m_nWeaponType.Ref(),
			fActualDamage, fRatio, cm);

		ZActor* pATarget = MDynamicCast(ZActor, pVictim);

		bool bPushSkip = false;
		if (pATarget)
			bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;

		float fKnockbackForce = pItem->GetKnockbackForce();
		if (bPushSkip)
		{
			ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met");
			fKnockbackForce = 1.0f;
		}

		pVictim->OnKnockback(pAttacker->m_Direction, fKnockbackForce);

		ZGetSoundEngine()->PlaySoundBladeDamage(pSkillDesc, pos);

		if (pAttacker == m_pMyCharacter)
		{
			CheckCombo(m_pMyCharacter, pVictim, !bPushSkip);
			CheckStylishAction(m_pMyCharacter);
		}

		bHit = true;
	}

	if (!bHit)
	{
		rvector vPos = pAttacker->GetPosition();
		rvector vDir = AttackerNorDir;

		vPos.z += 130.f;

		RBSPPICKINFO bpi;

		if (GetWorld()->GetBsp()->Pick(vPos, vDir, &bpi)) {
			float fDist = Magnitude(vPos - bpi.PickPos);

			if (fDist < fRange) {
				rplane r = bpi.pInfo->plane;
				rvector vWallDir = rvector(r.a, r.b, r.c);
				Normalize(vWallDir);

				ZGetEffectManager()->AddSlashEffectWall(bpi.PickPos - (vDir * 5.f), vWallDir, cm);

				rvector pos = bpi.PickPos;

				ZGetSoundEngine()->PlaySoundBladeConcrete(pSkillDesc, pos);
			}
		}
	}

	return;
}

bool ZGame::InRanged(ZObject* pAttacker, ZObject* pVictim)
{
	float fBotAtk = pAttacker->GetPosition().z;
	float fTopAtk = fBotAtk + pAttacker->GetCollHeight() + (pAttacker->GetCollHeight() * 0.5f);

	float fBotVct = pVictim->GetPosition().z;
	float fTopVct = fBotVct + pVictim->GetCollHeight();

	if (fBotVct > fTopAtk)
		return false;

	else if (fTopVct < fBotAtk)
		return false;

	return true;
}

bool ZGame::InRanged(ZObject* pAttacker, ZObject* pVictim, int& nDebugRegister)
{
	float fBotAtk = pAttacker->GetPosition().z;
	float fTopAtk = fBotAtk + pAttacker->GetCollHeight() + (pAttacker->GetCollHeight() * 0.5f);

	float fBotVct = pVictim->GetPosition().z;
	float fTopVct = fBotVct + pVictim->GetCollHeight();

	if (fBotVct > fTopAtk)
	{
		nDebugRegister = -10;
		return false;
	}

	else if (fTopVct < fBotAtk)
	{
		nDebugRegister = -10;
		return false;
	}

	nDebugRegister = FOR_DEBUG_REGISTER;
	return true;
}

void ZGame::OnPeerShot_Range_Damaged(ZObject* pOwner, float fShotTime, const rvector& pos, const rvector& to, ZPICKINFO pickinfo, DWORD dwPickPassFlag, rvector& v1, rvector& v2, ZItem* pItem, rvector& BulletMarkNormal, bool& bBulletMark, ZTargetType& nTargetType)
{
	MMatchItemDesc* pDesc = pItem->GetDesc();
	bool bReturnValue = !pDesc;
	if (!pDesc) PROTECT_DEBUG_REGISTER(bReturnValue) { return; }

	rvector dir = to - pos;

	bReturnValue = !(ZGetGame()->PickHistory(pOwner, fShotTime, pos, to, &pickinfo, dwPickPassFlag));
	if (!(ZGetGame()->PickHistory(pOwner, fShotTime, pos, to, &pickinfo, dwPickPassFlag)))
	{
		PROTECT_DEBUG_REGISTER(bReturnValue)
		{
			v1 = pos;
			v2 = pos + dir * 10000.f;
			nTargetType = ZTT_NOTHING;
			return;
		}
	}

	bReturnValue = (!pickinfo.pObject) && (!pickinfo.bBspPicked);
	if (pickinfo.bBspPicked)
	{
		PROTECT_DEBUG_REGISTER(pickinfo.nBspPicked_DebugRegister == FOR_DEBUG_REGISTER)
		{
			nTargetType = ZTT_OBJECT;

			v1 = pos;
			v2 = pickinfo.bpi.PickPos;

			BulletMarkNormal.x = pickinfo.bpi.pInfo->plane.a;
			BulletMarkNormal.y = pickinfo.bpi.pInfo->plane.b;
			BulletMarkNormal.z = pickinfo.bpi.pInfo->plane.c;
			Normalize(BulletMarkNormal);
			bBulletMark = true;
			return;
		}
	}
	else if ((!pickinfo.pObject) && (!pickinfo.bBspPicked))
	{
		PROTECT_DEBUG_REGISTER(bReturnValue)
		{
			return;
		}
	}
	ZObject* pObject = pickinfo.pObject;
	bool bGuard = pObject->IsGuard() && (pickinfo.info.parts != eq_parts_legs) &&
		DotProduct(dir, pObject->GetDirection()) < 0;

	if (pObject->IsGuard() && (pickinfo.info.parts != eq_parts_legs) &&
		DotProduct(dir, pObject->GetDirection()) < 0)
	{
		PROTECT_DEBUG_REGISTER(bGuard)
		{
			nTargetType = ZTT_CHARACTER_GUARD;
			rvector t_pos = pObject->GetPosition();
			t_pos.z += 100.f;
			ZGetEffectManager()->AddSwordDefenceEffect(t_pos + (-dir * 50.f), -dir);
			pObject->OnGuardSuccess();
			v1 = pos;
			v2 = pickinfo.info.vOut;
			return;
		}
	}

	nTargetType = ZTT_CHARACTER;

	ZActor* pATarget = MDynamicCast(ZActor, pickinfo.pObject);

	bool bPushSkip = false;

	if (pATarget)
	{
		bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;
	}

	float fKnockbackForce = pItem->GetKnockbackForce();

	if (bPushSkip)
	{
		rvector vPos = pOwner->GetPosition() + (pickinfo.pObject->GetPosition() - pOwner->GetPosition()) * 0.1f;
		ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met", vPos);
		fKnockbackForce = 1.0f;
	}

	pickinfo.pObject->OnKnockback(pOwner->m_Direction, fKnockbackForce);

	float fActualDamage = CalcActualDamage(pOwner, pickinfo.pObject, (float)pDesc->m_nDamage.Ref());
	float fRatio = pItem->GetPiercingRatio(pDesc->m_nWeaponType.Ref(), pickinfo.info.parts);
	ZDAMAGETYPE dt = (pickinfo.info.parts == eq_parts_head) ? ZD_BULLET_HEADSHOT : ZD_BULLET;

	if (ZGetGameClient()->GetChannelType() == MCHANNEL_TYPE_CLAN) {
		if (ZGetGameClient()->GetMatchStageSetting()->GetAntiLead() == false) {
			pickinfo.pObject->OnDamaged(pOwner, pOwner->GetPosition(), dt, pDesc->m_nWeaponType.Ref(), fActualDamage, fRatio);
		}
		else {
			((ZCharacter*)(pickinfo.pObject))->OnDamagedAPlayer(pOwner, pOwner->GetPosition(), dt, pDesc->m_nWeaponType.Ref(), fActualDamage, fRatio);
		}
	}
	else if (ZGetGameClient()->GetChannelType() == MCHANNEL_TYPE_DUELTOURNAMENT) {
		if (strstr(ZGetGameClient()->GetChannelName(), "Lead")) {
			pickinfo.pObject->OnDamaged(pOwner, pOwner->GetPosition(), dt, pDesc->m_nWeaponType.Ref(), fActualDamage, fRatio);
		}
		else {
			((ZCharacter*)(pickinfo.pObject))->OnDamagedAPlayer(pOwner, pOwner->GetPosition(), dt, pDesc->m_nWeaponType.Ref(), fActualDamage, fRatio);
		}
	}
	else {
		if (pickinfo.pObject->IsNPC() == true || strstr(ZGetGameClient()->GetChannelName(), "Lead"))
		{
			pickinfo.pObject->OnDamaged(pOwner, pOwner->GetPosition(), dt, pDesc->m_nWeaponType.Ref(), fActualDamage, fRatio);
		}
		else
		{
			((ZCharacter*)(pickinfo.pObject))->OnDamagedAPlayer(pOwner, pOwner->GetPosition(), dt, pDesc->m_nWeaponType.Ref(), fActualDamage, fRatio);
		}
	}

	if (pOwner == m_pMyCharacter)
	{
		CheckCombo(m_pMyCharacter, pickinfo.pObject, !bPushSkip);
		CheckStylishAction(m_pMyCharacter);
	}

	v1 = pos;
	v2 = pickinfo.info.vOut;
}
void ZGame::OnPeerShot_Range(const MMatchCharItemParts sel_type, const MUID& uidOwner, float fShotTime, const rvector& pos, const rvector& to)
{
	ZObject* pOwner = m_ObjectManager.GetObject(uidOwner);
	if (!pOwner) return;

	ZItem* pItem = pOwner->GetItems()->GetItem(sel_type);
	if (!pItem) return;
	MMatchItemDesc* pDesc = pItem->GetDesc();
	if (!pDesc) { return; }

	rvector dir = to - pos;

	Normalize(dir);

	rvector v1, v2;
	rvector BulletMarkNormal;
	bool bBulletMark = false;
	ZTargetType nTargetType = ZTT_OBJECT;

	ZPICKINFO pickinfo;

	memset(&pickinfo, 0, sizeof(ZPICKINFO));

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET | RM_FLAG_PASSBULLET;

	pOwner->Tremble(8.f, 50, 30);

	OnPeerShot_Range_Damaged(pOwner, fShotTime, pos, to, pickinfo, dwPickPassFlag, v1, v2, pItem, BulletMarkNormal, bBulletMark, nTargetType);

	bool bPlayer = false;
	rvector Pos = v1;
	if (pOwner == m_pMyCharacter)
	{
		Pos = RCameraPosition;
		bPlayer = true;
	}

	ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	ZApplication::GetSoundEngine()->PlaySEFire(pDesc, Pos.x, Pos.y, Pos.z, bPlayer);
#define SOUND_CULL_DISTANCE 1500.0F
	if (D3DXVec3LengthSq(&(v2 - pTargetCharacter->GetPosition())) < (SOUND_CULL_DISTANCE * SOUND_CULL_DISTANCE))
	{
		if (nTargetType == ZTT_OBJECT) {
			ZGetSoundEngine()->PlaySEHitObject(v2.x, v2.y, v2.z, pickinfo.bpi);
		}

		if (nTargetType == ZTT_CHARACTER) {
			ZGetSoundEngine()->PlaySEHitBody(v2.x, v2.y, v2.z);
		}
	}

	bool bDrawFireEffects = isInViewFrustum(v1, 100.f, RGetViewFrustum());

	if (!isInViewFrustum(v1, v2, RGetViewFrustum())
		&& !bDrawFireEffects) return;

	bool bDrawTargetEffects = isInViewFrustum(v2, 100.f, RGetViewFrustum());

	GetWorld()->GetWaters()->CheckSpearing(v1, v2, 250, 0.3);

	ZCharacterObject* pCOwnerObject = MDynamicCast(ZCharacterObject, pOwner);

	if (pCOwnerObject)
	{
		rvector pdir = v2 - v1;
		Normalize(pdir);

		int size = 3;

		rvector v[6];

		if (pCOwnerObject->IsRendered())
			size = pCOwnerObject->GetWeapondummyPos(v);
		else
		{
			size = 6;
			v[0] = v[1] = v[2] = v1;
			v[3] = v[4] = v[5] = v[0];
		}

		MMatchWeaponType wtype = pDesc->m_nWeaponType.Ref();
		bool bSlugOutput = pDesc->m_bSlugOutput;

		if (bBulletMark == false) BulletMarkNormal = -pdir;

		ZGetEffectManager()->AddShotEffect(v, size, v2, BulletMarkNormal, nTargetType, wtype, bSlugOutput, pCOwnerObject, bDrawFireEffects, bDrawTargetEffects);

		ZCharacterObject* pChar;

		if (ZGetConfiguration()->GetVideo()->bDynamicLight && pCOwnerObject != NULL)
		{
			pChar = pCOwnerObject;

			if (pChar->m_bDynamicLight)
			{
				pChar->m_vLightColor = g_CharLightList[GUN].vLightColor;
				pChar->m_fLightLife = g_CharLightList[GUN].fLife;
			}
			else
			{
				pChar->m_bDynamicLight = true;
				pChar->m_vLightColor = g_CharLightList[GUN].vLightColor;
				pChar->m_vLightColor.x = 1.0f;
				pChar->m_iDLightType = GUN;
				pChar->m_fLightLife = g_CharLightList[GUN].fLife;
			}
		}
	}

	GetWorld()->GetFlags()->CheckSpearing(v1, v2, BULLET_SPEAR_EMBLEM_POWER);
	if (Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->AddLightSource(v1, 2.0f, 75);
}

void ZGame::OnPeerShot_Shotgun(ZItem* pItem, ZCharacter* pOwnerCharacter, float fShotTime, const rvector& pos, const rvector& to)
{
	ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if (!pTargetCharacter) return;

	MMatchItemDesc* pDesc = pItem->GetDesc();
	if (!pDesc) { return; }

#define SHOTGUN_BULLET_COUNT	12
#define SHOTGUN_DIFFUSE_RANGE	0.1f

	if (pOwnerCharacter == NULL) return;

	int* seed = (int*)&fShotTime;
	srand(*seed);

	bool bHitGuard = false, bHitBody = false, bHitGround = false, bHitEnemy = false;
	rvector GuardPos, BodyPos, GroundPos;
	bool waterSound = false;

	rvector v1, v2;
	rvector dir;

	rvector origdir = to - pos;
	Normalize(origdir);

	int nHitCount = 0;
	vector<MTD_ShotInfo*> vShots;
	ZPICKINFO pickinfo;
	for (int i = 0; i < SHOTGUN_BULLET_COUNT; i++)
	{
		dir = origdir;
		{
			rvector r, up(0, 0, 1), right;
			D3DXQUATERNION q;
			D3DXMATRIX mat;

			float fAngle = (rand() % (31415 * 2)) / 1000.0f;
			float fForce = RANDOMFLOAT * SHOTGUN_DIFFUSE_RANGE;

			D3DXVec3Cross(&right, &dir, &up);
			D3DXVec3Normalize(&right, &right);
			D3DXMatrixRotationAxis(&mat, &right, fForce);
			D3DXVec3TransformCoord(&r, &dir, &mat);

			D3DXQuaternionRotationAxis(&q, &dir, fAngle);
			D3DXMatrixRotationQuaternion(&mat, &q);
			D3DXVec3TransformCoord(&r, &r, &mat);

			dir = r;
		}
		rvector BulletMarkNormal;
		bool bBulletMark = false;
		ZTargetType nTargetType = ZTT_OBJECT;

		ZPICKINFO pickinfo;

		memset(&pickinfo, 0, sizeof(ZPICKINFO));

		const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET | RM_FLAG_PASSBULLET;
		MTD_ShotInfo* pShotInfo = OnPeerShotgun_Damaged(pOwnerCharacter, fShotTime, pos, dir, pickinfo, dwPickPassFlag, v1, v2, pItem, BulletMarkNormal, bBulletMark, nTargetType, bHitEnemy);

		if (pShotInfo)
			vShots.push_back(pShotInfo);
	}

	if (vShots.size() > 0 && !ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
	{
		ZCharacter* pCharacter = (ZCharacter*)ZGetCharacterManager()->Find(MUID(0, vShots[0]->nLowId));
		if (pCharacter && !m_pMyCharacter->IsDie())
		{
			if (!GetMatch()->IsTeamPlay())
				pCharacter->OnDamagedAPlayer(pOwnerCharacter, vShots);
			else if (GetMatch()->IsTeamPlay() && ZGetGame()->GetMatch()->GetTeamKillEnabled() && pCharacter->IsTeam(m_pMyCharacter))
				pCharacter->OnDamagedAPlayer(pOwnerCharacter, vShots);
			else if (GetMatch()->IsTeamPlay() && !pCharacter->IsTeam(m_pMyCharacter))
				pCharacter->OnDamagedAPlayer(pOwnerCharacter, vShots);
		}

		vShots.clear();
	}

	if (bHitEnemy) {
		CheckStylishAction(pOwnerCharacter);
		CheckCombo(pOwnerCharacter, NULL, true);
	}

	ZApplication::GetSoundEngine()->PlaySEFire(pItem->GetDesc(), pos.x, pos.y, pos.z, (pOwnerCharacter == m_pMyCharacter));

	if (!pOwnerCharacter->IsRendered()) return;

	rvector v[6];

	int _size = pOwnerCharacter->GetWeapondummyPos(v);

	dir = to - pos;
	Normalize(dir);

	ZGetEffectManager()->AddShotgunEffect(const_cast<rvector&>(pos), v[1], dir, pOwnerCharacter);

	ZCharacter* pChar;
	if (ZGetConfiguration()->GetVideo()->bDynamicLight && pOwnerCharacter != NULL)
	{
		pChar = pOwnerCharacter;

		if (pChar->m_bDynamicLight)
		{
			pChar->m_vLightColor = g_CharLightList[SHOTGUN].vLightColor;
			pChar->m_fLightLife = g_CharLightList[SHOTGUN].fLife;
		}
		else
		{
			pChar->m_bDynamicLight = true;
			pChar->m_vLightColor = g_CharLightList[SHOTGUN].vLightColor;
			pChar->m_vLightColor.x = 1.0f;
			pChar->m_iDLightType = SHOTGUN;
			pChar->m_fLightLife = g_CharLightList[SHOTGUN].fLife;
		}
	}
	if (Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->AddLightSource(v1, 2.0f, 200);
}

MTD_ShotInfo* ZGame::OnPeerShotgun_Damaged(ZObject* pOwner, float fShotTime, const rvector& pos, rvector& dir, ZPICKINFO pickinfo, DWORD dwPickPassFlag, rvector& v1, rvector& v2, ZItem* pItem, rvector& BulletMarkNormal, bool& bBulletMark, ZTargetType& nTargetType, bool& bHitEnemy)
{
	ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	bool bReturnValue = !pTargetCharacter;
	if (!pTargetCharacter)PROTECT_DEBUG_REGISTER(bReturnValue) return NULL;

	MMatchItemDesc* pDesc = pItem->GetDesc();
	bReturnValue = !pDesc;
	if (!pDesc)PROTECT_DEBUG_REGISTER(bReturnValue) { _ASSERT(FALSE); return NULL; }

	bool waterSound = false;
	bReturnValue = !(ZGetGame()->PickHistory(pOwner, fShotTime, pos, pos + 10000.f * dir, &pickinfo, dwPickPassFlag));
	if (!(ZGetGame()->PickHistory(pOwner, fShotTime, pos, pos + 10000.f * dir, &pickinfo, dwPickPassFlag)))
	{
		PROTECT_DEBUG_REGISTER(bReturnValue)
		{
			v1 = pos;
			v2 = pos + dir * 10000.f;
			nTargetType = ZTT_NOTHING;
			waterSound = GetWorld()->GetWaters()->CheckSpearing(v1, v2, 250, 0.3, !waterSound);
			return NULL;
		}
	}
	bReturnValue = (!pickinfo.pObject) && (!pickinfo.bBspPicked);
	if (pickinfo.bBspPicked)
	{
		PROTECT_DEBUG_REGISTER(pickinfo.nBspPicked_DebugRegister == FOR_DEBUG_REGISTER)
		{
			nTargetType = ZTT_OBJECT;

			v1 = pos;
			v2 = pickinfo.bpi.PickPos;

			BulletMarkNormal.x = pickinfo.bpi.pInfo->plane.a;
			BulletMarkNormal.y = pickinfo.bpi.pInfo->plane.b;
			BulletMarkNormal.z = pickinfo.bpi.pInfo->plane.c;
			Normalize(BulletMarkNormal);
			bBulletMark = true;

			bool bDrawTargetEffects = isInViewFrustum(v2, 20.f, RGetViewFrustum());
			if (bDrawTargetEffects)
			{
				rvector pdir = v2 - v1;
				Normalize(pdir);

				int size = 3;
				bool bDrawFireEffects = isInViewFrustum(v1, 100.f, RGetViewFrustum());
				rvector v[6];

				ZCharacterObject* pCOwnerObject = MDynamicCast(ZCharacterObject, pOwner);
				if (pCOwnerObject->IsRendered())
					size = pCOwnerObject->GetWeapondummyPos(v);
				else
				{
					size = 6;
					v[0] = v[1] = v[2] = v1;
					v[3] = v[4] = v[5] = v[0];
				}

				MMatchWeaponType wtype = pDesc->m_nWeaponType.Ref();
				bool bSlugOutput = pDesc->m_bSlugOutput;
				ZGetEffectManager()->AddBulletMark(v2, BulletMarkNormal);
			}
			waterSound = GetWorld()->GetWaters()->CheckSpearing(v1, v2, 250, 0.3, !waterSound);
			return NULL;
		}
	}
	else if ((!pickinfo.pObject) && (!pickinfo.bBspPicked))
	{
		PROTECT_DEBUG_REGISTER(bReturnValue)
		{
			return NULL;
		}
	}

	ZObject* pObject = pickinfo.pObject;
	bool bGuard = pObject->IsGuard() && (pickinfo.info.parts != eq_parts_legs) &&
		DotProduct(dir, pObject->GetDirection()) < 0;

	if (pObject->IsGuard() && (pickinfo.info.parts != eq_parts_legs) &&
		DotProduct(dir, pObject->GetDirection()) < 0)
	{
		PROTECT_DEBUG_REGISTER(bGuard)
		{
			nTargetType = ZTT_CHARACTER_GUARD;
			rvector t_pos = pObject->GetPosition();
			t_pos.z += 100.f;
			ZGetEffectManager()->AddSwordDefenceEffect(t_pos + (-dir * 50.f), -dir);
			pObject->OnGuardSuccess();
			v1 = pos;
			v2 = pickinfo.info.vOut;
			return NULL;
		}
	}

	ZActor* pATarget = MDynamicCast(ZActor, pObject);
	nTargetType = ZTT_CHARACTER;

	bool bPushSkip = false;

	if (pATarget)
	{
		bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;
	}

	float fKnockbackForce = pItem->GetKnockbackForce() / (.5f * float(SHOTGUN_BULLET_COUNT));

	if (bPushSkip)
	{
		rvector vPos = pOwner->GetPosition() + (pickinfo.pObject->GetPosition() - pOwner->GetPosition()) * 0.1f;
		ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met", vPos);
		fKnockbackForce = 1.0f;
	}

	pObject->OnKnockback(dir, fKnockbackForce);

	float fActualDamage = CalcActualDamage(pOwner, pObject, (float)pDesc->m_nDamage.Ref());
	float fRatio = pItem->GetPiercingRatio(pDesc->m_nWeaponType.Ref(), pickinfo.info.parts);
	ZDAMAGETYPE dt = (pickinfo.info.parts == eq_parts_head) ? ZD_BULLET_HEADSHOT : ZD_BULLET;

	MTD_ShotInfo* pShotInfo = NULL;

	if (ZGetGameClient()->GetChannelType() == MCHANNEL_TYPE_CLAN) {
		if (ZGetGameClient()->GetMatchStageSetting()->GetAntiLead() == false) {
			pickinfo.pObject->OnDamaged(pOwner, pOwner->GetPosition(), dt, pDesc->m_nWeaponType.Ref(), fActualDamage, fRatio);
		}
		else {
			pShotInfo = new MTD_ShotInfo;
			pShotInfo->nLowId = pickinfo.pObject->GetUID().Low;
			pShotInfo->fDamage = fActualDamage;
			pShotInfo->fPosX = pOwner->GetPosition().x;
			pShotInfo->fPosY = pOwner->GetPosition().y;
			pShotInfo->fPosZ = pOwner->GetPosition().z;
			pShotInfo->fRatio = fRatio;
			pShotInfo->nDamageType = dt;
			pShotInfo->nWeaponType = pDesc->m_nWeaponType.Ref();
		}
	}
	else if (ZGetGameClient()->GetChannelType() == MCHANNEL_TYPE_DUELTOURNAMENT) {
		if (strstr(ZGetGameClient()->GetChannelName(), "Lead")) {
			pickinfo.pObject->OnDamaged(pOwner, pOwner->GetPosition(), dt, pDesc->m_nWeaponType.Ref(), fActualDamage, fRatio);
		}
		else {
			pShotInfo = new MTD_ShotInfo;
			pShotInfo->nLowId = pickinfo.pObject->GetUID().Low;
			pShotInfo->fDamage = fActualDamage;
			pShotInfo->fPosX = pOwner->GetPosition().x;
			pShotInfo->fPosY = pOwner->GetPosition().y;
			pShotInfo->fPosZ = pOwner->GetPosition().z;
			pShotInfo->fRatio = fRatio;
			pShotInfo->nDamageType = dt;
			pShotInfo->nWeaponType = pDesc->m_nWeaponType.Ref();
		}
	}
	else {
		if (pickinfo.pObject->IsNPC() == true || strstr(ZGetGameClient()->GetChannelName(), "Lead"))
		{
			pickinfo.pObject->OnDamaged(pOwner, pOwner->GetPosition(), dt, pDesc->m_nWeaponType.Ref(), fActualDamage, fRatio);
		}
		else
		{
			pShotInfo = new MTD_ShotInfo;
			pShotInfo->nLowId = pickinfo.pObject->GetUID().Low;
			pShotInfo->fDamage = fActualDamage;
			pShotInfo->fPosX = pOwner->GetPosition().x;
			pShotInfo->fPosY = pOwner->GetPosition().y;
			pShotInfo->fPosZ = pOwner->GetPosition().z;
			pShotInfo->fRatio = fRatio;
			pShotInfo->nDamageType = dt;
			pShotInfo->nWeaponType = pDesc->m_nWeaponType.Ref();
		}
	}

	if (!m_Match.IsTeamPlay() || (pTargetCharacter->GetTeamID() != pObject->GetTeamID()))
	{
		bHitEnemy = true;
	}

	v1 = pos;
	v2 = pickinfo.info.vOut;

	waterSound = GetWorld()->GetWaters()->CheckSpearing(v1, v2, 250, 0.3, !waterSound);
	return pShotInfo;
}

bool ZGame::CanISeeAttacker(ZCharacter* pAtk, const rvector& vRequestPos)
{
	const rvector& vAtkPos = pAtk->GetPosition();

	long double x = pow(vAtkPos.x - vRequestPos.x, 2);
	long double y = pow(vAtkPos.y - vRequestPos.y, 2);
	long double z = pow(vAtkPos.z - vRequestPos.z, 2);

	long double Len = x + y + z;

#define MAX_VIEW_LENGTH 800000

	if (MAX_VIEW_LENGTH < Len)
	{
#ifdef _DEBUG
		static rvector rv(0.0f, 0.0f, 0.0f);

		long double l = pow(vRequestPos.x - rv.x, 2) + pow(vRequestPos.y - rv.y, 2) + pow(vRequestPos.z - rv.z, 2);

		rv = vRequestPos;

		mlog("len : %f(%f), res(%d)\n", Len, sqrt(Len), MAX_VIEW_LENGTH < Len);
#endif
		return false;
	}

	return true;
}

void ZGame::OnPeerShot(const MUID& uid, float fShotTime, const rvector& pos, const rvector& to, const MMatchCharItemParts sel_type)
{
	ZCharacter* pOwnerCharacter = NULL;

	pOwnerCharacter = m_CharacterManager.Find(uid);

	if (pOwnerCharacter == NULL) return;
	if (!pOwnerCharacter->IsVisible()) return;

#ifdef LOCALE_NHNUSAA
	if (!CanISeeAttacker(pOwnerCharacter, pos)) return;
#endif

	pOwnerCharacter->OnShot();

	fShotTime -= pOwnerCharacter->m_fTimeOffset;

	ZItem* pItem = pOwnerCharacter->GetItems()->GetItem(sel_type);
	if (!pItem || !pItem->GetDesc()) return;

	if (pOwnerCharacter->CheckValidShotTime(pItem->GetDescID(), fShotTime, pItem))
	{
		pOwnerCharacter->UpdateValidShotTime(pItem->GetDescID(), fShotTime);
	}
	else
	{
		return;
	}

	if (uid == ZGetMyUID()) {
		int nCurrMagazine = pItem->GetBulletCurrMagazine();

		if (!pItem->Shot()) return;

		if (!(pItem->GetBulletCurrMagazine() < nCurrMagazine))
			if (sel_type != MMCIP_MELEE)
				ZGetApplication()->Exit();
	}
	else {
		if (!pItem->Shot()) {
			if (!ZGetGame()->IsReplay())
				return;
		}
	}

	if (sel_type == MMCIP_MELEE)
	{
		OnPeerShot_Melee(uid, fShotTime);

		return;
	}

	if ((sel_type != MMCIP_PRIMARY) && (sel_type != MMCIP_SECONDARY) && (sel_type != MMCIP_CUSTOM1)) return;

	if (!pItem->GetDesc()) return;
	MMatchWeaponType wtype = pItem->GetDesc()->m_nWeaponType.Ref();

	if (wtype == MWT_SHOTGUN)
	{
		OnPeerShot_Shotgun(pItem, pOwnerCharacter, fShotTime, pos, to);
		return;
	}
	else {
		OnPeerShot_Range(sel_type, uid, fShotTime, pos, to);

		rvector position;
		pOwnerCharacter->GetWeaponTypePos(weapon_dummy_muzzle_flash, &position);
		if (ZGetConfiguration()->GetVideo()->bDynamicLight)
		{
			RGetDynamicLightManager()->AddLight(GUNFIRE, position);
		}
	}
}

void ZGame::OnPeerDie(MUID& uidVictim, MUID& uidAttacker)
{
	ZCharacter* pVictim = m_CharacterManager.Find(uidVictim);
	if (pVictim == NULL) return;

	pVictim->ActDead();

	if (pVictim == m_pMyCharacter)
	{
		pVictim->Die();

		if (m_Match.IsWaitForRoundEnd())
		{
			if (m_CharacterManager.GetCount() > 2)
			{
				if (GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL)
					ReserveObserver();
			}
		}
#ifdef _QUEST
		else if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
		{
			if (m_CharacterManager.GetCount() >= 2)
			{
				ReserveObserver();
			}
		}
#endif

		CancelSuicide();
	}

	ZCharacter* pAttacker = m_CharacterManager.Find(uidAttacker);
	if (pAttacker == NULL) return;
	if (pAttacker != pVictim)
	{
		if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
		{
			if (pAttacker->GetKils() + 1 == 5)
			{
				MEMBER_SET_CHECKCRC(pAttacker->GetStatus(), nFantastic, pAttacker->GetStatus().Ref().nFantastic + 1);
				pAttacker->AddIcon(ZCI_FANTASTIC);
			}
			else if (pAttacker->GetKils() + 1 == 15)
			{
				MEMBER_SET_CHECKCRC(pAttacker->GetStatus(), nExcellent, pAttacker->GetStatus().Ref().nExcellent + 1);
				pAttacker->AddIcon(ZCI_EXCELLENT);
			}
			else if (pAttacker->GetKils() + 1 == 30)
			{
				MEMBER_SET_CHECKCRC(pAttacker->GetStatus(), nUnbelievable, pAttacker->GetStatus().Ref().nUnbelievable + 1);
				pAttacker->AddIcon(ZCI_UNBELIEVABLE);
			}
		}
		else
		{
			if (pAttacker->GetKils() >= 3)
			{
				MEMBER_SET_CHECKCRC(pAttacker->GetStatus(), nFantastic, pAttacker->GetStatus().Ref().nFantastic + 1);
				pAttacker->AddIcon(ZCI_FANTASTIC);
			}
		}

		if (pVictim->GetLastDamageType() == ZD_BULLET_HEADSHOT)
		{
			MEMBER_SET_CHECKCRC(pAttacker->GetStatus(), nHeadShot, pAttacker->GetStatus().Ref().nHeadShot + 1);
			pAttacker->AddIcon(ZCI_HEADSHOT);
		}
	}
}

void ZGame::OnPeerDead(const MUID& uidAttacker, const unsigned long int nAttackerArg,
	const MUID& uidVictim, const unsigned long int nVictimArg)
{
	ZCharacter* pVictim = m_CharacterManager.Find(uidVictim);
	ZCharacter* pAttacker = m_CharacterManager.Find(uidAttacker);

	bool bSuicide = false;
	if (uidAttacker == uidVictim) bSuicide = true;

	int nAttackerExp = 0, nVictimExp = 0;

	nAttackerExp = GetExpFromTransData(nAttackerArg);
	nVictimExp = -GetExpFromTransData(nVictimArg);

	if (pAttacker)
	{
		pAttacker->GetStatus().CheckCrc();

		pAttacker->GetStatus().Ref().AddExp(nAttackerExp);
		if (!bSuicide)
			pAttacker->GetStatus().Ref().AddKills();

		pAttacker->GetStatus().MakeCrc();
	}

	if (pVictim)
	{
		if (pVictim != m_pMyCharacter)
		{
			pVictim->Die();
		}

		pVictim->GetStatus().CheckCrc();

		pVictim->GetStatus().Ref().AddExp(nVictimExp);
		pVictim->GetStatus().Ref().AddDeaths();
		if (pVictim->GetStatus().Ref().nLife > 0)
			pVictim->GetStatus().Ref().nLife--;

		pVictim->GetStatus().MakeCrc();
	}

	if (bSuicide && (ZGetCharacterManager()->Find(uidAttacker) == ZGetGame()->m_pMyCharacter))
	{
		ZGetScreenEffectManager()->AddExpEffect(nVictimExp);
		int nExpPercent = GetExpPercentFromTransData(nVictimArg);
		ZGetMyInfo()->SetLevelPercent(nExpPercent);

		ZGetScreenEffectManager()->SetGaugeExpFromMyInfo();
	}
	else if (ZGetCharacterManager()->Find(uidAttacker) == m_pMyCharacter)
	{
		ZGetScreenEffectManager()->AddExpEffect(nAttackerExp);

		int nExpPercent = GetExpPercentFromTransData(nAttackerArg);
		ZGetMyInfo()->SetLevelPercent(nExpPercent);
		ZGetScreenEffectManager()->SetGaugeExpFromMyInfo();
	}
	else if (ZGetCharacterManager()->Find(uidVictim) == m_pMyCharacter)
	{
		ZGetScreenEffectManager()->AddExpEffect(nVictimExp);

		int nExpPercent = GetExpPercentFromTransData(nVictimArg);
		ZGetMyInfo()->SetLevelPercent(nExpPercent);
		ZGetScreenEffectManager()->SetGaugeExpFromMyInfo();
	}

	m_Match.AddRoundKills();

	CheckKillSound(pAttacker);
	OnPeerDieMessage(pVictim, pAttacker);
}

void ZGame::CheckKillSound(ZCharacter* pAttacker)
{
	if ((!pAttacker) || (pAttacker != m_pMyCharacter)) return;

	if (m_Match.GetRoundKills() == 1)
	{
	}
}

void ZGame::OnReceiveTeamBonus(const MUID& uidChar, const unsigned long int nExpArg)
{
	ZCharacter* pCharacter = m_CharacterManager.Find(uidChar);
	if (pCharacter == NULL) return;

	int nExp = 0;

	nExp = GetExpFromTransData(nExpArg);

	if (pCharacter)
	{
		pCharacter->GetStatus().CheckCrc();
		pCharacter->GetStatus().Ref().AddExp(nExp);
		pCharacter->GetStatus().MakeCrc();
	}

	if (pCharacter == m_pMyCharacter)
	{
#ifdef _DEBUG
		char szTemp[128];
		sprintf(szTemp, "TeamBonus = %d\n", nExp);
		OutputDebugString(szTemp);
#endif

		ZGetScreenEffectManager()->AddExpEffect(nExp);

		int nExpPercent = GetExpPercentFromTransData(nExpArg);
		ZGetMyInfo()->SetLevelPercent(nExpPercent);
		ZGetScreenEffectManager()->SetGaugeExpFromMyInfo();
	}
}

void ZGame::OnPeerDieMessage(ZCharacter* pVictim, ZCharacter* pAttacker)
{
	const char* testdeathnametable[ZD_END + 1] = { "에러", "총", "칼", "추락", "폭발", "HEADSHOT", "마지막칼질" };
	char szMsg[256] = "";

	const char* szAnonymous = "?아무개?";

	char szVictim[256];
	strcpy(szVictim, pVictim ? pVictim->GetUserAndClanName() : szAnonymous);

	char szAttacker[256];
	strcpy(szAttacker, pAttacker ? pAttacker->GetUserAndClanName() : szAnonymous);

	if (pAttacker == pVictim)
	{
		if (pVictim == m_pMyCharacter)
		{
			if (m_pMyCharacter->GetLastDamageType() == ZD_EXPLOSION) {
				sprintf(szMsg, ZMsg(MSG_GAME_LOSE_BY_MY_BOMB));
			}
			else {
				sprintf(szMsg, ZMsg(MSG_GAME_LOSE_MYSELF));
			}

			ZChatOutput(MCOLOR(0xFFCF2020), szMsg);
		}
		else
		{
			ZTransMsg(szMsg, MSG_GAME_WHO_LOSE_SELF, 1, szAttacker);
			ZChatOutput(MCOLOR(0xFF707070), szMsg);

			if (ZGetMyInfo()->IsAdminGrade()) {
				MMatchObjCache* pCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
				if (pCache && pCache->CheckFlag(MTD_PlayerFlags_AdminHide))
				{
					sprintf(szMsg, "^%d%s^9 스스로 패배",
						(pAttacker->GetTeamID() == MMT_BLUE) ? 3 : 1,
						pAttacker->GetProperty()->GetName());
					ZGetGameInterface()->GetCombatInterface()->m_AdminMsg.OutputChatMsg(szMsg);
				}
			}
		}
	}

	else if (pAttacker == m_pMyCharacter)
	{
		ZTransMsg(szMsg, MSG_GAME_WIN_FROM_WHO, 1, szVictim);
		ZChatOutput(MCOLOR(0xFF80FFFF), szMsg);
	}

	else if (pVictim == m_pMyCharacter)
	{
		ZTransMsg(szMsg, MSG_GAME_LOSE_FROM_WHO, 1, szAttacker);
		ZChatOutput(MCOLOR(0xFFCF2020), szMsg);
	}

	else
	{
		ZTransMsg(szMsg, MSG_GAME_WHO_WIN_FROM_OTHER, 2, szAttacker, szVictim);
		ZChatOutput(MCOLOR(0xFF707070), szMsg);

		if (ZGetMyInfo()->IsAdminGrade()) {
			MMatchObjCache* pCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
			if (pCache && pCache->CheckFlag(MTD_PlayerFlags_AdminHide))
			{
				sprintf(szMsg, "^%d%s^9 승리,  ^%d%s^9 패배",
					(pAttacker->GetTeamID() == MMT_BLUE) ? 3 : 1, pAttacker->GetProperty()->GetName(),
					(pVictim->GetTeamID() == MMT_BLUE) ? 3 : 1, pVictim->GetProperty()->GetName());
				ZGetGameInterface()->GetCombatInterface()->m_AdminMsg.OutputChatMsg(szMsg);
			}
		}
	}
}

void ZGame::OnReloadComplete(ZCharacter* pCharacter)
{
	ZItem* pItem = pCharacter->GetItems()->GetSelectedWeapon();

	pCharacter->GetItems()->Reload();

	if (pCharacter == m_pMyCharacter)
		ZApplication::GetSoundEngine()->PlaySound("we_weapon_rdy");

	return;
}

void ZGame::OnPeerSpMotion(MUID& uid, int nMotionType)
{
	ZCharacter* pCharacter = m_CharacterManager.Find(uid);

	if (pCharacter == NULL) return;

	pCharacter->m_dwStatusBitPackingValue.Ref().m_bSpMotion = true;

	ZC_STATE_LOWER zsl = ZC_STATE_TAUNT;

	if (nMotionType == ZC_SPMOTION_TAUNT)
	{
		zsl = ZC_STATE_TAUNT;

		char szSoundName[50];
		if (pCharacter->GetProperty()->nSex == MMS_MALE)
			sprintf(szSoundName, "fx2/MAL1%d", (RandomNumber(0, 300) % 3) + 1);
		else
			sprintf(szSoundName, "fx2/FEM1%d", (RandomNumber(0, 300) % 3) + 1);

		ZGetSoundEngine()->PlaySound(szSoundName, pCharacter->GetPosition());
	}
	else if (nMotionType == ZC_SPMOTION_BOW)
		zsl = ZC_STATE_BOW;
	else if (nMotionType == ZC_SPMOTION_WAVE)
		zsl = ZC_STATE_WAVE;
	else if (nMotionType == ZC_SPMOTION_LAUGH)
	{
		zsl = ZC_STATE_LAUGH;

		if (pCharacter->GetProperty()->nSex == MMS_MALE)
			ZGetSoundEngine()->PlaySound("fx2/MAL01", pCharacter->GetPosition());
		else
			ZGetSoundEngine()->PlaySound("fx2/FEM01", pCharacter->GetPosition());
	}
	else if (nMotionType == ZC_SPMOTION_CRY)
	{
		zsl = ZC_STATE_CRY;

		if (pCharacter->GetProperty()->nSex == MMS_MALE)
			ZGetSoundEngine()->PlaySound("fx2/MAL02", pCharacter->GetPosition());
		else
			ZGetSoundEngine()->PlaySound("fx2/FEM02", pCharacter->GetPosition());
	}
	else if (nMotionType == ZC_SPMOTION_DANCE)
		zsl = ZC_STATE_DANCE;

	pCharacter->m_SpMotion = zsl;

	pCharacter->SetAnimationLower(zsl);
}

void ZGame::OnPeerReload(MUID& uid)
{
	ZCharacter* pCharacter = m_CharacterManager.Find(uid);
	if (pCharacter == NULL || pCharacter->IsDie()) return;

	if (pCharacter == m_pMyCharacter)
		m_pMyCharacter->Animation_Reload();
	else
		OnReloadComplete(pCharacter);

	if (pCharacter->GetItems()->GetSelectedWeapon() != NULL) {
		rvector p = pCharacter->GetPosition() + rvector(0, 0, 160.f);
		ZApplication::GetSoundEngine()->PlaySEReload(pCharacter->GetItems()->GetSelectedWeapon()->GetDesc(), p.x, p.y, p.z, (pCharacter == m_pMyCharacter));
	}
}

void ZGame::OnPeerChangeCharacter(MUID& uid)
{
	ZCharacter* pCharacter = m_CharacterManager.Find(uid);

	if (pCharacter == NULL) return;

	pCharacter->TestToggleCharacter();
}

void ZGame::OnSetObserver(MUID& uid)
{
	ZCharacter* pCharacter = m_CharacterManager.Find(uid);
	if (pCharacter == NULL) return;

	if (pCharacter == m_pMyCharacter)
	{
		ZGetCombatInterface()->SetObserverMode(true);
	}
	pCharacter->SetVisible(false);
	pCharacter->ForceDie();
}

void ZGame::OnPeerSpawn(MUID& uid, rvector& pos, rvector& dir)
{
	m_nSpawnTime = timeGetTime();
	SetSpawnRequested(false);

	ZCharacter* pCharacter = m_CharacterManager.Find(uid);
	if (pCharacter == NULL) return;

	bool isRespawn = (pCharacter->IsDie() == true) ? true : false;

	pCharacter->SetVisible(true);
	pCharacter->Revival();
	pCharacter->SetPosition(pos);
	pCharacter->SetDirection(dir);
	pCharacter->SetSpawnTime(GetTime());

	ZGetEffectManager()->AddReBirthEffect(pos);

	if (pCharacter == m_pMyCharacter)
	{
		m_pMyCharacter->InitSpawn();

		if (isRespawn) {
			ZGetSoundEngine()->PlaySound("fx_respawn");
		}
		else {
			ZGetSoundEngine()->PlaySound("fx_whoosh02");
		}

		ZGetScreenEffectManager()->ReSetHpPanel();
	}
	pCharacter->GetStatus().CheckCrc();
	pCharacter->GetStatus().Ref().nDamageCaused = 0;
	pCharacter->GetStatus().MakeCrc();

	if (ZGetGameTypeManager()->IsTeamExtremeGame(GetMatch()->GetMatchType()))
		pCharacter->SetInvincibleTime(5000);
	else if (!ZGetGameTypeManager()->IsTeamGame(GetMatch()->GetMatchType()) && GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUELTOURNAMENT && GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL)
		pCharacter->SetInvincibleTime(1500);
}

void ZGame::OnPeerDash(MCommand* pCommand)
{
	MCommandParameter* pParam = pCommand->GetParameter(0);
	if (pParam->GetType() != MPT_BLOB) return;

	MUID uid = pCommand->GetSenderUID();
	ZPACKEDDASHINFO* ppdi = (ZPACKEDDASHINFO*)pParam->GetPointer();

	rvector pos, dir;
	int sel_type;

	pos = rvector(Roundf(ppdi->posx), Roundf(ppdi->posy), Roundf(ppdi->posz));
	dir = 1.f / 32000.f * rvector(ppdi->dirx, ppdi->diry, ppdi->dirz);
	sel_type = (int)ppdi->seltype;

	ZCharacter* pCharacter = m_CharacterManager.Find(uid);

	if (pCharacter == NULL) return;

	MMatchCharItemParts parts = (MMatchCharItemParts)sel_type;

	if (parts != pCharacter->GetItems()->GetSelectedWeaponParts()) {
		OnChangeWeapon(uid, parts);
	}

	ZGetEffectManager()->AddDashEffect(pos, dir, pCharacter);
}

rvector ZGame::GetFloor(rvector pos, rplane* pimpactplane, MUID myUID)
{
	rvector floor = ZGetGame()->GetWorld()->GetBsp()->GetFloor(pos + rvector(0, 0, 120), CHARACTER_RADIUS - 1.1f, 58.f, pimpactplane);

#ifdef ENABLE_CHARACTER_COLLISION
	ZObjectManager::iterator itor = m_ObjectManager.begin();
	for (; itor != m_ObjectManager.end(); ++itor)
	{
		ZObject* pObject = (*itor).second;
		if (pObject->IsCollideable())
		{
			rvector diff = pObject->GetPosition() - pos;
			diff.z = 0;

			if (Magnitude(diff) < CHARACTER_RADIUS && pos.z > pObject->GetPosition().z)
			{
				rvector newfloor = pObject->GetPosition() + rvector(0, 0, pObject->GetCollHeight());
				if (floor.z < newfloor.z)
				{
					if (m_pMyCharacter->GetUID() == myUID)
					{
						if (CharacterOverlapCollision(pObject, floor.z, newfloor.z) == false)
							continue;
					}

					floor = newfloor;
					if (pimpactplane)
					{
						rvector up = rvector(0, 0, 1);
						D3DXPlaneFromPointNormal(pimpactplane, &floor, &up);
					}
				}
			}
		}
	}
#endif

	return floor;
}

bool ZGame::CharacterOverlapCollision(ZObject* pFloorObject, float WorldFloorHeight, float ObjectFloorHeight)
{
	OVERLAP_FLOOR* pOverlapObject = m_pMyCharacter->GetOverlapFloor();

	if (pOverlapObject->FloorUID != pFloorObject->GetUID())
	{
		pOverlapObject->FloorUID = pFloorObject->GetUID();
		pOverlapObject->vecPosition.z = ObjectFloorHeight;
		pOverlapObject->nFloorCnt = 0;
		pOverlapObject->bJumpActivity = false;
	}
	else
	{
		if (pOverlapObject->bJumpActivity)
		{
			if (m_pMyCharacter->GetPosition().z - WorldFloorHeight > 20.f)
			{
				pOverlapObject->FloorUID = MUID(0, 0);
				pOverlapObject->nFloorCnt = 0;
				pOverlapObject->vecPosition.x = 0;
				pOverlapObject->vecPosition.y = 0;
				pOverlapObject->vecPosition.z = 0;
				pOverlapObject->bJumpActivity = false;
			}
			return false;
		}

		if (ObjectFloorHeight - pOverlapObject->vecPosition.z > 150.f)
		{
			pOverlapObject->vecPosition.z = ObjectFloorHeight;
			pOverlapObject->nFloorCnt++;
			if (pOverlapObject->nFloorCnt >= 3)
			{
				pOverlapObject->bJumpActivity = true;
				mlog("Jump bug Activity \n");
				return false;
			}
		}
	}

	return true;
}

bool ZGame::Pick(ZObject* pOwnerObject, rvector& origin, rvector& dir, ZPICKINFO* pickinfo, DWORD dwPassFlag, bool bMyChar)
{
	return PickHistory(pOwnerObject, GetTime(), origin, origin + 10000.f * dir, pickinfo, dwPassFlag, bMyChar);
}

bool ZGame::PickTo(ZObject* pOwnerObject, rvector& origin, rvector& to, ZPICKINFO* pickinfo, DWORD dwPassFlag, bool bMyChar)
{
	return PickHistory(pOwnerObject, GetTime(), origin, to, pickinfo, dwPassFlag, bMyChar);
}

bool ZGame::PickHistory(ZObject* pOwnerObject, float fTime, const rvector& origin, const rvector& to, ZPICKINFO* pickinfo, DWORD dwPassFlag, bool bMyChar)
{
	pickinfo->pObject = NULL;
	pickinfo->bBspPicked = false;
	pickinfo->nBspPicked_DebugRegister = -10;

	RPickInfo info;
	memset(&info, 0, sizeof(RPickInfo));

	ZObject* pObject = NULL;

	bool bCheck = false;

	float fCharacterDist = FLT_MAX;
	for (ZObjectManager::iterator i = m_ObjectManager.begin(); i != m_ObjectManager.end(); i++)
	{
		ZObject* pc = i->second;

		bCheck = false;

		if (bMyChar) {
			if (pc == pOwnerObject && pc->IsVisible()) {
				bCheck = true;
			}
		}
		else {
			if (pc != pOwnerObject && pc->IsVisible()) {
				bCheck = true;
			}
		}

		if (pc->IsDie())
			bCheck = false;

		if (bCheck)
		{
			rvector hitPos;
			ZOBJECTHITTEST ht = pc->HitTest(origin, to, fTime, &hitPos);
			if (ht != ZOH_NONE) {
				float fDistToChar = Magnitude(hitPos - origin);
				if (fDistToChar < fCharacterDist) {
					pObject = pc;
					fCharacterDist = fDistToChar;
					info.vOut = hitPos;
					switch (ht) {
					case ZOH_HEAD: info.parts = eq_parts_head; break;
					case ZOH_BODY: info.parts = eq_parts_chest; break;
					case ZOH_LEGS:	info.parts = eq_parts_legs; break;
					}
				}
			}
		}
	}

	RBSPPICKINFO bpi;
	bool bBspPicked = GetWorld()->GetBsp()->PickTo(origin, to, &bpi, dwPassFlag);

	int nCase = 0;

	if (pObject && bBspPicked)
	{
		if (Magnitude(info.vOut - origin) > Magnitude(bpi.PickPos - origin))
			nCase = 1;
		else
			nCase = 2;
	}
	else
		if (bBspPicked)
			nCase = 1;
		else
			if (pObject)
				nCase = 2;

	if (nCase == 0) return false;

	switch (nCase)
	{
	case 1:
		pickinfo->bBspPicked = true;
		pickinfo->nBspPicked_DebugRegister = FOR_DEBUG_REGISTER;
		pickinfo->bpi = bpi;
		break;
	case 2:
		pickinfo->pObject = pObject;
		pickinfo->info = info;
		break;
	}
	return true;
}

bool ZGame::ObjectColTest(ZObject* pOwner, rvector& origin, rvector& to, float fRadius, ZObject** poutTarget)
{
	for (ZObjectManager::iterator i = m_ObjectManager.begin(); i != m_ObjectManager.end(); i++)
	{
		ZObject* pc = i->second;

		if (pc == pOwner)
			continue;

		if (!pc->IsVisible())
			continue;

		if (pc->IsDie())
			continue;

		if (pc->ColTest(origin, to, fRadius, GetTime()))
		{
			*poutTarget = pc;
			return true;
		}
	}

	return false;
}

char* ZGame::GetSndNameFromBsp(const char* szSrcSndName, RMATERIAL* pMaterial)
{
	char szMaterial[256] = "";
	static char szRealSndName[256] = "";
	szRealSndName[0] = 0;

	if (pMaterial == NULL) return "";

	strcpy(szMaterial, pMaterial->Name.c_str());

	size_t nLen = strlen(szMaterial);

#define ZMETERIAL_SNDNAME_LEN 7

	if ((nLen > ZMETERIAL_SNDNAME_LEN) &&
		(!strnicmp(&szMaterial[nLen - ZMETERIAL_SNDNAME_LEN + 1], "mt", 2)))
	{
		strcpy(szRealSndName, szSrcSndName);
		strcat(szRealSndName, "_");
		strcat(szRealSndName, &szMaterial[nLen - ZMETERIAL_SNDNAME_LEN + 1]);
	}
	else
	{
		strcpy(szRealSndName, szSrcSndName);
	}

	return szRealSndName;
}

void ZGame::AutoAiming()
{
#ifdef _PUBLISH
	return;
#endif
}

#define MAX_PLAYERS		64

void ZGame::PostHPAPInfo()
{
	DWORD nNowTime = GetTickTime();

	if (m_pMyCharacter->GetInitialized() == false) return;

	if ((nNowTime - m_nLastTime[ZLASTTIME_HPINFO]) >= PEER_HP_TICK)
	{
		m_nLastTime[ZLASTTIME_HPINFO] = nNowTime;

		ZPostHPAPInfo(m_pMyCharacter->GetHP(), m_pMyCharacter->GetAP());
	}

#ifdef ENABLE_ADJUST_MY_DATA
#endif
}

void ZGame::PostDuelTournamentHPAPInfo()
{
	DWORD nNowTime = GetTickTime();

	if (m_pMyCharacter->GetInitialized() == false) return;

	if ((nNowTime - m_nLastTime[ZLASTTIME_HPINFO]) >= PEER_DUELTOURNAMENT_HPAP_TICK)
	{
		m_nLastTime[ZLASTTIME_HPINFO] = nNowTime;

		BYTE MaxHP = (BYTE)m_pMyCharacter->GetMaxHP();
		BYTE MaxAP = (BYTE)m_pMyCharacter->GetMaxAP();
		BYTE HP = (BYTE)m_pMyCharacter->GetHP();
		BYTE AP = (BYTE)m_pMyCharacter->GetAP();

		ZPostDuelTournamentHPAPInfo(MaxHP, MaxAP, HP, AP);
	}
}

void ZGame::PostBasicInfo()
{
	if (!ZGetGameInterface()->GetCombatInterface()->IsNetworkalive())
		return;

	DWORD nNowTime = timeGetTime();

	if (m_pMyCharacter->GetInitialized() == false) return;

	if (m_pMyCharacter->IsDie() && GetTime() - m_pMyCharacter->m_timeInfo.Ref().m_fDeadTime > 5.f) return;

	int nMoveTick = (ZGetGameClient()->GetAllowTunneling() == false) ? PEERMOVE_TICK : PEERMOVE_AGENT_TICK;

	if ((int)(nNowTime - m_nLastTime[ZLASTTIME_BASICINFO]) >= nMoveTick)
	{
		m_nLastTime[ZLASTTIME_BASICINFO] = nNowTime;

		ZPACKEDBASICINFO pbi;
		pbi.fTime = GetTime();

		pbi.posx = m_pMyCharacter->GetPosition().x;
		pbi.posy = m_pMyCharacter->GetPosition().y;
		pbi.posz = m_pMyCharacter->GetPosition().z;
		pbi.velx = m_pMyCharacter->GetVelocity().x;
		pbi.vely = m_pMyCharacter->GetVelocity().y;
		pbi.velz = m_pMyCharacter->GetVelocity().z;

		pbi.dirx = m_pMyCharacter->m_TargetDir.x * 32000;
		pbi.diry = m_pMyCharacter->m_TargetDir.y * 32000;
		pbi.dirz = m_pMyCharacter->m_TargetDir.z * 32000;

		pbi.upperstate = m_pMyCharacter->GetStateUpper();
		pbi.lowerstate = m_pMyCharacter->GetStateLower();
		pbi.selweapon = m_pMyCharacter->GetItems()->GetSelectedWeaponParts();

		ZPOSTCMD1(MC_PEER_BASICINFO, MCommandParameterBlob(&pbi, sizeof(ZPACKEDBASICINFO)));
	}
}

void ZGame::PostPeerPingInfo()
{
	if (!ZGetGameInterface()->GetCombatInterface()->IsShowScoreBoard()) return;

	DWORD nNowTime = GetTickTime();

	if ((nNowTime - m_nLastTime[ZLASTTIME_PEERPINGINFO]) >= PEER_PING_TICK) {
		m_nLastTime[ZLASTTIME_PEERPINGINFO] = nNowTime;

		unsigned long nTimeStamp = GetTickTime();
		MMatchPeerInfoList* pPeers = ZGetGameClient()->GetPeers();
		for (MMatchPeerInfoList::iterator itor = pPeers->begin(); itor != pPeers->end(); ++itor) {
			MMatchPeerInfo* pPeerInfo = (*itor).second;
			if (pPeerInfo->uidChar != ZGetGameClient()->GetPlayerUID()) {
				MCommandManager* MCmdMgr = ZGetGameClient()->GetCommandManager();
				MCommand* pCmd = new MCommand(MCmdMgr->GetCommandDescByID(MC_PEER_PING),
					pPeerInfo->uidChar, ZGetGameClient()->GetUID());
				pCmd->AddParameter(new MCmdParamUInt(nTimeStamp));
				ZGetGameClient()->Post(pCmd);

#ifdef _DEBUG
				g_nPingCount++;
#endif
				pPeerInfo->SetLastPingTime(nTimeStamp);
			}
		}
	}
}

void ZGame::PostSyncReport()
{
	DWORD nNowTime = GetTickTime();

#ifdef _PUBLISH
	if ((nNowTime - m_nLastTime[ZLASTTIME_SYNC_REPORT]) >= MATCH_CYCLE_CHECK_SPEEDHACK) {
#else
	if ((nNowTime - m_nLastTime[ZLASTTIME_SYNC_REPORT]) >= 1000) {
#endif
		m_nLastTime[ZLASTTIME_SYNC_REPORT] = nNowTime;
		int nDataChecksum = 0;
		if (m_DataChecker.UpdateChecksum() == false) {
			nDataChecksum = m_DataChecker.GetChecksum();
			ZGetApplication()->Exit();
		}
		ZPOSTCMD2(MC_MATCH_GAME_REPORT_TIMESYNC, MCmdParamUInt(nNowTime), MCmdParamUInt(nDataChecksum));
	}
}

void ZGame::CheckCombo(ZCharacter * pOwnerCharacter, ZObject * pHitObject, bool bPlaySound)
{
	if (pOwnerCharacter == pHitObject) return;

	ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if (!pTargetCharacter) return;

	if (pTargetCharacter != pOwnerCharacter) return;

	if (pHitObject)
	{
		if (pHitObject->IsDie()) return;
	}

	if (IsPlayerObject(pHitObject))
	{
		if (m_Match.IsTeamPlay() && (pTargetCharacter->GetTeamID() == ((ZCharacter*)(pHitObject))->GetTeamID()))
			return;

		if (m_Match.IsQuestDrived()) return;
	}

	UpdateCombo(true);

	if (Z_AUDIO_HITSOUND)
	{
#ifdef _BIRDSOUND
		ZGetSoundEngine()->PlaySound("fx_myhit", 128);
#else
		if (bPlaySound)
			if (ZGetSoundEngine()->Get3DSoundUpdate())
				ZGetSoundEngine()->PlaySound("fx_myhit");
#endif
	}
}

void ZGame::UpdateCombo(bool bShot)
{
	ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if (!pTargetCharacter) return;

	static DWORD nLastShotTime = timeGetTime();
	DWORD nNowTime = timeGetTime();

	pTargetCharacter->GetStatus().CheckCrc();

	if (bShot)
	{
		if (pTargetCharacter->GetStatus().Ref().nCombo < 2) {
			ZGetScreenEffectManager()->AddHit();
		}

		if ((nNowTime - nLastShotTime) < 700)
		{
			pTargetCharacter->GetStatus().Ref().nCombo++;
			if (pTargetCharacter->GetStatus().Ref().nCombo > MAX_COMBO)
				pTargetCharacter->GetStatus().Ref().nCombo = 1;
		}
		nLastShotTime = nNowTime;
	}
	else
	{
		if ((pTargetCharacter->GetStatus().Ref().nCombo > 0) && ((nNowTime - nLastShotTime) > 1000))
		{
			pTargetCharacter->GetStatus().Ref().nCombo = 0;
		}
	}

	pTargetCharacter->GetStatus().MakeCrc();
}

void ZGame::CheckStylishAction(ZCharacter * pCharacter)
{
	if (pCharacter->GetStylishShoted())
	{
		if (pCharacter == m_pMyCharacter)
		{
			ZGetScreenEffectManager()->AddCool();
		}
	}
}

#define RESERVED_OBSERVER_TIME	5000

void ZGame::OnReserveObserver()
{
	unsigned long int currentTime = timeGetTime();

	if (currentTime - m_nReservedObserverTime > RESERVED_OBSERVER_TIME)
	{
		if ((m_Match.GetRoundState() == MMATCH_ROUNDSTATE_PLAY) ||
			(m_Match.IsWaitForRoundEnd() && ZGetGameClient()->IsForcedEntry())
			)
		{
			ZGetGameInterface()->GetCombatInterface()->SetObserverMode(true);
			m_bReserveObserver = false;
		}
		else
		{
			m_bReserveObserver = false;
		}
	}
}

void ZGame::ReserveObserver()
{
	m_bReserveObserver = true;
	m_nReservedObserverTime = timeGetTime();
}

void ZGame::ReleaseObserver()
{
	if (!m_bReplaying.Ref())
	{
		m_bReserveObserver = false;
		ZGetGameInterface()->GetCombatInterface()->SetObserverMode(false);

		FlushObserverCommands();
	}
}

void ZGame::OnInvalidate()
{
	GetWorld()->OnInvalidate();
	ZGetFlashBangEffect()->OnInvalidate();
	m_CharacterManager.OnInvalidate();
}

void ZGame::OnRestore()
{
	GetWorld()->OnRestore();
	ZGetFlashBangEffect()->OnRestore();
	m_CharacterManager.OnRestore();
}

void ZGame::InitRound()
{
	SetSpawnRequested(false);
	ZGetGameInterface()->GetCamera()->StopShock();

	ZGetFlashBangEffect()->End();

	ZGetEffectManager()->Clear();
	m_WeaponManager.Clear();

#ifdef _WORLD_ITEM_
#endif

	ZGetCharacterManager()->InitRound();
}

void ZGame::AddEffectRoundState(MMATCH_ROUNDSTATE nRoundState, int nArg)
{
	switch (nRoundState)
	{
	case MMATCH_ROUNDSTATE_COUNTDOWN:
	{
		if (m_Match.IsWaitForRoundEnd() && m_Match.GetMatchType() != MMATCH_GAMETYPE_DUEL)
		{
			if (m_Match.GetCurrRound() + 1 == m_Match.GetRoundCount())
			{
				ZGetScreenEffectManager()->AddFinalRoundStart();
			}
			else
			{
				if (GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
				{
					ZRuleDuelTournament* pRule = (ZRuleDuelTournament*)m_Match.GetRule();
					int nRoundCount = pRule->GetDuelTournamentPlayCount();
					ZGetScreenEffectManager()->AddRoundStart(nRoundCount);
				}
				else
				{
					ZGetScreenEffectManager()->AddRoundStart(m_Match.GetCurrRound() + 1);
				}
			}
		}
	}
	break;
	case MMATCH_ROUNDSTATE_PLAY:
	{
		if (GetMatch()->GetMatchType() == MMATCH_GAMETYPE_CTF)
		{
			ZGetGameInterface()->PlayVoiceSound(VOICE_CTF, 1600);
			ZGetScreenEffectManager()->AddScreenEffect("ctf_splash");
		}
		else
			ZGetScreenEffectManager()->AddRock();
	}
	break;
	case MMATCH_ROUNDSTATE_FINISH:
	{
		if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DUEL && ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DUELTOURNAMENT && ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_QUEST && ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_SURVIVAL) {
			for (ZCharacterManager::iterator itor = m_CharacterManager.begin(); itor != m_CharacterManager.end(); ++itor)
			{
				ZCharacter* pCharacter = (ZCharacter*)(*itor).second;

				if (pCharacter->GetTeamID() == ZGetGame()->m_pMyCharacter->GetTeamID()) {
					char FinishStr[512];

					sprintf(FinishStr, "%s has dealt %d damage.", pCharacter->GetCharInfo()->szName, pCharacter->GetStatus().Ref().nDamageCaused);
					ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), FinishStr);
				}
			}
		}

		if (m_Match.IsTeamPlay())
		{
			int nRedTeam, nBlueTeam;
			m_Match.GetTeamAliveCount(&nRedTeam, &nBlueTeam);

			if (nArg == MMATCH_ROUNDRESULT_RED_ALL_OUT || nArg == MMATCH_ROUNDRESULT_BLUE_ALL_OUT)
			{
				ZGetScreenEffectManager()->AddWin();
			}
			else if (nArg == MMATCH_ROUNDRESULT_DRAW)
			{
				if (ZGetGameTypeManager()->IsTeamExtremeGame(GetMatch()->GetMatchType()))
				{
					MMatchTeam nMyTeam = (MMatchTeam)m_pMyCharacter->GetTeamID();
					MMatchTeam nEnemyTeam = (nMyTeam == MMT_BLUE ? MMT_RED : MMT_BLUE);

					int nMyScore = GetMatch()->GetTeamKills(nMyTeam);
					int nEnemyScore = GetMatch()->GetTeamKills(nEnemyTeam);

					if (nMyScore > nEnemyScore)
						ZGetScreenEffectManager()->AddWin();
					else if (nMyScore < nEnemyScore)
						ZGetScreenEffectManager()->AddLose();
					else
						ZGetScreenEffectManager()->AddDraw();
				}
				else
					ZGetScreenEffectManager()->AddDraw();
			}
			else
			{
				if (nArg == MMATCH_ROUNDRESULT_DRAW)
				{
					ZGetGameInterface()->PlayVoiceSound(VOICE_DRAW_GAME, 1200);
				}
				else {
					MMatchTeam nMyTeam = (MMatchTeam)m_pMyCharacter->GetTeamID();
					MMatchTeam nTeamWon = (nArg == MMATCH_ROUNDRESULT_REDWON ? MMT_RED : MMT_BLUE);

					if (ZGetMyInfo()->GetGameInfo()->bForcedChangeTeam)
					{
						nMyTeam = NegativeTeam(nMyTeam);
					}

					if (ZGetGameInterface()->GetCombatInterface()->GetObserver()->IsVisible()) {
						ZCharacter* pTarget = ZGetGameInterface()->GetCombatInterface()->GetObserver()->GetTargetCharacter();
						if (pTarget)
							nMyTeam = (MMatchTeam)pTarget->GetTeamID();
					}

					if (nTeamWon == nMyTeam)
						ZGetScreenEffectManager()->AddWin();
					else
						ZGetScreenEffectManager()->AddLose();

					if (GetMatch()->GetMatchType() == MMATCH_GAMETYPE_ASSASSINATE)
					{
						if (nTeamWon == MMT_RED)
							ZGetGameInterface()->PlayVoiceSound(VOICE_BLUETEAM_BOSS_DOWN, 2100);
						else
							ZGetGameInterface()->PlayVoiceSound(VOICE_REDTEAM_BOSS_DOWN, 2000);
					}
					else
					{
						if (nTeamWon == MMT_RED)
							ZGetGameInterface()->PlayVoiceSound(VOICE_RED_TEAM_WON, 1400);
						else
							ZGetGameInterface()->PlayVoiceSound(VOICE_BLUE_TEAM_WON, 1400);
					}
				}
			}

			int nTeam = 0;

			for (int j = 0; j < 2; j++)
			{
				bool bAllKill = true;
				ZCharacter* pAllKillPlayer = NULL;

				for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin();
					itor != ZGetCharacterManager()->end(); ++itor)
				{
					ZCharacter* pCharacter = (*itor).second;
					if (pCharacter == NULL) return;

					if (j == 0) {
						nTeam = MMT_RED;
					}
					else if (j == 1) {
						nTeam = MMT_BLUE;
					}

					if (pCharacter->GetTeamID() != nTeam)
						continue;

					if (pCharacter->IsDie())
					{
						ZCharacter* pKiller = ZGetCharacterManager()->Find(pCharacter->GetLastAttacker());
						if (pAllKillPlayer == NULL)
						{
							if (!pKiller || pKiller->GetTeamID() == nTeam)
							{
								bAllKill = false;
								break;
							}

							pAllKillPlayer = pKiller;
						}
						else
							if (pAllKillPlayer != pKiller)
							{
								bAllKill = false;
								break;
							}
					}
					else
					{
						bAllKill = false;
						break;
					}
				}

				if ((bAllKill) && (pAllKillPlayer))
				{
					MEMBER_SET_CHECKCRC(pAllKillPlayer->GetStatus(), nAllKill, pAllKillPlayer->GetStatus().Ref().nAllKill + 1);
					pAllKillPlayer->AddIcon(ZCI_ALLKILL);
				}
			}
		}

		else if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
		{
			ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
			if (pDuel)
			{
				bool bAddWin = false;
				bool bAddLose = false;
				int nCount = 0;

				MUID uidTarget;
				ZObserver* pObserver = ZGetGameInterface()->GetCombatInterface()->GetObserver();
				if (pObserver && pObserver->IsVisible())
					uidTarget = pObserver->GetTargetCharacter()->GetUID();

				else
					uidTarget = m_pMyCharacter->GetUID();

				for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin(); itor != ZGetCharacterManager()->end(); ++itor)
				{
					ZCharacter* pCharacter = (*itor).second;

					if ((pCharacter->GetUID() == pDuel->QInfo.m_uidChampion) || (pCharacter->GetUID() == pDuel->QInfo.m_uidChallenger))
					{
						if (uidTarget == pCharacter->GetUID())
						{
							if (pCharacter->IsDie())
								bAddLose |= true;
							else
								bAddWin |= true;
						}
						else
						{
							if (pCharacter->IsDie())
								bAddWin |= true;
							else
								bAddLose |= true;
						}

						nCount++;
					}
				}

				if ((nCount < 2) || (bAddWin == bAddLose))
				{
					ZGetScreenEffectManager()->AddDraw();
					ZGetGameInterface()->PlayVoiceSound(VOICE_DRAW_GAME, 1200);
				}

				else if (bAddWin)
				{
					ZGetScreenEffectManager()->AddWin();
					ZGetGameInterface()->PlayVoiceSound(VOICE_YOU_WON, 1000);
				}

				else
				{
					ZGetScreenEffectManager()->AddLose();
					ZGetGameInterface()->PlayVoiceSound(VOICE_YOU_LOSE, 1300);
				}
			}
		}
		else if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
		{
			if (!ZGetCombatInterface()->GetObserver()->IsVisible())
			{
				float fMaxHP = ZGetGame()->m_pMyCharacter->GetMaxHP();
				float fMaxAP = ZGetGame()->m_pMyCharacter->GetMaxAP();

				float fHP = ZGetGame()->m_pMyCharacter->GetHP();
				float fAP = ZGetGame()->m_pMyCharacter->GetAP();

				float fAccumulationDamage = ZGetGame()->m_pMyCharacter->GetAccumulationDamage();

				ZPostDuelTournamentGamePlayerStatus(ZGetGame()->m_pMyCharacter->GetUID(), fAccumulationDamage, fHP, fAP);

#ifndef _PUBLISH
				char szAccumulationDamagePrint[256];
				sprintf(szAccumulationDamagePrint, "누적대미지[%2.1f] 서버에 보냄", fAccumulationDamage);
				ZChatOutput(MCOLOR(255, 200, 200), szAccumulationDamagePrint);

#	ifdef _DUELTOURNAMENT_LOG_ENABLE_
				mlog(szAccumulationDamagePrint);
#	endif

#endif
				ZGetGame()->m_pMyCharacter->InitAccumulationDamage();
			}
		}
	}
	break;
	};
}

void ZGame::StartRecording()
{
	int nsscount = 0;

	char replayfilename[_MAX_PATH];
	char replayfilenameSafe[_MAX_PATH];
	char replayfoldername[_MAX_PATH];

	TCHAR szPath[MAX_PATH];
	if (GetMyDocumentsPath(szPath)) {
		strcpy(replayfoldername, szPath);
		strcat(replayfoldername, GUNZ_FOLDER);
		CreatePath(replayfoldername);
		strcat(replayfoldername, REPLAY_FOLDER);
		CreatePath(replayfoldername);
	}

	SYSTEMTIME t;
	GetLocalTime(&t);
	char szCharName[MATCHOBJECT_NAME_LENGTH];
	ValidateFilename(szCharName, ZGetMyInfo()->GetCharName(), '_');

	const char* szGameTypeAcronym = "";
	char szValidatedOppoClanName[32] = "";
	bool bClanGame = ZGetGameClient()->IsLadderGame();

	REPLAY_STAGE_SETTING_NODE stageSettingNode;

	if (GetMatch()) {
		if (bClanGame) szGameTypeAcronym = "CLAN_";
		else szGameTypeAcronym = MMatchGameTypeAcronym[GetMatch()->GetMatchType()];

		if (bClanGame) {
			const char* szOppositeClanName = "";

			if (0 == strcmp(ZGetMyInfo()->GetClanName(), ZGetCombatInterface()->GetRedClanName()))
				szOppositeClanName = ZGetCombatInterface()->GetBlueClanName();
			else
				szOppositeClanName = ZGetCombatInterface()->GetRedClanName();

			ValidateFilename(szValidatedOppoClanName, szOppositeClanName, '_');
		}
	}

	sprintf(replayfilename, "%s_%s_%4d%02d%02d_%02d%02d%02d%s%s",
		szGameTypeAcronym, szCharName, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond,
		bClanGame ? "_" : "", szValidatedOppoClanName);

	sprintf(replayfilenameSafe, "%s_nocharname_%4d%02d%02d_%02d%02d%02d",
		szGameTypeAcronym, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);

	char szFullPath[_MAX_PATH];

	strcpy(m_szReplayFileName, replayfilename);
	sprintf(szFullPath, "%s/%s." GUNZ_REC_FILE_EXT, replayfoldername, replayfilename);
	m_pReplayFile = zfopen(szFullPath, true);
	if (!m_pReplayFile)
	{
		strcpy(m_szReplayFileName, replayfilenameSafe);
		sprintf(szFullPath, "%s/%s." GUNZ_REC_FILE_EXT, replayfoldername, replayfilenameSafe);
		m_pReplayFile = zfopen(szFullPath, true);

		if (!m_pReplayFile) goto RECORDING_FAIL;
	}

	int nWritten;

	DWORD header;
	header = GUNZ_REC_FILE_ID;
	nWritten = zfwrite(&header, sizeof(header), 1, m_pReplayFile);
	if (nWritten == 0) goto RECORDING_FAIL;

	header = GUNZ_REC_FILE_VERSION;
	nWritten = zfwrite(&header, sizeof(header), 1, m_pReplayFile);
	if (nWritten == 0) goto RECORDING_FAIL;

	ConvertStageSettingNodeForRecord(ZGetGameClient()->GetMatchStageSetting()->GetStageSetting(), &stageSettingNode);

	nWritten = zfwrite(&stageSettingNode, sizeof(REPLAY_STAGE_SETTING_NODE), 1, m_pReplayFile);
	if (nWritten == 0) goto RECORDING_FAIL;

	if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUEL)
	{
		ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
		nWritten = zfwrite(&pDuel->QInfo, sizeof(MTD_DuelQueueInfo), 1, m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;
	}
	if (IsGameRuleCTF(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
	{
		ZRuleTeamCTF* pTeamCTF = (ZRuleTeamCTF*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
		nWritten = zfwrite(&pTeamCTF->GetRedCarrier(), sizeof(MUID), 1, m_pReplayFile);
		nWritten = zfwrite(&pTeamCTF->GetBlueCarrier(), sizeof(MUID), 1, m_pReplayFile);
		nWritten = zfwrite(&pTeamCTF->GetRedFlagPos(), sizeof(rvector), 1, m_pReplayFile);
		nWritten = zfwrite(&pTeamCTF->GetBlueFlagPos(), sizeof(rvector), 1, m_pReplayFile);
		int nRedFlagState = (int)pTeamCTF->GetRedFlagState();
		int nBlueFlagState = (int)pTeamCTF->GetBlueFlagState();
		nWritten = zfwrite(&nRedFlagState, sizeof(int), 1, m_pReplayFile);
		nWritten = zfwrite(&nBlueFlagState, sizeof(int), 1, m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;
	}
	else if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
	{
		int nType = (int)ZGetGameInterface()->GetDuelTournamentType();
		nWritten = zfwrite(&nType, sizeof(int), 1, m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;

		const vector<DTPlayerInfo>& vecDTPlayerInfo = ZGetGameInterface()->GetVectorDTPlayerInfo();

		int nCount = (int)vecDTPlayerInfo.size();
		nWritten = zfwrite(&nCount, sizeof(int), 1, m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;

		nWritten = zfwrite((void*)&vecDTPlayerInfo[0], sizeof(DTPlayerInfo), nCount, m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;

		ZRuleDuelTournament* pRule = (ZRuleDuelTournament*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
		nWritten = zfwrite((void*)&pRule->m_DTGameInfo, sizeof(MTD_DuelTournamentGameInfo), 1, m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;
	}

	int nCharacterCount = (int)m_CharacterManager.size();
	nWritten = zfwrite(&nCharacterCount, sizeof(nCharacterCount), 1, m_pReplayFile);
	if (nWritten == 0) goto RECORDING_FAIL;

	for (ZCharacterManager::iterator itor = m_CharacterManager.begin(); itor != m_CharacterManager.end(); ++itor)
	{
		ZCharacter* pCharacter = (*itor).second;
		if (!pCharacter->Save(m_pReplayFile)) goto RECORDING_FAIL;
	}

	float fTime = m_fTime.Ref();
	nWritten = zfwrite(&fTime, sizeof(float), 1, m_pReplayFile);
	if (nWritten == 0) goto RECORDING_FAIL;

	m_bRecording = true;
	ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM),
		ZMsg(MSG_RECORD_STARTING));
	return;

RECORDING_FAIL:

	if (m_pReplayFile)
	{
		zfclose(m_pReplayFile);
		m_pReplayFile = NULL;
	}

	ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), ZMsg(MSG_RECORD_CANT_SAVE));
}

void ZGame::StopRecording()
{
	if (!m_bRecording) return;

	bool bError = false;

	m_bRecording = false;

	ZObserverCommandList::iterator itr = m_ReplayCommandList.begin();
	for (size_t i = 0; i < m_ReplayCommandList.size(); i++)
	{
		ZObserverCommandItem* pItem = *itr;
		MCommand* pCommand = pItem->pCommand;

		const int BUF_SIZE = 1024;
		char CommandBuffer[BUF_SIZE];
		int nSize = pCommand->GetData(CommandBuffer, BUF_SIZE);

		int nWritten;
		nWritten = zfwrite(&pItem->fTime, sizeof(pItem->fTime), 1, m_pReplayFile);
		if (nWritten == 0) { bError = true; break; }
		nWritten = zfwrite(&pCommand->m_Sender, sizeof(pCommand->m_Sender), 1, m_pReplayFile);
		if (nWritten == 0) { bError = true; break; }
		nWritten = zfwrite(&nSize, sizeof(nSize), 1, m_pReplayFile);
		if (nWritten == 0) { bError = true; break; }
		nWritten = zfwrite(CommandBuffer, nSize, 1, m_pReplayFile);
		if (nWritten == 0) { bError = true; break; }

		itr++;
	}

	while (m_ReplayCommandList.size())
	{
		ZObserverCommandItem* pItem = *m_ReplayCommandList.begin();
		delete pItem->pCommand;
		delete pItem;
		m_ReplayCommandList.pop_front();
	}

	if (!zfclose(m_pReplayFile))
		bError = true;

	if (bError)
	{
		ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), ZMsg(MSG_RECORD_CANT_SAVE));
	}
	else
	{
		char szOutputFilename[256];
		sprintf(szOutputFilename, GUNZ_FOLDER REPLAY_FOLDER "/%s." GUNZ_REC_FILE_EXT, m_szReplayFileName);

		char szOutput[256];
		ZTransMsg(szOutput, MSG_RECORD_SAVED, 1, szOutputFilename);
		ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szOutput);
	}
}

void ZGame::ToggleRecording()
{
	if (m_bReplaying.Ref()) return;

	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
		return;

	if (!m_bRecording)
		StartRecording();
	else
		StopRecording();
}

DWORD dwReplayStartTime;

bool ZGame::OnLoadReplay(ZReplayLoader * pLoader)
{
	m_fTime.Set_CheckCrc(pLoader->GetGameTime());

	m_bReplaying.Set_CheckCrc(true);
	SetReadyState(ZGAME_READYSTATE_RUN);
	GetMatch()->SetRoundState(MMATCH_ROUNDSTATE_FREE);
	ZGetGameInterface()->GetCombatInterface()->SetObserverMode(true);
	ZGetGameInterface()->GetCombatInterface()->GetObserver()->SetTarget(ZGetGame()->m_pMyCharacter->GetUID());
	g_bProfile = true;
	dwReplayStartTime = timeGetTime();

	return true;
}

void ZGame::EndReplay()
{
	g_bProfile = false;

	DWORD dwReplayEndTime = timeGetTime();

#if FPSOLD
	mlog("replay end. profile saved. playtime = %3.3f seconds , average fps = %3.3f \n",
		float(dwReplayEndTime - dwReplayStartTime) / 1000.f,
		1000.f * g_nFrameCount / float(dwReplayEndTime - dwReplayStartTime));
#endif

	ZChangeGameState(GUNZ_LOBBY);
}

void ZGame::ConfigureCharacter(const MUID & uidChar, MMatchTeam nTeam, unsigned char nPlayerFlags)
{
	ZCharacterManager* pCharMgr = ZGetCharacterManager();
	ZCharacter* pChar = pCharMgr->Find(uidChar);
	if (pChar == NULL) return;

	pChar->SetAdminHide((nPlayerFlags & MTD_PlayerFlags_AdminHide) != 0);
	pChar->SetTeamID(nTeam);
	pChar->InitStatus();
	pChar->InitRound();

	ZGetCombatInterface()->OnAddCharacter(pChar);
}

void ZGame::RefreshCharacters()
{
	for (MMatchPeerInfoList::iterator itor = ZGetGameClient()->GetPeers()->begin();
		itor != ZGetGameClient()->GetPeers()->end(); ++itor)
	{
		MMatchPeerInfo* pPeerInfo = (*itor).second;
		ZCharacter* pCharacter = m_CharacterManager.Find(pPeerInfo->uidChar);

		if (pCharacter == NULL) {
			pCharacter = m_CharacterManager.Add(pPeerInfo->uidChar, rvector(0.0f, 0.0f, 0.0f));
			pCharacter->Create(&pPeerInfo->CharInfo);

			if (m_Match.GetRoundState() == MMATCH_ROUNDSTATE_PREPARE)
			{
				if (m_Match.IsTeamPlay())
				{
				}
			}
		}
	}
}

void ZGame::DeleteCharacter(const MUID & uid)
{
	bool bObserverDel = false;
	ZCharacter* pCharacter = ZGetCharacterManager()->Find(uid);

	ZObserver* pObserver = ZGetGameInterface()->GetCombatInterface()->GetObserver();
	if (pObserver->IsVisible())
	{
		if ((pCharacter != NULL) && (pCharacter == pObserver->GetTargetCharacter()))
		{
			bObserverDel = true;
		}
	}

	m_CharacterManager.Delete(uid);

	if (bObserverDel)
	{
		if (pObserver) pObserver->SetFirstTarget();
	}
}

void ZGame::OnStageEnterBattle(MCmdEnterBattleParam nParam, MTD_PeerListNode * pPeerNode)
{
	if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME) return;

	MUID uidChar = pPeerNode->uidChar;

	if (uidChar == ZGetMyUID())
	{
		if (ZGetGame()->CreateMyCharacter(&pPeerNode->CharInfo) == true)
		{
			ConfigureCharacter(uidChar, (MMatchTeam)pPeerNode->ExtendInfo.nTeam, pPeerNode->ExtendInfo.nPlayerFlags);
		}
	}
	else
	{
		OnAddPeer(pPeerNode->uidChar, pPeerNode->dwIP, pPeerNode->nPort, pPeerNode);
	}

	if (nParam == MCEP_FORCED)
	{
		ZCharacter* pChar = ZGetCharacterManager()->Find(uidChar);
		GetMatch()->OnForcedEntry(pChar);

		char temp[256] = "";
		if ((pPeerNode->ExtendInfo.nPlayerFlags & MTD_PlayerFlags_AdminHide) == 0) {
			ZTransMsg(temp, MSG_GAME_JOIN_BATTLE, 1, pChar->GetUserAndClanName());
			ZChatOutput(MCOLOR(ZCOLOR_GAME_INFO), temp);
		}
#ifdef _REPLAY_TEST_LOG
		mlog("[Add Character %s(%d)]\n", pChar->GetCharInfo()->szName, uidChar.Low);
#endif
	}

	ZGetGameClient()->OnStageEnterBattle(uidChar, nParam);
}

void ZGame::OnStageLeaveBattle(const MUID & uidChar, const bool bIsRelayMap)
{
	if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME) return;

	if (uidChar != ZGetMyUID()) {
		ZCharacter* pChar = ZGetCharacterManager()->Find(uidChar);

		if (pChar && !pChar->IsAdminHide() && !bIsRelayMap) {
			char temp[256] = "";
			ZTransMsg(temp, MSG_GAME_LEAVE_BATTLE, 1, pChar->GetUserAndClanName());
			ZChatOutput(MCOLOR(ZCOLOR_GAME_INFO), temp);
		}

		ZGetGameClient()->DeletePeer(uidChar);
		if (ZApplication::GetGameInterface()->GetState() == GUNZ_GAME) {
			DeleteCharacter(uidChar);
		}

		ZGetGameClient()->SetVoteInProgress(false);
		ZGetGameClient()->SetCanVote(false);
	}
}

void ZGame::OnAddPeer(const MUID & uidChar, DWORD dwIP, const int nPort, MTD_PeerListNode * pNode)
{
	if ((ZApplication::GetGameInterface()->GetState() != GUNZ_GAME) || (ZGetGame() == NULL)) return;

	if (uidChar != ZGetMyUID())
	{
		if (pNode == NULL) {
		}

		ZGetGameClient()->DeletePeer(uidChar);

		MMatchPeerInfo* pNewPeerInfo = new MMatchPeerInfo;

		if (uidChar == MUID(0, 0))	pNewPeerInfo->uidChar = MUID(0, nPort);
		else						pNewPeerInfo->uidChar = uidChar;

		in_addr addr;
		addr.s_addr = dwIP;
		char* pszIP = inet_ntoa(addr);
		strcpy(pNewPeerInfo->szIP, pszIP);

		pNewPeerInfo->dwIP = dwIP;
		pNewPeerInfo->nPort = nPort;

		if (!IsReplay())
			memcpy(&pNewPeerInfo->CharInfo, &(pNode->CharInfo), sizeof(MTD_CharInfo));
		else
		{
			MTD_CharInfo currInfo;
			ConvertCharInfo(&currInfo, &pNode->CharInfo, ZReplayLoader::m_nVersion);
			memcpy(&pNewPeerInfo->CharInfo, &currInfo, sizeof(MTD_CharInfo));
		}
		memcpy(&pNewPeerInfo->ExtendInfo, &(pNode->ExtendInfo), sizeof(MTD_ExtendInfo));

		ZGetGameClient()->AddPeer(pNewPeerInfo);

		RefreshCharacters();
	}

	ConfigureCharacter(uidChar, (MMatchTeam)pNode->ExtendInfo.nTeam, pNode->ExtendInfo.nPlayerFlags);
}

void ZGame::OnPeerList(const MUID & uidStage, void* pBlob, int nCount)
{
	if (ZGetGameClient()->GetStageUID() != uidStage) return;
	if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME) return;
	if ((ZGetGame() == NULL) || (ZGetCharacterManager() == NULL)) return;

	for (int i = 0; i < nCount; i++) {
		MTD_PeerListNode* pNode = (MTD_PeerListNode*)MGetBlobArrayElement(pBlob, i);
		OnAddPeer(pNode->uidChar, pNode->dwIP, pNode->nPort, pNode);

		ZCharacter* pChar = ZGetCharacterManager()->Find(pNode->uidChar);
		if (pChar) {
			pChar->SetVisible(false);
		}
	}
}

void ZGame::PostMyBuffInfo()
{
	if (m_pMyCharacter)
	{
		void* pBlob = m_pMyCharacter->MakeBuffEffectBlob();
		if (pBlob)
		{
			ZPostBuffInfo(pBlob);
			MEraseBlobArray(pBlob);
		}
	}
}

void ZGame::OnPeerBuffInfo(const MUID & uidSender, void* pBlobBuffInfo)
{
	if (uidSender == ZGetMyUID()) return;

	ZCharacter* pSender = ZGetCharacterManager()->Find(uidSender);
	if (!pSender) return;
	if (!pBlobBuffInfo) return;

	MTD_BuffInfo* pBuffInfo = NULL;
	int numElem = MGetBlobArrayCount(pBlobBuffInfo);

	if (MGetBlobArraySize(pBlobBuffInfo) != (8 + (sizeof(MTD_BuffInfo) * numElem)))
	{
		return;
	}

	for (int i = 0; i < numElem; ++i)
	{
		pBuffInfo = (MTD_BuffInfo*)MGetBlobArrayElement(pBlobBuffInfo, i);

		ApplyPotion(pBuffInfo->nItemId, pSender, (float)pBuffInfo->nRemainedTime);
	}
}

void ZGame::OnGameRoundState(const MUID & uidStage, int nRound, int nRoundState, int nArg)
{
	if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME) return;
	ZMatch* pMatch = GetMatch();
	if (pMatch == NULL) return;

	MMATCH_ROUNDSTATE RoundState = MMATCH_ROUNDSTATE(nRoundState);

	if ((RoundState == MMATCH_ROUNDSTATE_FREE) && (pMatch->GetRoundState() != RoundState))
	{
		pMatch->InitCharactersPosition();
		m_pMyCharacter->SetVisible(true);
		m_pMyCharacter->Revival();
		ReleaseObserver();
	}

	pMatch->SetRound(nRound);
	pMatch->SetRoundState(RoundState, nArg);
	AddEffectRoundState(RoundState, nArg);

	if (RoundState == MMATCH_ROUNDSTATE_FINISH)
	{
		ZGetMyInfo()->GetGameInfo()->InitRound();
	}
}

bool ZGame::FilterDelayedCommand(MCommand * pCommand)
{
	bool bFiltered = true;
	float fDelayTime = 0;

	MUID uid = pCommand->GetSenderUID();
	ZCharacter* pChar = ZGetCharacterManager()->Find(uid);
	if (!pChar) return false;

	switch (pCommand->GetID())
	{
	case MC_PEER_SKILL:
	{
		int nSkill;
		pCommand->GetParameter(&nSkill, 0, MPT_INT);
		fDelayTime = .15f;
		switch (nSkill) {
		case ZC_SKILL_UPPERCUT:
			if (pChar != m_pMyCharacter) pChar->SetAnimationLower(ZC_STATE_LOWER_UPPERCUT);
			break;
		case ZC_SKILL_SPLASHSHOT: break;
		case ZC_SKILL_DASH: break;
		}

		int sel_type;
		pCommand->GetParameter(&sel_type, 2, MPT_INT);
		MMatchCharItemParts parts = (MMatchCharItemParts)sel_type;
		if (parts != pChar->GetItems()->GetSelectedWeaponParts()) {
			OnChangeWeapon(uid, parts);
		}
	}break;

	case MC_PEER_SHOT:
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);
		if (pParam->GetType() != MPT_BLOB) break;
		ZPACKEDSHOTINFO* pinfo = (ZPACKEDSHOTINFO*)pParam->GetPointer();

		if (pinfo->sel_type != MMCIP_MELEE) return false;

		if (pChar != m_pMyCharacter &&
			(pChar->m_pVMesh->m_SelectWeaponMotionType == eq_wd_dagger ||
				pChar->m_pVMesh->m_SelectWeaponMotionType == eq_ws_dagger)) {
			pChar->SetAnimationUpper(ZC_STATE_UPPER_SHOT);
		}

		fDelayTime = .15f;

		MMatchCharItemParts parts = (MMatchCharItemParts)pinfo->sel_type;
		if (parts != pChar->GetItems()->GetSelectedWeaponParts()) {
			OnChangeWeapon(uid, parts);
		}
	}
	break;

	case MC_PEER_SHOT_MELEE:
	{
		float fShotTime;
		rvector pos;
		int nShot;

		pCommand->GetParameter(&fShotTime, 0, MPT_FLOAT);
		pCommand->GetParameter(&pos, 1, MPT_POS);
		pCommand->GetParameter(&nShot, 2, MPT_INT);

		if (pChar != m_pMyCharacter &&
			(pChar->m_pVMesh->m_SelectWeaponMotionType == eq_wd_dagger ||
				pChar->m_pVMesh->m_SelectWeaponMotionType == eq_ws_dagger)) {
			pChar->SetAnimationUpper(ZC_STATE_UPPER_SHOT);
		}

		fDelayTime = .1f;
		switch (nShot) {
		case 1: fDelayTime = .10f; break;
		case 2: fDelayTime = .15f; break;
		case 3: fDelayTime = .2f; break;
		case 4: fDelayTime = .25f; break;
		}

		if (nShot > 1)
		{
			char szFileName[20];
			if (pChar->GetProperty()->nSex == MMS_MALE)
				sprintf(szFileName, "fx2/MAL_shot_%02d", nShot);
			else
				sprintf(szFileName, "fx2/FEM_shot_%02d", nShot);

			ZGetSoundEngine()->PlaySound(szFileName, pChar->GetPosition());
		}
	}
	break;

	case MC_QUEST_PEER_NPC_ATTACK_MELEE:
		ZGetQuest()->OnPrePeerNPCAttackMelee(pCommand);
		fDelayTime = .4f; break;

	default:
		bFiltered = false;
		break;
	}

	if (bFiltered)
	{
		ZObserverCommandItem* pZCommand = new ZObserverCommandItem;
		pZCommand->pCommand = pCommand->Clone();
		pZCommand->fTime = GetTime() + fDelayTime;
		m_DelayedCommandList.push_back(pZCommand);
		return true;
	}

	return false;
}

void ZGame::PostSpMotion(ZC_SPMOTION_TYPE mtype)
{
	if (m_pMyCharacter == NULL) return;
	if (m_Match.GetRoundState() != MMATCH_ROUNDSTATE_PLAY) return;

	if ((m_pMyCharacter->m_AniState_Lower.Ref() == ZC_STATE_LOWER_IDLE1) ||
		(m_pMyCharacter->m_AniState_Lower.Ref() == ZC_STATE_LOWER_IDLE2) ||
		(m_pMyCharacter->m_AniState_Lower.Ref() == ZC_STATE_LOWER_IDLE3) ||
		(m_pMyCharacter->m_AniState_Lower.Ref() == ZC_STATE_LOWER_IDLE4))
	{
		MMatchWeaponType type = MWT_NONE;

		ZItem* pSItem = m_pMyCharacter->GetItems()->GetSelectedWeapon();

		if (pSItem && pSItem->GetDesc()) {
			type = pSItem->GetDesc()->m_nWeaponType.Ref();
		}

		if (mtype == ZC_SPMOTION_TAUNT)
			if ((type == MWT_MED_KIT) ||
				(type == MWT_REPAIR_KIT) ||
				(type == MWT_FOOD) ||
				(type == MWT_BULLET_KIT))
			{
				return;
			}

		ZPostSpMotion(mtype);
	}
}

void ZGame::OnEventUpdateJjang(const MUID & uidChar, bool bJjang)
{
	ZCharacter* pCharacter = m_CharacterManager.Find(uidChar);
	if (pCharacter == NULL) return;

	if (bJjang)
		ZGetEffectManager()->AddStarEffect(pCharacter);
}

bool ZGame::CanAttack(ZObject * pAttacker, ZObject * pTarget)
{
	if (!IsReplay())
		if (GetMatch()->GetRoundState() != MMATCH_ROUNDSTATE_PLAY) return false;
	if (pAttacker == NULL) return true;

	if (GetMatch()->IsTeamPlay()) {
		if (pAttacker->GetTeamID() == pTarget->GetTeamID()) {
			if (!GetMatch()->GetTeamKillEnabled())
				return false;
		}
	}

#ifdef _QUEST
	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
	{
		if (pAttacker->GetTeamID() == pTarget->GetTeamID())
		{
			return false;
		}
	}

#endif
	return true;
}

bool ZGame::CanAttack_DebugRegister(ZObject * pAttacker, ZObject * pTarget)
{
	if (!IsReplay())
		if (GetMatch()->GetRoundState() != MMATCH_ROUNDSTATE_PLAY) return false;
	if (pAttacker == NULL) return true;

	if (GetMatch()->IsTeamPlay()) {
		if (pAttacker->GetTeamID() == pTarget->GetTeamID()) {
			if (!GetMatch()->GetTeamKillEnabled())
				return false;
		}
	}

#ifdef _QUEST
	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
	{
		if (pAttacker->GetTeamID() == pTarget->GetTeamID())
		{
			return false;
		}
	}

#endif
	return true;
}

void ZGame::ShowReplayInfo(bool bShow)
{
	MWidget* pWidget = ZGetGameInterface()->GetIDLResource()->FindWidget("CombatChatOutput");
	if (pWidget)
		pWidget->Show(bShow);

	m_bShowReplayInfo = bShow;
}

void ZGame::OnLocalOptainSpecialWorldItem(MCommand * pCommand)
{
	int nWorldItemID;
	pCommand->GetParameter(&nWorldItemID, 0, MPT_INT);

	switch (nWorldItemID)
	{
	case WORLDITEM_PORTAL_ID:
	{
		MMATCH_GAMETYPE eGameType = ZGetGameClient()->GetMatchStageSetting()->GetGameType();
		if (!ZGetGameTypeManager()->IsQuestDerived(eGameType)) break;

		char nCurrSectorIndex = ZGetQuest()->GetGameInfo()->GetCurrSectorIndex();
		ZPostQuestRequestMovetoPortal(nCurrSectorIndex);
	}
	break;
	};
}

void ZGame::ReserveSuicide(void)
{
	m_bSuicide = true;
}

bool ZGame::OnRuleCommand(MCommand * pCommand)
{
#ifdef _QUEST
	if (ZGetQuest()->OnGameCommand(pCommand)) return true;
#endif

	switch (pCommand->GetID())
	{
	case MC_MATCH_ASSIGN_COMMANDER:
	case MC_MATCH_ASSIGN_BERSERKER:
	case MC_MATCH_FLAG_EFFECT:
	case MC_MATCH_FLAG_CAP:
	case MC_MATCH_FLAG_STATE:
	case MC_MATCH_GAME_DEAD:
	case MC_MATCH_DUEL_QUEUEINFO:
	case MC_MATCH_DUELTOURNAMENT_GAME_NEXT_MATCH_PLYAERINFO:
	case MC_MATCH_DUELTOURNAMENT_GAME_INFO:
	case MC_MATCH_DUELTOURNAMENT_GAME_ROUND_RESULT_INFO:
	case MC_MATCH_DUELTOURNAMENT_GAME_MATCH_RESULT_INFO:
	{
		if (m_Match.OnCommand(pCommand)) return true;
	};
	};

	return false;
}

void ZGame::OnResetTeamMembers(MCommand * pCommand)
{
	if (!m_Match.IsTeamPlay()) return;

	ZChatOutput(MCOLOR(ZCOLOR_GAME_INFO), ZMsg(MSG_GAME_MAKE_AUTO_BALANCED_TEAM));

	MCommandParameter* pParam = pCommand->GetParameter(0);
	if (pParam->GetType() != MPT_BLOB) return;
	void* pBlob = pParam->GetPointer();
	int nCount = MGetBlobArrayCount(pBlob);

	ZCharacterManager* pCharMgr = ZGetCharacterManager();

	for (int i = 0; i < nCount; i++)
	{
		MTD_ResetTeamMembersData* pDataNode = (MTD_ResetTeamMembersData*)MGetBlobArrayElement(pBlob, i);

		ZCharacter* pChar = pCharMgr->Find(pDataNode->m_uidPlayer);
		if (pChar == NULL) continue;

		if (pChar->GetTeamID() != ((MMatchTeam)pDataNode->nTeam))
		{
			if (pDataNode->m_uidPlayer == ZGetMyUID())
			{
				ZGetMyInfo()->GetGameInfo()->bForcedChangeTeam = true;
			}

			pChar->SetTeamID((MMatchTeam)pDataNode->nTeam);
		}
	}
}

void ZGame::MakeResourceCRC32(const DWORD dwKey, DWORD & out_crc32, DWORD & out_xor)
{
	out_crc32 = 0;
	out_xor = 0;

#ifdef _DEBUG
	static DWORD dwOutputCount = 0;
	++dwOutputCount;
#endif

	MMatchObjCacheMap* pObjCacheMap = ZGetGameClient()->GetObjCacheMap();
	if (NULL == pObjCacheMap)
	{
		return;
	}

	MMatchObjCacheMap::const_iterator	end = pObjCacheMap->end();
	MMatchObjCacheMap::iterator			it = pObjCacheMap->begin();
	MMatchObjCache* pObjCache = NULL;
	MMatchItemDesc* pitemDesc = NULL;
	MMatchCRC32XORCache					CRC32Cache;

	CRC32Cache.Reset();
	CRC32Cache.CRC32XOR(dwKey);

#ifdef _DEBUG
	mlog("Start ResourceCRC32Cache : %u\n", CRC32Cache.GetXOR());
#endif

	for (; end != it; ++it)
	{
		pObjCache = it->second;

		for (int i = 0; i < MMCIP_END; ++i)
		{
			pitemDesc = MGetMatchItemDescMgr()->GetItemDesc(pObjCache->GetCostume()->nEquipedItemID[i]);
			if (NULL == pitemDesc)
			{
				continue;
			}

			pitemDesc->CacheCRC32(CRC32Cache);

#ifdef _DEBUG
			if (10 > dwOutputCount)
			{
				mlog("ItemID : %d, CRCCache : %u\n"
					, pitemDesc->m_nID
					, CRC32Cache.GetXOR());
			}
#endif
		}
	}

#ifdef _DEBUG
	if (10 > dwOutputCount)
	{
		mlog("ResourceCRCSum : %u\n", CRC32Cache.GetXOR());
	}
#endif

	out_crc32 = CRC32Cache.GetCRC32();
	out_xor = CRC32Cache.GetXOR();
}

void ZGame::OnResponseUseSpendableBuffItem(MUID & uidItem, int nResult)
{
}

void ZGame::ApplyPotion(int nItemID, ZCharacter * pCharObj, float fRemainedTime)
{
	MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pDesc == NULL) { return; }

	MMatchDamageType nDamageType = pDesc->m_nDamageType.Ref();

	if (nDamageType == MMDT_HASTE)
	{
		ZModule_Movable* pMod = (ZModule_Movable*)pCharObj->GetModule(ZMID_MOVABLE);
		if (pMod)
		{
			if (fRemainedTime == 0)
				fRemainedTime = pDesc->m_nDamageTime.Ref() * 0.001f;

			pMod->SetMoveSpeedHasteRatio(pDesc->m_nItemPower.Ref() * 0.01f, fRemainedTime, nItemID);
		}
		ZGetEffectManager()->AddHasteBeginEffect(pCharObj->GetPosition(), pCharObj);
	}
	else if (nDamageType == MMDT_HEAL || nDamageType == MMDT_REPAIR)
	{
		if (pDesc->m_nDamageTime.Ref() == 0)
		{
			ZGetEffectManager()->AddPotionEffect(pCharObj->GetPosition(), pCharObj, pDesc->m_nEffectId);

			if (nDamageType == MMDT_HEAL)
			{
				int nAddedHP = pDesc->m_nItemPower.Ref();
				pCharObj->SetHP(min(pCharObj->GetHP() + nAddedHP, pCharObj->GetMaxHP()));
			}
			else if (nDamageType == MMDT_REPAIR)
			{
				int nAddedAP = pDesc->m_nItemPower.Ref();
				pCharObj->SetAP(min(pCharObj->GetAP() + nAddedAP, pCharObj->GetMaxAP()));
			}
		}
		else
		{
			ZModule_HealOverTime* pMod = (ZModule_HealOverTime*)pCharObj->GetModule(ZMID_HEALOVERTIME);
			if (pMod)
			{
				int nRemainedHeal = (int)fRemainedTime;
				if (nRemainedHeal == 0)
					nRemainedHeal = pDesc->m_nDamageTime.Ref();

				pMod->BeginHeal(pDesc->m_nDamageType.Ref(), pDesc->m_nItemPower.Ref(), nRemainedHeal, pDesc->m_nEffectId, nItemID);
			}

			switch (nDamageType)
			{
			case MMDT_HEAL:
				ZGetEffectManager()->AddHealOverTimeBeginEffect(pCharObj->GetPosition(), pCharObj);
				break;
			case MMDT_REPAIR:
				ZGetEffectManager()->AddRepairOverTimeBeginEffect(pCharObj->GetPosition(), pCharObj);
				break;
			}
		}
	}
}

void ZGame::OnUseTrap(int nItemID, ZCharacter * pCharObj, rvector & pos)
{
	MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pDesc == NULL) { return; }

	rvector velocity;
	velocity = pCharObj->m_TargetDir * 1300.f;
	velocity.z = velocity.z + 300.f;
	m_WeaponManager.AddTrap(pos, velocity, nItemID, pCharObj);
}

void ZGame::OnUseDynamite(int nItemID, ZCharacter * pCharObj, rvector & pos)
{
	MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pDesc == NULL) { return; }

	rvector velocity;
	velocity = pCharObj->m_TargetDir * 1300.f;
	velocity.z = velocity.z + 300.f;
	m_WeaponManager.AddDynamite(pos, velocity, pCharObj);
}

void ZGame::CheckZoneTrap(MUID uidOwner, rvector pos, MMatchItemDesc * pItemDesc, MMatchTeam nTeamID)
{
	if (!pItemDesc) return;

	float fRange = 300.f;

	ZObject* pTarget = NULL;
	ZCharacter* pOwnerCharacter = m_CharacterManager.Find(uidOwner);

	float fDist;
	bool bReturnValue;

	for (ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor)
	{
		pTarget = (*itor).second;

#ifndef _DEBUG
		bReturnValue = pTarget->GetUID() == uidOwner;
		if (pTarget->GetUID() == uidOwner)
			PROTECT_DEBUG_REGISTER(bReturnValue)
			continue;
#endif

		bReturnValue = CanAttack(pOwnerCharacter, pTarget);
		if (!bReturnValue)
			PROTECT_DEBUG_REGISTER(!CanAttack_DebugRegister(pOwnerCharacter, pTarget))
			continue;

		bReturnValue = !pTarget || pTarget->IsDie();
		if (!pTarget || pTarget->IsDie())
			PROTECT_DEBUG_REGISTER(bReturnValue)
			continue;

		fDist = Magnitude(pos - (pTarget->GetPosition() + rvector(0, 0, 10)));
		bReturnValue = fDist >= fRange;
		if (fDist >= fRange)
			PROTECT_DEBUG_REGISTER(bReturnValue)
			continue;

		if (pos.z > pTarget->GetPosition().z + pTarget->GetCollHeight())
			continue;

		if (m_pGameAction)
		{
			int nDuration = pItemDesc->m_nDamageTime.Ref();
			bool bApplied = false;
			switch (pItemDesc->m_nDamageType.Ref())
			{
			case MMDT_FIRE:
				bApplied = m_pGameAction->ApplyFireEnchantDamage(pTarget, pOwnerCharacter, pItemDesc->m_nItemPower.Ref(), nDuration);
				break;
			case MMDT_COLD:
				bApplied = m_pGameAction->ApplyColdEnchantDamage(pTarget, pItemDesc->m_nItemPower.Ref(), nDuration);
				break;
			default:
			{
				break;
			}
			}

			if (bApplied)
			{
				if (pOwnerCharacter)
				{
					CheckCombo(pOwnerCharacter, pTarget, true);
					CheckStylishAction(pOwnerCharacter);
				}

				GetWorld()->GetWaters()->CheckSpearing(pos, pos + rvector(0, 0, MAX_WATER_DEEP), 500, 0.8f);
			}
		}
	}
}

void ZGame::OnExplosionDynamite(MUID uidOwner, rvector pos, float fDamage, float fRange, float fKnockBack, MMatchTeam nTeamID)
{
	ZObject* pTarget = NULL;

	float fDist;

	for (ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor)
	{
		pTarget = (*itor).second;

		bool bReturnValue = !pTarget || pTarget->IsDie();
		if (!pTarget || pTarget->IsDie())
			PROTECT_DEBUG_REGISTER(bReturnValue)
			continue;

		fDist = Magnitude(pos - (pTarget->GetPosition() + rvector(0, 0, 80)));
		bReturnValue = fDist >= fRange;
		if (fDist >= fRange)
			PROTECT_DEBUG_REGISTER(bReturnValue)
			continue;

		rvector dir = pos - (pTarget->GetPosition() + rvector(0, 0, 80));
		Normalize(dir);

		ZActor* pATarget = MDynamicCast(ZActor, pTarget);

		bool bPushSkip = false;

		if (pATarget)
		{
			bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;
		}

		if (bPushSkip == false)
		{
			pTarget->AddVelocity(fKnockBack * 7.f * (fRange - fDist) * -dir);
		}
		else
		{
			ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met");
		}
		ZCharacter* pOwnerCharacter = ZGetGame()->m_CharacterManager.Find(uidOwner);
		if (pOwnerCharacter)
		{
			CheckCombo(pOwnerCharacter, pTarget, !bPushSkip);
			CheckStylishAction(pOwnerCharacter);
		}

		float fRatio = ZItem::GetPiercingRatio(MWT_DYNAMITYE, eq_parts_chest);
		pTarget->OnDamaged(pOwnerCharacter, pos, ZD_EXPLOSION, MWT_DYNAMITYE, fDamage, fRatio);
	}

#define SHOCK_RANGE		1500.f

	ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	float fPower = (SHOCK_RANGE - Magnitude(pTargetCharacter->GetPosition() + rvector(0, 0, 50) - pos)) / SHOCK_RANGE;

	if (fPower > 0)
		ZGetGameInterface()->GetCamera()->Shock(fPower * 500.f, .5f, rvector(0.0f, 0.0f, -1.0f));

	GetWorld()->GetWaters()->CheckSpearing(pos, pos + rvector(0, 0, MAX_WATER_DEEP), 500, 0.8f);
}