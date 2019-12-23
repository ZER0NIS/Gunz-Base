//////////////////////////////////////////////////////////////////////////////////////////////
// 2009. 6. 3 - Added By Hong KiJu

// Notice By Hong KiJu - 홍기주에게 문의하세용

// Test Code입니다. 아래와 같이 호출하시면 됩니다..-.,-;
// 함수의 위치는 MMatchServer Class 내에 위치시켜놨으나,
// 추후 원하는 위치로 함수를 옮기시면 됩니다.
// 함수의 파라미터 등도 수정하셔야 될겁니다. 일단 최초의 프로토타입 뿐입니다.

#include "stdafx.h"
#include "MMatchServer.h"

#include "MAsyncDBJob_SurvivalMode.h"

bool MMatchServer::OnRequestSurvivalModeGroupRanking()
{
	MAsyncDBJob_GetSurvivalModeGroupRanking *pAsyncDbJob_GetSurvivalModeGroupRanking = new MAsyncDBJob_GetSurvivalModeGroupRanking;
	if( 0 == pAsyncDbJob_GetSurvivalModeGroupRanking )
		return false;

	pAsyncDbJob_GetSurvivalModeGroupRanking->Input();
	MMatchServer::GetInstance()->PostAsyncJob( pAsyncDbJob_GetSurvivalModeGroupRanking );

	return true;
}

bool MMatchServer::OnRequestSurvivalModePrivateRanking( const MUID& uidStage, const MUID& uidPlayer, DWORD dwScenarioID, DWORD dwCID )
{
	MAsyncDBJob_GetSurvivalModePrivateRanking *pAsyncDbJob_GetSurvivalModePrivateRanking = new MAsyncDBJob_GetSurvivalModePrivateRanking;
	if( 0 == pAsyncDbJob_GetSurvivalModePrivateRanking )
		return false;

	pAsyncDbJob_GetSurvivalModePrivateRanking->Input( uidStage, uidPlayer, dwScenarioID, dwCID );
	MMatchServer::GetInstance()->PostAsyncJob( pAsyncDbJob_GetSurvivalModePrivateRanking );

	return true;
}

// DB에 SurvivalModeGameLog를 남길 때 호출되는 함수입니다.
bool MMatchServer::OnPostSurvivalModeGameLog()
{
	char *szGameName = "'Survival Mode Test Room";
	DWORD dwScenarioID = 1;			// 추후 Define해서 쓰시면 편할 것 같습니다. 
									// 1은 맨션, 2는 프리즌, 3은 던젼으로 DB에서는 셋팅되어 있음
	DWORD dwTotalRound = 20;		// 총 진행한 라운드수입니다. 

	DWORD dwMasterPlayerCID			= 1;	// 방장의 CID입니다(1은 테스트 코드, 내부 DB에는 발렌타인)
	DWORD dwMasterPlayerRankPoint	= 1000;
	DWORD dwPlayer2CID				= 4;	// 나머지 Player의 CID입니다(4는 라온하제)
	DWORD dwPlayer2RankPoint		= 4000;
	DWORD dwPlayer3CID				= 6;	// 나머지 Player의 CID입니다(6은 둡햏)
	DWORD dwPlayer3RankPoint		= 6000;
	DWORD dwPlayer4CID				= 8;	// 나머지 Player의 CID입니다
	DWORD dwPlayer4RankPoint		= 8000;

	DWORD dwGamePlayTime =	10;		// 게임을 플레이한 시간입니다.

	MAsyncDBJob_InsertSurvivalModeGameLog *pAsyncDbJob_InsertSurvivalGameLog = new MAsyncDBJob_InsertSurvivalModeGameLog;
	if( 0 == pAsyncDbJob_InsertSurvivalGameLog )
		return false;

	pAsyncDbJob_InsertSurvivalGameLog->Input( szGameName, dwScenarioID, dwTotalRound, 
		dwMasterPlayerCID, dwMasterPlayerRankPoint, dwPlayer2CID, dwPlayer2RankPoint, 
		dwPlayer3CID, dwPlayer3RankPoint, dwPlayer4CID, dwPlayer4RankPoint, 
		dwGamePlayTime);

	MMatchServer::GetInstance()->PostAsyncJob( pAsyncDbJob_InsertSurvivalGameLog );

	return true;
}
