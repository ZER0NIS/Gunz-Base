#include "stdafx.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MMatchObject.h"
#include "MMatchItem.h"
#include "MAgentObject.h"
#include "MMatchNotify.h"
#include "Msg.h"
#include "MMatchObjCache.h"
#include "MMatchStage.h"
#include "MMatchTransDataType.h"
#include "MMatchFormula.h"
#include "MMatchConfig.h"
#include "MCommandCommunicator.h"
#include "MMatchShop.h"
#include "MMatchTransDataType.h"
#include "MDebug.h"
#include "MMatchAuth.h"
#include "MMatchStatus.h"
#include "MAsyncDBJob.h"
#include "MMatchTransDataType.h"
#include "MUtil.h"

void CopyChannelPlayerListNodeForTrans(MTD_ChannelPlayerListNode* pDest, MMatchObject* pSrcObject)
{
	pDest->uidPlayer = pSrcObject->GetUID();
	strcpy(pDest->szName, pSrcObject->GetCharInfo()->m_szName);
	strcpy(pDest->szClanName, pSrcObject->GetCharInfo()->m_ClanInfo.m_szClanName);
	pDest->nLevel = (char)pSrcObject->GetCharInfo()->m_nLevel;
	pDest->nPlace = pSrcObject->GetPlace();
	pDest->nGrade = (unsigned char)pSrcObject->GetAccountInfo()->m_nUGrade;
	pDest->nPlayerFlags = pSrcObject->GetPlayerFlags();
	pDest->nCLID = pSrcObject->GetCharInfo()->m_ClanInfo.m_nClanID;

	MMatchObjectDuelTournamentCharInfo* pDTCharInfo = pSrcObject->GetDuelTournamentCharInfo();
	pDest->nDTLastWeekGrade = pDTCharInfo ? pDTCharInfo->GetLastWeekGrade() : 0;

	MMatchClan* pClan = MMatchServer::GetInstance()->GetClanMap()->GetClan(pSrcObject->GetCharInfo()->m_ClanInfo.m_nClanID);
	if (pClan)
		pDest->nEmblemChecksum = pClan->GetEmblemChecksum();
	else
		pDest->nEmblemChecksum = 0;
}


MMatchChannel* MMatchServer::FindChannel(const MUID& uidChannel)
{
	return m_ChannelMap.Find(uidChannel);
}

MMatchChannel* MMatchServer::FindChannel(const MCHANNEL_TYPE nChannelType, const char* pszChannelName)
{
	return m_ChannelMap.Find(nChannelType, pszChannelName);
}


bool MMatchServer::ChannelAdd(const char* pszChannelName, const char* pszRuleName, MUID* pAllocUID, MCHANNEL_TYPE nType, int nMaxPlayers, int nLevelMin, int nLevelMax,
							  const bool bIsTicketChannel, const DWORD dwTicketItemID, const bool bIsUseTicket, const char* pszChannelNameStrResId)
{
	return m_ChannelMap.Add(pszChannelName, pszRuleName, pAllocUID, nType, nMaxPlayers, nLevelMin, nLevelMax, bIsTicketChannel, dwTicketItemID, bIsUseTicket, pszChannelNameStrResId);
}

