#ifndef MMATCHSURVIVALGAMELOG_H
#define MMATCHSURVIVALGAMELOG_H


struct RankInfo
{
	DWORD dPlayerCID;
	DWORD dGainRankingPoint;
	RankInfo()
	{
		dPlayerCID = 0;
		dGainRankingPoint = 0;
	}
};

class MMatchSurvivalGameLogInfoManager
{
	char m_szGameName[MAX_CHATROOMNAME_STRING_LEN];

	DWORD m_dwScenarioID;
	DWORD m_dwTotalRound;
	
	DWORD m_dwMasterCID;
	typedef vector<RankInfo>			VECTORRANKINFO;
	VECTORRANKINFO m_vecRankInfo;
	
	DWORD m_dwGainXP;
	DWORD m_dwGainBP;
	DWORD m_dwGainRankingPoint;
	
	DWORD m_dwGameStartTime;
	DWORD m_dwGameEndTime;

public:
	MMatchSurvivalGameLogInfoManager();
	~MMatchSurvivalGameLogInfoManager();

	void Clear();

	void SetStageName(const char* sz);
	void SetScenarioID(DWORD n)					{ m_dwScenarioID = n; }
	void SetReachedRound(DWORD n)				{ m_dwTotalRound = n; }

	void SetMasterCID(DWORD cid)				{ m_dwMasterCID = cid; }
	void SetPlayerRankPoint(DWORD cid, DWORD RP);
	void AddPlayer(DWORD cid);

	void AccumulateXP(DWORD n)					{ m_dwGainXP += n; }
	void AccumulateBP(DWORD n)					{ m_dwGainBP += n; }
	void SetRankingPoint(DWORD n)				{ m_dwGainRankingPoint = n; }

	void SetStartTime(DWORD n)					{ m_dwGameStartTime = n; }
	void SetEndTime(DWORD n)					{ m_dwGameEndTime = n; }

	// 모아진 로그 정보를 DB에 전송
	bool PostInsertSurvivalGameLog();
};

#endif