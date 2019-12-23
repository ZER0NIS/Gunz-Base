//////////////////////////////////////////////////////////////////////////////////////////////
// Added By Hong KiJu
#pragma once

#ifndef _MASYNCDBJOB_DUELTOURNAMENT
#define _MASYNCDBJOB_DUELTOURNAMENT

#include "MAsyncDBJob.h"


class MAsyncDBJob_GetDuelTournamentTimeStamp : public MAsyncJob
{
protected:
	char m_szTimeStamp[DUELTOURNAMENT_TIMESTAMP_MAX_LENGTH + 1];

public:
	MAsyncDBJob_GetDuelTournamentTimeStamp(void) : MAsyncJob(MASYNCJOB_GET_DUELTOURNAMENT_TIMESTAMP, MUID(0, 0)) {}
	~MAsyncDBJob_GetDuelTournamentTimeStamp(void){}


	void Input() {}
	virtual void Run(void* pContext);
	char* GetTimeStamp(){ return m_szTimeStamp; }
};

class MAsyncDBJob_GetDuelTournamentCharInfo : public MAsyncJob
{
protected:
	DWORD m_dwPlayerCID;

	MUID m_uidPlayer;
	MMatchObjectDuelTournamentCharInfo m_pDTCharInfo;
public:
	MAsyncDBJob_GetDuelTournamentCharInfo(void) : MAsyncJob(MASYNCJOB_GET_DUELTOURNAMENT_CHARINFO, MUID(0, 0)) {
		m_dwPlayerCID = 0;
		m_uidPlayer = MUID(0, 0);
	}
	~MAsyncDBJob_GetDuelTournamentCharInfo(void){}

	void Input(MUID uidPlayer, DWORD dwPlayerCID) {
		m_uidPlayer = uidPlayer;
		m_dwPlayerCID = dwPlayerCID;		
	}

	virtual void Run(void* pContext);

	MUID GetPlayerUID()									{ return m_uidPlayer; }
	MMatchObjectDuelTournamentCharInfo* GetDTCharInfo()	{ return &m_pDTCharInfo; }	
};

class MAsyncDBJob_GetDuelTournamentPreviousCharInfo : public MAsyncJob
{
protected:
	MUID m_uidPlayer;
	DWORD m_dwPlayerCID;	

	int m_nPrevTP;
	int m_nPrevWins;
	int m_nPrevLoses;
	int m_nPrevRanking;
	int m_nPrevFinalWins;
public:
	MAsyncDBJob_GetDuelTournamentPreviousCharInfo(void) : MAsyncJob(MASYNCJOB_GET_DUELTOURNAMENT_PREVIOUS_CHARINFO, MUID(0, 0)) {
		m_dwPlayerCID = 0;
		m_uidPlayer = MUID(0, 0);

		m_nPrevTP = 0;
		m_nPrevWins = 0;
		m_nPrevLoses = 0;
		m_nPrevRanking = 0;
		m_nPrevFinalWins = 0;
	}
	~MAsyncDBJob_GetDuelTournamentPreviousCharInfo(void){}


	void Input(MUID uidPlayer, DWORD dwPlayerCID) {
		m_uidPlayer = uidPlayer;
		m_dwPlayerCID = dwPlayerCID;		
	}

	virtual void Run(void* pContext);
	MUID GetPlayerUID()					{ return m_uidPlayer; }

	int GetPrevTP()						{ return m_nPrevTP; }
	int GetPrevWins()					{ return m_nPrevWins; }
	int GetPrevLoses()					{ return m_nPrevLoses; }
	int GetPrevRanking()				{ return m_nPrevRanking; }
	int GetPrevFinalWins()				{ return m_nPrevFinalWins; }


};

class MAsyncDBJob_UpdateDuelTournamentCharInfo : public MAsyncJob
{
protected:
	DWORD m_dwPlayerCID;
	char m_szTimeStamp[DUELTOURNAMENT_TIMESTAMP_MAX_LENGTH + 1];
	MMatchObjectDuelTournamentCharInfo *m_pDTCharInfo;

public:
	MAsyncDBJob_UpdateDuelTournamentCharInfo(void) : MAsyncJob(MASYNCJOB_UPDATE_DUELTOURNAMENT_CHARINFO, MUID(0, 0)) {
		m_dwPlayerCID = 0;
		m_pDTCharInfo = NULL;
		ZeroMemory(m_szTimeStamp, DUELTOURNAMENT_TIMESTAMP_MAX_LENGTH + 1);
	}
	~MAsyncDBJob_UpdateDuelTournamentCharInfo(void){
		if( m_pDTCharInfo != NULL ) {
			delete m_pDTCharInfo;
		}
	}

	void Input(DWORD dwPlayerCID, char* szTimeStamp, MMatchObjectDuelTournamentCharInfo *pDTCharInfo) {
		m_dwPlayerCID = dwPlayerCID;
		m_pDTCharInfo = pDTCharInfo;

		strcpy(m_szTimeStamp, szTimeStamp);
		m_pDTCharInfo = new MMatchObjectDuelTournamentCharInfo(pDTCharInfo);
	}

