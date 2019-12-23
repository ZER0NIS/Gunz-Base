#include "stdafx.h"
#include "MMatchServer.h"
#include "MAsyncDBJob.h"
#include "MAsyncDBJob_FriendList.h"
#include "MAsyncDBJob_GetLoginInfo.h"
#include "MAsyncDBJob_InsertConnLog.h"
#include "MBlobArray.h"
#include "MMatchFormula.h"
#include "MAsyncDBJob_Event.h"
#include "../MAsyncDBJob_GetAccountItemList.h"
#include "MAsyncDBJob_BuyQuestItem.h"
#include "MAsyncDBJob_SurvivalMode.h"				// 2009. 6. 3 - Added By Hong KiJu

void MMatchServer::PostAsyncJob(MAsyncJob* pJob )
{
	m_AsyncProxy.PostJob(pJob);
}

void MMatchServer::ProcessAsyncJob()
{
	while(MAsyncJob* pJob = m_AsyncProxy.GetJobResult()) 
	{
		//////////////////////////////////////////////////////////////////////
		{
			MMatchObject* pPlayer = GetObject( pJob->GetOwnerUID() );
			if( NULL != pPlayer )
			{
				pPlayer->m_DBJobQ.bIsRunningAsyncJob = false;
			}
		}
		//////////////////////////////////////////////////////////////////////

		switch(pJob->GetJobID()) {
		case MASYNCJOB_GETACCOUNTCHARLIST:
			{
				OnAsyncGetAccountCharList(pJob);
			}
			break;				
		//case MASYNCJOB_GETACCOUNTCHARINFO:
		//	{
		//		OnAsyncGetAccountCharInfo(pJob);
		//	}
		//	break;				
		//case MASYNCJOB_GETCHARINFO:
		//	{
		//		OnAsyncGetCharInfo(pJob);
		//	}
		//	break;
		case MASYNCJOB_FRIENDLIST:
			{
				OnAsyncGetFriendList(pJob);
			}
			break;			
		case MASYNCJOB_CREATECHAR:
			{
				OnAsyncCreateChar(pJob);
			}
			break;
		case MASYNCJOB_GETLOGININFO:
			{
				OnAsyncGetLoginInfo(pJob);
			}
			break;
		case MASYNCJOB_DELETECHAR:
			{
				OnAsyncDeleteChar(pJob);
			}
			break;
		case MASYNCJOB_WINTHECLANGAME:
			{
				OnAsyncWinTheClanGame(pJob);
			}
			break;
		case MASYNCJOB_UPDATECHARINFODATA:
			{
				OnAsyncUpdateCharInfoData(pJob);
			}
			break;
		case MASYNCJOB_CHARFINALIZE:
			{
				OnAsyncCharFinalize(pJob);
			}
			break;
		//case MASYNCJOB_BRINGACCOUNTITEM:
		//	{
		//		OnAsyncBringAccountItem(pJob);
		//	}
		//	break;
		case MASYNCJOB_INSERTCONNLOG:
			{
				OnAsyncInsertConnLog(pJob);
			}
			break;
		case MASYNCJOB_INSERTGAMELOG:
			{
				OnAsyncInsertGameLog(pJob);
			}
			break;
		case MASYNCJOB_INSERTGAMEPLAYERLOG:
			{

			}
			break;
		case MASYNCJOB_CREATECLAN:
			{
				OnAsyncCreateClan(pJob);
			}
			break;
		case MASYNCJOB_EXPELCLANMEMBER:
			{
				OnAsyncExpelClanMember(pJob);
			}
			break;
			
		case MASYNCJOB_INSERTQUESTGAMELOG :
			{

			}
			break;
		case MASYNCJOB_UPDATEQUESTITEMINFO :
			{
			}
			break;
			

		case MASYNCJOB_PROBABILITYEVENTPERTIME :
			{
				OnAsyncInsertEvent( pJob );
			}
			break;

		case MASYNCJOB_UPDATEIPTOCOUNTRYLIST :
			{
				OnAsyncUpdateIPtoCoutryList( pJob );
			};
			break;

		case MASYNCJOB_UPDATEBLOCKCOUNTRYCODELIST :
			{
				OnAsyncUpdateBlockCountryCodeList( pJob );
			}
			break;

		case MASYNCJOB_UPDATECUSTOMIPLIST :
			{
				OnAsyncUpdateCustomIPList( pJob );
			}
			break;

		case MASYNCJOB_GETACCOUNTITEMLIST :
			{
				OnAsyncGetAccountItemList( pJob );
			}
			break;

		case MASYNCJOB_BUYQUESTITEM :
			{
				OnAsyncBuyQuestItem( pJob );
			}
			break;


		//////////////////////////////////////////////////////////////////////////////////////////////
		// 2009. 6. 3 - Added By Hong KiJu
		case MASYNCJOB_INSERT_SURVIVALMODE_GAME_LOG :
			{
				OnAsyncSurvivalModeGameLog(pJob);				
			}
			break;
		case MASYNCJOB_GET_SURVIVALMODE_GROUP_RANKING :
			{
				OnAsyncSurvivalModeGroupRanking(pJob);
			}
			break;

		case MASYNCJOB_GET_SURVIVALMODE_PRIVATE_RANKING :
			{
				OnAsyncSurvivalModePrivateRanking(pJob);
			}
			break;

		//////////////////////////////////////////////////////////////////////////////////////////////
		// Added By Hong KiJu(2009-09-25)
		case MASYNCJOB_GET_DUELTOURNAMENT_CHARINFO :
			{
				OnAsyncResponse_GetDuelTournamentCharacterInfo(pJob);
			}
			break;

		case MASYNCJOB_GET_DUELTOURNAMENT_PREVIOUS_CHARINFO :
			{
				OnAsyncResponse_GetDuelTournamentPreviousCharacterInfo(pJob);

			}
			break;

		case MASYNCJOB_GET_DUELTOURNAMENT_SIDERANKING :
			{
				OnAsyncResponse_GetDuelTournamentSideRanking(pJob);
			}
			break;
		
		case MASYNCJOB_GET_DUELTOURNAMENT_GROUPRANKING :
			{
				OnAsyncResponse_GetDuelTournamentGroupRanking(pJob);
			}
			break;

		case MASYNCJOB_GET_DUELTOURNAMENT_TIMESTAMP :
			{
				OnAsyncResponse_GetDuelTournamentTimeStamp(pJob);
			}
			break;

		case MASYNCJOB_UPDATE_DUELTOURNAMENT_CHARINFO :
		case MASYNCJOB_UPDATE_DUELTOURNAMENT_GAMELOG :
		case MASYNCJOB_INSERT_DUELTOURNAMENT_GAMELOGDETAIL :
			{

			}
			break;

		case MASYNCJOB_UPDATE_CHARITEM_COUNT :
			{

			}
			break;

		case MASYNCJOB_GET_BR_DESCRIPTION:
			{
				OnAsyncResponse_GetBR_Description(pJob);
			}
			break;

		case MASYNCJOB_GET_CHAR_BR_INFO:
			{
				OnAsyncResponse_GetCharBRInfo(pJob);
			}
			break;

		case MASYNCJOB_UPDATE_CHAR_BR_INFO:
			{
				OnAsyncResponse_UpdateCharBRInfo(pJob);
			}
			break;

		case MASYNCJOB_REWARD_CHAR_BR:
			{
				OnAsyncResponse_RewardCharBR(pJob);
			}
			break;

		default :
			{
				OnProcessAsyncJob( pJob );
			}
			break;
		};

		delete pJob;
	}
}



