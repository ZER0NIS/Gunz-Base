#ifndef _ZCHARACTER_H
#define _ZCHARACTER_H

//#pragma	once

#include "MRTTI.h"
#include "ZCharacterObject.h"
#include "MUID.h"
#include "RTypes.h"
#include "RVisualMeshMgr.h"

#include "MObjectTypes.h"
//#include "MObjectCharacter.h"
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

#define MAX_SPEED			1000.f			// �ִ�ӵ�..
#define RUN_SPEED			630.f			// �޸��� �ӵ�
#define BACK_SPEED			450.f			// �ڳ� ������ ���� �ӵ�
#define ACCEL_SPEED			7000.f			// ���ӵ�
#define STOP_SPEED			3000.f			// �ƹ�Ű�� �ȴ������� ���ӵ�..
#define STOP_FORMAX_SPEED	7100.f			// �޸��� �ӵ� �̻����� �ö����� ���� �����ϴ� �ӵ�

#define CHARACTER_RADIUS	35.f		// ĳ���� �浹 ������
#define CHARACTER_HEIGHT	180.0f		// ĳ���� �浹 ����

#define ARRIVAL_TOLER		5.f

class ZShadow;

struct ZANIMATIONINFO {
	char *Name;
	bool bEnableCancel;		// ĵ�� ��������
	bool bLoop;				// �ݺ� �Ǵ� ����
	bool bMove;				// �������� ���Ե� �ִϸ��̼�
	bool bContinuos;		// ���Ե� �������� ���ۺ��� ����Ǿ��ִ���.
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
	ZC_SPMOTION_BOW ,
	ZC_SPMOTION_WAVE ,
	ZC_SPMOTION_LAUGH ,
	ZC_SPMOTION_CRY ,
	ZC_SPMOTION_DANCE ,

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

	// grenade type
	ZC_WEAPON_SP_GRENADE,
	ZC_WEAPON_SP_ROCKET,
	ZC_WEAPON_SP_FLASHBANG,
	ZC_WEAPON_SP_SMOKE,
	ZC_WEAPON_SP_TEAR_GAS,

	ZC_WEAPON_SP_ITEMKIT,	// medikit, repairkit, bulletkit ���

	ZC_WEAPON_SP_POTION,
	ZC_WEAPON_SP_TRAP,
	ZC_WEAPON_SP_DYNAMITE,
	ZC_WEAPON_SP_END,
};

enum ZSTUNTYPE {
	ZST_NONE	=	-1,
	ZST_DAMAGE1	=	0,
	ZST_DAMAGE2,
	ZST_SLASH,			// ������ ����
	ZST_BLOCKED,		// Į �������� ����
	ZST_LIGHTNING,		// ��æƮ�� Lightning
	ZST_LOOP,			// ��ų�� root �Ӽ�
};


class ZSlot {
public:
	ZSlot() {
		m_WeaponID = 0;
	}

	int m_WeaponID;
};

/// ĳ���� �Ӽ� - �� ���� ������ �ʴ´�.
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
	ZCharacterProperty_Old() :	nSex(MMS_MALE),
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

