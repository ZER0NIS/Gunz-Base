#ifndef _ZOBJECT_H
#define _ZOBJECT_H

#include "ZPrerequisites.h"
#include "MUID.h"
#include "RTypes.h"
#include "MRTTI.h"
#include "ZModule.h"
#include "ZModule_Movable.h"
#include "ZModule_HPAP.h"
#include "ZCharacterItem.h"

#include "MMemoryProxy.h"

#include <list>
#include <string>

using namespace std;

_USING_NAMESPACE_REALSPACE2

enum ZOBJECTHITTEST {
	ZOH_NONE = 0,
	ZOH_BODY = 1,
	ZOH_HEAD = 2,
	ZOH_LEGS = 3
};

enum ZDAMAGETYPE {
	ZD_NONE=-1,
	ZD_BULLET,
	ZD_MELEE,
	ZD_FALLING,
	ZD_EXPLOSION,
	ZD_BULLET_HEADSHOT,
	ZD_KATANA_SPLASH,
	ZD_HEAL,
	ZD_REPAIR,
	ZD_MAGIC,
	ZD_FIRE,
	ZD_COLD,
	ZD_LIGHTNING,
	ZD_POISON,

	ZD_END
};

enum ZC_ENCHANT {
	ZC_ENCHANT_NONE = 0,
	ZC_ENCHANT_FIRE,
	ZC_ENCHANT_COLD,
	ZC_ENCHANT_LIGHTNING,
	ZC_ENCHANT_POISON,

	ZC_ENCHANT_END
};

/// 오브젝트 충돌에 필요한 값
struct ZObjectCollision
{
private:
	MProtectValue<bool>		bCollideable;		///< 충돌여부
	MProtectValue<float>	fRadius;			///< 반지름
	MProtectValue<float>	fHeight;			///< 높이
public:
	ZObjectCollision() {
		bCollideable.Set_MakeCrc(true);
		fRadius.Set_MakeCrc(0);
		fHeight.Set_MakeCrc(0);
	}

	float GetRadius()		{ return fRadius.Ref(); }
	float GetHeight()		{ return fHeight.Ref(); }
	
	void SetRadius(float r)		{ fRadius.Set_CheckCrc(r); }
	void SetHeight(float h)		{ fHeight.Set_CheckCrc(h); }

	bool IsCollideable()		{ return bCollideable.Ref(); }
	void SetCollideable(bool b)	{ bCollideable.Set_CheckCrc(b); }

	void ShiftFugitiveValues() {
		bCollideable.ShiftHeapPos_CheckCrc();
		fRadius.ShiftHeapPos_CheckCrc();
		fHeight.ShiftHeapPos_CheckCrc();
	}
};

/// 게임상의 모든 오브젝트 부모 클래스
class ZObject : public ZModuleContainer
{
	MDeclareRTTI;

public:
	// 핵 제작을 방해하기 위해 멤버변수의 위치를 빌드때마다 뒤섞기 위한 주석매크로(runtime/ShuffleCode.bat 실행)
	// m_Position과 m_Direction은 위조 커맨드를 만들고, 무한 ZPostShot 등의 핵을 만들 때 이용된다
	/* [[SHUFFLE_LINE]] ZObject */	ZModule_Movable			*m_pModule_Movable;	// 속도및 움직임을 관리하는 모듈

	/* [[SHUFFLE_LINE]] ZObject */	bool					m_bRendered;		///< 화면에 그려졌는지 여부

	/* [[SHUFFLE_LINE]] ZObject */	ZCharacterItem			m_Items;			///< 장비하고 있는 아이템

	/* [[SHUFFLE_LINE]] ZObject */	MProtectValue<rvector>	m_Position;			//mmemory proxy

	/* [[SHUFFLE_LINE]] ZObject */	RVisualMesh*			m_pVMesh;

	/* [[SHUFFLE_LINE]] ZObject */	rvector					m_Direction;

private:
	bool					m_bVisible;			///< 화면에 그릴지 여부
	float					m_fSpawnTime;		///< 태어난 시간
	float					m_fDeadTime;		///< 죽은시간
protected:
	MUID					m_UID;				///< 서버에서 부여한 유니크 ID(중요)
	bool					m_bIsNPC;			///< NPC인지 여부
	bool					m_bInitialized;		///< 초기화되었는지 여부
	bool					m_bInitialized_DebugRegister;	//jintriple3 디버그 레지스터 땜에..ㅠ

	ZObjectCollision		m_Collision;

	virtual void OnDraw();
	virtual void OnUpdate(float fDelta);
	virtual void OnDie() {} // 죽을때

public:
	ZObject();
	virtual ~ZObject();

	__forceinline const rvector& GetPosition() const
	{ 
		return m_Position.Ref();
	}
	
	//충돌과 관련있는 것들..메모리 프록시....
	float GetCollRadius()
	{
		return m_Collision.GetRadius(); 
	}
	float GetCollHeight()				
	{
		return m_Collision.GetHeight(); 
	}
	rvector GetCenterPos()				
	{ 
		return GetPosition() + rvector(0.0f, 0.0f, m_Collision.GetHeight()*0.5f);
	}
	virtual bool IsCollideable() 
	{ 
		return m_Collision.IsCollideable();
	}		

	__forceinline void SetPosition(const rvector& pos)
	{
		m_Position.Set_CheckCrc(pos);
	}

