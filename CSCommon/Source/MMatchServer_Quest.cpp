#include "stdafx.h"
#include "MMatchServer.h"
#include "MMatchStage.h"
#include "MMatchRule.h"
#include "MMatchRuleQuest.h"
#include "MMatchGameType.h"
#include "MMatchConfig.h"
#include "MBlobArray.h"
#include "MMatchShop.h"
#include "MAsyncDBJob_BuyQuestItem.h"

void MMatchServer::OnRequestNPCDead(const MUID& uidSender, const MUID& uidKiller, MUID& uidNPC, MVector& pos)
{
	MMatchObject* pSender = GetObject(uidSender);
	if (!IsEnabledObject(pSender)) { ASSERT( 0 ); return; }

	MMatchStage* pStage = FindStage(pSender->GetStageUID());
	if (pStage == NULL) { ASSERT( 0 ); return; }

	MMatchCharBattleTimeRewardInfoMap::iterator iter = pSender->GetCharInfo()->GetBRInfoMap().begin();
	for( ; iter !=  pSender->GetCharInfo()->GetBRInfoMap().end(); iter++ ) {
		MMatchCharBRInfo* pInfo = iter->second;
		pInfo->AddKillCount(1);	///< NPC Kill은 1 Kill로 계산!
	}

	if (MGetGameTypeMgr()->IsQuestDerived(pStage->GetStageSetting()->GetGameType()))
	{
		MMatchRule* pRule = pStage->GetRule();
		if( NULL == pRule ) return;
		if (false == MGetGameTypeMgr()->IsQuestDerived( pRule->GetGameType() )) return;

		MMatchRuleBaseQuest* pQuestRule  = reinterpret_cast< MMatchRuleBaseQuest* >( pRule );

		pQuestRule->OnRequestNPCDead((MUID&)uidSender, (MUID&)uidKiller, uidNPC, pos);
	}
	else
	{
		ASSERT( 0 );
	}
}


void MMatchServer::OnQuestRequestDead(const MUID& uidVictim)
{
	MMatchObject* pVictim = GetObject(uidVictim);
	if (pVictim == NULL) return;

	MMatchStage* pStage = FindStage(pVictim->GetStageUID());
	if (pStage == NULL) return;

	if ( !MGetGameTypeMgr()->IsQuestDerived(pStage->GetStageSetting()->GetGameType())) return;

	MMatchRuleBaseQuest* pQuestRule = (MMatchRuleBaseQuest*)pStage->GetRule();
	pQuestRule->OnRequestPlayerDead((MUID&)uidVictim);

	// 서버는 죽은줄 알고있었는데 또 죽었다고 신고들어온경우 죽었다는 메시지만 라우팅한다
	if (pVictim->CheckAlive() == false) {	
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SUICIDE, MUID(0,0));
		int nResult = MOK;
		pNew->AddParameter(new MCommandParameterInt(nResult));
		pNew->AddParameter(new MCommandParameterUID(pVictim->GetUID()));
		RouteToBattle(pStage->GetUID(), pNew);
		return;
	}

	pVictim->OnDead();


	// 죽었다는 메세지 보냄
	MCommand* pCmd = CreateCommand(MC_MATCH_QUEST_PLAYER_DEAD, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidVictim));
	RouteToBattle(pStage->GetUID(), pCmd);	
}



void MMatchServer::OnQuestTestRequestNPCSpawn(const MUID& uidPlayer, int nNPCType, int nNPCCount)
{
#ifdef _DEBUG
	if (MGetServerConfig()->GetServerMode() != MSM_QUEST) return;

	MMatchObject* pPlayer = GetObject(uidPlayer);
	if (pPlayer == NULL) return;
	MMatchStage* pStage = FindStage(pPlayer->GetStageUID());
	if (pStage == NULL) return;

#ifndef _DEBUG
	if (!IsAdminGrade(pPlayer)) return;
#endif

	if (MGetGameTypeMgr()->IsQuestDerived(pStage->GetStageSetting()->GetGameType()))
	{
		MMatchRuleBaseQuest* pQuestRule = (MMatchRuleBaseQuest*)pStage->GetRule();
		pQuestRule->OnRequestTestNPCSpawn(nNPCType, nNPCCount);
	}
#endif
}

