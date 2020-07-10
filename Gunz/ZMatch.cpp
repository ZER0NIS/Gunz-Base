#include "stdafx.h"

#include "ZMatch.h"
#include "ZGame.h"
#include <windows.h>
#include "ZCharacterManager.h"
#include "ZCharacter.h"
#include "ZPost.h"
#include "ZGameInterface.h"
#include "ZApplication.h"
#include "ZGameClient.h"
#include "ZScreenEffectManager.h"
#include "ZActionDef.h"
#include "ZWorldItem.h"
#include "ZApplication.h"
#include "ZRule.h"
#include "ZMapDesc.h"
#include "MMatchGlobal.h"
#include "ZRuleDuel.h"
#include "ZRuleDuelTournament.h"
#include "ZInput.h"

#define READY_COUNT    5

ZMatch::ZMatch() : m_pRule(NULL)
{
    m_pStageSetting = ZGetGameClient()->GetMatchStageSetting();
    m_nRoundState = MMATCH_ROUNDSTATE_PREPARE;
    m_nLastDeadTime = 0;
    m_nNowTime = 0;
    m_nSoloSpawnTime = -1;
    memset(m_nTeamScore, 0, sizeof(m_nTeamScore));
    memset(m_nTeamKillCount, 0, sizeof(m_nTeamKillCount));
}

ZMatch::~ZMatch()
{

}

bool ZMatch::Create()
{
    m_nNowTime = 0;
    m_nCurrRound = 0;
    memset(m_nTeamScore, 0, sizeof(m_nTeamScore));
    memset(m_nTeamKillCount, 0, sizeof(m_nTeamKillCount));

    m_pRule = ZRule::CreateRule(this, GetMatchType());

    return true;
}

void ZMatch::Destroy()
{
    if (m_pRule)
    {
        delete m_pRule; m_pRule = NULL;
    }

}

void ZMatch::SetRound(int nCurrRound)
{
    m_nCurrRound = nCurrRound;
}

void ZMatch::Update(float fDelta)
{
    m_nNowTime = timeGetTime();

    switch (GetRoundState())
    {
    case MMATCH_ROUNDSTATE_PREPARE:
        {

        }
        break;
    case MMATCH_ROUNDSTATE_PRE_COUNTDOWN:
        {
        }
        break;
    case MMATCH_ROUNDSTATE_COUNTDOWN:
        {
            if ((m_nNowTime - m_nStartTime) > ((READY_COUNT+1) * 1000))
            {
                m_nStartTime = m_nNowTime;
            }
        }
        break;
    case MMATCH_ROUNDSTATE_PLAY:
        {
            ProcessRespawn();
        }
        break;
    }

    if (m_pRule) m_pRule->Update(fDelta);
}


void ZMatch::ProcessRespawn()
{
#ifdef _QUEST
    if (ZGetGameTypeManager()->IsQuestDerived(GetMatchType())) return;
#endif
 
    if (!IsWaitForRoundEnd() && ZGetGame()->m_pMyCharacter)
    {
        static bool bLastDead = false;
        if (ZGetGame()->m_pMyCharacter->IsDie())
        {
            if (bLastDead == false) {
                m_nLastDeadTime = m_nNowTime;  
            }


            int nDelayAfterDying = RESPAWN_DELAYTIME_AFTER_DYING;

            m_nSoloSpawnTime = m_nNowTime - m_nLastDeadTime;

            if (m_nSoloSpawnTime >= RESPAWN_DELAYTIME_AFTER_DYING_MAX)    // ??¾?¼­ ¾?¹«°?µµ ¾?´©¸£°? ???¸¸? 15???? ¸®½÷??
            {
                SoloSpawn();
            }
            else if (m_nSoloSpawnTime >= nDelayAfterDying)
            {
                static bool st_bCapturedActionKey = false;
                bool bNow = ZIsActionKeyPressed(ZACTION_USE_WEAPON) || ZIsActionKeyPressed(ZACTION_JUMP);

                if ((st_bCapturedActionKey == true) && (bNow == false)) {
                    SoloSpawn();
                }

                st_bCapturedActionKey = bNow;
            }

            


        }

        bLastDead = ZGetGame()->m_pMyCharacter->IsDie();
    }

}