bool MMatchServer::ChannelJoin(const MUID& uidPlayer, const MCHANNEL_TYPE nChannelType, const char* pszChannelName)
{
	if ((nChannelType < 0) || (nChannelType >= MCHANNEL_TYPE_MAX)) return false;

	int nChannelNameLen = (int)strlen(pszChannelName);
	if ((nChannelNameLen >= CHANNELNAME_LEN) || (nChannelNameLen <= 0)) return false;


	MUID uidChannel = MUID(0,0);

	// 같이 이름의 채널이 존재하는지 검사한다.
	// 만약 있다면 그 채널로 바고 입장.
	// 없으면 채널 생성.
	MMatchChannel* pChannel = FindChannel(nChannelType, pszChannelName);
	
	if (pChannel == NULL)
	{
		// 프리셋 채널이면 채널을 만들 수 없다.
		if (nChannelType == MCHANNEL_TYPE_PRESET) 
			return false;

		if( nChannelType == MCHANNEL_TYPE_DUELTOURNAMENT )
			return false;

		//bool bbadf = GetChannelMap()->GetClanChannelTicketInfo().m_bIsTicketChannel; //debug
/*
		// 입장권을 사용하고, 클랜채널을 만들때는 유저가 입장권이 있는지 검사를 해줘야 한다.
		if( MGetServerConfig()->IsUseTicket() && 
			MCHANNEL_TYPE_CLAN == nChannelType && 
			GetChannelMap()->GetClanChannelTicketInfo().m_bIsTicketChannel )
		{
			MMatchObject* pObj = GetObject( uidPlayer );
			if( 0 != pObj )
			{
				// 생성 여부 검사 및 자격 미달이면 일반 채널로 이동을 고려해 줘야 한다.
				// 현제 상태에서 그냥 종료 시키면 서버에 접속을 못할수도 있다. - by SungE

				if( !pObj->GetCharInfo()->m_ItemList.IsHave( 
					GetChannelMap()->GetClanChannelTicketInfo().m_dwTicketItemID) )
				{
					// 입장권이 없음으로 일반 채널로 이동새켜줘야 한다.

					RouteResponseToListener(pObj, MC_MATCH_RESPONSE_RESULT, MERR_CANNOT_JOIN_NEED_TICKET);

					const MUID& uidChannel = FindFreeChannel( uidPlayer );
					if( MUID(0, 0) == uidChannel )
					{
						ASSERT( 0 && "들어갈 수 있는 채널을 찾지 못했음.");
						mlog( "MMatchServer_Channel.cpp - ChannelJoin : Can't find free channel.\n" );
						return false;
					}

					return ChannelJoin( uidPlayer, uidChannel );
				}
			}
			else
			{
				// 비정상 유저. 
				// 그냥 접속 종료를 시킨다.

				return false;
			}
		}
*/
		if (!ChannelAdd(pszChannelName, GetDefaultChannelRuleName(), &uidChannel, nChannelType)) 
			return false;
	}
	else
	{
		uidChannel = pChannel->GetUID();
	}
	
	return ChannelJoin(uidPlayer, uidChannel);
}

