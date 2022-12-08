#pragma once

#pragma warning (disable : 4244)
#pragma warning (disable : 4305)

#include <list>
#include <string>
#include <unordered_map>

#include <D3DX9.h>

#include "RTypes.h"

#define		MAX_NAME_LEN		40
#define		MAX_PATH_NAME_LEN	256
#define		MAX_ANI_KEY			100
#define		MAX_MESH_NODE_TABLE	300
#define		MAX_PHYSIQUE_KEY	4

#ifndef USING_VERTEX_SHADER
#define USING_VERTEX_SHADER
#endif
using namespace std;

#define EXPORTER_MESH_VER1	0x00000011
#define EXPORTER_MESH_VER2	0x00005001
#define EXPORTER_MESH_VER3	0x00005002
#define EXPORTER_MESH_VER4	0x00005003
#define EXPORTER_MESH_VER5	0x00005004
#define EXPORTER_MESH_VER6	0x00005005
#define EXPORTER_MESH_VER7	0x00005006
#define EXPORTER_MESH_VER8	0x00005007

#define EXPORTER_ANI_VER1	0x00000012
#define EXPORTER_ANI_VER2	0x00001001
#define EXPORTER_ANI_VER3	0x00001002
#define EXPORTER_ANI_VER4	0x00001003

#define EXPORTER_SIG		0x0107f060

typedef struct {
	DWORD	sig;
	DWORD	ver;
	int		mtrl_num;
	int		mesh_num;
} ex_hd_t;

typedef struct {
	DWORD	sig;
	DWORD	ver;
	int		maxframe;
	int		model_num;
	int		ani_type;
} ex_ani_t;

enum RWeaponMotionType {
	eq_weapon_etc = 0,

	eq_wd_katana,
	eq_ws_pistol,
	eq_wd_pistol,
	eq_wd_shotgun,
	eq_wd_rifle,
	eq_wd_grenade,
	eq_ws_dagger,
	eq_wd_item,
	eq_wd_rlauncher,
	eq_ws_smg,
	eq_wd_smg,
	eq_wd_sword,
	eq_wd_blade,
	eq_wd_dagger,

	eq_weapon_end,
};

typedef enum _RMeshPartsPosInfoType {
	eq_parts_pos_info_etc = 0,

	eq_parts_pos_info_Root,
	eq_parts_pos_info_Head,
	eq_parts_pos_info_HeadNub,
	eq_parts_pos_info_Neck,
	eq_parts_pos_info_Pelvis,
	eq_parts_pos_info_Spine,
	eq_parts_pos_info_Spine1,
	eq_parts_pos_info_Spine2,

	eq_parts_pos_info_LCalf,
	eq_parts_pos_info_LClavicle,
	eq_parts_pos_info_LFinger0,
	eq_parts_pos_info_LFingerNub,
	eq_parts_pos_info_LFoot,
	eq_parts_pos_info_LForeArm,
	eq_parts_pos_info_LHand,
	eq_parts_pos_info_LThigh,
	eq_parts_pos_info_LToe0,
	eq_parts_pos_info_LToe0Nub,
	eq_parts_pos_info_LUpperArm,

	eq_parts_pos_info_RCalf,
	eq_parts_pos_info_RClavicle,
	eq_parts_pos_info_RFinger0,
	eq_parts_pos_info_RFingerNub,
	eq_parts_pos_info_RFoot,
	eq_parts_pos_info_RForeArm,
	eq_parts_pos_info_RHand,
	eq_parts_pos_info_RThigh,
	eq_parts_pos_info_RToe0,
	eq_parts_pos_info_RToe0Nub,
	eq_parts_pos_info_RUpperArm,

	eq_parts_pos_info_Effect,

	eq_parts_pos_info_end
} RMeshPartsPosInfoType;

