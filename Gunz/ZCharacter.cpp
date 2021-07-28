#include "stdafx.h"

#include "ZApplication.h"
#include "ZGame.h"
#include "ZCharacter.h"
#include "RVisualMeshMgr.h"
#include "RealSpace2.h"
#include "MDebug.h"
#include "MObject.h"
#include "ZPost.h"
#include "ZGameInterface.h"
#include "RBspObject.h"
#include "zshadow.h"
#include "MProfiler.h"
#include "RShaderMgr.h"
#include "ZScreenEffectManager.h"
#include "RDynamicLight.h"
#include "ZConfiguration.h"
#include "MMatchObject.h"
#include "RCollisionDetection.h"
#include "ZEffectStaticMesh.h"
#include "ZEffectAniMesh.h"
#include "ZModule_HPAP.h"
#include "ZModule_Movable.h"
#include "ZModule_Resistance.h"
#include "ZModule_FireDamage.h"
#include "ZModule_ColdDamage.h"
#include "ZModule_LightningDamage.h"
#include "ZModule_PoisonDamage.h"
#include "ZModule_QuestStatus.h"
#include "ZModule_HealOverTime.h"
#include "ZGameConst.h"

#define ANGLE_TOLER			.1f
#define ANGLE_SPEED			12.f

#define ROLLBACK_TOLER		20.f

#define ENABLE_CHARACTER_COLLISION

#define ENABLE_FALLING_DAMAGE
#define IsKeyDown(key) ((GetAsyncKeyState(key) & 0x8000)!=0)
bool Enable_Cloth = true;

static ZANIMATIONINFO g_AnimationInfoTableLower[ZC_STATE_LOWER_END] = {
	{ ""				,true	,true	,false 	,false },

	{ "idle"			,true	,true	,false 	,false },
	{ "idle2"			,true	,true	,false 	,false },
	{ "idle3"			,true	,true	,false 	,false },
	{ "idle4"			,true	,true	,false 	,false },

	{ "run"				,true	,true	,false 	,false },
	{ "runB"			,true	,true	,false 	,false },
	{ "runL"			,true	,true	,false 	,false },
	{ "runR"			,true	,true	,false 	,false },

	{ "jumpU"			,true	,false	,false 	,false },
	{ "jumpD"			,true	,false	,false 	,false },

	{ "die" 			,true	,false	,false 	,false },
	{ "die2" 			,true	,false	,false 	,false },
	{ "die3" 			,true	,false	,false 	,false },
	{ "die4"			,true	,false	,false 	,false },

	{ "runLW"			,false	,true 	,false	,false},
	{ "runLW_down"		,false	,false	,false	,false},
	{ "runW" 			,false	,false	,true	,false},
	{ "runW_downF"		,false	,false	,false	,false},
	{ "runW_downB"		,false	,false	,false	,false},
	{ "runRW" 			,false	,true 	,false	,false},
	{ "runRW_down"		,false	,false 	,false	,false},

	{ "tumbleF"			,false	,false	,false	,false },
	{ "tumbleB"			,false	,false	,false	,false },
	{ "tumbleR"			,false	,false	,false	,false },
	{ "tumbleL"			,false	,false	,false	,false },

	{ "bind"			,false	,false	,false	,false },

	{ "jumpwallF"		,false	,false 	,false	,false },
	{ "jumpwallB"		,false	,false 	,false	,false },
	{ "jumpwallL"		,false	,false 	,false	,false },
	{ "jumpwallR"		,false	,false 	,false	,false },

	{ "attack1"			,false	,false 	,true  	,false },
	{ "attack1_ret"		,false	,false 	,true  	,true  },
	{ "attack2"			,false	,false 	,true  	,false },
	{ "attack2_ret"		,false	,false 	,true  	,true  },
	{ "attack3"			,false	,false 	,true  	,false },
	{ "attack3_ret"		,false	,false 	,true  	,true  },
	{ "attack4"			,false	,false 	,true  	,false },
	{ "attack4_ret"		,false	,false 	,true  	,true  },
	{ "attack5"			,false	,false 	,true  	,false },

	{ "attack_Jump"		,false	,false 	,false	,false },
	{ "uppercut"		,false	,false 	,true	,false },

	{ "guard_start"		,false	,false 	,true 	,false },
	{ "guard_idle"		,false	,false 	,false	,false },
	{ "guard_block1"	,false	,false 	,false	,false },
	{ "guard_block1_ret",false	,false 	,false	,false },
	{ "guard_block2"	,false	,false 	,false	,false },
	{ "guard_cancel"	,false	,false 	,false	,false },

	{ "blast"			,false	,false 	,false 	,false },
	{ "blast_fall"		,false	,false 	,false 	,false },
	{ "blast_drop"		,false	,false 	,false 	,false },
	{ "blast_stand"		,false	,false 	,false 	,false },
	{ "blast_airmove"	,false	,false 	,false 	,false },

	{ "blast_dagger"	 ,false ,false 	,false 	,false },
	{ "blast_drop_dagger",false ,false 	,false 	,false },

	{ "damage"			,false	,false 	,false 	,false },
	{ "damage2"			,false	,false 	,false 	,false },
	{ "damage_down"		,false	,false 	,false 	,false },

	{ "taunt"			,true	,false	,false	,false },
	{ "bow"				,true	,false	,false	,false },
	{ "wave"			,true	,false	,false	,false },
	{ "laugh"			,true	,false	,false	,false },
	{ "cry"				,true	,false	,false	,false },
	{ "dance"			,true	,false	,false	,false },

	{ "cancel"			,false	,false 	,false 	,false },
	{ "charge"			,false	,false 	,true  	,true  },
	{ "slash"			,false	,false 	,true  	,false },
	{ "jump_slash1"		,false	,false 	,false	,false },
	{ "jump_slash2"		,false	,false 	,false	,false },

	{ "lightning"		,false	,false 	,false	,false },
	{ "stun"			,false	,false 	,false	,false },

	{ "pit"				,false	,false 	,false	,false },
};

static ZANIMATIONINFO g_AnimationInfoTableUpper[ZC_STATE_UPPER_END] = {
	{ ""				,true	,true	,false	,false },

	{ "attackS"			,false	,false	,false	,false },
	{ "reload"			,false	,false	,false	,false },
	{ "load"			,false	,false	,false	,false },

	{ "guard_start"		,false	,false 	,false	,false },
	{ "guard_idle"		,false	,false 	,false	,false },
	{ "guard_block1"	,false	,false 	,false	,false },
	{ "guard_block1_ret",false	,false 	,false	,false },
	{ "guard_block2"	,false	,false 	,false	,false },
	{ "guard_cancel"	,false	,false 	,false	,false },
};

bool CheckTeenVersionMesh(RMesh** ppMesh)
{
	RWeaponMotionType type = eq_weapon_etc;

	type = (*ppMesh)->GetMeshWeaponMotionType();

	if (ZApplication::GetGameInterface()->GetTeenVersion()) {
		if (type == eq_wd_katana) {
			*ppMesh = ZGetWeaponMeshMgr()->Get("katana_wood");
			return true;
		}
		else if (type == eq_ws_dagger) {
			*ppMesh = ZGetWeaponMeshMgr()->Get("dagger_wood");
			return true;
		}
		else if (type == eq_wd_sword) {
			*ppMesh = ZGetWeaponMeshMgr()->Get("sword_wood");
			return true;
		}
		else if (type == eq_wd_blade) {
			*ppMesh = ZGetWeaponMeshMgr()->Get("blade_wood");
			return true;
		}
	}

	return false;
}

void ChangeCharHair(RVisualMesh* pVMesh, MMatchSex nSex, int nHairIndex)
{
	if ((nHairIndex < 0) || (nHairIndex >= MAX_COSTUME_HAIR)) return;
	if (pVMesh == NULL) return;

	char szMeshName[256];
	if (nSex == MMS_MALE) {
		strcpy(szMeshName, g_szHairMeshName[nHairIndex][MMS_MALE].c_str());
	}
	else {
		strcpy(szMeshName, g_szHairMeshName[nHairIndex][MMS_FEMALE].c_str());
	}

	pVMesh->SetParts(eq_parts_head, szMeshName);
}

void ChangeEquipAvatarParts(RVisualMesh* pVMesh, const unsigned long int* pItemID, MMatchSex nSex, int nHairIndex)
{
	pVMesh->ClearParts();

	char* szMeshName;
	MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(pItemID[MMCIP_AVATAR]);
	if (pDesc != NULL) {
		szMeshName = pDesc->m_pAvatarMeshName->Ref().m_szHeadMeshName;
		if (strlen(szMeshName) > 0)	pVMesh->SetParts(eq_parts_head, szMeshName);
		else							ChangeCharHair(pVMesh, nSex, nHairIndex);

		szMeshName = pDesc->m_pAvatarMeshName->Ref().m_szChestMeshName;
		if (strlen(szMeshName) > 0)	pVMesh->SetParts(eq_parts_chest, szMeshName);
		else							pVMesh->SetBaseParts(eq_parts_chest);

		szMeshName = pDesc->m_pAvatarMeshName->Ref().m_szHandMeshName;
		if (strlen(szMeshName) > 0)	pVMesh->SetParts(eq_parts_hands, szMeshName);
		else							pVMesh->SetBaseParts(eq_parts_hands);

		szMeshName = pDesc->m_pAvatarMeshName->Ref().m_szLegsMeshName;
		if (strlen(szMeshName) > 0)	pVMesh->SetParts(eq_parts_legs, szMeshName);
		else							pVMesh->SetBaseParts(eq_parts_legs);

		szMeshName = pDesc->m_pAvatarMeshName->Ref().m_szFeetMeshName;
		if (strlen(szMeshName) > 0)	pVMesh->SetParts(eq_parts_feet, szMeshName);
		else							pVMesh->SetBaseParts(eq_parts_feet);
	}
}

void ChangeEquipParts(RVisualMesh* pVMesh, const unsigned long int* pItemID)
{
	pVMesh->ClearParts();

	struct _ZPARTSPAIR
	{
		_RMeshPartsType			meshparts;
		MMatchCharItemParts		itemparts;
	};

	static _ZPARTSPAIR PartsPair[] =
	{
		{eq_parts_head,		MMCIP_HEAD},
		{eq_parts_chest,	MMCIP_CHEST},
		{eq_parts_hands,	MMCIP_HANDS},
		{eq_parts_legs,		MMCIP_LEGS},
		{eq_parts_feet,		MMCIP_FEET}
	};

	for (int i = 0; i < 5; i++) {
		if (pItemID[PartsPair[i].itemparts] != 0) {
			MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(pItemID[PartsPair[i].itemparts]);
			if (pDesc != NULL) {
				pVMesh->SetParts(PartsPair[i].meshparts, pDesc->m_pMItemName->Ref().m_szMeshName);
			}
		}
		else {
			pVMesh->SetBaseParts(PartsPair[i].meshparts);
		}
	}

	pVMesh->SetBaseParts(eq_parts_face);
}

void ChangeCharFace(RVisualMesh* pVMesh, MMatchSex nSex, int nFaceIndex)
{
	if ((nFaceIndex < 0) || (nFaceIndex >= MAX_COSTUME_FACE)) return;
	if (pVMesh == NULL) return;

	char szMeshName[256];

	if (nSex == MMS_MALE)
	{
		strcpy(szMeshName, g_szFaceMeshName[nFaceIndex][MMS_MALE].c_str());
	}
	else
	{
		strcpy(szMeshName, g_szFaceMeshName[nFaceIndex][MMS_FEMALE].c_str());
	}

	pVMesh->SetParts(eq_parts_face, szMeshName);
}

void ZChangeCharParts(RVisualMesh* pVMesh, MMatchSex nSex, int nHair, int nFace, const unsigned long int* pItemID)
{
	if (pVMesh == NULL) {
		return;
	}

	if (pItemID[MMCIP_AVATAR] != 0) {
		ChangeEquipAvatarParts(pVMesh, pItemID, nSex, nHair);

		pVMesh->m_bSkipRenderFaceParts = true;
	}
	else {
		ChangeEquipParts(pVMesh, pItemID);

		if (pItemID[MMCIP_HEAD] == 0) {
			ChangeCharHair(pVMesh, nSex, nHair);
		}

		pVMesh->m_bSkipRenderFaceParts = false;

		ChangeCharFace(pVMesh, nSex, nFace);
	}
}

void ZChangeCharWeaponMesh(RVisualMesh* pVMesh, unsigned long int nWeaponID)
{
	if (pVMesh)
	{
		if (nWeaponID == 0)
		{
			RWeaponMotionType type = eq_ws_dagger;
			pVMesh->AddWeapon(type, NULL);
			pVMesh->SelectWeaponMotion(type);

			return;
		}

		RWeaponMotionType type = eq_weapon_etc;
		MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nWeaponID);

		if (pDesc == NULL)
		{
			return;
		}

		RMesh* pMesh = ZGetWeaponMeshMgr()->Get(pDesc->m_pMItemName->Ref().m_szMeshName);

		if (pMesh)
		{
			type = pMesh->GetMeshWeaponMotionType();
			CheckTeenVersionMesh(&pMesh);
			pVMesh->AddWeapon(type, pMesh);
			pVMesh->SelectWeaponMotion(type);
		}
	}
}

MImplementRTTI(ZCharacter, ZCharacterObject);