bool MMatchServer::ChannelJoin(const MUID& uidPlayer, const MUID& uidChannel)
{
	bool bEnableInterface = true;
	MUID uidChannelTmp = uidChannel;

	MMatchChannel* pChannel = FindChannel(uidChannelTmp);
	if (pChannel == NULL) return false;

	// 입장권이 필요한 채널에서는 유저가 입장권이 있는지 검사를 해줘야 한다.
	if ( MGetServerConfig()->IsUseTicket())
	{
		bool bCheckTicket = false;

		MMatchObject* pObj = GetObject(uidPlayer);
		if ( !pObj)	return false;

		if ( MGetServerConfig()->GetServerMode() == MSM_NORMALS)	{ // 일반 서버일때
			// 자유/사설/클랜 채널이면 로비 인터페이스를 disable 시킨다.
			// 그 외에는 티켓 채널이면 티켓 검사만 해준다.
			if ( _stricmp( pChannel->GetRuleName() , MCHANNEL_RULE_NOVICE_STR) == 0) bEnableInterface = false;			
			else if ( pChannel->IsTicketChannel())									bCheckTicket = true;
		} else if ( (MGetServerConfig()->GetServerMode() == MSM_CLAN) || (MGetServerConfig()->GetServerMode() == MSM_QUEST)) { // 클랜 서버일때
			// 클랜/사설 채널이면 티켓 검사한다.
			// 자유 채널이면 로비 인터페이스를 disable 시킨다.
			// 그 외에는 티켓 채널이면 티켓 검사만 해준다.
			if ( (pChannel->GetChannelType() == MCHANNEL_TYPE_CLAN) || (pChannel->GetChannelType() == MCHANNEL_TYPE_USER)) bCheckTicket = true;			
			else if ( _stricmp( pChannel->GetRuleName() , MCHANNEL_RULE_NOVICE_STR) == 0)
				bEnableInterface = false;			
			else if ( pChannel->IsTicketChannel())
				bCheckTicket = true;
		} else { // 그외 서버일때
			// 자유/사설/클랜 채널이면 로비 인터페이스를 disable 시킨다.
			// 그 외에는 티켓 채널이면 티켓 검사만 해준다.
			if ( _stricmp( pChannel->GetRuleName() , MCHANNEL_RULE_NOVICE_STR) == 0) bEnableInterface = false;			
			else if ( pChannel->IsTicketChannel())									bCheckTicket = true;
		}


		// 운영자일때...
		if ( IsAdminGrade( pObj)) {
			bCheckTicket		= false;
			bEnableInterface	= true;
		}

		if ( bCheckTicket) { // 티켓 검사
			// 생성 여부 검사 및 자격 미달이면 일반 채널로 이동을 고려해 줘야 한다.
			// 현제 상태에서 그냥 종료 시키면 서버에 접속을 못할수도 있다. - by SungE
			if( !pObj->GetCharInfo()->m_ItemList.IsHave( GetChannelMap()->GetClanChannelTicketInfo().m_dwTicketItemID))	{
				
				// 입장 불가 메시지
				if ( pObj->GetPlace() == MMP_LOBBY)
					RouteResponseToListener( pObj, MC_MATCH_RESPONSE_RESULT, MERR_CANNOT_JOIN_NEED_TICKET);


				// 입장권이 없음으로 일반 채널로 이동새켜줘야 한다.
				const MUID& uidFreeChannel = FindFreeChannel( uidPlayer);
				
				if( MUID(0, 0) == uidChannel) {
					ASSERT( 0 && "들어갈 수 있는 채널을 찾지 못했음.");
					mlog( "MMatchServer_Channel.cpp - ChannelJoin : Can't find free channel.\n" );
					return false;
				}

				uidChannelTmp		= uidFreeChannel;				
				bEnableInterface	= false;
			}
		}
	}

	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if (pObj == NULL) return false;

	const int ret = ValidateChannelJoin(uidPlayer, uidChannelTmp);
	if (ret != MOK) {
		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_RESULT, ret);
		return false;
	}

	// Leave Old Channel
	MMatchChannel* pOldChannel = FindChannel(pObj->GetChannelUID());
	if (pOldChannel) {
		pOldChannel->RemoveObject(uidPlayer);
	}

	// Join
	pObj->SetChannelUID(uidChannelTmp);
	pObj->SetLadderChallenging(false);
	pObj->SetPlace(MMP_LOBBY);
	pObj->SetStageListTransfer(true);	// turn on Auto refresh stage list
	pObj->SetStageCursor(0);

	pChannel = FindChannel(uidChannelTmp);
	if (pChannel == NULL) return false;

	pChannel->AddObject(uidPlayer, pObj);
	ResponseChannelJoin(uidPlayer, uidChannelTmp, (int)pChannel->GetChannelType(), pChannel->GetNameStringResId(), bEnableInterface);
	ResponseChannelRule(uidPlayer, uidChannelTmp);	// Channel 규칙을 보내준다.		

	
	if( pChannel->GetRuleType() != MCHANNEL_RULE_DUELTOURNAMENT ) {
		// 듀얼 토너먼트가 아닐 경우, Stage List를 보낸다.
		StageList(uidPlayer, 0, false);		
	} else {
		// 듀얼 토너먼트일 경우, 자신의 정보 및 랭킹 정보를 보내준다.
		// ResponseDuelTournamentCharInfo(uidPlayer);		
	}

	ChannelResponsePlayerList(uidPlayer, uidChannelTmp, 0);
	return true;
}

