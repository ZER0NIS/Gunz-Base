#ifndef _MMATCHRULE_DUELTOURNAMENT_H
#define _MMATCHRULE_DUELTOURNAMENT_H


#include "MMatchRule.h"
#include "MDuelTournamentFormula.h"
#include <list>

#define DUELTOURNAMENT_PLAYER_UNEARNEDPOINT			0
#define DUELTOURNAMENT_PLAYER_LEAVE_PENALTYPOINT	10

using namespace std;

struct MDuelTournamentGameInfo
{
	MDUELTOURNAMENTTYPE		nType;
	int						nGameNumber;
	char					szTimeStamp[DUELTOURNAMENT_TIMESTAMP_MAX_LENGTH + 1];
};

struct MDuelTournamentMatchRecord
{
	MUID uidWinner, uidLoser;
	int nCID1, nCID2;
	int nTP1, nTP2;
	int nGainTP, nLoseTP;
	int nRoundCount;
	int nPlayer1WinCount, nPlayer2WinCount;
	bool bIsPlayer1Win, bIsPlayer2Win;

	bool bMatchFinish;

	DWORD nStartTime;
	DWORD nFinishTime;
};

struct MDuelTournamentRoundRecord
{
	MUID uidWinnerPlayer;
	MUID uidLoserPlayer;

	int fPlayer1DamagedPoint, fPlayer1HealthPoint, fPlayer1AmorPoint;
	int fPlayer2DamagedPoint, fPlayer2HealthPoint, fPlayer2AmorPoint;

	bool bPlayer1Win;
	bool bPlayer2Win;

	bool bIsTimeOut;

	DWORD nFinishTime;
};

class MDuelTournamentPlayerInfo
{	
public:
	MUID uidPlayer;
	int  nCID, nChangeTP;
	bool bLoser, bOutGame, bInMatch;
};

class MDuelTournamentPlayerMap : public map <MUID, MDuelTournamentPlayerInfo*>
{
public:
	~MDuelTournamentPlayerMap() {
		for(MDuelTournamentPlayerMap::iterator i=begin() ; i != end(); ++i){delete (*i).second;}
		clear();
	}

	MDuelTournamentPlayerInfo* GetPlayerInfo(MUID uidPlayer){
		if( find(uidPlayer) != end() ) return find(uidPlayer)->second;
		return NULL;
	}

	void InsertPlayerInfo(MUID uidPlayer, MDuelTournamentPlayerInfo *pInfo) {
		insert(pair<MUID, MDuelTournamentPlayerInfo*>(uidPlayer, pInfo));
	}

	bool EnterMatch(MUID uidPlayer){ 
		MDuelTournamentPlayerInfo* pInfo = GetPlayerInfo(uidPlayer);
		if( pInfo != NULL ){ pInfo->bInMatch = true; return true;}
		return false;
	}
	
	void LeaveMatch(MUID uidPlayer){
		MDuelTournamentPlayerInfo* pInfo = GetPlayerInfo(uidPlayer);
		if( pInfo != NULL ) pInfo->bInMatch = false;
	}

	void SetLoser(MUID uidPlayer){
		MDuelTournamentPlayerInfo* pInfo = GetPlayerInfo(uidPlayer);
		if( pInfo != NULL ) pInfo->bLoser = true;
	}

	bool IsLoser(MUID uidPlayer){
		MDuelTournamentPlayerInfo* pInfo = GetPlayerInfo(uidPlayer);
		if( pInfo != NULL ) return pInfo->bLoser;
		return false;
	}

	void SetOutStage(MUID uidPlayer){
		MDuelTournamentPlayerInfo* pInfo = GetPlayerInfo(uidPlayer);
		if( pInfo != NULL ) pInfo->bOutGame = true;
	}

	bool IsOutUser(MUID uidPlayer) {
		MDuelTournamentPlayerInfo* pInfo = GetPlayerInfo(uidPlayer);
		if( pInfo == NULL ) return true;
		return pInfo->bOutGame;
	}
};