int ZMatch::GetRoundReadyCount(void)
{
    return ( READY_COUNT - (timeGetTime() - m_nStartTime) / 1000 );
}


void ZMatch::OnDrawGameMessage()
{


#define CENTERMESSAGE    "CenterMessage"

    switch (GetRoundState())
    {
    case MMATCH_ROUNDSTATE_PREPARE:
        {
        }
        break;
    case MMATCH_ROUNDSTATE_COUNTDOWN:
        {

        }
        break;
    case MMATCH_ROUNDSTATE_PLAY:
        {

        }
        break;
    case MMATCH_ROUNDSTATE_FINISH:
        {

        }
        break;
    default:
        {

        }
        break;
    }

}



void ZMatch::SoloSpawn()
{
	auto MyTeam =ZGetGame()->m_pMyCharacter->GetTeamID();
	if (GetMatchType() == MMATCH_GAMETYPE_DUEL || MyTeam == MMT_SPECTATOR) return;
    if (GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT) return;

    if (ZGetGame()->m_pMyCharacter->IsDie())
    {
        rvector pos = rvector(0.0f, 0.0f, 0.0f);
        rvector dir = rvector(0.0f, 1.0f, 0.0f);

        ZMapSpawnData* pSpawnData = NULL;
        
        if (IsTeamPlay())
        {
            int nSpawnIndex[2] = { 0, 0 };
            for (int i = 0; i < 2; i++)
                if (ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_RED + i)
                    pSpawnData = ZGetGame()->GetMapDesc()->GetSpawnManager()->GetTeamData(i, nSpawnIndex[i]);
        }
        else
            pSpawnData = ZGetGame()->GetMapDesc()->GetSpawnManager()->GetSoloRandomData();
        

        if (pSpawnData == NULL)
        {

        }
        else
        {
                if (ZGetGame()->GetSpawnRequested() == false)
				{
                    ZPostRequestSpawn(ZGetMyUID(), pSpawnData->m_Pos, pSpawnData->m_Dir);
                    ZGetGame()->SetSpawnRequested(true);
                }   
        }
        
        m_nSoloSpawnTime = -1;
    }
}



