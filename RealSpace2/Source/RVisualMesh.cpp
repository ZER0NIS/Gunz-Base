#include "stdafx.h"
#include "RMeshMgr.h"
#include "RVisualMeshMgr.h"
#include "MDebug.h"
#include "RealSpace2.h"
#include "MProfiler.h"
#include "RCharCloth.h"
#include <functional>
#include "MeshManager.h"
#include <numeric>

_USING_NAMESPACE_REALSPACE2
_NAMESPACE_REALSPACE2_BEGIN

#define __BP(i,n) ;
#define __EP(i) ;

AniFrameInfo::AniFrameInfo()
{
	m_isOncePlayDone = false;
	m_isPlayDone = false;
	m_bChangeAnimation = false;

	m_nReserveTime = 0;
	m_nFrame = 0;
	m_nAddFrame = 0;

	m_save_time = 0;
	m_1frame_time = 0;

	m_pAniSet = NULL;
	m_pAniSetNext = NULL;
	m_pAniSetReserve = NULL;

	m_bBlendAniSet = false;
	m_fMaxBlendTime = 1.0f;
	m_fCurrentBlendTime = 0.5f;
	m_dwBackupBlendTime = 0;

	m_fSpeed = 4.8f;

	m_SoundInfo.Clear();

	m_pAniIdEventSet = NULL;
	m_pAniNameEventSet = NULL;
}

void AniFrameInfo::ClearFrame()
{
	m_nFrame = 0;
	m_save_time = timeGetTime();
	m_pAniSetNext = NULL;
	m_pAniSetReserve = NULL;
	m_nReserveTime = 0;
}

void AniFrameInfo::Frame(RAniMode amode, RVisualMesh* pVMesh)
{
	if (pVMesh == NULL)	return;

	if (m_pAniSet == NULL) return;

	DWORD cur = timeGetTime();

	if (m_bChangeAnimation) {
		m_bChangeAnimation = false;
		m_save_time = cur;
		m_1frame_time = cur;
		m_isPlayDone = false;
		m_isOncePlayDone = false;
		if (m_pAniIdEventSet != NULL)
			m_pAniNameEventSet = m_pAniIdEventSet->GetAniNameEventSet(m_pAniSet->GetName());
		if (m_pAniNameEventSet != NULL)
		{
			m_vecCheckAniEvent.clear();
			m_pAniNameEventSet->m_AniNameEventSetIter = m_pAniNameEventSet->m_AniNameEventSet.begin();
			while (m_pAniNameEventSet->m_AniNameEventSetIter != m_pAniNameEventSet->m_AniNameEventSet.end())
			{
				m_vecCheckAniEvent.push_back(false);
				m_pAniNameEventSet->m_AniNameEventSetIter++;
			}
		}

		m_nFrame = m_nAddFrame;

		if (m_pAniSet->IsHaveSoundFile()) {
			RAniSoundInfo* pSInfo = &m_SoundInfo;

			if (strcmp(m_pAniSet->GetName(), "jumpD") != 0)
				pSInfo->isPlay = true;

			pSInfo->SetName(m_pAniSet->GetSoundFileName());
			pSInfo->SetRelatedToBsp(m_pAniSet->IsSoundRelatedToMap());
			pSInfo->SetPos(pVMesh->m_WorldMat._41, pVMesh->m_WorldMat._42, pVMesh->m_WorldMat._43);
		}
	}

	AnimationLoopType looptype = m_pAniSet->GetAnimationLoopType();
	int max_frame = m_pAniSet->GetMaxFrame();

	if (m_isPlayDone) {
		if (m_pAniSet->GetAnimationLoopType() == RAniLoopType_Loop)
		{
			if (m_pAniNameEventSet != NULL)
			{
				m_vecCheckAniEvent.clear();
				m_pAniNameEventSet->m_AniNameEventSetIter = m_pAniNameEventSet->m_AniNameEventSet.begin();
				while (m_pAniNameEventSet->m_AniNameEventSetIter != m_pAniNameEventSet->m_AniNameEventSet.end())
				{
					m_vecCheckAniEvent.push_back(false);
					m_pAniNameEventSet->m_AniNameEventSetIter++;
				}
			}

			if (m_pAniSet->IsHaveSoundFile()) {
				RAniSoundInfo* pSInfo = &m_SoundInfo;
				pSInfo->isPlay = true;
				pSInfo->SetName(m_pAniSet->GetSoundFileName());
				pSInfo->SetRelatedToBsp(m_pAniSet->IsSoundRelatedToMap());
				pSInfo->SetPos(pVMesh->m_WorldMat._41, pVMesh->m_WorldMat._42, pVMesh->m_WorldMat._43);
			}

			m_isPlayDone = false;

			if (m_pAniSetNext) {
				pVMesh->SetAnimation(amode, m_pAniSetNext);
				m_pAniSetNext = NULL;
			}
			else {
				if (looptype == RAniLoopType_OnceIdle) {
					m_nFrame = max_frame - 1;
					m_save_time = cur;
					m_1frame_time = cur;
					return;
				}
				else if (looptype == RAniLoopType_HoldLastFrame) {
					m_nFrame = max_frame - 1;
					m_isPlayDone = true;
					m_save_time = cur;
					m_1frame_time = cur;
					return;
				}
				else if (looptype == RAniLoopType_OnceLowerBody) {
					m_nFrame = max_frame - 1;
					m_pAniSet = NULL;
					m_save_time = cur;
					return;
				}
				else if (looptype == RAniLoopType_Normal) {
				}
				else if (looptype == RAniLoopType_Loop) {
				}
			}
		}
	}

	if (m_pAniSetReserve) {
		if (cur > m_nReserveTime) {
			pVMesh->SetAnimation(amode, m_pAniSetReserve);
			m_pAniSetReserve = NULL;
			m_nReserveTime = 0;
			m_save_time = cur;
			m_isPlayDone = false;
		}
	}

	int bf = 0;
	int ef = max_frame;

	DWORD delta;

	if (m_save_time == 0) {
		delta = 0;
		m_save_time = cur;
	}
	else
		delta = cur - m_1frame_time;

	m_nFrame += (int)(delta * m_fSpeed);

	if (bf != 0) {
		m_nFrame += bf;
	}

	if (m_nFrame >= ef) {
		if (looptype == RAniLoopType_HoldLastFrame) {
			m_nFrame = max_frame - 1;
		}
		else if (looptype == RAniLoopType_OnceIdle) {
			m_nFrame = max_frame - 1;
		}
		else if (looptype == RAniLoopType_OnceLowerBody) {
			m_nFrame = max_frame - 1;
		}
		else if (looptype == RAniLoopType_Loop) {
			if (ef != 0)
				m_nFrame %= ef;
		}

		m_isPlayDone = true;
		m_isOncePlayDone = true;
		m_save_time = cur;
	}

	m_1frame_time = cur;
	if (m_pAniNameEventSet != NULL)
	{
		m_pAniNameEventSet->m_AniNameEventSetIter = m_pAniNameEventSet->m_AniNameEventSet.begin();
		m_iterCheckAniEvent = m_vecCheckAniEvent.begin();
		while (m_pAniNameEventSet->m_AniNameEventSetIter != m_pAniNameEventSet->m_AniNameEventSet.end())
		{
			if (!(*m_iterCheckAniEvent))
			{
				if ((*(m_pAniNameEventSet->m_AniNameEventSetIter))->GetBeginFrame() <= m_nFrame)
				{
					(*(m_pAniNameEventSet->m_AniNameEventSetIter))->m_vPos.x = pVMesh->m_WorldMat._41;
					(*(m_pAniNameEventSet->m_AniNameEventSetIter))->m_vPos.y = pVMesh->m_WorldMat._42;
					(*(m_pAniNameEventSet->m_AniNameEventSetIter))->m_vPos.z = pVMesh->m_WorldMat._43;
					m_pEventFunc((*m_pAniNameEventSet->m_AniNameEventSetIter));
					(*m_iterCheckAniEvent) = true;
				}
			}
			m_pAniNameEventSet->m_AniNameEventSetIter++;
			m_iterCheckAniEvent++;
		}
	}
}

void RFrameTime::Start(float fMax, DWORD MaxTime, DWORD ReturnMaxTime) {
	m_fMaxValue = fMax;
	m_dwStartTime = timeGetTime();
	m_dwEndTime = m_dwStartTime + MaxTime;
	m_dwReturnMaxTime = ReturnMaxTime;
	m_bActive = true;
}

void RFrameTime::Stop() {
	m_fCurValue = 0.f;
	m_fMaxValue = 0.f;
	m_bActive = false;
	m_bReturn = false;
}

void RFrameTime::Update() {
	if (!m_bActive) return;

	DWORD dwThisTime = timeGetTime();

	if (dwThisTime > m_dwEndTime) {
		if (m_bReturn || (m_dwReturnMaxTime == 0)) {
			Stop();
		}
		else {
			Start(m_fMaxValue, m_dwReturnMaxTime, 0);
			m_bReturn = true;
		}
	}

	if (m_nType == 0) {
		if (m_bReturn) {
			m_fCurValue = m_fMaxValue - (((dwThisTime - m_dwStartTime) / float(m_dwEndTime - m_dwStartTime)) * m_fMaxValue);
		}
		else {
			m_fCurValue = ((dwThisTime - m_dwStartTime) / float(m_dwEndTime - m_dwStartTime)) * m_fMaxValue;
		}
	}
	else if (m_nType == 1) {
	}
}