typedef enum _RMeshPartsType {
	eq_parts_etc = 0,
	eq_parts_head,
	eq_parts_face,
	eq_parts_chest,
	eq_parts_hands,
	eq_parts_legs,
	eq_parts_feet,
	eq_parts_sunglass,

	eq_parts_left_pistol,
	eq_parts_left_smg,
	eq_parts_left_blade,
	eq_parts_left_dagger,

	eq_parts_right_katana,
	eq_parts_right_pistol,
	eq_parts_right_smg,
	eq_parts_right_shotgun,
	eq_parts_right_rifle,
	eq_parts_right_grenade,
	eq_parts_right_item,
	eq_parts_right_dagger,
	eq_parts_right_rlauncher,
	eq_parts_right_sword,
	eq_parts_right_blade,

	eq_parts_end,
} RMeshPartsType;

enum CutParts {
	cut_parts_upper_body = 0,
	cut_parts_lower_body,
	cut_parts_etc_body,
	cut_parts_end,
};

enum LookAtParts {
	lookat_parts_etc = 0,
	lookat_parts_spine,
	lookat_parts_spine1,
	lookat_parts_spine2,
	lookat_parts_head,
	lookat_parts_end,
};

enum WeaponDummyType {
	weapon_dummy_etc = 0,
	weapon_dummy_muzzle_flash,
	weapon_dummy_cartridge01,
	weapon_dummy_cartridge02,
	weapon_dummy_end,
};

enum RPickType {
	pick_bbox = 0,
	pick_collision_mesh,
	pick_real_mesh,
	pick_end
};

enum RShaderConst {
	IDENTITY_MATRIX = 0,
	WORLD_MATRIX = 3,
	VIEW_PROJECTION_MATRIX = 6,
	CONSTANTS = 10,
	CAMERA_POSITION = 11,
	MATERIAL_AMBIENT = 12,
	MATERIAL_DIFFUSE,
	MATERIAL_SPECULAR,
	MATERIAL_POWER,
	GLOBAL_AMBIENT,
	LIGHT0_POSITION,
	LIGHT0_AMBIENT,
	LIGHT0_DIFFUSE,
	LIGHT0_SPECULAR,
	LIGHT0_RANGE,
	LIGHT1_POSITION,
	LIGHT1_AMBIENT,
	LIGHT1_DIFFUSE,
	LIGHT1_SPECULAR,
	LIGHT1_RANGE,
	LIGHT_ATTENUATION,
	LIGHT_ATTENUATION1,
	ANIMATION_MATRIX_BASE
};

enum RShaderBlendInput {
	VPOSITION,
	WEIGHT2,
	MATRIX_INDEX,
	NORMAL,
	TEXTURE_UV
};

struct	RTLVertex {
	D3DXVECTOR4 p;
	DWORD color;
	FLOAT tu, tv;
};

struct	RLVertex {
	D3DXVECTOR3 p;
	DWORD color;
	FLOAT tu, tv;
};

#ifndef _MAX_EXPORT

struct	RVertex {
	D3DXVECTOR3 p;
	D3DXVECTOR3 n;
	FLOAT tu, tv;
};

#endif

struct RBlendVertex
{
	D3DXVECTOR3 p;
	float weight1, weight2;
	float matIndex[3];
	D3DXVECTOR3 normal;
	float tu, tv;
};

#define RTLVertexType		(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#define RLVertexType		(D3DFVF_XYZ   |D3DFVF_DIFFUSE|D3DFVF_TEX1)
#define RVertexType			(D3DFVF_XYZ   |D3DFVF_NORMAL |D3DFVF_TEX1)
#define RBLENDVERTEXTYPE	( D3DFVF_XYZB2 | D3DFVF_XYZ | D3DFVF_DIFFUSE  | D3DFVF_TEX1 | D3DFVF_NORMAL )

class RPosKey : public D3DXVECTOR3 {
public:
	int	frame;
};

class RQuatKey : public D3DXQUATERNION {
public:
	int	frame;
};

#define RRotKey RQuatKey

class RVisKey {
public:
	float v;
	int frame;
};

class RTMKey : public D3DXMATRIX {
public:
	int frame;
};

class RVertexAniKey : public D3DXVECTOR3 {
public:
	int frame;
};