/// �޸��� �������� ���� ���� ĳ���� �Ӽ� - ���� ����ü�� ���÷��� ȣȯ������ ���ܵξ���
// ���� �Ӽ� ����ü���� �Ⱦ��� ��������� �����ȴ�
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
	MProtectValue<float>		fMaxHP;		// �� ���� �ٲٰ� �ڻ�->�������ؼ� HP�� ��Ƣ���ϴ� ����� ��ŷ
	MProtectValue<float>		fMaxAP;
	ZCharacterProperty() :	nSex(MMS_MALE),
		nHair(0),
		nFace(0),
		nLevel(1)
	{ 
		nameCharClan.Ref().szName[0] = 0;
		nameCharClan.Ref().szClanName[0] = 0;
		nameCharClan.MakeCrc();

		//fMaxHP.Set_MakeCrc(1000.f);
		//fMaxAP.Set_MakeCrc(1000.f);
		fMaxHP.Set_MakeCrc(DEFAULT_CHAR_HP);
		fMaxAP.Set_MakeCrc(DEFAULT_CHAR_AP);
	}
	void SetName(const char* name)	   { nameCharClan.CheckCrc(); strcpy(nameCharClan.Ref().szName, name);	   nameCharClan.MakeCrc(); }
	void SetClanName(const char* name) { nameCharClan.CheckCrc(); strcpy(nameCharClan.Ref().szClanName, name); nameCharClan.MakeCrc(); }
	const char* GetName() { return nameCharClan.Ref().szName; }
	const char* GetClanName() { return nameCharClan.Ref().szClanName; }

	void CopyToOldStruct(ZCharacterProperty_Old& old)	// ���÷��� �����
	{
		memcpy(old.szName, nameCharClan.Ref().szName, MATCHOBJECT_NAME_LENGTH);
		memcpy(old.szClanName, nameCharClan.Ref().szClanName, CLAN_NAME_LENGTH);
		
		old.nSex = nSex;
		old.nHair = nHair;
		old.nFace = nFace;
		old.nLevel = nLevel;
		old.fMaxHP = fMaxHP.Ref();		// TodoH(��) - ���÷��� ����..
		old.fMaxAP = fMaxAP.Ref();		// TodoH(��) - ���÷��� ����..
		old.nMoveSpeed = 100;
		old.nWeight = 0;
		old.nMaxWeight = 100;
		old.nSafeFall = 100;
	}

	void CopyFromOldStruct(ZCharacterProperty_Old& old)	// ���÷��� �����
	{
		memcpy(nameCharClan.Ref().szName, old.szName, MATCHOBJECT_NAME_LENGTH);
		memcpy(nameCharClan.Ref().szClanName, old.szClanName, CLAN_NAME_LENGTH);
		nameCharClan.MakeCrc();

		nSex = old.nSex;
		nHair = old.nHair;
		nFace = old.nFace;
		nLevel = old.nLevel;
		fMaxHP.Set_MakeCrc(old.fMaxHP);		// TodoH(��) - ���÷��� ����..
		fMaxAP.Set_MakeCrc(old.fMaxAP);		// TodoH(��) - ���÷��� ����..
	}
};

/// ĳ���� ���°�
struct ZCharacterStatus
{	// ������ ���� ���÷��� �ε� ����ؾ��մϴ�
	int			nLife;
	int			nKills;
	int			nDeaths;
	int			nLoadingPercent;	// ó�� �濡 ���ö� �ε��� �� �Ǿ������� �ۼ�Ʈ 100�� �Ǹ� �ε��� �ٵȰ�
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
	void AddDeaths(int nAddedDeaths = 1) { nDeaths += nAddedDeaths;  }
	void AddExp(int _nExp=1) { nExp += _nExp; }
};

// �̰��� ĳ���ͳ��� �ְ�޴� �����ͷ� ���߿� ��ǥ ������ �ٰŰ� �ȴ�.
/*
struct ZHPItem {
	MUID muid;
	float fHP;
};
*/

//struct ZHPInfoItem : public CMemPoolSm<ZHPInfoItem>{
//	ZHPInfoItem()	{ pHPTable=NULL; }
//	~ZHPInfoItem()	{ if(pHPTable) delete pHPTable; }
//	
//	int		nCount;
//	ZHPItem *pHPTable;
//};


//class ZHPInfoHistory : public list<ZHPInfoItem*> {
//};

#define CHARACTER_ICON_DELAY		2.f
#define CHARACTER_ICON_FADE_DELAY	.2f
#define CHARACTER_ICON_SIZE			32.f		// ������ ũ�� (640x480����)

class ZModule_HPAP;
class ZModule_QuestStatus;

