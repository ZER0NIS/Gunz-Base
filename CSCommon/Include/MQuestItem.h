#ifndef _MQUEST_ITEM_H
#define _MQUEST_ITEM_H

#include "MUID.h"
#include <map>
using std::map;

#include <mmsystem.h>

#include "MDebug.h"
#include "MQuestConst.h"
#include "MZFileSystem.h"
#include "MXml.h"
#include "MSync.h"
#include "MMatchGlobal.h"

class MMatchObject;

#define QUEST_ITEM_FILE_NAME	"XML/Quest/zquestitem.xml"

#define MQICTOK_ITEM		"ITEM"
#define MQICTOK_ID			"id"
#define MQICTOK_NAME		"name"
#define MQICTOK_TYPE		"type"
#define MQICTOK_DESC		"desc"
#define MQICTOK_UNIQUE		"unique"
#define MQICTOK_PRICE		"price"
#define MQICTOK_SECRIFICE	"secrifice"
#define MQICTOK_PARAM		"param"

class MQuestMonsterBible
{
public:
	MQuestMonsterBible() {}
	~MQuestMonsterBible() {}

	void WriteMonsterInfo(int nMonsterBibleIndex);
	bool IsKnownMonster(const int nMonsterBibleIndex);

	inline const char MakeBit(const int nMonsterBibleIndex)
	{
		return 0x01 << (nMonsterBibleIndex % 8);
	}

	inline int operator [] (int nIndex) const
	{
		return m_szData[nIndex];
	}

	void Clear() { memset(m_szData, 0, MAX_DB_MONSTERBIBLE_SIZE); }
	const bool Copy(const char* pszData, const int nSize);
	const char* GetData() { return m_szData; }

private:
	char m_szData[MAX_DB_MONSTERBIBLE_SIZE];
};

#define MONSTER_BIBLE_SIZE sizeof(MQuestMonsterBible)

struct MQuestItemDesc
{
	unsigned long int	m_nItemID;
	char				m_szQuestItemName[32];
	int					m_nLevel;
	MQuestItemType		m_nType;
	int					m_nPrice;
	bool				m_bUnique;
	bool				m_bSecrifice;
	char				m_szDesc[8192];
	int					m_nLifeTime;
	int					m_nParam;

	int GetSellBountyValue(int nCnt) { return int(m_nPrice * 0.25) * nCnt; }
};

class MQuestItemDescManager : public map< int, MQuestItemDesc* >
{
public:
	MQuestItemDescManager();
	~MQuestItemDescManager();

	static MQuestItemDescManager& GetInst()
	{
		static MQuestItemDescManager QuestItemDescManager;
		return QuestItemDescManager;
	}

	bool ReadXml(const char* szFileName);
	bool ReadXml(MZFileSystem* pFileSystem, const char* szFileName);
	void ParseQuestItem(MXmlElement& element);
	void Clear();

	MQuestItemDesc* FindQItemDesc(const int nItemID);
	MQuestItemDesc* FindMonserBibleDesc(const int nMonsterBibleIndex);

	inline bool IsQItemID(const int nQItemID)
	{
		if ((MINID_QITEM_LIMIT > nQItemID) || (MAXID_QITEM_LIMIT < nQItemID))
			return false;
		return true;
	}

	inline bool IsMonsterBibleID(const int nQItemID)
	{
		if ((MINID_MONBIBLE_LIMIT > nQItemID) || (MAXID_MONBIBLE_LIMIT < nQItemID))
			return false;
		return true;
	}
private:
	map< int, MQuestItemDesc* >	m_MonsterBibleMgr;
};

#define GetQuestItemDescMgr() MQuestItemDescManager::GetInst()

struct SimpleQuestItem
{
	unsigned long int	m_nItemID;
	unsigned int		m_nCount;
};

class MBaseQuestItem
{
public:
	MBaseQuestItem() {}
	virtual ~MBaseQuestItem() {}
};

