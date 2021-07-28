#ifndef _ZCHARACTER_H
#define _ZCHARACTER_H

#include "MRTTI.h"
#include "ZCharacterObject.h"
#include "MUID.h"
#include "RTypes.h"
#include "RVisualMeshMgr.h"

#include "MObjectTypes.h"
#include "ZItem.h"
#include "ZCharacterItem.h"
#include "ZCharacterBuff.h"

#include "MMatchObject.h"
#include "RCharCloth.h"
#include "ZFile.h"
#include "Mempool.h"

#include "ZModule_HPAP.h"

#include <list>
#include <string>

using namespace std;

_USING_NAMESPACE_REALSPACE2

#define MAX_SPEED			1000.f
#define RUN_SPEED			630.f
#define BACK_SPEED			450.f
#define ACCEL_SPEED			7000.f
#define STOP_SPEED			3000.f
#define STOP_FORMAX_SPEED	7100.f

#define CHARACTER_RADIUS	35.f
#define CHARACTER_HEIGHT	180.0f

#define ARRIVAL_TOLER		5.f

class ZShadow;

struct ZANIMATIONINFO {
	char* Name;
	bool bEnableCancel;
	bool bLoop;
	bool bMove;
	bool bContinuos;
};

struct ZFACECOSTUME
{
	char* szMaleMeshName;
	char* szFemaleMeshName;
};

enum ZC_SKILL {
	ZC_SKILL_NONE = 0,

	ZC_SKILL_UPPERCUT,
	ZC_SKILL_SPLASHSHOT,
	ZC_SKILL_DASH,
	ZC_SKILL_CHARGEDSHOT,

	ZC_SKILL_END
};

enum ZC_DIE_ACTION
{
	ZC_DIE_ACTION_RIFLE = 0,
	ZC_DIE_ACTION_KNIFE,
	ZC_DIE_ACTION_SHOTGUN,
	ZC_DIE_ACTION_ROCKET,

	ZC_DIE_ACTION_END
};

enum ZC_SPMOTION_TYPE {
	ZC_SPMOTION_TAUNT = 0,
	ZC_SPMOTION_BOW,
	ZC_SPMOTION_WAVE,
	ZC_SPMOTION_LAUGH,
	ZC_SPMOTION_CRY,
	ZC_SPMOTION_DANCE,

	ZC_SPMOTION_END
};

enum ZC_WEAPON_SLOT_TYPE {
	ZC_SLOT_MELEE_WEAPON = 0,
	ZC_SLOT_PRIMARY_WEAPON,
	ZC_SLOT_SECONDARY_WEAPON,
	ZC_SLOT_ITEM1,
	ZC_SLOT_ITEM2,

	ZC_SLOT_END,
};

enum ZC_SHOT_SP_TYPE {
	ZC_WEAPON_SP_NONE = 0,

	ZC_WEAPON_SP_GRENADE,
	ZC_WEAPON_SP_ROCKET,
	ZC_WEAPON_SP_FLASHBANG,
	ZC_WEAPON_SP_SMOKE,
	ZC_WEAPON_SP_TEAR_GAS,

	ZC_WEAPON_SP_ITEMKIT,

	ZC_WEAPON_SP_POTION,
	ZC_WEAPON_SP_TRAP,
	ZC_WEAPON_SP_DYNAMITE,
	ZC_WEAPON_SP_END,
};

enum ZSTUNTYPE {
	ZST_NONE = -1,
	ZST_DAMAGE1 = 0,
	ZST_DAMAGE2,
	ZST_SLASH,
	ZST_BLOCKED,
	ZST_LIGHTNING,
	ZST_LOOP,
};

class ZSlot {
public:
	ZSlot() {
		m_WeaponID = 0;
	}

	int m_WeaponID;
};