// 1bit ��ŷ
struct ZCharaterStatusBitPacking { 
	union {
		struct {
			bool	m_bLand:1;				// ���� ���� ���� ����ִ���..
			bool	m_bWallJump:1;			// ������ ������
			bool	m_bJumpUp:1;			// �����ö󰡴���
			bool	m_bJumpDown:1;			// ����������
			bool	m_bWallJump2:1;			// �̰� walljump �Ŀ� ������ �ι�° ƨ���� ������ ����..
			bool	m_bTumble:1;			// ���� ��
			bool	m_bBlast:1;				// ��������Ҷ� ( �ö󰥶� )
			bool	m_bBlastFall:1;			// ������� ��������
			bool	m_bBlastDrop:1;			// �������� ���� ƨ�涧
			bool	m_bBlastStand:1;		// �Ͼ��
			bool	m_bBlastAirmove:1;		// ����ȸ���� ����
			bool	m_bSpMotion:1;
			bool	m_bCommander:1;			///< ����
		//	bool	m_bCharging:1;			// �������� �ִ���
		//	bool	m_bCharged:1;			// �����λ���
			bool	m_bLostConEffect:1;		// ��Ʈ�� ������ ������ �Ӹ��� �ߴ� ����Ʈ�� ���;� �ϴ���.
			bool	m_bChatEffect:1;		// ä�ý� �Ӹ��� �ߴ� ����Ʈ�� ���;� �ϴ���.
			bool	m_bBackMoving:1;		// �ڷ� �̵��Ҷ�

			bool	m_bAdminHide:1;					///< admin hide �Ǿ��ִ���..
			bool	m_bDie:1;						///< �׾����� üũ
			bool	m_bStylishShoted:1;				///< ���������� ��� ��Ÿ�ϸ��� �ߴ��� üũ
			bool	m_bFallingToNarak:1;			///< �������� �������� �ִ��� ����
			bool	m_bStun:1;						///< stun ..�����ϼ����ԵȻ���.
			bool	m_bDamaged:1;					///< ������ ����
				
			bool	m_bPlayDone:1;				// �ִϸ��̼� �÷��̰� �� �Ǿ�����. ������������ �Ѿ�� ����
			bool	m_bPlayDone_upper:1;		// ��ü �ִϸ��̼� �÷��̰� �� �Ǿ�����. ������������ �Ѿ�� ����
			bool	m_bIsLowModel:1;
			bool	m_bTagger:1;					///< ����
		};

		DWORD dwFlagsPublic;
	}; // ��ŷ ��
};

struct ZUserAndClanName
{
	char m_szUserName[MATCHOBJECT_NAME_LENGTH];			///< �����̸�(��ڴ� '���')
	char m_szUserAndClanName[MATCHOBJECT_NAME_LENGTH];  ///< �����̸�(Ŭ���̸�) ��� ��ĵ�ؼ� �ֵ��� �޸� ���� ���ϱ� ��� ���ܹ�����..
};


/// ĳ���� Ŭ����
class ZCharacter : public ZCharacterObject
{
	MDeclareRTTI;
	//friend class ZCharacterManager;
private:
protected:

	// ����. �ѹ� �����ǰ� �Ҹ�ɶ� ���� �����
	ZModule_HPAP			*m_pModule_HPAP;
	ZModule_QuestStatus		*m_pModule_QuestStatus;
	ZModule_Resistance		*m_pModule_Resistance;
	ZModule_FireDamage		*m_pModule_FireDamage;
	ZModule_ColdDamage		*m_pModule_ColdDamage;
	ZModule_PoisonDamage	*m_pModule_PoisonDamage;
	ZModule_LightningDamage	*m_pModule_LightningDamage;
	ZModule_HealOverTime	*m_pModule_HealOverTime;
	

	ZCharacterProperty					m_Property;		///< HP ���� ĳ���� �Ӽ�
	MProtectValue<ZCharacterStatus>		m_Status;		///< �÷��̾� ���°�

	MProtectValue<MTD_CharInfo>			m_MInitialInfo;		///< ĳ���� �ʱ�����


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ���� �ȿ����� �ɸ��� ���� ���� ���� �� �Լ�
protected:	
	//���������ӽ��ּ� ZCharacterBuff m_CharacterBuff;				///< ����ǰ� �ִ� ����

	float m_fPreMaxHP;
	float m_fPreMaxAP;

public:
	//���������ӽ��ּ� 
	/*
	void SetCharacterBuff(MTD_CharBuffInfo* pCharBuffInfo);
	ZCharacterBuff* GetCharacterBuff() { return &m_CharacterBuff;}
	*/

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

	__forceinline void SetHP(float nHP)	{ m_pModule_HPAP->SetHP(nHP); }
	__forceinline void SetAP(float nAP) { m_pModule_HPAP->SetAP(nAP); }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

protected:
	MProtectValue<ZUserAndClanName>*  m_pMUserAndClanName;  ///< ĳ����,Ŭ����

	struct KillInfo {
		int			m_nKillsThisRound;				///< �̹����忡���� kills ( unbelievable ����)
		float		m_fLastKillTime;				///< �������� ���� �ð� (excellent)�� ǥ���ϱ� ����
	};
	MProtectValue<KillInfo> m_killInfo;

	struct DamageInfo {
		DWORD			m_dwLastDamagedTime;		// ���������� ���� ���� �ð�
		ZSTUNTYPE		m_nStunType;				///< �´� �ִϸ��̼� ����.. 2:������Ÿ��,4:lightning,5:����
		ZDAMAGETYPE		m_LastDamageType;			///< ������ ������ Ÿ��		
		MMatchWeaponType m_LastDamageWeapon;		///< ������ ������ ����..
		rvector			m_LastDamageDir;			///< ������ ������ ���� ( �״� ����� ���� )
		float			m_LastDamageDot;
		float			m_LastDamageDistance;

		MUID			m_LastThrower;				///< ������ ��� ���
		float			m_tmLastThrowClear;			///< ������ ��� ��� �ؾ �Ǵ½ð�
	};
	MProtectValue<DamageInfo> m_damageInfo;

	int	m_nWhichFootSound;	///< �߼Ҹ��� ������ �������� ��� ������ ������ ����

	MProtectValue<DWORD>* m_pMdwInvincibleStartTime;		// ������ ���� �ð�
	MProtectValue<DWORD>* m_pMdwInvincibleDuration;		// ������ ���ӽð�

	virtual void UpdateSound();

	void InitMesh();	///< ĳ���� ���� ���� �޽����� ����. InitCharInfo���� ȣ��	


	void InitProperties();

//	float m_fIconStartTime[ZCI_END];	///< �Ӹ����� �ߴ� �����ܵ�

	void CheckLostConn();
	virtual void OnLevelDown();
	virtual void OnLevelUp();
	virtual void OnDraw();
	virtual void OnDie();

	void ShiftFugitiveValues();

public:
	float	m_fLastValidTime;		// Dead Reckoning�� �ʿ��� ���� -> ���� �ڵ忡�� �ʿ�����
	DWORD		m_dwIsValidTime;	//����� �������� ��ŷ �� ���� Ÿ�� üũ..��������..

//	float	m_fDistToFloor;			// �ٴڱ����� �Ÿ�
//	rplane	m_FloorPlane;			// �ٴ� ����� ������
//	float	m_fFallHeight;			// ���ϰ� ���۵� ����

	MProtectValue<ZCharaterStatusBitPacking> m_dwStatusBitPackingValue;	// ��� �ΰ������� crcüũ���� ���ϰڴ�...;;

	//mmemory proxy
	MProtectValue<bool>* m_bCharged;
	MProtectValue<bool>* m_bCharging;

	

	MProtectValue<float>	m_fChargedFreeTime;		// �����ΰ� Ǯ���� �ð�
	MProtectValue<int>		m_nWallJumpDir;			// �������ϴ� ����
	MProtectValue<int>		m_nBlastType;			// �ܵ��迭�߰�~