void MMatchServer::ResponseChannelJoin(MUID uidPlayer, MUID uidChannel, int nChannelType
									   , const char* szChannelStrResId, bool bEnableInterface)
{
	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if (pObj == NULL) return;

	MMatchChannel *pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return;

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_CHANNEL_RESPONSE_JOIN), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidChannel));
	pNew->AddParameter(new MCommandParameterInt(nChannelType));

	if (szChannelStrResId[0] != 0) {
		pNew->AddParameter(new MCommandParameterString((char*)szChannelStrResId));	// 공식 채널이면 클라에서 현재 언어로 번역할 수 있도록 스트링리소스ID를 넘긴다
	} else {
		pNew->AddParameter(new MCommandParameterString((char*)pChannel->GetName()));
	}
	
	pNew->AddParameter(new MCommandParameterBool(bEnableInterface));
	RouteToListener(pObj, pNew);
}

bool MMatchServer::ChannelLeave(const MUID& uidPlayer, const MUID& uidChannel)
{
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return false;
	pChannel->RemoveObject(uidPlayer);

	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if (pObj == NULL) return false;

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_CHANNEL_LEAVE),MUID(0,0),m_This);
	pNew->AddParameter(new MCommandParameterUID(uidPlayer));
	pNew->AddParameter(new MCommandParameterUID(pChannel->GetUID()));
	RouteToListener(pObj, pNew);

	if (pObj) 
	{
		pObj->SetChannelUID(MUID(0,0));
		pObj->SetPlace(MMP_OUTSIDE);
		pObj->SetStageListTransfer(false);	// turn off Auto refresh stage list
	}
	return true;
}

/*
// RAONHAJE 임시코드
#include "CMLexicalAnalyzer.h"
bool StageGo(MMatchServer* pServer, const MUID& uidPlayer, char* pszChat)
{
	MMatchObject* pChar = pServer->GetObject(uidPlayer);
	if (pChar == NULL)	return false;
	if (pChar->GetPlace() != MMP_LOBBY) return false;
	MMatchChannel* pChannel = pServer->FindChannel(pChar->GetChannelUID());
	if (pChannel == NULL) return false;

	bool bResult = false;
	CMLexicalAnalyzer lex;
	lex.Create(pszChat);

	if (lex.GetCount() >= 1) {
		char* pszCmd = lex.GetByStr(0);
		if (pszCmd) {
			if (_stricmp(pszCmd, "/go") == 0) {
				if (lex.GetCount() >= 2) {
					char* pszTarget = lex.GetByStr(1);
					if (pszTarget) {
						int nRoomNo = atoi(pszTarget);
						MMatchStage* pStage = pChannel->GetStage(nRoomNo-1);
						if (pStage) {
							//pServer->StageJoin(uidPlayer, pStage->GetUID());
							MCommand* pNew = pServer->CreateCommand(MC_MATCH_REQUEST_STAGE_JOIN, pServer->GetUID());
							pNew->AddParameter(new MCommandParameterUID(uidPlayer));
							pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));
							pServer->Post(pNew);
							bResult = true;
						}
					}
				}
			}	// go
		}
	}

	lex.Destroy();
	return bResult;
}
*/

bool MMatchServer::ChannelChat(const MUID& uidPlayer, const MUID& uidChannel, char* pszChat)
{
	if( 0 == strlen(pszChat) ) return false;
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return false;
	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) return false;
	if (pObj->GetAccountInfo()->m_nUGrade == MMUG_CHAT_LIMITED) return false;

	int nGrade = (int) pObj->GetAccountInfo()->m_nUGrade;