void MMatchServer::OnQuestTestRequestClearNPC(const MUID& uidPlayer)
{
#ifndef _DEBUG
	return;
#endif

	if (MGetServerConfig()->GetServerMode() != MSM_QUEST) return;

	MMatchObject* pPlayer = GetObject(uidPlayer);
	if (pPlayer == NULL) return;
	MMatchStage* pStage = FindStage(pPlayer->GetStageUID());
	if (pStage == NULL) return;

#ifndef _DEBUG
	if (!IsAdminGrade(pPlayer)) return;
#endif

	if (MGetGameTypeMgr()->IsQuestDerived(pStage->GetStageSetting()->GetGameType()))
	{
		MMatchRuleBaseQuest* pQuestRule = (MMatchRuleBaseQuest*)pStage->GetRule();
		pQuestRule->OnRequestTestClearNPC();
	}

}


void MMatchServer::OnQuestTestRequestSectorClear(const MUID& uidPlayer)
{
#ifndef _DEBUG
	return;
#endif

	if (MGetServerConfig()->GetServerMode() != MSM_QUEST) return;

	MMatchObject* pPlayer = GetObject(uidPlayer);
	if (pPlayer == NULL) return;
	MMatchStage* pStage = FindStage(pPlayer->GetStageUID());
	if (pStage == NULL) return;

#ifndef _DEBUG
	if (!IsAdminGrade(pPlayer)) return;
#endif

	if (MGetGameTypeMgr()->IsQuestDerived( pStage->GetStageSetting()->GetGameType() ))
	{
		MMatchRuleBaseQuest* pQuestRule = (MMatchRuleBaseQuest*)pStage->GetRule();
		pQuestRule->OnRequestTestSectorClear();
	}

}

void MMatchServer::OnQuestTestRequestQuestFinish(const MUID& uidPlayer)
{
#ifndef _DEBUG
	return;
#endif

	if (MGetServerConfig()->GetServerMode() != MSM_QUEST) return;

	MMatchObject* pPlayer = GetObject(uidPlayer);
	if (pPlayer == NULL) return;
	MMatchStage* pStage = FindStage(pPlayer->GetStageUID());
	if (pStage == NULL) return;

#ifndef _DEBUG
	if (!IsAdminGrade(pPlayer)) return;
#endif

	if (MGetGameTypeMgr()->IsQuestDerived( pStage->GetStageSetting()->GetGameType() ))
	{
		MMatchRuleBaseQuest* pQuestRule = (MMatchRuleBaseQuest*)pStage->GetRule();
		pQuestRule->OnRequestTestFinish();
	}

}



void MMatchServer::OnQuestRequestMovetoPortal(const MUID& uidPlayer)
{
	MMatchObject* pPlayer = GetObject(uidPlayer);
	if (pPlayer == NULL) return;
	MMatchStage* pStage = FindStage(pPlayer->GetStageUID());
	if (pStage == NULL) return;

	if (false == MGetGameTypeMgr()->IsQuestDerived( pStage->GetStageSetting()->GetGameType() )) return;

	MMatchRuleBaseQuest* pQuestRule = (MMatchRuleBaseQuest*)pStage->GetRule();
	pQuestRule->OnRequestMovetoPortal(uidPlayer);
}

void MMatchServer::OnQuestReadyToNewSector(const MUID& uidPlayer)
{
	MMatchObject* pPlayer = GetObject(uidPlayer);
	if (pPlayer == NULL) return;
	MMatchStage* pStage = FindStage(pPlayer->GetStageUID());
	if (pStage == NULL) return;

	if (false == MGetGameTypeMgr()->IsQuestDerived( pStage->GetStageSetting()->GetGameType() )) return;

	MMatchRuleBaseQuest* pQuestRule = (MMatchRuleBaseQuest*)pStage->GetRule();
	pQuestRule->OnReadyToNewSector(uidPlayer);

}


