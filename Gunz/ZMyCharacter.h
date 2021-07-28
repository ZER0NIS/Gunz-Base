#ifndef _ZMYCHARACTER_H
#define _ZMYCHARACTER_H

#include "MRTTI.h"
#include "ZCharacter.h"

enum ZChangeWeaponType
{
	ZCWT_NONE = 0,
	ZCWT_PREV,
	ZCWT_NEXT,
	ZCWT_MELEE,
	ZCWT_PRIMARY,
	ZCWT_SECONDARY,
	ZCWT_CUSTOM1,
	ZCWT_CUSTOM2,
	ZCWT_END,
};

enum ZUsingStamina {
	ZUS_Tumble = 0,
	ZUS_Jump,
};

enum ZDELAYEDWORK {
	ZDW_SHOT,
	ZDW_UPPERCUT,
	ZDW_DASH,
	ZDW_SLASH,
};

struct ZDELAYEDWORKITEM {
	float fTime;
	ZDELAYEDWORK nWork;
};

struct OVERLAP_FLOOR
{
	MUID	FloorUID;
	int		nFloorCnt;
	rvector	vecPosition;
	bool	bJumpActivity;
};

typedef list<ZDELAYEDWORKITEM*> ZDELAYEDWORKLIST;

struct ZMyCharaterStatusBitPacking
{
	union {
		struct {
			bool	m_bWallHang : 1;

			bool	m_bLimitJump : 1;
			bool	m_bLimitTumble : 1;
			bool	m_bLimitWall : 1;

			bool	m_bMoveLimit : 1;
			bool	m_bMoving : 1;

			bool	m_bReleasedJump : 1;
			bool	m_bJumpQueued : 1;
			bool	m_bWallJumpQueued : 1;
			bool	m_bHangSuccess : 1;
			bool	m_bSniferMode : 1;

			bool	m_bEnterCharge : 1;

			bool	m_bJumpShot : 1;
			bool	m_bShot : 1;
			bool	m_bShotReturn : 1;

			bool	m_bSkill : 1;
			bool	m_b1ShotSended : 1;

			bool	m_bGuard : 1;
			bool	m_bGuardBlock_ret : 1;
			bool	m_bGuardStart : 1;
			bool	m_bGuardCancel : 1;
			bool	m_bGuardKey : 1;
			bool	m_bGuardByKey : 1;
			bool	m_bDrop : 1;
			bool	m_bSlash : 1;
			bool	m_bJumpSlash : 1;
			bool	m_bJumpSlashLanding : 1;
			bool	m_bReserveDashAttacked : 1;

			bool	m_bLButtonPressed : 1;
			bool	m_bLButtonFirstPressed : 1;
			bool	m_bLButtonQueued : 1;

			bool	m_bRButtonPressed : 1;
			bool	m_bRButtonFirstPressed : 1;
			bool	m_bRButtonFirstReleased : 1;
		};

		DWORD dwFlags[2];
	};
};

class ZMyCharacter : public ZCharacter {
	MDeclareRTTI;
protected:
	virtual void OnDraw();
public:
#ifdef _DEBUG
	bool m_bGuardTest;
#endif

	struct ShotTimeInfo
	{
		float	m_fNextShotTimeType[MMCIP_END];
		float	m_fNextShotTime;
		float	m_fLastShotTime;
	};
	MProtectValue<ShotTimeInfo> m_shotTimeInfo;

	struct TimeInfo
	{
		float	m_fDeadTime;

		float	m_fWallJumpTime;
		float	m_fLastJumpPressedTime;
		float	m_fJump2Time;
		float	m_fHangTime;

		float	m_bJumpSlashTime;
		float	m_fGuardStartTime;
	};
	MProtectValue<TimeInfo> m_timeInfo;

	MProtectValue<ZMyCharaterStatusBitPacking> m_statusFlags;

	int		m_nTumbleDir;
	int		m_nWallJump2Dir;

	struct ButtonTime {
		float	m_fLastLButtonPressedTime;
		float	m_fLastRButtonPressedTime;
	};
	MProtectValue<ButtonTime> m_btnTime;

