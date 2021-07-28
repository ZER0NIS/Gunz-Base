#pragma once

#include "MMatchGlobal.h"

enum MMATCH_MAP
{
	MMATCH_MAP_MANSION = 0,
	MMATCH_MAP_PRISON = 1,
	MMATCH_MAP_STATION = 2,
	MMATCH_MAP_PRISON_II = 3,
	MMATCH_MAP_BATTLE_ARENA = 4,
	MMATCH_MAP_TOWN = 5,
	MMATCH_MAP_DUNGEON = 6,
	MMATCH_MAP_RUIN = 7,
	MMATCH_MAP_ISLAND = 8,
	MMATCH_MAP_GARDEN = 9,
	MMATCH_MAP_CASTLE = 10,
	MMATCH_MAP_FACTORY = 11,
	MMATCH_MAP_PORT = 12,
	MMATCH_MAP_LOST_SHRINE = 13,
	MMATCH_MAP_STAIRWAY = 14,
	MMATCH_MAP_SNOWTOWN = 15,
	MMATCH_MAP_HALL = 16,
	MMATCH_MAP_CATACOMB = 17,
	MMATCH_MAP_JAIL = 18,
	MMATCH_MAP_SHOWERROOM = 19,
	MMATCH_MAP_HIGH_HAVEN = 20,
	MMATCH_MAP_CITADEL = 21,

	MMATCH_MAP_RELAYMAP = 22,

	MMATCH_MAP_WEAPON_SHOP = 23,
	MMATCH_MAP_MAX = 512 	// Custom: Raise the max maps to 512
};

#define MMATCH_MAP_COUNT	MMATCH_MAP_MAX

#define MMATCH_MAPNAME_RELAYMAP				"RelayMap"

class MMapDesc
{
private:
	const struct MapInfo
	{
		int			nMapID;
		char		szMapName[MAPNAME_LENGTH];
		char		szMapImageName[MAPNAME_LENGTH];
		char		szBannerName[MAPNAME_LENGTH];
		float		fExpRatio;
		int			nMaxPlayers;
		bool		bOnlyDuelMap;
		bool		bIsCTFMap;
	};

	MapInfo	m_MapVectors[MMATCH_MAP_COUNT];
	MMapDesc();
public:
	~MMapDesc() { }

	static MMapDesc* GetInstance();

	bool Initialize(const char* szFileName);
	bool Initialize(MZFileSystem* pfs, const char* szFileName);
	bool MIsCorrectMap(const int nMapID);

	bool IsMapOnlyDuel(const int nMapID);
	bool IsCTFMap(const int nMapID);
	int GetMapID(const int nMapID);
	const char* GetMapName(const int nMapID);
	const char* GetMapImageName(const char* szMapName);
	const char* GetBannerName(const char* szMapName);
	float GetExpRatio(const int nMapID);
	int GetMaxPlayers(const int nMapID);

	int GetMapCount() { return MMATCH_MAP_COUNT; }
};

inline MMapDesc* MGetMapDescMgr()
{
	return MMapDesc::GetInstance();
}