	ZC_STATE_LOWER	m_SpMotion;

	
	/*
	bool	m_bClimb;				// �ε �ö󰡰��ִ� ���
	rvector	m_ClimbDir;				// �ö󰡴� ����
	float	m_fClimbZ;				// �ö󰡱� ������ ����
	rplane	m_ClimbPlane;
	*/

//	bool	m_bRendered;				///< ���������ӿ��� ȭ�鿡 ���Դ���

	/////// ��Ʈ������ �ӽ÷� �� ���� �Ա� ���� ���� ���õ� ��������
	//int m_t_parts[6];	//����
	//int m_t_parts2[6];	//����
	
	
	// rvector		m_vProxyPosition, m_vProxyDirection;	///< �̵����� �ִ� �ִϸ��̼��� ������ ��ġ�� ���� ����
	
//	ZHPInfoHistory		m_HPHistory;		///< ��ǥ�� ���� ���ʰ��� �����͸� �������ִ´�

	ZCharacter();
	virtual ~ZCharacter();

	virtual bool Create(MTD_CharInfo* pCharInfo/*, MTD_CharBuffInfo* pCharBuffInfo*/);//���������ӽ��ּ� 
	virtual void Destroy();
	
	void InitMeshParts();
	
	void EmptyHistory();

	rvector m_TargetDir;
	rvector m_DirectionLower,m_DirectionUpper;

	// �� �������� �̵��ӵ� ��ŷ����� ��
	MProtectValue<rvector> m_RealPositionBefore;			// �ִϸ��̼��� �������� �����ϱ� ���� ����
	MProtectValue<rvector> m_AnimationPositionDiff;
	MProtectValue<rvector> m_Accel;


	MProtectValue<ZC_STATE_UPPER>	m_AniState_Upper;		// ��ü �ִϸ��̼� ����
	MProtectValue<ZC_STATE_LOWER>	m_AniState_Lower;		// ��ü �ִϸ��̼� ���� (�⺻)
	ZANIMATIONINFO *m_pAnimationInfo_Upper,*m_pAnimationInfo_Lower;

	void AddIcon(int nIcon);
//	float GetIconStartTime(int nIcon);

	MProtectValue<int>				m_nVMID;	// VisualMesh ID
	//MUID	m_UID;		// �������� �ο��� ĳ������ UID
	MProtectValue<MMatchTeam>		m_nTeamID;	// Team ID

	MProtectValue<MCharacterMoveMode>		m_nMoveMode;
	MProtectValue<MCharacterMode>			m_nMode;
	MProtectValue<MCharacterState>			m_nState;

//	RVisualMesh*			m_pVMesh;

//	float	m_fLastAdjustedTime;
	MProtectValue<float>	m_fAttack1Ratio;//Į������ ��� ù��°������ ����Ÿ���� �����Ѵ�..
//	rvector m_AdjustedNormal;

	/*
	bool	m_bAutoDir;

	bool	m_bLeftMoving,m_bRightMoving;
	bool	m_bForwardMoving;
	*/

//	float	m_fLastUpdateTime;
	float	m_fLastReceivedTime;	// basicinfo �����͸� ������ ���� �ð�

	float	m_fTimeOffset;				// ���� �� ĳ������ �ð�����
	float	m_fAccumulatedTimeError;	// ����ð��� ������ ����
	int		m_nTimeErrorCount;			// ����ð��� ������ ������ ȸ��


	float	m_fGlobalHP;			// �ٸ�������� ���� ��ĳ������ HP�� ���.. ��ǥ�� ����.
	int		m_nReceiveHPCount;		// ��ճ�������...