void MMatchServer::OnAsyncGetLoginInfo(MAsyncJob* pJobInput)
{
	MAsyncDBJob_GetLoginInfo* pJob = (MAsyncDBJob_GetLoginInfo*)pJobInput;


	if (pJob->GetResult() != MASYNC_RESULT_SUCCEED) 
	{		
		// Notify Message 필요 -> 로그인 관련 - 해결(Login Fail 메세지 이용)
		// Disconnect(pJob->GetCommUID());
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(pJob->GetCommUID(), MERR_FAILED_GETACCOUNTINFO);
		Post(pCmd);

		pJob->DeleteMemory();
		return;
	}

	MMatchAccountInfo* pAccountInfo = pJob->GetAccountInfo();
	if( pAccountInfo == 0 ) return;

	MMatchAccountPenaltyInfo* pAccountPenaltyInfo = pJob->GetAccountPenaltyInfo();
	if( pAccountPenaltyInfo == 0 ) return;


#ifndef _DEBUG
	// 중복 로그인이면 이전에 있던 사람을 끊어버린다.
	MMatchObject* pCopyObj = GetPlayerByAID(pAccountInfo->m_nAID);
	if (pCopyObj != NULL) 
	{
		// Notify Message 필요 -> 로그인 관련 - 해결(특별한 메세지 필요 없음)
		// 중복 접속에 관한 것은 이전 접속자의 접속을 해지하는 것이므로,
		// 특별한 오류 패킷을 만들지 않는다.
		Disconnect(pCopyObj->GetUID());
	}
#endif

	// 사용정지 계정인지 확인한다.
	if ((pAccountInfo->m_nUGrade == MMUG_BLOCKED) || (pAccountInfo->m_nUGrade == MMUG_PENALTY))
	{
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(pJob->GetCommUID(), MERR_CLIENT_MMUG_BLOCKED);
		Post(pCmd);

		pJob->DeleteMemory();
		return;
	}

	AddObjectOnMatchLogin(pJob->GetCommUID(), pJob->GetAccountInfo(), pJob->GetAccountPenaltyInfo(), 
		pJob->IsFreeLoginIP(), pJob->GetCountryCode3(), pJob->GetChecksumPack());

/*
	// 할당...
	MUID AllocUID = CommUID;
	int nErrCode = ObjectAdd(CommUID);
	if(nErrCode!=MOK) {
		LOG(LOG_DEBUG, MErrStr(nErrCode) );
	}

	MMatchObject* pObj = GetObject(AllocUID);
	if (pObj == NULL)
	{
		Disconnect(CommUID);
		delete pJob->GetAccountInfo();
		return;
	}

	pObj->AddCommListener(CommUID);
	pObj->SetObjectType(MOT_PC);
	memcpy(pObj->GetAccountInfo(), pAccountInfo, sizeof(MMatchAccountInfo));
	pObj->SetFreeLoginIP(pJob->IsFreeLoginIP());
	pObj->SetCountryCode3( pJob->GetCountryCode3() );


	MCommObject* pCommObj = (MCommObject*)m_CommRefCache.GetRef(CommUID);
	if (pCommObj != NULL)
	{
		pObj->SetPeerAddr(pCommObj->GetIP(), pCommObj->GetIPString(), pCommObj->GetPort());
	}
	
	SetClientClockSynchronize(CommUID);

	MCommand* pCmd = CreateCmdMatchResponseLoginOK(CommUID, AllocUID, pAccountInfo->m_szUserID, pAccountInfo->m_nUGrade, pAccountInfo->m_nPGrade);
	Post(pCmd);	


	// 접속 로그
	MAsyncDBJob_InsertConnLog* pNewJob = new MAsyncDBJob_InsertConnLog();
	pNewJob->Input(pObj->GetAccountInfo()->m_nAID, pObj->GetIPString(), pObj->GetCountryCode3() );
	PostAsyncJob(pNewJob);

#ifndef _DEBUG
	// Client DataFile Checksum을 검사한다.
	unsigned long nChecksum = pJob->GetChecksumPack() ^ CommUID.High ^ CommUID.Low;
	if (nChecksum != GetItemFileChecksum()) {
		LOG(LOG_PROG, "Invalid ZItemChecksum(%u) , UserID(%s) ", nChecksum, pObj->GetAccountInfo()->m_szUserID);
		Disconnect(CommUID);
	}
#endif

	delete pJob->GetAccountInfo();
*/

}