RVisualLightMgr::RVisualLightMgr()
{
	for (int i = 0; i < VISUAL_LIGHT_MAX; i++) {
		m_LightEnable[i] = 0;
	}
}

int RVisualLightMgr::GetLightCount()
{
	int nCnt = 0;

	for (int i = 0; i < VISUAL_LIGHT_MAX; i++) {
		if (m_LightEnable[i]) nCnt++;
	}
	return nCnt;
}

void RVisualLightMgr::Clone(RVisualMesh* pVMesh)
{
	if (!pVMesh) return;

	for (int i = 0; i < VISUAL_LIGHT_MAX; i++)
	{
		pVMesh->m_LightMgr.m_Light[i] = m_Light[i];
		pVMesh->m_LightMgr.m_LightEnable[i] = m_LightEnable[i];
	}
}

void RVisualLightMgr::SetLight(int index, D3DLIGHT9* light, bool ShaderOnly)
{
	if (light) {
		m_Light[index] = *light;
		if (ShaderOnly)
			m_LightEnable[index] = 2;
		else
			m_LightEnable[index] = 1;
	}
	else {
		m_LightEnable[index] = 0;
	}
}

void RVisualLightMgr::UpdateLight()
{
	for (int i = 0; i < VISUAL_LIGHT_MAX; i++) {
		if (m_LightEnable[i] == 1) {
			RGetDevice()->SetLight(i, &m_Light[i]);
			RGetDevice()->LightEnable(i, TRUE);

			if (RShaderMgr::mbUsingShader)
			{
				RGetShaderMgr()->setLight(i, &m_Light[i]);
				RGetShaderMgr()->LightEnable(i, TRUE);
			}
		}
		else if (m_LightEnable[i] == 2)
		{
			RGetDevice()->LightEnable(i, FALSE);

			if (RShaderMgr::mbUsingShader)
			{
				RGetShaderMgr()->setLight(i, &m_Light[i]);
				RGetShaderMgr()->LightEnable(i, TRUE);
			}
		}
		else {
			RGetDevice()->LightEnable(i, FALSE);
		}
	}
}

RVisualMesh::RVisualMesh() {
	m_FrameInfo = new AniFrameInfo[ani_mode_end];

	m_pMesh = NULL;
	m_pLowPolyMesh = NULL;

	m_id = -1;

	m_nAnimationState = APState_Stop;

	m_vBMax = rvector(1.f, 1.f, 1.f);
	m_vBMin = rvector(-1.f, -1.f, -1.f);

	m_isScale = false;
	m_vScale = rvector(1.f, 1.f, 1.f);

	m_bIsRender = false;
	m_bIsRenderWeapon = false;
	m_bIsRenderFirst = true;

	m_isDrawWeaponState = false;

	for (int i = 0; i < eq_weapon_end; i++) {
		m_WeaponVisualMesh[i] = NULL;
	}

	for (int i = 0; i < eq_parts_end; i++) {
		m_WeaponPartInfo[i].Init();
	}

	for (int i = 0; i < weapon_dummy_end; i++) {
		D3DXMatrixIdentity(&m_WeaponDummyMatrix[i]);
		D3DXMatrixIdentity(&m_WeaponDummyMatrix2[i]);
	}

	m_SelectWeaponMotionType = eq_weapon_etc;
	m_pSelectWeaponMotionType_AntiHack = new RWeaponMotionType;
	*m_pSelectWeaponMotionType_AntiHack = eq_weapon_etc;

	m_vRotXYZ = rvector(0.f, 0.f, 0.f);

	m_vTargetPos = rvector(0.f, 0.f, 0.f);

	D3DXMatrixIdentity(&m_ToonUVMat);

	D3DXMatrixIdentity(&m_RotMat);

	D3DXMatrixIdentity(&m_UpperRotMat);

	D3DXMatrixScaling(&m_ScaleMat, 1.f, 1.f, 1.f);

	m_pTracks[0] = NULL;
	m_pTracks[1] = NULL;

	m_bDrawTracks = false;

	m_bDrawTracksMotion[0] = true;
	m_bDrawTracksMotion[1] = true;

	m_bCheckViewFrustum = true;
	m_bGrenadeRenderOnoff = true;
	m_bGrenadeFire = false;
	m_bAddGrenade = false;
	m_bDrawGrenade = true;

	m_fVis = 1.0f;

	m_vPos = rvector(0.f, 0.f, 0.f);
	m_vDir = rvector(0.f, 0.f, 0.f);
	m_vUp = rvector(0.f, 0.f, 0.f);

	D3DXMatrixIdentity(&m_WorldMat);

	m_bIsCharacter = false;
	m_bIsNpc = false;

	m_pBipMatrix = NULL;

	m_bRenderInstantly = false;

	m_pAniNodeTable = NULL;
	m_nAniNodeTableCnt = 0;

	m_bRenderMatrix = false;

	m_EnchantType = REnchantType_None;
	m_GrenadeFireTime = 0;

	m_pTOCCL = NULL;

	m_NPCBlendColor.r = 0.6f;
	m_NPCBlendColor.g = 0.6f;
	m_NPCBlendColor.b = 0.6f;
	m_NPCBlendColor.a = 1.0f;

	m_fUAniValue = 0.f;
	m_fVAniValue = 0.f;
	m_bUVAni = false;

	m_pCloth = NULL;
	m_bClothGame = false;
	m_fClothDist = 0.f;

	m_ToonTexture = NULL;
	m_bToonLighting = true;
	m_bToonTextureRender = false;
	m_bToonColor = 0xffffffff;

	m_bCalcBoxWithScale = false;
	m_bSkipRenderFaceParts = false;
}

RVisualMesh::~RVisualMesh()
{
	if (IsDynamicResourceLoad())
		GetMeshManager()->RemoveObject(this);

	Destroy();

	delete m_pSelectWeaponMotionType_AntiHack;
}

void RVisualMesh::Destroy()
{
	DestroyCloth();

	RemoveAllWeapon();

	if (m_pTracks[0]) {
		delete m_pTracks[0];
		m_pTracks[0] = NULL;
	}

	if (m_pTracks[1]) {
		delete m_pTracks[1];
		m_pTracks[1] = NULL;
	}

	if (m_pBipMatrix) {
		delete[] m_pBipMatrix;
		m_pBipMatrix = NULL;
	}

	if (m_pAniNodeTable) {
		delete[] m_pAniNodeTable;
		m_pAniNodeTable = NULL;
	}

	delete[] m_FrameInfo;
}

bool RVisualMesh::Create(RMesh* pMesh) {
	m_pMesh = pMesh;

	if (m_pMesh) {
		m_bIsCharacter = m_pMesh->m_isCharacterMesh;
		m_bIsNpc = m_pMesh->m_isNPCMesh;
	}

	if (m_bIsCharacter || m_bIsNpc) {
		if (m_pBipMatrix == NULL) {
			m_pBipMatrix = new rmatrix[eq_parts_pos_info_end];

			for (int i = 0; i < eq_parts_pos_info_end; i++) {
				D3DXMatrixIdentity(&m_pBipMatrix[i]);
			}
		}
	}

	return true;
}

bool RVisualMesh::BBoxPickCheck(int mx, int my)
{
	LPDIRECT3DDEVICE9 dev = RGetDevice();

	int sw = RGetScreenWidth();
	int sh = RGetScreenHeight();

	rvector pos, dir;

	rmatrix matProj = RProjection;

	rvector v;

	v.x = (((2.0f * mx) / sw) - 1) / matProj._11;
	v.y = -(((2.0f * my) / sh) - 1) / matProj._22;
	v.z = 1.0f;

	rmatrix m, matView = RView;

	D3DXMatrixInverse(&m, NULL, &matView);

	dir.x = v.x * m._11 + v.y * m._21 + v.z * m._31;
	dir.y = v.x * m._12 + v.y * m._22 + v.z * m._32;
	dir.z = v.x * m._13 + v.y * m._23 + v.z * m._33;

	pos.x = m._41;
	pos.y = m._42;
	pos.z = m._43;

	D3DXVec3Normalize(&dir, &dir);

	return BBoxPickCheck(pos, dir);
}

void BBoxSubCalc(D3DXVECTOR3* max, D3DXVECTOR3* min)
{
	float t;

	if (max->x < min->x) { t = max->x;	max->x = min->x; min->x = t; }
	if (max->y < min->y) { t = max->y;	max->y = min->y; min->y = t; }
	if (max->z < min->z) { t = max->z; max->z = min->z; min->z = t; }
}

