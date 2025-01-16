#ifndef _MMATCHTRANSDATATYPE_H
#define _MMATCHTRANSDATATYPE_H

#include "MMatchObject.h"
#include "MMatchRule.h"
#include "MMatchStageSetting.h"
#include "MMatchGameType.h"
#include "MMatchGlobal.h"

#pragma pack(push, old)
#pragma pack(1)

struct MTD_AccountCharInfo
{
	char				szName[MATCHOBJECT_NAME_LENGTH];
	char				nCharNum;
	unsigned char		nLevel;
};

struct MTD_CharInfo
{
	char				szName[MATCHOBJECT_NAME_LENGTH];
	char				szClanName[CLAN_NAME_LENGTH];
	MMatchClanGrade		nClanGrade;
	unsigned short		nClanContPoint;
	char				nCharNum;
	unsigned short		nLevel;
	char				nSex;
	char				nHair;
	char				nFace;
	unsigned long int	nXP;
	int					nBP;
	float				fBonusRate;
	unsigned short		nPrize;
	unsigned short		nHP;
	unsigned short		nAP;
	unsigned short		nMaxWeight;
	unsigned short		nSafeFalls;
	unsigned short		nFR;
	unsigned short		nCR;
	unsigned short		nER;
	unsigned short		nWR;

	unsigned long int	nEquipedItemDesc[MMCIP_END];

	MMatchUserGradeID	nUGradeID;

	unsigned int		nClanCLID;

	int					nDTLastWeekGrade;

	MUID				uidEquipedItem[MMCIP_END];
	unsigned long int	nEquipedItemCount[MMCIP_END];
};

struct MTD_BuffInfo
{
	unsigned long int	nItemId;
	unsigned short		nRemainedTime;
};

struct MTD_MyExtraCharInfo
{
	char	nLevelPercent;
};

struct MTD_SimpleCharInfo
{
	char				szName[32];
	char				nLevel;
	char				nSex;
	char				nHair;
	char				nFace;
	unsigned long int	nEquipedItemDesc[MMCIP_END];
};

struct MTD_MySimpleCharInfo
{
	unsigned char		nLevel;
	unsigned long int	nXP;
	int					nBP;
};

struct MTD_CharLevelInfo
{
	unsigned char		nLevel;
	unsigned long int	nCurrLevelExp;
	unsigned long int	nNextLevelExp;
};

struct MTD_RoundPeerInfo
{
	MUID			uidChar;
	unsigned char	nHP;
	unsigned char	nAP;
};

struct MTD_RoundKillInfo
{
	MUID	uidAttacker;
	MUID	uidVictim;
};

struct MTD_ItemNode
{
	MUID				uidItem;
	unsigned long int	nItemID;
	int					nRentMinutePeriodRemainder;
	int					iMaxUseHour;
	int					nCount;
};

struct MTD_RelayMap
{
	int				nMapID;
};

struct MTD_AccountItemNode
{
	int					nAIID;
	unsigned long int	nItemID;
	int					nRentMinutePeriodRemainder;
	int					nCount;
};

struct MTD_GameInfoPlayerItem
{
	MUID	uidPlayer;
	bool	bAlive;
	int		nKillCount;
	int		nDeathCount;
};

struct MTD_GameInfo
{
	char	nRedTeamScore;
	char	nBlueTeamScore;

	short	nRedTeamKills;
	short	nBlueTeamKills;
};

struct MTD_RuleInfo
{
	unsigned char	nRuleType;
};

struct MTD_RuleInfo_Assassinate : public MTD_RuleInfo
{
	MUID	uidRedCommander;
	MUID	uidBlueCommander;
};

struct MTD_RuleInfo_Berserker : public MTD_RuleInfo
{
	MUID	uidBerserker;
};

enum MTD_PlayerFlags {
	MTD_PlayerFlags_AdminHide = 1,
	MTD_PlayerFlags_BridgePeer = 1 << 1,
	MTD_PlayerFlags_Premium = 1 << 2
};

struct MTD_ChannelPlayerListNode
{
	MUID			uidPlayer;
	char			szName[MATCHOBJECT_NAME_LENGTH];
	char			szClanName[CLAN_NAME_LENGTH];
	char			nLevel;
	char			nDTLastWeekGrade;
	MMatchPlace		nPlace;
	unsigned char	nGrade;
	unsigned char	nPlayerFlags;
	unsigned int	nCLID;
	unsigned int	nEmblemChecksum;
};

struct MTD_ClanMemberListNode
{
	MUID				uidPlayer;
	char				szName[MATCHOBJECT_NAME_LENGTH];
	char				nLevel;
	MMatchClanGrade		nClanGrade;
	MMatchPlace			nPlace;
};