void MMatchServer::OnRequestCharQuestItemList( const MUID& uidSender )
{
	if( MSM_QUEST != MGetServerConfig()->GetServerMode() ) 
		return;

	OnResponseCharQuestItemList( uidSender );
}
void MMatchServer::OnResponseCharQuestItemList( const MUID& uidSender )
{
	MMatchObject* pPlayer = GetObject( uidSender );
	if( !IsEnabledObject(pPlayer) )
		return;

	// 이전에 디비 억세스를 안했었으면 디비에서 퀘스트 아이템 정보를 가져온다
	if( !pPlayer->GetCharInfo()->m_QuestItemList.IsDoneDbAccess() )
	{
		if( !m_MatchDBMgr.GetCharQuestItemInfo(pPlayer->GetCharInfo()) )
		{
			mlog( "DB Query(ResponseCharacterItemList > GetcharQuestItemInfo) failed\n" );
			return;
		}
	}

	MCommand* pNewCmd = CreateCommand( MC_MATCH_RESPONSE_CHAR_QUEST_ITEM_LIST, MUID(0, 0) );
	if( 0 == pNewCmd )
	{
		mlog( "MMatchServer::OnResponseCharQuestItemList - Command생성 실패.\n" );
		return;
	}

	// 갖고 있는 퀘스트 아이템 리스트 전송.
	int					nIndex			= 0;
	MTD_QuestItemNode*	pQuestItemNode	= 0;
	void*				pQuestItemArray = MMakeBlobArray( static_cast<int>(sizeof(MTD_QuestItemNode)), 
														  static_cast<int>(pPlayer->GetCharInfo()->m_QuestItemList.size()) );

	MQuestItemMap::iterator itQItem, endQItem;
	endQItem = pPlayer->GetCharInfo()->m_QuestItemList.end();
	for( itQItem = pPlayer->GetCharInfo()->m_QuestItemList.begin(); itQItem != endQItem; ++itQItem )
	{
		pQuestItemNode = reinterpret_cast< MTD_QuestItemNode* >( MGetBlobArrayElement(pQuestItemArray, nIndex++) );
		Make_MTDQuestItemNode( pQuestItemNode, itQItem->second->GetItemID(), itQItem->second->GetCount() );
	}

	pNewCmd->AddParameter( new MCommandParameterBlob(pQuestItemArray, MGetBlobArraySize(pQuestItemArray)) );
	MEraseBlobArray( pQuestItemArray );

	RouteToListener( pPlayer, pNewCmd );
}