ZCharacter::ZCharacter() : ZCharacterObject(), m_DirectionLower(1, 0, 0), m_DirectionUpper(1, 0, 0), m_TargetDir(1, 0, 0)
{
	m_fPreMaxHP = 0.f;
	m_fPreMaxAP = 0.f;

	m_Status.MakeCrc();

	m_nMoveMode.Set_MakeCrc(MCMM_RUN);
	m_nMode.Set_MakeCrc(MCM_OFFENSIVE);
	m_nState.Set_MakeCrc(MCS_STAND);

	m_nVMID.Set_MakeCrc(-1);
	m_fLastShotTime = 0.0f;

	m_slotInfo.Ref().m_nSelectSlot = 0;
	m_slotInfo.MakeCrc();

	m_killInfo.Ref().m_fLastKillTime = 0;
	m_killInfo.Ref().m_nKillsThisRound = 0;
	m_killInfo.MakeCrc();

	m_damageInfo.Ref().m_LastDamageType = ZD_NONE;
	m_damageInfo.Ref().m_LastDamageWeapon = MWT_NONE;
	m_damageInfo.Ref().m_LastDamageDot = 0.f;
	m_damageInfo.Ref().m_LastDamageDistance = 0.f;
	m_damageInfo.Ref().m_LastThrower = MUID(0, 0);
	m_damageInfo.Ref().m_nStunType = ZST_NONE;
	m_damageInfo.Ref().m_dwLastDamagedTime = 0;
	m_damageInfo.MakeCrc();

	m_SpMotion = ZC_STATE_TAUNT;

	m_nTeamID.Set_MakeCrc(MMT_ALL);

	m_nBlastType.Set_MakeCrc(0);

	m_AniState_Upper.Set_MakeCrc(ZC_STATE_UPPER_NONE);
	m_AniState_Lower.Set_MakeCrc(ZC_STATE_LOWER_NONE);

	m_pAnimationInfo_Lower = NULL;
	m_pAnimationInfo_Upper = NULL;

	m_fLastReceivedTime = 0;

	m_fLastValidTime = 0.0f;
	m_Accel.Set_MakeCrc(rvector(0.0f, 0.0f, 0.0f));
	m_bRendered = false;

	m_nWallJumpDir.Set_MakeCrc(0);
	m_fChargedFreeTime.Set_MakeCrc(0);

	m_RealPositionBefore.Set_MakeCrc(rvector(0.f, 0.f, 0.f));
	m_AnimationPositionDiff.Set_MakeCrc(rvector(0.f, 0.f, 0.f));

	m_fGlobalHP = 0.f;
	m_nReceiveHPCount = 0;

	m_fAttack1Ratio.Set_MakeCrc(1.f);

	m_nWhichFootSound = 0;

	m_fTimeOffset = 0;
	m_nTimeErrorCount = 0;
	m_fAccumulatedTimeError = 0;

	m_pModule_HPAP = new ZModule_HPAP;
	m_pModule_Resistance = new ZModule_Resistance;
	m_pModule_FireDamage = new ZModule_FireDamage;
	m_pModule_ColdDamage = new ZModule_ColdDamage;
	m_pModule_PoisonDamage = new ZModule_PoisonDamage;
	m_pModule_LightningDamage = new ZModule_LightningDamage;
	m_pModule_HealOverTime = new ZModule_HealOverTime;
	m_pModule_QuestStatus = NULL;

	AddModule(m_pModule_HPAP);
	AddModule(m_pModule_Resistance);
	AddModule(m_pModule_FireDamage);
	AddModule(m_pModule_ColdDamage);
	AddModule(m_pModule_PoisonDamage);
	AddModule(m_pModule_LightningDamage);
	AddModule(m_pModule_HealOverTime);

	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
	{
		m_pModule_QuestStatus = new ZModule_QuestStatus();
		AddModule(m_pModule_QuestStatus);
	}

	m_Collision.SetRadius(CHARACTER_RADIUS - 2);
	m_Collision.SetHeight(CHARACTER_HEIGHT);

	m_bCharged = new MProtectValue<bool>;
	m_bCharged->Set_MakeCrc(false);
	m_bCharging = new MProtectValue<bool>;
	m_bCharging->Set_MakeCrc(false);

	m_pMdwInvincibleStartTime = new MProtectValue<DWORD>;
	m_pMdwInvincibleStartTime->Set_MakeCrc(0);
	m_pMdwInvincibleDuration = new MProtectValue<DWORD>;
	m_pMdwInvincibleDuration->Set_MakeCrc(0);

	ZCharaterStatusBitPacking uState;
	uState.dwFlagsPublic = 1;

	uState.m_bTagger = false;
	m_dwStatusBitPackingValue.Set(uState);

	m_pMUserAndClanName = new MProtectValue<ZUserAndClanName>;
	m_pMUserAndClanName->Ref().m_szUserName[0] = 0;
	m_pMUserAndClanName->Ref().m_szUserAndClanName[0] = 0;
	m_pMUserAndClanName->MakeCrc();
	SetInvincibleTime(0);
}

ZCharacter::~ZCharacter()
{
	RemoveModule(m_pModule_HPAP);
	RemoveModule(m_pModule_Resistance);
	RemoveModule(m_pModule_FireDamage);
	RemoveModule(m_pModule_ColdDamage);
	RemoveModule(m_pModule_PoisonDamage);
	RemoveModule(m_pModule_LightningDamage);
	RemoveModule(m_pModule_HealOverTime);
	RemoveModule(m_pModule_QuestStatus);

	delete m_pModule_HPAP;
	delete m_pModule_Resistance;
	delete m_pModule_FireDamage;
	delete m_pModule_ColdDamage;
	delete m_pModule_PoisonDamage;
	delete m_pModule_LightningDamage;
	delete m_pModule_HealOverTime;

	SAFE_DELETE(m_bCharged);
	SAFE_DELETE(m_bCharging);
	SAFE_DELETE(m_pMdwInvincibleDuration);
	SAFE_DELETE(m_pMdwInvincibleStartTime);
	SAFE_DELETE(m_pMUserAndClanName);

	SAFE_DELETE(m_pModule_QuestStatus);

	EmptyHistory();

	Destroy();
}

void ZCharacter::EmptyHistory()
{
	if (m_bInitialized == false) return;

	while (m_BasicHistory.size())
	{
		delete* m_BasicHistory.begin();
		m_BasicHistory.erase(m_BasicHistory.begin());
	}
}

void ZCharacter::Stop()
{
	SetAnimationLower(ZC_STATE_LOWER_IDLE1);
}

bool ZCharacter::IsTeam(ZCharacter* pChar)
{
	if (pChar) {
		if (pChar->GetTeamID() == GetTeamID()) {
			return true;
		}
	}
	return false;
}

bool ZCharacter::IsMoveAnimation()
{
	return g_AnimationInfoTableLower[m_AniState_Lower.Ref()].bMove;
}

void ZCharacter::SetAnimation(char* AnimationName, bool bEnableCancel, int time)
{
	if (m_bInitialized == false) return;

	SetAnimation(ani_mode_lower, AnimationName, bEnableCancel, time);
}

void ZCharacter::SetAnimation(RAniMode mode, char* AnimationName, bool bEnableCancel, int time)
{
	if (m_bInitialized == false)
		return;

	if (!m_pVMesh)
		return;

	if (time) {
		m_pVMesh->SetReserveAnimation(mode, AnimationName, time);
	}
	else {
		m_pVMesh->SetAnimation(mode, AnimationName, bEnableCancel);
	}
}

DWORD g_dwLastAnimationTime = timeGetTime();

void ZCharacter::SetAnimationLower(ZC_STATE_LOWER nAni)
{
	if (m_bInitialized == false) return;
	if ((IsDie()) && (IsHero())) return;

	if (nAni == m_AniState_Lower.Ref()) return;
	if (nAni < 0 || nAni >= ZC_STATE_LOWER_END)
	{
		return;
	}

	m_AniState_Lower.Set_CheckCrc(nAni);
	m_pAnimationInfo_Lower = &g_AnimationInfoTableLower[nAni];

	SetAnimation(ani_mode_lower, m_pAnimationInfo_Lower->Name, m_pAnimationInfo_Lower->bEnableCancel, 0);

	{
		if (!g_AnimationInfoTableLower[nAni].bContinuos)
			m_RealPositionBefore.Set_CheckCrc(rvector(0, 0, 0));
	}

#ifdef TRACE_ANIMATION
	{
		DWORD curtime = timeGetTime();
		if (ZGetGame()->m_pMyCharacter == this)
			mlog("animation - %d %s   - %d frame , interval %d \n", nAni,
				m_pVMesh->GetFrameInfo(ani_mode_lower)->m_pAniSet->GetName(), m_pVMesh->GetFrameInfo(ani_mode_lower)->m_nFrame,
				curtime - g_dwLastAnimationTime);
		g_dwLastAnimationTime = curtime;
	}
#endif
}

void ZCharacter::SetAnimationUpper(ZC_STATE_UPPER nAni)
{
	if (m_bInitialized == false) return;
	if ((IsDie()) && (IsHero())) return;

	if (nAni == m_AniState_Upper.Ref()) 	return;

	if (nAni < 0 || nAni >= ZC_STATE_UPPER_END)
	{
		return;
	}

#ifdef TRACE_ANIMATION
	{
		DWORD curtime = timeGetTime();
		mlog("upper Animation Index : %d %s @ %d \n", nAni, g_AnimationInfoTableUpper[nAni].Name, curtime - g_dwLastAnimationTime);
		if (m_AniState_Upper.Ref() == 3 && nAni == 0)
		{
			bool a = false;
		}
		g_dwLastAnimationTime = curtime;
	}
#endif

	m_AniState_Upper.Set_CheckCrc(nAni);
	m_pAnimationInfo_Upper = &g_AnimationInfoTableUpper[nAni];
	if (m_pAnimationInfo_Upper == NULL || m_pAnimationInfo_Upper->Name == NULL)
	{
		mlog("Fail to Get Animation Info.. Ani Index : [%d]\n", nAni);
		return;
	}
	SetAnimation(ani_mode_upper, m_pAnimationInfo_Upper->Name, m_pAnimationInfo_Upper->bEnableCancel, 0);
}

void ZCharacter::UpdateLoadAnimation()
{
	if (m_bInitialized == false) return;

	if (m_pAnimationInfo_Lower)
	{
		SetAnimation(m_pAnimationInfo_Lower->Name, m_pAnimationInfo_Lower->bEnableCancel, 0);
		SetAnimationUpper(ZC_STATE_UPPER_NONE);
		SetAnimationUpper(ZC_STATE_UPPER_LOAD);
		m_dwStatusBitPackingValue.Ref().m_bPlayDone_upper = false;
	}
}

bool CheckVec(rvector& v1, rvector& v2) {
	if (fabs(v1.x - v2.x) < 0.03f)
		if (fabs(v1.y - v2.y) < 0.03f)
			if (fabs(v1.z - v2.z) < 0.03f)
				return false;
	return true;
}

bool g_debug_rot = false;

void ZCharacter::UpdateMotion(float fDelta)
{
	if (m_bInitialized == false) return;
	if (IsDie()) {
		m_pVMesh->m_vRotXYZ.x = 0.f;
		m_pVMesh->m_vRotXYZ.y = 0.f;
		m_pVMesh->m_vRotXYZ.z = 0.f;

		return;
	}

	if ((m_AniState_Lower.Ref() == ZC_STATE_LOWER_IDLE1) ||
		(m_AniState_Lower.Ref() == ZC_STATE_LOWER_RUN_FORWARD) ||
		(m_AniState_Lower.Ref() == ZC_STATE_LOWER_RUN_BACK))
	{
		m_Direction = m_TargetDir;

		rvector targetdir = m_TargetDir;
		targetdir.z = 0;
		Normalize(targetdir);

		rvector dir = m_Accel.Ref();
		dir.z = 0;

		if (Magnitude(dir) < 10.f) {
			dir = targetdir;
		}
		else
			Normalize(dir);

		bool bInversed = false;
		if (DotProduct(targetdir, dir) < -cos(pi / 4.f) + 0.01f)
		{
			dir = -dir;
			bInversed = true;
		}

		float fAngleLower = GetAngleOfVectors(dir, m_DirectionLower);

		rmatrix mat;

#define ROTATION_SPEED	400.f

		if (fAngleLower > 5.f / 180.f * pi)
		{
			D3DXMatrixRotationZ(&mat, max(-ROTATION_SPEED * fDelta / 180.f * pi, -fAngleLower));
			m_DirectionLower = m_DirectionLower * mat;
		}

		if (fAngleLower < -5.f / 180.f * pi)
		{
			D3DXMatrixRotationZ(&mat, min(ROTATION_SPEED * fDelta / 180.f * pi, -fAngleLower));
			m_DirectionLower = m_DirectionLower * mat;
		}

		float fAngle = GetAngleOfVectors(m_TargetDir, m_DirectionLower);

		if (fAngle < -65.f / 180.f * pi)
		{
			fAngle = -65.f / 180.f * pi;
			D3DXMatrixRotationZ(&mat, -65.f / 180.f * pi);
			m_DirectionLower = m_Direction * mat;
		}

		if (fAngle >= 65.f / 180.f * pi)
		{
			fAngle = 65.f / 180.f * pi;
			D3DXMatrixRotationZ(&mat, 65.f / 180.f * pi);
			m_DirectionLower = m_Direction * mat;
		}

		m_pVMesh->m_vRotXYZ.x = -fAngle * 180 / pi * .9f;

		m_pVMesh->m_vRotXYZ.y = (m_TargetDir.z + 0.05f) * 50.f;
	}
	else
	{
		m_Direction = m_TargetDir;
		m_DirectionLower = m_Direction;

		m_pVMesh->m_vRotXYZ.x = 0.f;
		m_pVMesh->m_vRotXYZ.y = 0.f;
		m_pVMesh->m_vRotXYZ.z = 0.f;
	}
}

void GetDTM(bool* pDTM, int mode, bool isman)
{
	if (!pDTM) return;

	if (mode == 0) { pDTM[0] = true; pDTM[1] = false; }
	else if (mode == 1) { pDTM[0] = false; pDTM[1] = true; }
	else if (mode == 2) { pDTM[0] = false; pDTM[1] = false; }
	else if (mode == 3) { pDTM[0] = true; pDTM[1] = true; }
}

void ZCharacter::CheckDrawWeaponTrack()
{
	if (m_pVMesh == NULL) return;

	bool bDrawTracks = false;

	if ((m_pVMesh->m_SelectWeaponMotionType == eq_wd_katana) ||
		(m_pVMesh->m_SelectWeaponMotionType == eq_wd_sword) ||
		(m_pVMesh->m_SelectWeaponMotionType == eq_wd_blade))
	{
		ZC_STATE_LOWER aniState_Lower = m_AniState_Lower.Ref();
		ZC_STATE_UPPER aniState_Upper = m_AniState_Upper.Ref();

		if ((ZC_STATE_LOWER_ATTACK1 <= aniState_Lower && aniState_Lower <= ZC_STATE_LOWER_GUARD_CANCEL) ||
			(ZC_STATE_UPPER_LOAD <= aniState_Upper && aniState_Upper <= ZC_STATE_UPPER_GUARD_CANCEL))
		{
			if (aniState_Upper != ZC_STATE_UPPER_GUARD_IDLE) {
				if ((aniState_Lower != ZC_STATE_LOWER_ATTACK1_RET) &&
					(aniState_Lower != ZC_STATE_LOWER_ATTACK2_RET) &&
					(aniState_Lower != ZC_STATE_LOWER_ATTACK3_RET) &&
					(aniState_Lower != ZC_STATE_LOWER_ATTACK4_RET))
					bDrawTracks = true;
			}
		}
	}

	bool bDTM[2];

	bDTM[0] = true;
	bDTM[1] = true;

	bool bMan = IsMan();

	if (m_pVMesh->m_SelectWeaponMotionType == eq_wd_blade)
	{
		if (m_AniState_Lower.Ref() == ZC_STATE_LOWER_ATTACK1) GetDTM(bDTM, 0, bMan);
		else if (m_AniState_Lower.Ref() == ZC_STATE_LOWER_ATTACK2) GetDTM(bDTM, 1, bMan);
		else if (m_AniState_Lower.Ref() == ZC_STATE_LOWER_ATTACK3) GetDTM(bDTM, 2, bMan);
		else if (m_AniState_Lower.Ref() == ZC_STATE_LOWER_ATTACK4) GetDTM(bDTM, 3, bMan);
	}

	m_pVMesh->m_bDrawTracksMotion[0] = bDTM[0];
	m_pVMesh->m_bDrawTracksMotion[1] = bDTM[1];

	m_pVMesh->SetDrawTracks(bDrawTracks);
}

