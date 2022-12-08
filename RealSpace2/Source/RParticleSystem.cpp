#include "stdafx.h"
#include "RParticleSystem.h"
#include "RealSpace2.h"
#include "MDebug.h"

_USING_NAMESPACE_REALSPACE2
_NAMESPACE_REALSPACE2_BEGIN

const u32 POINTVERTEX::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;

bool RParticle::Update(float fTimeElapsed)
{
	velocity += accel * fTimeElapsed;
	position += velocity * fTimeElapsed;
	ftime += fTimeElapsed;

	return true;
}

RParticles::RParticles()
{
	m_Texture = 0;
}

RParticles::~RParticles()
{
	Destroy();
}

bool RParticles::Create(const char* szTextureName, float fSize)
{
	m_Texture = RCreateBaseTexture(szTextureName);
	m_fSize = fSize;

	if (m_Texture == 0)
	{
		return false;
	}
	return true;
}

void RParticles::Destroy()
{
	RDestroyBaseTexture(m_Texture);
	Clear();
}

void RParticles::Clear()
{
	while (size())
	{
		delete* begin();
		erase(begin());
	}
}

inline DWORD FtoDW(FLOAT f) { return *((DWORD*)&f); }

#define LIFETIME	500.f

bool RParticles::Draw()
{
	if (size() == 0)
		return true;

	auto pd3dDevice = RGetDevice();

	HRESULT hr;

	pd3dDevice->SetRenderState(D3DRS_POINTSIZE, FtoDW(m_fSize));
	pd3dDevice->SetTexture(0, m_Texture->GetTexture());

	POINTVERTEX* pVertices;
	DWORD        dwNumParticlesToRender = 0;

	m_RPSystem->m_dwBase += FLUSH_COUNT;

	if (m_RPSystem->m_dwBase >= DISCARD_COUNT)
		m_RPSystem->m_dwBase = 0;

	if (FAILED(m_RPSystem->m_pVB->Lock(m_RPSystem->m_dwBase * sizeof(POINTVERTEX), FLUSH_COUNT * sizeof(POINTVERTEX),
		(VOID**)&pVertices, m_RPSystem->m_dwBase ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD)))
	{
		return false;
	}

	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

	for (iterator i = begin(); i != end(); i++)
	{
		RParticle* pParticle = *i;
		rvector vPos(pParticle->position);
		rvector vVel(pParticle->velocity);
		FLOAT fLength = Magnitude(vVel);
		UINT dwSteps;

		dwSteps = 1;

		for (DWORD j = 0; j < dwSteps; j++)
		{
			pVertices->v = vPos;
			if (!isInViewFrustum(vPos, m_fSize, RGetViewFrustum())) continue;

			static D3DXCOLOR czero = D3DXCOLOR(0, 0, 0, 0), cone = D3DXCOLOR(1, 1, 1, 1);

			D3DXCOLOR color;
			D3DXColorLerp(&color, &cone, &czero, pParticle->ftime / LIFETIME);

			pVertices->color = color;
			pVertices++;

			if (++dwNumParticlesToRender == FLUSH_COUNT)
			{
				m_RPSystem->m_pVB->Unlock();

				if (FAILED(hr = pd3dDevice->DrawPrimitive(D3DPT_POINTLIST, m_RPSystem->m_dwBase, dwNumParticlesToRender)))
					return false;

				m_RPSystem->m_dwBase += FLUSH_COUNT;

				if (m_RPSystem->m_dwBase >= DISCARD_COUNT)
					m_RPSystem->m_dwBase = 0;

				if (FAILED(hr = m_RPSystem->m_pVB->Lock(m_RPSystem->m_dwBase * sizeof(POINTVERTEX), FLUSH_COUNT * sizeof(POINTVERTEX),
					(VOID**)&pVertices, m_RPSystem->m_dwBase ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD)))
				{
					return false;
				}

				dwNumParticlesToRender = 0;
			}
			vPos += vVel;
		}
	}

	m_RPSystem->m_pVB->Unlock();

	if (dwNumParticlesToRender)
	{
		if (FAILED(hr = pd3dDevice->DrawPrimitive(D3DPT_POINTLIST, m_RPSystem->m_dwBase, dwNumParticlesToRender)))
			return false;
	}

	return true;
}

bool RParticles::Update(float fTime)
{
	for (iterator i = begin(); i != end();)
	{
		RParticle* pp = *i;
		if ((pp->ftime > LIFETIME) || (pp->Update(fTime) == false))
		{
			delete pp;
			iterator j = i;
			i++;
			erase(j);
		}
		else
		{
			i++;
		}
	}
	return true;
}

RParticleSystem* RParticleSystem::GetInstance()
{
	static RParticleSystem m_ParticleSystem;
	return &m_ParticleSystem;
}

RParticleSystem::RParticleSystem() : m_dwBase(DISCARD_COUNT)
{
}

RParticleSystem::~RParticleSystem()
{
	Destroy();
}

void RParticleSystem::Destroy()
{
	while (size())
	{
		delete* begin();
		erase(begin());
	}

	Invalidate();
}

bool RParticleSystem::Restore()
{
	HRESULT hr = RGetDevice()->CreateVertexBuffer(DISCARD_COUNT *
		sizeof(POINTVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_POINTS,
		POINTVERTEX::FVF, D3DPOOL_DEFAULT, MakeWriteProxy(m_pVB), NULL);

	if (FAILED(hr))
	{
		SAFE_RELEASE(m_pVB);
		mlog("RParticleSystem::Restore failed\n");
		return false;
	}

	m_dwBase = 0;

	return true;
}

bool RParticleSystem::Invalidate()
{
	SAFE_RELEASE(m_pVB);
	return true;
}

void RParticleSystem::BeginState()
{
	auto pd3dDevice = RGetDevice();

	pd3dDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRS_POINTSCALEENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRS_POINTSIZE_MIN, FtoDW(0.00f));
	pd3dDevice->SetRenderState(D3DRS_POINTSCALE_A, FtoDW(0.00f));
	pd3dDevice->SetRenderState(D3DRS_POINTSCALE_B, FtoDW(0.00f));
	pd3dDevice->SetRenderState(D3DRS_POINTSCALE_C, FtoDW(1.00f));

	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	pd3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);

	pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	pd3dDevice->SetStreamSource(0, m_pVB.get(), 0, sizeof(POINTVERTEX));
	pd3dDevice->SetFVF(POINTVERTEX::FVF);

	rmatrix World;
	D3DXMatrixIdentity(&World);
	RGetDevice()->SetTransform(D3DTS_WORLD, &World);
}

void RParticleSystem::EndState()
{
	auto pd3dDevice = RGetDevice();
	pd3dDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
	pd3dDevice->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
	pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

	pd3dDevice->SetStreamSource(0, NULL, 0, 0);
}

bool RParticleSystem::Draw()
{
	BeginState();
	for (iterator i = begin(); i != end(); i++)
	{
		RParticles* pParticles = *i;
		pParticles->Draw();
	}
	EndState();
	return true;
}

bool RParticleSystem::Update(float fTime)
{
	for (iterator i = begin(); i != end(); i++)
	{
		RParticles* pParticles = *i;
		pParticles->Update(fTime);
	}

	return true;
}

RParticles* RParticleSystem::AddParticles(const char* szTextureName, float fSize)
{
	RParticles* pp = new RParticles;
	if (!(pp->Create(szTextureName, fSize)))
	{
		return NULL;
	}
	push_back(pp);

	return pp;
}

_NAMESPACE_REALSPACE2_END