bool RVisualMesh::BBoxPickCheck(const rvector& pos, const rvector& dir)
{
	rvector min, max;

	CalcBox();

	D3DXVec3TransformCoord(&min, &m_vBMax, &m_WorldMat);
	D3DXVec3TransformCoord(&max, &m_vBMin, &m_WorldMat);

	BBoxSubCalc(&max, &min);

	return D3DXBoxBoundProbe(&min, &max, &pos, &dir) ? true : false;
}

bool RVisualMesh::Pick(int x, int y, RPickInfo* pInfo)
{
	if (m_pMesh) {
		if (!BBoxPickCheck(x, y))
			return false;

		return m_pMesh->Pick(x, y, pInfo, &m_WorldMat);
	}
	return false;
}

bool RVisualMesh::Pick(rvector* vInVec, RPickInfo* pInfo)
{
	if (m_pMesh) {
		if (!BBoxPickCheck(vInVec[0], vInVec[1]))
			return false;

		return m_pMesh->Pick(vInVec, pInfo, &m_WorldMat);
	}
	return false;
}

bool RVisualMesh::Pick(const rvector& pos, const rvector& dir, RPickInfo* pInfo)
{
	if (m_pMesh) {
		if (!BBoxPickCheck(pos, dir))
			return false;

		return m_pMesh->Pick(pos, dir, pInfo, &m_WorldMat);
	}
	return false;
}

bool RVisualMesh::Pick(rvector& pos, rvector& dir, rvector* v, float* t)
{
	if (m_pMesh) {
		if (!BBoxPickCheck(pos, dir))
			return false;

		RPickInfo info;
		bool hr = m_pMesh->Pick(pos, dir, &info, &m_WorldMat);

		*v = info.vOut;
		*t = info.t;

		return hr;
	}
	return false;
}

void RVisualMesh::Play(RAniMode amode) {
	AniFrameInfo* pInfo = GetFrameInfo(amode);
	if (pInfo == NULL) return;

	m_nAnimationState = APState_Play;

	pInfo->m_isPlayDone = false;
	pInfo->m_isOncePlayDone = false;
	pInfo->m_save_time = timeGetTime();
}

void RVisualMesh::Stop(RAniMode amode) {
	AniFrameInfo* pInfo = GetFrameInfo(amode);
	if (pInfo == NULL) return;

	m_nAnimationState = APState_Stop;

	if (m_pMesh) {
		pInfo->m_nFrame = 0;
	}

	pInfo->m_isPlayDone = false;
}

void RVisualMesh::Pause(RAniMode amode) {
	AniFrameInfo* pInfo = GetFrameInfo(amode);
	if (pInfo == NULL) return;

	m_nAnimationState = APState_Pause;

	pInfo->m_isPlayDone = false;
}

int RVisualMesh::GetMaxFrame(RAniMode amode)
{
	AniFrameInfo* pInfo = GetFrameInfo(amode);
	if (pInfo == NULL) return 0;

	if (pInfo->m_pAniSet) {
		return pInfo->m_pAniSet->GetMaxFrame();
	}
	return 0;
}

void RVisualMesh::Frame(RAniMode amode)
{
	AniFrameInfo* pInfo = GetFrameInfo(amode);

	if (pInfo == NULL) return;

	pInfo->Frame(amode, this);
}

void RVisualMesh::Frame() {
	if (m_pMesh) {
		if (m_nAnimationState == APState_Play) {
			Frame(ani_mode_lower);
			Frame(ani_mode_upper);
		}
		else if (m_nAnimationState == APState_Pause) {
		}
	}
}

void RVisualMesh::RenderMatrix()
{
	m_bRenderMatrix = true;

	Render(false, false);

	m_bRenderMatrix = false;
}

void RVisualMesh::Render(ROcclusionList* pOCCL)
{
	m_pTOCCL = pOCCL;

	Render(false, false);

	m_pTOCCL = NULL;
}

void RVisualMesh::Render(bool low, bool render_buffer) {
	if (m_pMesh) {
		rboundingbox bbox;

		CalcBox();

		rboundingbox srcbox;

		m_bAddGrenade = false;

		{
			srcbox.vmax = m_vBMax;
			srcbox.vmin = m_vBMin;
		}

		TransformBox(&bbox, srcbox, m_WorldMat);

		if (m_fVis < 0.001f) {
			m_bIsRender = false;
			return;
		}

		if (m_bCheckViewFrustum) {
			if (isInViewFrustumWithZ(&bbox, RGetViewFrustum()) == false) {
				m_bIsRender = false;
				return;
			}
		}

		m_bIsRender = true;

		if (m_isScale) {
			m_pMesh->SetScale(m_vScale);
		}

		bool isRenderedLowPolyModel = false;

		__BP(199, "RVisualMeshMgr::Render::MeshRenderLow");

		AniFrameInfo* pAniLow = GetFrameInfo(ani_mode_lower);
		AniFrameInfo* pAniUp = GetFrameInfo(ani_mode_upper);

		m_FrameTime.Update();

		if (low) {
			if (m_pLowPolyMesh) {
				m_pLowPolyMesh->SetAnimation(pAniLow->m_pAniSet, pAniUp->m_pAniSet);
				m_pLowPolyMesh->SetFrame(pAniLow->m_nFrame, pAniUp->m_nFrame);
				m_pLowPolyMesh->SetMeshVis(m_fVis);
				m_pLowPolyMesh->SetVisualMesh(this);
				m_pLowPolyMesh->Render(&m_WorldMat, true);

				m_vBMax = m_pLowPolyMesh->m_vBBMax;
				m_vBMin = m_pLowPolyMesh->m_vBBMin;

				isRenderedLowPolyModel = true;
			}
		}

		__EP(199);

		__BP(201, "RVisualMeshMgr::Render::MeshRender");

		if (!isRenderedLowPolyModel) {
			m_pMesh->SetAnimation(pAniLow->m_pAniSet, pAniUp->m_pAniSet);

			m_pMesh->SetFrame(pAniLow->m_nFrame, pAniUp->m_nFrame);
			m_pMesh->SetMeshVis(m_fVis);

			m_pMesh->SetVisualMesh(this);
			m_pMesh->Render(&m_WorldMat, false);

			if (m_pMesh->GetPhysiqueMesh()) {
				m_vBMax = m_pMesh->m_vBBMaxNodeMatrix * 1.1f;
				m_vBMin = m_pMesh->m_vBBMinNodeMatrix * 1.1f;
			}
			else {
			}
		}

		__EP(201);

		RenderWeapon();
	}
}

bool RVisualMesh::UpdateSpWeaponFire()
{
	AniFrameInfo* pAniUp = GetFrameInfo(ani_mode_upper);

	if (!pAniUp) return false;

	m_bGrenadeRenderOnoff = true;

	if (m_bDrawGrenade) {
		if (pAniUp->m_pAniSet) {
			if (strcmp(pAniUp->m_pAniSet->GetName(), "attackS") == 0) {
				if (m_bGrenadeFire) {
					if (pAniUp->m_nFrame > 2 * 160) {
						m_bGrenadeRenderOnoff = false;
						m_bGrenadeFire = false;
						m_bAddGrenade = true;
						return true;
					}
				}
			}
		}
	}

	if (m_bGrenadeFire) {
		if (m_GrenadeFireTime + 70 < timeGetTime()) {
			m_bGrenadeRenderOnoff = false;
			m_bGrenadeFire = false;
			m_bAddGrenade = true;
			return true;
		}
	}

	return false;
}

void RVisualMesh::GetMotionInfo(int& sel_parts, int& sel_parts2, bool& bCheck, bool& bRender)
{
	AniFrameInfo* pAniUp = GetFrameInfo(ani_mode_upper);

	switch (m_SelectWeaponMotionType) {
	case eq_ws_dagger:
	{
		sel_parts = eq_parts_right_dagger;
	}
	break;

	case eq_wd_dagger:
	{
		sel_parts = eq_parts_right_dagger;
		sel_parts2 = eq_parts_left_dagger;
		bCheck = true;
	}
	break;

	case eq_wd_katana:
	{
		sel_parts = eq_parts_right_katana;
	}
	break;

	case eq_wd_sword:
	{
		sel_parts = eq_parts_right_sword;
	}
	break;

	case eq_wd_blade:
	{
		sel_parts = eq_parts_right_blade;
		sel_parts2 = eq_parts_left_blade;
		bCheck = true;
	}
	break;

	case eq_ws_pistol:
	{
		sel_parts = eq_parts_right_pistol;
	}
	break;

	case eq_wd_pistol:
	{
		sel_parts = eq_parts_right_pistol;
		sel_parts2 = eq_parts_left_pistol;
		bCheck = true;
	}
	break;

	case eq_ws_smg:
	{
		sel_parts = eq_parts_right_smg;
	}
	break;

	case eq_wd_smg:
	{
		sel_parts = eq_parts_right_smg;
		sel_parts2 = eq_parts_left_smg;
		bCheck = true;
	}
	break;

	case eq_wd_shotgun:
	{
		sel_parts = eq_parts_right_shotgun;
	}
	break;

	case eq_wd_rifle:
	{
		sel_parts = eq_parts_right_rifle;
	}
	break;

	case eq_wd_grenade:
	{
		if (m_bDrawGrenade) {
			bRender = m_bGrenadeRenderOnoff;
		}
		else {
			bRender = false;
		}

		sel_parts = eq_parts_right_grenade;
	}
	break;

	case eq_wd_item:
	{
		sel_parts = eq_parts_right_item;
	}
	break;

	case eq_wd_rlauncher:
	{
		sel_parts = eq_parts_right_rlauncher;
	}
	break;

	default:

		break;
	}
}

