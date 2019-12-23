#include "stdafx.h"
#include "MBMatchServer.h"
#include "MBMatchAsyncDBJob_NetmarbleLogin.h"
#include "MBMatchAsyncDBJob_NHNLogin.h"
#include "MBMatchAsyncDBJob_GameOnLogin.h"
#include "MSharedCommandTable.h"
#include "MBlobArray.h"
#include "MMatchFormula.h"
#include "MMatchConfig.h"
#include "MAsyncDBJob.h"
#include "MAsyncDBJob_FriendList.h"
#include "MAsyncDBJob_GetLoginInfo.h"
#include "MAsyncDBJob_InsertConnLog.h"
#include "MAsyncDBJob_Event.h"
#include "MBMatchAsyncDBJob_BuyBountyItem.h"
#include "MBMatchAsyncDBJob_SellItem.h"
#include "MBMatchAsyncDBJob_GambleItem.h"
#include "MBMatchAsyncDBJob_GambleQuestItem.h"
#include "MBMatchAsyncDBJob_BringAccountItem.h"
#include "MBMatchAsyncDBJob_BringAccountItemStackable.h"
#include "MBMatchAsyncDBJob_BringBackAccountItem.h"
#include "MBMatchAsyncDBJob_BringBackAccountItemStackable.h"
#include "MBMatchAsyncDBJob_UpdateEquipItem.h"
#include "MBMatchAsyncDBJob_TakeoffItem.h"
#include "RTypes.h"



#ifdef _MONITORING
//#include "MMonitorCommand.h"
#include "MBMatchAsyncJob_GetCpuInfo.h"
#endif

#ifdef _GAMEGUARD
#include "MBMatchGGSvrRef.h"
#define SAFE_DELETE(p)  { if(p) { delete (p); (p)=NULL; } }
#endif


void MBMatchServer::OnProcessAsyncJob( MAsyncJob* pJob )
{
	switch( pJob->GetJobID() )
	{
	case MASYNCJOB_NETMARBLE_KOR_LOGIN :
		{
			OnAsyncNetmarbleLogin( pJob );
		}
		break;

	case MASYNCJOB_NHNLOGIN :
		{
			OnAsyncNHNLogin( pJob );
		}
		break;

	case MASYNCJOB_GAMEONLOGIN :
		{
			OnAsyncGameOnLogin( pJob );
		}
		break;

	case MASYNCJOB_GETACCOUNTCHARINFO:
		{
			OnAsyncGetAccountCharInfo(pJob);
		}
		break;				

	case MASYNCJOB_GETCHARINFO:
		{
			OnAsyncGetCharInfo(pJob);
		}
		break;

	case MASYNCJOB_SETBLOCKHACKING :
		{
			// 특별히 하는 작업은 없음.
		}
		break;

	case MASYNCJOB_RESETACCOUNTHACKINGBLOCK :
		{
			// 특별히 하는 작업은 없음.
		}
		break;

	case MASYNCJOB_UPDATCHARCLANCONTPOINT :
		{
			// 특별히 하는 작업은 없음.
		}
		break;

	case MASYNCJOB_GETSYSTEMINFO:
		{
			OnAsyncGetCpuUsage(pJob);
		}
		break;

	case MASYNCJOB_BUYBOUNTRYITEM :
		{
			OnAsyncBuyBountyItem( pJob );
		}
		break;

	case MASYNCJOB_SELL_ITEM_TO_BOUNTY:
		{
			OnAsyncSelling_Item_ToBounty( pJob );
		}
		break;

	case MASYNCJOB_SELL_SPENDABLEITEM_TO_BOUNTY:
		{
			OnAsyncSelling_SpendableItem_ToBounty( pJob );
		}
		break;

	case MASYNCJOB_GAMBLE_ITEM :
		{
			OnAsyncGambleItem( pJob );
		}
		break;

	case MASYNCJOB_GAMBLE_QUESTITEM :
		{
			OnAsyncGambleQuestItem( pJob );
		}
		break;


	case MASYNCJOB_BRING_ACCOUNTITEM:
		{
			OnAsyncBringAccountItem(pJob);
		}
		break;

	case MASYNCJOB_BRING_ACCOUNTITEM_STACKABLE:
		{
			OnAsyncBringAccountItemStackable(pJob);
		}
		break;

	case MASYNCJOB_BRINGBACK_ACCOUNTITEM :
		{
			OnBringBackAccountItem( pJob );
		}
		break;

	case MASYNCJOB_BRINGBACK_ACCOUNTITEM_STACKABLE :
		{
			OnBringBackAccountItemStackable( pJob );
		}
		break;

	case MASYNCJOB_UPDATECHARLEVEL :
		{
			// 할게 없음.
		}
		break;

	case MASYNCJOB_UPDATEEQUIPITEM :
		{
			OnUpdateEquipItem( pJob );
		}
		break;

	case MASYNCJOB_TAKEOFFITEM :
		{
			OnTakeoffItem( pJob );
		}
		break;

	case MASYNCJOB_UPDATEACCOUNTLASTLOGINTIME : 
		{
			// 할일이 없음.
		}
		break;

	case MASYNCJOB_INSERT_CHAR_BUFF_INFO :
	case MASYNCJOB_UPDATE_CHAR_BUFF_INFO :
		break;

	default :
#ifdef _DEBUG
		mlog( "invalid async job id : %d\n",pJob->GetJobID() );
#endif
		ASSERT( 0 );
		break;
	}
}