struct RFaceInfoOld {
	int				m_point_index[3];
	D3DXVECTOR3		m_point_tex[3];
	int				m_mtrl_id;
};

struct RFaceInfo {
	int				m_point_index[3];
	D3DXVECTOR3		m_point_tex[3];
	int				m_mtrl_id;
	int				m_sg_id;
};

struct RFaceNormalInfo {
	D3DXVECTOR3 m_normal;
	D3DXVECTOR3 m_pointnormal[3];
};

struct RPhysiqueInfo {
	RPhysiqueInfo() {
		for (int i = 0; i < MAX_PHYSIQUE_KEY; i++)
			m_parent_name[i][0] = 0;

		m_num = 0;
	};

	char	m_parent_name[MAX_PHYSIQUE_KEY][MAX_NAME_LEN];
	float	m_weight[MAX_PHYSIQUE_KEY];
	int		m_parent_id[MAX_PHYSIQUE_KEY];
	int		m_num;

	D3DXVECTOR3 m_offset[MAX_PHYSIQUE_KEY];
};

#define USE_VERTEX_SW 1
#define USE_VERTEX_HW 1<<1

class RIndexBuffer final {
public:
	RIndexBuffer();
	virtual ~RIndexBuffer();

	void Lock();
	void Unlock();

	void Update(int size, WORD* pData);
	bool Create(int size, WORD* pData, DWORD flag = USE_VERTEX_HW | USE_VERTEX_SW, DWORD Usage = D3DUSAGE_WRITEONLY, D3DPOOL Pool = D3DPOOL_MANAGED);

	int GetFaceCnt();

	void SetIndices();

public:

	bool	m_bUseSWVertex;
	bool	m_bUseHWVertex;

	DWORD	m_dwUsage;
	D3DPOOL	m_dwPool;
	DWORD	m_dwLockFlag;

	WORD* m_pIndex;
	WORD* m_i;

	int m_size;

	D3DPtr<IDirect3DIndexBuffer9> m_ib;
};

class RVertexBuffer final {
public:
	RVertexBuffer();
	virtual ~RVertexBuffer();

	void Init();
	void Clear();

	bool Create(char* pVertex, DWORD fvf, int VertexSize, int VertexCnt, DWORD flag, DWORD Usage = D3DUSAGE_WRITEONLY, D3DPOOL Pool = D3DPOOL_MANAGED);

	bool Update(char* pVertex, DWORD fvf, int VertexSize, int VertexCnt);
	bool UpdateData(char* pVertex);
	bool UpdateDataSW(char* pVertex);
	bool UpdateDataHW(char* pVertex);

	bool UpdateData(D3DXVECTOR3* pVec);

#ifndef _MAX_EXPORT

	void UpdateDataLVert(RLVertex* pVert, D3DXVECTOR3* pVec, int nCnt);
	void UpdateDataVert(RVertex* pVert, D3DXVECTOR3* pVec, int nCnt);

#endif

	void Lock();
	void Unlock();

	void SetStreamSource();

	void Render();
	void RenderFVF();
	void Render(RIndexBuffer* ib);
	void RenderSoft();
	void RenderIndexSoft(RIndexBuffer* ib);
	void SetVertexBuffer();
	void SetVSVertexBuffer();
	void RenderIndexBuffer(RIndexBuffer* ib);

	void ConvertSilhouetteBuffer(float fLineWidth);
	void ReConvertSilhouetteBuffer(float fLineWidth);

public:

	bool  m_is_init;
	bool  m_bUseSWVertex;
	bool  m_bUseHWVertex;
	char* m_pVert;
	char* m_v;

	DWORD	m_dwFVF;
	DWORD	m_dwUsage;
	D3DPOOL	m_dwPool;
	DWORD	m_dwLockFlag;
	int		m_nVertexSize;
	int		m_nVertexCnt;
	int		m_nBufferSize;
	int		m_nRealBufferSize;

	int		m_nRenderCnt;

	D3DPRIMITIVETYPE m_PrimitiveType;

	D3DPtr<IDirect3DVertexBuffer9> 	m_vb;
};