	virtual void Run(void* pContext);
};


class MAsyncDBJob_UpdateDuelTournamentGameLog : public MAsyncJob
{
protected:
	int m_nLogID;
	int m_nChampionCID;
	char m_szTimeStamp[DUELTOURNAMENT_TIMESTAMP_MAX_LENGTH + 1];

public:
	MAsyncDBJob_UpdateDuelTournamentGameLog(void) : MAsyncJob(MASYNCJOB_UPDATE_DUELTOURNAMENT_GAMELOG, MUID(0, 0)) {
		memset(m_szTimeStamp, 0, DUELTOURNAMENT_TIMESTAMP_MAX_LENGTH + 1);
	}
	~MAsyncDBJob_UpdateDuelTournamentGameLog(void){}


	void Input(char* szTimeStamp, int nLogID, int nChampionCID) {
		m_nLogID = nLogID;
		m_nChampionCID = nChampionCID;
		strcpy(m_szTimeStamp, szTimeStamp);
	}

	virtual void Run(void* pContext);
};


class MAsyncDBJob_InsertDuelTournamentGameLogDetail : public MAsyncJob
{
protected:
	int m_nLogID;
	char m_szTimeStamp[DUELTOURNAMENT_TIMESTAMP_MAX_LENGTH + 1];

	MDUELTOURNAMENTROUNDSTATE m_nDTRoundState;
	int m_nPlayTime;
	
	int m_nWinnerCID;
	int m_nLoserCID;
	int m_nGainTP;
	int m_nLoseTP;

public:
	MAsyncDBJob_InsertDuelTournamentGameLogDetail(void) : MAsyncJob(MASYNCJOB_INSERT_DUELTOURNAMENT_GAMELOGDETAIL, MUID(0, 0)) {
		memset(m_szTimeStamp, 0, DUELTOURNAMENT_TIMESTAMP_MAX_LENGTH + 1);
	}
	~MAsyncDBJob_InsertDuelTournamentGameLogDetail(void){}

	void Input(int nLogID, char* szTimeStamp, MDUELTOURNAMENTROUNDSTATE nDTRoundState, int nWinnerCID, int nLoserCID, int nGainTP, int nLoseTp, int nPlayTime) {
		m_nLogID = nLogID;
		strcpy(m_szTimeStamp, szTimeStamp);

		m_nDTRoundState = nDTRoundState;
		m_nPlayTime = nPlayTime;

		m_nWinnerCID = nWinnerCID;
		m_nLoserCID = nLoserCID;
		m_nGainTP = nGainTP;
		m_nLoseTP = nLoseTp;
	}

	virtual void Run(void* pContext);
};

class MAsyncDBJob_GetDuelTournamentSideRankingInfo : public MAsyncJob
{
protected:
	DWORD m_dwPlayerCID;

	MUID m_uidPlayer;
	list<DTRankingInfo*> m_SideRankingList;
public:
	MAsyncDBJob_GetDuelTournamentSideRankingInfo(void) : MAsyncJob(MASYNCJOB_GET_DUELTOURNAMENT_SIDERANKING, MUID(0, 0)) {
		m_dwPlayerCID = 0;
		m_uidPlayer = MUID(0, 0);
	}

	~MAsyncDBJob_GetDuelTournamentSideRankingInfo(void)
	{
		RemoveListAll();
	}


	void Input(MUID uidPlayer, DWORD dwPlayerCID) {
		m_uidPlayer = uidPlayer;
		m_dwPlayerCID = dwPlayerCID;		
	}

	virtual void Run(void* pContext);

	MUID GetPlayerUID()							{ return m_uidPlayer; }
	list<DTRankingInfo*>* GetSideRankingList()	{ return &m_SideRankingList; }	

	void RemoveListAll()
	{
		list<DTRankingInfo*>::iterator iter = m_SideRankingList.begin();
		for(; iter != m_SideRankingList.end(); ){
			DTRankingInfo *pInfo = (*iter);
			delete pInfo;

			iter = m_SideRankingList.erase(iter);			
		}
	}
};


class MAsyncDBJob_GetDuelTournamentGroupRankingInfo : public MAsyncJob
{
protected:
	list<DTRankingInfo*> m_GroupRankingList;

public:
	MAsyncDBJob_GetDuelTournamentGroupRankingInfo(void) : MAsyncJob(MASYNCJOB_GET_DUELTOURNAMENT_GROUPRANKING, MUID(0, 0)) 
	{}

	~MAsyncDBJob_GetDuelTournamentGroupRankingInfo(void)
	{
		RemoveListAll();
	}


	void Input() {}

	virtual void Run(void* pContext);

	list<DTRankingInfo*>* GetGroupRankingList()	{ return &m_GroupRankingList; }	

	void RemoveListAll()
	{
		list<DTRankingInfo*>::iterator iter = m_GroupRankingList.begin();
		for(; iter != m_GroupRankingList.end(); ){
			DTRankingInfo *pInfo = (*iter);
			iter = m_GroupRankingList.erase(iter);

			delete pInfo;
		}
	}
};
#endif