void ZMatch::InitCharactersPosition()
{
    // ?????? °?¿?
    if (IsTeamPlay())
    {
        int nSpawnIndex[ 2] = { 0, 0 };

        for (ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.begin();
            itor != ZGetGame()->m_CharacterManager.end(); ++itor)
        {
            ZCharacter* pCharacter = (*itor).second;
            for (int i = 0; i < 2; i++)
            {
                if (pCharacter->GetTeamID() == MMT_RED + i)
                {
                    ZMapSpawnData* pSpawnData = ZGetGame()->GetMapDesc()->GetSpawnManager()->GetTeamData(i, nSpawnIndex[i]);
                    if (pSpawnData != NULL)
                    {
                        pCharacter->SetPosition(pSpawnData->m_Pos);
                        pCharacter->SetDirection(pSpawnData->m_Dir);

                        nSpawnIndex[i]++;
                    }
                }
            }
        }

        return;
    }

    // µ?¾?¸?µ? ?? °?¿?
    else if ( ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
    {
        ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
        if ( pDuel)
        {
            int nIndex = 2;
            if ( pDuel->QInfo.m_uidChampion == ZGetMyUID())
                nIndex = 0;
            else if ( pDuel->QInfo.m_uidChallenger == ZGetMyUID())
                nIndex = 1;

            // µ?¾? ??¿? ¸???¸? ¸? ?³?½°? µ?¹??° ?§?¡
            if ( MGetMapDescMgr()->IsMapOnlyDuel( ZGetGameClient()->GetMatchStageSetting()->GetMapIndex()))
            {
                ZMapSpawnData* pSpawnData = ZGetGame()->GetMapDesc()->GetSpawnManager()->GetData( nIndex);
                if ( pSpawnData != NULL)
                {
                    ZGetGame()->m_pMyCharacter->SetPosition( pSpawnData->m_Pos);
                    ZGetGame()->m_pMyCharacter->SetDirection( pSpawnData->m_Dir);
                }
            }

            // µ?¾? ??¿? ¸??? ¾?´?¸? ????°? °°?÷ ¹?½?
            else
            {
                ZMapSpawnData* pSpawnData = ZGetGame()->GetMapDesc()->GetSpawnManager()->GetTeamData( nIndex, 0);
                if (pSpawnData != NULL)
                {
                    ZGetGame()->m_pMyCharacter->SetPosition( pSpawnData->m_Pos);
                    ZGetGame()->m_pMyCharacter->SetDirection( pSpawnData->m_Dir);
                }
            }

            return;
        }
    }

    else if ( ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT )
    {
        ZRuleDuelTournament *pDTRule = (ZRuleDuelTournament*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
        if (pDTRule)
        {
            int nIndex = 2;
            if (pDTRule->m_DTGameInfo.uidPlayer1 == ZGetMyUID())                nIndex = 0;
            else if (pDTRule->m_DTGameInfo.uidPlayer2 == ZGetMyUID())            nIndex = 1;

            // µ?¾? ??¿? ¸???¸? ¸? ?³?½°? µ?¹??° ?§?¡
            if ( MGetMapDescMgr()->IsMapOnlyDuel( ZGetGameClient()->GetMatchStageSetting()->GetMapIndex()))
            {
                ZMapSpawnData* pSpawnData = ZGetGame()->GetMapDesc()->GetSpawnManager()->GetData(nIndex);
                if (pSpawnData != NULL)
                {
                    ZGetGame()->m_pMyCharacter->SetPosition(pSpawnData->m_Pos);
                    ZGetGame()->m_pMyCharacter->SetDirection(pSpawnData->m_Dir);
                }
            }

            // µ?¾? ??¿? ¸??? ¾?´?¸? ????°? °°?÷ ¹?½?
            else
            {
                ZMapSpawnData* pSpawnData = ZGetGame()->GetMapDesc()->GetSpawnManager()->GetTeamData(nIndex, 0);
                if (pSpawnData != NULL)
                {
                    ZGetGame()->m_pMyCharacter->SetPosition(pSpawnData->m_Pos);
                    ZGetGame()->m_pMyCharacter->SetDirection(pSpawnData->m_Dir);
                }
            }

            return;
        }
    }

    // ½?·??? °?¿?³× ??µµ??µµ ¾?´?¸? ·£´?
    ZMapSpawnData* pSpawnData = ZGetGame()->GetMapDesc()->GetSpawnManager()->GetSoloRandomData();
    if (pSpawnData != NULL)
    {
        ZGetGame()->m_pMyCharacter->SetPosition(pSpawnData->m_Pos);
        ZGetGame()->m_pMyCharacter->SetDirection(pSpawnData->m_Dir);
    }
}

void ZMatch::InitCharactersProperties()
{
    for (ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.begin();
        itor != ZGetGame()->m_CharacterManager.end(); ++itor)
    {
        ZCharacter* pCharacter = (*itor).second;
        pCharacter->InitStatus();
        pCharacter->SetVisible(true);
    }

}

void ZMatch::InitRound()
{
    ZGetGame()->InitRound();

    InitCharactersPosition();
    InitCharactersProperties();

    ZGetWorldItemManager()->Reset();

    rvector pos = ZGetGame()->m_pMyCharacter->GetPosition();
    rvector dir = ZGetGame()->m_pMyCharacter->m_DirectionLower;

    m_nRoundKills = 0;

    bool isObserver = false;

    if (ZGetGame()->GetSpawnRequested() == false) 
    {
        if (GetMatchType() == MMATCH_GAMETYPE_DUEL)
        {
            for (ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.begin();
                itor != ZGetGame()->m_CharacterManager.end(); ++itor)
            {
                ZCharacter* pCharacter = (*itor).second;
                pCharacter->ForceDie();
                pCharacter->SetVisible(false);
            }
        }
        else if(GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
        {
            for (ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.begin();
                itor != ZGetGame()->m_CharacterManager.end(); ++itor)
            {
                ZCharacter* pCharacter = (*itor).second;
                pCharacter->ForceDie();
                pCharacter->SetVisible(false);
            }
        }
        else
        {
            ZPostRequestSpawn(ZGetMyUID(), pos, dir);
            ZGetGame()->SetSpawnRequested(true);
        }
    }
   
    MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
    if (pObjCache && pObjCache->CheckFlag(MTD_PlayerFlags_AdminHide))
	{
        ZGetGameInterface()->GetCombatInterface()->SetObserverMode(true);
    } 
	else 
	{
      
        if (!isObserver)
            ZGetGame()->ReleaseObserver();
        else
        {
            ZGetGameInterface()->GetCombatInterface()->SetObserverMode(true);
            ZGetGame()->ReserveObserver();
            ZGetGame()->m_pMyCharacter->ForceDie();

        }
    }

    memset(m_nTeamKillCount, 0, sizeof(m_nTeamKillCount));
}



void ZMatch::SetRoundState(MMATCH_ROUNDSTATE nRoundState, int nArg)
{
    if (m_nRoundState == nRoundState) return;
    m_nRoundState = nRoundState;

#ifdef _LOG_ENABLE_ROUNDSTATE_
    char buf[128];
    switch(ZGetGame()->GetMatch()->GetRoundState())
    {
    case MMATCH_ROUNDSTATE_PREPARE:        sprintf(buf, "RoundState: PREPARE \n"); break;
    case MMATCH_ROUNDSTATE_COUNTDOWN:    sprintf(buf, "RoundState: COUNTDOWN \n"); break;
    case MMATCH_ROUNDSTATE_PLAY:        sprintf(buf, "RoundState: PLAY \n"); break;
    case MMATCH_ROUNDSTATE_FINISH:        sprintf(buf, "RoundState: FINISH \n"); break;
    case MMATCH_ROUNDSTATE_EXIT:        sprintf(buf, "RoundState: EXIT \n"); break;
    case MMATCH_ROUNDSTATE_FREE:        sprintf(buf, "RoundState: FREE \n"); break;
    case MMATCH_ROUNDSTATE_FAILED:        sprintf(buf, "RoundState: FAILED \n"); break;
    case MMATCH_ROUNDSTATE_PRE_COUNTDOWN: sprintf(buf, "RoundState: PRE_COUNTDOWN \n"); break;
    }
    OutputDebugString(buf);
#endif

    //# ¾??¸·?´? ?? switch ³»÷?¿¡ ·?¿¡ µ?¸¥ if else if else¸¦ ??°¡???? ¾?´?°? ??°?½?´?´?.
    //# ¶?¿?µ?»??? ÷¯?­½? ·?¿¡ µ?¶? ??¾??? ???÷ ZRule***??¿¡ ³?´? °??? ??°?½?´?´?. => virtual ZRule::OnSetRoundState()

    switch(m_nRoundState) 
    {

    case MMATCH_ROUNDSTATE_PREPARE: 
        {

        }
        break;
    case MMATCH_ROUNDSTATE_PRE_COUNTDOWN:
        {
        }
        break;
    case MMATCH_ROUNDSTATE_COUNTDOWN : 
        {
            OutputDebugString("MMATCH_ROUNDSTATE_COUNTDOWN>> InitRound BEGIN \n");
            InitRound();
            OutputDebugString("MMATCH_ROUNDSTATE_COUNTDOWN>> InitRound END \n");
        }
        break;
    case MMATCH_ROUNDSTATE_PLAY:
        {
            if (!IsTeamPlay())
            {

            }
        }
        break;
    case MMATCH_ROUNDSTATE_FINISH:
        {
            ZGetGame()->FlushObserverCommands();

            if (GetMatchType() == MMATCH_GAMETYPE_DUEL)
            {                
                for (ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.begin();
                    itor != ZGetGame()->m_CharacterManager.end(); ++itor)
                {
                    ZCharacter* pCharacter = (*itor).second;
                    if (pCharacter->GetKils() >= GetRoundCount())
                    {
                        ZGetGameInterface()->FinishGame();
                        break;
                    }
                }    
            }
            else if(GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
            {
                //
            }
            else
            {
                if (m_nCurrRound + 1 >= GetRoundCount())
                {
                    ZGetGameInterface()->FinishGame();
                }        
            }

            if (IsTeamPlay())
            {
                if (nArg == MMATCH_ROUNDRESULT_RED_ALL_OUT )
                {
                    if( m_nTeamScore[MMT_BLUE] < m_nTeamScore[MMT_RED] )
                        m_nTeamScore[MMT_BLUE] = m_nTeamScore[MMT_RED] = 0;
                    else
                        m_nTeamScore[MMT_BLUE]++;
                }
                else if( nArg ==  MMATCH_ROUNDRESULT_BLUE_ALL_OUT )
                {
                    if( m_nTeamScore[MMT_RED] < m_nTeamScore[MMT_BLUE] )
                        m_nTeamScore[MMT_RED] =    m_nTeamScore[MMT_BLUE] = 0;
                    else
                        m_nTeamScore[MMT_RED]++;
                }
                else if (nArg == MMATCH_ROUNDRESULT_DRAW)
                {
                    // Do nothing...
                } 
                else {
                    MMatchTeam nTeamWon = (nArg == MMATCH_ROUNDRESULT_REDWON ? MMT_RED : MMT_BLUE);
                    if (nTeamWon == MMT_RED)
                        m_nTeamScore[MMT_RED]++;
                    else if (nTeamWon == MMT_BLUE)
                        m_nTeamScore[MMT_BLUE]++;
                }
            }
        }
        break;
    case MMATCH_ROUNDSTATE_EXIT:
        {
            
        }
        break;
    case MMATCH_ROUNDSTATE_FREE:
        {

        }
        break;
    };

    m_pRule->OnSetRoundState(m_nRoundState);
}

const char* ZMatch::GetTeamName(int nTeamID)
{
    switch (nTeamID)
    {
    case MMT_SPECTATOR:
        return MMATCH_SPECTATOR_STR;
    case MMT_RED:
        return MMATCH_TEAM1_NAME_STR;
    case MMT_BLUE:
        return MMATCH_TEAM2_NAME_STR;
    default:
        return "";
    }
    return "";
}


int ZMatch::GetRoundCount()
{
//    if (IsWaitForRoundEnd())
        return m_pStageSetting->GetStageSetting()->nRoundMax;

    return 1;
}


void ZMatch::GetTeamAliveCount(int* pnRedTeam, int* pnBlueTeam)
{
    int nRedTeam = 0, nBlueTeam = 0;
    if (IsTeamPlay())
    {
        for (ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.begin();
            itor != ZGetGame()->m_CharacterManager.end(); ++itor)
        {
            ZCharacter* pCharacter = (*itor).second;
            if (!pCharacter->IsDie())
            {
                if (pCharacter->GetTeamID() == 0)
                {
                    nRedTeam++;
                }
                else
                {
                    nBlueTeam++;
                }
            }
        }
    }

    *pnRedTeam = nRedTeam;
    *pnBlueTeam = nBlueTeam;
}

void ZMatch::RespawnSolo()
{
    if ((!IsWaitForRoundEnd()) && (ZGetGame()->m_pMyCharacter->IsDie()))
    {
        SoloSpawn();
    }
}

void ZMatch::OnForcedEntry(ZCharacter* pCharacter)
{
    if (pCharacter == NULL)
    {
        _ASSERT(0); return;
    }
    // ³­???? ??·¹??¾?°¡ ³× ??½??? °?¿?
    if (pCharacter == ZGetGame()->m_pMyCharacter)
    {
        // AdminHide ?³¸®
        MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
        if (pObjCache && pObjCache->CheckFlag(MTD_PlayerFlags_AdminHide)) {
            ZGetGameInterface()->GetCombatInterface()->SetObserverMode(true);
        } else {
            // ????·¹??¸? ½÷????????
            if (IsWaitForRoundEnd()/* && ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL*/)
            {
                pCharacter->SetVisible(false);
                pCharacter->ForceDie();
                ZGetCombatInterface()->SetObserverMode(true);
            }    
            else    // ¼?·???°?¿?´? ¹?·? ½÷??
            {
                InitRound();
            }
        }
    }
    else
    {
        // ????·¹??¿¡ ´?¸¥ ??·¹??¾?°¡ ³­????¸? ´??½??÷??? ??????¾? ??±?¶§¹®¿¡ ?³¸¯?? ÷¸¿©???? ¾??½
        // ´? FREE»????? ¶§¿¡´? ÷¸¿©??´?
        if (IsWaitForRoundEnd() && (GetRoundState() != MMATCH_ROUNDSTATE_FREE))
        {
            if (pCharacter != NULL)
            {
                pCharacter->SetVisible(false);
            }
            pCharacter->ForceDie();
        }
    }
}

int ZMatch::GetRemainedSpawnTime()
{
    int nTimeSec = -1;

    if (GetRoundState() == MMATCH_ROUNDSTATE_PLAY)
    {
        if (!IsWaitForRoundEnd())
        {
            if (ZGetGame()->m_pMyCharacter->IsDie())
            {
                if (m_nSoloSpawnTime < 0) return -1;

                //¹?????÷¸??½???¼® 
                /*ZBuffSummary* pBuffSummary = ZGetGame()->m_pMyCharacter->GetCharacterBuff()->GetBuffSummary();
                int nDelayAfterDying = pBuffSummary->GetRespawnTime(RESPAWN_DELAYTIME_AFTER_DYING);*/
                int nDelayAfterDying = RESPAWN_DELAYTIME_AFTER_DYING;

                int nElapsedTime = m_nSoloSpawnTime;
                if (nElapsedTime > nDelayAfterDying) 
                    nElapsedTime = nDelayAfterDying;

                nTimeSec = (((nDelayAfterDying - nElapsedTime)+999) / 1000);
            }
        }
    }

    return nTimeSec;
}


void ZMatch::SetRoundStartTime(DWORD dwElapsed)
{
	//! Custom: dwElapsed
	m_dwStartTime = timeGetTime() - dwElapsed; // Yes, decrease.

	if (timeGetTime() <= dwElapsed)
		m_dwStartTime = timeGetTime(); // glitch?
}

DWORD ZMatch::GetRemaindTime( void)
{
    return ( timeGetTime() - m_dwStartTime);
}


bool ZMatch::OnCommand(MCommand* pCommand)
{
    if (m_pRule) return m_pRule->OnCommand(pCommand);
    return false;
}

void ZMatch::OnResponseRuleInfo(MTD_RuleInfo* pInfo)
{
    if (pInfo->nRuleType != GetMatchType()) return;

    if (m_pRule)
    {
        m_pRule->OnResponseRuleInfo(pInfo);
    }
}