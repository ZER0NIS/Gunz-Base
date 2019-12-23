#include "stdafx.h"

/*
#include <winsock2.h>
#include "MErrorTable.h"
#include "ZConfiguration.h"
#include "ZGameClient.h"
#include "MSharedCommandTable.h"
#include "MCommandLogFrame.h"
#include "ZIDLResource.h"
#include "MBlobArray.h"
#include "ZInterface.h"
#include "ZApplication.h"
#include "ZGameInterface.h"
#include "MMatchChannel.h"
#include "MMatchStage.h"
#include "ZPost.h"
#include "MComboBox.h"
#include "MTextArea.h"
#include "MDebug.h"
#include "ZMyInfo.h"
#include "ZNetRepository.h"
#include "ZCountDown.h"

#include "ZLanguageConf.h"
*/

void ZGameClient::OnDuelTournamentPrepare(MDUELTOURNAMENTTYPE nType, MUID uidStage, void* pBlobPlayerInfo)
{
	m_uidStage = uidStage;

	vector<DTPlayerInfo> vecDTPlayerInfo;

	int nCount = MGetBlobArrayCount(pBlobPlayerInfo);
	if(nCount != GetDTPlayerCount(nType)) { }

	for (int i=0; i<nCount; ++i)
	{
		DTPlayerInfo *pPlayerInfo = reinterpret_cast<DTPlayerInfo*>(MGetBlobArrayElement(pBlobPlayerInfo, i));
		if (!pPlayerInfo) { break; }

		DTPlayerInfo playerInfo;
		strcpy(playerInfo.m_szCharName, pPlayerInfo->m_szCharName);
		playerInfo.uidPlayer = pPlayerInfo->uidPlayer;
		playerInfo.m_nTP = pPlayerInfo->m_nTP;
		vecDTPlayerInfo.push_back(playerInfo);

		//////////////////////////////////////////////////// LOG ////////////////////////////////////////////////////
#ifdef _DUELTOURNAMENT_LOG_ENABLE_
		char szbuf[32] = {0, };
		switch(nType)
		{
		case MDUELTOURNAMENTTYPE_FINAL:			sprintf(szbuf, "TYPE_FINAL"); break;
		case MDUELTOURNAMENTTYPE_SEMIFINAL:		sprintf(szbuf, "TYPE_SEMIFINAL"); break;
		case MDUELTOURNAMENTTYPE_QUATERFINAL:	sprintf(szbuf, "TYPE_QUATERFINAL"); break;
		default:								sprintf(szbuf, "TYPE_FAIL"); break;
		}
		mlog("[MC_MATCH_DUELTOURNAMENT_PREPARE_MATCH] Type:%s, Count:%d, Player(%d:%d)%s, TP:%d \n", 
			szbuf, i, pPlayerInfo->uidPlayer.High, pPlayerInfo->uidPlayer.Low, pPlayerInfo->m_szCharName, pPlayerInfo->m_nTP);
#endif
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	}

	ZApplication::GetGameInterface()->SetDuelTournamentCharacterList((MDUELTOURNAMENTTYPE)nType, vecDTPlayerInfo);
}

void ZGameClient::OnDuelTournamentLaunch(const MUID& uidStage, const char* pszMapName)
{	
	m_uidStage = uidStage;
	strcpy(m_szStageName, "DuelTournament_Stage");

	SetAllowTunneling(false);

	m_MatchStageSetting.SetMapName(const_cast<char*>(pszMapName));

	// 암호화 키 설정  - 클랜전은 OnStageJoin대신 여기서 암호화키를 설정한다.
	unsigned int nStageNameChecksum = m_szStageName[0] + m_szStageName[1] + m_szStageName[2] + m_szStageName[3];
	InitPeerCrypt(uidStage, nStageNameChecksum);

	if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME)
	{
		ZChangeGameState(GUNZ_GAME);		// thread safely
	}
}