void MBMatchServer::OnAsyncNetmarbleLogin(MAsyncJob* pJobInput)
{
	MBMatchAsyncDBJob_NetmarbleLogin* pJob = (MBMatchAsyncDBJob_NetmarbleLogin*)pJobInput;

	if (pJob->GetResult() != MASYNC_RESULT_SUCCEED) 
	{		
		// Notify Message 필요 -> 로그인 관련 - 해결(Login Fail 메세지 이용)
		// Disconnect(pJob->GetCommUID());
		int nErrCode = pJob->GetErrorCode();

		// 0이면 정상, -1이면 Default값, 1~6이면 에러 코드!
		if( nErrCode > 0 && nErrCode < 10 ) {
			LOG(LOG_PROG, "OnAsyncNetmarbleLogin - Can't Make UserID(ID=%s, AID=%u, Code=%d)\n", 
				pJob->GetAccountInfo()->m_szUserID, pJob->GetAID(), nErrCode );
			
		}
			
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(pJob->GetCommUID(), MERR_FAILED_NETMARBLE_KOR_CANT_GET_ACCOUNTINFO);
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

	AddObjectOnMatchLogin(pJob->GetCommUID(), pJob->GetAccountInfo(), pJob->GetAccountPenaltyInfo()
		, pJob->IsFreeLoginIP(), pJob->GetCountryCode3(), pJob->GetChecksumPack());
}

void MBMatchServer::OnAsyncNHNLogin( MAsyncJob* pJob )
{
	if( 0 == pJob ) return;
	
	MBMatchAsyncDBJob_NHNLogin* pNHNJob = static_cast< MBMatchAsyncDBJob_NHNLogin* >( pJob );

	if( MASYNC_RESULT_SUCCEED != pNHNJob->GetResult() )
	{
		if( 0 != pNHNJob->GetAccountInfo()->m_szUserID )
			LOG(LOG_PROG, "NHN USA Login Fail:UserID=%s", pNHNJob->GetAccountInfo()->m_szUserID );		

		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(pNHNJob->GetUID(), MERR_FAILED_NHNUSA_INVALID_AUTHSTR);
		Post(pCmd);	

		pNHNJob->DeleteMemory();
		return;
	}

	
	const MMatchAccountInfo* pAccountInfo = pNHNJob->GetAccountInfo();
	if( 0 == pAccountInfo ) return;

	const MMatchAccountPenaltyInfo* pAccountPenaltyInfo = pNHNJob->GetAccountPenaltyInfo();
	if( 0 == pAccountPenaltyInfo ) return;


	// 중복 로긴 검사.
	MMatchObject* pCopyObj = GetPlayerByAID( pNHNJob->GetAID() );
	if( 0 != pCopyObj )
	{
		// 이전 접속 종료.	
		LOG(LOG_PROG, "Duplicated Login. ID=%s, AID=%u\n", 
			pNHNJob->GetAccountInfo()->m_szUserID, pNHNJob->GetAID() );
		
		// Notify Message 필요 -> 로그인 관련 - 해결(특별한 메세지 필요 없음)
		// 중복 접속에 관한 것은 이전 접속자의 접속을 해지하는 것이므로,
		// 특별한 오류 패킷을 만들지 않는다.
		Disconnect( pCopyObj->GetUID() );
	}

	// 우저 등급을 확인한다.
	if ((MMUG_BLOCKED == pAccountInfo->m_nUGrade) || (MMUG_PENALTY == pAccountInfo->m_nUGrade))
	{
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(pNHNJob->GetUID(), MERR_CLIENT_MMUG_BLOCKED);
		Post(pCmd);	

		pNHNJob->DeleteMemory();
		return;
	}

#ifdef _GAMEGUARD
	if( MGetServerConfig()->IsUseGamegaurd() )
	{
		// ggtest
		MBMatchGameguard* pGameguard = new MBMatchGameguard( GetGlobalClockCount() );
		if( 0 == pGameguard )
		{
			LOG(LOG_PROG,  "GAMEGUARD ERR :MBMatchGameguard memory alloc fail\n" );

			// Notify Message 필요 -> 로그인 관련 - 해결(Login Fail 메세지 이용)
			// Disconnect( pNHNJob->GetUID() );
			// 서버쪽의 문제이므로, 로그인 실패 -> 재시도하라고 알려주는 Message
			MCommand* pCmd = CreateCmdMatchResponseLoginFailed(pNHNJob->GetUID(), MERR_FAILED_LOGIN_RETRY);
			Post(pCmd);	

			pNHNJob->DeleteMemory();
			return;
		}

		if( !pGameguard->Init() )
		{
			// Notify Message 필요 -> 로그인 관련 - 해결(Login Fail 메세지 이용)
			// Disconnect( pNHNJob->GetUID() );
			// 서버쪽의 문제이므로, 로그인 실패 -> 재시도하라고 알려주는 Message
			MCommand* pCmd = CreateCmdMatchResponseLoginFailed(pNHNJob->GetUID(), MERR_FAILED_LOGIN_RETRY);
			Post(pCmd);	

			pNHNJob->DeleteMemory();
			SAFE_DELETE(pGameguard);
			return;
		}

		if( !pGameguard->CreateAuthQuery() )
		{
			LOG(LOG_PROG,  "GAMEGUARD ERR :create new gameguard data fail in nhn login async job. AID(%u), ErrorCode(%u)\n", 
				pAccountInfo->m_nAID, pGameguard->GetLastError() );
			
			// Notify Message 필요 -> 로그인 관련 - 해결(Login Fail 메세지 이용)
			// Disconnect( pNHNJob->GetUID() );
			// 서버쪽의 문제이므로, 로그인 실패 -> 재시도하라고 알려주는 Message
			MCommand* pCmd = CreateCmdMatchResponseLoginFailed(pNHNJob->GetUID(), MERR_FAILED_LOGIN_RETRY);
			Post(pCmd);	

			pNHNJob->DeleteMemory();
			SAFE_DELETE(pGameguard);
			return;
		}

		const GG_AUTH_DATA& AuthData = pGameguard->GetServerAuth()->GetAuthQuery();
		RequestFirstGameguardAuth( pNHNJob->GetUID()
			, AuthData.dwIndex
			, AuthData.dwValue1
			, AuthData.dwValue2
			, AuthData.dwValue3 );

       	if( !m_Security.GetManager(MSI_GAMEGUARD)->AddUserSecurityInfo(pNHNJob->GetUID(), pGameguard) )
		{
			LOG(LOG_PROG,  "GAMEGUARD ERR :add security info fail. AID(%u)\n", pAccountInfo->m_nAID );
			
			// Notify Message 필요 -> 로그인 관련 - 해결(Login Fail 메세지 이용)
			// Disconnect( pNHNJob->GetUID() );
			// 서버쪽의 문제이므로, 로그인 실패 -> 재시도하라고 알려주는 Message
			MCommand* pCmd = CreateCmdMatchResponseLoginFailed(pNHNJob->GetUID(), MERR_FAILED_LOGIN_RETRY);
			Post(pCmd);	

			pNHNJob->DeleteMemory();
			SAFE_DELETE(pGameguard);
			return;
		}
	}
#endif

	AddObjectOnMatchLogin(pNHNJob->GetUID(), pNHNJob->GetAccountInfo(), pNHNJob->GetAccountPenaltyInfo()
		, pNHNJob->IsFreeLoginIP(), pNHNJob->GetCountryCode3(), pNHNJob->GetCheckSumPak());

#ifdef _GAMEGUARD
	if( MGetServerConfig()->IsUseGamegaurd() )
	{
		// 반드시 AddObjectOnMatchLogin함수가 먼저 호출이 되어야 MMatchObject트가 등록이 되어 있다.
		//  이전에 호출을 하면 GetObject(...)에서 NULL을 반환함.
		MMatchObject* pPlayer = GetObject( pNHNJob->GetUID() );
		if( NULL == pPlayer )
		{
			_ASSERT( 0 );

			m_Security.GetManager(MSI_GAMEGUARD)->Delete( pNHNJob->GetUID() );

			// Notify Message 필요 -> 로그인 관련 - 해결(Login Fail 메세지 이용)
			// Disconnect( pNHNJob->GetUID() );
			// 서버쪽의 문제이므로, 로그인 실패 -> 재시도하라고 알려주는 Message
			MCommand* pCmd = CreateCmdMatchResponseLoginFailed(pNHNJob->GetUID(), MERR_FAILED_LOGIN_RETRY);
			Post(pCmd);	

			pNHNJob->DeleteMemory();
			return;
		}

		pPlayer->FirstGameguardReqeustIsSent();
	}
#endif

	if (pNHNJob->GetAccountInfo())
	{
		pNHNJob->DeleteMemory();
	}
}

void MBMatchServer::OnAsyncGameOnLogin( MAsyncJob* pJob )
{
	if( 0 == pJob ) return;
	
	MBMatchAsyncDBJob_GameOnLogin* pGameOnJob = static_cast< MBMatchAsyncDBJob_GameOnLogin* >( pJob );

	if( MASYNC_RESULT_SUCCEED != pGameOnJob->GetResult() )
	{
		mlog( "GameOn Login fail" );
		if( 0 != pGameOnJob->GetAccountInfo()->m_szUserID )
			mlog( " : UserID=%s", pGameOnJob->GetAccountInfo()->m_szUserID );
		mlog( "\n" );
		pGameOnJob->DeleteMemory();
		return;
	}


	const MMatchAccountInfo* pAccountInfo = pGameOnJob->GetAccountInfo();
	if( 0 == pAccountInfo ) return;

	const MMatchAccountPenaltyInfo* pAccountPenaltyInfo = pGameOnJob->GetAccountPenaltyInfo();
	if( 0 == pAccountPenaltyInfo ) return;

	// 중복 로긴 검사.

	MMatchObject* pCopyObj = GetPlayerByAID( pGameOnJob->GetAID() );
	if( 0 != pCopyObj )
	{
		// 이전 접속 종료.	
		mlog( "Duplicated login. AID=%u\n", pGameOnJob->GetAID() );
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(pCopyObj->GetUID(), MERR_MULTIPLE_LOGIN);
		Post(pCmd);	
	}

	// 우저 등급을 확인한다.
	if ((MMUG_BLOCKED == pAccountInfo->m_nUGrade) || (MMUG_PENALTY == pAccountInfo->m_nUGrade))
	{
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(pGameOnJob->GetUID(), MERR_CLIENT_MMUG_BLOCKED);
		Post(pCmd);	

		pGameOnJob->DeleteMemory();

		return;
	}


#ifdef _GAMEGUARD
	if( MGetServerConfig()->IsUseGamegaurd() )
	{
		// ggtest
		MBMatchGameguard* pGameguard = new MBMatchGameguard( GetGlobalClockCount() );
		if( 0 == pGameguard )
		{
			LOG(LOG_PROG,  "GAMEGUARD ERR :MBMatchGameguard memory alloc fail\n" );

			// Notify Message 필요 -> 로그인 관련 - 해결(Login Fail 메세지 이용)
			// Disconnect( pGameOnJob->GetUID() );
			// 서버쪽의 문제이므로, 로그인 실패 -> 재시도하라고 알려주는 Message
			MCommand* pCmd = CreateCmdMatchResponseLoginFailed(pGameOnJob->GetUID(), MERR_FAILED_LOGIN_RETRY);
			Post(pCmd);	

			pGameOnJob->DeleteMemory();	
			return;
		}

		if( !pGameguard->Init() )
		{
			// Notify Message 필요 -> 로그인 관련 - 해결(Login Fail 메세지 이용)
			// Disconnect( pGameOnJob->GetUID() );
			// 서버쪽의 문제이므로, 로그인 실패 -> 재시도하라고 알려주는 Message
			MCommand* pCmd = CreateCmdMatchResponseLoginFailed(pGameOnJob->GetUID(), MERR_FAILED_LOGIN_RETRY);
			Post(pCmd);	

			pGameOnJob->DeleteMemory();	
			SAFE_DELETE(pGameguard);
			return;
		}

		if( !pGameguard->CreateAuthQuery() )
		{
			LOG(LOG_PROG,  "GAMEGUARD ERR :create new gameguard data fail in nhn login async job. AID(%u), ErrorCode(%u)\n", 
				pAccountInfo->m_nAID, pGameguard->GetLastError() );
			
			// Notify Message 필요 -> 로그인 관련 - 해결(Login Fail 메세지 이용)
			// Disconnect( pGameOnJob->GetUID() );
			// 서버쪽의 문제이므로, 로그인 실패 -> 재시도하라고 알려주는 Message
			MCommand* pCmd = CreateCmdMatchResponseLoginFailed(pGameOnJob->GetUID(), MERR_FAILED_LOGIN_RETRY);
			Post(pCmd);	

			pGameOnJob->DeleteMemory();
			SAFE_DELETE(pGameguard);
			return;
		}

		const GG_AUTH_DATA& AuthData = pGameguard->GetServerAuth()->GetAuthQuery();
		RequestFirstGameguardAuth( pGameOnJob->GetUID()
			, AuthData.dwIndex
			, AuthData.dwValue1
			, AuthData.dwValue2
			, AuthData.dwValue3 );

       	if( !m_Security.GetManager(MSI_GAMEGUARD)->AddUserSecurityInfo(pGameOnJob->GetUID(), pGameguard) )
		{
			LOG(LOG_PROG,  "GAMEGUARD ERR :add security info fail. AID(%u)\n", pAccountInfo->m_nAID );
			
			// Notify Message 필요 -> 로그인 관련(보류 - GameOn)
			// Disconnect( pGameOnJob->GetUID() );
			// 서버쪽의 문제이므로, 로그인 실패 -> 재시도하라고 알려주는 Message
			MCommand* pCmd = CreateCmdMatchResponseLoginFailed(pGameOnJob->GetUID(), MERR_FAILED_LOGIN_RETRY);
			Post(pCmd);	

			pGameOnJob->DeleteMemory();
			SAFE_DELETE(pGameguard);
			return;
		}
	}
#endif


	AddObjectOnMatchLogin(pGameOnJob->GetUID(), pGameOnJob->GetAccountInfo(), pGameOnJob->GetAccountPenaltyInfo()
		, pGameOnJob->IsFreeLoginIP(), pGameOnJob->GetCountryCode3(), pGameOnJob->GetCheckSumPak());

#ifdef _GAMEGUARD
	if( MGetServerConfig()->IsUseGamegaurd() )
	{
		// 반드시 AddObjectOnMatchLogin함수가 먼저 호출이 되어야 MMatchObject트가 등록이 되어 있다.
		//  이전에 호출을 하면 GetObject(...)에서 NULL을 반환함.
		MMatchObject* pPlayer = GetObject( pGameOnJob->GetUID() );
		if( NULL == pPlayer )
		{
			_ASSERT( 0 );

			m_Security.GetManager(MSI_GAMEGUARD)->Delete( pGameOnJob->GetUID() );

			// Notify Message 필요 -> 로그인 관련 - 해결(Login Fail 메세지 이용)
			// Disconnect( pGameOnJob->GetUID() );
			// 서버쪽의 문제이므로, 로그인 실패 -> 재시도하라고 알려주는 Message
			MCommand* pCmd = CreateCmdMatchResponseLoginFailed(pGameOnJob->GetUID(), MERR_FAILED_LOGIN_RETRY);
			Post(pCmd);	

			pGameOnJob->DeleteMemory();
			return;
		}

		pPlayer->FirstGameguardReqeustIsSent();
	}
#endif

}


void MBMatchServer::OnAsyncGetAccountCharInfo(MAsyncJob* pJobResult)
{
	MAsyncDBJob_GetAccountCharInfo* pJob = (MAsyncDBJob_GetAccountCharInfo*)pJobResult;

	if (pJob->GetResult() != MASYNC_RESULT_SUCCEED) {
		char szTime[128]="";
		_strtime(szTime);

		mlog("[%s] Async DB Query(ResponseAccountCharInfo) Failed\n", szTime);
		return;
	}		

	MMatchObject* pObj = GetObject(pJob->GetUID());
	if (pObj == NULL) return;

	MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_ACCOUNT_CHARINFO, MUID(0,0));
	pNewCmd->AddParameter(new MCommandParameterChar((char)pJob->GetCharNum()));

	void* pCharArray = MMakeBlobArray(sizeof(MTD_CharInfo), 1);

	MTD_CharInfo* pTransCharInfo = (MTD_CharInfo*)MGetBlobArrayElement(pCharArray, 0);
	memcpy(pTransCharInfo, pJob->GetCharInfo(), sizeof(MTD_CharInfo));

	pNewCmd->AddParameter(new MCommandParameterBlob(pCharArray, MGetBlobArraySize(pCharArray)));
	MEraseBlobArray(pCharArray);

	RouteToListener(pObj, pNewCmd);
}