void ZCharacter::UpdateSpWeapon()
{
	if (!m_pVMesh) return;

	m_pVMesh->UpdateSpWeaponFire();

	if (m_pVMesh->m_bAddGrenade) {
		rvector vWeapon[2];

		vWeapon[0] = m_pVMesh->GetCurrentWeaponPosition();

		rvector nPos = m_pVMesh->GetBipTypePosition(eq_parts_pos_info_Spine1);
		rvector nDir = vWeapon[0] - nPos;

		Normalize(nDir);

		RBSPPICKINFO bpi;
		if (ZGetGame()->GetWorld()->GetBsp()->Pick(nPos, nDir, &bpi))
		{
			if (D3DXPlaneDotCoord(&(bpi.pInfo->plane), &vWeapon[0]) < 0) {
				vWeapon[0] = bpi.PickPos - nDir;
			}
		}

		vWeapon[1] = m_TargetDir;
		vWeapon[1].z += 0.1f;
		Normalize(vWeapon[1]);

		if (m_UID == ZGetGame()->m_pMyCharacter->m_UID) {
			int type = ZC_WEAPON_SP_GRENADE;

			RVisualMesh* pWVMesh = m_pVMesh->GetSelectWeaponVMesh();

			if (pWVMesh) {
				if (pWVMesh->m_pMesh) {
					if (strncmp(pWVMesh->m_pMesh->GetName(), "flashbang", 9) == 0) {
						type = ZC_WEAPON_SP_FLASHBANG;
					}
					else if (strncmp(pWVMesh->m_pMesh->GetName(), "smoke", 5) == 0) {
						type = ZC_WEAPON_SP_SMOKE;
					}
					else if (strncmp(pWVMesh->m_pMesh->GetName(), "tear_gas", 8) == 0) {
						type = ZC_WEAPON_SP_TEAR_GAS;
					}
					else if (strncmp(pWVMesh->m_pMesh->GetName(), "trap", 4) == 0) {
						type = ZC_WEAPON_SP_TRAP;
					}
					else if (strncmp(pWVMesh->m_pMesh->GetName(), "dynamite", 8) == 0) {
						type = ZC_WEAPON_SP_DYNAMITE;
					}
				}
			}

			int sel_type = GetItems()->GetSelectedWeaponParts();

			ZPostShotSp(vWeapon[0], vWeapon[1], type, sel_type);
			m_pVMesh->m_bAddGrenade = false;
		}
	}
}

bool ZCharacter::IsMan()
{
	if (m_pVMesh) {
		if (m_pVMesh->m_pMesh) {
			if (strcmp(m_pVMesh->m_pMesh->GetName(), "heroman1") == 0) {
				return true;
			}
		}
	}
	return false;
}

void ZCharacter::OnDraw()
{
	m_bRendered = false;

	if (m_bInitialized == false) return;
	if (!IsVisible()) return;
	if (IsAdminHide()) return;

	auto ZCharacterDraw = MBeginProfile("ZCharacter::Draw");

	if (m_pVMesh && !Enable_Cloth)	m_pVMesh->DestroyCloth();

	if (m_nVMID.Ref() == -1)
		return;

	rboundingbox bb;
	bb.vmax = GetPosition() + rvector(50, 50, 190);
	bb.vmin = GetPosition() - rvector(50, 50, 0);
	if (!ZGetGame()->GetWorld()->GetBsp()->IsVisible(bb)) return;
	if (!isInViewFrustum(&bb, RGetViewFrustum())) return;

	auto ZCharacterDrawLight = MBeginProfile("ZCharacter::Draw::Light");
	Draw_SetLight(GetPosition());
	MEndProfile(ZCharacterDrawLight);

	if (ZGetGame()->m_bShowWireframe)
	{
		RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
		RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	}

	const ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	bool bNarakSetState = ((uStatus.m_bFallingToNarak) && ((ZGetGame()->GetWorld()->IsFogVisible())));
	if (bNarakSetState)
	{
		RGetDevice()->SetRenderState(D3DRS_FOGENABLE, FALSE);
		RSetWBuffer(false);
	}

	CheckDrawWeaponTrack();

	RGetDevice()->SetRenderState(D3DRS_LIGHTING, TRUE);

	if (IsDie())
	{
#define TRAN_AFTER		3.f
#define VANISH_TIME		2.f

		float fOpacity = max(0.f, min(1.0f, (
			VANISH_TIME - (ZGetGame()->GetTime() - GetDeadTime() - TRAN_AFTER)) / VANISH_TIME));
		m_pVMesh->SetVisibility(fOpacity);
	}
	else
		if (!m_bHero) m_pVMesh->SetVisibility(1.f);

	auto ZCharacterDrawVisualMeshRender = MBeginProfile("ZCharacter::Draw::VisualMesh::Render");

	UpdateEnchant();

	bool bGame = ZGetGame() ? true : false;

	rvector cpos = ZApplication::GetGameInterface()->GetCamera()->GetPosition();
	cpos = GetPosition() - cpos;
	float dist = Magnitude(cpos);

	m_pVMesh->SetClothValue(bGame, fabs(dist));
	m_pVMesh->Render(uStatus.m_bIsLowModel);
	m_bRendered = m_pVMesh->m_bIsRender;

	if (m_pVMesh->m_bIsRenderWeapon && (m_pVMesh->GetVisibility() > 0.05f))
	{
		DrawEnchant(m_AniState_Lower.Ref(), m_bCharged->Ref());
	}

	if (bNarakSetState)
		RSetWBuffer(true);

	MEndProfile(ZCharacterDrawVisualMeshRender);

	MEndProfile(ZCharacterDraw);
}

bool ZCharacter::CheckDrawGrenade()
{
	if (m_Items.GetSelectedWeapon() == NULL) return false;

	if (m_pVMesh) {
		if (m_pVMesh->m_SelectWeaponMotionType == eq_wd_grenade) {
			if (m_Items.GetSelectedWeapon()->GetBulletCurrMagazine()) {
				return true;
			}
		}
	}
	return false;
}

bool ZCharacter::Pick(const rvector& pos, const rvector& dir, RPickInfo* pInfo)
{
	if (m_bInitialized == false) return false;

	return ZObject::Pick(pos, dir, pInfo);
}

#define GRAVITY_CONSTANT	2500.f
#define DAMAGE_VELOCITY		1700.f
#define MAX_FALL_SPEED		3000.f
#define MAX_FALL_DAMAGE		50.f
#define BLASTED_KNOCKBACK_RATIO	3.f

void ZCharacter::UpdateHeight(float fDelta)
{
	ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	if (uStatus.m_bFallingToNarak) return;

	uStatus.m_bJumpUp = (GetVelocity().z > 0);

	if (GetVelocity().z < 0 && GetDistToFloor()>35.f)
	{
		if (!uStatus.m_bJumpDown) {
			uStatus.m_bJumpDown = true;
			uStatus.m_bJumpUp = false;
		}
	}

	if (!uStatus.m_bWallJump)
	{
		if (m_pModule_Movable->isLanding())
		{
			if (GetPosition().z + 100.f < m_pModule_Movable->GetFallHeight())
			{
				float fSpeed = fabs(GetVelocity().z);
				if (fSpeed > DAMAGE_VELOCITY)
				{
					float fDamage = MAX_FALL_DAMAGE * (fSpeed - DAMAGE_VELOCITY) / (MAX_FALL_SPEED - DAMAGE_VELOCITY);
#ifdef	ENABLE_FALLING_DAMAGE
#endif
				}

				RBspObject* r_map = ZGetGame()->GetWorld()->GetBsp();

				rvector vPos = GetPosition();
				rvector vDir = rvector(0.f, 0.f, -1.f);
				vPos.z += 50.f;

				RBSPPICKINFO pInfo;

				if (r_map->Pick(vPos, vDir, &pInfo)) {
					vPos = pInfo.PickPos;

					vDir.x = pInfo.pInfo->plane.a;
					vDir.y = pInfo.pInfo->plane.b;
					vDir.z = pInfo.pInfo->plane.c;

					ZGetEffectManager()->AddLandingEffect(vPos, vDir);

					AniFrameInfo* pInfo = m_pVMesh->GetFrameInfo(ani_mode_lower);
					RAniSoundInfo* pSInfo = &pInfo->m_SoundInfo;

					if (pSInfo->Name[0]) {
						pSInfo->isPlay = true;
						UpdateSound();
					}
					else {
						strcpy(pSInfo->Name, "man_jump");
						pSInfo->isPlay = true;
						UpdateSound();
					}
				}
			}
		}
	}

	return;
}

int ZCharacter::GetSelectWeaponDelay(MMatchItemDesc* pSelectItemDesc)
{
	if (!pSelectItemDesc) return 0;

	int nReturnDelay = pSelectItemDesc->m_nDelay.Ref();

	if (pSelectItemDesc->m_nType.Ref() != MMIT_MELEE)
		return 0;

	switch (pSelectItemDesc->m_nWeaponType.Ref())
	{
	case MWT_DAGGER:
		nReturnDelay -= 266;
		break;

	case MWT_DUAL_DAGGER:
		nReturnDelay -= 299;
		break;

	case MWT_KATANA:
		nReturnDelay -= 299;
		break;

	case MWT_DOUBLE_KATANA:
		nReturnDelay -= 299;
		break;

	case MWT_GREAT_SWORD:
		nReturnDelay -= 399;
		break;

	default:
		break;
	}

	return nReturnDelay;
}
void ZCharacter::UpdateSpeed()
{
	if (m_pVMesh == NULL) return;

	float speed = 4.8f;
	float speed_upper = 4.8f;

	if (GetItems() && GetItems()->GetSelectedWeapon() && GetItems()->GetSelectedWeapon()->GetDesc())
	{
		if (GetItems()->GetSelectedWeapon()->GetDesc()->m_nType.Ref() == MMIT_MELEE)
		{
			ZC_STATE_LOWER aniState_Lower = m_AniState_Lower.Ref();
			if ((aniState_Lower == ZC_STATE_LOWER_ATTACK1) || (aniState_Lower == ZC_STATE_LOWER_ATTACK1_RET) ||
				(aniState_Lower == ZC_STATE_LOWER_ATTACK2) || (aniState_Lower == ZC_STATE_LOWER_ATTACK2_RET) ||
				(aniState_Lower == ZC_STATE_LOWER_ATTACK3) || (aniState_Lower == ZC_STATE_LOWER_ATTACK3_RET) ||
				(aniState_Lower == ZC_STATE_LOWER_ATTACK4) || (aniState_Lower == ZC_STATE_LOWER_ATTACK4_RET) ||
				(aniState_Lower == ZC_STATE_LOWER_ATTACK5) || (aniState_Lower == ZC_STATE_LOWER_JUMPATTACK) ||
				(m_AniState_Upper.Ref() == ZC_STATE_UPPER_SHOT))
			{
				MMatchItemDesc* pRangeDesc = GetItems()->GetSelectedWeapon()->GetDesc();

				int  nWeaponDelay = GetSelectWeaponDelay(pRangeDesc);

				int max_frame = 0;

				max_frame = m_pVMesh->GetMaxFrame(ani_mode_upper);

				if (max_frame == 0)
					max_frame = m_pVMesh->GetMaxFrame(ani_mode_lower);

				if (max_frame) {
					int _time = (int)(max_frame / 4.8f);

					int as = _time + nWeaponDelay;

					if (as < 1)	as = 1;

					float fas = 0.f;

					fas = (_time / (float)(as));

					m_fAttack1Ratio.Set_CheckCrc(fas);

					speed = 4.8f * m_fAttack1Ratio.Ref();
				}

				if (speed < 0.1f)
					speed = 0.1f;
			}
		}
	}

	if (m_AniState_Upper.Ref() == ZC_STATE_UPPER_LOAD)
	{
		speed = 4.8f * 1.2f;
		speed_upper = 4.8f * 1.2f;
	}

	m_pVMesh->SetSpeed(speed, speed);
}

void ZCharacter::OnUpdate(float fDelta)
{
	if (m_bInitialized == false) return;
	if (!IsVisible()) return;

	UpdateSpeed();

	if (m_pVMesh) {
		m_pVMesh->SetVisibility(1.f);
		m_pVMesh->Frame();
	}

	UpdateSound();
	UpdateMotion(fDelta);

	if (m_pVMesh && Enable_Cloth && m_pVMesh->isChestClothMesh())
	{
		if (IsDie())
		{
			rvector force = rvector(0, 0, -150);
			m_pVMesh->UpdateForce(force);
			m_pVMesh->SetClothState(CHARACTER_DIE_STATE);
		}
		else
		{
			rvector force = -GetVelocity() * 0.15;
			force.z += -90;
			m_pVMesh->UpdateForce(force);
		}
	}

	rvector vRot(0.0f, 0.0f, 0.0f);
	rvector vProxyPosition(0.0f, 0.0f, 0.0f);
	rvector vProxyDirection(0.0f, 0.0f, 0.0f);

	ZObserver* pObserver = ZGetCombatInterface()->GetObserver();
	if (pObserver->IsVisible())
	{
		rvector _vDir;
		if (!GetHistory(&vProxyPosition, &_vDir, ZGetGame()->GetTime() - pObserver->GetDelay()))
			return;

		vProxyDirection = m_DirectionLower;

		float fAngle = GetAngleOfVectors(_vDir, vProxyDirection);

		vRot.x = -fAngle * 180 / pi * .9f;
		vRot.y = (_vDir.z + 0.05f) * 50.f;
		vRot.z = 0.f;

		m_pVMesh->m_vRotXYZ = vRot;
	}
	else
	{
		vProxyPosition = GetPosition();
		vProxyDirection = m_DirectionLower;
	}

	if (IsDie()) {
		vProxyDirection = m_Direction;
	}

	vProxyDirection.z = 0;
	Normalize(vProxyDirection);

	if (m_nVMID.Ref() == -1) return;

	D3DXMATRIX world;
	MakeWorldMatrix(&world, rvector(0, 0, 0), vProxyDirection, rvector(0, 0, 1));

	rvector MeshPosition;

	if (IsMoveAnimation())
	{
		rvector footposition = m_pVMesh->GetFootPosition();

		rvector RealPosition = footposition * world;

		if (m_AniState_Lower.Ref() == ZC_STATE_LOWER_RUN_WALL)
		{
			rvector headpos = rvector(0.f, 0.f, 0.f);

			if (m_pVMesh) {
				AniFrameInfo* pAniLow = m_pVMesh->GetFrameInfo(ani_mode_lower);
				AniFrameInfo* pAniUp = m_pVMesh->GetFrameInfo(ani_mode_upper);
				m_pVMesh->m_pMesh->SetAnimation(pAniLow->m_pAniSet, pAniUp->m_pAniSet);
				m_pVMesh->m_pMesh->SetFrame(pAniLow->m_nFrame, pAniUp->m_nFrame);
				m_pVMesh->m_pMesh->SetMeshVis(m_pVMesh->m_fVis);
				m_pVMesh->m_pMesh->SetVisualMesh(m_pVMesh);

				m_pVMesh->m_pMesh->RenderFrame();

				RMeshNode* pNode = NULL;

				pNode = m_pVMesh->m_pMesh->FindNode(eq_parts_pos_info_Head);

				if (pNode) {
					headpos.x = pNode->m_mat_result._41;
					headpos.y = pNode->m_mat_result._42;
					headpos.z = pNode->m_mat_result._43;
				}
			}
			rvector rootpos = 0.5f * (footposition + headpos) * world;

			MeshPosition = vProxyPosition + rvector(0, 0, 90) - rootpos;
		}
		else
			MeshPosition = vProxyPosition - RealPosition;

		m_AnimationPositionDiff.Set_CheckCrc(footposition - m_RealPositionBefore.Ref());

		m_AnimationPositionDiff.Set_CheckCrc(m_AnimationPositionDiff.Ref() * world);

		m_RealPositionBefore.Set_CheckCrc(footposition);
	}
	else
		MeshPosition = vProxyPosition;

	ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	MakeWorldMatrix(&world, MeshPosition, vProxyDirection, rvector(0, 0, 1));
	m_pVMesh->SetWorldMatrix(world);

	rvector cpos = ZApplication::GetGameInterface()->GetCamera()->GetPosition();
	cpos = vProxyPosition - cpos;
	float dist = Magnitude(cpos);

	uStatus.m_bIsLowModel = (fabs(dist) > 3000.f);
	if (uStatus.m_bFallingToNarak) uStatus.m_bIsLowModel = false;

	uStatus.m_bDamaged = false;

	CheckLostConn();

	if (m_bCharging->Ref() && (m_AniState_Lower.Ref() != ZC_STATE_CHARGE && m_AniState_Lower.Ref() != ZC_STATE_LOWER_ATTACK1)) {
		m_bCharging->Set_CheckCrc(false);
	}

	if (m_bCharged->Ref() && ZGetGame()->GetTime() > m_fChargedFreeTime.Ref()) {
		m_bCharged->Set_CheckCrc(false);
	}

	UpdateSpWeapon();
}

