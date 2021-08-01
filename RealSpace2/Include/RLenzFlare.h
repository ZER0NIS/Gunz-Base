#pragma once

#include "RBaseTexture.h"

_NAMESPACE_REALSPACE2_BEGIN

enum FLARE_ELEMENT_TYPE
{
	FLARE_ELEMENT_SPHERE = 0,
	FLARE_ELEMENT_RING,
	FLARE_ELEMENT_SPOT,
	FLARE_ELEMENT_POLYGON,
	FLARE_ELEMENT_ETC,
	FLARE_ELEMENT_GROW,
	MAX_NUMBER_ELEMENT = 10,
};

#define MAX_LENZFLARE_NUMBER 1
#define MAX_NUMBER_TEXTURE MAX_NUMBER_ELEMENT

struct sFlareElement
{
	int iType;
	float width, height;
	DWORD color;
	int iTextureIndex;
};

class RBspObject;

class RLenzFlare
{
protected:
	static bool    mbIsReady;
	static RLenzFlare  msInstance;
	static sFlareElement msElements[MAX_NUMBER_ELEMENT];
	int        miNumFlareElement;
	int* miElementOrder;
	static RealSpace2::RBaseTexture* msTextures[MAX_NUMBER_TEXTURE];
	rvector      mLightList[MAX_LENZFLARE_NUMBER];
	int        miNumLight;

protected:
	static bool ReadXmlElement(MXmlElement* PNode, char* Path);
	bool  open(const char* pFileName_, MZFileSystem* pfs_);
	bool  draw(float x_, float y_,
		float width_, float height_,
		float alpha,
		DWORD color_,
		int textureIndex_);

public:
	static bool Create(const char* filename_);
	static bool Destroy();
	static bool IsReady();
	static RLenzFlare* GetInstance() { return &msInstance; }

	void  Initialize();
	bool  Render(rvector& light_pos, rvector& centre_, RBspObject* pbsp_);
	bool  Render(rvector& centre_, RBspObject* pbsp_);
	bool  SetLight(rvector& pos_);
	void  Clear() { miNumLight = 0; }
	int   GetNumLight() const { return miNumLight; }
	rvector GetLightPos(int i) const { return mLightList[i]; }

public:
	RLenzFlare(void);
	~RLenzFlare(void);
};

#ifndef __DEFINED_GLOBAL_LENZFLARE_METHOD__
#define __DEFINED_GLOBAL_LENZFLARE_METHOD__

bool RCreateLenzFlare(const char* filename_);
bool RDestroyLenzFlare();
bool RReadyLenzFlare();

RLenzFlare* RGetLenzFlare();

#endif

_NAMESPACE_REALSPACE2_END