	MProtectValue<int>		m_nShot;
	MProtectValue<float>	m_f1ShotTime;
	MProtectValue<float>	m_fSkillTime;

	MProtectValue<int>		m_nGuardBlock;

	MProtectValue<float>		m_fDropTime;

	rvector	m_vReserveDashAttackedDir;
	float	m_fReserveDashAttackedTime;
	MUID	m_uidReserveDashAttacker;

	MProtectValue<float>	m_fStunEndTime;

	void WallJump2();

	ZMyCharacter();
	~ZMyCharacter();

	void InitSpawn();

	void ProcessInput(float fDelta);

	bool CheckWall(rvector& Pos);

	virtual void UpdateAnimation() override final;
	virtual void OnUpdate(float fTime) override;

	virtual void UpdateLimit();

	virtual void OnChangeWeapon(char* WeaponModelName) override;

	void Animation_Reload();

	void OnTumble(int nDir);
	virtual void OnBlast(rvector& dir) override final;
	void OnRecoil(int nControlability);
	void OnDashAttacked(rvector& dir);
	void ReserveDashAttacked(MUID uid, float time, rvector& dir);

	virtual void Revival() override final;
	virtual void InitStatus() override final;;
	virtual void InitRound() override final;;
	virtual void InitBullet() override final;

	virtual void SetDirection(const rvector& dir) override final;
	virtual void OnDamagedAnimation(ZObject* pAttacker, int type) override final;

	void OutputDebugString_CharacterState();

	float GetCAFactor() { return m_fCAFactor.Ref(); }
	bool IsGuard();

	void ShotBlocked();

	void ReleaseButtonState();

	OVERLAP_FLOOR* GetOverlapFloor() { return &m_OverlapFloor; }
private:
	MProtectValue<float> m_fCAFactor;
	MProtectValue<float> m_fElapsedCAFactorTime;
	OVERLAP_FLOOR m_OverlapFloor;

	ZDELAYEDWORKLIST m_DelayedWorkList;

	void OnDelayedWork(ZDELAYEDWORKITEM* pItem);
	void AddDelayedWork(float fTime, ZDELAYEDWORK nWork);
	void ProcessDelayedWork();

	virtual void OnDie() override final;
	void CalcRangeShotControllability(rvector& vOutDir, rvector& vSrcDir, int nControllability);
	float GetControllabilityFactor();
	void UpdateCAFactor(float fDelta);
	void ReleaseLButtonQueue();

	void UpdateButtonState();

	void ProcessShot();
	void ProcessGadget();
	void ProcessGuard();

	void OnGadget_Hanging();
	void OnGadget_Snifer();

	void OnShotMelee();
	void OnShotRange();
	void OnShotRocket();
	void OnShotItem();
	void OnShotCustom();

	void Charged();
	void EnterCharge();
	void Discharged();
	void ChargedShot();
	void JumpChargedShot();

	float GetGravityConst();

public:
	virtual void OnGuardSuccess();
	virtual void OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio = 1.f, int nMeleeType = -1);
	virtual void OnKnockback(rvector& dir, float fForce) override final;
	virtual void OnMeleeGuardSuccess() override final;
	virtual void OnStun(float fTime) override final;

protected:
	MProtectValue<unsigned long> m_nLastUseSpendableItemTime;
public:
	void	SetLastUseSpendableItemTime(unsigned long nValue) { m_nLastUseSpendableItemTime.Set_CheckCrc(nValue); }
	int		GetLastUseSpendableItemTime() { return m_nLastUseSpendableItemTime.Ref(); }

	void* MakeBuffEffectBlob();
};

#ifndef _PUBLISH
class ZDummyCharacter : public ZMyCharacter
{
private:
	float m_fNextAniTime;
	float m_fElapsedTime;

	float m_fNextShotTime;
	float m_fShotElapsedTime;

	float m_fShotDelayElapsedTime;

	bool m_bShotting;
	bool m_bShotEnable;
	virtual void  OnUpdate(float fDelta);
public:
	ZDummyCharacter();
	virtual ~ZDummyCharacter();

	void SetShotEnable(bool bEnable) { m_bShotEnable = bEnable; }
};
#endif

#endif