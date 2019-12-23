//////////////////////////////////////////////////////////////////////////////////////////////
// 2009. 6. 3 - Added By Hong KiJu

#pragma once

#ifndef _MASYNCDBJOB_INSERTSURVIVALLOG_H
#define _MASYNCDBJOB_INSERTSURVIVALLOG_H

#include "MAsyncDBJob.h"

class MAsyncDBJob_InsertSurvivalModeGameLog : public MAsyncJob 
{
protected:
	char m_szGameName[MAX_CHATROOMNAME_STRING_LEN];
	DWORD m_dwScenarioID;
	DWORD m_dwTotalRound;
    DWORD m_dwMasterPlayerCID;
	DWORD m_dwMasterPlayerRankPoint;
	DWORD m_dwPlayer2CID;
	DWORD m_dwPlayer2RankPoint;
	DWORD m_dwPlayer3CID;
	DWORD m_dwPlayer3RankPoint;
	DWORD m_dwPlayer4CID;
	DWORD m_dwPlayer4RankPoint;
	DWORD m_dwGamePlayTime;

public:
	MAsyncDBJob_InsertSurvivalModeGameLog() : MAsyncJob(MASYNCJOB_INSERT_SURVIVALMODE_GAME_LOG, MUID(0, 0)){}
	~MAsyncDBJob_InsertSurvivalModeGameLog(){}

	void Input(char *szGameName, DWORD dwScenarioID, DWORD dwTotalRound, 
				DWORD dwMasterPlayerCID, DWORD dwMasterPlayerRankPoint,
				DWORD dw2PCID, DWORD dw2PRankPoint, 
				DWORD dw3PCID, DWORD dw3PRankPoint, 
				DWORD dw4PCID, DWORD dw4PRankPoint, 
				DWORD dwGamePlayTime)
	{
		
		ZeroMemory(m_szGameName, MAX_CHATROOMNAME_STRING_LEN);
		strcpy(m_szGameName, szGameName);

		m_dwScenarioID				= dwScenarioID;
		m_dwTotalRound				= dwTotalRound;
		m_dwMasterPlayerCID			= dwMasterPlayerCID;
		m_dwMasterPlayerRankPoint	= dwMasterPlayerRankPoint;
		m_dwPlayer2CID				= dw2PCID;
		m_dwPlayer2RankPoint		= dw2PRankPoint;
		m_dwPlayer3CID				= dw3PCID;
		m_dwPlayer3RankPoint		= dw3PRankPoint;
		m_dwPlayer4CID				= dw4PCID;
		m_dwPlayer4RankPoint		= dw4PRankPoint;
		m_dwGamePlayTime			= dwGamePlayTime;
	}

	virtual void Run(void* pContext);
};



class MAsyncDBJob_GetSurvivalModeGroupRanking : public MAsyncJob 
{
protected:
	vector<RANKINGINFO*> m_RankingVec[MAX_SURVIVAL_SCENARIO_COUNT];

public:
	MAsyncDBJob_GetSurvivalModeGroupRanking() : MAsyncJob(MASYNCJOB_GET_SURVIVALMODE_GROUP_RANKING, MUID(0, 0)){}
	~MAsyncDBJob_GetSurvivalModeGroupRanking() {
		for (int s=0; s<MAX_SURVIVAL_SCENARIO_COUNT; ++s)
			for (unsigned int i=0; i<m_RankingVec[s].size(); ++i)
				delete m_RankingVec[s][i];
	}

	void Input()
	{}

	virtual void Run(void* pContext);

	RANKINGINFO* GetRankingInfo(DWORD dwScenario, DWORD index)	// 실제 시나리오ID는 1부터 시작하지만 dwScenario는 시나리오'배열'인덱스임을 주의
	{
		if( dwScenario >= MAX_SURVIVAL_SCENARIO_COUNT) return NULL;
		if (index >= m_RankingVec[dwScenario].size()) return NULL;

		return m_RankingVec[dwScenario][index];
	}
};



class MAsyncDBJob_GetSurvivalModePrivateRanking : public MAsyncJob 
{
protected:
	MUID m_uidStage;
	MUID m_uidPlayer;
	DWORD m_dwScenarioID;
	DWORD m_dwCID;
	RANKINGINFO m_RankingInfo[MAX_SURVIVAL_SCENARIO_COUNT];

public:
	MAsyncDBJob_GetSurvivalModePrivateRanking() : MAsyncJob(MASYNCJOB_GET_SURVIVALMODE_PRIVATE_RANKING, MUID(0, 0)){}
	~MAsyncDBJob_GetSurvivalModePrivateRanking(){}

	void Input(const MUID& uidStage, const MUID& uidPlayer, DWORD dwScenarioID, DWORD dwCID)
	{
		m_uidStage = uidStage;
		m_uidPlayer = uidPlayer;
		m_dwScenarioID = dwScenarioID;
		m_dwCID = dwCID;

		for(int i = 0; i < MAX_SURVIVAL_SCENARIO_COUNT; i++)
		{
			m_RankingInfo[i].dwCID = dwCID;
			m_RankingInfo[i].szCharName[0] = 0;
			m_RankingInfo[i].dwRanking = 0;
			m_RankingInfo[i].dwRankingPoint = 0;
		}
	}

	virtual void Run(void* pContext);

	MUID GetStageUID()						{ return m_uidStage; }
	MUID GetPlayerUID()						{ return m_uidPlayer; }
	DWORD GetScenarioID()					{ return m_dwScenarioID; }
	DWORD GetCID()							{ return m_dwCID; }

	RANKINGINFO* GetPrivateRankingInfo(DWORD dwScenarioID)
	{
		if( dwScenarioID > MAX_SURVIVAL_SCENARIO_COUNT)
			return NULL;

		return &m_RankingInfo[dwScenarioID - 1];
	}
};
#endif