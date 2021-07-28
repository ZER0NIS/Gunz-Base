#ifndef _ZACTOR_H
#define _ZACTOR_H

#include "MRTTI.h"
#include "ZCharacterObject.h"
#include "ZCharacter.h"
#include "MUID.h"
#include "RVisualMeshMgr.h"
#include "MObjectTypes.h"
#include "ZStateMachine.h"
#include "ZAI_Base.h"
#include "MBaseQuest.h"
#include "ZBrain.h"
#include "ZActorAnimation.h"
#include "ZTask.h"
#include "ZTaskManager.h"
#include "ZModule_HPAP.h"
#include "MMatchTransDataType.h"
#include "ZNPCInfoFromServer.h"

#include <list>
#include <string>
using namespace std;

_USING_NAMESPACE_REALSPACE2

enum ZACTOR_FLAG
{
	AF_NONE = 0,
	AF_LAND = 0x1,
	AF_BLAST = 0x2,
	AF_MOVING = 0x4,
	AF_DEAD = 0x8,
	AF_REQUESTED_DEAD = 0x10,
	AF_BLAST_DAGGER = 0x20,

	AF_MY_CONTROL = 0x100,

	AF_SOUND_WOUNDED = 0x1000,
};

struct ZACTOR_BASICINFO {
	float			fTime;
	MUID			uidNPC;
	short			posx, posy, posz;
	short			velx, vely, velz;
	short			dirx, diry, dirz;
	BYTE			anistate;
};

struct MQuestNPCInfo;

class ZActor : public ZCharacterObject
{
	MDeclareRTTI;

	friend ZBrain;
	friend ZActorAnimation;
private:
	unsigned long int		m_nFlags;
	void UpdateHeight(float fDelta);
	void UpdatePosition(float fDelta);
protected:
	float					m_fTC;
	int						m_nQL;
	MMatchItemDesc			m_ItemDesc;
	MQuestNPCInfo* m_pNPCInfo;
	ZActorAnimation			m_Animation;
	ZBrain* m_pBrain;
	ZTaskManager			m_TaskManager;
	float					m_TempBackupTime;
	float					m_fSpeed;
	int						m_nDamageCount;
	bool					m_bReserveStandUp;
	DWORD					m_dwStandUp;
private:
	void InitFromNPCType(MQUEST_NPC nNPCType, float fTC, int nQL);
	void InitMesh(char* szMeshName, MQUEST_NPC nNPCType);
	void OnTaskFinished(ZTASK_ID nLastID);
	static void OnTaskFinishedCallback(ZActor* pActor, ZTASK_ID nLastID);
	inline static int CalcMaxHP(int nQL, int nSrcHP);
	inline static int CalcMaxAP(int nQL, int nSrcAP);
protected:
	enum ZACTOR_LASTTIME
	{
		ACTOR_LASTTIME_HPINFO = 0,
		ACTOR_LASTTIME_BASICINFO,
		ACTOR_LASTTIME_MAX
	};
	unsigned long int	m_nLastTime[ACTOR_LASTTIME_MAX];

	ZModule_HPAP* m_pModule_HPAP;
	ZModule_Resistance* m_pModule_Resistance;
	ZModule_FireDamage* m_pModule_FireDamage;
	ZModule_ColdDamage* m_pModule_ColdDamage;
	ZModule_PoisonDamage* m_pModule_PoisonDamage;
	ZModule_LightningDamage* m_pModule_LightningDamage;
	ZModule_Skills* m_pModule_Skills;

	rvector				m_vAddBlastVel;
	float				m_fAddBlastVelTime;
	rvector				m_TargetDir;
	rvector				m_Accel;
	float				m_fDelayTime;

	MQUEST_NPC			m_nNPCType;

protected:
	virtual void OnDraw() override;
	virtual void OnUpdate(float fDelta) override;

	virtual void InitProperty();
	virtual void InitStatus();
	virtual bool ProcessMotion(float fDelta);
	virtual void ProcessNetwork(float fDelta);
	virtual void ProcessAI(float fDelta);

	void ProcessMovement(float fDelta);
	void CheckDead(float fDelta);

	void PostBasicInfo();
	void PostBossHpAp();

public:
	ZActor();
	virtual ~ZActor() override;
	static ZActor* CreateActor(MQUEST_NPC nNPCType, float fTC, int nQL, bool bForceCollRadius35 = false);
	void InputBasicInfo(ZBasicInfo* pni, BYTE anistate);
	void InputBossHpAp(float fHp, float fAp);
	void Input(AI_INPUT_SET nInput);
	void SetMyControl(bool bMyControl);

	inline ZA_ANIM_STATE	GetCurrAni();
	inline void SetFlag(unsigned int nFlag, bool bValue);
	inline bool CheckFlag(unsigned int nFlag);
	inline void SetFlags(unsigned int nFlags);
	inline unsigned long GetFlags();
	inline bool IsMyControl();
	inline int GetHP();
	inline int GetAP();
	inline int GetMaxHP();
	inline int GetMaxAP();
	inline float GetTC();
	inline int GetQL();
	inline float GetHitRate();
	inline int GetActualHP();
	inline int GetActualAP();
	inline int GetActualMaxHP();
	inline int GetActualMaxAP();