struct ZCharacterProperty_Old
{
	char		szName[MATCHOBJECT_NAME_LENGTH];
	char		szClanName[CLAN_NAME_LENGTH];
	MMatchSex	nSex;
	int			nHair;
	int			nFace;
	int			nLevel;
	float		fMaxHP;
	float		fMaxAP;
	int			nMoveSpeed;
	int			nWeight;
	int			nMaxWeight;
	int			nSafeFall;
	ZCharacterProperty_Old() : nSex(MMS_MALE),
		nHair(0),
		nFace(0),
		nLevel(1),
		fMaxHP(1000.f),
		fMaxAP(1000.f),
		nMoveSpeed(100),
		nWeight(0),
		nMaxWeight(100),
		nSafeFall(100)
	{
		szName[0] = 0;
		szClanName[0] = 0;
	}
	void SetName(const char* name) { strcpy(szName, name); }
	void SetClanName(const char* name) { strcpy(szClanName, name); }
};

struct ZCharacterProperty_CharClanName
{
	char		szName[MATCHOBJECT_NAME_LENGTH];
	char		szClanName[CLAN_NAME_LENGTH];
};
struct ZCharacterProperty
{
	MProtectValue<ZCharacterProperty_CharClanName> nameCharClan;
	MMatchSex	nSex;
	int			nHair;
	int			nFace;
	int			nLevel;
	MProtectValue<float>		fMaxHP;
	MProtectValue<float>		fMaxAP;
	ZCharacterProperty() : nSex(MMS_MALE),
		nHair(0),
		nFace(0),
		nLevel(1)
	{
		nameCharClan.Ref().szName[0] = 0;
		nameCharClan.Ref().szClanName[0] = 0;
		nameCharClan.MakeCrc();

		fMaxHP.Set_MakeCrc(DEFAULT_CHAR_HP);
		fMaxAP.Set_MakeCrc(DEFAULT_CHAR_AP);
	}
	void SetName(const char* name) { nameCharClan.CheckCrc(); strcpy(nameCharClan.Ref().szName, name);	   nameCharClan.MakeCrc(); }
	void SetClanName(const char* name) { nameCharClan.CheckCrc(); strcpy(nameCharClan.Ref().szClanName, name); nameCharClan.MakeCrc(); }
	const char* GetName() { return nameCharClan.Ref().szName; }
	const char* GetClanName() { return nameCharClan.Ref().szClanName; }

	void CopyToOldStruct(ZCharacterProperty_Old& old)
	{
		memcpy(old.szName, nameCharClan.Ref().szName, MATCHOBJECT_NAME_LENGTH);
		memcpy(old.szClanName, nameCharClan.Ref().szClanName, CLAN_NAME_LENGTH);

		old.nSex = nSex;
		old.nHair = nHair;
		old.nFace = nFace;
		old.nLevel = nLevel;
		old.fMaxHP = fMaxHP.Ref();
		old.fMaxAP = fMaxAP.Ref();
		old.nMoveSpeed = 100;
		old.nWeight = 0;
		old.nMaxWeight = 100;
		old.nSafeFall = 100;
	}

	void CopyFromOldStruct(ZCharacterProperty_Old& old)
	{
		memcpy(nameCharClan.Ref().szName, old.szName, MATCHOBJECT_NAME_LENGTH);
		memcpy(nameCharClan.Ref().szClanName, old.szClanName, CLAN_NAME_LENGTH);
		nameCharClan.MakeCrc();

		nSex = old.nSex;
		nHair = old.nHair;
		nFace = old.nFace;
		nLevel = old.nLevel;
		fMaxHP.Set_MakeCrc(old.fMaxHP);
		fMaxAP.Set_MakeCrc(old.fMaxAP);
	}
};

struct ZCharacterStatus
{
	int			nLife;
	int			nKills;
	int			nDeaths;
	int			nLoadingPercent;
	int			nCombo;
	int			nMaxCombo;
	int			nAllKill;
	int			nExcellent;
	int			nFantastic;
	int			nHeadShot;
	int			nUnbelievable;
	int			nExp;
	int			nDamageCaused;
	int			nDamageTaken;