class MMatchRuleDuelTournament : public MMatchRule {
private:
	MDuelTournamentGameInfo					m_GameInfo;
	MMatchDuelTournamentMatch				m_CurrentMatchInfo;	
	map<int, MMatchDuelTournamentMatch*>*	m_DuelTournamentMatchMap;

	MDuelTournamentPlayerMap m_DTPlayerMap;

	MDuelTournamentMatchRecord m_MatchRecord;
	MDuelTournamentRoundRecord m_RoundRecord;
	
	MDuelTournamentFormula m_DuelTournamentCalculator;

	bool					m_bIsRoundFinish;				///< 라운드 끝나는가
	bool					m_bIsRoundTimeOut;
	bool					m_bIsCorrectFinish;	
	
	enum DTLogLevel { DTLOG_DEBUG = 1, DTLOG_RELEASE = 2,};

protected:	
	virtual void OnBegin();											///< 전체 게임 시작시 호출
	virtual void OnEnd();											///< 전체 게임 종료시 호출
	
	virtual void OnRoundBegin();									///< 라운드 시작할 때 호출
	virtual void OnRoundEnd();										///< 라운드 끝날 때 호출
	bool OnRoundEnd_PlayerOut();
	void OnRoundEnd_TimeOut();

	virtual bool OnRun();

	virtual void OnLeaveBattle(MUID& uidChar);									///< 게임중 나갔을때 호출된다.
	virtual void OnGameKill(const MUID& uidAttacker, const MUID& uidVictim);	///< 킬했을때 도전자의 킬인지 챔피언의 킬인지 체크	

	virtual bool RoundCount(){
		if( m_MatchRecord.bMatchFinish == true && m_CurrentMatchInfo.nNextMatchNumber == 0 ) return false;
		return true;
	}

	virtual bool OnCheckEnableBattleCondition(){
		MMatchObject* pObj;

		if (m_pStage == NULL) {
			SetRoundFinish(true);
			return false;
		}

		if (m_CurrentMatchInfo.uidPlayer1 == MUID(0, 0) || m_CurrentMatchInfo.uidPlayer2 == MUID(0, 0)) {
			SetRoundFinish(true);
		} else if( (pObj = m_pStage->GetObj(m_CurrentMatchInfo.uidPlayer1)) == NULL ) {
			OnLeaveBattle(m_CurrentMatchInfo.uidPlayer1);
		} else if( (pObj = m_pStage->GetObj(m_CurrentMatchInfo.uidPlayer2)) == NULL ) {
			OnLeaveBattle(m_CurrentMatchInfo.uidPlayer2);
		}

		return true;
	}

	virtual bool OnCheckRoundFinish() { return IsRoundFinish();}
	virtual void OnCommand(MCommand* pCommand);

	void OnPreCountDown();

	void SetDTRoundState(MMATCH_ROUNDSTATE nState);

	void InitMatchRecord();
	void InitRoundRecord();

	void SpawnPlayers();										///< 플레이어들을 스폰시킨다.	
	void PrepareDuelTournamentMatchRecord();	
	
	void OnMatchFinish(bool bIsPlayer1Win);
	void OnMatchNotFinish();

	void MakeNextMatch();
	void RecordGameResult();

	void UpdateDuelTournamentPlayerInfo(MUID uidPlayer, MDUELTOURNAMENTROUNDSTATE nState, bool bIsWinner, int nChangeTP, bool bIsLeaveUser = false);
	void InsertDuelTournamentGameLogDeatil(MDUELTOURNAMENTROUNDSTATE nDTRoundState, int nWinnerCID, int nLoserCID, int nGainTP, int nLoseTP, int nPlayTime);

