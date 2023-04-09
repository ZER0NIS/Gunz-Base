#pragma once

#include "d3d9.h"
#include "RMeshUtil.h"
#include "vector"

using namespace std;

enum MAP_LIGHT_TYPE
{
	GUNFIRE,
	EXPLOSION,
	MAP_LIGHT_NUM,
};

#define MAX_EXPLOSION_LIGHT	10

struct sMapLightObj
{
	rvector		vLightColor;
	float		fRange;
	float		fLife;
	rvector		vPos;
	bool		bUsing;
	sMapLightObj();
};

typedef struct
{
	int			iType;
	rvector		vLightColor;
	float		fRange;
	float		fLife;
} sMapLight;

class RDynamicLightManager
{
protected:
	static	RDynamicLightManager	msInstance;
	static	sMapLight 				msMapLightList[MAP_LIGHT_NUM];

	bool										mbGunLight;
	sMapLightObj						mGunLight;
	vector<sMapLightObj>			mExplosionLightList;

	float								mTime;
	rvector							mvPosition;

	int									miNumEnableLight;

public:
	bool	AddLight(MAP_LIGHT_TYPE light_type_, rvector& pos_);
	void	Update();
	void	Initialize();
	int		SetLight(rvector pos_);
	void	ReleaseLight();
	void	SetPosition(rvector& pos_);

	bool	SetLight(rvector& pos_, int lightIndex_, float maxDistance_);

	static	RDynamicLightManager* GetInstance()
	{
		return &msInstance;
	}
	static	sMapLight* GetLightMapList()
	{
		return msMapLightList;
	}
	bool	IsThereLight()
	{
		if (mbGunLight || mExplosionLightList.size())
		{
			return true;
		}
		return false;
	}

public:
	RDynamicLightManager();
	~RDynamicLightManager();
};

RDynamicLightManager* RGetDynamicLightManager();
sMapLight* RGetMapLightList();