	ZCharacterStatus() :
		nLife(10),
		nKills(0),
		nDeaths(0),
		nLoadingPercent(0),
		nCombo(0),
		nMaxCombo(0),
		nAllKill(0),
		nExcellent(0),
		nFantastic(0),
		nHeadShot(0),
		nUnbelievable(0),
		nExp(0),
		nDamageCaused(0),
		nDamageTaken(0)

	{  }

	void AddKills(int nAddedKills = 1) { nKills += nAddedKills; }
	void AddDeaths(int nAddedDeaths = 1) { nDeaths += nAddedDeaths; }
	void AddExp(int _nExp = 1) { nExp += _nExp; }
};

#define CHARACTER_ICON_DELAY		2.f
#define CHARACTER_ICON_FADE_DELAY	.2f
#define CHARACTER_ICON_SIZE			32.f

class ZModule_HPAP;
class ZModule_QuestStatus;

struct ZCharaterStatusBitPacking {
	union {
		struct {
			bool	m_bLand : 1;
			bool	m_bWallJump : 1;
			bool	m_bJumpUp : 1;
			bool	m_bJumpDown : 1;
			bool	m_bWallJump2 : 1;
			bool	m_bTumble : 1;
			bool	m_bBlast : 1;
			bool	m_bBlastFall : 1;
			bool	m_bBlastDrop : 1;
			bool	m_bBlastStand : 1;
			bool	m_bBlastAirmove : 1;
			bool	m_bSpMotion : 1;
			bool	m_bCommander : 1;
			bool	m_bLostConEffect : 1;
			bool	m_bChatEffect : 1;
			bool	m_bBackMoving : 1;

			bool	m_bAdminHide : 1;
			bool	m_bDie : 1;
			bool	m_bStylishShoted : 1;
			bool	m_bFallingToNarak : 1;
			bool	m_bStun : 1;
			bool	m_bDamaged : 1;

			bool	m_bPlayDone : 1;
			bool	m_bPlayDone_upper : 1;
			bool	m_bIsLowModel : 1;
			bool	m_bTagger : 1;
		};

		DWORD dwFlagsPublic;
	};
};

struct ZUserAndClanName
{
	char m_szUserName[MATCHOBJECT_NAME_LENGTH];
	char m_szUserAndClanName[MATCHOBJECT_NAME_LENGTH];
};

class ZCharacter : public ZCharacterObject
{
	MDeclareRTTI;
private:
protected:

	ZModule_HPAP* m_pModule_HPAP;
	ZModule_QuestStatus* m_pModule_QuestStatus;
	ZModule_Resistance* m_pModule_Resistance;
	ZModule_FireDamage* m_pModule_FireDamage;
	ZModule_ColdDamage* m_pModule_ColdDamage;
	ZModule_PoisonDamage* m_pModule_PoisonDamage;
	ZModule_LightningDamage* m_pModule_LightningDamage;
	ZModule_HealOverTime* m_pModule_HealOverTime;

	ZCharacterProperty					m_Property;
	MProtectValue<ZCharacterStatus>		m_Status;

	MProtectValue<MTD_CharInfo>			m_MInitialInfo;

protected:
	float m_fPreMaxHP;
	float m_fPreMaxAP;

public:
	void ApplyBuffEffect();

	float GetMaxHP();
	float GetMaxAP();
	float GetHP();
	float GetAP();
	void InitAccumulationDamage();
	float GetAccumulationDamage();
	void EnableAccumulationDamage(bool bAccumulationDamage);

	__forceinline void SetMaxHP(float nMaxHP) { m_pModule_HPAP->SetMaxHP(nMaxHP); }
	__forceinline void SetMaxAP(float nMaxAP) { m_pModule_HPAP->SetMaxAP(nMaxAP); }

	__forceinline void SetHP(float nHP) { m_pModule_HPAP->SetHP(nHP); }
	__forceinline void SetAP(float nAP) { m_pModule_HPAP->SetAP(nAP); }

protected:
	MProtectValue<ZUserAndClanName>* m_pMUserAndClanName;