void MMatchServer::OnAsyncGetAccountCharList(MAsyncJob* pJobResult)
{
	MAsyncDBJob_GetAccountCharList* pJob = (MAsyncDBJob_GetAccountCharList*)pJobResult;

	if (pJob->GetResult() != MASYNC_RESULT_SUCCEED) {
		char szTime[128]="";
		_strtime(szTime);

		mlog("[%s] Async DB Query(ResponseAccountCharList) Failed\n", szTime);
		return;
	}		

	MMatchObject* pObj = GetObject(pJob->GetUID());
	if (pObj == NULL) 
		return;

	const int					nCharCount		= pJob->GetCharCount();
	const MTD_AccountCharInfo * pCharList		= pJob->GetCharList();
	MTD_AccountCharInfo*		pTransCharInfo	= NULL;
	int							nCharMaxLevel	= 0;

	MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_ACCOUNT_CHARLIST, MUID(0,0));
	void* pCharArray = MMakeBlobArray(sizeof(MTD_AccountCharInfo), nCharCount);

	for (int i = 0; i < nCharCount; i++)
	{
		pTransCharInfo = (MTD_AccountCharInfo*)MGetBlobArrayElement(pCharArray, i);
		memcpy(pTransCharInfo, &pCharList[i], sizeof(MTD_AccountCharInfo));

		nCharMaxLevel = max(nCharMaxLevel, pTransCharInfo->nLevel);
	}

	pObj->CheckNewbie( nCharMaxLevel );

	pNewCmd->AddParameter(new MCommandParameterBlob(pCharArray, MGetBlobArraySize(pCharArray)));
	MEraseBlobArray(pCharArray);
    
	RouteToListener( pObj, pNewCmd );
}

//void MMatchServer::OnAsyncGetAccountCharInfo(MAsyncJob* pJobResult)
//{
//	MAsyncDBJob_GetAccountCharInfo* pJob = (MAsyncDBJob_GetAccountCharInfo*)pJobResult;
//
//	if (pJob->GetResult() != MASYNC_RESULT_SUCCEED) {
//		char szTime[128]="";
//		_strtime(szTime);
//
//		mlog("[%s] Async DB Query(ResponseAccountCharInfo) Failed\n", szTime);
//		return;
//	}		
//
//	MMatchObject* pObj = GetObject(pJob->GetUID());
//	if (pObj == NULL) return;
//	if (pJob->GetResultCommand() == NULL) return;
//
//	RouteToListener(pObj, pJob->GetResultCommand());
//}


