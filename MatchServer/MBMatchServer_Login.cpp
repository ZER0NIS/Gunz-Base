#include "stdafx.h"
#include "MBMatchServer.h"
#include "MBMatchNHNAuth.h"
#include "MBMatchAsyncDBJob_NHNLogin.h"
#include "MBMatchAsyncDBJob_GameOnLogin.h"
#include "MBMatchAsyncDBJob_NetmarbleLogin.h"
#include "MMatchConfig.h"
#include "MMatchLocale.h"
#include "MMatchAuth.h"
#include "MBMatchAuth.h"
#include "MMatchStatus.h"
#include "MMatchGlobal.h"
#include "MBMatchGameOnAuth.h"


void MBMatchServer::OnRequestLoginNetmarble(const MUID& CommUID, const char* szAuthCookie, const char* szDataCookie, const char* szCPCookie, const char* szSpareData, int nCmdVersion, unsigned long nChecksumPack)
{
	return;
	
}


void MBMatchServer::OnRequestLoginNHNUSA( const MUID& CommUID, const char* pszUserID, const char* pszAuthStr, const int nCommandVersion, const int nCheckSumPack, char* szEncryptMd5Value )
{
	return;
}

void MBMatchServer::OnRequestLoginGameOn( const MUID& CommUID, const char* szString, const char* szStatIndex, int nCommandVersion, int nCheckSumPack, char* szEncryptMd5Value )
{
	return;
}


void MBMatchServer::OnRequestAccountCharList(const MUID& uidPlayer, unsigned char* pbyGuidAckMsg)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	
#ifdef _HSHIELD
	if( MGetServerConfig()->IsUseHShield() )
	{
		DWORD dwRet = HShield_AnalyzeGuidAckMsg(pbyGuidAckMsg, pObj->GetHShieldInfo()->m_pbyGuidReqInfo, 
			&pObj->GetHShieldInfo()->m_pCRCInfo);

		if(dwRet!= ERROR_SUCCESS)
		{
			pObj->GetHShieldInfo()->m_bGuidAckPass = false;
			LOG(LOG_FILE, "@AnalyzeGuidAckMsg - Find Hacker(%s) : (Error Code = %x)", 
				pObj->GetAccountName(), dwRet);

			MCommand* pNewCmd = CreateCommand(MC_MATCH_FIND_HACKING, MUID(0,0));
			RouteToListener(pObj, pNewCmd);

#ifndef _DEBUG
			// 비정상적 유저이므로 캐릭터 선택할때 접속을 끊는다.
			pObj->SetHacker(true);
#endif
		}
		else
		{
			pObj->GetHShieldInfo()->m_bGuidAckPass = true;
			if(pObj->GetHShieldInfo()->m_pCRCInfo == NULL)
				mlog("%s's HShield_AnalyzeGuidAckMsg Success. but pCrcInfo is NULL...\n", pObj->GetAccountName());
		}
//		SendHShieldReqMsg();
	}
#endif

	const MMatchHackingType MHackingType = pObj->GetAccountInfo()->m_HackingType;
	
	if( MGetServerConfig()->IsBlockHacking() && (MMHT_NO != MHackingType) && !IsAdminGrade(pObj) )
	{
		// 여기서 블럭 시간 만료를 검사해 줘야 한다.
		if( !IsExpiredBlockEndTime(pObj->GetAccountInfo()->m_EndBlockDate) )
		{
			DWORD dwMsgID = 0;

			if( MMHT_XTRAP_HACKER == MHackingType)				 dwMsgID = MERR_BLOCK_HACKER;
			else if(MMHT_HSHIELD_HACKER == MHackingType)		 dwMsgID = MERR_BLOCK_HACKER;
			else if(MMHT_BADUSER == MHackingType)				 dwMsgID = MERR_BLOCK_BADUSER;			
			else if(MMHT_COMMAND_FLOODING == MHackingType)		 dwMsgID = MERR_FIND_FLOODING;
			else if(MMHT_GAMEGUARD_HACKER == MHackingType) {}
			else
				dwMsgID = MERR_FIND_HACKER;

			pObj->GetDisconnStatusInfo().SetMsgID( dwMsgID );
			pObj->GetDisconnStatusInfo().SetStatus( MMDS_DISCONN_WAIT );
			return;
		}
		else if( MMHT_SLEEP_ACCOUNT == MHackingType ) 
		{
			pObj->GetDisconnStatusInfo().SetMsgID( MERR_BLOCK_SLEEP_ACCOUNT );
			pObj->GetDisconnStatusInfo().SetStatus( MMDS_DISCONN_WAIT );
			return;
		}
		else
		{
			// 기간이 만료되었으면 DB를 정상유저로 업데이트 해준다.
			pObj->GetAccountInfo()->m_HackingType = MMHT_NO;

			MAsyncDBJob_ResetAccountHackingBlock* pResetBlockJob = new MAsyncDBJob_ResetAccountHackingBlock(uidPlayer);
			pResetBlockJob->Input( pObj->GetAccountInfo()->m_nAID, MMHT_NO );

			// PostAsyncJob( pResetBlockJob );
			pObj->m_DBJobQ.DBJobQ.push_back( pResetBlockJob );
		}
	}

	if( pObj->GetAccountPenaltyInfo()->IsBlock(MPC_CONNECT_BLOCK) ) {
		pObj->GetDisconnStatusInfo().SetMsgID( MERR_BLOCK_BADUSER );
		pObj->GetDisconnStatusInfo().SetStatus( MMDS_DISCONN_WAIT );
		return;
	}

	// Async DB //////////////////////////////
	pObj->UpdateTickLastPacketRecved();

	// 퀘스트 모드가 아니면 업데이트할 데이터가 없음. -- by SungE. 2006-11-07
	if( MSM_QUEST == MGetServerConfig()->GetServerMode() )
	{
		if( 0 != pObj->GetCharInfo() )
		{
			MAsyncDBJob_UpdateQuestItemInfo* pQItemUpdateJob = new MAsyncDBJob_UpdateQuestItemInfo(pObj->GetUID());
			if( 0 == pQItemUpdateJob )
				return;

			if( !pQItemUpdateJob->Input(pObj->GetCharInfo()->m_nCID, 
				pObj->GetCharInfo()->m_QuestItemList, 
				pObj->GetCharInfo()->m_QMonsterBible) )
			{
				mlog( "MMatchServer::OnAsyncGetAccountCharList - 객체 생성 실패.\n" );
				delete pQItemUpdateJob;
				return;
			}

			// 여기서 퀘스트 업데이트 진행이 되면 CharFinalize에서 중복 업데이트가 될수 있기에,
			//  CharFinalize에서 아래 플레그가 true면 업데이트를 진행하게 해놓았음.
			pObj->GetCharInfo()->m_QuestItemList.SetDBAccess( false );

			// PostAsyncJob( pQItemUpdateJob );
			pObj->m_DBJobQ.DBJobQ.push_back( pQItemUpdateJob );
		}
	}

	MAsyncDBJob_GetAccountCharList* pJob=new MAsyncDBJob_GetAccountCharList(uidPlayer,pObj->GetAccountInfo()->m_nAID);
	// PostAsyncJob(pJob);
	pObj->m_DBJobQ.DBJobQ.push_back( pJob );
}