	struct KillInfo {
		int			m_nKillsThisRound;
		float		m_fLastKillTime;
	};
	MProtectValue<KillInfo> m_killInfo;

	struct DamageInfo {
		DWORD			m_dwLastDamagedTime;
		ZSTUNTYPE		m_nStunType;
		ZDAMAGETYPE		m_LastDamageType;
		MMatchWeaponType m_LastDamageWeapon;
		rvector			m_LastDamageDir;
		float			m_LastDamageDot;
		float			m_LastDamageDistance;

		MUID			m_LastThrower;
		float			m_tmLastThrowClear;
	};
	MProtectValue<DamageInfo> m_damageInfo;

	int	m_nWhichFootSound;

	MProtectValue<DWORD>* m_pMdwInvincibleStartTime;
	MProtectValue<DWORD>* m_pMdwInvincibleDuration;

	void UpdateSound();

	void InitMesh();
	void InitProperties();

	void CheckLostConn();
	void OnLevelDown();
	void OnLevelUp();
	virtual void OnDraw() override;
	virtual void OnDie() override;

public:
	float	m_fLastValidTime;
	DWORD	m_dwIsValidTime;

	MProtectValue<ZCharaterStatusBitPacking> m_dwStatusBitPackingValue;

	MProtectValue<bool>* m_bCharged;
	MProtectValue<bool>* m_bCharging;

	MProtectValue<float>	m_fChargedFreeTime;
	MProtectValue<int>		m_nWallJumpDir;
	MProtectValue<int>		m_nBlastType;

	ZC_STATE_LOWER	m_SpMotion;

	ZCharacter();
	virtual ~ZCharacter() override;

	virtual bool Create(MTD_CharInfo* pCharInfo);
	virtual void Destroy();

	void InitMeshParts();

	void EmptyHistory();

	rvector m_TargetDir;
	rvector m_DirectionLower, m_DirectionUpper;

	MProtectValue<rvector> m_RealPositionBefore;
	MProtectValue<rvector> m_AnimationPositionDiff;
	MProtectValue<rvector> m_Accel;

	MProtectValue<ZC_STATE_UPPER>	m_AniState_Upper;
	MProtectValue<ZC_STATE_LOWER>	m_AniState_Lower;
	ZANIMATIONINFO* m_pAnimationInfo_Upper, * m_pAnimationInfo_Lower;

	void AddIcon(int nIcon);
	MProtectValue<int>				m_nVMID;
	MProtectValue<MMatchTeam>		m_nTeamID;

	MProtectValue<MCharacterMoveMode>		m_nMoveMode;
	MProtectValue<MCharacterMode>			m_nMode;
	MProtectValue<MCharacterState>			m_nState;

	MProtectValue<float>	m_fAttack1Ratio;
	float	m_fLastReceivedTime;

	float	m_fTimeOffset;
	float	m_fAccumulatedTimeError;
	int		m_nTimeErrorCount;

	float	m_fGlobalHP;
	int		m_nReceiveHPCount;

	int		m_nLastShotItemID;
	float	m_fLastShotTime;

	void	SetInvincibleTime(int nDuration);
	bool	isInvincible();

	bool IsMan();

	virtual void OnUpdate(float fDelta) override;
	void  UpdateSpeed();
	float GetMoveSpeedRatio();

	void UpdateVelocity(float fDelta);
	void UpdateHeight(float fDelta);
	void UpdateMotion(float fDelta = 0.f);
	virtual void UpdateAnimation();

	int  GetSelectWeaponDelay(MMatchItemDesc* pSelectItemDesc);

	void UpdateLoadAnimation();

	void Stop();
	void CheckDrawWeaponTrack();
	void UpdateSpWeapon();

	void SetAnimation(char* AnimationName, bool bEnableCancel, int tick);
	void SetAnimation(RAniMode mode, char* AnimationName, bool bEnableCancel, int tick);