//void MMatchServer::OnAsyncGetCharInfo(MAsyncJob* pJobResult)
//{
//	MAsyncDBJob_GetCharInfo* pJob = (MAsyncDBJob_GetCharInfo*)pJobResult;
//
//	if (pJob->GetResult() != MASYNC_RESULT_SUCCEED) {
//		mlog("DB Query(OnAsyncGetCharInfo > GetCharInfoByAID) Failed\n");
//		return;
//	}
//
//	MMatchObject* pObj = GetObject(pJob->GetUID());
//	if (pObj == NULL) return;
//
//	if (pObj->GetCharInfo())
//	{
//		// 이전에 캐릭이 선택되어 있었다면 캐릭끝날때 로그 남긴다
//		if (pObj->GetCharInfo()->m_nCID != 0)
//		{
//			CharFinalize(pObj->GetUID());		// 캐릭끝날때 디비 로그 등 처리
//		}
//
//		pObj->FreeCharInfo();
//		pObj->FreeFriendInfo();
//	}
//
//	if (pJob->GetCharInfo() == NULL)
//	{
//		mlog("pJob->GetCharInfo() IS NULL\n");
//		return;
//	}
//	pObj->SetCharInfo(pJob->GetCharInfo());		// Save Async Result
////	pObj->SetFriendInfo(pJob->GetFriendInfo());	// Save Async Result
//
//	if (CharInitialize(pJob->GetUID()) == false)
//	{
//		mlog("OnAsyncGetCharInfo > CharInitialize failed");
//		return;
//	}
//
//	// Client에 선택한 캐릭터 정보 전송
//	MTD_CharInfo trans_charinfo;
//	CopyCharInfoForTrans(&trans_charinfo, pJob->GetCharInfo(), pObj);
//	
//	MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_SELECT_CHAR, MUID(0,0));
//	pNewCmd->AddParameter(new MCommandParameterInt(MOK));		// result
//
//	void* pCharArray = MMakeBlobArray(sizeof(MTD_CharInfo), 1);
//	MTD_CharInfo* pTransCharInfo = (MTD_CharInfo*)MGetBlobArrayElement(pCharArray, 0);
//	memcpy(pTransCharInfo, &trans_charinfo, sizeof(MTD_CharInfo));
//	pNewCmd->AddParameter(new MCommandParameterBlob(pCharArray, MGetBlobArraySize(pCharArray)));
//	MEraseBlobArray(pCharArray);
//
//
//	// 내 캐릭터의 추가 정보
//	void* pMyExtraInfoArray = MMakeBlobArray(sizeof(MTD_MyExtraCharInfo), 1);
//	MTD_MyExtraCharInfo* pMyExtraInfo = (MTD_MyExtraCharInfo*)MGetBlobArrayElement(pMyExtraInfoArray, 0);
//	int nPercent = MMatchFormula::GetLevelPercent(trans_charinfo.nXP, (int)trans_charinfo.nLevel);
//	pMyExtraInfo->nLevelPercent = (char)nPercent;
//	pNewCmd->AddParameter(new MCommandParameterBlob(pMyExtraInfoArray, MGetBlobArraySize(pMyExtraInfoArray)));
//	MEraseBlobArray(pMyExtraInfoArray);
//
//	RouteToListener(pObj, pNewCmd);
//
//#ifdef _DELETE_CLAN
//	if( MMCDS_NORMAL != pJob->GetDeleteState() )
//	{
//		if( MMCDS_WAIT )
//		{
//			// 글랜 폐쇄 날짜를 알려줌.
//			
//			MCommand* pCmdDelClan = CreateCommand( MC_MATCH_CLAN_ACCOUNCE_DELETE, pObj->GetUID() );
//			pCmdDelClan->AddParameter( new MCmdParamStr(pObj->GetCharInfo()->m_ClanInfo.m_strDeleteDate.c_str()) );
//			Post( pCmdDelClan );
//		}
//		else if( MMCDS_DELETE )
//		{
//			// 클랜 폐쇄 시킴.
//		}
//	}
//#endif
//}

void MMatchServer::OnAsyncGetFriendList(MAsyncJob* pJobInput)
{
	MAsyncDBJob_FriendList* pJob = (MAsyncDBJob_FriendList*)pJobInput;

	if (pJob->GetResult() != MASYNC_RESULT_SUCCEED) 
	{
		return;
	}

	MMatchObject* pObj = GetObject(pJob->GetUID());
	if (!IsEnabledObject(pObj)) return;

	pObj->SetFriendInfo(pJob->GetFriendInfo());	// Save Async Result

	FriendList(pObj->GetUID());
}

void MMatchServer::OnAsyncCreateChar(MAsyncJob* pJobResult)
{
	MAsyncDBJob_CreateChar* pJob = (MAsyncDBJob_CreateChar*)pJobResult;

	if (pJob->GetResult() != MASYNC_RESULT_SUCCEED) {
		char szTime[128]="";
		_strtime(szTime);

		mlog("[%s] Async DB Query(CreateChar) Failed\n", szTime);
		return;
	}		

	MMatchObject* pObj = GetObject(pJob->GetUID());
	if (pObj == NULL) return;

	// Make Result
	MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_CREATE_CHAR, MUID(0,0));
	pNewCmd->AddParameter(new MCommandParameterInt(pJob->GetDBResult()));			// result
	pNewCmd->AddParameter(new MCommandParameterString(pJob->GetCharName()));	// 만들어진 캐릭터 이름

	RouteToListener( pObj, pNewCmd );
}

void MMatchServer::OnAsyncDeleteChar(MAsyncJob* pJobResult)
{
	MAsyncDBJob_DeleteChar* pJob = (MAsyncDBJob_DeleteChar*)pJobResult;

	if (pJob->GetResult() != MASYNC_RESULT_SUCCEED) {
		char szTime[128]="";
		_strtime(szTime);

		mlog("[%s] Async DB Query(DeleteChar) Failed\n", szTime);
	}		

	MMatchObject* pObj = GetObject(pJob->GetUID());
	if (pObj == NULL) return;

	RouteResponseToListener(pObj, MC_MATCH_RESPONSE_DELETE_CHAR, pJob->GetDeleteResult());
}

