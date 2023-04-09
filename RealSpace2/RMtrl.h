#pragma once

#include <list>
#include <vector>
#include <string>
#include <map>
#include <memory>

#include <D3DX9.h>

#include "RBaseTexture.h"

_USING_NAMESPACE_REALSPACE2

class RMtrl
{
public:
	RMtrl();
	~RMtrl();
	RMtrl(const RMtrl& rhs) = delete;

	void CheckAniTexture();

	LPDIRECT3DTEXTURE9 GetTexture();

	void Restore(LPDIRECT3DDEVICE9 dev, char* path = NULL);

	void	SetTColor(DWORD color);
	DWORD	GetTColor();

public:
	std::unique_ptr<RBaseTexture> m_pTexture;
	std::unique_ptr<RBaseTexture> m_pToonTexture;

	D3DTEXTUREFILTERTYPE m_FilterType;
	D3DTEXTUREFILTERTYPE m_ToonFilterType;

	DWORD				m_AlphaRefValue;
	D3DTEXTUREOP		m_TextureBlendMode;
	D3DTEXTUREOP		m_ToonTextureBlendMode;

	D3DXCOLOR	m_ambient;
	D3DXCOLOR	m_diffuse;
	D3DXCOLOR	m_specular;

	float	m_power;

	char	m_mtrl_name[255];

	char	m_name[255];
	char	m_opa_name[255];

	char	m_name_ani_tex[255];
	char	m_name_ani_tex_ext[20];

	int		m_id;
	int		m_u_id;
	int		m_mtrl_id;
	int		m_sub_mtrl_id;

	int		m_sub_mtrl_num;

	bool	m_bDiffuseMap;
	bool	m_bTwoSided;
	bool	m_bAlphaMap;
	bool	m_bAlphaTestMap;
	bool	m_bAdditive;

	int		m_nAlphaTestValue;

	bool	m_bUse;

	bool	m_bAniTex;
	int 	m_nAniTexCnt;
	int 	m_nAniTexSpeed;
	int 	m_nAniTexGap;
	DWORD	m_backup_time;

	bool	m_bObjectMtrl;

	DWORD	m_dwTFactorColor;

	std::unique_ptr<std::unique_ptr<RBaseTexture>[]> m_pAniTexture;
};

using namespace std;

#define MAX_MTRL_NODE 100

class RMtrlMgr final : public std::list<RMtrl*>
{
public:
	RMtrlMgr();
	~RMtrlMgr();
	RMtrlMgr(const RMtrlMgr&) = delete;

	int		Add(char* name, int u_id = -1);
	int		Add(RMtrl* tex);

	void	Del(const std::unique_ptr<RMtrl>& tex);

	void	Del(RMtrl* tex);
	void	Del(int id);

	int		LoadList(char* name);
	int		SaveList(char* name);

	void	DelAll();
	void	Restore(LPDIRECT3DDEVICE9 dev, char* path = NULL);

	void	ClearUsedCheck();
	void	ClearUsedMtrl();

	void	SetObjectTexture(bool bObject) { m_bObjectMtrl = bObject; }

	RMtrl* Get_s(int mtrl_id, int sub_id);

	LPDIRECT3DTEXTURE9 Get(int id);
	LPDIRECT3DTEXTURE9 Get(int id, int sub_id);
	LPDIRECT3DTEXTURE9 GetUser(int id);
	LPDIRECT3DTEXTURE9 Get(char* name);

	RMtrl* GetMtrl(char* name);
	RMtrl* GetToolMtrl(char* name);

	int		GetNum();

	std::vector<RMtrl*>	m_node_table;

	bool	m_bObjectMtrl;
	int		m_id_last;
};