class MQuestItem : public MBaseQuestItem
{
public:
	MQuestItem() : MBaseQuestItem(), m_nCount(0), m_pDesc(0), m_bKnown(false)
	{
	}

	virtual ~MQuestItem()
	{
	}

	bool Create(const unsigned long int nItemID, const int nCount, MQuestItemDesc* pDesc, bool bKnown = true);
	int Increase(const int nCount = 1);
	int Decrease(const int nCount = 1);

	unsigned long int	GetItemID() { return m_nItemID; }
	int GetCount() { return m_nCount; }
	bool IsKnown() { return m_bKnown; }
	MQuestItemDesc* GetDesc();
	void SetDesc(MQuestItemDesc* pDesc) { m_pDesc = pDesc; }
	void SetItemID(unsigned long int nItemID) { m_nItemID = nItemID; }

	bool SetCount(int nCount, bool bKnown = true);
private:
	unsigned long int	m_nItemID;
	MQuestItemDesc* m_pDesc;
	int					m_nCount;
	bool				m_bKnown;
};

class MQuestItemMap : public map< unsigned long int, MQuestItem* >
{
public:
	MQuestItemMap() : m_bDoneDbAccess(false)
	{
	}

	~MQuestItemMap() {}

	void SetDBAccess(const bool bState) { m_bDoneDbAccess = bState; }
	bool IsDoneDbAccess() { return m_bDoneDbAccess; }

	virtual bool	CreateQuestItem(const unsigned long int nItemID, const int nCount, bool bKnown = true);
	void			Clear();
	void			Remove(const unsigned long int nItemID);
	MQuestItem* Find(const unsigned long int nItemID);
	void			Insert(unsigned long int nItemID, MQuestItem* pQuestItem);

private:
	bool					m_bDoneDbAccess;
};

class DBQuestCachingData
{
public:
	DBQuestCachingData() : m_dwLastUpdateTime(timeGetTime()), m_nPlayCount(0), m_bEnableUpdate(false), m_nShopTradeCount(0),
		m_pObject(0), m_nRewardCount(0)
	{
	}

	~DBQuestCachingData()
	{
	}

	bool IsRequestUpdate()
	{
		if ((MAX_PLAY_COUNT < m_nPlayCount) || (MAX_ELAPSE_TIME < GetUpdaetElapse()) ||
			(MAX_SHOP_TRADE_COUNT < m_nShopTradeCount) || (MAX_REWARD_COUNT < m_nRewardCount) ||
			m_bEnableUpdate)
			return m_bEnableUpdate = true;

		return m_bEnableUpdate = false;
	}

	bool IsRequestUpdateWhenLogout()
	{
		return ((0 < (m_nShopTradeCount + m_nRewardCount)) || m_bEnableUpdate);
	}

	void IncreasePlayCount(const int nCount = 1);
	void IncreaseShopTradeCount(const int nCount = 1);
	void IncreaseRewardCount(const int nCount = 1);
	bool CheckUniqueItem(MQuestItem* pQuestItem);
	void SacrificeQuestItem() { m_bEnableUpdate = true; }
	void Reset();

	DWORD GetUpdaetElapse()
	{
		return timeGetTime() - m_dwLastUpdateTime;
	}

	void SetEnableUpdateState(const bool bState) { m_bEnableUpdate = bState; }
	void SetCharObject(MMatchObject* pObject) { m_pObject = pObject; }

	bool DoUpdateDBCharQuestItemInfo();

private:
	MMatchObject* m_pObject;
	DWORD			m_dwLastUpdateTime;
	int				m_nPlayCount;
	int				m_nShopTradeCount;
	bool			m_bEnableUpdate;
	int				m_nRewardCount;
};

inline bool IsQuestItemID(unsigned int nItemID)
{
	if ((MIN_QUESTITEM_ID <= nItemID) && (nItemID <= MAX_QUESTITEM_ID)) return true;
	return false;
}

#endif