void MMatchServer::OnAsyncWinTheClanGame(MAsyncJob* pJobInput)
{
	if (pJobInput->GetResult() != MASYNC_RESULT_SUCCEED) {
		char szTime[128]="";
		_strtime(szTime);

		mlog("[%s] Async DB Query(OnAsyncWinTheClanGame) Failed\n", szTime);
		return;
	}		

}


void MMatchServer::OnAsyncUpdateCharInfoData(MAsyncJob* pJobInput)
{
	if (pJobInput->GetResult() != MASYNC_RESULT_SUCCEED) {
		char szTime[128]="";
		_strtime(szTime);

		mlog("[%s] Async DB Query(OnAsyncUpdateCharInfoData) Failed\n", szTime);
		return;
	}		

}

void MMatchServer::OnAsyncCharFinalize(MAsyncJob* pJobInput)
{
	if (pJobInput->GetResult() != MASYNC_RESULT_SUCCEED) {
		char szTime[128]="";
		_strtime(szTime);

		mlog("[%s] Async DB Query(OnAsyncCharFinalize) Failed\n", szTime);
		return;
	}

}

//void MMatchServer::OnAsyncBringAccountItem(MAsyncJob* pJobResult)
//{
//	MAsyncDBJob_BringAccountItem* pJob = (MAsyncDBJob_BringAccountItem*)pJobResult;
//
//	MMatchObject* pObj = GetObject(pJob->GetUID());
//	if (!IsEnabledObject(pObj)) return;
//
//	int nRet = MERR_UNKNOWN;
//
//	if (pJob->GetResult() == MASYNC_RESULT_SUCCEED) 
//	{
//		unsigned long int nNewCIID =	pJob->GetNewCIID();
//		unsigned long int nNewItemID =	pJob->GetNewItemID();
//		bool bIsRentItem =				pJob->GetRentItem();
//		int nRentMinutePeriodRemainder = pJob->GetRentMinutePeriodRemainder();
//
//
//
//
//		// 오브젝트에 아이템 추가
//		MUID uidNew = MMatchItemMap::UseUID();
//		pObj->GetCharInfo()->m_ItemList.CreateItem(uidNew, nNewCIID, nNewItemID, bIsRentItem, nRentMinutePeriodRemainder);
//
//		nRet = MOK;
//	}		
//
//	ResponseCharacterItemList(pJob->GetUID());	// 새로 바뀐 아이템 리스트도 다시 뿌려준다.
//
//
//	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_ACCOUNTITEM, MUID(0,0));
//	pNew->AddParameter(new MCmdParamInt(nRet));
//	RouteToListener(pObj, pNew);
//
//
//}

void MMatchServer::OnAsyncInsertConnLog(MAsyncJob* pJobResult)
{
	if (pJobResult->GetResult() != MASYNC_RESULT_SUCCEED) {
		char szTime[128]="";
		_strtime(szTime);

		mlog("[%s] Async DB Query(OnAsyncInsertConnLog) Failed\n", szTime);
		return;
	}		

}

void MMatchServer::OnAsyncInsertGameLog(MAsyncJob* pJobResult)
{
	MAsyncDBJob_InsertGameLog* pJob = (MAsyncDBJob_InsertGameLog*)pJobResult;

	if (pJob->GetResult() != MASYNC_RESULT_SUCCEED) {
		char szTime[128]=""; _strtime(szTime);
		mlog("[%s] Async DB Query(OnAsyncInsertGameLog) Failed\n", szTime);
		return;
	}

	MMatchStage* pStage = FindStage(pJob->GetOwnerUID());
	if( pStage == NULL ) return;

	pStage->SetGameLogID(pJob->GetID());
}

