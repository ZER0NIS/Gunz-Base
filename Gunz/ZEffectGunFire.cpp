#include "stdafx.h"

#include "ZApplication.h"
#include "ZEffectGunFire.h"
#include "RealSpace2.h"
/*
ZEffectGunFire::ZEffectGunFire(ZEffectBillboardSource*	pSources[2], rvector& Pos, rvector& Dir,ZCharacter* pChar)
: m_Pos(Pos), m_Dir(Dir)
{
	m_pSources[0] = pSources[0];
	m_pSources[1] = pSources[1];
	m_nStartTime = timeGetTime();
	D3DXVec3Normalize(&m_Dir, &m_Dir);
	m_nDrawMode = ZEDM_ADD;
	m_nStartAddTime = 0;

	m_isLeftWeapon = false;

	if(pChar) {
		m_isMovingPos = true;
		m_uid = pChar->m_UID;
	}
	else {
		m_isMovingPos = false;
		m_uid = MUID(0,0);
	}

}

ZEffectGunFire::~ZEffectGunFire(void)
{
}

#define GUNFIRE_SCALE		25
#define GUNFIRE_LIFE_TIME	60
bool 
ZEffectGunFire::Draw(unsigned long int nTime)
{
	if(nTime < m_nStartTime + m_nStartAddTime) {
		return true;
	}

	if(m_pSources[0]==NULL) return false;
	if(m_pSources[1]==NULL) return false;

	if(m_bRender) {

		if( m_isMovingPos ) {

			ZCharacter* pCharacter = g_pGame->m_CharacterManager.Find(m_uid);

			if(pCharacter) {
				rvector pos;

				if(m_isLeftWeapon) {
					if(pCharacter->GetWeaponTypePosLeft(weapon_dummy_muzzle_flash,&pos) ) {
						m_Pos = pos;
					}
				}
				else {
					if(pCharacter->GetWeaponTypePos(weapon_dummy_muzzle_flash,&pos) ) {
						m_Pos = pos;
					}
				}
			}
		}
	
		// �ѱ� ���⿡ �����
		rvector Right, Dir, Up;
		D3DXVec3Cross(&Right, &m_Dir, &RealSpace2::RCameraDirection);
		D3DXVec3Cross(&Dir, &m_Dir, &Right);
		D3DXVec3Cross(&Up, &Dir, &m_Dir);
		D3DXVec3Normalize(&Dir, &Dir);
		D3DXVec3Normalize(&Up, &Up);

		m_pSources[0]->Draw(m_Pos+m_Dir*GUNFIRE_SCALE, Dir, Up, rvector(GUNFIRE_SCALE, GUNFIRE_SCALE, GUNFIRE_SCALE), 1.0f);
		// ī�޶� �����
		m_bisRendered = m_pSources[1]->Draw(m_Pos, -RealSpace2::RCameraDirection, RealSpace2::RCameraUp, rvector(GUNFIRE_SCALE, GUNFIRE_SCALE, GUNFIRE_SCALE), 1.0f);
	}
	else m_bisRendered = false;


	// ����!!!
	// �ʿ� ����Ʈ�� �׸��鼭 ������Ʈ�� �ٲٱ� ������ ���� ���־�� �Ѵ�.

	// Light
//	D3DCOLORVALUE d = {1, 1, 1, 1};
//	D3DCOLORVALUE z = {0, 0, 0, 0};
//	D3DLIGHT9 light;
//	light.Type = D3DLIGHT_POINT;
//	light.Diffuse = d;
//	light.Specular = d;
//	light.Ambient = d;
//	light.Position = m_Pos;
//	light.Direction = m_Dir;
//	light.Range = 800;	// cm
//	light.Falloff = 1.f;
//	light.Attenuation0 = 0.0f;
//	light.Attenuation1 = 0.02f;
//	light.Attenuation2 = 0.0f;
//	light.Theta = 0.0f;
//	light.Phi = 0.0f;

//	ZGetGame()->m_bsp.DrawLight(&light);

	if(timeGetTime() - m_nStartAddTime - m_nStartTime > GUNFIRE_LIFE_TIME) 
		return false;
	return true;
	//return false;	// �ѹ��� �׸�
}

#define GUNFIRE_SCALE2		50
#define GUNFIRE_LIFE_TIME2	60

ZEffectGunFire2::ZEffectGunFire2(ZEffectBillboardSource* pSources[4], rvector& Pos, rvector& Dir,ZCharacter* pChar)
: m_Pos(Pos), m_Dir(Dir)
{
	m_pSources[0] = pSources[0];
	m_pSources[1] = pSources[1];
	m_pSources[2] = pSources[2];
	m_pSources[3] = pSources[3];

	m_nStartTime = timeGetTime();
	D3DXVec3Normalize(&m_Dir, &m_Dir);
	m_nDrawMode = ZEDM_ADD;

	m_isLeftWeapon = false;

	if(pChar) {
		m_isMovingPos = true;
		m_uid = pChar->m_UID;
	}
	else {
		m_isMovingPos = false;
		m_uid = MUID(0,0);
	}
}

ZEffectGunFire2::~ZEffectGunFire2(void)
{
}

bool ZEffectGunFire2::Draw(unsigned long int nTime)
{
	ZEffectBillboardSource*	pSources1 = NULL;
	ZEffectBillboardSource*	pSources2 = NULL;

	if(timeGetTime()-m_nStartTime > GUNFIRE_LIFE_TIME2/2) {
		pSources1 = m_pSources[1];
		pSources2 = m_pSources[3];
	}
	else {
		pSources1 = m_pSources[0];
		pSources2 = m_pSources[2];
	}

	if(m_bRender) {

		if( m_isMovingPos ) {

			ZCharacter* pCharacter = g_pGame->m_CharacterManager.Find(m_uid);

			if(pCharacter) {
				rvector pos;

				if(m_isLeftWeapon) {
					if(pCharacter->GetWeaponTypePosLeft(weapon_dummy_muzzle_flash,&pos) ) {
						m_Pos = pos;
					}
				}
				else {
					if(pCharacter->GetWeaponTypePos(weapon_dummy_muzzle_flash,&pos) ) {
						m_Pos = pos;
					}
				}
			}
		}

		// �ѱ� ���⿡ �����
		rvector Right, Dir, Up;
		D3DXVec3Cross(&Right, &m_Dir, &RealSpace2::RCameraDirection);
		D3DXVec3Cross(&Dir, &m_Dir, &Right);
		D3DXVec3Cross(&Up, &Dir, &m_Dir);
		D3DXVec3Normalize(&Dir, &Dir);
		D3DXVec3Normalize(&Up, &Up);

		pSources1->Draw(m_Pos+m_Dir*GUNFIRE_SCALE2, Dir, Up, rvector(GUNFIRE_SCALE2, GUNFIRE_SCALE2, GUNFIRE_SCALE2), 1.0f);
		// ī�޶� �����
		m_bisRendered = pSources2->Draw(m_Pos, -RealSpace2::RCameraDirection, RealSpace2::RCameraUp, rvector(GUNFIRE_SCALE2, GUNFIRE_SCALE2, GUNFIRE_SCALE2), 1.0f);
	}
	else m_bisRendered = false;

	if(timeGetTime()-m_nStartTime > GUNFIRE_LIFE_TIME2)
		return false;
	return true;
}
*/