void MBMatchServer::OnAsyncGetCharInfo(MAsyncJob* pJobResult)
{
	MAsyncDBJob_GetCharInfo* pJob = (MAsyncDBJob_GetCharInfo*)pJobResult;

	if (pJob->GetResult() != MASYNC_RESULT_SUCCEED) {
		mlog("DB Query(OnAsyncGetCharInfo > GetCharInfoByAID) Failed\n");
		return;
	}

	MMatchObject* pObj = GetObject(pJob->GetUID());
	if (pObj == NULL) return;

	if (pObj->GetCharInfo()) {
		// 이전에 캐릭이 선택되어 있었다면 캐릭끝날때 로그 남긴다
		if (pObj->GetCharInfo()->m_nCID != 0) {
			CharFinalize(pObj->GetUID());		// 캐릭끝날때 디비 로그 등 처리
		}

		pObj->FreeCharInfo();
		pObj->FreeDuelTournamentInfo();
		pObj->FreeFriendInfo();

		pObj->FreeCharBuff();
	}

	if (pJob->GetCharInfo() == NULL)
	{
		mlog("pJob->GetCharInfo() IS NULL\n");
		return;
	}

	pObj->SetCharInfo(pJob->GetCharInfo());		// Save Async Result 
//	pObj->SetFriendInfo(pJob->GetFriendInfo());	// Save Async Result

	if (CharInitialize(pJob->GetUID()) == false)
	{
		mlog("OnAsyncGetCharInfo > CharInitialize failed");
		return;
	}

	//--------------------------------------------------------------------------------
	SendDuelTournamentCharInfoToPlayer(pJob->GetUID());
	SendDuelTournamentPreviousCharInfoToPlayer(pJob->GetUID());
	//--------------------------------------------------------------------------------

	
	MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_SELECT_CHAR, MUID(0,0));
	pNewCmd->AddParameter(new MCommandParameterInt(MOK));		// result

	// 내 케릭터의 정보
	void* pCharInfoArray = MMakeBlobArray(sizeof(MTD_CharInfo), 1);
	MTD_CharInfo* pTransCharInfo = (MTD_CharInfo*)MGetBlobArrayElement(pCharInfoArray, 0);
	CopyCharInfoForTrans(pTransCharInfo, pJob->GetCharInfo(), pObj);
	pNewCmd->AddParameter(new MCommandParameterBlob(pCharInfoArray, MGetBlobArraySize(pCharInfoArray)));

	//버프정보임시주석 
	/*// 내 케릭터의 버프 정보
	void* pCharBuffInfoArray = MMakeBlobArray(sizeof(MTD_CharBuffInfo), 1);
	MTD_CharBuffInfo* pTransCharBuffInfo = (MTD_CharBuffInfo*)MGetBlobArrayElement(pCharBuffInfoArray, 0);
	CopyCharBuffInfoForTrans(pTransCharBuffInfo, pJob->GetCharInfo(), pObj);
	pNewCmd->AddParameter(new MCommandParameterBlob(pCharBuffInfoArray, MGetBlobArraySize(pCharBuffInfoArray)));
	*/

	// 내 캐릭터의 추가 정보
	void* pCharExtraInfoArray = MMakeBlobArray(sizeof(MTD_MyExtraCharInfo), 1);
	MTD_MyExtraCharInfo* pMyExtraInfo = (MTD_MyExtraCharInfo*)MGetBlobArrayElement(pCharExtraInfoArray , 0);
	int nPercent = MMatchFormula::GetLevelPercent(pTransCharInfo->nXP, (int)pTransCharInfo->nLevel);
	pMyExtraInfo->nLevelPercent = (char)nPercent;
	pNewCmd->AddParameter(new MCommandParameterBlob(pCharExtraInfoArray , MGetBlobArraySize(pCharExtraInfoArray )));

	MEraseBlobArray(pCharInfoArray);
	MEraseBlobArray(pCharExtraInfoArray);
	//버프정보임시주석 MEraseBlobArray(pCharBuffInfoArray);

	RouteToListener(pObj, pNewCmd);	
}

