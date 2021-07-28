#ifndef _ZCHARACTEROBJECT_H
#define _ZCHARACTEROBJECT_H

#include "ZObject.h"

enum ZC_STATE_UPPER {
	ZC_STATE_UPPER_NONE = 0,

	ZC_STATE_UPPER_SHOT,
	ZC_STATE_UPPER_RELOAD,
	ZC_STATE_UPPER_LOAD,

	ZC_STATE_UPPER_GUARD_START,
	ZC_STATE_UPPER_GUARD_IDLE,
	ZC_STATE_UPPER_GUARD_BLOCK1,
	ZC_STATE_UPPER_GUARD_BLOCK1_RET,
	ZC_STATE_UPPER_GUARD_BLOCK2,
	ZC_STATE_UPPER_GUARD_CANCEL,

	ZC_STATE_UPPER_END
};

enum ZC_STATE_LOWER {
	ZC_STATE_LOWER_NONE = 0,

	ZC_STATE_LOWER_IDLE1,
	ZC_STATE_LOWER_IDLE2,
	ZC_STATE_LOWER_IDLE3,
	ZC_STATE_LOWER_IDLE4,

	ZC_STATE_LOWER_RUN_FORWARD,
	ZC_STATE_LOWER_RUN_BACK,
	ZC_STATE_LOWER_RUN_LEFT,
	ZC_STATE_LOWER_RUN_RIGHT,

	ZC_STATE_LOWER_JUMP_UP,
	ZC_STATE_LOWER_JUMP_DOWN,

	ZC_STATE_LOWER_DIE1,
	ZC_STATE_LOWER_DIE2,
	ZC_STATE_LOWER_DIE3,
	ZC_STATE_LOWER_DIE4,

	ZC_STATE_LOWER_RUN_WALL_LEFT,
	ZC_STATE_LOWER_RUN_WALL_LEFT_DOWN,
	ZC_STATE_LOWER_RUN_WALL,
	ZC_STATE_LOWER_RUN_WALL_DOWN_FORWARD,
	ZC_STATE_LOWER_RUN_WALL_DOWN,
	ZC_STATE_LOWER_RUN_WALL_RIGHT,
	ZC_STATE_LOWER_RUN_WALL_RIGHT_DOWN,

	ZC_STATE_LOWER_TUMBLE_FORWARD,
	ZC_STATE_LOWER_TUMBLE_BACK,
	ZC_STATE_LOWER_TUMBLE_RIGHT,
	ZC_STATE_LOWER_TUMBLE_LEFT,

	ZC_STATE_LOWER_BIND,

	ZC_STATE_LOWER_JUMP_WALL_FORWARD,
	ZC_STATE_LOWER_JUMP_WALL_BACK,
	ZC_STATE_LOWER_JUMP_WALL_LEFT,
	ZC_STATE_LOWER_JUMP_WALL_RIGHT,

	ZC_STATE_LOWER_ATTACK1,
	ZC_STATE_LOWER_ATTACK1_RET,
	ZC_STATE_LOWER_ATTACK2,
	ZC_STATE_LOWER_ATTACK2_RET,
	ZC_STATE_LOWER_ATTACK3,
	ZC_STATE_LOWER_ATTACK3_RET,
	ZC_STATE_LOWER_ATTACK4,
	ZC_STATE_LOWER_ATTACK4_RET,
	ZC_STATE_LOWER_ATTACK5,

	ZC_STATE_LOWER_JUMPATTACK,
	ZC_STATE_LOWER_UPPERCUT,

	ZC_STATE_LOWER_GUARD_START,
	ZC_STATE_LOWER_GUARD_IDLE,
	ZC_STATE_LOWER_GUARD_BLOCK1,
	ZC_STATE_LOWER_GUARD_BLOCK1_RET,
	ZC_STATE_LOWER_GUARD_BLOCK2,
	ZC_STATE_LOWER_GUARD_CANCEL,

	ZC_STATE_LOWER_BLAST,
	ZC_STATE_LOWER_BLAST_FALL,
	ZC_STATE_LOWER_BLAST_DROP,
	ZC_STATE_LOWER_BLAST_STAND,
	ZC_STATE_LOWER_BLAST_AIRMOVE,

	ZC_STATE_LOWER_BLAST_DAGGER,
	ZC_STATE_LOWER_BLAST_DROP_DAGGER,

	ZC_STATE_DAMAGE,
	ZC_STATE_DAMAGE2,
	ZC_STATE_DAMAGE_DOWN,

	ZC_STATE_TAUNT,
	ZC_STATE_BOW,
	ZC_STATE_WAVE,
	ZC_STATE_LAUGH,
	ZC_STATE_CRY,
	ZC_STATE_DANCE,

	ZC_STATE_DAMAGE_BLOCKED,
	ZC_STATE_CHARGE,
	ZC_STATE_SLASH,
	ZC_STATE_JUMP_SLASH1,
	ZC_STATE_JUMP_SLASH2,

	ZC_STATE_DAMAGE_LIGHTNING,
	ZC_STATE_DAMAGE_STUN,

	ZC_STATE_PIT,

	ZC_STATE_LOWER_END
};

class ZShadow;

struct ZBasicInfo {
	rvector position;
	rvector velocity;
	rvector direction;
};

struct ZBasicInfoItem : public CMemPoolSm<ZBasicInfoItem>
{
	ZBasicInfo info;
	float	fReceivedTime;
	float	fSendTime;
};

class ZBasicInfoHistory : public list<ZBasicInfoItem*> {
};

class ZCharacterObject : public ZObject
{
	MDeclareRTTI;
private:
	float			m_fTremblePower;
public:
	ZCharacterObject();
	virtual ~ZCharacterObject();

public:

	bool CreateShadow();
	void DestroyShadow();

	bool GetWeaponTypePos(WeaponDummyType type, rvector* pos, bool bLeft = false);

	int GetWeapondummyPos(rvector* pVec);

	bool GetCurrentWeaponDirection(rvector* dir);

	void UpdateEnchant();

	void DrawEnchantSub(ZC_ENCHANT etype, rvector& pos);

	void DrawEnchant(ZC_STATE_LOWER AniState_Lower, bool bCharged);
	void EnChantWeaponEffect(ZC_ENCHANT etype, int nLevel);
	void EnChantSlashEffect(rvector* pOutPos, int cnt, ZC_ENCHANT etype, bool bDoubleWeapon);
	void EnChantMovingEffect(rvector* pOutPos, int cnt, ZC_ENCHANT etype, bool bDoubleWeapon);

	MMatchItemDesc* GetEnchantItemDesc();
	ZC_ENCHANT	GetEnchantType();

	void DrawShadow();
	void Draw_SetLight(const rvector& vPosition);

	bool IsDoubleGun();

	void SetHero(bool bHero = true) { m_bHero = bHero; }
	bool IsHero() { return m_bHero; }

	void EmptyHistory();
	virtual bool GetHistory(rvector* pos, rvector* direction, float fTime);

public:

	ZBasicInfoHistory	m_BasicHistory;

	char	m_pSoundMaterial[16];

	bool	m_bLeftShot;

	float	m_fTime;

	int		m_iDLightType;
	float	m_fLightLife;
	rvector	m_vLightColor;

	ZShadow* m_pshadow;
	bool		m_bDynamicLight;

	virtual void OnKnockback(const rvector& dir, float fForce);
	void SetTremblePower(float fPower) { m_fTremblePower = fPower; }

protected:

	bool	m_bHero;
};

#endif