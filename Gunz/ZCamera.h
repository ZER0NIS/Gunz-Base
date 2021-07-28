#ifndef _ZCAMERA_H
#define _ZCAMERA_H

#include "RTypes.h"

_USING_NAMESPACE_REALSPACE2

#define CAMERA_DEFAULT_DISTANCE		290.f

#define CAMERA_DEFAULT_ANGLEX		(pi/2.f)
#define CAMERA_DEFAULT_ANGLEZ		(pi/4.f)

#define CAMERA_ANGLEX_MIN	0.01f
#define CAMERA_ANGLEX_MAX	(pi-0.01f)

#define CAMERA_DIST_MAX		1000.f
#define CAMERA_DIST_MIN		150.f

enum ZCAMERALOOKMODE
{
	ZCAMERA_DEFAULT = 0,
	ZCAMERA_FREEANGLE = 1,
	ZCAMERA_FREELOOK = 2,
	ZCAMERA_MINIMAP = 3,

	ZCAMERA_END
};

class ZCamera {
public:
	ZCamera();

	void Update(float fElapsed);

	float m_fAngleX;
	float m_fAngleZ;
	float m_fDist;

	bool  m_bAutoAiming;

	rvector m_Target;
	rvector	m_vCameraTrackOffset;

	void RecoilAngle(float fRecoilAngleX, float fRecoilAngleZ);
	void SetMaxRecoilAngleX(float fMax);

	rvector GetPosition() { return m_Position; }
	void SetPosition(rvector& pos) { m_Position = pos; }
	void SetDirection(const rvector& dir);

	bool m_bShocked;
	rvector m_CameraShockOffset;
	rvector m_CameraShockVelocity;
	float	m_fShockStartTime;
	float	m_fShockPower, m_fShockDuration;

	void Shock(float fPower, float fDuration, rvector& vDir);
	void StopShock();
	void Init();

	ZCAMERALOOKMODE	GetLookMode() { return m_nLookMode; }
	void SetLookMode(ZCAMERALOOKMODE mode);
	void SetNextLookMode();

private:
	ZCAMERALOOKMODE		m_nLookMode;

	float m_fRecoilAngleX;
	float m_fRecoilAngleZ;
	float m_fMaxRecoilAngleX;

	float m_fCurrentAngleX;
	float m_fCurrentAngleZ;

	rvector m_Position;
	rvector m_CurrentTarget;

	bool CheckCollisionWall(float& fRealDist, rvector& pos, rvector& dir);
	void CalcMaxPayneCameraZ(float& fRealDist, float& fAddedZ, float fAngleX);
	rvector GetCurrentDir();

public:
	float m_fCurrentDist;
	bool m_bCollision[4];
	float m_fPH;
};

#endif