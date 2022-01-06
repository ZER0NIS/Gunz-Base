#pragma once

#include "RVisualMeshUtil.h"
#include "RAniEventInfo.h"
#include "MeshManager.h"
#include <array>

class RCharCloth;

_NAMESPACE_REALSPACE2_BEGIN

class RVisualMesh;
class ROcclusionList;

typedef void(ANIEVE_HANDLER)(RAniEventInfo* pAniEventInfo);

class AniFrameInfo {
public:
	AniFrameInfo();

	void ClearFrame();

	void Frame(RAniMode amode, RVisualMesh* pVMesh);

	ANIEVE_HANDLER* m_pEventFunc;

	RAniIDEventSet* m_pAniIdEventSet;

	RAniNameEventSet* m_pAniNameEventSet;
	void SetAnimEventHandler(ANIEVE_HANDLER pEventFunc) { m_pEventFunc = pEventFunc; }

	void SetAniIdEventSet(RAniIDEventSet* pEventSet) { m_pAniIdEventSet = pEventSet; }

	vector<bool>			m_vecCheckAniEvent;
	vector<bool>::iterator	m_iterCheckAniEvent;

	bool			m_isOncePlayDone;
	bool			m_isPlayDone;

	bool			m_bChangeAnimation;

	DWORD			m_nReserveTime;

	int				m_nFrame;
	int				m_nAddFrame;
	DWORD			m_save_time;
	DWORD			m_1frame_time;

	RAnimation* m_pAniSet;
	RAnimation* m_pAniSetNext;
	RAnimation* m_pAniSetReserve;

	float			m_fSpeed;

	RAniSoundInfo	m_SoundInfo;

	bool			m_bBlendAniSet;
	float			m_fMaxBlendTime;
	float			m_fCurrentBlendTime;
	DWORD			m_dwBackupBlendTime;
};

class RFrameTime
{
public:
	RFrameTime() {
		m_bActive = false;
		m_bReturn = false;
		m_fMaxValue = 0.f;
		m_dwStartTime = 0;
		m_dwEndTime = 0;
		m_nType = 0;
		m_fCurValue = 0.f;
		m_dwReturnMaxTime = 0;
	}

	~RFrameTime() {
	}

public:
	void Start(float fMax, DWORD MaxTime, DWORD ReturnMaxTime);
	void Stop();

	void Update();

	float GetValue() {
		return m_fCurValue;
	}

public:

	int   m_nType;
	bool  m_bActive;
	bool  m_bReturn;
	float m_fMaxValue;
	float m_fCurValue;
	DWORD m_dwStartTime;
	DWORD m_dwEndTime;
	DWORD m_dwReturnMaxTime;
};

#define VISUAL_LIGHT_MAX 3

class RVisualLightMgr
{
public:
	RVisualLightMgr();

	int GetLightCount();

	void SetLight(int index, D3DLIGHT9* light, bool ShaderOnly);

	void UpdateLight();

	void Clone(RVisualMesh* pVMesh);

public:
	D3DLIGHT9 m_Light[VISUAL_LIGHT_MAX];
	int		  m_LightEnable[VISUAL_LIGHT_MAX];
};

class RQuery
{
public:
	RQuery() {
		m_nSpendingTime = NULL;
		m_DataCnt = NULL;
		m_Query = NULL;
	}

	virtual ~RQuery() {
		Destroy();
	}

	HRESULT Create(LPDIRECT3DDEVICE9 dev) {
		return dev->CreateQuery(D3DQUERYTYPE_OCCLUSION, &m_Query);
	}

	void Destroy() {
		if (m_Query) {
			m_Query->Release();
			m_Query = NULL;
		}
	}

	HRESULT Begin() {
		return m_Query->Issue(D3DISSUE_BEGIN);
	}

	HRESULT End() {
		return m_Query->Issue(D3DISSUE_END);
	}