void MMatchServer::OnRequestBuyQuestItem( const MUID& uidSender, const unsigned long int nItemID, const int nItemCount )
{
	if (MGetServerConfig()->GetServerMode() == MSM_QUEST) {
		OnResponseBuyQuestItem( uidSender, nItemID, nItemCount );
	}
}
void MMatchServer::OnResponseBuyQuestItem( const MUID& uidSender, const unsigned long int nItemID, const int nItemCount )
{
	MMatchObject* pPlayer = GetObject( uidSender );
	if( !IsEnabledObject(pPlayer) ) return;

	MQuestItemDescManager::iterator itQItemDesc = GetQuestItemDescMgr().find( nItemID );
	if( GetQuestItemDescMgr().end() == itQItemDesc ) {
		mlog( "MMatchServer::OnResponseBuyQuestItem - %d아이템 description을 찾지 못했습니다.\n", nItemID );
		return;
	}

	MQuestItemDesc* pQuestItemDesc = itQItemDesc->second;
	if( 0 == pQuestItemDesc ) {
		mlog( "MMatchServer::OnRequestBuyQuestItem - %d의 item description이 비정상적입니다.\n", nItemID );
		return;
	}

	// 상점에서 판매되고 있는 아이템인지 검사.
	if( !MGetMatchShop()->IsSellItem(pQuestItemDesc->m_nItemID) ) {
		mlog( "MMatchServer::OnRequestBuyQuestItem - %d는 상점에서 판매되고 있는 아이템이 아님.\n", pQuestItemDesc->m_nItemID );
		return;
	}

	// 충분한 바운티가 되는지 검사.
	if( pPlayer->GetCharInfo()->m_nBP < (itQItemDesc->second->m_nPrice * nItemCount) ) {
		// 바운티가 부족한다는 정보를 알려줘야 함.
		// 임시로 MMatchItem에서 사용하는걸 사용했음.
		// 필요하면 Quest item에 맞는 커맨드로 수정해야 함.
		MCommand* pBPLess = CreateCommand( MC_MATCH_RESPONSE_BUY_QUEST_ITEM, MUID(0,0) );
		pBPLess->AddParameter( new MCmdParamInt(MERR_TOO_EXPENSIVE_BOUNTY) );
		pBPLess->AddParameter( new MCmdParamInt(pPlayer->GetCharInfo()->m_nBP) );
		RouteToListener(pPlayer, pBPLess);
		return;
	}

	MQuestItemMap::iterator itQItem = pPlayer->GetCharInfo()->m_QuestItemList.find( nItemID );
	if( pPlayer->GetCharInfo()->m_QuestItemList.end() != itQItem ) 
	{
		// 최대 개수를 넘는지 검사.
		if( MAX_QUEST_ITEM_COUNT > itQItem->second->GetCount() + nItemCount ) {			
			itQItem->second->Increase(nItemCount); // 개수 증가
		} else {
			// 가질수 있는 아이템의 최대 수를 넘어섰음.
			// 임시로 MMatchItem에서 사용하는걸 사용했음. 필요하면 Quest item에 맞는 커맨드로 수정해야 함.
			MCommand* pTooMany = CreateCommand( MC_MATCH_RESPONSE_BUY_QUEST_ITEM, MUID(0,0) );
			pTooMany->AddParameter( new MCmdParamInt(MERR_TOO_MANY_ITEM) );
			pTooMany->AddParameter( new MCmdParamInt(pPlayer->GetCharInfo()->m_nBP) );
			RouteToListener(pPlayer, pTooMany);
			return;
		}
	} else {
		MQuestItem* pNewQuestItem = new MQuestItem;
		if( 0 == pNewQuestItem ) {
			mlog( "MMatchServer::OnResponseBuyQuestItem - 새로운 퀘스트 아이템 생성 실패.\n" );
			return;
		}

		if( !pNewQuestItem->Create(nItemID, nItemCount, GetQuestItemDescMgr().FindQItemDesc(nItemID)) ) {
			delete pNewQuestItem;
			mlog( "MMatchServer::OnResponseBuyQeustItem - %d번호 아이템 Create( ... )함수 호출 실패.\n" );
			return;
		}

		pPlayer->GetCharInfo()->m_QuestItemList.insert( MQuestItemMap::value_type(nItemID, pNewQuestItem) );
	}

	UpdateCharDBCachingData(pPlayer);		///< XP, BP, KillCount, DeathCount 캐슁 업데이트
	//UpdateCharItemDBCachingData(pPlayer);	///< Character Item에서 업데이트가 필요한 것들 업데이트 <-불필요함


	MAsyncDBJob_BuyQuestItem* pBuyQuestItemJob = new MAsyncDBJob_BuyQuestItem( uidSender );
	if( NULL == pBuyQuestItemJob ) { return; }	
	pBuyQuestItemJob->Input( uidSender, pPlayer->GetCharInfo()->m_nCID, nItemCount, pQuestItemDesc->m_nPrice);	// 디비에 바운티 더해준다
	pPlayer->m_DBJobQ.DBJobQ.push_back( pBuyQuestItemJob );
}

void MMatchServer::OnRequestSellQuestItem( const MUID& uidSender, const unsigned long int nItemID, const int nCount )
{
	if (MGetServerConfig()->GetServerMode() == MSM_QUEST)
	{
		OnResponseSellQuestItem( uidSender, nItemID, nCount );
	}
}