/*
	// RAONHAJE : GO 임시코드
	if (pszChat[0] == '/')
		if (StageGo(this, uidPlayer, pszChat))
			return true;
*/
	///< 홍기주(2009.08.04)
	///< 현재 해당 사용자가 있는 Channel과 보내온 Channel UID가 다를 경우!
	///< 다른 채널에게도 Msg를 보낼 수 있는 문제가 있음 (해킹 프로그램 사용시)
	if( uidChannel != pObj->GetChannelUID() )
	{
		//LOG(LOG_FILE,"MMatchServer::ChannelChat - Different Channel(S:%d, P:%d)", uidChannel, pObj->GetChannelUID());
		return false;
	}

	MUID uidStage = pObj->GetStageUID();
	if( uidStage != MUID(0, 0) )
	{
		//LOG(LOG_FILE,"MMatchServer::ChannelChat - Player In Stage(S:%d), Not Lobby", uidStage);
		return false;
	}

	if( pObj->GetAccountPenaltyInfo()->IsBlock(MPC_CHAT_BLOCK) ) {
		return false;
	}

	///< 채팅 메세지로 도배를 시도할 경우, '벙어리' 기능	
	if( pObj->IsChatBanUser() == true )	return false;
	if( pObj->CheckChatFlooding() )
	{
		pObj->SetChatBanUser();
		
		MCommand* pCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_CHANNEL_DUMB_CHAT), pObj->GetUID(), m_This);
		Post(pCmd);

		LOG(LOG_FILE, "MMatchServer::ChannelChat - Set Dumb Player(MUID:%d%d, Name:%s)", pObj->GetUID().High, pObj->GetUID().Low, pObj->GetName());
		return false;
	}

	///<여기까지....

	MCommand* pCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_CHANNEL_CHAT), MUID(0,0), m_This);
	pCmd->AddParameter(new MCommandParameterUID(uidChannel));
	pCmd->AddParameter(new MCommandParameterString(pObj->GetCharInfo()->m_szName));
	pCmd->AddParameter(new MCommandParameterString(pszChat));
	pCmd->AddParameter(new MCommandParameterInt(nGrade));

	RouteToChannelLobby(uidChannel, pCmd);
	return true;
}

void MMatchServer::OnRequestRecommendedChannel(const MUID& uidComm)
{
	MUID uidChannel = FindFreeChannel( uidComm );

	if (MUID(0,0) == uidChannel ) 
	{
		if( !ChannelAdd(GetDefaultChannelName(), GetDefaultChannelRuleName(), &uidChannel, MCHANNEL_TYPE_PRESET) )
		{
			mlog( "Channel Add fail for recommand.\n" );
			return;	// 생성조차 실패하면 낭패....
		}		
	}

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_RESPONSE_RECOMMANDED_CHANNEL),
									uidComm, m_This);
	pNew->AddParameter(new MCommandParameterUID(uidChannel));
	Post(pNew);
}

void MMatchServer::OnRequestChannelJoin(const MUID& uidPlayer, const MUID& uidChannel)
{
	ChannelJoin(uidPlayer, uidChannel);
}

void MMatchServer::OnRequestChannelJoin(const MUID& uidPlayer, const MCHANNEL_TYPE nChannelType, const char* pszChannelName)
{
	if ((nChannelType < 0) || (nChannelType >= MCHANNEL_TYPE_MAX)) return;

	ChannelJoin(uidPlayer, nChannelType, pszChannelName);
}

void MMatchServer::OnChannelChat(const MUID& uidPlayer, const MUID& uidChannel, char* pszChat)
{
	ChannelChat(uidPlayer, uidChannel, pszChat);
}

void MMatchServer::OnStartChannelList(const MUID& uidPlayer, const int nChannelType)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	pObj->SetChannelListTransfer(true, MCHANNEL_TYPE(nChannelType));
}

void MMatchServer::OnStopChannelList(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	pObj->SetChannelListTransfer(false);

}