	DWORD GetRenderCount() {
		if (m_Query == NULL) return 0;

		int nCnt = 0;
		m_DataCnt = 0;

		while (m_Query->GetData((void*)&m_DataCnt, sizeof(DWORD), D3DGETDATA_FLUSH) == S_FALSE) {
			m_nSpendingTime++;
			nCnt++;
			if (nCnt > 5000) {
				m_DataCnt = 1000;
				break;
			}
		}
		return m_DataCnt;
	}

	bool isNeedRender() {
		if (m_Query == NULL) return true;

		DWORD Cnt = GetRenderCount();

		if (Cnt > 10)
			return true;
		return false;
	}

public:

	int					m_nSpendingTime;
	DWORD				m_DataCnt;
	LPDIRECT3DQUERY9	m_Query;
};

class RVisualMesh {
public:
	RVisualMesh();
	~RVisualMesh();

	bool Create(RMesh* pMesh);
	void Destroy();

	bool BBoxPickCheck(int x, int y);
	bool BBoxPickCheck(const rvector& pos, const rvector& dir);

	bool Pick(int x, int y, RPickInfo* pInfo);
	bool Pick(rvector* vInVec, RPickInfo* pInfo);
	bool Pick(const rvector& pos, const rvector& dir, RPickInfo* pInfo);
	bool Pick(rvector& pos, rvector& dir, rvector* v, float* t);

	int  GetMaxFrame(RAniMode amode);

	void Frame(RAniMode amode);
	void Frame();

	void Render(ROcclusionList* pOCCL);
	void Render(bool low = false, bool render_buffer = false);
	void RenderWeapon();
	void RenderMatrix();

	void GetMotionInfo(int& sel_parts, int& sel_parts2, bool& bCheck, bool& bRender);

	void SetPos(rvector pos, rvector dir, rvector up);
	void SetWorldMatrix(rmatrix& mat);

	bool CalcBox();

	bool SkipRenderFaceParts() { return m_bSkipRenderFaceParts; }

	void SetParts(RMeshPartsType parts, const char* name);

	void ClearParts();

	RMeshNode* GetParts(RMeshPartsType parts);

	void SetBaseParts(RMeshPartsType parts);

	bool SetAnimation(RAnimation* pAniSet, bool b = false);
	bool SetAnimation(char* ani_name, bool b = false);
	bool SetAnimation(RAniMode animode, RAnimation* pAniSet, bool b = false);
	bool SetAnimation(RAniMode animode, char* ani_name, bool b = false);

	bool SetBlendAnimation(RAnimation* pAniSet, float blend_time = 0.5f, bool b = false);
	bool SetBlendAnimation(char* ani_name, float blend_time = 0.5f, bool b = false);
	bool SetBlendAnimation(RAniMode animode, RAnimation* pAniSet, float blend_time = 0.5f, bool b = false);
	bool SetBlendAnimation(RAniMode animode, char* ani_name, float blend_time = 0.5f, bool b = false);

	bool SetNextAnimation(RAnimation* pAniSet);
	bool SetNextAnimation(char* ani_name);
	bool SetNextAnimation(RAniMode animode, RAnimation* pAniSet);
	bool SetNextAnimation(RAniMode animode, char* ani_name);

	bool SetReserveAnimation(RAnimation* pAniSet, int tick);
	bool SetReserveAnimation(char* ani_name, int tick);
	bool SetReserveAnimation(RAniMode animode, RAnimation* pAniSet, int tick);
	bool SetReserveAnimation(RAniMode animode, char* ani_name, int tick);

	void ClearAnimation();
	void ClearFrame();

	void CheckAnimationType(RAnimation* pAniSet);

	void SetSpeed(float s, float s_up = 4.8f);

	void Play(RAniMode amode = ani_mode_lower);
	void Stop(RAniMode amode = ani_mode_lower);
	void Pause(RAniMode amode = ani_mode_lower);

