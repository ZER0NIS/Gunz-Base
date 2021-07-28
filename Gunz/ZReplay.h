#ifndef _ZREPLAY_H
#define _ZREPLAY_H

struct REPLAY_STAGE_SETTING_NODE
{
	MUID				uidStage;
	char				szMapName[MAPNAME_LENGTH];
	char				nMapIndex;
	MMATCH_GAMETYPE		nGameType;
	int					nRoundMax;
	int					nLimitTime;
	int					nLimitLevel;
	int					nMaxPlayers;
	bool				bTeamKillEnabled;
	bool				bTeamWinThePoint;
	bool				bForcedEntryEnabled;
	char				szStageName[STAGENAME_LENGTH]; // Custom: Added stage name to replays
};

void ConvertStageSettingNodeForReplay(const REPLAY_STAGE_SETTING_NODE* pSource, MSTAGE_SETTING_NODE* pTarget);
void ConvertStageSettingNodeForRecord(const MSTAGE_SETTING_NODE* pSource, REPLAY_STAGE_SETTING_NODE* pTarget);

extern bool g_bTestFromReplay;

bool CreateReplayGame(const char* filename);
void ConvertCharInfo(MTD_CharInfo* currCharInfo, void* oldCharInfo, int nVerOld);

#define GUNZ_REC_FILE_ID		0x95b1308a

#define GUNZ_REC_FILE_VERSION	7
#define GUNZ_REC_FILE_EXT		"gzr"

class ZReplay
{
private:
public:
	ZReplay() {}
	~ZReplay() {}
};

class ZGame;

class ZReplayLoader
{
public:
	static unsigned int				m_nVersion;
private:
	REPLAY_STAGE_SETTING_NODE		m_StageSetting;
	float							m_fGameTime;

	bool LoadHeader(ZFile* file);
	bool LoadStageSetting(ZFile* file);
	bool LoadStageSettingEtc(ZFile* file);
	bool LoadCharInfo(ZFile* file);
	bool LoadCommandStream(ZFile* file);

	void ChangeGameState();

	MCommand* CreateCommandFromStream(char* pStream);
	MCommand* CreateCommandFromStreamVersion2(char* pStream);
	bool ParseVersion2Command(char* pStream, MCommand* pCmd);
	MCommandParameter* MakeVersion2CommandParameter(MCommandParameterType nType, char* pStream, unsigned short int* pnDataCount);
public:
	ZReplayLoader();
	~ZReplayLoader() {}
	bool Load(const char* filename);
	float GetGameTime() const { return m_fGameTime; }
};

#pragma pack(push, old)
#pragma pack(1)

struct MTD_CharInfo_v0
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

	unsigned long int	nEquipedItemDesc[12];

	MMatchUserGradeID	nUGradeID;
};

struct MTD_CharInfo_v2
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

	unsigned long int	nEquipedItemDesc[12];

	MMatchUserGradeID	nUGradeID;

	unsigned int		nClanCLID;
};

struct MTD_CharInfo_v5
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

	unsigned long int	nEquipedItemDesc[12];

	MMatchUserGradeID	nUGradeID;

	unsigned int		nClanCLID;

	int					nDTLastWeekGrade;
};

#pragma pack(pop, old)

#endif