enum MTD_WorldItemSubType
{
	MTD_Dynamic = 0,
	MTD_Static = 1,
};

struct MTD_WorldItem
{
	unsigned short	nUID;
	unsigned short	nItemID;
	unsigned short  nItemSubType;
	short			x;
	short			y;
	short			z;
};

struct MTD_ActivatedTrap
{
	MUID				uidOwner;
	unsigned short		nItemID;
	unsigned long int	nTimeElapsed;
	short	x;
	short	y;
	short	z;
};

struct MTD_QuickJoinParam
{
	unsigned long int	nMapEnum;
	unsigned long int	nModeEnum;
};

struct MTD_CharClanInfo
{
	char				szClanName[CLAN_NAME_LENGTH];
	MMatchClanGrade		nGrade;
};

struct MTD_CharInfo_Detail
{
	char				szName[32];
	char				szClanName[CLAN_NAME_LENGTH];
	MMatchClanGrade		nClanGrade;
	int					nClanContPoint;
	unsigned short		nLevel;
	char				nSex;
	char				nHair;
	char				nFace;
	unsigned long int	nXP;
	int					nBP;

	int					nKillCount;
	int					nDeathCount;

	unsigned long int	nTotalPlayTimeSec;
	unsigned long int	nConnPlayTimeSec;

	unsigned long int	nEquipedItemDesc[MMCIP_END];

	MMatchUserGradeID	nUGradeID;

	unsigned int		nClanCLID;
};

struct MTD_StageListNode
{
	MUID			uidStage;
	unsigned char	nNo;
	char			szStageName[STAGENAME_LENGTH];
	char			nPlayers;
	char			nMaxPlayers;
	STAGE_STATE		nState;
	MMATCH_GAMETYPE nGameType;
	char			nMapIndex;
	int				nSettingFlag;
	char			nMasterLevel;
	char			nLimitLevel;
};

struct MTD_ExtendInfo
{
	char			nTeam;
	unsigned char	nPlayerFlags;
	unsigned char	nReserved1;
	unsigned char	nReserved2;
};

struct MTD_PeerListNode
{
	MUID				uidChar;
	DWORD				dwIP;
	unsigned int		nPort;
	MTD_CharInfo		CharInfo;
	MTD_ExtendInfo		ExtendInfo;
};

struct MTD_ReplierNode
{
	char szName[MATCHOBJECT_NAME_LENGTH];
};

struct MTD_LadderTeamMemberNode
{
	char szName[MATCHOBJECT_NAME_LENGTH];
};

struct MTD_ClanInfo
{
	char				szClanName[CLAN_NAME_LENGTH];
	short				nLevel;
	int					nPoint;
	int					nTotalPoint;
	int					nRanking;
	char				szMaster[MATCHOBJECT_NAME_LENGTH];
	unsigned short		nWins;
	unsigned short		nLosses;
	unsigned short		nTotalMemberCount;
	unsigned short		nConnedMember;
	unsigned int		nCLID;
	unsigned int		nEmblemChecksum;
};

struct MTD_StandbyClanList
{
	char				szClanName[CLAN_NAME_LENGTH];
	short				nPlayers;
	short				nLevel;
	int					nRanking;
	unsigned int		nCLID;
	unsigned int		nEmblemChecksum;
};

struct MTD_QuestGameInfo
{
	unsigned short		nQL;
	float				fNPC_TC;
	unsigned short		nNPCCount;

	unsigned char		nNPCInfoCount;
	unsigned char		nNPCInfo[MAX_QUEST_NPC_INFO_COUNT];
	unsigned short		nMapSectorCount;
	unsigned short		nMapSectorID[MAX_QUEST_MAP_SECTOR_COUNT];
	char				nMapSectorLinkIndex[MAX_QUEST_MAP_SECTOR_COUNT];
	unsigned char		nRepeat;
	MMATCH_GAMETYPE		eGameType;
};

struct MTD_QuestReward
{
	MUID				uidPlayer;
	int					nXP;
	int					nBP;
};

struct MTD_QuestItemNode
{
	int		m_nItemID;
	int		m_nCount;
};

struct MTD_QuestZItemNode
{
	unsigned int		m_nItemID;
	int					m_nRentPeriodHour;
	int					m_nItemCnt;
};

struct MTD_NPCINFO
{
	BYTE	m_nNPCTID;
	WORD	m_nMaxHP;
	WORD	m_nMaxAP;
	BYTE	m_nInt;
	BYTE	m_nAgility;
	float	m_fAngle;
	float	m_fDyingTime;

	float	m_fCollisonRadius;
	float	m_fCollisonHight;

	BYTE	m_nAttackType;
	float	m_fAttackRange;
	DWORD	m_nWeaponItemID;
	float	m_fDefaultSpeed;
};