// CPU 사용량을 조사한다.
void MBMatchServer::OnAsyncGetCpuUsage(MAsyncJob* pJobResult)
{
#ifdef _MONITORING
	MBMatchAsyncJob_GetCpuInfo* pJob = (MBMatchAsyncJob_GetCpuInfo*)pJobResult;

	UINT uiCpuUsage = pJob->GetCpuUsage();
	
//	m_Monitor.OnPostAsyncCpuUsage(pJob->GetProtocolID(), pJob->GetTaskName(), uiCpuUsage);

	pJob->SetResult(MASYNC_RESULT_SUCCEED);
#endif
}

void MBMatchServer::OnAsyncBuyBountyItem( MAsyncJob* pJobResult )
{
	MBMatchAsyncDBJob_BuyBountyItem* pJob = (MBMatchAsyncDBJob_BuyBountyItem*)(pJobResult);

	if( MASYNC_RESULT_SUCCEED != pJob->GetResult() ) {
		MCommand* pNew = CreateCommand( MC_MATCH_RESPONSE_BUY_ITEM, pJob->GetPlayerUID() );
		pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_BUY_ITEM));
		PostSafeQueue( pNew );

		return;
	}

	MMatchObject* pObj = GetObject( pJob->GetPlayerUID() );
	if( NULL == pObj ) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if( NULL == pCharInfo ) return;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 아이템 지급

	const DWORD			dwPrice			= pJob->GetPrice();
	const WORD			wRentHourPeriod	= pJob->GetRentHourPeriod();
	const bool			bIsGambleItem	= pJob->IsGambleItem();
	const DWORD			dwNewCIID		= pJob->GetNewCIID();
	const unsigned int	nItemID			= pJob->GetItemID();
	const int			nItemCount		= pJob->GetItemCount();

	WORD wRentMinPeriod = RENT_PERIOD_UNLIMITED;
	if( RENT_PERIOD_UNLIMITED != wRentHourPeriod ) {
		wRentMinPeriod = (wRentHourPeriod * 60) - 1; // 상점에서 사고 있는 사이에도 시간이 지나기 때문에.
	}

	if( !bIsGambleItem ) 
	{
		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
		if (pItemDesc == NULL) return;

		MMatchItem* pItem = pCharInfo->m_ItemList.GetItemByCIID(dwNewCIID);
		if( pItem == NULL ) 
		{
			pCharInfo->m_ItemList.CreateItem( MMatchItemMap::UseUID(), dwNewCIID, nItemID, 
				RENT_PERIOD_UNLIMITED != wRentHourPeriod, wRentMinPeriod, wRentHourPeriod, nItemCount );
		}
		else
		{
			if( pItem->GetDescID() == nItemID && pItemDesc->IsSpendableItem())
			{
				pItem->IncreaseCount(nItemCount);
			}
			else
			{
				mlog("OnAsyncBuyBountyItem - FAILED (ItemID(%d, %d), OutCIID(%d))\n", nItemID, pItem->GetDescID(), dwNewCIID);
				_ASSERT(0);
				return;
			}
		}		
	}
	else {
		const MMatchCharGambleItem *pGItem = pCharInfo->m_GambleItemManager.GetGambleItemByCIID(dwNewCIID);
		if( pGItem == NULL ) {
 			pCharInfo->m_GambleItemManager.AddGambleItem( MMatchItemMap::UseUID(), dwNewCIID, nItemID, nItemCount );
		} else {
			pCharInfo->m_GambleItemManager.SetGambleItemCount(dwNewCIID, pGItem->GetItemCount() + nItemCount);

		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// 오브젝트에 바운티 깎는다.
	pCharInfo->m_nBP -= dwPrice;
	
	UpdateCharDBCachingData(pObj);		///< XP, BP, KillCount, DeathCount 캐슁 업데이트
	UpdateCharItemDBCachingData(pObj);	///< Character Item에서 업데이트가 필요한 것들 업데이트

	MCommand* pNew = CreateCommand( MC_MATCH_RESPONSE_BUY_ITEM, pObj->GetUID() );
	pNew->AddParameter(new MCmdParamInt(MOK));
	PostSafeQueue( pNew );

	ResponseCharacterItemList( pObj->GetUID() );
}

void MBMatchServer::OnAsyncSelling_Item_ToBounty( MAsyncJob* pJobResult )
{
	MBMatchAsyncDBJob_SellItem* pJob = (MBMatchAsyncDBJob_SellItem*)pJobResult;
	if( MASYNC_RESULT_SUCCEED != pJob->GetResult() ) 
	{
		MCommand* pNew = CreateCommand( MC_MATCH_RESPONSE_SELL_ITEM, pJob->GetPlayerUID() );
		pNew->AddParameter( new MCmdParamInt(MERR_CANNOT_SELL_ITEM) );
		PostSafeQueue( pNew );		
		return;
	}

	MMatchObject* pObj = GetObject( pJob->GetPlayerUID() );
	if( NULL == pObj ) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if( NULL == pCharInfo ) return;

	MUID& uidPlayer		= pJob->GetPlayerUID();
	MUID& uidItem		= pJob->GetItemUID();

	
	MMatchItem*	pCheckItem = pCharInfo->m_ItemList.GetItem(uidItem);
	if( NULL != pCheckItem )
	{	
		// 만약 비동기 작업 중에 착용했다면... 해제시킨다.
		MMatchCharItemParts parts;
		if( pCharInfo->m_EquipedItem.IsEquipedItem(uidItem, parts) ) {
			OnRequestTakeoffItem( uidPlayer, parts );
		}

		pObj->GetCharInfo()->m_ItemList.RemoveItem(uidItem);
		pObj->GetCharInfo()->m_nBP += pJob->GetSellPrice();

		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, uidPlayer);
		pNew->AddParameter(new MCmdParamInt(MOK));
		PostSafeQueue( pNew );

		ResponseCharacterItemList(uidPlayer);
	}


}