	void Draw();
	void Update(float fDelta);
	virtual bool Pick(int x,int y,RPickInfo* pInfo);
	virtual bool Pick(int x,int y,rvector* v,float* f);
	virtual bool Pick(rvector& pos,rvector& dir, RPickInfo* pInfo = NULL);
	virtual bool GetHistory(rvector *pos,rvector *direction,float fTime); // 판정/옵저버등 과거의 데이터를 얻는다

	// interface
	void SetVisualMesh(RVisualMesh* pVMesh);
	RVisualMesh* GetVisualMesh()					{ return m_pVMesh; }
	bool IsVisible()								{ return m_bVisible; }
	void SetVisible(bool bVisible)					{ m_bVisible = bVisible; }
	bool GetInitialized()							{ return m_bInitialized; }
	MUID& GetUID()									{ return m_UID; }
	void SetUID(MUID& uid)							{ m_UID = uid; }
	void SetSpawnTime(float fTime);
	float GetSpawnTime()							{ return m_fSpawnTime; }
	void SetDeadTime(float fTime);
	float GetDeadTime()								{ return m_fDeadTime; }
	bool IsNPC()									{ return m_bIsNPC; }
	const rvector& GetDirection()					{ return m_Direction; }

	// 움직임 모듈과 관계된것들
	const rvector& GetVelocity()		{ return m_pModule_Movable->GetVelocity(); }
	void SetVelocity(rvector& vel)		{ m_pModule_Movable->SetVelocity(vel); }
	void SetVelocity(float x,float y,float z) { SetVelocity(rvector(x,y,z)); }
	void AddVelocity(rvector& add)		{ SetVelocity(GetVelocity()+add); }

	float GetDistToFloor()				{ return m_pModule_Movable->GetDistToFloor(); }
	
	ZCharacterItem* GetItems()		{ return &m_Items; }

public:
	// 여기있는 것들은 상속받는 오브젝트에서 자신에 맞게 오버라이드해서 사용한다.
	virtual float ColTest(const rvector& pos, const rvector& vec, float radius, rplane* out=0) { return 1.0f; }
	virtual bool ColTest(const rvector& p1, rvector& p2, float radius, float fTime);		// 원기둥을 이용하여 간단하게 구와 충돌테스트
	virtual bool IsAttackable()	 { return true; }
	virtual bool IsDie() { return false; }
	virtual void SetDirection(rvector& dir);
	virtual bool IsGuard()	{ return false; }
	virtual MMatchTeam GetTeamID() { return MMT_ALL; }
	
	// 특정시점의 hit test 를 리턴해줘야 한다, pOutPos 가 있으면 hit된 위치를 리턴해줘야 한다
	virtual ZOBJECTHITTEST HitTest( const rvector& origin, const rvector& to, float fTime, rvector *pOutPos=NULL );

	// 액션에 관한 이벤트

	// 장검 오른쪽 띄우기
	virtual void OnBlast(rvector &dir)	{ }	
	virtual void OnBlastDagger(rvector &dir,rvector& pos) { }

	// 동작이나 이벤트에 관한 것들.
	virtual void OnGuardSuccess() { }
	virtual void OnMeleeGuardSuccess() { }
	
	// knockback을 적용받아야한다
	virtual void OnKnockback(rvector& dir, float fForce) { }

	// 맞을때 몸을 떨게 한다.
	// fValue : 강도, MaxTime : 최대시간 , nReturnMaxTime : 복귀시간
	void Tremble(float fValue, DWORD nMaxTime, DWORD nReturnMaxTime);

	// 실제로 피를 빼준다
//	virtual void OnDamage(int damage, float fRatio = 1.0f);

	// 데미지를 입었다.
	virtual void OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio=1.f, int nMeleeType=-1);

	// 모션등이 변경되지 않고 피만 줄이기위해..
	virtual void OnDamagedSkill(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio=1.f, int nMeleeType=-1);

	// 강베기를 맞았다
	virtual void OnDamagedAnimation(ZObject *pAttacker,int type) { }

	// 비명을 지른다
	virtual void OnScream() { }

	// HP/AP를 회복한다
	__forceinline void OnHealing(ZObject* pOwner,int nHP,int nAP);

	void OnSimpleDamaged(ZObject* pAttacker, float fDamage, float fPiercingRatio);

	// stun 된다.
	virtual void OnStun(float fTime)	{ }

public:
	// 땜빵을 위해 만들어논 virtual 함수 - 차차 정리하자
	virtual bool IsRendered() { return m_bRendered; }		/// 이전 프레임에서 화면에 그렸는지 여부

	// 메모리핵의 대상이 되는 변수들의 힙위치를 옮긴다
	void ShiftFugitiveValues();

protected:
	bool Move(rvector &diff)	{ return m_pModule_Movable->Move(diff); }

private:
	// 강제인라인을 위해 헤더로 옮긴 함수 중 외부 타입을 필요로하는 코드만 따로 뺀것
	void AddHealEffect();
};


/// 플레이어 오브젝트인지 여부
bool IsPlayerObject(ZObject* pObject);


// HP/AP를 회복한다
__forceinline void ZObject::OnHealing(ZObject* pOwner,int nHP, int nAP)
{
	if( nHP <= 0 && nAP <= 0 ) return;

	ZModule_HPAP *pModule = (ZModule_HPAP*)GetModule(ZMID_HPAP);
	if(!pModule) return;

	pModule->SetHP( min( pModule->GetHP() + nHP, pModule->GetMaxHP() ) );
	pModule->SetAP( min( pModule->GetAP() + nAP, pModule->GetMaxAP() ) );

	// TODO: 이펙트 추가. 임시로 달았음
	AddHealEffect();
}

#endif