void ZCharacter::CheckLostConn()
{
	ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	if (ZGetGame()->GetTime() - m_fLastReceivedTime > 1.f)
	{
		if (!uStatus.m_bLostConEffect)
		{
			uStatus.m_bLostConEffect = true;
			ZGetEffectManager()->AddLostConIcon(this);
		}
		SetVelocity(rvector(0, 0, 0));
	}
	else
		uStatus.m_bLostConEffect = false;
}

float ZCharacter::GetMoveSpeedRatio()
{
	float fRatio = 1.f;

	MMatchItemDesc* pMItemDesc = GetSelectItemDesc();

	if (pMItemDesc)
	{
		if (pMItemDesc->m_nLimitSpeed.Ref() <= 100)
			fRatio = pMItemDesc->m_nLimitSpeed.Ref() / 100.f;
		else
			fRatio = 0.1f;
	}

	return m_pModule_Movable->GetMoveSpeedRatio() * fRatio;
}

void ZCharacter::UpdateVelocity(float fDelta)
{
	rvector dir = rvector(GetVelocity().x, GetVelocity().y, 0);
	float fSpeed = Magnitude(dir);
	Normalize(dir);

	float fRatio = GetMoveSpeedRatio();

	float max_speed = MAX_SPEED * fRatio;

	if (fSpeed > max_speed) fSpeed = max_speed;

	const ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	bool bTumble = !IsDie() && (uStatus.m_bTumble ||
		(ZC_STATE_LOWER_TUMBLE_FORWARD <= m_AniState_Lower.Ref() && m_AniState_Lower.Ref() <= ZC_STATE_LOWER_TUMBLE_LEFT));

	if (uStatus.m_bLand && !uStatus.m_bWallJump && !bTumble)
	{
		rvector forward = m_TargetDir;
		forward.z = 0;
		Normalize(forward);

		float run_speed = RUN_SPEED * fRatio;
		float back_speed = BACK_SPEED * fRatio;
		float stop_formax_speed = STOP_FORMAX_SPEED * (1 / fRatio);

		if (DotProduct(forward, dir) > cosf(10.f * pi / 180.f))
		{
			if (fSpeed > run_speed)
				fSpeed = max(fSpeed - stop_formax_speed * fDelta, run_speed);
		}
		else
		{
			if (fSpeed > back_speed)
				fSpeed = max(fSpeed - stop_formax_speed * fDelta, back_speed);
		}
	}

	if (IS_ZERO(Magnitude(m_Accel.Ref())) && (uStatus.m_bLand && !uStatus.m_bWallJump && !uStatus.m_bWallJump2 && !bTumble))
	{
		if (uStatus.m_bBlast && m_nBlastType.Ref() == 1) {
		}
		else {
			fSpeed = max(fSpeed - STOP_SPEED * fDelta, 0);
		}
	}

	SetVelocity(dir.x * fSpeed, dir.y * fSpeed, GetVelocity().z);
}

void ZCharacter::UpdateAnimation()
{
	if (m_bInitialized == false) return;
	SetAnimationLower(ZC_STATE_LOWER_IDLE1);
}

#define CHARACTER_COLLISION_DIST	70.f
#define ST_MAX_WEAPON 200
#define ST_MAX_PARTS  200

struct WeaponST {
	int		id;
	char* name;
	RWeaponMotionType weapontype;
};

WeaponST g_WeaponST[ST_MAX_WEAPON] = {
	{ 0,"pistol01",	eq_wd_katana },
	{ 1,"pistol02",	eq_wd_katana },
	{ 2,"katana01",	eq_wd_katana },
	{ 3,"rifle01",	eq_wd_rifle  },
};

void ZCharacter::OnSetSlot(int nSlot, int WeaponID)
{
}

void ZCharacter::SetTargetDir(rvector vTarget) {
	Normalize(vTarget);
	m_TargetDir = vTarget;
}

void ZCharacter::OnChangeSlot(int nSlot)
{
	if (m_bInitialized == false) return;
	if (nSlot < 0 || nSlot > ZC_SLOT_END - 1)
		return;

	if (m_pVMesh) {
		MEMBER_SET_CHECKCRC(m_slotInfo, m_nSelectSlot, nSlot);

		int nWeaponID = m_slotInfo.Ref().m_Slot[nSlot].m_WeaponID;

		int SelModelID = nWeaponID;
		int SelToggleID = nWeaponID;

		RWeaponMotionType type = eq_weapon_etc;

		if (nWeaponID != -1)
			type = g_WeaponST[nWeaponID].weapontype;

		RMesh* pMesh = ZGetWeaponMeshMgr()->GetFast(SelModelID);

		if (pMesh) {
			if (SelModelID == -1) {
				m_pVMesh->RemoveWeapon(type);
			}
			else {
				CheckTeenVersionMesh(&pMesh);
				m_pVMesh->AddWeapon(type, pMesh);
				m_pVMesh->SelectWeaponMotion(type);
			}
		}
	}
}

void ZCharacter::OnChangeWeapon(char* WeaponModelName)
{
	if (m_bInitialized == false)
		return;

	if (m_pVMesh) {
		RWeaponMotionType type = eq_weapon_etc;

		RMesh* pMesh = ZGetWeaponMeshMgr()->Get(WeaponModelName);

		if (pMesh) {
			type = pMesh->GetMeshWeaponMotionType();

			CheckTeenVersionMesh(&pMesh);

			m_pVMesh->AddWeapon(type, pMesh);
			m_pVMesh->SelectWeaponMotion(type);
			UpdateLoadAnimation();
		}

		if (eq_wd_katana == type)
		{
#ifdef _BIRDSOUND
			ZGetSoundEngine()->PlaySoundCharacter("fx_blade_sheath", GetPosition(), IsObserverTarget());
#else
			ZGetSoundEngine()->PlaySoundSheath(m_Items.GetSelectedWeapon()->GetDesc(), GetPosition(), IsObserverTarget());
#endif
		}
		else if ((eq_wd_dagger == type) || (eq_ws_dagger == type))
		{
#ifdef _BIRDSOUND
			ZGetSoundEngine()->PlaySoundCharacter("fx_dagger_sheath", GetPosition(), IsObserverTarget());
#else
			ZGetSoundEngine()->PlaySound("fx_dagger_sheath", GetPosition(), IsObserverTarget());
#endif
		}
		else if (eq_wd_sword == type)
		{
#ifdef _BIRDSOUND
			ZGetSoundEngine()->PlaySoundCharacter("fx_dagger_sheath", GetPosition(), IsObserverTarget());
#else
			ZGetSoundEngine()->PlaySound("fx_dagger_sheath", GetPosition(), IsObserverTarget());
#endif
		}
		else if (eq_wd_blade == type)
		{
#ifdef _BIRDSOUND
			ZGetSoundEngine()->PlaySoundCharacter("fx_dagger_sheath", GetPosition(), IsObserverTarget());
#else
			ZGetSoundEngine()->PlaySound("fx_dagger_sheath", GetPosition(), IsObserverTarget());
#endif
		}
	}
}

char* GetPartsNextName(RMeshPartsType ptype, RVisualMesh* pVMesh, bool bReverse)
{
	static bool bFirst = true;
	static vector<RMeshNode*> g_table[6 * 2];
	static int g_parts[6 * 2];

	if (bFirst) {
		RMesh* pMesh = ZGetMeshMgr()->Get("heroman1");

		if (pMesh) {
			pMesh->GetPartsNode(eq_parts_chest, g_table[0]);
			pMesh->GetPartsNode(eq_parts_head, g_table[1]);
			pMesh->GetPartsNode(eq_parts_hands, g_table[2]);
			pMesh->GetPartsNode(eq_parts_legs, g_table[3]);
			pMesh->GetPartsNode(eq_parts_feet, g_table[4]);
			pMesh->GetPartsNode(eq_parts_face, g_table[5]);
		}

		pMesh = ZGetMeshMgr()->Get("herowoman1");

		if (pMesh) {
			pMesh->GetPartsNode(eq_parts_chest, g_table[6]);
			pMesh->GetPartsNode(eq_parts_head, g_table[7]);
			pMesh->GetPartsNode(eq_parts_hands, g_table[8]);
			pMesh->GetPartsNode(eq_parts_legs, g_table[9]);
			pMesh->GetPartsNode(eq_parts_feet, g_table[10]);
			pMesh->GetPartsNode(eq_parts_face, g_table[11]);
		}

		bFirst = false;
	}

	int mode = 0;

	if (ptype == eq_parts_chest)	mode = 0;
	else if (ptype == eq_parts_head)	mode = 1;
	else if (ptype == eq_parts_hands)	mode = 2;
	else if (ptype == eq_parts_legs)	mode = 3;
	else if (ptype == eq_parts_feet)	mode = 4;
	else if (ptype == eq_parts_face)	mode = 5;
	else return NULL;

	if (pVMesh) {
		if (pVMesh->m_pMesh) {
			if (strcmp(pVMesh->m_pMesh->GetName(), "heroman1") != 0) {
				mode += 6;
			}
		}
	}

	if (bReverse) {
		g_parts[mode]--;

		if (g_parts[mode] < 0) {
			g_parts[mode] = (int)g_table[mode].size() - 1;
		}
	}
	else {
		g_parts[mode]++;

		if (g_parts[mode] > (int)g_table[mode].size() - 1) {
			g_parts[mode] = 0;
		}
	}

	return g_table[mode][g_parts[mode]]->GetName();
}

void ZCharacter::OnChangeParts(RMeshPartsType partstype, int PartsID)
{
#ifndef _PUBLISH
	if (m_bInitialized == false) return;
	if (m_pVMesh) {
		if (partstype > eq_parts_etc&& partstype < eq_parts_left_pistol) {
			if (PartsID == 0) {
				m_pVMesh->SetBaseParts(partstype);
			}
			else {
				char* Name = NULL;

				if (MEvent::GetCtrlState()) {
					Name = GetPartsNextName(partstype, m_pVMesh, true);
				}
				else {
					Name = GetPartsNextName(partstype, m_pVMesh, false);
				}

				if (Name)
					m_pVMesh->SetParts(partstype, Name);
			}
		}
	}

	if (Enable_Cloth)
		m_pVMesh->ChangeChestCloth(1.f, 1);
#endif
}

void ZCharacter::OnAttack(int type, rvector& pos)
{
}

float ZCharacter::GetMaxHP()
{
	return m_Property.fMaxHP.Ref();
}

float ZCharacter::GetMaxAP()
{
	return m_Property.fMaxAP.Ref();
}

float ZCharacter::GetHP()
{
	return m_pModule_HPAP->GetHP();
}

float ZCharacter::GetAP()
{
	return m_pModule_HPAP->GetAP();
}

void ZCharacter::InitAccumulationDamage()
{
	m_pModule_HPAP->InitAccumulationDamage();
}
float ZCharacter::GetAccumulationDamage() {
	return m_pModule_HPAP->GetAccumulationDamage();
}

void ZCharacter::EnableAccumulationDamage(bool bAccumulationDamage) {
	m_pModule_HPAP->EnableAccumulationDamage(bAccumulationDamage);
}

void ZCharacter::Die()
{
	OnDie();
}

void ZCharacter::OnDie()
{
	if (m_bInitialized == false)
		return;

	if (!IsVisible())
		return;

	ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	uStatus.m_bDie = true;
	m_Collision.SetCollideable(false);
	uStatus.m_bPlayDone = false;

	if (uStatus.m_bFallingToNarak == true)
	{
		if (m_Property.nSex == MMS_MALE)
			ZGetSoundEngine()->PlaySound("fx2/MAL10", GetPosition(), IsObserverTarget());
		else
			ZGetSoundEngine()->PlaySound("fx2/FEM10", GetPosition(), IsObserverTarget());
	}
	else
	{
		if (m_Property.nSex == MMS_MALE)
			ZGetSoundEngine()->PlaySound("fx2/MAL08", GetPosition(), IsObserverTarget());
		else
			ZGetSoundEngine()->PlaySound("fx2/FEM08", GetPosition(), IsObserverTarget());
	}
}

void ZCharacter::Revival()
{
	if (m_bInitialized == false) return;
	InitStatus();

	ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	uStatus.m_bDie = false;
	m_Collision.SetCollideable(true);

	if (IsAdminHide())
		uStatus.m_bDie = true;

	SetAnimationLower(ZC_STATE_LOWER_IDLE1);
}

void ZCharacter::SetDirection(const rvector& dir)
{
	m_Direction = dir;
	m_DirectionLower = dir;
	m_DirectionUpper = dir;
	m_TargetDir = dir;
}

void ZCharacter::OnKnockback(const rvector& dir, float fForce)
{
	if (IsHero())
		ZCharacterObject::OnKnockback(dir, fForce);
}