void MBMatchServer::OnAsyncSelling_SpendableItem_ToBounty( MAsyncJob* pJobResult )
{
	MBMatchAsyncDBJob_SellItem* pJob = (MBMatchAsyncDBJob_SellItem*)pJobResult;
	if( MASYNC_RESULT_SUCCEED != pJob->GetResult() ) 
	{
		MCommand* pNew = CreateCommand( MC_MATCH_RESPONSE_SELL_ITEM, pJob->GetPlayerUID() );
		pNew->AddParameter( new MCmdParamInt(MERR_CANNOT_SELL_ITEM) );
		PostSafeQueue( pNew );		
		return;
	}

	MMatchObject* pObj = GetObject( pJob->GetPlayerUID() );
	if( NULL == pObj ) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if( NULL == pCharInfo ) return;

	MUID& uidPlayer		= pJob->GetPlayerUID();
	MUID& uidItem		= pJob->GetItemUID();

	MMatchItem*	pCheckItem = pCharInfo->m_ItemList.GetItem(uidItem);
	if( NULL != pCheckItem )
	{	
		// 만약 비동기 작업 중에 착용했다면... 해제시킨다.
		MMatchCharItemParts parts;
		if( pCharInfo->m_EquipedItem.IsEquipedItem(uidItem, parts) ) {
			OnRequestTakeoffItem( uidPlayer, parts );
		}

		int nSellItemCount	= pJob->GetSellItemCount();
		int nItemTotalCount = pObj->GetCharInfo()->m_ItemList.GetItemCount(uidItem);

		if( nItemTotalCount == nSellItemCount ) {
			pObj->GetCharInfo()->m_ItemList.RemoveItem(uidItem);
		} 
		else if( nItemTotalCount > nSellItemCount ) {	
			MMatchItem *pItem = pObj->GetCharInfo()->m_ItemList.GetItem(uidItem);
			if( pItem == NULL )	return;
			pItem->DecreaseCount(nSellItemCount);
		} 
		else { 
			return; 
		}

		pObj->GetCharInfo()->m_nBP += pJob->GetSellPrice();

		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, uidPlayer);
		pNew->AddParameter(new MCmdParamInt(MOK));
		PostSafeQueue( pNew );

		ResponseCharacterItemList(uidPlayer);
	}

	const MMatchCharGambleItem* pCharGItem = pObj->GetCharInfo()->m_GambleItemManager.GetGambleItemByUID( uidItem );
	if( pCharGItem != NULL ) 
	{
		int nSellItemCount	= pJob->GetSellItemCount();
		int nItemTotalCount = pCharGItem->GetItemCount();

		if( nItemTotalCount == nSellItemCount ) {	
			pObj->GetCharInfo()->m_GambleItemManager.DeleteGambleItem( uidItem );
		} 
		else if( nItemTotalCount > nSellItemCount ) {
			pObj->GetCharInfo()->m_GambleItemManager.SetGambleItemCount( uidItem,  nItemTotalCount - nSellItemCount);
		} 
		else { 
			return; 
		}

		pObj->GetCharInfo()->m_nBP += pJob->GetSellPrice();

		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, uidPlayer);
		pNew->AddParameter(new MCmdParamInt(MOK));
		PostSafeQueue( pNew );

		ResponseCharacterItemList(uidPlayer);
	}
}

