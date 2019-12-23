#include "stdafx.h"
#include "MMatchStageSetting.h"
#include "MMatchServer.h"
#include "MMatchObject.h"

MMatchStageSetting::MMatchStageSetting()
{
	Clear();
}


MMatchStageSetting::~MMatchStageSetting()
{
	Clear();
}

unsigned long MMatchStageSetting::GetChecksum()
{
	return (m_StageSetting.Ref().nMapIndex + m_StageSetting.Ref().nGameType + m_StageSetting.Ref().nMaxPlayers);
}

void MMatchStageSetting::SetDefault()
{
	m_StageSetting.Ref().nMapIndex = 0;
	strcpy(m_StageSetting.Ref().szMapName, MMATCH_DEFAULT_STAGESETTING_MAPNAME);
	m_StageSetting.Ref().nGameType = MMATCH_DEFAULT_STAGESETTING_GAMETYPE;
	m_StageSetting.Ref().bTeamKillEnabled = MMATCH_DEFAULT_STAGESETTING_TEAMKILL;
	m_StageSetting.Ref().bTeamWinThePoint = MMATCH_DEFAULT_STAGESETTING_TEAM_WINTHEPOINT;
	m_StageSetting.Ref().bForcedEntryEnabled = MMATCH_DEFAULT_STAGESETTING_FORCEDENTRY;
	m_StageSetting.Ref().nLimitTime = MMATCH_DEFAULT_STAGESETTING_LIMITTIME;
	m_StageSetting.Ref().nMaxPlayers = MMATCH_DEFAULT_STAGESETTING_MAXPLAYERS;
	m_StageSetting.Ref().nRoundMax = MMATCH_DEFAULT_STAGESETTING_ROUNDMAX;
	m_StageSetting.Ref().nLimitLevel = MMATCH_DEFAULT_STAGESETTING_LIMITLEVEL;
	m_StageSetting.Ref().bAutoTeamBalancing = MMATCH_DEFAULT_STAGESETTING_AUTOTEAMBALANCING;
	m_StageSetting.Ref().uidStage = MUID(0,0);
	m_StageSetting.Ref().bIsRelayMap = false;
	m_StageSetting.Ref().bIsStartRelayMap = false;
	m_StageSetting.Ref().nRelayMapListCount = 0;
	for (int i=0; i<MAX_RELAYMAP_LIST_COUNT; ++i)
		m_StageSetting.Ref().MapList[i].nMapID = -1;
	m_StageSetting.Ref().nRelayMapType = RELAY_MAP_TURN;
	m_StageSetting.Ref().nRelayMapRepeatCount = RELAY_MAP_3REPEAT;
	m_StageSetting.Ref().bAntiLead = true;
#ifdef _VOTESETTING
	m_StageSetting.Ref().bVoteEnabled = true;
	m_StageSetting.Ref().bObserverEnabled = false;
#endif

	m_StageSetting.MakeCrc();
}

void MMatchStageSetting::SetMapName(char* pszName)
{ 
	if(pszName == NULL) return;
	m_StageSetting.CheckCrc();
	if( strlen(pszName) < MAPNAME_LENGTH )
	{
		memset( m_StageSetting.Ref().szMapName, 0, MAPNAME_LENGTH );

		strcpy(m_StageSetting.Ref().szMapName, pszName); // -- by SungE 2007-04-02

		// MapIndex까지 함께 세팅해준다.
		m_StageSetting.Ref().nMapIndex = 0;
		for (int i = 0; i < MMATCH_MAP_MAX; i++)
		{
			if (!_stricmp( MGetMapDescMgr()->GetMapName(i), pszName))
			{
				m_StageSetting.Ref().nMapIndex = MGetMapDescMgr()->GetMapID(i);
				break;
			}
		}
	}
	m_StageSetting.MakeCrc();
}

void MMatchStageSetting::SetMapIndex(int nMapIndex)
{
	m_StageSetting.CheckCrc();
	m_StageSetting.Ref().nMapIndex = nMapIndex; 

	// MapName까지 함께 세팅해준다.
	if ( MGetMapDescMgr()->MIsCorrectMap(nMapIndex))
	{
		strcpy(m_StageSetting.Ref().szMapName, MGetMapDescMgr()->GetMapName(nMapIndex)); 
	}
	m_StageSetting.MakeCrc();
}

void MMatchStageSetting::Clear()
{
	SetDefault();
	m_CharSettingList.DeleteAll();
	m_uidMaster = MUID(0,0);
	m_nStageState = STAGE_STATE_STANDBY;
	m_bIsCheckTicket = false;

}

MSTAGE_CHAR_SETTING_NODE* MMatchStageSetting::FindCharSetting(const MUID& uid)
{
	for (MStageCharSettingList::iterator i=m_CharSettingList.begin();i!=m_CharSettingList.end();i++) {
		if (uid == (*i)->uidChar) return (*i);
	}
	return NULL;
}

bool MMatchStageSetting::IsTeamPlay()
{
	return MGetGameTypeMgr()->IsTeamGame(m_StageSetting.Ref().nGameType);
}

bool MMatchStageSetting::IsWaitforRoundEnd()
{
	return MGetGameTypeMgr()->IsWaitForRoundEnd(m_StageSetting.Ref().nGameType);
}

bool MMatchStageSetting::IsQuestDrived()
{
	return MGetGameTypeMgr()->IsQuestDerived(m_StageSetting.Ref().nGameType);
}

void MMatchStageSetting::UpdateStageSetting(MSTAGE_SETTING_NODE* pSetting)
{
	// move to MMatchServer::OnStageSetting(...) - by SungE 2007-05-14
	//if( STAGE_BASIC_MAX_PLAYERCOUNT < pSetting->nMaxPlayers )
	//{
	//	if( QuestTestServer() && MGetGameTypeMgr()->IsQuestDerived(pSetting->nGameType) )
	//	{
	//		pSetting->nMaxPlayers = STAGE_QUEST_MAX_PLAYER;
	//	}
	//	else
	//	{
	//		pSetting->nMaxPlayers = STAGE_BASIC_MAX_PLAYERCOUNT;
	//	}
	//}

	m_StageSetting.Set_CheckCrc(*pSetting);
	/*m_StageSetting.CheckCrc();
	memcpy(&m_StageSetting.Ref(), pSetting, sizeof(MSTAGE_SETTING_NODE));
	m_StageSetting.MakeCrc();*/
}


void MMatchStageSetting::UpdateCharSetting(const MUID& uid, unsigned int nTeam, MMatchObjectStageState nStageState)
{
	MSTAGE_CHAR_SETTING_NODE* pNode = FindCharSetting(uid);
	if (pNode) {
		pNode->nTeam = nTeam;
		pNode->nState = nStageState;
	} else {
		MSTAGE_CHAR_SETTING_NODE* pNew = new MSTAGE_CHAR_SETTING_NODE;
		pNew->uidChar = uid;
		pNew->nTeam = nTeam;
		pNew->nState = nStageState;
		m_CharSettingList.push_back(pNew);
	}			
}



const MMatchGameTypeInfo* MMatchStageSetting::GetCurrGameTypeInfo()
{ 
	return MGetGameTypeMgr()->GetInfo(m_StageSetting.Ref().nGameType); 
}

void MMatchStageSetting::SetRelayMapList(RelayMap* pValue)
{ 
	m_StageSetting.CheckCrc();
	memcpy(m_StageSetting.Ref().MapList, pValue, sizeof(m_StageSetting.Ref().MapList));
	m_StageSetting.MakeCrc();
}
