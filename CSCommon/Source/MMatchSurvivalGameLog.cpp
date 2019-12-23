#include "stdafx.h"
#include "MMatchSurvivalGameLog.h"
#include "MAsyncDBJob_SurvivalMode.h"

MMatchSurvivalGameLogInfoManager::MMatchSurvivalGameLogInfoManager()
{
	Clear();
}

MMatchSurvivalGameLogInfoManager::~MMatchSurvivalGameLogInfoManager()
{

}

void MMatchSurvivalGameLogInfoManager::Clear()
{
	strcpy(m_szGameName, "");

	m_dwScenarioID = 0;
	m_dwTotalRound = 0;

	m_dwMasterCID = 0;
	m_vecRankInfo.clear();

	m_dwGainXP = 0;
	m_dwGainBP = 0;
	m_dwGainRankingPoint = 0;
	m_dwGameStartTime= 0;
	m_dwGameEndTime = 0;
}

void MMatchSurvivalGameLogInfoManager::SetStageName( const char* sz )
{
	if( (0 == sz) || (MAX_CHATROOMNAME_STRING_LEN < strlen(sz)) )
		return;

	strcpy( m_szGameName, sz );
}

bool MMatchSurvivalGameLogInfoManager::PostInsertSurvivalGameLog()
{
	const int nElapsedPlayTime = (m_dwGameEndTime - m_dwGameStartTime) / 60000; // 분단위로 계산을 함.

	MAsyncDBJob_InsertSurvivalModeGameLog* pAsyncDbJob_InsertGameLog = new MAsyncDBJob_InsertSurvivalModeGameLog;
	if( 0 == pAsyncDbJob_InsertGameLog )
		return false;

	//_ASSERT(!m_vecRankInfo.empty() && m_vecRankInfo.size()<=4);

	// 마스터와 나머지 3명의 플레이어 랭킹정보를 추린다
	vector<RankInfo>::iterator it = m_vecRankInfo.begin();
	vector<RankInfo>::iterator itEnd = m_vecRankInfo.end();
	RankInfo masterRankinfo, rankinfo[3];
	for(int i=0;it != itEnd;it++)
	{
		RankInfo ri = (*it);
		if ( ri.dPlayerCID == m_dwMasterCID ) 
		{
			masterRankinfo.dPlayerCID = ri.dPlayerCID;
			masterRankinfo.dGainRankingPoint = ri.dGainRankingPoint;
			continue;
		}
		rankinfo[i++] = ri;
	}

	pAsyncDbJob_InsertGameLog->Input( m_szGameName, 
		m_dwScenarioID, m_dwTotalRound,  
		m_dwMasterCID, masterRankinfo.dGainRankingPoint,
		rankinfo[0].dPlayerCID, rankinfo[0].dGainRankingPoint, 
		rankinfo[1].dPlayerCID, rankinfo[1].dGainRankingPoint, 
		rankinfo[2].dPlayerCID, rankinfo[2].dGainRankingPoint, 
		nElapsedPlayTime );

	MMatchServer::GetInstance()->PostAsyncJob( pAsyncDbJob_InsertGameLog );

	return true;
}
void MMatchSurvivalGameLogInfoManager::AddPlayer(DWORD cid)
{
	RankInfo rankInfo;
	rankInfo.dPlayerCID = cid;
	m_vecRankInfo.push_back(rankInfo); 
}
void MMatchSurvivalGameLogInfoManager::SetPlayerRankPoint(DWORD cid, DWORD RP)
{
	vector<RankInfo>::iterator it = m_vecRankInfo.begin();
	vector<RankInfo>::iterator itEnd = m_vecRankInfo.end();
	RankInfo rankinfo;
	for(int i=0;it != itEnd;it++, i++)
	{
		if( i >= 4 )
		{// 서바이벌 최대 인원
			ASSERT(0);
			break;
		}
		rankinfo = (*it);
		if(rankinfo.dPlayerCID == cid)
		{
			rankinfo.dGainRankingPoint = RP;
			(*it) = rankinfo;

#ifdef _DEBUG
			char sz[128];
			sprintf(sz, "N rankinfo.dPlayerCID[%d], rankinfo.dGainRankingPoint[%d]\n", rankinfo.dPlayerCID, rankinfo.dGainRankingPoint);
			OutputDebugString(sz);
#endif
		}

	}
}