void MBMatchServer::OnAsyncGambleItem( MAsyncJob* pJobResult )
{
	MBMatchAsyncDBJob_GambleItem* pJob = (MBMatchAsyncDBJob_GambleItem*)pJobResult;
	if( MASYNC_RESULT_SUCCEED != pJob->GetResult() ) return;

	MMatchObject* pObj = GetObject( pJob->GetPlayerUID() );
	if( NULL == pObj ) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if( NULL == pCharInfo ) return;


	WORD		wRentMinPeriod	= RENT_PERIOD_UNLIMITED;
	const MUID& uidPlayer		= pJob->GetPlayerUID();
	const MUID& uidGItem		= pJob->GetGItemUID();
	const DWORD dwCIID			= pJob->GetRewardItemCIID();
	const DWORD dwRIID			= pJob->GetRewardItemID();
	const DWORD dwItemCnt		= pJob->GetRewardItemCnt();
	const WORD	wRentHourPeriod = pJob->GetRentHourPeriod();
    
	if( RENT_PERIOD_UNLIMITED != wRentHourPeriod )
		wRentMinPeriod = (wRentHourPeriod * 60) - 1; // 상점에서 사고 있는 사이에도 시간이 지나기 때문에.

	MMatchItem* pItem = pCharInfo->m_ItemList.GetItemByCIID(dwCIID);
	if( pItem == NULL ) 
	{
		pCharInfo->m_ItemList.CreateItem( MMatchItemMap::UseUID(), dwCIID, dwRIID, 
			RENT_PERIOD_UNLIMITED != wRentHourPeriod, wRentMinPeriod, wRentHourPeriod, dwItemCnt );
	}
	else
	{
		MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(dwRIID);
		if( pItem->GetDescID() == dwRIID && pDesc->IsSpendableItem())
		{
			pItem->IncreaseCount(dwItemCnt);
		}
		else
		{
			mlog("OnAsyncGambleItem - FAILED (ItemID(%d, %d), OutCIID(%d))\n", dwRIID, pItem->GetDescID(), dwCIID);
			_ASSERT(0);
			return;
		}
	}	

	const MMatchCharGambleItem* pGambleItem = pCharInfo->m_GambleItemManager.GetGambleItemByUID(uidGItem);
	if (pGambleItem == NULL) return;

	if( pGambleItem->GetItemCount() > 1 ) {
		const MMatchCharGambleItem*	pGItem = pCharInfo->m_GambleItemManager.GetGambleItemByUID( uidGItem );
		pCharInfo->m_GambleItemManager.SetGambleItemCount(pGItem->GetUID(), pGItem->GetItemCount() - 1);
	} else {
		pCharInfo->m_GambleItemManager.DeleteGambleItem( uidGItem );
	}

	// 보상 아이템의 내용을 클라이언트에 전송(아이템 아이디, 기간(분단위))
	MCommand* pNew = CreateCommand( MC_MATCH_RESPONSE_GAMBLE, uidPlayer );
	pNew->AddParameter( new MCmdParamUInt(dwRIID) );
	pNew->AddParameter( new MCmdParamUInt(dwItemCnt) );	
	pNew->AddParameter( new MCmdParamUInt(wRentHourPeriod * 60) );

	PostSafeQueue(pNew);

	ResponseCharacterItemList( uidPlayer );
}


void MBMatchServer::OnAsyncGambleQuestItem( MAsyncJob* pJobResult )
{
	MBMatchAsyncDBJob_GambleQuestItem* pJob = (MBMatchAsyncDBJob_GambleQuestItem*)pJobResult;
	if( MASYNC_RESULT_SUCCEED != pJob->GetResult() ) return;

	MMatchObject* pObj = GetObject( pJob->GetPlayerUID() );
	if( NULL == pObj ) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if( NULL == pCharInfo ) return;


	const MUID& uidPlayer		= pJob->GetPlayerUID();
	const MUID& uidGItem		= pJob->GetGItemUID();
	const DWORD dwCIID			= pJob->GetCIID();
	const DWORD dwRIID			= pJob->GetRIID();


	MQuestItemMap::iterator itMyQItem = pCharInfo->m_QuestItemList.find( dwRIID );
	if( pCharInfo->m_QuestItemList.end() != itMyQItem ) {
		const int nOver = itMyQItem->second->Increase( 1 );				//< 기존에 가지고 있던 퀘스트 아이템. 수량만 증가 시켜주면 됨.
	} else {
		pCharInfo->m_QuestItemList.CreateQuestItem( dwRIID, 1 );		///< 처음 획득한 퀘스트 아이템. 새로 추가시켜 줘야 함.
	}

	const MMatchCharGambleItem* pGambleItem = pCharInfo->m_GambleItemManager.GetGambleItemByUID(uidGItem);
	if (pGambleItem == NULL) return;

	if( pGambleItem->GetItemCount() > 1 ) {
		const MMatchCharGambleItem*	pGItem = pCharInfo->m_GambleItemManager.GetGambleItemByUID( uidGItem );
		pCharInfo->m_GambleItemManager.SetGambleItemCount(pGItem->GetUID(), pGItem->GetItemCount() - 1);		
	} else {
		pCharInfo->m_GambleItemManager.DeleteGambleItem( uidGItem );
	}

	// 보상 아이템의 내용을 클라이언트에 전송(아이템 아이디, 기간(분단위))
	MCommand* pNew = CreateCommand( MC_MATCH_RESPONSE_GAMBLE, uidPlayer );
	pNew->AddParameter( new MCmdParamUInt(dwRIID) );
	pNew->AddParameter( new MCmdParamUInt(0) );
	PostSafeQueue(pNew);

	ResponseCharacterItemList( uidPlayer );
	OnRequestCharQuestItemList( uidPlayer );
}