static int g_bDrawWeaponTrack[2][4] = {
	{2,1,0,3},
	{1,2,1,2},
};

void GetRenderTrack(int isMan, int nMotion, bool& left, bool& right)
{
}

void RVisualMesh::DrawEnchant(RVisualMesh* pVWMesh, int mode, rmatrix& m)
{
	return;

	if (m_EnchantType == REnchantType_None)
		return;

	if (m_EnchantType == REnchantType_Fire) {
		DrawEnchantFire(pVWMesh, mode, m);
	}
	else if (m_EnchantType == REnchantType_Cold) {
		DrawEnchantCold(pVWMesh, mode, m);
	}
	else if (m_EnchantType == REnchantType_Lightning) {
		DrawEnchantLighting(pVWMesh, mode, m);
	}
	else if (m_EnchantType == REnchantType_Poison) {
		DrawEnchantPoison(pVWMesh, mode, m);
	}
}

void RVisualMesh::SetSpRenderMode(ALPHAPASS ePass)
{
	if (m_pMesh) {
		m_pMesh->SetSpRenderMode(ePass);

		if (m_WeaponVisualMesh[m_SelectWeaponMotionType])
			m_WeaponVisualMesh[m_SelectWeaponMotionType]->SetSpRenderMode(ePass);
	}
}

void RVisualMesh::ClearPartInfo()
{
	for (int i = 0; i < eq_parts_end; i++) {
		m_WeaponPartInfo[i].isUpdate = false;
	}
}

int	RVisualMesh::GetLastWeaponTrackPos(rvector* pOutVec)
{
	int cnt = 0;

	if (m_pTracks[0]) {
		cnt += m_pTracks[0]->GetLastAddVertex(&pOutVec[0]);
	}

	if (m_pTracks[1]) {
		cnt += m_pTracks[1]->GetLastAddVertex(&pOutVec[cnt]);
	}

	return cnt;
}

static bool g_toggle = false;

void RVisualMesh::DrawEnchantFire(RVisualMesh* pVWMesh, int mode, rmatrix& m)
{
	static RFireEffectTexture m_FireEffectTexture;

	if (g_toggle == false) {
		m_FireEffectTexture.Create(RGetDevice(), 128, 128);
		g_toggle = true;
	}

	m_FireEffectTexture.ProcessFire(1);

	m_FireEffectTexture.UpdateTexture();

	LPDIRECT3DDEVICE9 dev = RGetDevice();

	if (m_WeaponVisualMesh[m_SelectWeaponMotionType]) {
		RVisualMesh* pVWMesh = m_WeaponVisualMesh[m_SelectWeaponMotionType];

		rvector vmax, vmin, vcenter;
		rvector vpos[10];

		vmax = pVWMesh->m_vBMax;
		vmin = pVWMesh->m_vBMin;

		vcenter = (vmax - vmin) / 2;

		vpos[0] = vmin + vcenter;
		vpos[1] = vmin + vcenter;

		vpos[0].y = vmax.y;
		vpos[1].y = vmin.y + 35.f;
		vpos[2] = vmax;
		vpos[3] = vmin;

		D3DXVec3TransformCoord(&vpos[0], &vpos[0], &m);
		D3DXVec3TransformCoord(&vpos[1], &vpos[1], &m);
		D3DXVec3TransformCoord(&vpos[2], &vpos[2], &m);
		D3DXVec3TransformCoord(&vpos[3], &vpos[3], &m);
		static RLVertex pVert[10];

		rvector add[2];

		add[0] = rvector(0, 0, 0);
		add[1] = add[0];
		DWORD color = 0xaf9f9f9f;

		pVert[0].p = vpos[0];
		pVert[0].color = color;
		pVert[0].tu = 0.0f;
		pVert[0].tv = 0.9f;

		pVert[1].p = vpos[1];
		pVert[1].color = color;
		pVert[1].tu = 1.0f;
		pVert[1].tv = 0.9f;

		pVert[2].p = vpos[2];
		pVert[2].p.x += add[0].x;
		pVert[2].p.y += add[0].y;
		pVert[2].p.z += 50.f;
		pVert[2].color = color;
		pVert[2].tu = 0.0f;
		pVert[2].tv = 0.0f;

		pVert[3].p = vpos[3];
		pVert[3].p.x += add[1].x;
		pVert[3].p.y += add[1].y;
		pVert[3].p.z += 50.f;
		pVert[3].color = color;
		pVert[3].tu = 1.0f;
		pVert[3].tv = 0.0f;

		static D3DXMATRIX _init_mat = GetIdentityMatrix();
		dev->SetTransform(D3DTS_WORLD, &_init_mat);

		dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

		dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		dev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		dev->SetRenderState(D3DRS_ALPHAREF, 0x08);
		dev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

		dev->SetRenderState(D3DRS_LIGHTING, FALSE);
		dev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

		dev->SetTexture(0, m_FireEffectTexture.GetTexture());
		dev->SetFVF(RLVertexType);

		dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, (LPVOID)pVert, sizeof(RLVertex));

		dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		dev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		dev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		dev->SetRenderState(D3DRS_LIGHTING, TRUE);
	}
}

void RVisualMesh::DrawEnchantCold(RVisualMesh* pVWMesh, int mode, rmatrix& m)
{
}

void RVisualMesh::DrawEnchantLighting(RVisualMesh* pVWMesh, int mode, rmatrix& m)
{
}

void RVisualMesh::DrawEnchantPoison(RVisualMesh* pVWMesh, int mode, rmatrix& m)
{
}

void RVisualMesh::RenderWeapon()
{
	if (m_WeaponVisualMesh[m_SelectWeaponMotionType]) {
		bool btrack_render = true;

		if (m_pMesh)
			if (m_pMesh->m_eRenderMode == PASS_DEPTH_ONLY)
				btrack_render = false;

		RVisualMesh* pVWMesh = m_WeaponVisualMesh[m_SelectWeaponMotionType];

		m_LightMgr.Clone(pVWMesh);

		bool b2hCheck = false;
		bool bRender = true;

		rmatrix m, m2;
		float vis, vis2;

		int sel_parts = 0;
		int sel_parts2 = 0;

		GetMotionInfo(sel_parts, sel_parts2, b2hCheck, bRender);

		m = m_WeaponPartInfo[sel_parts].mat * m_WorldMat;
		vis = m_WeaponPartInfo[sel_parts].vis;

		if (m_bCheckViewFrustum == false) {
			pVWMesh->SetCheckViewFrustum(false);
		}

		if (m_bRenderMatrix)
			pVWMesh->m_bRenderMatrix = true;

		pVWMesh->SetWorldMatrix(m);

		if (bRender && vis) {
			pVWMesh->SetVisibility(min(m_fVis, vis));
			pVWMesh->Render();
			m_bIsRenderWeapon = true;
		}
		else {
			pVWMesh->CalcBox();
			m_bIsRenderWeapon = false;
		}

		if (m_isScale) {
			pVWMesh->m_WeaponDummyMatrix[weapon_dummy_muzzle_flash] = m_ScaleMat * pVWMesh->m_WeaponDummyMatrix[weapon_dummy_muzzle_flash];
			pVWMesh->m_WeaponDummyMatrix[weapon_dummy_cartridge01] = m_ScaleMat * pVWMesh->m_WeaponDummyMatrix[weapon_dummy_cartridge01];
			pVWMesh->m_WeaponDummyMatrix[weapon_dummy_cartridge02] = m_ScaleMat * pVWMesh->m_WeaponDummyMatrix[weapon_dummy_cartridge02];
		}

		m_WeaponDummyMatrix[weapon_dummy_muzzle_flash] = pVWMesh->m_WeaponDummyMatrix[weapon_dummy_muzzle_flash] * m;
		m_WeaponDummyMatrix[weapon_dummy_cartridge01] = pVWMesh->m_WeaponDummyMatrix[weapon_dummy_cartridge01] * m;
		m_WeaponDummyMatrix[weapon_dummy_cartridge02] = pVWMesh->m_WeaponDummyMatrix[weapon_dummy_cartridge02] * m;

		if (m_bRenderMatrix)
			pVWMesh->m_bRenderMatrix = false;

		if (btrack_render) {
			DrawTracks(m_bDrawTracks, pVWMesh, 0, m);
			DrawEnchant(pVWMesh, 0, m);
		}

		if (b2hCheck) {
			m = m_WeaponPartInfo[sel_parts2].mat * m_WorldMat;
			vis2 = m_WeaponPartInfo[sel_parts2].vis;

			if (m_bRenderMatrix)
				pVWMesh->m_bRenderMatrix = true;

			pVWMesh->SetWorldMatrix(m);

			if (vis2) {
				pVWMesh->SetVisibility(min(m_fVis, vis2));
				pVWMesh->Render();
			}
			else {
				pVWMesh->CalcBox();
			}

			if (m_isScale) {
				pVWMesh->m_WeaponDummyMatrix2[weapon_dummy_muzzle_flash] = m_ScaleMat * pVWMesh->m_WeaponDummyMatrix2[weapon_dummy_muzzle_flash];
				pVWMesh->m_WeaponDummyMatrix2[weapon_dummy_cartridge01] = m_ScaleMat * pVWMesh->m_WeaponDummyMatrix2[weapon_dummy_cartridge01];
				pVWMesh->m_WeaponDummyMatrix2[weapon_dummy_cartridge02] = m_ScaleMat * pVWMesh->m_WeaponDummyMatrix2[weapon_dummy_cartridge02];
			}

			m_WeaponDummyMatrix2[weapon_dummy_muzzle_flash] = pVWMesh->m_WeaponDummyMatrix[weapon_dummy_muzzle_flash] * m;
			m_WeaponDummyMatrix2[weapon_dummy_cartridge01] = pVWMesh->m_WeaponDummyMatrix[weapon_dummy_cartridge01] * m;
			m_WeaponDummyMatrix2[weapon_dummy_cartridge02] = pVWMesh->m_WeaponDummyMatrix[weapon_dummy_cartridge02] * m;

			if (m_bRenderMatrix)
				pVWMesh->m_bRenderMatrix = false;

			if (btrack_render) {
				DrawTracks(m_bDrawTracks, pVWMesh, 1, m);
				DrawEnchant(pVWMesh, 0, m);
			}
		}
	}
}