void ZCharacter::UpdateSound()
{
	if (m_bInitialized == false) return;
	if (m_pVMesh) {
		char szSndName[128];
		RMATERIAL* pMaterial = NULL;
		RBSPPICKINFO bpi;
		if (ZGetGame()->GetWorld()->GetBsp()->Pick(GetPosition() + rvector(0, 0, 100), rvector(0, 0, -1), &bpi)) {
			pMaterial = ZGetGame()->GetWorld()->GetBsp()->GetMaterial(bpi.pNode, bpi.nIndex);
		}

		AniFrameInfo* pInfo = m_pVMesh->GetFrameInfo(ani_mode_lower);

		int nFrame = pInfo->m_nFrame;

		int nCurrFoot = 0;

#define FRAME(x) int(float(x)/30.f*4800.f)
		if (m_AniState_Lower.Ref() == ZC_STATE_LOWER_RUN_FORWARD ||
			m_AniState_Lower.Ref() == ZC_STATE_LOWER_RUN_BACK) {
			if (FRAME(8) < nFrame && nFrame < FRAME(18))
				nCurrFoot = 1;
		}

		if (m_AniState_Lower.Ref() == ZC_STATE_LOWER_RUN_WALL_LEFT ||
			m_AniState_Lower.Ref() == ZC_STATE_LOWER_RUN_WALL_RIGHT) {
			if (nFrame < FRAME(9)) nCurrFoot = 1;
			else if (nFrame < FRAME(17)) nCurrFoot = 0;
			else if (nFrame < FRAME(24)) nCurrFoot = 1;
			else if (nFrame < FRAME(32)) nCurrFoot = 0;
			else if (nFrame < FRAME(40)) nCurrFoot = 1;
			else if (nFrame < FRAME(48)) nCurrFoot = 0;
			else if (nFrame < FRAME(55)) nCurrFoot = 1;
		}

		if (m_AniState_Lower.Ref() == ZC_STATE_LOWER_RUN_WALL) {
			if (nFrame < FRAME(8)) nCurrFoot = 1;
			else if (nFrame < FRAME(16)) nCurrFoot = 0;
			else if (nFrame < FRAME(26)) nCurrFoot = 1;
			else if (nFrame < FRAME(40)) nCurrFoot = 0;
		}

		if (m_nWhichFootSound != nCurrFoot && pMaterial) {
			if (m_nWhichFootSound == 0)
			{
				rvector pos = m_pVMesh->GetLFootPosition();
				char* szSndName = ZGetGame()->GetSndNameFromBsp("man_fs_l", pMaterial);

#ifdef _BIRDSOUND
				ZApplication::GetSoundEngine()->PlaySoundCharacter(szSndName, pos, IsObserverTarget());
#else
				ZApplication::GetSoundEngine()->PlaySound(szSndName, pos, IsObserverTarget());
#endif
			}
			else
			{
				rvector pos = m_pVMesh->GetRFootPosition();
				char* szSndName = ZGetGame()->GetSndNameFromBsp("man_fs_r", pMaterial);
#ifdef _BIRDSOUND
				ZApplication::GetSoundEngine()->PlaySoundCharacter(szSndName, pos, IsObserverTarget());
#else
				ZApplication::GetSoundEngine()->PlaySound(szSndName, pos, IsObserverTarget());
#endif
			}
			m_nWhichFootSound = nCurrFoot;
		}

		RAniSoundInfo* pSInfo;
		RAniSoundInfo* pSInfoTable[2];

		rvector p;

		AniFrameInfo* pAniLow = m_pVMesh->GetFrameInfo(ani_mode_lower);
		AniFrameInfo* pAniUp = m_pVMesh->GetFrameInfo(ani_mode_upper);

		pSInfoTable[0] = &pAniLow->m_SoundInfo;
		pSInfoTable[1] = &pAniUp->m_SoundInfo;

		for (int i = 0; i < 2; i++) {
			pSInfo = pSInfoTable[i];

			if (pSInfo->isPlay)
			{
				p = pSInfo->Pos;

				if (pMaterial)
				{
					strcpy(szSndName, ZGetGame()->GetSndNameFromBsp(pSInfo->Name, pMaterial));

					int nStr = (int)strlen(szSndName);
					strncpy(m_pSoundMaterial, szSndName + (nStr - 6), 7);

					ZApplication::GetSoundEngine()->PlaySoundElseDefault(szSndName, pSInfo->Name, p, IsObserverTarget());
				}
				else {
					m_pSoundMaterial[0] = 0;

					strcpy(szSndName, pSInfo->Name);
#ifdef _BIRDSOUND
					ZApplication::GetSoundEngine()->PlaySoundCharacter(szSndName, p, IsObserverTarget());
#else
					ZApplication::GetSoundEngine()->PlaySound(szSndName, p, IsObserverTarget());
#endif
				}

				pSInfo->Clear();
			}
		}
	}

	ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();
	if (uStatus.m_bDamaged && (!IsDie()) && (GetHP() < 30.f))
	{
		DWORD currTime = timeGetTime();

		if ((m_damageInfo.Ref().m_dwLastDamagedTime + 5000) < currTime)
		{
			if (GetProperty()->nSex == MMS_MALE)
			{
				int nRetVal = ZGetSoundEngine()->PlaySound("fx2/MAL06", GetPosition(), IsObserverTarget());
				if (nRetVal == 0)
					ZGetSoundEngine()->PlaySound("ooh_male", GetPosition(), IsObserverTarget());
			}

			else
			{
				int nRetVal = ZGetSoundEngine()->PlaySound("fx2/FEM06", GetPosition(), IsObserverTarget());
				if (nRetVal == 0)
					ZGetSoundEngine()->PlaySound("ooh_female", GetPosition(), IsObserverTarget());
			}

			MEMBER_SET_CHECKCRC(m_damageInfo, m_dwLastDamagedTime, currTime);
		}

		uStatus.m_bDamaged = false;
	}
}

bool ZCharacter::DoingStylishMotion()
{
	if ((m_AniState_Lower.Ref() >= ZC_STATE_LOWER_RUN_WALL_LEFT) &&
		(m_AniState_Lower.Ref() <= ZC_STATE_LOWER_JUMP_WALL_BACK))
	{
		return true;
	}

	return false;
}

void ZCharacter::UpdateStylishShoted()
{
	ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	if (DoingStylishMotion())
	{
		uStatus.m_bStylishShoted = true;
	}
	else
	{
		uStatus.m_bStylishShoted = false;
	}
}

void ZCharacter::InitHPAP()
{
	m_pModule_HPAP->SetMaxHP(GetMaxHP());
	m_pModule_HPAP->SetMaxAP(GetMaxAP());

	m_pModule_HPAP->SetHP(GetMaxHP());
	m_pModule_HPAP->SetAP(GetMaxAP());
}

void ZCharacter::InitBullet()
{
	int nBulletSpare, nBulletCurrMagazine;
	if (!m_Items.GetItem(MMCIP_PRIMARY)->IsEmpty())
	{
		nBulletSpare = m_Items.GetItem(MMCIP_PRIMARY)->GetDesc()->m_nMaxBullet.Ref();
		nBulletCurrMagazine = m_Items.GetItem(MMCIP_PRIMARY)->GetDesc()->m_nMagazine.Ref();

		m_Items.GetItem(MMCIP_PRIMARY)->InitBullet(nBulletSpare, nBulletCurrMagazine);
	}
	if (!m_Items.GetItem(MMCIP_SECONDARY)->IsEmpty())
	{
		nBulletSpare = m_Items.GetItem(MMCIP_SECONDARY)->GetDesc()->m_nMaxBullet.Ref();
		nBulletCurrMagazine = m_Items.GetItem(MMCIP_SECONDARY)->GetDesc()->m_nMagazine.Ref();

		m_Items.GetItem(MMCIP_SECONDARY)->InitBullet(nBulletSpare, nBulletCurrMagazine);
	}

	for (int i = MMCIP_CUSTOM1; i < MMCIP_CUSTOM2 + 1; i++)
	{
		if (!m_Items.GetItem(MMatchCharItemParts(i))->IsEmpty())
		{
			MMatchItemDesc* pDesc = m_Items.GetItem(MMatchCharItemParts(i))->GetDesc();

			if (!pDesc->IsSpendableItem())
			{
				nBulletSpare = pDesc->m_nMaxBullet.Ref();
				nBulletCurrMagazine = pDesc->m_nMagazine.Ref();

				m_Items.GetItem(MMatchCharItemParts(i))->InitBullet(nBulletSpare, nBulletCurrMagazine);
			}
			else
			{
				nBulletCurrMagazine = pDesc->m_nMagazine.Ref();

				if (GetUID() != ZGetMyUID())
					nBulletSpare = MAX_SPENDABLE_ITEM_COUNT;
				else
				{
					nBulletSpare = 0;
					ZMyItemNode* pItemNode = ZGetMyInfo()->GetItemList()->GetEquipedItem(MMatchCharItemParts(i));
					if (pItemNode)
					{
						if (pItemNode->GetItemCount() >= nBulletCurrMagazine)
							nBulletSpare = pItemNode->GetItemCount() - nBulletCurrMagazine;
						else
						{
							nBulletCurrMagazine = pItemNode->GetItemCount();
							nBulletSpare = 0;
						}
					}
					else
					{
						nBulletCurrMagazine = 0;
						nBulletSpare = 0;
					}
				}

				m_Items.GetItem(MMatchCharItemParts(i))->InitBullet(nBulletSpare, nBulletCurrMagazine);
			}
		}
	}
}

void ZCharacter::InitStatus()
{
	InitHPAP();
	InitBullet();

	ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	uStatus.m_bCommander = false;
	uStatus.m_bDie = false;

	uStatus.m_bStylishShoted = false;
	uStatus.m_bStun = false;

	uStatus.m_bBlast = false;
	uStatus.m_bBlastFall = false;
	uStatus.m_bBlastDrop = false;
	uStatus.m_bBlastStand = false;
	uStatus.m_bBlastAirmove = false;

	uStatus.m_bSpMotion = false;
	uStatus.m_bLostConEffect = false;
	uStatus.m_bChatEffect = false;
	uStatus.m_bFallingToNarak = false;

	if (IsAdminHide()) {
		uStatus.m_bDie = true;
		SetVisible(false);
	}

	SetVelocity(0, 0, 0);
	m_Collision.SetCollideable(true);
	m_SpMotion = ZC_STATE_TAUNT;

	m_fLastReceivedTime = 0;
	m_killInfo.CheckCrc();
	m_killInfo.Ref().m_fLastKillTime = 0;
	m_killInfo.Ref().m_nKillsThisRound = 0;
	m_killInfo.MakeCrc();

	MEMBER_SET_CHECKCRC(m_damageInfo, m_LastDamageType, ZD_NONE);
	SetLastThrower(MUID(0, 0), 0.0f);

	EmptyHistory();

	if (m_pVMesh) {
		m_pVMesh->SetVisibility(1);
	}

	m_bCharged->Set_CheckCrc(false);
	m_bCharging->Set_CheckCrc(false);

#ifndef _PUBLISH
	char szLog[128];
	sprintf(szLog, "ZCharacter::InitStatus() - %s(%u) Initialized \n",
		GetProperty()->GetName(), m_UID.Low);
	OutputDebugString(szLog);
#endif

	InitModuleStatus();
}

void ZCharacter::TestChangePartsAll()
{
	if (IsMan()) {
		OnChangeParts(eq_parts_chest, 0);
		OnChangeParts(eq_parts_head, 0);
		OnChangeParts(eq_parts_hands, 0);
		OnChangeParts(eq_parts_legs, 0);
		OnChangeParts(eq_parts_feet, 0);
		OnChangeParts(eq_parts_face, 0);
	}
	else {
		OnChangeParts(eq_parts_chest, 0);
		OnChangeParts(eq_parts_head, 0);
		OnChangeParts(eq_parts_hands, 0);
		OnChangeParts(eq_parts_legs, 0);
		OnChangeParts(eq_parts_feet, 0);
		OnChangeParts(eq_parts_face, 0);
	}
}

#define AddText(s) { str.Add(#s,false); str.Add(" :",false); str.Add(s);}
#define AddTextEnum(s,e) {str.Add(#s,false); str.Add(" :",false); str.Add(#e);}