void MMatchServer::OnResponseSellQuestItem( const MUID& uidSender, const unsigned long int nItemID, const int nCount )
{
	MMatchObject* pPlayer = GetObject( uidSender );
	if( !IsEnabledObject(pPlayer) )
	{
		mlog( "MMatchServer::OnResponseSellQuestItem - find user fail.\n" );
		return;
	}

	MQuestItemDescManager::iterator itQItemDesc = GetQuestItemDescMgr().find( nItemID );
	if( GetQuestItemDescMgr().end() == itQItemDesc )
	{
		mlog( "MMatchServer::OnResponseSellQuestItem - find item(%u) description fail.\n", nItemID );
		return;
	}
	
	MQuestItemDesc* pQItemDesc = itQItemDesc->second;
	if( 0 == pQItemDesc )
	{
		mlog( "MMatchServer::OnResponseSellQuestItem - item(%u) description is null point.\n", nItemID );
		return;
	}

	// 아이템 카운트 검사.
	MQuestItemMap::iterator itQItem = pPlayer->GetCharInfo()->m_QuestItemList.find( nItemID );
	if( pPlayer->GetCharInfo()->m_QuestItemList.end() != itQItem ) {
		if( nCount > itQItem->second->GetCount() ) {
			return;
		}

		UpdateCharDBCachingData(pPlayer);		///< XP, BP, KillCount, DeathCount 캐슁 업데이트
		//UpdateCharItemDBCachingData(pPlayer);	///< Character Item에서 업데이트가 필요한 것들 업데이트 <-불필요함

		// 디비에 바운티 더해준다
		int nPrice = ( pQItemDesc->GetSellBountyValue(nCount) );
		if (!m_MatchDBMgr.UpdateCharBP(pPlayer->GetCharInfo()->m_nCID, nPrice))	{
			/*
			MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, MUID(0,0));
			pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_SELL_ITEM));
			RouteToListener(pObj, pNew);

			return false;
			*/
			return;
		}

		itQItem->second->Decrease( nCount );
		pPlayer->GetCharInfo()->m_nBP += nPrice;		// 되팔시는 1/4만 받을수 있음.

	} else {
		// 존제하지 않는 아이템을 팔려고 하였음.
		mlog( "MMatchServer::OnResponseSellQuestItem - user is not owner. itemid(%u)\n", nItemID );
		ASSERT( 0 );
		return;
	}

	// 아이템 거래 카운트 증가. 내부에서 디비 업데이트 결정.
	pPlayer->GetCharInfo()->GetDBQuestCachingData().IncreaseShopTradeCount();

	MCommand* pCmd = CreateCommand( MC_MATCH_RESPONSE_SELL_QUEST_ITEM, MUID(0, 0) );
	if( 0 == pCmd ) {
		return;
	}

	pCmd->AddParameter( new MCmdParamInt(MOK) );
	pCmd->AddParameter( new MCmdParamInt(pPlayer->GetCharInfo()->m_nBP) );
	RouteToListener( pPlayer, pCmd );

	// 퀘스트 아이템 리스트를 다시 전송함.
	OnRequestCharQuestItemList( pPlayer->GetUID() );
}


void MMatchServer::OnRequestDropSacrificeItemOnSlot( const MUID& uidSender, const int nSlotIndex, const unsigned long int nItemID )
{
#ifdef _QUEST_ITEM
	if (MGetServerConfig()->GetServerMode() == MSM_QUEST)
	{
		MMatchObject* pPlayer = GetObject( uidSender );
		if( !IsEnabledObject(pPlayer) )
		{
			mlog( "MMatchServer::OnRequestDropSacrificeItemOnSlot - invalid user.\n" );
			return;
		}

		MMatchStage* pStage = FindStage( pPlayer->GetStageUID() );
		if( 0 != pStage )
		{
			if(STAGE_STATE_RUN == pStage->GetState())
				return; // 게임시작시 희생아이템을 슬롯에 올리면 무한 재료 버그 발생되기 때문에 여기서 막아줌

			const MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
			if( 0 == pNode )
			{
				mlog( "MMatchServer::OnRequestDropSacrificeItemOnSlot - find stage fail.\n" );
				return;
			}

			if( MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType) )
			{
				MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule() );
				if( 0 == pRuleQuest )
					return;

				pRuleQuest->OnRequestDropSacrificeItemOnSlot( uidSender, nSlotIndex, nItemID );
			}
		}
	}	
#endif
}


void MMatchServer::OnRequestCallbackSacrificeItem( const MUID& uidSender, const int nSlotIndex, const unsigned long int nItemID )
{
#ifdef _QUEST_ITEM
	if (MGetServerConfig()->GetServerMode() == MSM_QUEST)
	{
		MMatchObject* pPlayer = GetObject( uidSender );
		if( !IsEnabledObject(pPlayer) )
		{
			mlog( "MMatchServer::OnRequestDropSacrificeItemOnSlot - invalid user.\n" );
			return;
		}

		MMatchStage* pStage = FindStage( pPlayer->GetStageUID() );
		if( 0 != pStage )
		{
			const MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
			if( 0 == pNode )
			{
				mlog( "MMatchServer::OnRequestCallbackSacrificeItem - find stage fail.\n" );
				return;
			}

			if( MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType) )
			{
				MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule() );
				if( 0 == pRuleQuest )
					return;

				pRuleQuest->OnRequestCallbackSacrificeItem( uidSender, nSlotIndex, nItemID );
			}
		}
	}
#endif
}