inline D3DXQUATERNION* WINAPI D3DXQuaternionUnitAxisToUnitAxis2(D3DXQUATERNION* pOut, const D3DXVECTOR3* pvFrom, const D3DXVECTOR3* pvTo)
{
	D3DXVECTOR3 vAxis;
	D3DXVec3Cross(&vAxis, pvFrom, pvTo);
	pOut->x = vAxis.x;
	pOut->y = vAxis.y;
	pOut->z = vAxis.z;
	pOut->w = D3DXVec3Dot(pvFrom, pvTo);
	return pOut;
}

inline D3DXQUATERNION* WINAPI D3DXQuaternionAxisToAxis(D3DXQUATERNION* pOut, const D3DXVECTOR3* pvFrom, const D3DXVECTOR3* pvTo)
{
	D3DXVECTOR3 vA, vB;
	D3DXVec3Normalize(&vA, pvFrom);
	D3DXVec3Normalize(&vB, pvTo);
	D3DXVECTOR3 vHalf(vA + vB);
	D3DXVec3Normalize(&vHalf, &vHalf);
	return D3DXQuaternionUnitAxisToUnitAxis2(pOut, &vA, &vHalf);
}

class CD3DArcBall
{
	INT            m_iWidth;
	INT            m_iHeight;
	FLOAT          m_fRadius;
	FLOAT          m_fRadiusTranslation;

	D3DXQUATERNION m_qDown;
	D3DXQUATERNION m_qNow;
	D3DXMATRIX     m_matRotation;
	D3DXMATRIX     m_matRotationDelta;
	D3DXMATRIX     m_matTranslation;
	D3DXMATRIX     m_matTranslationDelta;
	BOOL           m_bDrag;
	BOOL           m_bRightHanded;

	D3DXVECTOR3 ScreenToVector(int sx, int sy);

public:

	LRESULT     HandleMouseMessages(HWND, UINT, WPARAM, LPARAM);

	D3DXMATRIX* GetRotationMatrix() { return &m_matRotation; }
	D3DXMATRIX* GetRotationDeltaMatrix() { return &m_matRotationDelta; }
	D3DXMATRIX* GetTranslationMatrix() { return &m_matTranslation; }
	D3DXMATRIX* GetTranslationDeltaMatrix() { return &m_matTranslationDelta; }
	BOOL        IsBeingDragged() { return m_bDrag; }

	VOID        SetRadius(FLOAT fRadius);
	VOID        SetWindow(INT w, INT h, FLOAT r = 0.9);
	VOID        SetRightHanded(BOOL bRightHanded) { m_bRightHanded = bRightHanded; }

	CD3DArcBall();
};

void	RRot2Quat(RQuatKey& q, RRotKey& v);
void	RQuat2Mat(D3DXMATRIX& mat, RQuatKey& q);
int		RMatInv(D3DXMATRIX& q, D3DXMATRIX& a);
void	ConvertMat(rmatrix& mat1, rmatrix& mat2);

inline D3DXVECTOR3 operator*(D3DXVECTOR3& in_vec, D3DXMATRIX& mat) {
	D3DXVECTOR3 out;

	FLOAT x = in_vec.x * mat._11 + in_vec.y * mat._21 + in_vec.z * mat._31 + mat._41;
	FLOAT y = in_vec.x * mat._12 + in_vec.y * mat._22 + in_vec.z * mat._32 + mat._42;
	FLOAT z = in_vec.x * mat._13 + in_vec.y * mat._23 + in_vec.z * mat._33 + mat._43;
	FLOAT w = in_vec.x * mat._14 + in_vec.y * mat._24 + in_vec.z * mat._34 + mat._44;

	out.x = x / w;
	out.y = y / w;
	out.z = z / w;

	return out;
}

inline D3DXMATRIX operator*(D3DXMATRIX& in1, D3DXMATRIX& in2) {
	D3DXMATRIX out;
	D3DXMatrixMultiply(&out, &in1, &in2);
	return out;
}