	/*
	float	m_fAveragePingTime;				// �����ð� ��� ��Ʈ�� ���� �ð�

	// �۷ι� �ð����� ���̸� �����Ѵ�. �� ���ڰ� ���� Ŀ���� ������ ���ǵ����� �ǽɵȴ�.
	#define TIME_ERROR_CORRECTION_PERIOD	20

	int		m_nTimeErrorCount;
	float	m_TimeErrors[TIME_ERROR_CORRECTION_PERIOD];
	float	m_fAccumulatedTimeError;

	list<float> m_PingData;			// ��� �� Ÿ���� ������ ����� ����.
	*/

//	DWORD m_dwBackUpTime;

	// ���� �߻�ӵ��� �̻������� �����Ѵ�.
	int		m_nLastShotItemID;
	float	m_fLastShotTime;

	void	SetInvincibleTime(int nDuration);
	bool	isInvincible();

	bool IsMan();

	virtual void  OnUpdate(float fDelta);
	virtual void  UpdateSpeed();
	virtual float GetMoveSpeedRatio();

	virtual void UpdateVelocity(float fDelta);
	virtual void UpdateHeight(float fDelta);
	virtual void UpdateMotion(float fDelta=0.f);
	virtual void UpdateAnimation();

	int  GetSelectWeaponDelay(MMatchItemDesc* pSelectItemDesc);

	void UpdateLoadAnimation();

	void Stop();
	//void DrawForce(bool bDrawShadow);	

	void CheckDrawWeaponTrack();
	void UpdateSpWeapon();

	void SetAnimation(char *AnimationName,bool bEnableCancel,int tick);
	void SetAnimation(RAniMode mode,char *AnimationName,bool bEnableCancel,int tick);

	void SetAnimationLower(ZC_STATE_LOWER nAni);
	void SetAnimationUpper(ZC_STATE_UPPER nAni);
	
	ZC_STATE_LOWER GetStateLower() { return m_AniState_Lower.Ref(); }
	ZC_STATE_UPPER GetStateUpper() { return m_AniState_Upper.Ref(); }

	bool IsUpperPlayDone()	{ return m_dwStatusBitPackingValue.Ref().m_bPlayDone_upper; }

	bool IsMoveAnimation();		// �������� ���Ե� �ִϸ��̼��ΰ� ?

//	bool IsRendered()		{ return m_bRendered; }

	bool IsTeam(ZCharacter* pChar);

	bool IsRunWall();
	bool IsMeleeWeapon();
	virtual bool IsCollideable();

//	void SetAnimationForce(ZC_STATE nState, ZC_STATE_SUB nStateSub) {}

	void SetTargetDir(rvector vDir); 

//	bool Pick(int x,int y,RPickInfo* pInfo);
//	bool Pick(int x,int y,rvector* v,float* f);
	virtual bool Pick(rvector& pos,rvector& dir, RPickInfo* pInfo = NULL);

//	bool Move(rvector &diff);

	void OnSetSlot(int nSlot,int WeaponID);
	void OnChangeSlot(int nSlot);

	virtual void OnChangeWeapon(char* WeaponModelName);
	void OnChangeParts(RMeshPartsType type,int PartsID);
	void OnAttack(int type,rvector& pos);
//	void OnHeal(ZCharacter* pAttacter,int type,int heal);
	void OnShot();

	void ChangeWeapon(MMatchCharItemParts nParts);

	int GetLastShotItemID()	{ return m_nLastShotItemID; }
	float GetLastShotTime()						{ return m_fLastShotTime; }
	bool CheckValidShotTime(int nItemID, float fTime, ZItem* pItem);
	void UpdateValidShotTime(int nItemID, float fTime)	
	{ 
		m_nLastShotItemID = nItemID;
		m_fLastShotTime = fTime;
	}

	bool IsDie() { return m_dwStatusBitPackingValue.Ref().m_bDie; }
	void ForceDie() 
	{
		SetHP(0); 
		m_dwStatusBitPackingValue.Ref().m_bDie = true; 
	}		// �̰��� �׳� ���� ���·� ����� ���Ҷ� ���

	void SetAccel(rvector& accel) { m_Accel.Set_CheckCrc(accel); }
	virtual void SetDirection(rvector& dir);