	void SetAnimationLower(ZC_STATE_LOWER nAni);
	void SetAnimationUpper(ZC_STATE_UPPER nAni);

	ZC_STATE_LOWER GetStateLower() { return m_AniState_Lower.Ref(); }
	ZC_STATE_UPPER GetStateUpper() { return m_AniState_Upper.Ref(); }

	bool IsUpperPlayDone() { return m_dwStatusBitPackingValue.Ref().m_bPlayDone_upper; }

	bool IsMoveAnimation();

	bool IsTeam(ZCharacter* pChar);

	bool IsRunWall();
	bool IsMeleeWeapon();
	virtual bool IsCollideable() override;

	void SetTargetDir(rvector vDir);

	virtual bool Pick(const rvector& pos, const rvector& dir, RPickInfo* pInfo = NULL) override;

	void OnSetSlot(int nSlot, int WeaponID);
	void OnChangeSlot(int nSlot);

	virtual void OnChangeWeapon(char* WeaponModelName);
	void OnChangeParts(RMeshPartsType type, int PartsID);
	void OnAttack(int type, rvector& pos);
	void OnShot();

	void ChangeWeapon(MMatchCharItemParts nParts);

	int GetLastShotItemID() { return m_nLastShotItemID; }
	float GetLastShotTime() { return m_fLastShotTime; }
	bool CheckValidShotTime(int nItemID, float fTime, ZItem* pItem);
	void UpdateValidShotTime(int nItemID, float fTime)
	{
		m_nLastShotItemID = nItemID;
		m_fLastShotTime = fTime;
	}

	bool IsDie()	override { return m_dwStatusBitPackingValue.Ref().m_bDie; }
	auto IsAlive() const { return !m_dwStatusBitPackingValue.Ref().m_bDie; }
	void ForceDie()
	{
		SetHP(0);
		m_dwStatusBitPackingValue.Ref().m_bDie = true;
	}

	void SetAccel(const rvector& accel) { m_Accel.Set_CheckCrc(accel); }
	virtual void SetDirection(const rvector& dir) override;

	struct SlotInfo
	{
		int		m_nSelectSlot;
		ZSlot	m_Slot[ZC_SLOT_END];
	};
	MProtectValue<SlotInfo>	m_slotInfo;

	MProtectValue<ZCharacterStatus>& GetStatus() { return m_Status; }

	ZCharacterProperty* GetProperty() { return &m_Property; }

	MMatchUserGradeID GetUserGrade() { return m_MInitialInfo.Ref().nUGradeID; }
	unsigned int GetClanID() { return m_MInitialInfo.Ref().nClanCLID; }

	void SetName(const char* szName) { m_Property.SetName(szName); }

	const char* GetUserName() { return m_pMUserAndClanName->Ref().m_szUserName; }
	const char* GetUserAndClanName() { return m_pMUserAndClanName->Ref().m_szUserAndClanName; }
	bool IsAdminName();
	bool IsAdminHide() { return m_dwStatusBitPackingValue.Ref().m_bAdminHide; }
	void SetAdminHide(bool bHide) { m_dwStatusBitPackingValue.Ref().m_bAdminHide = bHide; }

	int GetKils() { return GetStatus().Ref().nKills; }

	bool CheckDrawGrenade();

	bool GetStylishShoted() { return m_dwStatusBitPackingValue.Ref().m_bStylishShoted; }
	void UpdateStylishShoted();

	MUID GetLastAttacker() { return m_pModule_HPAP->GetLastAttacker(); }
	void SetLastAttacker(MUID uid) { m_pModule_HPAP->SetLastAttacker(uid); }
	ZDAMAGETYPE GetLastDamageType() { return m_damageInfo.Ref().m_LastDamageType; }
	void SetLastDamageType(ZDAMAGETYPE type) { MEMBER_SET_CHECKCRC(m_damageInfo, m_LastDamageType, type); }

