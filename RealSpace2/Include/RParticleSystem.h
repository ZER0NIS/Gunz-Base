#pragma once

#include "RTypes.h"
#include "RBaseTexture.h"

#define DISCARD_COUNT	2048
#define FLUSH_COUNT		512

_NAMESPACE_REALSPACE2_BEGIN

struct RParticle {
	rvector position;
	rvector velocity;
	rvector accel;
	float	ftime;

	virtual bool Update(float fTimeElapsed);
};

struct POINTVERTEX
{
	D3DXVECTOR3 v;
	D3DCOLOR color;

	static const u32 FVF;
};

class RParticleSystem;

class RParticles : public std::list<RParticle*> {
protected:
	rvector mInitialPos;

public:
	RParticles();
	virtual ~RParticles();

	bool Create(const char* szTextureName, float fSize);
	void Destroy();
	void Clear();

	virtual bool Draw();
	virtual bool Update(float fTime);

protected:
	float	m_fSize;

	RBaseTexture* m_Texture;
	RParticleSystem* m_RPSystem;
};

class RParticleSystem : public std::list<RParticles*> {
public:
	RParticleSystem();
	virtual ~RParticleSystem();

	void Destroy();

	virtual bool Draw();
	virtual bool Update(float fTime);

	RParticles* AddParticles(const char* szTextureName, float fSize);

	void BeginState();
	void EndState();

	bool Restore();
	bool Invalidate();

	D3DPtr<IDirect3DVertexBuffer9> m_pVB;
	DWORD m_dwBase;

	static RParticleSystem* GetInstance();
};

inline RParticleSystem* RGetParticleSystem() { return RParticleSystem::GetInstance(); }

_NAMESPACE_REALSPACE2_END