void MMatchServer::ChannelList(const MUID& uidPlayer, MCHANNEL_TYPE nChannelType)
{
	MMatchObject* pChar = GetObject(uidPlayer);
	if (! IsEnabledObject(pChar)) return;

	if (pChar->GetPlace() != MMP_LOBBY) return;		// 로비가 아니면 무시
	if ((nChannelType < 0) || (nChannelType >= MCHANNEL_TYPE_MAX)) return;

	// Count Active Channels
	int nChannelCount = (int)m_ChannelMap.GetChannelCount(nChannelType);
	if (nChannelCount <= 0) return;

// 채널리스트는 최대 100개까지만 리스트를 보낸다.
#define MAX_CHANNEL_LIST_NODE		100

	nChannelCount = min(nChannelCount, MAX_CHANNEL_LIST_NODE);

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_CHANNEL_LIST), MUID(0,0), m_This);

	void* pChannelArray = MMakeBlobArray(sizeof(MCHANNELLISTNODE), nChannelCount);
	int nIndex=0;
	for (map<MUID, MMatchChannel*>::iterator itor=m_ChannelMap.GetTypesChannelMapBegin(nChannelType); 
		itor!=m_ChannelMap.GetTypesChannelMapEnd(nChannelType); itor++) {

		if (nIndex >= nChannelCount) break;

		MMatchChannel* pChannel = (*itor).second;

		MCHANNELLISTNODE* pNode = (MCHANNELLISTNODE*)MGetBlobArrayElement(pChannelArray, nIndex++);
		pNode->uidChannel = pChannel->GetUID();
		pNode->nNo = nIndex;
		pNode->nPlayers = (unsigned char)pChannel->GetObjCount();
		pNode->nMaxPlayers = pChannel->GetMaxPlayers();
		pNode->nChannelType = pChannel->GetChannelType();
		strcpy(pNode->szChannelName, pChannel->GetName());
		strcpy(pNode->szChannelNameStrResId, pChannel->GetNameStringResId());
		pNode->bIsUseTicket = pChannel->IsUseTicket();
		pNode->nTicketID = pChannel->GetTicketItemID();
	}
	pNew->AddParameter(new MCommandParameterBlob(pChannelArray, MGetBlobArraySize(pChannelArray)));
	MEraseBlobArray(pChannelArray);

	RouteToListener(pChar, pNew);
}





//void MMatchServer::OnChannelRequestPlayerList(const MUID& uidPlayer, const MUID& uidChannel, int nPage)
//{
//	ChannelResponsePlayerList(uidPlayer, uidChannel, nPage);
//}

void MMatchServer::OnChannelRequestPlayerList(const MUID& uidPlayer, const MUID& uidChannel, int nPage)
{
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return;
	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if (! IsEnabledObject(pObj)) return;

	MRefreshClientChannelImpl* pImpl = pObj->GetRefreshClientChannelImplement();
	pImpl->SetCategory(nPage);
	pImpl->SetChecksum(0);
	pImpl->Enable(true);
	pChannel->SyncPlayerList(pObj, nPage);
}

void MMatchServer::ChannelResponsePlayerList(const MUID& uidPlayer, const MUID& uidChannel, int nPage)
{
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return;
	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if (! IsEnabledObject(pObj)) return;

	int nObjCount = (int)pChannel->GetObjCount();
	int nNodeCount = 0;
	int nPlayerIndex;

	if (nPage < 0) nPage = 0;

	nPlayerIndex = nPage * NUM_PLAYERLIST_NODE;
	if (nPlayerIndex >= nObjCount) 
	{
		nPage = (nObjCount / NUM_PLAYERLIST_NODE);
		nPlayerIndex = nPage * NUM_PLAYERLIST_NODE;
	}

	MUIDRefCache::iterator FirstItor = pChannel->GetObjBegin();

	for (int i = 0; i < nPlayerIndex; i++) 
	{
		if (FirstItor == pChannel->GetObjEnd()) break;
		FirstItor++;
	}

	nNodeCount = nObjCount - nPlayerIndex;
	if (nNodeCount <= 0) 
	{
		return;
	}
	else if (nNodeCount > NUM_PLAYERLIST_NODE) nNodeCount = NUM_PLAYERLIST_NODE;


	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_CHANNEL_RESPONSE_PLAYER_LIST), MUID(0,0), m_This);
	//pNew->AddParameter(new MCommandParameterUID(uidChannel));
	pNew->AddParameter(new MCommandParameterUChar((unsigned char)nObjCount));
	pNew->AddParameter(new MCommandParameterUChar((unsigned char)nPage));

	void* pPlayerArray = MMakeBlobArray(sizeof(MTD_ChannelPlayerListNode), nNodeCount);

	int nArrayIndex=0;
	for (MUIDRefCache::iterator i=FirstItor; i != pChannel->GetObjEnd(); i++) 
	{
		MMatchObject* pScanObj = (MMatchObject*)(*i).second;

		MTD_ChannelPlayerListNode* pNode = (MTD_ChannelPlayerListNode*)MGetBlobArrayElement(pPlayerArray, nArrayIndex++);

		if (IsEnabledObject(pScanObj))
		{
			CopyChannelPlayerListNodeForTrans(pNode, pScanObj);		
		}

		if (nArrayIndex >= nNodeCount) break;
	}

	pNew->AddParameter(new MCommandParameterBlob(pPlayerArray, MGetBlobArraySize(pPlayerArray)));
	MEraseBlobArray(pPlayerArray);
	RouteToListener(pObj, pNew);
}

