#ifndef _ZMODULE_MOVABLE_H
#define _ZMODULE_MOVABLE_H

#include "ZModule.h"
#include "ZModuleID.h"

class ZModule_Movable : public ZModule {
private:
	MProtectValue<float>	m_fDistToFloor;
	MProtectValue<rplane>	m_FloorPlane;

	rvector m_lastMove;

	MProtectValue<bool>		m_bFalling;
	MProtectValue<float>	m_fFallHeight;

	MProtectValue<bool>		m_bLanding;
	MProtectValue<bool>		m_bAdjusted;
	MProtectValue<float>	m_fLastAdjustedTime;

	rvector m_Velocity;

	MProtectValue<bool>		m_bRestrict;
	MProtectValue<float>	m_fRestrictTime;
	MProtectValue<float>	m_fRestrictDuration;
	MProtectValue<float>	m_fRestrictRatio;

	MProtectValue<bool>		m_bHaste;
	MProtectValue<float>	m_fHasteTime;
	MProtectValue<float>	m_fHasteDuration;
	MProtectValue<float>	m_fHasteRatio;

	int		m_nHasteItemId;
	float	m_fNextHasteEffectTime;

	bool	m_bForceCollRadius35;
protected:
	void OnAdd();

public:

	DECLARE_ID(ZMID_MOVABLE)
	ZModule_Movable();

	virtual bool Update(float fElapsed);
	virtual void InitStatus();

	const rvector& GetVelocity() { return m_Velocity; }
	void SetVelocity(rvector& vel) { m_Velocity = vel; }
	void SetVelocity(float x, float y, float z) { m_Velocity = rvector(x, y, z); }

	const rvector& GetLastMove() { return m_lastMove; }

	bool Move(rvector& diff);

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

protected:
	void UpdatePosition(float fDelta);
};

#endif