	bool isOncePlayDone(RAniMode amode = ani_mode_lower);

	void SetScale(rvector& v);
	void ClearScale();

	void AddWeapon(RWeaponMotionType type, RMesh* pVMesh, RAnimation* pAni = NULL);
	void RemoveWeapon(RWeaponMotionType type);
	void RemoveAllWeapon();

	int				GetSetectedWeaponMotionID();
	void			SelectWeaponMotion(RWeaponMotionType type);
	RVisualMesh* GetSelectWeaponVMesh();

	void	SetRotXYZ(rvector v);

	void	SetLowPolyModel(RMesh* pMesh);
	RMesh* GetLowPolyModle();

	void	SetDrawTracks(bool s);

	void	SetCheckViewFrustum(bool b);

	rmatrix GetCurrentWeaponPositionMatrix(bool right = false);
	rvector GetCurrentWeaponPosition(bool right = false);

	bool IsSelectWeaponGrenade();

	rvector			GetBipRootPos(int frame);
	D3DXQUATERNION	GetBipRootRot(int frame);
	rmatrix			GetBipRootMat(int frame);

	rvector			GetFootPosition();

	void GetBipTypeMatrix(rmatrix* mat, RMeshPartsPosInfoType type);

	void GetHeadMatrix(rmatrix* mat);
	void GetRFootMatrix(rmatrix* mat);
	void GetLFootMatrix(rmatrix* mat);
	void GetRootMatrix(rmatrix* mat);

	rvector GetBipTypePosition(RMeshPartsPosInfoType type);

	rvector		GetHeadPosition();
	rvector		GetRFootPosition();
	rvector		GetLFootPosition();
	rvector		GetRootPosition();

	void	SetVisibility(float vis) { m_fVis = vis; }
	float	GetVisibility() { return m_fVis; }

	void SetDrawGrenade(bool b) { m_bDrawGrenade = b; }

	void OutputDebugString_CharacterState();

	bool IsClothModel();

	void DrawTracks(bool draw, RVisualMesh* pVWMesh, int mode, rmatrix& m);

	void DrawEnchant(RVisualMesh* pVWMesh, int mode, rmatrix& m);

	void DrawEnchantFire(RVisualMesh* pVWMesh, int mode, rmatrix& m);
	void DrawEnchantCold(RVisualMesh* pVWMesh, int mode, rmatrix& m);
	void DrawEnchantLighting(RVisualMesh* pVWMesh, int mode, rmatrix& m);
	void DrawEnchantPoison(RVisualMesh* pVWMesh, int mode, rmatrix& m);

	int	 GetLastWeaponTrackPos(rvector* pOutVec);

	AniFrameInfo* GetFrameInfo(RAniMode mode);

	void GetBBox(rvector& max, rvector& min);

	bool GetWeaponDummyMatrix(WeaponDummyType type, rmatrix* mat, bool bLeft);
	bool GetWeaponDummyPos(WeaponDummyType type, rvector* pos, bool bLeft);

	void SetNPCBlendColor(D3DCOLORVALUE color) { m_NPCBlendColor = color; }

	void SetSpRenderMode(ALPHAPASS ePass);
	void ClearPartInfo();

	void  GetWeaponPos(rvector* p, bool bLeft = false);
	float GetWeaponSize();
	bool  IsDoubleWeapon();

	void GetEnChantColor(DWORD* color);
	void SetEnChantType(REnchantType EnchantType) {
		m_EnchantType = EnchantType;
	}

	void UpdateMotionTable();
	bool UpdateSpWeaponFire();

	void SetUVAnimation(float u, float v);
	void ClearUvAnimation();

	void UpdateWeaponDummyMatrix(RMeshNode* pMNode);
	void UpdateWeaponPartInfo(RMeshNode* pMNode);

	void OnRestore();
	void OnInvalidate();

public:

	bool CreateCloth(RMeshNode* pMeshNode, float fAccel, int Numiter);
	void DestroyCloth();