void MMatchServer::OnAsyncCreateClan(MAsyncJob* pJobResult)
{
	MAsyncDBJob_CreateClan* pJob = (MAsyncDBJob_CreateClan*)pJobResult;

	MUID uidMaster = pJob->GetMasterUID();
	MMatchObject* pMasterObject = GetObject(uidMaster);
	

	if (pJob->GetResult() != MASYNC_RESULT_SUCCEED) {
		if (IsEnabledObject(pMasterObject))
		{
			RouteResponseToListener(pMasterObject, MC_MATCH_CLAN_RESPONSE_AGREED_CREATE_CLAN, MERR_CLAN_CANNOT_CREATE);
		}
		return;
	}		

	int nNewCLID = pJob->GetNewCLID();

	if ( (pJob->GetDBResult() == false) || (nNewCLID ==0) )
	{
		if (IsEnabledObject(pMasterObject))
		{
			RouteResponseToListener(pMasterObject, MC_MATCH_CLAN_RESPONSE_AGREED_CREATE_CLAN, MERR_CLAN_CANNOT_CREATE);
		}
		return;
	}


	// 마스터의 바운티를 깎는다.
	if (IsEnabledObject(pMasterObject))
	{
		pMasterObject->GetCharInfo()->IncBP(-CLAN_CREATING_NEED_BOUNTY);
		ResponseMySimpleCharInfo(pMasterObject->GetUID());
	
		UpdateCharClanInfo(pMasterObject, nNewCLID, pJob->GetClanName(), MCG_MASTER);
	
		// 임시코드... 잘못된 MMatchObject*가 온다면 체크하여 잡기위함...20090224 by kammir
		if(pMasterObject->GetCharInfo()->m_ClanInfo.GetClanID() >= 9000000)
			LOG(LOG_FILE, "[OnAsyncCreateClan()] %s's ClanID:%d.", pMasterObject->GetAccountName(), pMasterObject->GetCharInfo()->m_ClanInfo.GetClanID());

	}


	MMatchObject* pSponsorObjects[CLAN_SPONSORS_COUNT];
	//_ASSERT(CLAN_SPONSORS_COUNT == 4);

	pSponsorObjects[0] = GetObject(pJob->GetMember1UID());
	pSponsorObjects[1] = GetObject(pJob->GetMember2UID());
	pSponsorObjects[2] = GetObject(pJob->GetMember3UID());
	pSponsorObjects[3] = GetObject(pJob->GetMember4UID());

	for (int i = 0; i < CLAN_SPONSORS_COUNT; i++)
	{
		if (IsEnabledObject(pSponsorObjects[i]))
		{
			UpdateCharClanInfo(pSponsorObjects[i], nNewCLID, pJob->GetClanName(), MCG_MEMBER);
			// 임시코드... 잘못된 MMatchObject*가 온다면 체크하여 잡기위함...20090224 by kammir
			if(pSponsorObjects[i]->GetCharInfo()->m_ClanInfo.GetClanID() >= 9000000)
				LOG(LOG_FILE, "[OnAsyncCreateClan()] %s's ClanID:%d.", pSponsorObjects[i]->GetAccountName(), pSponsorObjects[i]->GetCharInfo()->m_ClanInfo.GetClanID());

			RouteResponseToListener(pSponsorObjects[i], MC_MATCH_RESPONSE_RESULT, MRESULT_CLAN_CREATED);
		}
	}

	if (IsEnabledObject(pMasterObject))
	{
		RouteResponseToListener(pMasterObject, MC_MATCH_CLAN_RESPONSE_AGREED_CREATE_CLAN, MOK);
	}
}

void MMatchServer::OnAsyncExpelClanMember(MAsyncJob* pJobResult)
{
	MAsyncDBJob_ExpelClanMember* pJob = (MAsyncDBJob_ExpelClanMember*)pJobResult;

	MMatchObject* pAdminObject = GetObject(pJob->GetAdminUID());

	if (pJobResult->GetResult() != MASYNC_RESULT_SUCCEED) 
	{
		if (IsEnabledObject(pAdminObject))
		{
			RouteResponseToListener(pAdminObject, MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER, MERR_CLAN_CANNOT_EXPEL_FOR_NO_MEMBER);
		}
		return;
	}		

	int nDBRet = pJob->GetDBResult();
	switch (nDBRet)
	{
	case MMatchDBMgr::ER_NO_MEMBER:
		{
			if (IsEnabledObject(pAdminObject))
			{
				RouteResponseToListener(pAdminObject, MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER, MERR_CLAN_CANNOT_EXPEL_FOR_NO_MEMBER);
			}
			return;
		}
		break;
	case MMatchDBMgr::ER_WRONG_GRADE:
		{
			if (IsEnabledObject(pAdminObject))
			{
				RouteResponseToListener(pAdminObject, MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER, MERR_CLAN_CANNOT_CHANGE_GRADE);
			}
			return;
		}
		break;
	}


	// 만약 당사자가 접속해있으면 클랜탈퇴되었다고 알려줘야한다.
	MMatchObject* pMemberObject = GetPlayerByName(pJob->GetTarMember());
	if (IsEnabledObject(pMemberObject))
	{
		UpdateCharClanInfo(pMemberObject, 0, "", MCG_NONE);
		// 임시코드... 잘못된 MMatchObject*가 온다면 체크하여 잡기위함...20090224 by kammir
		if(pMemberObject->GetCharInfo()->m_ClanInfo.GetClanID() >= 9000000)
			LOG(LOG_FILE, "[OnAsyncExpelClanMember()] %s's ClanID:%d.", pMemberObject->GetAccountName(), pMemberObject->GetCharInfo()->m_ClanInfo.GetClanID());

	}

	if (IsEnabledObject(pAdminObject))
	{
		RouteResponseToListener(pAdminObject, MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER, MOK);
	}
}



void MMatchServer::OnAsyncInsertEvent( MAsyncJob* pJobResult )
{
	if( 0 == pJobResult )
		return;

	MAsyncDBJob_EventLog* pEventJob = 
		reinterpret_cast< MAsyncDBJob_EventLog* >( pJobResult );

	if( pEventJob->GetAnnounce().empty() )
		return;

	if( MASYNC_RESULT_SUCCEED == pJobResult->GetResult() )
	{
		MCommand* pCmd;
		AsyncEventObjVec::const_iterator it, end;
		const AsyncEventObjVec& EventObjList = pEventJob->GetEventObjList();

		end = EventObjList.end();
		for( it = EventObjList.begin(); it != end; ++it )
		{
			if( MUID(0, 0) != it->uidUser )
			{
				pCmd = CreateCommand( MC_MATCH_ANNOUNCE, it->uidUser );
				if( 0 != pCmd )
				{
					pCmd->AddParameter( new MCmdParamUInt(0) );
					pCmd->AddParameter( new MCmdParamStr(pEventJob->GetAnnounce().c_str()) );
					Post( pCmd );
				}
			}
		}
	}
}