void MMatchServer::OnChannelRequestAllPlayerList(const MUID& uidPlayer, const MUID& uidChannel, unsigned long int nPlaceFilter,
												 unsigned long int nOptions)
{
	ChannelResponseAllPlayerList(uidPlayer, uidChannel, nPlaceFilter, nOptions);
}


void MMatchServer::ChannelResponseAllPlayerList(const MUID& uidPlayer, const MUID& uidChannel, unsigned long int nPlaceFilter,
												unsigned long int nOptions)
{
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return;
	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if (! IsEnabledObject(pObj)) return;

	int nNodeCount = 0;

	MMatchObject* ppTransObjectArray[DEFAULT_CHANNEL_MAXPLAYERS];
	memset(ppTransObjectArray, 0, sizeof(MMatchObject*) * DEFAULT_CHANNEL_MAXPLAYERS);

	// TransObjectArray에 전송할 Object의 포인터만 저장해놓는다.
	for (MUIDRefCache::iterator i=pChannel->GetObjBegin(); i != pChannel->GetObjEnd(); i++) 
	{
		MMatchObject* pScanObj = (MMatchObject*)(*i).second;

		if (IsEnabledObject(pScanObj))
		{
			if (CheckBitSet(nPlaceFilter, (pScanObj->GetPlace())))
			{
				bool bScanObjOK = true;
				switch (nOptions)
				{
				case MCP_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST_NONCLAN:
					{
						if (pScanObj->GetCharInfo()->m_ClanInfo.IsJoined()) bScanObjOK = false;
					}
					break;
				case MCP_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST_MYCLAN:
					{
						if (!pObj->GetCharInfo()->m_ClanInfo.IsJoined()) 
						{
							bScanObjOK = false;
						}
						else if (pScanObj->GetCharInfo()->m_ClanInfo.m_nClanID != pObj->GetCharInfo()->m_ClanInfo.m_nClanID)
						{
							bScanObjOK = false;
						}
					}
					break;
				}

				if (bScanObjOK)
				{
					ppTransObjectArray[nNodeCount] = pScanObj;
					nNodeCount++;

					if (nNodeCount >= DEFAULT_CHANNEL_MAXPLAYERS) break;
				}
			}
		}
	}

	if (nNodeCount <= 0) return;

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_CHANNEL_RESPONSE_ALL_PLAYER_LIST), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidChannel));

	void* pPlayerArray = MMakeBlobArray(sizeof(MTD_ChannelPlayerListNode), nNodeCount);

	for (int i = 0; i < nNodeCount; i++)
	{
		MMatchObject* pScanObj = ppTransObjectArray[i];

		MTD_ChannelPlayerListNode* pNode = (MTD_ChannelPlayerListNode*)MGetBlobArrayElement(pPlayerArray, i);

		if (IsEnabledObject(pScanObj))
		{
			CopyChannelPlayerListNodeForTrans(pNode, pScanObj);
		}
	}

	pNew->AddParameter(new MCommandParameterBlob(pPlayerArray, MGetBlobArraySize(pPlayerArray)));
	MEraseBlobArray(pPlayerArray);
	RouteToListener(pObj, pNew);
}



void MMatchServer::ResponseChannelRule(const MUID& uidPlayer, const MUID& uidChannel)
{
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return;
	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) return;

	MCommand* pNew = CreateCommand(MC_MATCH_CHANNEL_RESPONSE_RULE, MUID(0,0));
	pNew->AddParameter( new MCommandParameterUID(uidChannel) );
	pNew->AddParameter( new MCmdParamStr(const_cast<char*>(pChannel->GetRuleName())) );
	RouteToListener(pObj, pNew);
}