void MBMatchServer::OnAsyncBringAccountItem(MAsyncJob* pJobResult)
{
	MBMatchAsyncDBJob_BringAccountItem* pJob = (MBMatchAsyncDBJob_BringAccountItem*)pJobResult;

	if (pJob->GetResult() != MASYNC_RESULT_SUCCEED) 
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_ACCOUNTITEM, pJob->GetUID());
		pNew->AddParameter(new MCmdParamInt(MERR_UNKNOWN));
		PostSafeQueue( pNew );
	}

	MMatchObject* pObj = GetObject(pJob->GetUID());
	if (!IsEnabledObject(pObj)) return;

	unsigned long int noutCIID			= pJob->GetOutCIID();
	unsigned long int noutItemID		= pJob->GetOutItemID();
	bool boutIsRentItem					= pJob->GetOutIsRentItem();
	int noutRentMinutePeriodRemainder	= pJob->GetOutRentMinutePeriodRemainder();
	/// ZERONIS : ADD const - down
	const DWORD woutRentHourPeriod		= pJob->GetOutRentHourPeriod();

	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(noutItemID);
	if( pItemDesc == NULL ) return;

	pObj->GetCharInfo()->m_ItemList.CreateItem(MMatchItemMap::UseUID(), noutCIID, noutItemID, 
		boutIsRentItem, noutRentMinutePeriodRemainder, woutRentHourPeriod);

	ResponseCharacterItemList(pJob->GetUID());

	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_ACCOUNTITEM, pJob->GetUID());
	pNew->AddParameter(new MCmdParamInt(MOK));
	PostSafeQueue( pNew );
}

void MBMatchServer::OnAsyncBringAccountItemStackable(MAsyncJob* pJobResult)
{
	MBMatchAsyncDBJob_BringAccountItemStackable* pJob = (MBMatchAsyncDBJob_BringAccountItemStackable*)pJobResult;

	MMatchObject* pObj = GetObject(pJob->GetUID());
	if (!IsEnabledObject(pObj)) return;

	if (pJob->GetResult() != MASYNC_RESULT_SUCCEED) 
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_ACCOUNTITEM, pJob->GetUID());
		pNew->AddParameter(new MCmdParamInt(MERR_UNKNOWN));
		PostSafeQueue( pNew );
	}

	unsigned long int noutCIID			= pJob->GetOutCIID();
	unsigned long int noutItemID		= pJob->GetOutItemID();
	unsigned int noutItemCnt			= pJob->GetOutItemCnt();
	bool boutIsRentItem					= pJob->GetOutIsRentItem();
	int noutRentMinutePeriodRemainder	= pJob->GetOutRentMinutePeriodRemainder();
	/// ZERONIS : ADD const 
	const DWORD woutRentHourPeriod		= pJob->GetOutRentHourPeriod();

	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(noutItemID);
	if( pItemDesc )
	{
		MMatchItem *pItem = pObj->GetCharInfo()->m_ItemList.GetItemByCIID(noutCIID);
		if( pItem == NULL ) 
		{
			pObj->GetCharInfo()->m_ItemList.CreateItem(MMatchItemMap::UseUID(), noutCIID, noutItemID, 
				boutIsRentItem, noutRentMinutePeriodRemainder, woutRentHourPeriod, noutItemCnt);
		} 
		else 
		{
			if( pItem->GetDescID() != noutItemID || !pItemDesc->IsSpendableItem() ) 
			{
				mlog("OnAsyncBringAccountItemStackable - FAILED (ItemID(%d, %d), OutCIID(%d))\n", noutItemID, pItem->GetDescID(), noutCIID);
				return;
			}

			pItem->IncreaseCount(noutItemCnt);
		}
	}
	else
	{
		const MMatchGambleItem* pGItemDesc = GetGambleMachine().GetGambleItemByGambleItemID(noutItemID);
		if( pGItemDesc == NULL ) return;

		const MMatchCharGambleItem* pItem = pObj->GetCharInfo()->m_GambleItemManager.GetGambleItemByCIID(noutCIID);
		if( pItem == NULL ) pObj->GetCharInfo()->m_GambleItemManager.AddGambleItem(MMatchItemMap::UseUID(), noutCIID, noutItemID, noutItemCnt);
		else				pObj->GetCharInfo()->m_GambleItemManager.SetGambleItemCount(noutCIID, pItem->GetItemCount() + noutItemCnt);
	}

	ResponseCharacterItemList(pJob->GetUID());

	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_ACCOUNTITEM, pJob->GetUID());
	pNew->AddParameter(new MCmdParamInt(MOK));
	PostSafeQueue( pNew );	
}

void MBMatchServer::OnBringBackAccountItem( MAsyncJob* pJobResult )
{
	MBMatchAsyncDBJob_BringBackAccountItem* pJob = (MBMatchAsyncDBJob_BringBackAccountItem*)pJobResult;

	if( MASYNC_RESULT_SUCCEED != pJob->GetResult() ) 
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, pJob->GetOwnerUID() );
		pNew->AddParameter(new MCmdParamInt(MERR_BRING_BACK_ACCOUNTITEM));
		PostSafeQueue( pNew );
		return;
	}

	MMatchObject* pObj = GetObject( pJob->GetOwnerUID() );
	if( NULL == pObj ) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if( NULL == pCharInfo ) return;

	const MUID& uidPlayer	= pJob->GetOwnerUID();
	const MUID& uidItem		= pJob->GetItemUID();

	MMatchCharItemParts parts;
	if( pCharInfo->m_EquipedItem.IsEquipedItem(uidItem, parts) ) {
		OnRequestTakeoffItem( uidPlayer, parts );
	}

	pCharInfo->m_ItemList.RemoveItem(uidItem);

	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, uidPlayer);
	pNew->AddParameter(new MCmdParamInt(MOK));
	PostSafeQueue( pNew );

	ResponseCharacterItemList(uidPlayer);
}