void ZCharacter::OutputDebugString_CharacterState()
{
	return;

	RDebugStr str;

	str.Add("//////////////////////////////////////////////////////////////");

	AddText(m_bInitialized);
	AddText(m_bHero);

	AddText(m_nVMID.Ref());

	AddText(m_UID.High);
	AddText(m_UID.Low);
	AddText(m_nTeamID.Ref());

	str.AddLine();

	str.Add("######  m_Property  #######\n");

	AddText(m_Property.GetName());
	AddText(m_Property.nSex);

	str.AddLine();

	str.Add("######  m_Status  #######\n");

	AddText(GetHP());
	AddText(GetAP());
	AddText(m_Status.Ref().nLife);
	AddText(m_Status.Ref().nKills);
	AddText(m_Status.Ref().nDeaths);
	AddText(m_Status.Ref().nLoadingPercent);
	AddText(m_Status.Ref().nCombo);
	AddText(m_Status.Ref().nMaxCombo);
	AddText(m_Status.Ref().nAllKill);
	AddText(m_Status.Ref().nExcellent);
	AddText(m_Status.Ref().nFantastic);
	AddText(m_Status.Ref().nHeadShot);
	AddText(m_Status.Ref().nUnbelievable);

	str.AddLine();

	str.Add("######  m_Items  #######\n");

	ZItem* pItem = m_Items.GetSelectedWeapon();

#define IF_SITEM_ENUM(a)		if(a==m_Items.GetSelectedWeaponType())		{ AddTextEnum(m_Items.GetSelectedWeaponType(),a); }
#define ELSE_IF_SITEM_ENUM(a)	else if(a==m_Items.GetSelectedWeaponType())	{ AddTextEnum(m_Items.GetSelectedWeaponType(),a); }

	IF_SITEM_ENUM(MMCIP_HEAD)
		ELSE_IF_SITEM_ENUM(MMCIP_CHEST)
		ELSE_IF_SITEM_ENUM(MMCIP_HANDS)
		ELSE_IF_SITEM_ENUM(MMCIP_LEGS)
		ELSE_IF_SITEM_ENUM(MMCIP_FEET)
		ELSE_IF_SITEM_ENUM(MMCIP_FINGERL)
		ELSE_IF_SITEM_ENUM(MMCIP_FINGERR)
		ELSE_IF_SITEM_ENUM(MMCIP_AVATAR)
		ELSE_IF_SITEM_ENUM(MMCIP_MELEE)
		ELSE_IF_SITEM_ENUM(MMCIP_PRIMARY)
		ELSE_IF_SITEM_ENUM(MMCIP_SECONDARY)
		ELSE_IF_SITEM_ENUM(MMCIP_CUSTOM1)
		ELSE_IF_SITEM_ENUM(MMCIP_CUSTOM2)
		ELSE_IF_SITEM_ENUM(MMCIP_COMMUNITY1)
		ELSE_IF_SITEM_ENUM(MMCIP_COMMUNITY2)
		ELSE_IF_SITEM_ENUM(MMCIP_LONGBUFF1)
		ELSE_IF_SITEM_ENUM(MMCIP_LONGBUFF2)

		ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	AddText(uStatus.m_bDie);
	AddText(uStatus.m_bStylishShoted);
	AddText(IsVisible());
	AddText(uStatus.m_bStun);
	AddText(m_damageInfo.Ref().m_nStunType);
	AddText(uStatus.m_bPlayDone);

	AddText(m_killInfo.Ref().m_nKillsThisRound);
	AddText(m_killInfo.Ref().m_fLastKillTime);

	str.AddLine(1);

#define IF_LD_ENUM(a)		if(a==m_damageInfo.Ref().m_LastDamageType)			{ AddTextEnum(m_damageInfo.Ref().m_LastDamageType,a); }
#define ELSE_IF_LD_ENUM(a)	else if(a==m_damageInfo.Ref().m_LastDamageType)		{ AddTextEnum(m_damageInfo.Ref().m_LastDamageType,a); }

	IF_LD_ENUM(ZD_NONE)
		ELSE_IF_LD_ENUM(ZD_BULLET)
		ELSE_IF_LD_ENUM(ZD_MELEE)
		ELSE_IF_LD_ENUM(ZD_FALLING)
		ELSE_IF_LD_ENUM(ZD_EXPLOSION)
		ELSE_IF_LD_ENUM(ZD_BULLET_HEADSHOT)
		ELSE_IF_LD_ENUM(ZD_KATANA_SPLASH)
		ELSE_IF_LD_ENUM(ZD_HEAL)
		ELSE_IF_LD_ENUM(ZD_REPAIR)
		AddText(m_damageInfo.Ref().m_LastDamageDir);
	AddText(GetSpawnTime());

	AddText(m_fLastValidTime);
	AddText(GetDistToFloor());
	AddText(uStatus.m_bLand);
	AddText(uStatus.m_bWallJump);
	AddText(m_nWallJumpDir.Ref());
	AddText(uStatus.m_bJumpUp);
	AddText(uStatus.m_bJumpDown);
	AddText(uStatus.m_bWallJump2);
	AddText(uStatus.m_bBlast);
	AddText(uStatus.m_bBlastFall);
	AddText(uStatus.m_bBlastDrop);
	AddText(uStatus.m_bBlastStand);
	AddText(uStatus.m_bBlastAirmove);
	AddText(uStatus.m_bSpMotion);
	AddText(m_bDynamicLight);
	AddText(m_iDLightType);
	AddText(m_fLightLife);
	AddText(m_vLightColor);
	AddText(m_fTime);
	AddText(m_bLeftShot);
	AddText(m_TargetDir);
	AddText(GetPosition());
	AddText(m_Direction);
	AddText(m_DirectionLower);
	AddText(m_DirectionUpper);
	AddText(m_RealPositionBefore.Ref());
	AddText(m_Accel.Ref());

	str.AddLine(1);

#define IF_Upper_ENUM(a)		if(a==m_AniState_Upper.Ref())		{ AddTextEnum(m_AniState_Upper.Ref(),a); }
#define ELSE_IF_Upper_ENUM(a)	else if(a==m_AniState_Upper.Ref())	{ AddTextEnum(m_AniState_Upper.Ref(),a); }

#define IF_Lower_ENUM(a)		if(a==m_AniState_Lower.Ref())		{ AddTextEnum(m_AniState_Lower.Ref(),a); }
#define ELSE_IF_Lower_ENUM(a)	else if(a==m_AniState_Lower.Ref())	{ AddTextEnum(m_AniState_Lower.Ref(),a); }

	IF_Upper_ENUM(ZC_STATE_UPPER_NONE)
		ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_SHOT)
		ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_RELOAD)
		ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_LOAD)
		ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_START)
		ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_IDLE)
		ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_BLOCK1)
		ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_BLOCK1_RET)
		ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_BLOCK2)
		ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_CANCEL)

		IF_Lower_ENUM(ZC_STATE_LOWER_NONE)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_IDLE1)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_IDLE2)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_IDLE3)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_IDLE4)

		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_FORWARD)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_BACK)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_LEFT)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_RIGHT)

		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_UP)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_DOWN)

		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_DIE1)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_DIE2)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_DIE3)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_DIE4)

		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL_LEFT)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL_LEFT_DOWN)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL_DOWN)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL_RIGHT)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL_RIGHT_DOWN)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_TUMBLE_FORWARD)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_TUMBLE_BACK)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_TUMBLE_RIGHT)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_TUMBLE_LEFT)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BIND)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_WALL_FORWARD)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_WALL_BACK)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_WALL_LEFT)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_WALL_RIGHT)

		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK1)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK1_RET)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK2)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK2_RET)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK3)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK3_RET)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK4)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK4_RET)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK5)

		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMPATTACK)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_UPPERCUT)

		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_START)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_IDLE)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_BLOCK1)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_BLOCK1_RET)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_BLOCK2)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_CANCEL)

		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BLAST)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BLAST_FALL)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BLAST_DROP)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BLAST_STAND)
		ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BLAST_AIRMOVE)

		ELSE_IF_Lower_ENUM(ZC_STATE_DAMAGE)
		ELSE_IF_Lower_ENUM(ZC_STATE_DAMAGE2)
		ELSE_IF_Lower_ENUM(ZC_STATE_DAMAGE_DOWN)

		ELSE_IF_Lower_ENUM(ZC_STATE_TAUNT)
		ELSE_IF_Lower_ENUM(ZC_STATE_BOW)
		ELSE_IF_Lower_ENUM(ZC_STATE_WAVE)
		ELSE_IF_Lower_ENUM(ZC_STATE_LAUGH)
		ELSE_IF_Lower_ENUM(ZC_STATE_CRY)
		ELSE_IF_Lower_ENUM(ZC_STATE_DANCE)

		if (m_pAnimationInfo_Upper) {
			AddText(m_pAnimationInfo_Upper->Name);
		}

	if (m_pAnimationInfo_Lower) {
		AddText(m_pAnimationInfo_Lower->Name);
	}

	str.AddLine(1);

	if (m_nMoveMode.Ref() == MCMM_WALK) { AddTextEnum(m_nMoveMode.Ref(), MCMM_WALK); }
	else if (m_nMoveMode.Ref() == MCMM_RUN) { AddTextEnum(m_nMoveMode.Ref(), MCMM_RUN); }

	if (m_nMode.Ref() == MCM_PEACE) { AddTextEnum(m_nMode.Ref(), MCM_PEACE); }
	else if (m_nMode.Ref() == MCM_OFFENSIVE) { AddTextEnum(m_nMode.Ref(), MCM_OFFENSIVE); }

	if (m_nState.Ref() == MCS_STAND) { AddTextEnum(m_nState.Ref(), MCS_STAND); }
	else if (m_nState.Ref() == MCS_SIT) { AddTextEnum(m_nState.Ref(), MCS_SIT); }
	else if (m_nState.Ref() == MCS_DEAD) { AddTextEnum(m_nState.Ref(), MCS_DEAD); }

	str.AddLine(1);

	AddText(uStatus.m_bBackMoving);
	AddText(m_fLastReceivedTime);
	AddText(m_fGlobalHP);
	AddText(m_nReceiveHPCount);

	AddText(m_slotInfo.Ref().m_nSelectSlot);

	str.PrintLog();

	if (m_pVMesh) {
		m_pVMesh->OutputDebugString_CharacterState();
	}
}

void ZCharacter::TestToggleCharacter()
{
	if (m_pVMesh->m_pMesh) {
		RMesh* pMesh = NULL;

		if (strcmp(m_pVMesh->m_pMesh->GetName(), "heroman1") == 0) {
			pMesh = ZGetMeshMgr()->Get("herowoman1");
			m_pVMesh->m_pMesh = pMesh;
			m_pVMesh->ClearParts();
			TestChangePartsAll();
		}
		else {
			pMesh = ZGetMeshMgr()->Get("heroman1");
			m_pVMesh->m_pMesh = pMesh;
			m_pVMesh->ClearParts();
			TestChangePartsAll();
		}
	}
}

void ZCharacter::InitMesh()
{
	RMesh* pMesh;

	char szMeshName[64];
	if (m_Property.nSex == MMS_MALE)
	{
		strcpy(szMeshName, "heroman1");
	}
	else
	{
		strcpy(szMeshName, "herowoman1");
	}
	pMesh = ZGetMeshMgr()->Get(szMeshName);

	if (!pMesh) {
		mlog("AddCharacter 원하는 모델을 찾을수 없음\n");
	}

	int nVMID = ZGetGame()->m_VisualMeshMgr.Add(pMesh);

	if (nVMID == -1) {
		mlog("AddCharacter 캐릭터 생성 실패\n");
	}

	m_nVMID.Set_CheckCrc(nVMID);

	RVisualMesh* pVMesh = ZGetGame()->m_VisualMeshMgr.GetFast(nVMID);
	SetVisualMesh(pVMesh);
}

void ZCharacter::ChangeLowPolyModel()
{
	if (m_pVMesh == NULL)
		return;

	char szMeshName[64];

	bool cloth_model = false;

	if (m_pVMesh) {
		cloth_model = m_pVMesh->IsClothModel();
	}

	if (cloth_model) {
		strcpy(szMeshName, "heroman_low1");
	}
	else {
		strcpy(szMeshName, "heroman_low2");
	}

	RMesh* pLowMesh = ZGetMeshMgr()->Get(szMeshName);
}

bool ZCharacter::IsAdminName()
{
	if (m_MInitialInfo.Ref().nUGradeID == MMUG_DEVELOPER ||
		m_MInitialInfo.Ref().nUGradeID == MMUG_ADMIN)
		return true;
	return false;
}

void ZCharacter::InitProperties()
{
	const MTD_CharInfo* pCharInfo = &m_MInitialInfo.Ref();

	m_Property.SetName(pCharInfo->szName);
	m_Property.SetClanName(pCharInfo->szClanName);
	m_Property.nSex = (MMatchSex)pCharInfo->nSex;
	m_Property.nHair = pCharInfo->nHair;
	m_Property.nFace = pCharInfo->nFace;
	m_Property.nLevel = pCharInfo->nLevel;

	float fAddedAP = DEFAULT_CHAR_AP;
	for (int i = 0; i < MMCIP_END; i++) {
		if (!m_Items.GetItem(MMatchCharItemParts(i))->IsEmpty()) {
			if (m_Items.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nAP.Ref() > 40) {
				m_Items.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nAP.Ref() = 0;
			}
			fAddedAP += m_Items.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nAP.Ref();
		}
	}

	float fAddedHP = DEFAULT_CHAR_HP;
	for (int i = 0; i < MMCIP_END; i++) {
		if (!m_Items.GetItem(MMatchCharItemParts(i))->IsEmpty()) {
			fAddedHP += m_Items.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nHP.Ref();
		}
	}

	m_Property.fMaxAP.Set_CheckCrc(pCharInfo->nAP + fAddedAP);
	m_Property.fMaxHP.Set_CheckCrc(pCharInfo->nHP + fAddedHP);

	m_fPreMaxHP = pCharInfo->nHP + fAddedHP;
	m_fPreMaxAP = pCharInfo->nAP + fAddedAP;

	if (GetUserGrade() == MMUG_DEVELOPER) {
		m_pMUserAndClanName->CheckCrc();
		strcpy(m_pMUserAndClanName->Ref().m_szUserName, ZMsg(MSG_WORD_DEVELOPER));
		strcpy(m_pMUserAndClanName->Ref().m_szUserAndClanName, ZMsg(MSG_WORD_DEVELOPER));
		m_pMUserAndClanName->MakeCrc();
	}
	else if (GetUserGrade() == MMUG_ADMIN) {
		m_pMUserAndClanName->CheckCrc();
		strcpy(m_pMUserAndClanName->Ref().m_szUserName, ZMsg(MSG_WORD_ADMIN));
		strcpy(m_pMUserAndClanName->Ref().m_szUserAndClanName, ZMsg(MSG_WORD_ADMIN));
		m_pMUserAndClanName->MakeCrc();
	}
	else {
		m_pMUserAndClanName->CheckCrc();
		strcpy(m_pMUserAndClanName->Ref().m_szUserName, m_Property.GetName());
		if (strlen(m_Property.GetClanName()) != 0)
			sprintf(m_pMUserAndClanName->Ref().m_szUserAndClanName, "%s(%s)", m_Property.GetName(), m_Property.GetClanName());
		else
			sprintf(m_pMUserAndClanName->Ref().m_szUserAndClanName, "%s", m_Property.GetName());
		m_pMUserAndClanName->MakeCrc();
	}

	MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(GetUID());

	ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	if (pObjCache && IsAdminGrade(pObjCache->GetUGrade()) &&
		pObjCache->CheckFlag(MTD_PlayerFlags_AdminHide))
		uStatus.m_bAdminHide = true;
	else
		uStatus.m_bAdminHide = false;
}

bool ZCharacter::Create(MTD_CharInfo* pCharInfo)
{
	m_MInitialInfo.Set(*pCharInfo);
	m_MInitialInfo.MakeCrc();

	for (int i = 0; i < MMCIP_END; i++) {
		m_Items.EquipItem(MMatchCharItemParts(i), pCharInfo->nEquipedItemDesc[i], pCharInfo->nEquipedItemCount[i]);
	}

	InitProperties();
	InitMesh();

	m_bInitialized = true;
	m_bInitialized_DebugRegister = true;

	SetAnimationLower(ZC_STATE_LOWER_IDLE1);
	SetAnimationUpper(ZC_STATE_UPPER_NONE);

	InitMeshParts();

	CreateShadow();

	m_pSoundMaterial[0] = 0;

	if (Enable_Cloth) {
		m_pVMesh->ChangeChestCloth(1.f, 1);
	}

	ChangeLowPolyModel();

	m_dwStatusBitPackingValue.Ref().m_bIsLowModel = false;
	SetVisible(true);

	m_fAttack1Ratio.Set_CheckCrc(1.f);

	ZGetEmblemInterface()->AddClanInfo(GetClanID());

	return true;
}
void ZCharacter::Destroy()
{
	if (m_bInitialized) {
		ZGetEmblemInterface()->DeleteClanInfo(GetClanID());
	}

	m_bInitialized = false;
	m_bInitialized_DebugRegister = false;

	DestroyShadow();
}