void RVisualMesh::GetWeaponPos(rvector* pOut, bool bLeft)
{
	if (!pOut) return;

	RVisualMesh* pVWMesh = m_WeaponVisualMesh[m_SelectWeaponMotionType];

	if (!pVWMesh) return;

	rvector vmax, vmin, vcenter, p1, p2;

	vmax = pVWMesh->m_vBMax;
	vmin = pVWMesh->m_vBMin;

	vmax.y = 0;
	vmin.y = 0;

	vcenter = (vmax - vmin) / 2;

	p1 = pVWMesh->m_vBMax - vcenter;
	p2 = pVWMesh->m_vBMin + vcenter;

	bool b2hCheck = false;
	bool bRender = true;

	rmatrix m, m2;

	int sel_parts = 0;
	int sel_parts2 = 0;

	GetMotionInfo(sel_parts, sel_parts2, b2hCheck, bRender);

	if (bLeft)
		m = m_WeaponPartInfo[sel_parts2].mat * m_WorldMat;
	else
		m = m_WeaponPartInfo[sel_parts].mat * m_WorldMat;

	D3DXVec3TransformCoord(&p1, &p1, &m);
	D3DXVec3TransformCoord(&p2, &p2, &m);

	pOut[0] = p1;
	pOut[1] = p2;
}

float RVisualMesh::GetWeaponSize()
{
	RVisualMesh* pVWMesh = m_WeaponVisualMesh[m_SelectWeaponMotionType];

	if (!pVWMesh) return 0.f;

	float fWeaponSize = Magnitude(pVWMesh->m_vBMin - pVWMesh->m_vBMax);

	return fWeaponSize;
}

bool RVisualMesh::IsDoubleWeapon()
{
	int sel_parts = 0;
	int sel_parts2 = 0;
	bool bCheck = false;
	bool bRender = false;

	GetMotionInfo(sel_parts, sel_parts2, bCheck, bRender);

	return bCheck;
}

void RVisualMesh::GetEnChantColor(DWORD* color)
{
	if (!color) return;

	if (m_EnchantType == REnchantType_Fire) {
		color[0] = 0x4fff6666;
		color[1] = 0x0fff6666;
	}
	else if (m_EnchantType == REnchantType_Cold) {
		color[0] = 0x4f6666ff;
		color[1] = 0x0f6666ff;
	}
	else if (m_EnchantType == REnchantType_Lightning) {
		color[0] = 0x4f66ffff;
		color[1] = 0x0f66ffff;
	}
	else if (m_EnchantType == REnchantType_Poison) {
		color[0] = 0x4f66ff66;
		color[1] = 0x0f66ff66;
	}
	else {
		color[0] = 0x4fffffff;
		color[1] = 0x0fffffff;
	}
}

void RVisualMesh::DrawTracks(bool draw, RVisualMesh* pVWMesh, int mode, rmatrix& m)
{
	if (pVWMesh == NULL)	return;

	if (m_bRenderMatrix) return;

	if (mode < 0 || mode > 1)	return;

	if (draw && m_bDrawTracksMotion[mode]) {
		rvector vmax, vmin, vcenter;
		rvector vpos[2];

		vmax = pVWMesh->m_vBMax;
		vmin = pVWMesh->m_vBMin;

		vcenter = (vmax - vmin) / 2;

		vpos[0] = vmin + vcenter;
		vpos[1] = vmin + vcenter;

		vpos[0].y = vmax.y;
		vpos[1].y = vmin.y + 35.f;

		D3DXVec3TransformCoord(&vpos[0], &vpos[0], &m);
		D3DXVec3TransformCoord(&vpos[1], &vpos[1], &m);

		static RLVertex pVert[2];
		static DWORD color[2];

		GetEnChantColor(color);

		pVert[0].p = vpos[0];
		pVert[0].color = color[0];

		pVert[1].p = vpos[1];
		pVert[1].color = color[1];

		if (!m_pTracks[mode]) {
			m_pTracks[mode] = new RWeaponTracks;
			m_pTracks[mode]->Create(3);
		}

		m_pTracks[mode]->m_vSwordPos[0] = vpos[0];
		m_pTracks[mode]->m_vSwordPos[1] = vpos[1];

		m_pTracks[mode]->AddVertex(pVert);
		m_pTracks[mode]->Render();
	}
	else {
		if (m_pTracks[mode]) {
			m_pTracks[mode]->Clear();
		}
	}
}

rvector RVisualMesh::GetCurrentWeaponPosition(bool right)
{
	rmatrix m = GetCurrentWeaponPositionMatrix(right);

	rvector v;

	v.x = m._41;
	v.y = m._42;
	v.z = m._43;

	return v;
}

rmatrix RVisualMesh::GetCurrentWeaponPositionMatrix(bool right)
{
	int sel_parts = 0;
	int sel_parts2 = 0;
	bool bCheck = false;
	bool bRender = false;

	GetMotionInfo(sel_parts, sel_parts2, bCheck, bRender);

	rmatrix m;

	if (right && bCheck)
		m = m_WeaponPartInfo[sel_parts2].mat * m_WorldMat;
	else
		m = m_WeaponPartInfo[sel_parts].mat * m_WorldMat;

	return m;
}

bool RVisualMesh::CalcBox()
{
	_BP("RVisualMesh::CalcBox");

	if (m_pMesh) {
		m_pMesh->SetVisualMesh(this);

		if (m_pMesh->GetPhysiqueMesh()) {
			if (m_bIsRenderFirst) {
				if (m_bCalcBoxWithScale) {
					rmatrix _scale_mat;
					D3DXMatrixScaling(&_scale_mat, m_vScale.x, m_vScale.y, m_vScale.z);
					rmatrix worldmat_scl = _scale_mat * m_WorldMat;
					m_pMesh->CalcBoxNode(&worldmat_scl);
				}
				else {
					m_pMesh->CalcBoxNode(&m_WorldMat);
				}
				m_bIsRenderFirst = false;

				m_vBMax = m_pMesh->m_vBBMaxNodeMatrix * 1.2f;
				m_vBMin = m_pMesh->m_vBBMinNodeMatrix * 1.2f;
			}
		}
		else {
			if (m_bIsRenderFirst)
			{
				if (m_bCalcBoxWithScale) {
					rmatrix _scale_mat;
					D3DXMatrixScaling(&_scale_mat, m_vScale.x, m_vScale.y, m_vScale.z);
					rmatrix worldmat_scl = _scale_mat * m_WorldMat;
					m_pMesh->CalcBoxNode(&worldmat_scl);
				}
				else {
					m_pMesh->CalcBox(&m_WorldMat);
				}
				m_bIsRenderFirst = false;

				m_vBMax = m_pMesh->m_vBBMax;
				m_vBMin = m_pMesh->m_vBBMin;
			}
		}

		_EP("RVisualMesh::CalcBox");
		return true;
	}
	_EP("RVisualMesh::CalcBox");
	return false;
}

void RVisualMesh::SetPos(rvector pos, rvector dir, rvector up) {
	m_vPos = pos;
	m_vDir = dir;
	m_vUp = up;
}

void RVisualMesh::SetWorldMatrix(rmatrix& mat) {
	m_WorldMat = mat;
}