	void SendDuelTournamentGameInfo(bool bIsRoundEnd = false);
	void SendDuelTournamentNextGamePlayerInfo();
	void SendDuelTournamentRoundResultInfo(bool bIsMatchFinish, bool bIsRoundDraw);
	void SendDuelTournamentMatchResultInfo();
	
	bool IsRoundFinish()							{ return m_bIsRoundFinish; }
	void SetRoundFinish(bool bValue)				{ m_bIsRoundFinish = bValue; }

	bool IsRoundTimeOut()							{ return m_bIsRoundTimeOut; }
	void SetRoundTimeOut(bool bValue)				{ m_bIsRoundTimeOut = bValue; }

	bool IsCorrectFinish()							{ return m_bIsCorrectFinish; }
	void SetCorrectFinish(bool bValue)				{ m_bIsCorrectFinish = bValue; }
	
	void DTLog(DTLogLevel nLevel, const char *pFormat,...){
		if( nLevel == DTLOG_DEBUG ) {
#ifdef _DUELTOURNAMENT_LOG_ENABLE_	
			va_list args;
			static char temp[1024];

			va_start(args, pFormat);
			vsprintf(temp, pFormat, args);

			MMatchServer::GetInstance()->LOG(MMatchServer::LOG_PROG, "DT_LOG(%d) - %s", m_GameInfo.nGameNumber, temp);
#endif
		} else if( nLevel == DTLOG_RELEASE ) {
			va_list args;
			static char temp[1024];

			va_start(args, pFormat);
			vsprintf(temp, pFormat, args);

			MMatchServer::GetInstance()->LOG(MMatchServer::LOG_PROG, "DT_LOG(%d) - %s", m_GameInfo.nGameNumber, temp);
		}
	}

	void LogInfo() {
#ifdef _DUELTOURNAMENT_LOG_ENABLE_
		if (m_pStage == NULL) return;	

		MMatchObject* pObj1, *pObj2;
		pObj1 = m_pStage->GetObj(m_CurrentMatchInfo.uidPlayer1);
		pObj2 = m_pStage->GetObj(m_CurrentMatchInfo.uidPlayer2);

		int nIndex = 0;
		char szTemp[1024] = {0, };	
		for (MDuelTournamentPlayerMap::iterator i = m_DTPlayerMap.begin(); i!= m_DTPlayerMap.end();  i++)
		{
			MDuelTournamentPlayerInfo* pInfo = (*i).second;
			if( pInfo->bOutGame == false && pInfo->bInMatch == false ) {
				MMatchObject* pObj = m_pStage->GetObj((*i).first);

				if( pObj != NULL ){
					nIndex += sprintf(&szTemp[nIndex], "Player(%d%d) - %s, ", pObj->GetUID().High, pObj->GetUID().Low, pObj->GetName());
				}
			}			
		}

		DTLog(DTLOG_DEBUG, "┏━Logging DuelTournament Info━━━━━━━━━━━━━━━━━━━━━━━━");
		DTLog(DTLOG_DEBUG, "┃MatchNumber : %d, RoundCount : %d", m_CurrentMatchInfo.nMatchNumber, m_MatchRecord.nRoundCount);

		if (pObj1 != NULL) DTLog(DTLOG_DEBUG, "┃Player1 name : %s, WinCount = %d", pObj1->GetName(), m_MatchRecord.nPlayer1WinCount);
		if (pObj2 != NULL) DTLog(DTLOG_DEBUG, "┃Player2 name : %s, WinCount = %d", pObj2->GetName(), m_MatchRecord.nPlayer2WinCount);

		DTLog(DTLOG_DEBUG, "┃Wait Queue Info : %s", szTemp);
		DTLog(DTLOG_DEBUG, "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
#endif
	}

	
public:
	MMatchRuleDuelTournament(MMatchStage* pStage);
	virtual ~MMatchRuleDuelTournament(){}

	virtual MMATCH_GAMETYPE GetGameType() { return MMATCH_GAMETYPE_DUELTOURNAMENT; }
};


#endif