void ZCharacter::InitMeshParts()
{
	RMeshPartsType mesh_parts_type;

	if (m_pVMesh)
	{
		ZItem* pAvatarItem = GetItems()->GetItem(MMCIP_AVATAR);
		if (pAvatarItem == NULL) {
			return;
		}

		if (pAvatarItem && pAvatarItem->IsEmpty()) {
			for (int i = 0; i < MMCIP_END; i++) {
				switch (MMatchCharItemParts(i))
				{
				case MMCIP_HEAD:	mesh_parts_type = eq_parts_head;	break;
				case MMCIP_CHEST:	mesh_parts_type = eq_parts_chest;	break;
				case MMCIP_HANDS:	mesh_parts_type = eq_parts_hands;	break;
				case MMCIP_LEGS:	mesh_parts_type = eq_parts_legs;	break;
				case MMCIP_FEET:	mesh_parts_type = eq_parts_feet;	break;
				default: continue;
				}

				if (!GetItems()->GetItem(MMatchCharItemParts(i))->IsEmpty()) {
					m_pVMesh->SetParts(mesh_parts_type, GetItems()->GetItem(MMatchCharItemParts(i))->GetDesc()->m_pMItemName->Ref().m_szMeshName);
				}
				else {
					m_pVMesh->SetBaseParts(mesh_parts_type);
				}
			}

			if (GetItems()->GetItem(MMCIP_HEAD)->IsEmpty()) {
				ChangeCharHair(m_pVMesh, m_Property.nSex, m_Property.nHair);
			}

			m_pVMesh->m_bSkipRenderFaceParts = false;

			ChangeCharFace(m_pVMesh, m_Property.nSex, m_Property.nFace);
		}
		else {
			char* szMeshName;
			MMatchItemDesc* pDesc = pAvatarItem->GetDesc();
			if (pDesc != NULL) {
				m_pVMesh->ClearParts();

				szMeshName = pDesc->m_pAvatarMeshName->Ref().m_szHeadMeshName;
				if (strlen(szMeshName) > 0)	m_pVMesh->SetParts(eq_parts_head, szMeshName);
				else							ChangeCharHair(m_pVMesh, m_Property.nSex, m_Property.nHair);

				szMeshName = pDesc->m_pAvatarMeshName->Ref().m_szChestMeshName;
				if (strlen(szMeshName) > 0)	m_pVMesh->SetParts(eq_parts_chest, szMeshName);
				else							m_pVMesh->SetBaseParts(eq_parts_chest);

				szMeshName = pDesc->m_pAvatarMeshName->Ref().m_szHandMeshName;
				if (strlen(szMeshName) > 0)	m_pVMesh->SetParts(eq_parts_hands, szMeshName);
				else							m_pVMesh->SetBaseParts(eq_parts_hands);

				szMeshName = pDesc->m_pAvatarMeshName->Ref().m_szLegsMeshName;
				if (strlen(szMeshName) > 0)	m_pVMesh->SetParts(eq_parts_legs, szMeshName);
				else							m_pVMesh->SetBaseParts(eq_parts_legs);

				szMeshName = pDesc->m_pAvatarMeshName->Ref().m_szFeetMeshName;
				if (strlen(szMeshName) > 0)	m_pVMesh->SetParts(eq_parts_feet, szMeshName);
				else							m_pVMesh->SetBaseParts(eq_parts_feet);

				m_pVMesh->m_bSkipRenderFaceParts = true;
			}
		}
	}

	SetAnimationUpper(ZC_STATE_UPPER_NONE);
	SetAnimationLower(ZC_STATE_LOWER_IDLE1);

	if (!ZGetGame()->GetMatch()->IsRuleGladiator())
	{
		if (!m_Items.GetItem(MMCIP_PRIMARY)->IsEmpty())			ChangeWeapon(MMCIP_PRIMARY);
		else if (!m_Items.GetItem(MMCIP_SECONDARY)->IsEmpty())	ChangeWeapon(MMCIP_SECONDARY);
		else if (!m_Items.GetItem(MMCIP_MELEE)->IsEmpty())		ChangeWeapon(MMCIP_MELEE);
		else if (!m_Items.GetItem(MMCIP_CUSTOM1)->IsEmpty())	ChangeWeapon(MMCIP_CUSTOM1);
		else if (!m_Items.GetItem(MMCIP_CUSTOM2)->IsEmpty())	ChangeWeapon(MMCIP_CUSTOM2);
		else ChangeWeapon(MMCIP_PRIMARY);
	}
	else
	{
		if (!m_Items.GetItem(MMCIP_MELEE)->IsEmpty()) ChangeWeapon(MMCIP_MELEE);
		else if (!m_Items.GetItem(MMCIP_CUSTOM1)->IsEmpty()) ChangeWeapon(MMCIP_CUSTOM1);
		else if (!m_Items.GetItem(MMCIP_CUSTOM2)->IsEmpty()) ChangeWeapon(MMCIP_CUSTOM2);
		else ChangeWeapon(MMCIP_PRIMARY);
	}
}

void ZCharacter::ChangeWeapon(MMatchCharItemParts nParts)
{
	if (m_Items.GetSelectedWeaponParts() == nParts) return;

	if (nParts < 0 || nParts > MMCIP_END)
	{
		return;
	}
	if (m_Items.GetItem(nParts) == NULL) return;
	if (m_Items.GetItem(nParts)->GetDesc() == NULL) return;

	if (ZGetGame()->GetMatch()->IsRuleGladiator())
	{
		if ((nParts == MMCIP_PRIMARY) || (nParts == MMCIP_SECONDARY)) {
			return;
		}
	}

	MMatchCharItemParts BackupParts = m_Items.GetSelectedWeaponParts();

	m_Items.SelectWeapon(nParts);

	if (m_Items.GetSelectedWeapon() == NULL) return;

	MMatchItemDesc* pSelectedItemDesc = m_Items.GetSelectedWeapon()->GetDesc();

	if (pSelectedItemDesc == NULL) {
		m_Items.SelectWeapon(BackupParts);
		mlog("선택된 무기의 데이터가 없다.\n");
		mlog("ZCharacter 무기상태와 RVisualMesh 의 무기상태가 틀려졌다\n");
		return;
	}

	OnChangeWeapon(pSelectedItemDesc->m_pMItemName->Ref().m_szMeshName);

	if (nParts != MMCIP_MELEE)
		m_bCharged->Set_CheckCrc(false);
}

bool ZCharacter::CheckValidShotTime(int nItemID, float fTime, ZItem* pItem)
{
#ifdef _CHECKVALIDSHOTLOG
	char szTime[32]; _strtime(szTime);
	char szLog[256];
#endif

	if (GetLastShotItemID() == nItemID)
	{
		if (fTime - GetLastShotTime() < (float)pItem->GetDesc()->m_nDelay.Ref() / 1000.0f)
		{
			MMatchWeaponType nWeaponType = pItem->GetDesc()->m_nWeaponType.Ref();
			if ((MWT_DAGGER <= nWeaponType && nWeaponType <= MWT_DOUBLE_KATANA)
				&& (fTime - GetLastShotTime() >= 0.23f))
			{
			}
			else if ((nWeaponType == MWT_DOUBLE_KATANA || nWeaponType == MWT_DUAL_DAGGER)
				&& (fTime - GetLastShotTime() >= 0.099f))
			{
			}
			else
			{
#ifdef _CHECKVALIDSHOTLOG
				sprintf(szLog, "IGNORE>> [%s] (%u:%u) Interval(%0.2f) Delay(%0.2f) \n",
					szTime, GetUID().High, GetUID().Low, fTime - GetLastShotTime(), (float)pItem->GetDesc()->m_nDelay / 1000.0f);
				OutputDebugString(szLog);
#endif
				m_dwIsValidTime = -10;
				return false;
			}
		}
	}

#ifdef _CHECKVALIDSHOTLOG
	sprintf(szLog, "[%s] (%u:%u) %u(%f)\n",
		szTime, GetUID().High, GetUID().Low, nItemID, fTime);
	OutputDebugString(szLog);
#endif
	m_dwIsValidTime = FOR_DEBUG_REGISTER;
	return true;
}

bool ZCharacter::IsObserverTarget()
{
	if (ZGetCombatInterface()->GetObserver()->GetTargetCharacter() == this)
	{
		return true;
	}

	return false;
}
void ZCharacter::OnDamagedAnimation(ZObject* pAttacker, int type)
{
	if (pAttacker == NULL)
		return;

	ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	if (!uStatus.m_bBlastDrop && !uStatus.m_bBlastDrop)
	{
		rvector dir = GetPosition() - pAttacker->GetPosition();
		Normalize(dir);

		uStatus.m_bStun = true;
		SetVelocity(0, 0, 0);

		float fRatio = GetMoveSpeedRatio();

		if (type == SEM_WomanSlash5 || type == SEM_ManSlash5)
		{
			AddVelocity(dir * MAX_SPEED * fRatio);
			MEMBER_SET_CHECKCRC(m_damageInfo, m_nStunType, ZST_SLASH);

			ZCharacterObject* pCObj = MDynamicCast(ZCharacterObject, pAttacker);

			if (pCObj) {
				ZC_ENCHANT etype = pCObj->GetEnchantType();
				if (etype == ZC_ENCHANT_LIGHTNING)
					MEMBER_SET_CHECKCRC(m_damageInfo, m_nStunType, ZST_LIGHTNING);
			}
		}
		else {
			AddVelocity(dir * RUN_SPEED * fRatio);
			MEMBER_SET_CHECKCRC(m_damageInfo, m_nStunType, (ZSTUNTYPE)((type) % 2));
			if (type <= SEM_ManSlash4)
				MEMBER_SET_CHECKCRC(m_damageInfo, m_nStunType, (ZSTUNTYPE)(1 - m_damageInfo.Ref().m_nStunType));
		}

		uStatus.m_bPlayDone = false;
	}
}

void ZCharacter::ActDead()
{
	if (m_bInitialized == false)	return;
	if (!IsVisible())			return;

	rvector vDir = m_damageInfo.Ref().m_LastDamageDir;
	vDir.z = 0.f;
	Normalize(vDir);
	vDir.z = 0.6f;
	Normalize(vDir);

	float fForce = 1.f;

	bool bKnockBack = false;

	SetDeadTime(ZGetGame()->GetTime());

	ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	if ((GetStateLower() != ZC_STATE_LOWER_DIE1) && (GetStateLower() != ZC_STATE_LOWER_DIE2) &&
		(GetStateLower() != ZC_STATE_LOWER_DIE3) && (GetStateLower() != ZC_STATE_LOWER_DIE4))
	{
		ZC_STATE_LOWER lower_motion;

		float dot = m_damageInfo.Ref().m_LastDamageDot;

		switch (m_damageInfo.Ref().m_LastDamageWeapon)
		{
		case MWT_DAGGER:
		case MWT_DUAL_DAGGER:
		case MWT_KATANA:
		case MWT_GREAT_SWORD:
		case MWT_DOUBLE_KATANA:
			bKnockBack = false;
			break;

		case MWT_PISTOL:
		case MWT_PISTOLx2:
		case MWT_REVOLVER:
		case MWT_REVOLVERx2:
		case MWT_SMG:
		case MWT_SMGx2:
		case MWT_RIFLE:
		case MWT_SNIFER:
			if (m_damageInfo.Ref().m_LastDamageDistance < 800.f)
			{
				fForce = 300 + (1.f - (m_damageInfo.Ref().m_LastDamageDistance / 800.f)) * 500.f;

				bKnockBack = true;
			}
			break;

		case MWT_SHOTGUN:
		case MWT_SAWED_SHOTGUN:
		case MWT_MACHINEGUN:
			if (m_damageInfo.Ref().m_LastDamageDistance < 1000.f)
			{
				fForce = 400 + (1.f - (m_damageInfo.Ref().m_LastDamageDistance / 1000.f)) * 500.f;

				bKnockBack = true;
			}
			break;

		case MWT_ROCKET:
		case MWT_FRAGMENTATION:
			fForce = 600.f;
			bKnockBack = true;

			break;

		case MWT_TRAP:
		{
			bKnockBack = false;
		}
		break;

		case MWT_DYNAMITYE:
		{
			fForce = 600.f;
			bKnockBack = true;
		}
		break;

		default:
			lower_motion = ZC_STATE_LOWER_DIE1;
			break;
		}

		if (m_damageInfo.Ref().m_LastDamageType == ZD_BULLET_HEADSHOT) {
			bKnockBack = true;
			fForce = 700.f;
		}

		if (bKnockBack) {
			ZObject::OnKnockback(vDir, fForce);
		}

		if (bKnockBack) {
			if (dot < 0)	lower_motion = ZC_STATE_LOWER_DIE3;
			else		lower_motion = ZC_STATE_LOWER_DIE4;
		}
		else {
			if (dot < 0)	lower_motion = ZC_STATE_LOWER_DIE1;
			else		lower_motion = ZC_STATE_LOWER_DIE2;
		}

		if (GetPosition().z <= DIE_CRITICAL_LINE)
		{
			lower_motion = ZC_STATE_PIT;

			m_dwStatusBitPackingValue.Ref().m_bFallingToNarak = true;
		}
		SetAnimationLower(lower_motion);
	}

	if (GetStateUpper() != ZC_STATE_UPPER_NONE)
	{
		SetAnimationUpper(ZC_STATE_UPPER_NONE);
	}

#define EXCELLENT_TIME	3.0f
	ZCharacter* pLastAttacker = ZGetCharacterManager()->Find(GetLastAttacker());
	if (pLastAttacker && pLastAttacker != this)
	{
		if (ZGetGame()->GetTime() - pLastAttacker->m_killInfo.Ref().m_fLastKillTime < EXCELLENT_TIME
			&& ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL
			&& ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUELTOURNAMENT)
		{
			MEMBER_SET_CHECKCRC(pLastAttacker->GetStatus(), nExcellent, pLastAttacker->GetStatus().Ref().nExcellent + 1)
				pLastAttacker->AddIcon(ZCI_EXCELLENT);
		}

		MEMBER_SET_CHECKCRC(pLastAttacker->m_killInfo, m_fLastKillTime, ZGetGame()->GetTime());

		if (!uStatus.m_bLand && GetDistToFloor() > 200.f && ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL
			&& ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUELTOURNAMENT)
		{
			MEMBER_SET_CHECKCRC(pLastAttacker->GetStatus(), nFantastic, pLastAttacker->GetStatus().Ref().nFantastic + 1)
				pLastAttacker->AddIcon(ZCI_FANTASTIC);
		}

		if (pLastAttacker && ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL
			&& ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUELTOURNAMENT)
		{
			m_killInfo.CheckCrc();
			ZCharacter::KillInfo& lastAttackerKillInfo = m_killInfo.Ref();
			lastAttackerKillInfo.m_nKillsThisRound++;
			if (lastAttackerKillInfo.m_nKillsThisRound == 3)
				MEMBER_SET_CHECKCRC(pLastAttacker->GetStatus(), nUnbelievable, pLastAttacker->GetStatus().Ref().nUnbelievable + 1);
			if (lastAttackerKillInfo.m_nKillsThisRound >= 3)
			{
				pLastAttacker->AddIcon(ZCI_UNBELIEVABLE);
			}
			m_killInfo.MakeCrc();
		}
	}
}

void ZCharacter::AddIcon(int nIcon)
{
	if (nIcon < 0 || nIcon >= 5) return;

	ZGetEffectManager()->AddCharacterIcon(this, nIcon);

	ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if (pTargetCharacter == this)
	{
		ZGetScreenEffectManager()->AddPraise(nIcon);
	}
}
void ZCharacter::ToggleClothSimulation()
{
	if (!m_pVMesh) return;

	if (Enable_Cloth)
		m_pVMesh->ChangeChestCloth(1.f, 1);
	else
		m_pVMesh->DestroyCloth();
}

bool ZCharacter::Save(ZFile* file)
{
	size_t n;

	n = zfwrite(&m_bHero, sizeof(m_bHero), 1, file);
	if (n != 1) return false;

	n = zfwrite(&m_MInitialInfo.Ref(), sizeof(m_MInitialInfo.Ref()), 1, file);
	if (n != 1) return false;

	n = zfwrite(&m_UID, sizeof(m_UID), 1, file);
	if (n != 1) return false;

	ZCharacterProperty_Old oldProperty;
	m_Property.CopyToOldStruct(oldProperty);
	n = zfwrite(&oldProperty, sizeof(oldProperty), 1, file);
	if (n != 1) return false;

	float fHP = m_pModule_HPAP->GetHP();
	n = zfwrite(&fHP, sizeof(float), 1, file);
	if (n != 1) return false;

	float fAP = m_pModule_HPAP->GetAP();
	n = zfwrite(&fAP, sizeof(float), 1, file);
	if (n != 1) return false;

	n = zfwrite(&m_Status.Ref(), sizeof(ZCharacterStatus), 1, file);
	if (n != 1) return false;

	if (!m_Items.Save(file)) return false;

	n = zfwrite((void*)&GetPosition(), sizeof(GetPosition()), 1, file);
	if (n != 1) return false;

	n = zfwrite(&m_Direction, sizeof(m_Direction), 1, file);
	if (n != 1) return false;

	n = zfwrite(&m_nTeamID.Ref(), sizeof(m_nTeamID.Ref()), 1, file);
	if (n != 1) return false;

	ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	bool bDie = uStatus.m_bDie;
	n = zfwrite(&bDie, sizeof(bDie), 1, file);
	if (n != 1) return false;

	bool bAdminHide = uStatus.m_bAdminHide;
	n = zfwrite(&bAdminHide, sizeof(bAdminHide), 1, file);
	if (n != 1) return false;

	return true;
}