void MBMatchServer::OnBringBackAccountItemStackable( MAsyncJob* pJobResult )
{
	MBMatchAsyncDBJob_BringBackAccountItemStackable* pJob = (MBMatchAsyncDBJob_BringBackAccountItemStackable*)pJobResult;

	if( MASYNC_RESULT_SUCCEED != pJob->GetResult() ) 
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, pJob->GetOwnerUID());
		pNew->AddParameter(new MCmdParamInt(MERR_BRING_BACK_ACCOUNTITEM));
		PostSafeQueue( pNew );
		return;
	}

	MMatchObject* pObj = GetObject( pJob->GetOwnerUID() );
	if( NULL == pObj ) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if( NULL == pCharInfo )	return;

	const MUID& uidPlayer	= pJob->GetOwnerUID();
	const MUID& uidItem		= pJob->GetItemUID();

	MMatchItem *pItem = pCharInfo->m_ItemList.GetItem(uidItem);
	if( pItem )
	{
		unsigned int nRemainCnt = pItem->GetItemCount() - pJob->GetItemCnt();
		if( nRemainCnt > 0 )
		{
			pItem->SetItemCount(nRemainCnt);
		}
		else 
		{
			MMatchCharItemParts parts;
			if( pCharInfo->m_EquipedItem.IsEquipedItem(uidItem, parts) ) {
				OnRequestTakeoffItem( uidPlayer, parts );
			}
			pCharInfo->m_ItemList.RemoveItem(uidItem);			
		}
	}
	else
	{
		const MMatchCharGambleItem* pGambleItem = pCharInfo->m_GambleItemManager.GetGambleItemByUID( uidItem );
		if( NULL == pGambleItem ) return;

		int nRemainCnt = pGambleItem->GetItemCount() - pJob->GetItemCnt();
		if( nRemainCnt > 0) pCharInfo->m_GambleItemManager.SetGambleItemCount( uidItem, nRemainCnt );
		else				pCharInfo->m_GambleItemManager.DeleteGambleItem( uidItem );
	}

	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, uidPlayer);
	pNew->AddParameter(new MCmdParamInt(MOK));
	PostSafeQueue( pNew );

	ResponseCharacterItemList(uidPlayer);
}

void MBMatchServer::OnUpdateEquipItem( MAsyncJob* pJobResult )
{
	//MBMatchAsyncDBJob_UpdateEquipItem* pJob = (MBMatchAsyncDBJob_UpdateEquipItem*)pJobResult;

	//int nResult = MOK;

	//if( MASYNC_RESULT_SUCCEED != pJob->GetResult() || !pJob->GetRet() ) 
	//{
	//	MCommand* pNew = CreateCommand( MC_MATCH_RESPONSE_EQUIP_ITEM, pJob->GetPlayerUID() );
	//	pNew->AddParameter(new MCommandParameterInt(MERR_CANNOT_EQUIP_ITEM));
	//	PostSafeQueue( pNew );

	//	return;
	//}

	//MMatchObject* pObj = GetObject( pJob->GetPlayerUID() );
	//if( NULL == pObj )
	//{
	//	return;
	//}

	//MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	//if( NULL == pCharInfo )
	//{
	//	return;
	//}

	//const MUID&					uidPlayer	= pJob->GetPlayerUID();
	//const MMatchCharItemParts	parts		= pJob->GetParts();
	//MMatchItem*					pItem		= pCharInfo->m_ItemList.GetItem( pJob->GetItemUID() );

	//MCommand* pNew = CreateCommand( MC_MATCH_RESPONSE_EQUIP_ITEM, uidPlayer );
	//pNew->AddParameter( new MCommandParameterInt(MOK) );
	//PostSafeQueue( pNew );

	//pCharInfo->m_EquipedItem.SetItem(parts, pJob->GetItemUID(), pItem);

	//ResponseCharacterItemList( uidPlayer );

	//if( FindStage(pJob->GetStageUID()) )
	//{
	//	// 만약 스테이지 안이면 스테이지에 알려서 유저의 Look을 업데이트 해줘야 한다.
	//	if( FindStage(pJob->GetStageUID()) )
	//	{
	//		MCommand* pEquipInfo = CreateCommand( MC_MATCH_ROUTE_UPDATE_STAGE_EQUIP_LOOK, MUID(0, 0) );
	//		pEquipInfo->AddParameter( new MCmdParamUID(uidPlayer) );
	//		pEquipInfo->AddParameter( new MCmdParamInt(pJob->GetParts()) );
	//		pEquipInfo->AddParameter( new MCmdParamInt(0) );
	//		RouteToStage( pJob->GetStageUID(), pEquipInfo );
	//	}
	//}
}


void MBMatchServer::OnTakeoffItem( MAsyncJob* pJobResult )
{
	//MBMatchAsyncDBJob_TakeoffItem* pJob = (MBMatchAsyncDBJob_TakeoffItem*)pJobResult;

	//if( MASYNC_RESULT_SUCCEED != pJob->GetResult() || !pJob->GetRet() ) 
	//{
	//	MCommand* pNew = CreateCommand( MC_MATCH_RESPONSE_TAKEOFF_ITEM, pJob->GetPlayerUID() );
	//	pNew->AddParameter( new MCommandParameterInt(MERR_CANNOT_TAKEOFF_ITEM) );
	//	PostSafeQueue( pNew );

	//	return;
	//}

	//MMatchObject* pObj = GetObject( pJob->GetPlayerUID() );
	//if( NULL == pObj )
	//{
	//	return;
	//}

	//MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	//if( NULL == pCharInfo )
	//{
	//	return;
	//}

	//const MUID&					uidPlayer	= pJob->GetPlayerUID();
	//const MMatchCharItemParts	parts		= pJob->GetParts();
	//

	//MCommand* pNew = CreateCommand( MC_MATCH_RESPONSE_TAKEOFF_ITEM, uidPlayer );
	//pNew->AddParameter( new MCommandParameterInt(MOK) );
	//PostSafeQueue( pNew );

	//pCharInfo->m_EquipedItem.Remove( pJob->GetParts() );


	//ResponseCharacterItemList( uidPlayer );


	//if( FindStage(pJob->GetStageUID()) )
	//{
	//	// 만약 스테이지 안이면 스테이지에 알려서 유저의 Look을 업데이트 해줘야 한다.
	//	if( FindStage(pJob->GetStageUID()) )
	//	{
	//		MCommand* pEquipInfo = CreateCommand( MC_MATCH_ROUTE_UPDATE_STAGE_EQUIP_LOOK, MUID(0, 0) );
	//		pEquipInfo->AddParameter( new MCmdParamUID(uidPlayer) );
	//		pEquipInfo->AddParameter( new MCmdParamInt(pJob->GetParts()) );
	//		pEquipInfo->AddParameter( new MCmdParamInt(0) );
	//		RouteToStage( pJob->GetStageUID(), pEquipInfo );
	//	}
	//}
}