	struct SlotInfo
	{
		int		m_nSelectSlot;
		ZSlot	m_Slot[ZC_SLOT_END];
	};
	MProtectValue<SlotInfo>	m_slotInfo;

	MProtectValue<ZCharacterStatus>& GetStatus()	{ return m_Status; }

	// Character Property
	ZCharacterProperty* GetProperty() { return &m_Property; }

	MMatchUserGradeID GetUserGrade()	{ return m_MInitialInfo.Ref().nUGradeID; }
	unsigned int GetClanID()	{ return m_MInitialInfo.Ref().nClanCLID; }

	void SetName(const char* szName) { m_Property.SetName(szName); }

	const char *GetUserName()			{ return m_pMUserAndClanName->Ref().m_szUserName;	}		// ��ڴ� ó����
	const char *GetUserAndClanName()	{ return m_pMUserAndClanName->Ref().m_szUserAndClanName; }	// ��ڴ� Ŭ��ǥ�� ����
	bool IsAdminName();
	bool IsAdminHide()			{ return m_dwStatusBitPackingValue.Ref().m_bAdminHide;	}
	void SetAdminHide(bool bHide) { m_dwStatusBitPackingValue.Ref().m_bAdminHide = bHide; }
	
//	void SetMoveSpeed(int nMoveSpeed) { m_Property.nMoveSpeed = nMoveSpeed; }
//	void SetWeight(int nWeight) { m_Property.nWeight = nWeight; }
//	void SetMaxWeight(int nMaxWeight) { m_Property.nMaxWeight = nMaxWeight; }
//	void SetSafeFall(int nSafeFall) { m_Property.nSafeFall = nSafeFall; }

	int GetKils() { return GetStatus().Ref().nKills; }

	bool CheckDrawGrenade();

//	int GetWeaponEffectType();
//	float GetCurrentWeaponRange();
//	float GetMeleeWeaponRange();

	bool GetStylishShoted() { return m_dwStatusBitPackingValue.Ref().m_bStylishShoted; }
	void UpdateStylishShoted();
	
	MUID GetLastAttacker() { return m_pModule_HPAP->GetLastAttacker(); }
	void SetLastAttacker(MUID uid) { m_pModule_HPAP->SetLastAttacker(uid); }
	ZDAMAGETYPE GetLastDamageType() { return m_damageInfo.Ref().m_LastDamageType; }
	void SetLastDamageType(ZDAMAGETYPE type) { MEMBER_SET_CHECKCRC(m_damageInfo, m_LastDamageType, type); }

	bool DoingStylishMotion();
	
	bool IsObserverTarget();

	MMatchTeam GetTeamID() { return m_nTeamID.Ref(); }
	void SetTeamID(MMatchTeam nTeamID) { m_nTeamID.Set_CheckCrc(nTeamID); }
	bool IsSameTeam(ZCharacter* pCharacter) 
	{ 
		if (pCharacter->GetTeamID() == -1) return false;
		if (pCharacter->GetTeamID() == GetTeamID()) return true; return false; 
	}
	bool IsTagger() { return m_dwStatusBitPackingValue.Ref().m_bTagger; }
	void SetTagger(bool bTagger) { m_dwStatusBitPackingValue.Ref().m_bTagger = bTagger; }

	void SetLastThrower(MUID uid, float fTime) { MEMBER_SET_CHECKCRC(m_damageInfo, m_LastThrower, uid); MEMBER_SET_CHECKCRC(m_damageInfo, m_tmLastThrowClear, fTime); }
	const MUID& GetLastThrower() { return m_damageInfo.Ref().m_LastThrower; }
	float GetLastThrowClearTime() { return m_damageInfo.Ref().m_tmLastThrowClear; }

	// �����̳� �̺�Ʈ�� ���� �͵�.
	//void Damaged(ZCharacter* pAttacker, rvector& dir, RMeshPartsType partstype,MMatchCharItemParts wtype,int nCount=-1);
	//void DamagedGrenade(MUID uidOwner, rvector& dir, float fDamage,int nTeamID);
	//void DamagedFalling(float fDamage);
	//void DamagedKatanaSplash(ZCharacter* pAttacker,float fDamageRange);

