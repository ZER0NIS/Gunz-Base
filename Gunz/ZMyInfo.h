#pragma once

#include "ZPrerequisites.h"
#include "ZMyItemList.h"
#include "ZMyBuffMgr.h"

// HShield stuff
// TODO: Remove
#define SIZEOF_REQMSG		160
#define SIZEOF_ACKMSG		56
#define SIZEOF_GUIDREQMSG	20
#define SIZEOF_GUIDACKMSG	20

struct ZMySystemInfo
{
	bool			bInGameNoChat;			
	unsigned char	pbyAckMsg[SIZEOF_ACKMSG];		
	unsigned char	pbyGuidAckMsg[SIZEOF_GUIDACKMSG];

	ZMySystemInfo()
	{
		bInGameNoChat = false;
		memset(pbyAckMsg, 0, sizeof(pbyAckMsg));
		memset(pbyGuidAckMsg, 0, sizeof(pbyGuidAckMsg));
	}
};

struct ZMyGameInfo
{
	bool		bForcedChangeTeam;

	ZMyGameInfo() { Init(); }
	void Init()
	{
		bForcedChangeTeam = false;
	}
	void InitRound()		
	{
		bForcedChangeTeam = false;
	}
};

class ZMyInfo
{
private:
	bool			m_bCreated;
	ZMySystemInfo	m_MySystemInfo;
	ZMyGameInfo		m_MyGameInfo;
protected:
	char					m_szAccountID[256];
	MMatchUserGradeID		m_nUGradeID;
	MMatchPremiumGradeID	m_nPGradeID;

	char			m_szCharName[MATCHOBJECT_NAME_LENGTH];
	char			m_szClanName[CLAN_NAME_LENGTH];
	MMatchSex		m_nSex;
	int				m_nHair;		
	int				m_nFace;	
	int				m_nRace;
	unsigned long int		m_nXP;
	int						m_nBP;
	int						m_nLevel;
	int						m_nLevelPercent;
	MMatchClanGrade			m_nClanGrade;
	bool					m_bNewbie;				
	ZMyBuffMgr		        m_BuffMgr;
	ZMyItemList		        m_ItemList;

public:
	ZMyInfo();
	virtual ~ZMyInfo();
	bool InitCharInfo(const char* szCharName, const char* szClanName, const MMatchClanGrade nClanGrade, const MMatchSex nSex, const int nHair, const int nFace);
	bool InitAccountInfo(const char* szAccountID, MMatchUserGradeID nUGradeID, MMatchPremiumGradeID nPGradeID);
	
	void Clear();
	void Destroy();
	void Serialize();	

	static ZMyInfo*		GetInstance();
	ZMyItemList*		GetItemList() { return &m_ItemList; }
	ZMySystemInfo*		GetSystemInfo()	{ return &m_MySystemInfo; }
	ZMyGameInfo*		GetGameInfo()	{ return &m_MyGameInfo; }
	MMatchSex			GetSex() { return m_nSex; }
	int					GetHair() { return m_nHair; }
	int					GetFace() { return m_nFace; }
	int					GetRace() { return m_nRace; }
	int					GetLevel() const { return m_nLevel;}
	int					GetLevelPercent()	{ return m_nLevelPercent; }
	const char*			GetCharName() { return m_szCharName; }
	const char*			GetClanName() { return m_szClanName; }
	const char*			GetAccountID()	{ return m_szAccountID; }
	unsigned long int	GetXP() { return m_nXP; }
	int					GetBP() { return m_nBP; }
	int					GetCash() { return 0; }
	int					GetHP();
	int					GetAP();
	bool				IsNewbie()	{ return m_bNewbie; }
	MMatchUserGradeID	GetUGradeID()		{ return m_nUGradeID; }
	MMatchPremiumGradeID	GetPGradeID()		{ return m_nPGradeID; }
	MMatchClanGrade		GetClanGrade()	{ return m_nClanGrade; }
	bool				IsAdminGrade() {
		if ( (GetUGradeID() == MMUG_EVENTMASTER) || (GetUGradeID() == MMUG_DEVELOPER) || (GetUGradeID() == MMUG_ADMIN) )
			return true;
		else
			return false;
	}
	bool				IsPremiumIPUser() { return (m_nPGradeID == MMPG_PREMIUM_IP); }
	bool				IsClanJoined() { return ((m_szClanName[0] == 0) ? false : true); }

	// set ¾¾¸®Áî
	void SetXP(unsigned long int nXP)			{ m_nXP = nXP; }
	void SetBP(int nBP)							{ m_nBP = nBP; }
	void SetLevel( int nLevel );
	void SetLevelPercent(int nPercent)			{ m_nLevelPercent = nPercent; }
	void SetClanInfo(const char* szClanName, const MMatchClanGrade nClanGrade);
	void SetNewbie(bool bNewbie)				{ m_bNewbie = bNewbie; }
};

inline ZMyInfo* ZGetMyInfo() { return ZMyInfo::GetInstance(); }