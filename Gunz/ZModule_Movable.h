#ifndef _ZMODULE_MOVABLE_H
#define _ZMODULE_MOVABLE_H

#include "ZModule.h"
#include "ZModuleID.h"

class ZModule_Movable : public ZModule {
private:
	// 실린더의 반지름과 높이
//	float	m_fRadius;
//	float	m_fHeight;

	// 초기화만 하고 사용하지 않는 변수는 주석처리. 메모리핵을 위한 검색의 단서가 되기 때문
	//float	m_fMaxSpeed;	// 최고속도
	//bool	m_bGravity;		// 중력의 영향을 받는가?

	MProtectValue<float>	m_fDistToFloor;	/// 바닥까지의 거리
	MProtectValue<rplane>	m_FloorPlane;	/// 바닥 평면의 방정식

	rvector m_lastMove;		// 마지막으로 움직인 거리

	MProtectValue<bool>		m_bFalling;		// 낙하중이다
	MProtectValue<float>	m_fFallHeight;	// 낙하가 시작된 시점

	MProtectValue<bool>		m_bLanding;		// 이번에 착지했나
	MProtectValue<bool>		m_bAdjusted;	// 마지막 움직임이 (벽때문에) 비벼졌나
	MProtectValue<float>	m_fLastAdjustedTime;	// 마지막으로 비빈 시간

	rvector m_Velocity;		// 곧 private 으로 간다

	MProtectValue<bool>		m_bRestrict;		// 이속제한
	MProtectValue<float>	m_fRestrictTime;	// 제한이 걸린 시각
	MProtectValue<float>	m_fRestrictDuration;// 제한의 지속시간
	MProtectValue<float>	m_fRestrictRatio;	// 감속 비율

	MProtectValue<bool>		m_bHaste;			// 가속보너스
	MProtectValue<float>	m_fHasteTime;		// 가속이 걸린 시각
	MProtectValue<float>	m_fHasteDuration;	// 가속의 지속시간
	MProtectValue<float>	m_fHasteRatio;		// 가속 비율

	int		m_nHasteItemId;						// 가속을 일으킨 아이템 ID
	float	m_fNextHasteEffectTime;

	bool	m_bForceCollRadius35;	// 맵충돌검사시 radius를 35로 강제한다. 35는 플레이어 캐릭터의 radius.
									// (높낮이가 복잡한 맵은 npc radius가 클수록 충돌 오판정이 심하다, 달리 방도가 없으면
									// 이것으로 우회. 몬스터 옆구리가 벽에 파묻히는 부작용 있음)

protected:
	void OnAdd();

public:

	DECLARE_ID(ZMID_MOVABLE)
	ZModule_Movable();

	virtual bool Update(float fElapsed);
	virtual void InitStatus();

	const rvector &GetVelocity() { return m_Velocity; }
	void SetVelocity(rvector &vel) { m_Velocity=vel; }
	void SetVelocity(float x,float y,float z) { m_Velocity=rvector(x,y,z); }

	const rvector &GetLastMove() { return m_lastMove; }

	bool Move(rvector &diff);

	void UpdateGravity(float fDelta);

	float GetDistToFloor() { return m_fDistToFloor.Ref(); }

	float GetFallHeight() { return m_fFallHeight.Ref(); }
	bool isLanding() { return m_bLanding.Ref(); }
	bool isAdjusted() { return m_bAdjusted.Ref(); }
	float GetAdjustedTime() { return m_fLastAdjustedTime.Ref(); }

	float GetMoveSpeedRatio();
	void SetMoveSpeedRestrictRatio(float fRatio, float fDuration);
	void SetMoveSpeedHasteRatio(float fRatio, float fDuration, int nItemId);

	bool GetHasteBuffInfo(MTD_BuffInfo& out);

	void ForceCollRadius35(bool b) { m_bForceCollRadius35 = b; }
	
	void ShiftFugitiveValues();

//	void SetCollision(float fRadius, float fHeight) { m_fRadius = fRadius; m_fHeight = fHeight; }
	//void SetRadius(float fRadius) { m_fRadius = fRadius; }
//	float GetRadius()			{ return m_fRadius; }
//	float GetHeight()			{ return m_fHeight; }

protected:
	void UpdatePosition(float fDelta);
};

#endif