	void Revival();
	void Die();
	void ActDead();
	__forceinline void InitHPAP();	
	virtual void InitStatus();
	virtual void InitRound();
	virtual void InitItemBullet();

	void TestChangePartsAll();
	void TestToggleCharacter();

	virtual void OutputDebugString_CharacterState();

	void ToggleClothSimulation();
	void ChangeLowPolyModel();
	bool IsFallingToNarak() { return m_dwStatusBitPackingValue.Ref().m_bFallingToNarak; }

	MMatchItemDesc* GetSelectItemDesc() {
		if(GetItems())
			if(GetItems()->GetSelectedWeapon())
				return GetItems()->GetSelectedWeapon()->GetDesc();
		return NULL;
	}

	void LevelUp();
	void LevelDown();

	bool Save(ZFile *file);
	bool Load(ZFile *file,int nVersion);	// ���߿� MZFile * �� ����

	RMesh *GetWeaponMesh(MMatchCharItemParts parts);

	virtual float ColTest(const rvector& pos, const rvector& vec, float radius, rplane* out=0);
	virtual bool IsAttackable();

	virtual bool IsGuard();
	virtual void OnMeleeGuardSuccess();


	virtual void OnDamagedAnimation(ZObject *pAttacker,int type);

	// ZObject�� �°� ���� �����̳� �̺�Ʈ�� ���� �͵�.
	virtual ZOBJECTHITTEST HitTest( const rvector& origin, const rvector& to, float fTime ,rvector *pOutPos=NULL );

	virtual void OnKnockback(rvector& dir, float fForce);
//	virtual void OnDamage(int damage, float fRatio = 1.0f);
	virtual void OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio=1.f, int nMeleeType=-1);
	virtual void OnDamagedAPlayer(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio=1.f, int nMeleeTpye=-1);
	virtual void OnDamagedAPlayer(ZObject* pAttacker, vector<MTD_ShotInfo*> vShots);
	virtual void OnScream();

	int GetDTLastWeekGrade() { return m_MInitialInfo.Ref().nDTLastWeekGrade; }
	const MTD_CharInfo* GetCharInfo() const { return &m_MInitialInfo.Ref(); }
};

void ZChangeCharParts(RVisualMesh* pVMesh, MMatchSex nSex, int nHair, int nFace, const unsigned long int* pItemID);
void ZChangeCharWeaponMesh(RVisualMesh* pVMesh, unsigned long int nWeaponID);
bool CheckTeenVersionMesh(RMesh** ppMesh);

//dll-injection���� ȣ������ ���ϵ��� ���� �ζ��̴�
__forceinline void ZCharacter::InitHPAP()
{	
	m_pModule_HPAP->SetMaxHP(GetMaxHP());
	m_pModule_HPAP->SetMaxAP(GetMaxAP());

	m_pModule_HPAP->SetHP(GetMaxHP());
	m_pModule_HPAP->SetAP(GetMaxAP());

	/*
	m_pModule_HPAP->SetMaxHP(m_Property.fMaxHP.Ref());
	m_pModule_HPAP->SetMaxAP(m_Property.fMaxAP.Ref());

	m_pModule_HPAP->SetHP(m_Property.fMaxHP.Ref());
	m_pModule_HPAP->SetAP(m_Property.fMaxAP.Ref());
	*/
}

//////////////////////////////////////////////////////////////////////////
//	ENUM
//////////////////////////////////////////////////////////////////////////
enum CHARACTER_LIGHT_TYPE
{
	GUN,
	SHOTGUN,
	CANNON,
	NUM_LIGHT_TYPE,
};

//////////////////////////////////////////////////////////////////////////
//	STRUCT
//////////////////////////////////////////////////////////////////////////
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