bool ZCharacter::Load(ZFile* file, int nVersion)
{
	size_t n;

	n = zfread(&m_UID, sizeof(m_UID), 1, file);
	if (n != 1) return false;

	ZCharacterProperty_Old oldProperty;
	n = zfread(&oldProperty, sizeof(oldProperty), 1, file);
	if (n != 1) return false;
	m_Property.CopyFromOldStruct(oldProperty);

	InitHPAP();

	float fHP;
	n = zfread(&fHP, sizeof(float), 1, file);
	if (n != 1) return false;
	m_pModule_HPAP->SetHP(fHP);

	float fAP;
	n = zfread(&fAP, sizeof(float), 1, file);
	if (n != 1) return false;
	m_pModule_HPAP->SetAP(fAP);

	n = zfread(&m_Status.Ref(), sizeof(ZCharacterStatus), 1, file);
	if (n != 1) return false;
	m_Status.MakeCrc();

	if (!m_Items.Load(file, nVersion)) return false;

	rvector pos;
	n = zfread(&pos, sizeof(pos), 1, file);
	if (n != 1) return false;
	SetPosition(pos);

	n = zfread(&m_Direction, sizeof(m_Direction), 1, file);
	if (n != 1) return false;

	MMatchTeam teamID;
	n = zfread(&teamID, sizeof(teamID), 1, file);
	if (n != 1) return false;
	m_nTeamID.Set_CheckCrc(teamID);

	ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	bool bDie;
	n = zfread(&bDie, sizeof(bDie), 1, file);
	uStatus.m_bDie = bDie;
	if (n != 1) return false;

	if (nVersion >= 2) {
		bool bAdminHide;
		n = zfread(&bAdminHide, sizeof(bAdminHide), 1, file);
		uStatus.m_bAdminHide = bAdminHide;
		if (n != 1) return false;
	}

	return true;
}

void ZCharacter::OnLevelDown()
{
	m_Property.nLevel--;
}
void ZCharacter::OnLevelUp()
{
	m_Property.nLevel++;
	ZGetEffectManager()->AddLevelUpEffect(this);
}

void ZCharacter::LevelUp()
{
	OnLevelUp();
}
void ZCharacter::LevelDown()
{
	OnLevelDown();
}

RMesh* ZCharacter::GetWeaponMesh(MMatchCharItemParts parts)
{
	ZItem* pWeapon = m_Items.GetItem(parts);

	if (!pWeapon) return NULL;

	if (pWeapon->GetDesc() == NULL) return NULL;

	RMesh* pMesh = ZGetWeaponMeshMgr()->Get(pWeapon->GetDesc()->m_pMItemName->Ref().m_szMeshName);
	return pMesh;
}

bool ZCharacter::IsRunWall()
{
	ZC_STATE_LOWER s = m_AniState_Lower.Ref();

	if ((s == ZC_STATE_LOWER_RUN_WALL_LEFT) ||
		(s == ZC_STATE_LOWER_RUN_WALL_LEFT_DOWN) ||
		(s == ZC_STATE_LOWER_RUN_WALL) ||
		(s == ZC_STATE_LOWER_RUN_WALL_DOWN_FORWARD) ||
		(s == ZC_STATE_LOWER_RUN_WALL_DOWN) ||
		(s == ZC_STATE_LOWER_RUN_WALL_RIGHT) ||
		(s == ZC_STATE_LOWER_RUN_WALL_RIGHT_DOWN) ||
		(s == ZC_STATE_LOWER_JUMP_WALL_FORWARD) ||
		(s == ZC_STATE_LOWER_JUMP_WALL_BACK) ||
		(s == ZC_STATE_LOWER_JUMP_WALL_LEFT) ||
		(s == ZC_STATE_LOWER_JUMP_WALL_RIGHT)) {
		return true;
	}
	return false;
}

bool ZCharacter::IsMeleeWeapon()
{
	ZItem* pItem = m_Items.GetSelectedWeapon();

	if (pItem) {
		if (pItem->GetDesc()) {
			if (pItem->GetDesc()->m_nType.Ref() == MMIT_MELEE) {
				return true;
			}
		}
	}

	return false;
}

bool ZCharacter::IsCollideable()
{
	const ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	if (m_Collision.IsCollideable())
	{
		return ((!IsDie() && !uStatus.m_bBlastDrop));
	}

	return m_Collision.IsCollideable();
}

bool ZCharacter::IsAttackable()
{
	if (IsDie()) return false;
	return true;
}

float ZCharacter::ColTest(const rvector& pos, const rvector& vec, float radius, rplane* out)
{
	return SweepTest(rsphere(pos, radius), vec, rsphere(GetPosition(), CHARACTER_COLLISION_DIST), out);
}

bool ZCharacter::IsGuard()
{
	return ((ZC_STATE_LOWER_GUARD_IDLE <= m_AniState_Lower.Ref() && m_AniState_Lower.Ref() <= ZC_STATE_LOWER_GUARD_BLOCK2) ||
		(ZC_STATE_UPPER_GUARD_IDLE <= m_AniState_Upper.Ref() && m_AniState_Upper.Ref() <= ZC_STATE_UPPER_GUARD_BLOCK2));
}

void ZCharacter::InitRound()
{
	if (GetUserGrade() == MMUG_STAR) {
		ZGetEffectManager()->AddStarEffect(this);
	}
}

ZOBJECTHITTEST ZCharacter::HitTest(const rvector& origin, const rvector& to, float fTime, rvector* pOutPos)
{
	rvector footpos, headpos, characterdir;
	if (GetHistory(&footpos, &characterdir, fTime))
	{
		footpos.z += 5.f;
		if (IsRendered())
		{
			headpos = GetVisualMesh()->GetHeadPosition();
			headpos.z += 5.f;
		}
		else
			headpos = footpos + rvector(0, 0, 180);

		rvector rootpos = (footpos + headpos) * 0.5f;

		rvector nearest = GetNearestPoint(headpos, origin, to);
		float fDist = Magnitude(nearest - headpos);
		float fDistToChar = Magnitude(nearest - origin);

		rvector ap, cp;

		if (fDist < 15.f)
		{
			if (pOutPos) *pOutPos = nearest;
			return ZOH_HEAD;
		}
		else
		{
			rvector dir = to - origin;
			Normalize(dir);

			rvector rootdir = (rootpos - headpos);
			Normalize(rootdir);
			float fDist = GetDistanceBetweenLineSegment(origin, to, headpos + 20.f * rootdir, rootpos - 20.f * rootdir, &ap, &cp);
			if (fDist < 30)
			{
				rvector ap2cp = ap - cp;
				float fap2cpsq = D3DXVec3LengthSq(&ap2cp);
				float fdiff = sqrtf(30.f * 30.f - fap2cpsq);

				if (pOutPos) *pOutPos = ap - dir * fdiff;;
				return ZOH_BODY;
			}
			else
			{
				float fDist = GetDistanceBetweenLineSegment(origin, to, rootpos - 20.f * rootdir, footpos, &ap, &cp);
				if (fDist < 30)
				{
					rvector ap2cp = ap - cp;
					float fap2cpsq = D3DXVec3LengthSq(&ap2cp);
					float fdiff = sqrtf(30.f * 30.f - fap2cpsq);

					if (pOutPos) *pOutPos = ap - dir * fdiff;;
					return ZOH_LEGS;
				}
			}
		}
	}
	return ZOH_NONE;
}

void ZCharacter::OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio, int nMeleeType)
{
	if (m_bInitialized == false)
		PROTECT_DEBUG_REGISTER(m_bInitialized_DebugRegister == false)
		return;
	bool bDebugRegister = !IsVisible() || IsDie();
	if (!IsVisible() || IsDie())
		PROTECT_DEBUG_REGISTER(bDebugRegister)
		return;

	bool bCanAttack = ZGetGame()->CanAttack(pAttacker, this) || (pAttacker == this && (damageType == ZD_EXPLOSION || damageType == ZD_FALLING));
	bDebugRegister = ZGetGame()->CanAttack(pAttacker, this) || (pAttacker == this && (damageType == ZD_EXPLOSION || damageType == ZD_FALLING));
	bool bReturnValue = ZGetGameTypeManager()->IsTeamExtremeGame(ZGetGame()->GetMatch()->GetMatchType());

	if (ZGetGameTypeManager()->IsTeamExtremeGame(ZGetGame()->GetMatch()->GetMatchType()))
	{
		PROTECT_DEBUG_REGISTER(bReturnValue)
		{
			if (damageType != ZD_FALLING)
			{
				bCanAttack &= !isInvincible();
				bDebugRegister &= !isInvincible();
			}
		}
	}

	rvector dir = GetPosition() - srcPos;
	Normalize(dir);

	m_damageInfo.CheckCrc();
	m_damageInfo.Ref().m_LastDamageDir = dir;
	m_damageInfo.Ref().m_LastDamageType = damageType;
	m_damageInfo.Ref().m_LastDamageWeapon = weaponType;
	m_damageInfo.Ref().m_LastDamageDot = DotProduct(m_Direction, dir);
	m_damageInfo.Ref().m_LastDamageDistance = Magnitude(GetPosition() - srcPos);
	m_damageInfo.MakeCrc();

	if (!bCanAttack)
		PROTECT_DEBUG_REGISTER(!bDebugRegister)
		return;

	ZObject::OnDamaged(pAttacker, srcPos, damageType, weaponType, fDamage, fPiercingRatio, nMeleeType);

	if (damageType == ZD_MELEE) OnDamagedAnimation(pAttacker, nMeleeType);

	m_dwStatusBitPackingValue.Ref().m_bDamaged = true;

#ifdef _CHATOUTPUT_ENABLE_CHAR_DAMAGE_INFO_
	char szDamagePrint[256];
	sprintf(szDamagePrint, "%s에게 대미지[%2.1f], 남은 HP[%2.1f] AP[%2.1f]", GetUserName(), fDamage, GetHP(), GetAP());
	ZChatOutput(MCOLOR(255, 100, 100), szDamagePrint);
#endif
}

void ZCharacter::OnDamagedAPlayer(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio, int nMeleeTpye)
{
	if (damageType == ZD_MELEE)
	{
		OnDamaged(pAttacker, srcPos, damageType, weaponType, fDamage, fPiercingRatio, nMeleeTpye);
		return;
	}

	if (pAttacker != NULL && fDamage > 0)
	{
		if (pAttacker == ZGetGame()->m_pMyCharacter && this != pAttacker && !ZGetGame()->m_pMyCharacter->IsDie())
		{
			void* pBlobArray = MMakeBlobArray(sizeof(MTD_ShotInfo), 1);
			void* pBlobElement = MGetBlobArrayElement(pBlobArray, 0);

			MTD_ShotInfo shotInfo;
			shotInfo.fDamage = fDamage;
			shotInfo.fPosX = srcPos.x;
			shotInfo.fPosY = srcPos.y;
			shotInfo.fPosZ = srcPos.z;
			shotInfo.nDamageType = damageType;
			shotInfo.fRatio = fPiercingRatio;
			shotInfo.nLowId = GetUID().Low;
			shotInfo.nWeaponType = weaponType;

			memcpy(pBlobElement, &shotInfo, sizeof(MTD_ShotInfo));
			ZGetGameClient()->GetPeerPacketCrypter().Encrypt((char*)pBlobElement, sizeof(MTD_ShotInfo));
			ZPOSTCMDLEAD(MC_GUNZ_ANTILEAD, GetUID(), MCommandParameterBlob(pBlobArray, MGetBlobArraySize(pBlobArray)));
		}
	}
}
void ZCharacter::OnDamagedAPlayer(ZObject* pAttacker, vector<MTD_ShotInfo*> vShots)
{
	if (vShots.size() > 0)
	{
		if (ZGetGameClient()->GetPlayerUID().Low != vShots[0]->nLowId && GetUID().Low == vShots[0]->nLowId)
		{
			if (pAttacker == ZGetGame()->m_pMyCharacter && this != pAttacker && !ZGetGame()->m_pMyCharacter->IsDie())
			{
				void* pBlobArray = MMakeBlobArray(sizeof(MTD_ShotInfo), vShots.size());
				for (int i = 0; i < vShots.size(); ++i)
				{
					MTD_ShotInfo* pShot = vShots[i];
					void* pElement = MGetBlobArrayElement(pBlobArray, i);
					memcpy(pElement, pShot, sizeof(MTD_ShotInfo));
					ZGetGameClient()->GetPeerPacketCrypter().Encrypt((char*)pElement, sizeof(MTD_ShotInfo));
				}

				ZPOSTCMDLEAD(MC_GUNZ_ANTILEAD, GetUID(), MCommandParameterBlob(pBlobArray, MGetBlobArraySize(pBlobArray)));
			}
		}
	}
}
void ZCharacter::OnScream()
{
	if (GetProperty()->nSex == MMS_MALE)
		ZGetSoundEngine()->PlaySound("ooh_male", GetPosition(), IsObserverTarget());
	else
		ZGetSoundEngine()->PlaySound("ooh_female", GetPosition(), IsObserverTarget());
}

void ZCharacter::OnMeleeGuardSuccess()
{
	ZGetSoundEngine()->PlaySound("fx_guard", GetPosition(), IsObserverTarget());
}

void ZCharacter::OnShot()
{
	ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();
	if (uStatus.m_bChatEffect) uStatus.m_bChatEffect = false;
}

void ZCharacter::SetInvincibleTime(int nDuration)
{
	m_pMdwInvincibleStartTime->Set_CheckCrc(gaepanEncode(timeGetTime(), 4));
	m_pMdwInvincibleDuration->Set_CheckCrc(gaepanEncode(nDuration, 5));
}

bool ZCharacter::isInvincible()
{
	return ((int)timeGetTime() < (gaepanDecode(m_pMdwInvincibleStartTime->Ref(), 4) + gaepanDecode(m_pMdwInvincibleDuration->Ref(), 5)));
}

void ZCharacter::ApplyBuffEffect()
{
}

int gaepanEncode(int a, int depth)
{
	if (depth > 0)
	{
		int t = a ^ 0xa56b21;
		t += 516912;
		t = gaepanEncode(t, depth - 1);
		return t;
	}
	return a;
}

int gaepanDecode(int a, int depth)
{
	if (depth > 0)
	{
		int t = gaepanDecode(a, depth - 1);
		t -= 516912;
		t ^= 0xa56b21;
		return t;
	}
	return a;
}