	void RunTo(rvector& dir);
	void Stop(bool bWithAniStop = true);
	void RotateTo(rvector& dir);
	void OnNeglect(int nNum);

	virtual void OnBlast(rvector& dir) override;
	virtual void OnBlastDagger(rvector& dir, rvector& pos) override;
	virtual bool IsCollideable() override;
	virtual bool IsAttackable() override;
	virtual void Attack_Melee();
	virtual void Attack_Range(rvector& dir);
	virtual void Skill(int nSkill);

	bool isThinkAble();

	ZBrain* GetBrain() { return m_pBrain; }
	MQuestNPCInfo* GetNPCInfo() { return m_pNPCInfo; }
	ZTaskManager* GetTaskManager() { return &m_TaskManager; }

	virtual ZOBJECTHITTEST HitTest(const rvector& origin, const rvector& to, float fTime, rvector* pOutPos = NULL)  override;;

	virtual bool IsDie() override;

	virtual MMatchTeam GetTeamID() const override { return MMT_BLUE; }

	virtual void OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType,
		MMatchWeaponType weaponType, float fDamage, float fPiercingRatio = 1.f, int nMeleeType = -1) override;

	virtual void OnKnockback(const rvector& dir, float fForce) override;
	virtual void OnDie() override;
	virtual void OnPeerDie(MUID& uidKiller);

	bool IsDieAnimationDone();
	bool CanSee(ZObject* pTarget);
	bool CanAttackRange(ZObject* pTarget);
	bool CanAttackMelee(ZObject* pTarget, ZSkillDesc* pSkillDesc = NULL);

	MUID GetLastAttacker() { return m_pModule_HPAP->GetLastAttacker(); }

	char m_szOwner[64];
	float m_fLastBasicInfo;
	void SetOwner(const char* szOwner) { strcpy(m_szOwner, szOwner); }
};

void SetClientNPCInfoFromServerNPCInfo(MQuestNPCInfo* pClientNPCInfo, const MTD_NPCINFO* pServerNPCInfo);

inline const MTD_NPCINFO* GetMyActorServerNPCInfo(const MQUEST_NPC nNPCID)
{
	if (NULL == ZGetQuest())
	{
		return NULL;
	}

	return ZGetQuest()->GetNPCInfoFromServerMgr().GetNPCInfo(nNPCID);
}

inline void ZActor::SetFlags(unsigned int nFlags)
{
	if (m_nFlags != nFlags)
	{
		m_nFlags = nFlags;
	}
}

inline void ZActor::SetFlag(unsigned int nFlag, bool bValue)
{
	if (bValue) m_nFlags |= nFlag;
	else m_nFlags &= ~nFlag;
}

inline bool ZActor::CheckFlag(unsigned int nFlag)
{
	return ((m_nFlags & nFlag) != 0);
}

inline unsigned long ZActor::GetFlags()
{
	return m_nFlags;
}

inline ZA_ANIM_STATE ZActor::GetCurrAni()
{
	return m_Animation.GetCurrState();
}

inline int ZActor::GetHP()
{
	return m_pModule_HPAP->GetHP();
}

inline int ZActor::GetAP()
{
	return m_pModule_HPAP->GetAP();
}
inline int ZActor::GetMaxHP()
{
	return m_pModule_HPAP->GetMaxHP();
}

inline int ZActor::GetMaxAP()
{
	return m_pModule_HPAP->GetMaxAP();
}
inline float ZActor::GetTC()
{
	return m_fTC;
}

inline int ZActor::GetQL()
{
	return m_nQL;
}

inline float ZActor::GetHitRate()
{
	return (m_fTC * m_pNPCInfo->fAttackHitRate);
}

inline bool ZActor::IsMyControl()
{
	return CheckFlag(AF_MY_CONTROL);
}

inline int ZActor::GetActualHP()
{
	return (int)((float)m_pModule_HPAP->GetHP() * m_fTC);
}

inline int ZActor::GetActualAP()
{
	return (int)((float)m_pModule_HPAP->GetAP() * m_fTC);
}

inline int ZActor::GetActualMaxHP()
{
	const MTD_NPCINFO* pSvrQuestNPCInfo = GetMyActorServerNPCInfo(m_pNPCInfo->nID);
	if (NULL == pSvrQuestNPCInfo)
	{
		return 0;
	}

	return (int)((float)m_pModule_HPAP->GetMaxHP() * m_fTC);
}

inline int ZActor::GetActualMaxAP()
{
	const MTD_NPCINFO* pSvrQuestNPCInfo = GetMyActorServerNPCInfo(m_pNPCInfo->nID);
	if (NULL == pSvrQuestNPCInfo)
	{
		return 0;
	}

	return (int)((float)m_pModule_HPAP->GetMaxAP() * m_fTC);
}

inline int ZActor::CalcMaxHP(int nQL, int nSrcHP)
{
	return (int)(((float)nQL * 0.2f + 1) * nSrcHP);
}

inline int ZActor::CalcMaxAP(int nQL, int nSrcAP)
{
	return (int)(((float)nQL * 0.2f + 1) * nSrcAP);
}

#endif