void MMatchServer::OnRequestQL( const MUID& uidSender )
{
#ifdef _QUEST_ITEM
	if (MGetServerConfig()->GetServerMode() == MSM_QUEST)
	{
		MMatchObject* pPlayer = GetObject( uidSender );
		if( !IsEnabledObject(pPlayer) )
		{
			mlog( "MMatchServer::OnRequestQL - invlaid user.\n" );
			return;
		}

		MMatchStage* pStage = FindStage( pPlayer->GetStageUID() );
		if( 0 != pStage )
		{
			const MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
			if( 0 == pNode )
			{
				mlog( "MMatchServer::OnRequestQL - find stage fail.\n" );
				return;
			}

			if( MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType) )
			{
				MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule() );
				if( 0 == pRuleQuest )
					return;

				pRuleQuest->OnRequestQL( uidSender );
			}
		}
	}	
#endif
}


void MMatchServer::OnRequestSacrificeSlotInfo( const MUID& uidSender )
{
#ifdef _QUEST_ITEM
	if (MGetServerConfig()->GetServerMode() == MSM_QUEST)
	{
		MMatchObject* pPlayer = GetObject( uidSender );
		if( !IsEnabledObject(pPlayer) )
		{
			mlog( "MMatchServer::OnRequestSacrificeSlotInfo - invalid user.\n" );
			return;
		}

		MMatchStage* pStage = FindStage( pPlayer->GetStageUID() );
		if( 0 != pStage )
		{
			const MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
			if( 0 == pNode )
			{
				mlog( "MMatchServer::OnRequestSacrificeSlotInfo - find stage fail.\n" );
				return;
			}

			if( MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType) )
			{
				MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule() );
				if( 0 == pRuleQuest )
					return;

				pRuleQuest->OnRequestSacrificeSlotInfo( uidSender );
			}
		}
	}
#endif
}

void MMatchServer::OnQuestStageMapset(const MUID& uidStage, int nMapsetID)
{
	if (QuestTestServer())
	{



	}
}


void MMatchServer::OnRequestMonsterBibleInfo( const MUID& uidSender )
{
	if (MGetServerConfig()->GetServerMode() == MSM_QUEST)
	{
		MMatchObject* pPlayer = GetObject( uidSender );
		if( !IsEnabledObject(pPlayer) )
		{
			mlog( "MMatchServer::OnRequestMonsterBibleInfo - invalid user.\n" );
			return;
		}

		OnResponseMonsterBibleInfo( uidSender );
	}
}


void MMatchServer::OnResponseMonsterBibleInfo( const MUID& uidSender )
{
	MMatchObject* pObj = GetObject( uidSender );
	if( !IsEnabledObject(pObj) )
		return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if( 0 == pCharInfo )
		return;

	// 디비에서 케릭터 퀘스트 관련정보를 가저와있는지 검사를 함.
	if( !pCharInfo->m_QuestItemList.IsDoneDbAccess() )
	{
		mlog( "MMatchServer::OnResponseMonsterBibleInfo - not load db monsterbible info.\n" );
		return;
	}	

	void* pMonBibleInfoBlob = MMakeBlobArray( MONSTER_BIBLE_SIZE, 1 );
	if( 0 == pMonBibleInfoBlob )
	{
		mlog( "MMatchServer::OnResponseMonsterBibleInfo - make blob fail.\n" );
		return;
	}

	MQuestMonsterBible* pMonBible = reinterpret_cast< MQuestMonsterBible * >( MGetBlobArrayElement(pMonBibleInfoBlob, 0) );
	if( 0 == pMonBible )
	{
		mlog( "MMatchServer::OnResponseMonsterBibleInfo - typecast fail.\n" );
		return;
	}

	memcpy( pMonBible, &(pCharInfo->m_QMonsterBible), MONSTER_BIBLE_SIZE );


	MCommand* pCmd = CreateCommand( MC_MATCH_RESPONSE_MONSTER_BIBLE_INFO, MUID(0, 0) );
	if( 0 == pCmd )
	{
		mlog( "MMatchServer::OnResponseMonsterBibleInfo - create command fail.\n" );
		return;
	}

	pCmd->AddParameter( new MCmdParamUID(uidSender) );
	pCmd->AddParameter( new MCommandParameterBlob(pMonBibleInfoBlob, MGetBlobArraySize(pMonBibleInfoBlob)) );

	RouteToListener( pObj, pCmd );

	MEraseBlobArray( pMonBibleInfoBlob );
}


void MMatchServer::OnQuestPong( const MUID& uidSender )
{
	MMatchObject* pObj = GetObject( uidSender );
	if( 0 == pObj ) 
		return;

	pObj->SetQuestLatency(GetGlobalClockCount());
	pObj->m_bQuestRecvPong = true;
}