void MMatchServer::OnAsyncUpdateIPtoCoutryList( MAsyncJob* pJobResult )
{
	MCommand* pCmd = CreateCommand( MC_LOCAL_UPDATE_IP_TO_COUNTRY, GetUID() );
	if( 0 != pCmd )
		Post( pCmd );
}


void MMatchServer::OnAsyncUpdateBlockCountryCodeList( MAsyncJob* pJobResult )
{
	MCommand* pCmd = CreateCommand( MC_LOCAL_UPDATE_BLOCK_COUTRYCODE, GetUID() );
	if( 0 != pCmd )
		Post( pCmd );
}


void MMatchServer::OnAsyncUpdateCustomIPList( MAsyncJob* pJobResult )
{
	MCommand* pCmd = CreateCommand( MC_LOCAL_UPDATE_CUSTOM_IP, GetUID() );
	if( 0 != pCmd )
		Post( pCmd );
}


void MMatchServer::OnAsyncGetAccountItemList( MAsyncJob* pJobResult )
{
	MAsyncDBJob_GetAccountItemList* pJob = (MAsyncDBJob_GetAccountItemList*)pJobResult;

	if( MASYNC_RESULT_SUCCEED != pJob->GetResult() ) {
		mlog("GetAccountItemList Failed\n");
		return;
	}

	MMatchObject* pObj = GetObject( pJob->GetPlayerUID() );
	if( NULL == pObj ) return;

	if( !pJob->GetExpiredAccountItems().empty() ) {
		ResponseExpiredItemIDList(pObj, pJob->GetExpiredAccountItems());
	}

	const int nAccountItemCount = pJob->GetAccountItemCount();

	if (nAccountItemCount > 0) {
		MAccountItemNode* accountItems = pJob->GetAccountItemList();
		if( NULL == accountItems ) return;

		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_ACCOUNT_ITEMLIST, pObj->GetUID());

		// 갖고 있는 아이템 리스트 전송
		int nCountableAccountItemCount = 0;
		for(int i = 0; i < nAccountItemCount; i++ ) {
			if( accountItems[i].nCount > 0 && accountItems[i].nItemID > 0 ) { 
				nCountableAccountItemCount++; 
			}
		}

		void* pItemArray = MMakeBlobArray(sizeof(MTD_AccountItemNode), nCountableAccountItemCount);		

		int nIndex = 0;
		for (int i = 0; i < nAccountItemCount; i++)
		{
			if( accountItems[i].nItemID == 0 ) continue;
			MTD_AccountItemNode* pItemNode = (MTD_AccountItemNode*)MGetBlobArrayElement(pItemArray, nIndex);			

			if( accountItems[i].nCount > 0 ) {
				Make_MTDAccountItemNode(pItemNode, accountItems[i].nAIID, accountItems[i].nItemID
					, accountItems[i].nRentMinutePeriodRemainder, accountItems[i].nCount);

				nIndex++;

				if( nIndex == nCountableAccountItemCount ) { break;	}
			}			
		}

		pNew->AddParameter(new MCommandParameterBlob(pItemArray, MGetBlobArraySize(pItemArray)));
		MEraseBlobArray(pItemArray);

		PostSafeQueue( pNew );
	}
}


void MMatchServer::OnAsyncBuyQuestItem( MAsyncJob* pJobReslt )
{
	MAsyncDBJob_BuyQuestItem* pJob = (MAsyncDBJob_BuyQuestItem*)pJobReslt;
	if( MASYNC_RESULT_SUCCEED != pJob->GetResult() ){ return; }

	MMatchObject* pPlayer = GetObject( pJob->GetPlayerUID() );
	if( NULL == pPlayer ) {	return; }

	MMatchCharInfo* pCharInfo = pPlayer->GetCharInfo();
	if( NULL == pCharInfo ) { return; }

	// 아이템 거래 카운트 증가. 내부에서 디비 업데이트 결정.
	pCharInfo->GetDBQuestCachingData().IncreaseShopTradeCount(pJob->GetItemCount());
	pCharInfo->m_nBP -= pJob->GetPrice();

	
	MCommand* pNewCmd = CreateCommand( MC_MATCH_RESPONSE_BUY_QUEST_ITEM, pJob->GetPlayerUID() );
	if( 0 == pNewCmd ) {
		mlog( "MMatchServer::OnResponseBuyQuestItem - new Command실패.\n" );
		return;
	}
	
	pNewCmd->AddParameter( new MCmdParamInt(MOK) );
	pNewCmd->AddParameter( new MCmdParamInt(pCharInfo->m_nBP) );
	PostSafeQueue( pNewCmd );

	// 퀘스트 아이템 리스트를 다시 전송함.
	OnRequestCharQuestItemList( pJob->GetPlayerUID() );
}

//////////////////////////////////////////////////////////////////////////////////////////////
// 2009. 6. 3 - Added By Hong KiJu

void MMatchServer::OnAsyncSurvivalModeGameLog( MAsyncJob* pJobResult )
{
	MAsyncDBJob_InsertSurvivalModeGameLog *pJob = (MAsyncDBJob_InsertSurvivalModeGameLog *)pJobResult;

	if( MASYNC_RESULT_SUCCEED != pJob->GetResult() )
	{
		return;
	}
}