	bool DoingStylishMotion();

	bool IsObserverTarget();

	MMatchTeam GetTeamID() const override { return m_nTeamID.Ref(); }
	void SetTeamID(MMatchTeam nTeamID) { m_nTeamID.Set_CheckCrc(nTeamID); }
	bool IsSameTeam(const ZCharacter* pCharacter) const
	{
		if (pCharacter->GetTeamID() == -1) return false;
		if (pCharacter->GetTeamID() == GetTeamID()) return true; return false;
	}
	bool IsTagger() const { return m_dwStatusBitPackingValue.Ref().m_bTagger; }
	void SetTagger(bool bTagger) { m_dwStatusBitPackingValue.Ref().m_bTagger = bTagger; }

	void SetLastThrower(MUID uid, float fTime) { MEMBER_SET_CHECKCRC(m_damageInfo, m_LastThrower, uid); MEMBER_SET_CHECKCRC(m_damageInfo, m_tmLastThrowClear, fTime); }
	const MUID& GetLastThrower() { return m_damageInfo.Ref().m_LastThrower; }
	float GetLastThrowClearTime() { return m_damageInfo.Ref().m_tmLastThrowClear; }

	virtual void Revival();
	void Die();
	void ActDead();
	virtual void InitHPAP();
	virtual void InitStatus();
	virtual void InitRound();
	virtual void InitBullet();

	void TestChangePartsAll();
	void TestToggleCharacter();

	virtual void OutputDebugString_CharacterState();

	void ToggleClothSimulation();
	void ChangeLowPolyModel();
	bool IsFallingToNarak() { return m_dwStatusBitPackingValue.Ref().m_bFallingToNarak; }

	MMatchItemDesc* GetSelectItemDesc() {
		if (GetItems())
			if (GetItems()->GetSelectedWeapon())
				return GetItems()->GetSelectedWeapon()->GetDesc();
		return NULL;
	}

	void LevelUp();
	void LevelDown();

	bool Save(ZFile* file);
	bool Load(ZFile* file, int nVersion);

	RMesh* GetWeaponMesh(MMatchCharItemParts parts);

	virtual float ColTest(const rvector& pos, const rvector& vec, float radius, rplane* out = 0) override;
	virtual bool IsAttackable() override;

	virtual bool IsGuard();
	virtual void OnMeleeGuardSuccess() override;

	virtual void OnDamagedAnimation(ZObject* pAttacker, int type) override;

	virtual ZOBJECTHITTEST HitTest(const rvector& origin, const rvector& to,
		float fTime, rvector* pOutPos = nullptr) override;

	virtual void OnKnockback(const rvector& dir, float fForce) override;
	virtual void OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType,
		float fDamage, float fPiercingRatio = 1.f, int nMeleeType = -1) override;
	virtual void OnScream() override;

	virtual void OnDamagedAPlayer(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio = 1.f, int nMeleeTpye = -1);
	virtual void OnDamagedAPlayer(ZObject* pAttacker, vector<MTD_ShotInfo*> vShots);

	int GetDTLastWeekGrade() { return m_MInitialInfo.Ref().nDTLastWeekGrade; }
	const MTD_CharInfo* GetCharInfo() const { return &m_MInitialInfo.Ref(); }
};

void ZChangeCharParts(RVisualMesh* pVMesh, MMatchSex nSex, int nHair, int nFace, const unsigned long int* pItemID);
void ZChangeCharWeaponMesh(RVisualMesh* pVMesh, unsigned long int nWeaponID);
bool CheckTeenVersionMesh(RMesh** ppMesh);

enum CHARACTER_LIGHT_TYPE
{
	GUN,
	SHOTGUN,
	CANNON,
	NUM_LIGHT_TYPE,
};

typedef struct
{
	int			iType;
	float		fLife;
	rvector		vLightColor;
	float		fRange;
}	sCharacterLight;

int gaepanEncode(int a, int depth);
int gaepanDecode(int a, int depth);

#endif