const MUID MMatchServer::FindFreeChannel(  const MUID& uidPlayer  )
{
	MUID uidChannel = MUID(0,0);

	if (uidChannel == MUID(0,0) &&
		MGetServerConfig()->IsEnabledDuelTournament() && 
		MGetServerConfig()->IsSendLoginUserToDuelTournamentChannel())
	{
		for(map<MUID, MMatchChannel*>::iterator itor=m_ChannelMap.GetTypesChannelMapBegin(MCHANNEL_TYPE_DUELTOURNAMENT); 
			itor!=m_ChannelMap.GetTypesChannelMapEnd(MCHANNEL_TYPE_DUELTOURNAMENT); itor++) {

				MUID uid = (*itor).first;
				if (MOK == ValidateChannelJoin(uidPlayer, uid)) {
					MMatchChannel* pChannel = FindChannel(uid);
					if (pChannel) {
						if (pChannel->GetMaxPlayers()*0.8 < pChannel->GetObjCount()) continue;
						uidChannel = uid;
						break;
					}
				}
			}
	}

	if (uidChannel == MUID(0,0))
	{
		// Find proper channel by Level
		for(map<MUID, MMatchChannel*>::const_iterator itor=m_ChannelMap.GetTypesChannelMapBegin(MCHANNEL_TYPE_PRESET); 
			itor!=m_ChannelMap.GetTypesChannelMapEnd(MCHANNEL_TYPE_PRESET); itor++) {
				MUID uid = (*itor).first;
				if (MOK == ValidateChannelJoin(uidPlayer, uid)) {
					MMatchChannel* pChannel = FindChannel(uid);
					if (pChannel) {
						if (pChannel->GetLevelMin() <= 0) continue;
						if (pChannel->GetMaxPlayers()*0.8 < pChannel->GetObjCount()) continue;
						uidChannel = uid;
						break;
					}
				}
			}
	}

	// 디버그 버전은 무조건 자유채널로 입장하게 만들었다.
//#ifdef _DEBUG
//	for(map<MUID, MMatchChannel*>::iterator itor=m_ChannelMap.GetTypesChannelMapBegin(MCHANNEL_TYPE_PRESET); 
//		itor!=m_ChannelMap.GetTypesChannelMapEnd(MCHANNEL_TYPE_PRESET); itor++) {
//		MUID uid = (*itor).first;
//		MMatchChannel* pChannel = FindChannel(uid);
//		if (pChannel) 
//		{
//			uidChannel = uid;
//				break;
//		}
//	}
//#endif

	// 레벨제한으로 못들어가면 공개채널로 들어간다.
	if (uidChannel == MUID(0,0))
	{
		for(map<MUID, MMatchChannel*>::iterator itor=m_ChannelMap.GetTypesChannelMapBegin(MCHANNEL_TYPE_PRESET); 
			itor!=m_ChannelMap.GetTypesChannelMapEnd(MCHANNEL_TYPE_PRESET); itor++) {

			MUID uid = (*itor).first;
			if (MOK == ValidateChannelJoin(uidPlayer, uid)) {
				MMatchChannel* pChannel = FindChannel(uid);
				if (pChannel) {
					if (pChannel->GetMaxPlayers()*0.8 < pChannel->GetObjCount()) continue;
					uidChannel = uid;
					break;
				}
			}
		}
	}

	// 만약 들어갈데가 없으면 사설채널로 들어간다.
	if (uidChannel == MUID(0,0))
	{
		for(map<MUID, MMatchChannel*>::iterator itor=m_ChannelMap.GetTypesChannelMapBegin(MCHANNEL_TYPE_USER); 
			itor!=m_ChannelMap.GetTypesChannelMapEnd(MCHANNEL_TYPE_USER); itor++) {
			MUID uid = (*itor).first;
			if (MOK == ValidateChannelJoin(uidPlayer, uid)) {
				MMatchChannel* pChannel = FindChannel(uid);
				if (pChannel) {
					uidChannel = uid;
					break;
				}
			}
		}
	}

	return uidChannel;
}
