#ifndef _ZREPLAY_H
#define _ZREPLAY_H

struct REPLAY_STAGE_SETTING_NODE 
{
	MUID				uidStage;
	char				szMapName[MAPNAME_LENGTH];	// 맵이름
	char				nMapIndex;					// 맵인덱스
	MMATCH_GAMETYPE		nGameType;					// 게임타입
	int					nRoundMax;					// 라운드
	int					nLimitTime;					// 제한시간(1 - 1분)
	int					nLimitLevel;				// 제한레벨
	int					nMaxPlayers;				// 최대인원
	bool				bTeamKillEnabled;			// 팀킬여부
	bool				bTeamWinThePoint;			// 선승제 여부
	bool				bForcedEntryEnabled;		// 게임중 참가 여부
	char				szStageName[64];
};

void ConvertStageSettingNodeForReplay(const REPLAY_STAGE_SETTING_NODE* pSource, MSTAGE_SETTING_NODE* pTarget);
void ConvertStageSettingNodeForRecord(const MSTAGE_SETTING_NODE* pSource, REPLAY_STAGE_SETTING_NODE* pTarget);

extern bool g_bTestFromReplay;

bool CreateReplayGame(const char *filename);
void ConvertCharInfo(MTD_CharInfo* currCharInfo, void* oldCharInfo, int nVerOld);

// 건즈 리플레이 파일
#define GUNZ_REC_FILE_ID		0x95b1308a

// version 4 : duel 모드의 현재 상태 저장이 추가되었습니다.
// version 5 : duel_tournament용 정보가 추가되었습니다. (MTD_CharInfo에 지난주 랭킹등급 정보 추가, 듀얼토너먼트용 게임 상태 저장)
// version 6 : 장착 아이템에 갯수정보가 추가되었습니다. (소모성 아이템용)
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
	bool LoadStageSettingEtc(ZFile* file);	// 스테이지타입에 따른 추가 스테이지 데이터, ( duel모드의 순서등.. )
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

//////////////////////////////////////////////////////
// 기존 버전의 캐릭터 정보 구조체
//////////////////////////////////////////////////////

#pragma pack(push, old)
#pragma pack(1)

struct MTD_CharInfo_v0	// ver0 ver1
{
	// 캐릭터 정보
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

	// 아이템 정보
	unsigned long int	nEquipedItemDesc[12];		// MMatchCharItemParts_v0::MMCIP_END0

	// account 의 정보
	MMatchUserGradeID	nUGradeID;
};

struct MTD_CharInfo_v2	// ver2 ~ 4
{
	// 캐릭터 정보
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

	// 아이템 정보
	unsigned long int	nEquipedItemDesc[12];		// MMatchCharItemParts_v0::MMCIP_END0

	// account 의 정보
	MMatchUserGradeID	nUGradeID;

	// ClanCLID
	unsigned int		nClanCLID;
};

struct MTD_CharInfo_v5	// ver5
{
	// 캐릭터 정보
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

	// 아이템 정보
	unsigned long int	nEquipedItemDesc[12];		// MMatchCharItemParts_v0::MMCIP_END0

	// account 의 정보
	MMatchUserGradeID	nUGradeID;

	// ClanCLID
	unsigned int		nClanCLID;

	// 지난주 듀얼토너먼트 등급
	int					nDTLastWeekGrade;	
};

#pragma pack(pop, old)


#endif