struct MTD_SurvivalRanking
{
	char	m_szCharName[MATCHOBJECT_NAME_LENGTH];
	DWORD	m_dwPoint;
	DWORD	m_dwRank;

	MTD_SurvivalRanking() : m_dwPoint(0), m_dwRank(0) { m_szCharName[0] = 0; }
};

#if defined(LOCALE_NHNUSAA)
struct MTD_ServerStatusInfo
{
	DWORD			m_dwIP;
	DWORD			m_dwAgentIP;
	int				m_nPort;
	unsigned char	m_nServerID;
	short			m_nMaxPlayer;
	short			m_nCurPlayer;
	char			m_nType;
	bool			m_bIsLive;
	char			m_szServerName[64];
};
#else
struct MTD_ServerStatusInfo
{
	DWORD			m_dwIP;
	int				m_nPort;
	unsigned char	m_nServerID;
	short			m_nMaxPlayer;
	short			m_nCurPlayer;
	char			m_nType;
	bool			m_bIsLive;
	char			m_szServerName[64];
};
#endif

struct MTD_ResetTeamMembersData
{
	MUID			m_uidPlayer;
	char			nTeam;
};

struct MTD_DuelQueueInfo
{
	MUID			m_uidChampion;
	MUID			m_uidChallenger;
	MUID			m_WaitQueue[14];
	char			m_nQueueLength;
	char			m_nVictory;
	bool			m_bIsRoundEnd;
};

struct MTD_DuelTournamentGameInfo
{
	MUID			uidPlayer1;
	MUID			uidPlayer2;
	int				nMatchType;
	int				nMatchNumber;
	int				nRoundCount;
	bool			bIsRoundEnd;
	char			nWaitPlayerListLength;
	BYTE			dummy[2];
	MUID			WaitPlayerList[8];
};

struct MTD_DuelTournamentNextMatchPlayerInfo
{
	MUID			uidPlayer1;
	MUID			uidPlayer2;
};

struct MTD_DuelTournamentRoundResultInfo
{
	MUID			uidWinnerPlayer;
	MUID			uidLoserPlayer;
	bool			bIsTimeOut;
	bool			bDraw;
	bool			bIsMatchFinish;
	BYTE			dummy[2];
};

struct MTD_DuelTournamentMatchResultInfo
{
	int				nMatchNumber;
	int				nMatchType;
	MUID			uidWinnerPlayer;
	MUID			uidLoserPlayer;
	int				nGainTP;
	int				nLoseTP;
};

#pragma pack(pop, old)

enum ZAdminAnnounceType
{
	ZAAT_CHAT = 0,
	ZAAT_MSGBOX
};

struct MTD_GambleItemNode
{
	MUID			uidItem;
	unsigned int	nItemID;
	unsigned int	nItemCnt;
};

struct MTD_DBGambleItmeNode
{
	unsigned int	nItemID;
	char			szName[MAX_GAMBLEITEMNAME_LEN];
	char			szDesc[MAX_GAMBLEITEMDESC_LEN];
	int				nBuyPrice;
	bool			bIsCash;
};

struct MTD_ShopItemInfo
{
	unsigned int	nItemID;
	int				nItemCount;
};

struct MTD_ShotInfo
{
	unsigned long nLowId;
	float fPosX;
	float fPosY;
	float fPosZ;
	float fDamage;
	float fRatio;
	char nDamageType;
	char nWeaponType;
};
void Make_MTDItemNode(MTD_ItemNode* pout, MUID& uidItem, unsigned long int nItemID, int nRentMinutePeriodRemainder, int iMaxUseHour, int nCount);
void Make_MTDAccountItemNode(MTD_AccountItemNode* pout, int nAIID, unsigned long int nItemID, int nRentMinutePeriodRemainder, int nCount);

void Make_MTDQuestItemNode(MTD_QuestItemNode* pOut, const unsigned long int nItemID, const int nCount);

struct MMatchWorldItem;
void Make_MTDWorldItem(MTD_WorldItem* pOut, MMatchWorldItem* pWorldItem);

class MMatchActiveTrap;
void Make_MTDActivatedTrap(MTD_ActivatedTrap* pOut, MMatchActiveTrap* pTrapItem);

inline unsigned long int MakeExpTransData(int nAddedXP, int nPercent)
{
	unsigned long int ret = 0;
	ret |= (nAddedXP & 0x0000FFFF) << 16;
	ret |= nPercent & 0xFFFF;
	return ret;
}
inline int GetExpFromTransData(unsigned long int nValue)
{
	return (int)((nValue & 0xFFFF0000) >> 16);
}
inline int GetExpPercentFromTransData(unsigned long int nValue)
{
	return (int)(nValue & 0x0000FFFF);
}

#endif
