#pragma once

#include "MMatchGlobal.h"
#include <list>
#include <map>

#include "MMatchObject.h"
#include "MUID.h"

class MCommand;
class MCommandCommunicator;
class MMatchObject;

struct MMatchObjCacheCostume
{
	MMatchSex			nSex;
	unsigned char		nHair;
	unsigned char		nFace;
	unsigned long int	nEquipedItemID[MMCIP_END];
};

class MMatchObjCache {
protected:
	MUID					m_uidObject;
	char					m_szName[MATCHOBJECT_NAME_LENGTH];
	char					m_szClanName[CLAN_NAME_LENGTH];
	char					m_nLevel;
	MMatchUserGradeID		m_nUGrade;
	MMatchPremiumGradeID	m_nPGrade;
	unsigned char			m_nPlayerFlags;
	unsigned int			m_nCLID;
	unsigned int			m_nEmblemChecksum;
	MMatchObjCacheCostume	m_Costume;
	unsigned int			m_nRank;
	int						m_nKillCount;
	int						m_nDeathCount;
	int						m_nDTGrade;

public:
	MMatchObjCache() {
		m_szName[0] = NULL;
		m_nLevel = 0;
		m_nUGrade = MMUG_FREE;
		m_nPGrade = MMPG_FREE;
		memset(&m_Costume, 0, sizeof(MMatchObjCacheCostume));
		ResetFlag();
		m_nDTGrade = 0;
	}
	virtual ~MMatchObjCache() {}

	MUID GetUID() { return m_uidObject; }

	char* GetName() { return m_szName; }
	char* GetClanName() { return m_szClanName; }
	int GetLevel() { return m_nLevel; }
	MMatchUserGradeID		GetUGrade() { return m_nUGrade; }
	MMatchPremiumGradeID	GetPGrade() { return m_nPGrade; }

	void SetInfo(const MUID& uid, const char* szName, const char* szClanName, int nLevel,
		MMatchUserGradeID nUGrade, MMatchPremiumGradeID nPGrade
		, const unsigned int nRank, const int nKillCount, const int nDeathCount, int nDTGrade)
	{
		m_uidObject = uid;
		strcpy(m_szName, szName);
		strcpy(m_szClanName, szClanName);
		m_nLevel = (char)nLevel;
		m_nUGrade = nUGrade;
		m_nPGrade = nPGrade;
		m_nPlayerFlags = 0;
		m_nRank = nRank;
		m_nKillCount = nKillCount;
		m_nDeathCount = nDeathCount;
		m_nDTGrade = nDTGrade;
	}

	unsigned char GetFlags() { return m_nPlayerFlags; }
	void SetFlags(unsigned char nFlags) { m_nPlayerFlags = nFlags; }
	bool CheckFlag(unsigned char nFlagIdx) { return (m_nPlayerFlags & nFlagIdx) ? true : false; }
	void ResetFlag() { m_nPlayerFlags = 0; }
	void SetFlag(unsigned char nFlagIdx, bool bSet)
	{
		if (bSet) m_nPlayerFlags |= nFlagIdx;
		else m_nPlayerFlags &= (0xff ^ nFlagIdx);
	}
	unsigned int GetCLID() { return m_nCLID; }
	void SetCLID(unsigned int nCLID) { m_nCLID = nCLID; }
	unsigned int GetEmblemChecksum() { return m_nEmblemChecksum; }
	void SetEmblemChecksum(unsigned int nChecksum) { m_nEmblemChecksum = nChecksum; }

	const unsigned int GetRank() { return m_nRank; }
	const int GetKillCount() { return m_nKillCount; }
	const int GetDeathCount() { return m_nDeathCount; }

	int GetDTGrade() { return m_nDTGrade; }

	void AssignCostume(MMatchObjCacheCostume* pCostume) { memcpy(&m_Costume, pCostume, sizeof(MMatchObjCacheCostume)); }
	MMatchObjCacheCostume* GetCostume() { return &m_Costume; }
};

class MMatchObjCacheList : public std::list<MMatchObjCache*> {};
class MMatchObjCacheMap : public std::map<MUID, MMatchObjCache*> {
public:
	void Insert(const MUID& uid, MMatchObjCache* pCache) {
		insert(value_type(uid, pCache));
	}
	MMatchObjCache* Find(const MUID& uid) {
		iterator it = find(uid);
		if (it != end())
			return it->second;
		return NULL;
	}
};

enum MATCHCACHEMODE {
	MATCHCACHEMODE_ADD = 0,
	MATCHCACHEMODE_REMOVE,
	MATCHCACHEMODE_UPDATE,
	MATCHCACHEMODE_REPLACE
};

class MMatchObjectCacheBuilder {
	MMatchObjCacheList	m_ObjectCacheList;

public:
	MMatchObjectCacheBuilder();
	virtual ~MMatchObjectCacheBuilder();

	void AddObject(MMatchObject* pObj);
	void Reset();
	MCommand* GetResultCmd(MATCHCACHEMODE nType, MCommandCommunicator* pCmdComm);
};