inline D3DXMATRIX operator~(D3DXMATRIX& in) {
	D3DXMATRIX out;
	D3DXMatrixInverse(&out, 0, &in);
	return out;
}

inline rmatrix RGetRotX(float a) {
	rmatrix mat;
	D3DXMatrixRotationX(&mat, D3DX_PI / 180.f * a);
	return mat;
}

inline rmatrix RGetRotY(float a) {
	rmatrix mat;
	D3DXMatrixRotationY(&mat, D3DX_PI / 180.f * a);
	return mat;
}

inline rmatrix RGetRotZ(float a) {
	rmatrix mat;
	D3DXMatrixRotationZ(&mat, D3DX_PI / 180.f * a);
	return mat;
}

inline rmatrix GetIdentityMatrix() {
	D3DXMATRIX _init_mat;
	D3DXMatrixIdentity(&_init_mat);
	return _init_mat;
}

inline rvector GetTransPos(rmatrix& m) {
	return rvector(m._41, m._42, m._43);
}

void draw_line(LPDIRECT3DDEVICE9 dev, D3DXVECTOR3* vec, int size, DWORD color);
void draw_box(rmatrix* wmat, rvector& max, rvector& min, DWORD color);
void draw_query_fill_box(rmatrix* wmat, rvector& max, rvector& min, DWORD color);

void _GetModelTry(RLVertex* pVert, int size, DWORD color, int* face_num);
void _draw_try(LPDIRECT3DDEVICE9 dev, rmatrix& mat, float size, DWORD color);
void _draw_matrix(LPDIRECT3DDEVICE9 dev, rmatrix& mat, float size);

class RDebugStr
{
public:
	RDebugStr();
	~RDebugStr();

	void Clear();

	void Add(char* str, bool line = true);
	void Add(bool b, bool line = true);
	void Add(char c, bool line = true);
	void Add(short s, bool line = true);
	void Add(WORD  w, bool line = true);
	void Add(int i, bool line = true);
	void Add(DWORD d, bool line = true);
	void Add(float f, bool line = true);
	void Add(rvector& v, bool line = true);
	void AddLine(int cnt = 1);
	void AddTab(int cnt = 1);

	void PrintLog();

public:

	char m_temp[256];

	string m_str;
};

void GetPath(const char* str, char* path);

class RBaseObject
{
public:
	RBaseObject() {
		m_NameID = -1;
	}
	virtual ~RBaseObject() {
	}

public:
	const char* GetName() const;
	void  SetName(const char* name);

	bool  CheckName(const char* name);
	bool  CheckName(const std::string& name);
public:
	int		m_NameID;
	std::string	m_Name;
};

#pragma warning(disable : 4996)

template <class T>
class RHashList : public std::list<T>
{
protected:
	std::unordered_map<std::string, T>	m_HashMap;
	std::unordered_map<int, T>			m_HashMapID;
public:
	void PushBack(T pNode) {
		this->push_back(pNode);
		m_HashMap.insert({ std::string(pNode->GetName()), pNode });
		if (pNode->m_NameID != -1)
			m_HashMapID.insert({ pNode->m_NameID, pNode });
	}

	void Clear() {
		m_HashMap.clear();
		m_HashMapID.clear();
		this->clear();
	}

	auto Erase(typename RHashList<T>::iterator where) {
		auto itor = this->erase(where);

		if (itor != this->end()) {
			auto it = m_HashMap.find(std::string((*itor)->GetName()));

			if (it != m_HashMap.end()) {
				m_HashMap.erase(it);
			}

			auto it_id = m_HashMapID.find((*itor)->m_NameID);

			if (it_id != m_HashMapID.end()) {
				m_HashMapID.erase(it_id);
			}
		}
		return itor;
	}

	T Find(const char* name) {
		auto itor = m_HashMap.find(name);

		if (itor != m_HashMap.end()) {
			return (*itor).second;
		}
		return NULL;
	}

	T Find(int id) {
		auto itor = m_HashMapID.find(id);
		if (itor != m_HashMapID.end()) {
			return itor->second;
		}
		return nullptr;
	}
};