	bool ChangeChestCloth(float fAccel, int Numiter);

	void UpdateForce(D3DXVECTOR3& force);
	void SetClothState(int state);
	void UpdateCloth();
	void RenderCloth();

	bool isChestClothMesh() { return m_pCloth ? true : false; }

	void SetClothForce(D3DXVECTOR3& f);

	void SetClothValue(bool bGame, float fDist);

private:

	float			m_fClothDist;
	bool			m_bClothGame;
	RCharCloth* m_pCloth;

public:

	void SetLight(int index, D3DLIGHT9* light, bool ShaderOnly) { m_LightMgr.SetLight(index, light, ShaderOnly); }
	void UpdateLight() { m_LightMgr.UpdateLight(); }

public:

	bool			m_bIsNpc;
	bool			m_bIsCharacter;

	bool			m_bDrawTracksMotion[2];
	bool			m_bDrawTracks;
	bool			m_isDrawWeaponState;
	bool			m_bDrawGrenade;

	bool			m_isScale;
	rvector			m_vScale;

	rvector			m_vTargetPos;
	rvector			m_vRotXYZ;
	RFrameTime		m_FrameTime;

	rmatrix			m_RotMat;

	rmatrix				m_ToonUVMat;
	LPDIRECT3DTEXTURE9	m_ToonTexture;
	bool				m_bToonLighting;
	bool				m_bToonTextureRender;
	DWORD				m_bToonColor;

	rvector			m_vPos;
	rvector			m_vDir;
	rvector			m_vUp;
	rmatrix			m_WorldMat;
	rmatrix			m_ScaleMat;
	std::array<RMeshNodePtr, eq_parts_end> m_pTMesh;
	RMesh* m_pMesh;
	RMesh* m_pLowPolyMesh;

	RVisualLightMgr m_LightMgr;

	RWeaponTracks* m_pTracks[2];

	REnchantType	m_EnchantType;

	int				m_id;
	float			m_fVis;

	int				m_nAnimationState;

	D3DCOLORVALUE	m_NPCBlendColor;

	AniFrameInfo* m_FrameInfo;

	D3DXVECTOR3		m_vBMax;
	D3DXVECTOR3		m_vBMin;

	RVisualMesh* m_WeaponVisualMesh[eq_weapon_end];
	RPartsInfo		m_WeaponPartInfo[eq_parts_end];
	D3DXMATRIX		m_WeaponMatrixTemp;
	D3DXMATRIX		m_WeaponDummyMatrix[weapon_dummy_end];
	D3DXMATRIX		m_WeaponDummyMatrix2[weapon_dummy_end];

	RWeaponMotionType	m_SelectWeaponMotionType;
	RWeaponMotionType* m_pSelectWeaponMotionType_AntiHack;

	rmatrix* m_pBipMatrix;

	rmatrix			m_UpperRotMat;

	RAnimationNode** m_pAniNodeTable;
	int					m_nAniNodeTableCnt;

	ROcclusionList* m_pTOCCL;

	float			m_fUAniValue;
	float			m_fVAniValue;
	bool			m_bUVAni;

	bool			m_bRenderMatrix;

	bool			m_bRenderInstantly;
	bool			m_bIsRender;
	bool			m_bIsRenderWeapon;
	bool			m_bIsRenderFirst;
	bool			m_bCheckViewFrustum;
	bool			m_bGrenadeRenderOnoff;
	bool			m_bGrenadeFire;
	bool			m_bAddGrenade;
	DWORD			m_GrenadeFireTime;

	bool			m_bCalcBoxWithScale;
	bool			m_bSkipRenderFaceParts;
};

class RCharacterVisualMesh : public RVisualMesh {
public:
	RCharacterVisualMesh() {
	}
	~RCharacterVisualMesh() {
	}

public:
};

_NAMESPACE_REALSPACE2_END