void RVisualMesh::AddWeapon(RWeaponMotionType type, RMesh* pMesh, RAnimation* pAni)
{
	if ((type < eq_weapon_etc) || (type > eq_weapon_end - 1)) {
		mlog("RVisualMesh::AddWeapon �ϰ� RVisualMesh �� ���Դ±���~~~\n");
	}

	RemoveWeapon(type);

	RVisualMesh* pVMesh = new RVisualMesh;
	pVMesh->Create(pMesh);

	if (pAni) {
		pVMesh->SetAnimation(ani_mode_upper, pAni);
		pVMesh->Stop();
	}

	m_WeaponVisualMesh[type] = pVMesh;
}

void RVisualMesh::RemoveWeapon(RWeaponMotionType type)
{
	if (m_WeaponVisualMesh[type]) {
		delete m_WeaponVisualMesh[type];
		m_WeaponVisualMesh[type] = NULL;
	}
}

void RVisualMesh::RemoveAllWeapon()
{
	for (int i = 0; i < eq_weapon_end; i++) {
		if (m_WeaponVisualMesh[i]) {
			delete m_WeaponVisualMesh[i];
			m_WeaponVisualMesh[i] = NULL;
		}
	}
}

void RVisualMesh::ClearParts()
{
	for (auto&& node : m_pTMesh)
		node.reset();
}

void RVisualMesh::SetParts(RMeshPartsType parts, const char* name)
{
	if (parts < 0 && parts >= eq_parts_end)
		return;

	if (!m_pMesh) return;

	auto& TMesh = m_pTMesh[parts];
	if (TMesh && strcmp(TMesh->GetName(), name) == 0)
		return;

	if (IsDynamicResourceLoad())
	{
		auto lambda = [this, parts](RMeshNodePtr Node, const char* NodeName)
		{
			if (!Node)
				Node = RMeshNodePtr{ m_pMesh->GetPartsNode(NodeName), {false} };

			if (!Node)
			{
				MLog("RVisualMesh::SetParts(): Failed to find parts %s\n", NodeName);
				return;
			}

			m_pTMesh[parts] = std::move(Node);
			m_pMesh->ConnectPhysiqueParent(m_pTMesh[parts].get());
		};
		GetMeshManager()->Get(m_pMesh->GetName(), name, this, lambda);
	}
	else
	{
		RMeshNode* pNode = m_pMesh->GetPartsNode(name);

		if (pNode)
		{
			m_pTMesh[parts] = RMeshNodePtr{ pNode };
			m_pMesh->ConnectPhysiqueParent(pNode);
		}
	}
}

RMeshNode* RVisualMesh::GetParts(RMeshPartsType parts)
{
	if (parts < 0 || parts >= eq_parts_end)
		return NULL;

	return m_pTMesh[parts].get();
}

void RVisualMesh::ClearFrame()
{
	for (int i = 0; i < ani_mode_end; i++) {
		m_FrameInfo[i].ClearFrame();
	}
}

enum apm_type {
	apm_attack1 = 1,
	apm_attack2,
	apm_walk,
	apm_run,
	apm_die,
};

void RVisualMesh::CheckAnimationType(RAnimation* pAniSet)
{
	if (!pAniSet) return;
}

bool RVisualMesh::SetBlendAnimation(RAnimation* pAniSet, float blend_time, bool b)
{
	return SetBlendAnimation(ani_mode_lower, pAniSet, blend_time, b);
}

bool RVisualMesh::SetBlendAnimation(char* ani_name, float blend_time, bool b)
{
	return SetBlendAnimation(ani_mode_lower, ani_name, blend_time, b);
}

bool RVisualMesh::SetBlendAnimation(RAniMode animode, RAnimation* pAniSet, float blend_time, bool b)
{
	RAnimation* pAS[2];

	AniFrameInfo* pInfo = GetFrameInfo(animode);

	if (pInfo->m_bBlendAniSet == false) {
		pInfo->m_bBlendAniSet = true;
		pInfo->m_fMaxBlendTime = blend_time;
		pInfo->m_fCurrentBlendTime = 0.f;
	}
	else {
		pInfo->m_bBlendAniSet = true;
		pInfo->m_fMaxBlendTime = blend_time;
		pInfo->m_fCurrentBlendTime = 0.f;
	}

	pAS[0] = GetFrameInfo(ani_mode_lower)->m_pAniSet;
	pAS[1] = GetFrameInfo(ani_mode_upper)->m_pAniSet;

	if (SetAnimation(animode, pAniSet, b)) {
		GetFrameInfo(ani_mode_blend_lower)->m_pAniSet = pAS[0];
		GetFrameInfo(ani_mode_blend_upper)->m_pAniSet = pAS[1];

		return true;
	}

	return false;
}

bool RVisualMesh::SetBlendAnimation(RAniMode animode, char* ani_name, float blend_time, bool b)
{
	RAnimation* pAS[2];

	AniFrameInfo* pInfo = GetFrameInfo(animode);

	if (pInfo->m_bBlendAniSet == false) {
		pInfo->m_bBlendAniSet = true;
		pInfo->m_fMaxBlendTime = blend_time;
		pInfo->m_fCurrentBlendTime = 0.f;
	}
	else {
		pInfo->m_bBlendAniSet = true;
		pInfo->m_fMaxBlendTime = blend_time;
		pInfo->m_fCurrentBlendTime = 0.f;
	}

	pAS[0] = GetFrameInfo(ani_mode_lower)->m_pAniSet;
	pAS[1] = GetFrameInfo(ani_mode_upper)->m_pAniSet;

	if (SetAnimation(animode, ani_name, b)) {
		GetFrameInfo(ani_mode_blend_lower)->m_pAniSet = pAS[0];
		GetFrameInfo(ani_mode_blend_upper)->m_pAniSet = pAS[1];

		return true;
	}

	return false;
}

bool RVisualMesh::SetAnimation(RAnimation* pAniSet, bool b)
{
	return SetAnimation(ani_mode_lower, pAniSet, b);
}

bool RVisualMesh::SetAnimation(char* ani_name, bool b)
{
	return SetAnimation(ani_mode_lower, ani_name, b);
}

bool RVisualMesh::SetAnimation(RAniMode animode, RAnimation* pAniSet, bool b)
{
	if (!pAniSet)
		return false;

	_BP("VMesh::SetAnimation");

	if (pAniSet->GetAniNodeCount() == 0) {
		return false;
	}

	AniFrameInfo* pInfo = GetFrameInfo(animode);

	if (pInfo == NULL) return false;

	bool bChange = false;
	bool bSaveFrame = false;

	if (b) {
		bChange = true;
	}
	else {
		if (pInfo->m_pAniSet) {
			if (pInfo->m_pAniSet != pAniSet) {
				bChange = true;
			}
		}
		else {
			bChange = true;
		}
	}

	if (bChange) {
		if (pInfo->m_pAniSet && pAniSet) {
			if (strcmp(pInfo->m_pAniSet->GetName(), pAniSet->GetName()) == 0) {
				if (pInfo->m_pAniSet->m_weapon_motion_type != pAniSet->m_weapon_motion_type) {
					bSaveFrame = true;
				}
			}
		}

		if (bSaveFrame) {
			float ff = 0.f;

			if (pInfo->m_pAniSet->GetMaxFrame())
				ff = pInfo->m_nFrame / (float)pInfo->m_pAniSet->GetMaxFrame();

			pInfo->m_nAddFrame = pAniSet->GetMaxFrame() * ff;
		}
		else {
			pInfo->m_nAddFrame = 0;
		}

		pInfo->m_bChangeAnimation = true;
		pInfo->m_pAniSet = pAniSet;
		Play(animode);
	}

	_EP("VMesh::SetAnimation");

	return true;
}

void RVisualMesh::UpdateMotionTable()
{
	if (m_pMesh == NULL) return;

	int meshnode_cnt = m_pMesh->m_data_num;

	if (m_nAniNodeTableCnt != meshnode_cnt) {
		if (m_pAniNodeTable) {
			delete[] m_pAniNodeTable;
			m_pAniNodeTable = NULL;
		}
	}

	if (m_pAniNodeTable == NULL) {
		m_pAniNodeTable = new RAnimationNode * [meshnode_cnt];
		m_nAniNodeTableCnt = meshnode_cnt;
		memset(m_pAniNodeTable, 0, sizeof(RAnimationNode*) * meshnode_cnt);
	}
	else {
		memset(m_pAniNodeTable, 0, sizeof(RAnimationNode*) * m_nAniNodeTableCnt);
	}

	RAnimation* pAL = GetFrameInfo(ani_mode_lower)->m_pAniSet;
	RAnimation* pAU = GetFrameInfo(ani_mode_upper)->m_pAniSet;

	if (!pAL && !pAU)
		return;

	RAnimationNode* pANode = NULL;
	RMeshNode* pM = NULL;

	int pid = -1;

	if (pAL) {
		int node_cnt = pAL->GetAniNodeCount();

		for (int i = 0; i < node_cnt; i++) {
			pANode = pAL->GetAniNode(i);

			pid = m_pMesh->_FindMeshId(pANode->GetName());

			if (pid != -1) {
				RMeshNode* pM = m_pMesh->m_data[pid];

				if (pM) {
					m_pAniNodeTable[pid] = pANode;
				}
			}
		}
	}

	if (pAU) {
		int node_cnt = pAU->GetAniNodeCount();

		for (int i = 0; i < node_cnt; i++) {
			pANode = pAU->GetAniNode(i);

			pid = m_pMesh->_FindMeshId(pANode->GetName());

			if (pid != -1) {
				RMeshNode* pM = m_pMesh->m_data[pid];

				if (pM) {
					if (pM->m_CutPartsType == cut_parts_upper_body)
						m_pAniNodeTable[pid] = pANode;
				}
			}
		}
	}
}

