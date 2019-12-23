#ifndef MMatchBuff_H
#define MMatchBuff_H

//#include "MMatchBuffDesc.h"

#include <map>
using namespace std;

enum MMatchBuffEffectType
{
	MMBET_NORMAL = 0,
	MMBET_DOTE,
	MMBET_END,
};

enum MMatchBuffPeriodType
{
	MMBPT_NONE = 0,
	MMBPT_LONG,
	MMBPT_SHORT,
	MMBPT_END,
};

struct MMatchBuffName
{
	char szBuffName[128];
	char szBuffIconName[128];
};

struct MMatchBuffInfo
{
	int nHP;
	int nAP;

	float fSpeed_Ratio;

	int nRespawnDecTime;

	inline MMatchBuffInfo& operator=(const MMatchBuffInfo& b)
	{
		nHP = b.nHP;
		nAP = b.nAP;

		fSpeed_Ratio = b.fSpeed_Ratio;

		nRespawnDecTime = b.nRespawnDecTime;

		return *this;
	}

	inline MMatchBuffInfo& operator+(const MMatchBuffInfo& b)
	{
		nHP = nHP + b.nHP;
		nAP = nAP + b.nAP;

		fSpeed_Ratio = fSpeed_Ratio * b.fSpeed_Ratio;

		nRespawnDecTime = nRespawnDecTime + b.nRespawnDecTime;

		return *this;
	}
};

struct MMatchBuffDesc
{
	unsigned long int		m_nBuffID;

	MProtectValue<MMatchBuffEffectType> m_nBuffEffectType;
	MProtectValue<MMatchBuffPeriodType>	m_nBuffPeriodType;
	MProtectValue<int>					m_nBuffPeriod;

	MProtectValue<MMatchBuffInfo>*		m_pBuffInfo;

	char				m_szBuffName[128];
	char				m_szBuffDesc[8192];
	char				m_szBuffIconName[128];

	MMatchBuffDesc();
	~MMatchBuffDesc();	

	void CacheCRC32( MMatchCRC32XORCache& crc );
	void ShiftFugitiveValues();
};

class MMatchBuffDescMgr : public map<int, MMatchBuffDesc*>
{
protected:
	unsigned long m_nChecksum;

	MMatchBuffDescMgr();
	virtual ~MMatchBuffDescMgr();

	bool ParseItem(MXmlElement& element);

public:
	bool ReadXml(const char* szFileName);
	bool ReadXml(MZFileSystem* pFileSystem, const char* szFileName);

	void Clear();

	MMatchBuffDesc* GetBuffDesc(unsigned long int nBuffID);

	unsigned long GetChecksum() { return m_nChecksum; }

	static MMatchBuffDescMgr* GetInstance() 
	{
		static MMatchBuffDescMgr m_BuffDescMgr;
		return &m_BuffDescMgr;
	}


	bool SetBuffName(MMatchItemDescMgr* pItemDescMgr);
};

inline MMatchBuffDescMgr* MGetMatchBuffDescMgr() { return MMatchBuffDescMgr::GetInstance(); }

#include "../../StringLiteral/cxr_MMatchBuff.h"


class MMatchBuff
{
protected:	
	MUID	m_uidBuff;

	int		m_nBuffID;
	int		m_nRegTime;	
	int		m_nBuffPeriodRemainder;

	MMatchBuffDesc* m_pBuffDesc;

public:
	MMatchBuff(){}
	~MMatchBuff(){}

	virtual void Reset();
	virtual bool Set(MUID& uidBuff, int nBuffID, int nRegTime, int nBuffPeriodRemainder);

	bool IsExpired(int nGlobalTick);

	MUID GetBuffUID()				{ return m_uidBuff; }
	int  GetBuffID()				{ return m_pBuffDesc == NULL ? 0 : m_pBuffDesc->m_nBuffID; }
	int  GetBuffPeriod()			{ return m_pBuffDesc == NULL ? 0 : m_pBuffDesc->m_nBuffPeriod.Ref(); }	
	
	int  GetBuffPeriodRemainder(int nGlobalTick);
};



/////////////////////////////////////////////////////////////////////////////
// MMatchShortBuff
class MMatchShortBuff : public MMatchBuff
{
protected:
public:
	MMatchShortBuff() : MMatchBuff() {}
	~MMatchShortBuff() {}
};


/////////////////////////////////////////////////////////////////////////////
// MMatchShortBuffMap
class MMatchShortBuffMap : public map<MUID, MMatchShortBuff*>
{
protected:
public:
	void Clear();
	void Remove(MUID& uidBuff);
	bool Insert(MUID& uidBuff, MMatchShortBuff* pBuff);
	MMatchShortBuff* GetShortBuffByBuffID(int nBuffID);
};

#endif