void MMatchServer::OnAsyncSurvivalModeGroupRanking(MAsyncJob* pJobResult)
{
	MAsyncDBJob_GetSurvivalModeGroupRanking* pJob = (MAsyncDBJob_GetSurvivalModeGroupRanking*)pJobResult;
	
	if( MASYNC_RESULT_SUCCEED != pJob->GetResult() )
	{
		return;
	}
/*#ifdef _DEBUG
	// 다음은 MAsyncDBJob_GetSurvivalModeGroupRanking Class의 사용 요령입니다.
	// Class에서 더 필요한 부분이 있으면 수정하시면 됩니다.
	for(DWORD dwScenarioID = 1; dwScenarioID <= MAX_SURVIVAL_SCENARIO_COUNT; dwScenarioID++)
	{
		mlog("-------- Scenario ID = %d, Top 100 --------\n", dwScenarioID);

		for(int i = 1; i <= 100; i++)
		{
			RANKINGINFO *pRankingInfo = pJob->GetRankingInfo(dwScenarioID, i);
			
			if( NULL != pRankingInfo )
			{
				mlog("CID = %d, Ranking = %d, Ranking Point = %d\n", 
					pRankingInfo->dwCID, pRankingInfo->dwRanking, pRankingInfo->dwRankingPoint);
			}

		}
		
	}
#endif*/

	//가짜 목록 생성
	/*char sz[256];
	for (DWORD s=0; s<MAX_SURVIVAL_SCENARIO_COUNT; ++s)
	{
		for (int i=0; i<MAX_SURVIVAL_RANKING_LIST; ++i)
		{
			RANKINGINFO *pRankingInfo = pJob->GetRankingInfo(s, i);
			if (pRankingInfo)
			{
				pSurvivalRankInfo->SetRanking(s, i, 
					pRankingInfo->dwRanking, pRankingInfo->szCharName, pRankingInfo->dwRankingPoint);
			}
		}
	}*/

	MSurvivalRankInfo* pSurvivalRankInfo = MMatchServer::GetInstance()->GetQuest()->GetSurvivalRankInfo();
	pSurvivalRankInfo->ClearRanking();

	for (DWORD s=0; s<MAX_SURVIVAL_SCENARIO_COUNT; ++s)
	{
		for (int i=0; i<MAX_SURVIVAL_RANKING_LIST; ++i)
		{
			RANKINGINFO *pRankingInfo = pJob->GetRankingInfo(s, i);
			if (pRankingInfo)
			{
				pSurvivalRankInfo->SetRanking(s, i, 
					pRankingInfo->dwRanking, pRankingInfo->szCharName, pRankingInfo->dwRankingPoint);
			}
		}
	}
}

void MMatchServer::OnAsyncSurvivalModePrivateRanking(MAsyncJob* pJobResult)
{
	MAsyncDBJob_GetSurvivalModePrivateRanking* pJob = (MAsyncDBJob_GetSurvivalModePrivateRanking*)pJobResult;

	if( MASYNC_RESULT_SUCCEED != pJob->GetResult() )
	{
		mlog("MMatchServer::OnAsyncSurvivalModePrivateRanking - 실패! stageUID[%d] playerCID[%d]\n", pJob->GetStageUID(), pJob->GetCID());
		return;
	}
#ifdef _DEBUG
	mlog("MMatchServer::OnAsyncSurvivalModePrivateRanking - Test Log입니다. 성공!\n");

	// 다음은 MAsyncDBJob_GetSurvivalModePrivateRanking Class의 사용 요령입니다.
	// Class에서 더 필요한 부분이 있으면 수정하시면 됩니다.
	mlog("-------- User Ranking Info --------\n");
	mlog("User CID = %d\n", pJob->GetCID());

	for(DWORD dwScenarioID = 1; dwScenarioID < MAX_SURVIVAL_SCENARIO_COUNT + 1; dwScenarioID++)
	{
		RANKINGINFO* pRankingInfo = pJob->GetPrivateRankingInfo(dwScenarioID);

		mlog("Scenario ID = %01d, Ranking = %d, Ranking Point = %d\n", 
			dwScenarioID, pRankingInfo->dwRanking, pRankingInfo->dwRankingPoint);
	}
#endif

	//_ASSERT( pJob->GetScenarioID()-1 < MAX_SURVIVAL_SCENARIO_COUNT );
	RANKINGINFO* pRankingInfo = pJob->GetPrivateRankingInfo( pJob->GetScenarioID() );
	if (pRankingInfo)
	{
		// 플레이어에게 랭킹 정보를 보낸다		
		MCommand* pCmdPrivateRanking = MGetMatchServer()->CreateCommand( MC_SURVIVAL_PRIVATERANKING, MUID(0, 0) );
		if( NULL == pCmdPrivateRanking )
			return;

		pCmdPrivateRanking->AddParameter( new MCommandParameterUInt(pRankingInfo->dwRanking) );
		pCmdPrivateRanking->AddParameter( new MCommandParameterUInt(pRankingInfo->dwRankingPoint) );
		
		RouteToObjInStage(pJob->GetStageUID(), pJob->GetPlayerUID(), pCmdPrivateRanking);
	}
	
}