bool RVisualMesh::SetAnimation(RAniMode animode, char* ani_name, bool b)
{
	if (!m_pMesh)
		return false;

	AniFrameInfo* pAniLow = GetFrameInfo(ani_mode_lower);
	AniFrameInfo* pAniUp = GetFrameInfo(ani_mode_upper);

	if (!ani_name) {
		if (animode == ani_mode_upper) {
			pAniUp->m_pAniSet = NULL;
		}
		return false;
	}

	if (ani_name[0] == 0) {
		if (animode == ani_mode_upper) {
			pAniUp->m_pAniSet = NULL;
		}
		return false;
	}

	int wtype = -1;

	if (m_SelectWeaponMotionType != eq_weapon_etc)
		wtype = GetSetectedWeaponMotionID();

	if (animode == ani_mode_upper) {
		if (pAniLow->m_pAniSet) {
			if (strcmp(ani_name, "attackS") == 0) {
				if (strcmp(pAniLow->m_pAniSet->GetName(), "idle") != 0) {
					if ((wtype != eq_wd_katana) &&
						(wtype != eq_wd_grenade) &&
						(wtype != eq_ws_dagger) &&
						(wtype != eq_wd_shotgun)
						)
					{
						pAniUp->m_pAniSet = NULL;
						return true;
					}
				}
			}
		}
	}

	RAnimation* pAniSet = m_pMesh->m_ani_mgr.GetAnimation(ani_name, wtype);

	return SetAnimation(animode, pAniSet, b);
}

bool RVisualMesh::SetNextAnimation(RAnimation* pAniSet)
{
	return SetNextAnimation(ani_mode_lower, pAniSet);
}

bool RVisualMesh::SetNextAnimation(char* ani_name)
{
	return SetNextAnimation(ani_mode_lower, ani_name);
}

bool RVisualMesh::SetNextAnimation(RAniMode animode, RAnimation* pAniSet)
{
	if (!pAniSet) return false;

	AniFrameInfo* pInfo = GetFrameInfo(animode);

	if (pInfo->m_pAniSet) {
		if (strstr(pInfo->m_pAniSet->GetName(), pAniSet->GetName())) {
			return false;
		}
	}

	pInfo->m_pAniSetNext = pAniSet;

	Play(animode);

	return true;
}

bool RVisualMesh::SetNextAnimation(RAniMode animode, char* ani_name)
{
	if (!m_pMesh)
		return false;

	int wtype = -1;

	if (m_SelectWeaponMotionType != eq_weapon_etc)
		wtype = GetSetectedWeaponMotionID();

	RAnimation* pAniSet = m_pMesh->m_ani_mgr.GetAnimation(ani_name, wtype);

	return SetNextAnimation(animode, pAniSet);
}

bool RVisualMesh::SetReserveAnimation(RAnimation* pAniSet, int tick)
{
	return SetReserveAnimation(ani_mode_lower, pAniSet, tick);
}

bool RVisualMesh::SetReserveAnimation(char* ani_name, int tick)
{
	return SetReserveAnimation(ani_mode_lower, ani_name, tick);
}

bool RVisualMesh::SetReserveAnimation(RAniMode animode, RAnimation* pAniSet, int tick)
{
	if (!pAniSet) return false;

	AniFrameInfo* pInfo = GetFrameInfo(animode);

	if (pInfo == NULL) return false;

	pInfo->m_nReserveTime = timeGetTime() + tick;
	pInfo->m_pAniSetReserve = pAniSet;

	Play(animode);

	return true;
}

bool RVisualMesh::SetReserveAnimation(RAniMode animode, char* ani_name, int tick)
{
	if (!m_pMesh)
		return false;

	int wtype = -1;

	if (m_SelectWeaponMotionType != eq_weapon_etc)
		wtype = GetSetectedWeaponMotionID();

	RAnimation* pAniSet = m_pMesh->m_ani_mgr.GetAnimation(ani_name, wtype);

	return SetReserveAnimation(animode, pAniSet, tick);
}

#define AddText(s)		 { str.Add(#s,false); str.Add(" :",false); str.Add(s);}
#define AddTextEnum(s,e) { str.Add(#s,false); str.Add(" :",false); str.Add(#e);}

void RVisualMesh::OutputDebugString_CharacterState()
{
	return;

	RDebugStr str;

	str.Add("//////////////////   visual mesh   ////////////////////");

	AddText(m_id);
	AddText(m_nAnimationState);

	str.PrintLog();
}

#undef AddText
#undef AddTextEnum

void RVisualMesh::ClearAnimation()
{
	m_pMesh = NULL;

	Stop();
}

void RVisualMesh::SetBaseParts(RMeshPartsType parts)
{
	if (parts < 0 && parts >= eq_parts_end)
		return;

	m_pTMesh[parts] = NULL;
}

rvector	RVisualMesh::GetBipRootPos(int frame)
{
	rvector v = rvector(0.f, 0.f, 0.f);

	RAnimation* pAni = GetFrameInfo(ani_mode_lower)->m_pAniSet;

	if (pAni) {
		if (pAni->GetBipRootNode()) {
			v = pAni->GetBipRootNode()->GetPosValue(frame);

			if (m_isScale) v = v * m_ScaleMat;
		}
	}

	return v;
}

rvector	RVisualMesh::GetFootPosition()
{
	rvector v = rvector(0.f, 0.f, 0.f);

	AniFrameInfo* pAniLow = GetFrameInfo(ani_mode_lower);
	AniFrameInfo* pAniUp = GetFrameInfo(ani_mode_upper);

	if (pAniLow == NULL || pAniUp == NULL)
		return v;

	if (m_pMesh) {
		m_pMesh->SetAnimation(pAniLow->m_pAniSet, pAniUp->m_pAniSet);
		m_pMesh->SetFrame(pAniLow->m_nFrame, pAniUp->m_nFrame);
		m_pMesh->SetMeshVis(m_fVis);
		m_pMesh->SetVisualMesh(this);

		m_pMesh->RenderFrame();

		RMeshNode* pNode = NULL;

		pNode = m_pMesh->FindNode(eq_parts_pos_info_LFoot);

		if (pNode) {
			v.x = pNode->m_mat_result._41;
			v.y = pNode->m_mat_result._42;
			v.z = pNode->m_mat_result._43;

			if (m_isScale) v = v * m_ScaleMat;
		}

		pNode = m_pMesh->FindNode(eq_parts_pos_info_RFoot);

		if (pNode) {
			v.x += pNode->m_mat_result._41;
			v.y += pNode->m_mat_result._42;
			v.z += pNode->m_mat_result._43;

			v *= .5f;
			v.y -= 12.f;

			if (m_isScale) v = v * m_ScaleMat;
		}
	}

	return v;
}

void RVisualMesh::GetBipTypeMatrix(rmatrix* mat, RMeshPartsPosInfoType type)
{
	if (m_pBipMatrix)
	{
		if (m_isScale) {
			*mat = m_pBipMatrix[type] * m_ScaleMat * m_WorldMat;
		}
		else {
			*mat = m_pBipMatrix[type] * m_WorldMat;
		}
	}
}

void RVisualMesh::GetRFootMatrix(rmatrix* mat) { GetBipTypeMatrix(mat, eq_parts_pos_info_RFoot); }
void RVisualMesh::GetLFootMatrix(rmatrix* mat) { GetBipTypeMatrix(mat, eq_parts_pos_info_LFoot); }
void RVisualMesh::GetHeadMatrix(rmatrix* mat) { GetBipTypeMatrix(mat, eq_parts_pos_info_Head); }
void RVisualMesh::GetRootMatrix(rmatrix* mat) { GetBipTypeMatrix(mat, eq_parts_pos_info_Root); }

rvector RVisualMesh::GetBipTypePosition(RMeshPartsPosInfoType type)
{
	rvector rv;
	rmatrix m;
	GetBipTypeMatrix(&m, type);

	rv.x = m._41;
	rv.y = m._42;
	rv.z = m._43;

	return rv;
}

rvector	RVisualMesh::GetRFootPosition()
{
	rvector rv;
	rmatrix m;
	GetRFootMatrix(&m);

	rv.x = m._41;
	rv.y = m._42;
	rv.z = m._43;

	return rv;
}

rvector RVisualMesh::GetLFootPosition()
{
	rvector rv;
	rmatrix m;
	GetLFootMatrix(&m);

	rv.x = m._41;
	rv.y = m._42;
	rv.z = m._43;

	return rv;
}

rvector	RVisualMesh::GetHeadPosition()
{
	rvector rv;
	rmatrix m;
	GetHeadMatrix(&m);

	rv.x = m._41;
	rv.y = m._42;
	rv.z = m._43;

	rvector root = GetRootPosition() - rv;
	Normalize(root);

	return rv + 10.f * root;
}

rvector	RVisualMesh::GetRootPosition()
{
	rvector rv;
	rmatrix m;
	GetRootMatrix(&m);

	rv.x = m._41;
	rv.y = m._42;
	rv.z = m._43;

	return rv;
}

D3DXQUATERNION RVisualMesh::GetBipRootRot(int frame)
{
	D3DXQUATERNION q = D3DXQUATERNION(0.f, 0.f, 0.f, 0.f);

	RAnimation* pAni = GetFrameInfo(ani_mode_lower)->m_pAniSet;

	if (pAni) {
		if (pAni->GetBipRootNode()) {
			q = pAni->GetBipRootNode()->GetRotValue(frame);
		}
	}

	return q;
}

rmatrix	RVisualMesh::GetBipRootMat(int frame)
{
	rmatrix m;

	D3DXQUATERNION q = GetBipRootRot(frame);
	D3DXVECTOR3 v = GetBipRootRot(frame);

	D3DXMatrixRotationQuaternion(&m, &q);

	m._41 = v.x;
	m._42 = v.y;
	m._43 = v.z;

	return m;
}

void RVisualMesh::SetSpeed(float s, float s_up)
{
	GetFrameInfo(ani_mode_lower)->m_fSpeed = s;
	GetFrameInfo(ani_mode_upper)->m_fSpeed = s_up;
}

bool RVisualMesh::isOncePlayDone(RAniMode amode)
{
	AniFrameInfo* pInfo = GetFrameInfo(amode);
	if (pInfo)
		return pInfo->m_isOncePlayDone;
	return false;
}

void RVisualMesh::SetScale(rvector& v)
{
	m_vScale = v;
	D3DXMatrixScaling(&m_ScaleMat, v.x, v.y, v.z);
	m_isScale = true;
}

void RVisualMesh::ClearScale()
{
	m_vScale = rvector(1.f, 1.f, 1.f);
	D3DXMatrixScaling(&m_ScaleMat, 1.f, 1.f, 1.f);
	m_isScale = false;
}

void RVisualMesh::GetBBox(rvector& vMax, rvector& vMin)
{
	vMax = m_vBMax;
	vMin = m_vBMin;
}
int RVisualMesh::GetSetectedWeaponMotionID()
{
	return (int)m_SelectWeaponMotionType;
}

void RVisualMesh::SelectWeaponMotion(RWeaponMotionType type)
{
	m_SelectWeaponMotionType = type;
	*m_pSelectWeaponMotionType_AntiHack = type;
}

RVisualMesh* RVisualMesh::GetSelectWeaponVMesh()
{
	return m_WeaponVisualMesh[m_SelectWeaponMotionType];
}

void RVisualMesh::SetRotXYZ(rvector v) {
	m_vRotXYZ = v;
}

void RVisualMesh::SetLowPolyModel(RMesh* pMesh) {
	m_pLowPolyMesh = pMesh;
}

RMesh* RVisualMesh::GetLowPolyModle() {
	return m_pLowPolyMesh;
}

void RVisualMesh::SetDrawTracks(bool s) {
	m_bDrawTracks = s;
}

void RVisualMesh::SetCheckViewFrustum(bool b) {
	m_bCheckViewFrustum = b;
}

bool RVisualMesh::IsSelectWeaponGrenade()
{
	if (m_SelectWeaponMotionType == eq_wd_grenade)
		return true;
	return false;
}

bool RVisualMesh::IsClothModel()
{
	if (m_pTMesh[eq_parts_chest]) {
		if (m_pTMesh[eq_parts_chest]->m_point_color_num)
			return true;
	}

	return false;
}

bool RVisualMesh::GetWeaponDummyMatrix(WeaponDummyType type, rmatrix* mat, bool bLeft)
{
	if (type > weapon_dummy_etc&& type < weapon_dummy_end)
	{
		rmatrix m;

		if (bLeft) {
			m = m_WeaponDummyMatrix2[type];
		}
		else {
			m = m_WeaponDummyMatrix[type];
		}

		if (m_bIsNpc)
		{
			if (m_isScale)
				m = m_ScaleMat * m;
			m = m * m_WorldMat;
		}

		*mat = m;

		return true;
	}
	return false;
}

bool RVisualMesh::GetWeaponDummyPos(WeaponDummyType type, rvector* pos, bool bLeft)
{
	rmatrix m;

	if (GetWeaponDummyMatrix(type, &m, bLeft)) {
		pos->x = m._41;
		pos->y = m._42;
		pos->z = m._43;
		return true;
	}
	return false;
}

void RVisualMesh::SetUVAnimation(float u, float v)
{
	m_fUAniValue = u;
	m_fVAniValue = v;
	m_bUVAni = true;
}

void RVisualMesh::ClearUvAnimation()
{
	m_fUAniValue = 0.f;
	m_fVAniValue = 0.f;
	m_bUVAni = false;
}

void RVisualMesh::UpdateWeaponDummyMatrix(RMeshNode* pMeshNode)
{
	if (!pMeshNode) return;

	if (pMeshNode->m_WeaponDummyType != weapon_dummy_etc) {
		m_WeaponDummyMatrix[pMeshNode->m_WeaponDummyType] = pMeshNode->m_mat_result;
	}
}

void RVisualMesh::UpdateWeaponPartInfo(RMeshNode* pMeshNode)
{
	if (!pMeshNode) return;

	if (pMeshNode->m_PartsType != eq_parts_etc)
	{
		if (pMeshNode->m_pAnimationNode)
		{
			m_WeaponPartInfo[pMeshNode->m_PartsType].mat = pMeshNode->m_mat_result;
			m_WeaponPartInfo[pMeshNode->m_PartsType].vis = pMeshNode->GetNodeVisValue();
			m_WeaponPartInfo[pMeshNode->m_PartsType].isUpdate = true;
		}
	}
}

AniFrameInfo* RVisualMesh::GetFrameInfo(RAniMode mode)
{
	return &m_FrameInfo[mode];
}

void RVisualMesh::OnRestore()
{
	if (m_pCloth)
		m_pCloth->OnRestore();
}

void RVisualMesh::OnInvalidate()
{
	if (m_pCloth)
		m_pCloth->OnInvalidate();
}

bool RVisualMesh::CreateCloth(RMeshNode* pMeshNode, float fAccel, int Numiter)
{
	DestroyCloth();

	m_pCloth = new RCharCloth;
	if (!m_pCloth->create(m_pMesh, pMeshNode))
	{
		DestroyCloth();
		return false;
	}

	m_pCloth->SetAccelationRatio(fAccel);
	m_pCloth->SetNumIteration(Numiter);

	pMeshNode->m_bClothMeshNodeSkip = false;

	return true;
}

void RVisualMesh::DestroyCloth()
{
	// CUSTOM: ZERONIS: SAFE_DELETE TO SAFE_DELETE_ARARY
	SAFE_DELETE_ARRAY(m_pCloth);
}

void RVisualMesh::SetClothState(int state)
{
	if (m_pCloth) {
		m_pCloth->SetStatus(state);
	}
}

void RVisualMesh::UpdateForce(D3DXVECTOR3& force)
{
	if (m_pCloth) {
		m_pCloth->setForce(force);
	}
}

void RVisualMesh::UpdateCloth()
{
	if (m_pCloth)
		m_pCloth->update(m_bClothGame, &m_WorldMat, m_fClothDist);
}

void RVisualMesh::RenderCloth()
{
	if (m_pCloth)
		m_pCloth->render();
}
bool RVisualMesh::ChangeChestCloth(float fAccel, int Numiter)
{
	RMeshNode* pMeshNode = GetParts(eq_parts_chest);

	if (pMeshNode == NULL)
	{
		int nMeshNode = m_pMesh->m_data_num;

		RMeshNode* pTMN;

		for (int i = 0; i < nMeshNode; ++i)
		{
			pTMN = m_pMesh->m_data[i];

			if (pTMN->m_PartsType == eq_parts_chest) {
				pMeshNode = pTMN;
				break;
			}
		}
	}

	if (m_pCloth && (m_pCloth->mpMeshNode == pMeshNode))
		return true;

	if (pMeshNode && pMeshNode->m_point_color_num > 0)
	{
		if (!CreateCloth(pMeshNode, fAccel, Numiter))
		{
			DestroyCloth();
			return false;
		}
	}
	else {
		DestroyCloth();
	}

	return true;
}

void RVisualMesh::SetClothForce(D3DXVECTOR3& f)
{
	if (m_pCloth)
		m_pCloth->setForce(f);
}

void RVisualMesh::SetClothValue(bool bGame, float fDist)
{
	m_fClothDist = fDist;
	m_bClothGame = bGame;
}

#undef __BP
#undef __EP

_NAMESPACE_REALSPACE2_END