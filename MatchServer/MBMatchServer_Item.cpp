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
#include "MMatchDBGambleItem.h"
#include "MMatchGambleMachine.h"
#include "MBMatchServer.h"
#include "MMatchUtil.h"
#include "MMatchItemFunction.h"
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




void MBMatchServer::OnRequestShopItemList(const MUID& uidPlayer, const int nFirstItemIndex, const int nItemCount)
{
	ResponseShopItemList(uidPlayer, nFirstItemIndex, nItemCount);
}

void MBMatchServer::ResponseShopItemList(const MUID& uidPlayer, const int nFirstItemIndex, const int nItemCount)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	if( NULL == pObj->GetCharInfo() ) return;

	SendGambleItemList( pObj );

	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SHOP_ITEMLIST, uidPlayer);

	// 커맨드의 파라메터로 블럭을 추가한다. //
	pNew->AddParameter( MakeShopGambleItemListBlob() );
	pNew->AddParameter( MakeShopItemListBlob(nFirstItemIndex, nItemCount) );

	PostSafeQueue( pNew );
}

void MBMatchServer::OnRequestGamble(const MUID& uidPlayer, const MUID& uidItem)
{
	// 자신의 인벤토리에 존재하는 아이템인지 확인한다. //
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) { return; }

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if (pCharInfo == NULL) { return; }

	// 유저가 가지고 있는 아이템 고유 아이디로 해당 아이템 찾기
	const MMatchCharGambleItem* pGambleItem = pCharInfo->m_GambleItemManager.GetGambleItemByUID(uidItem);
	if (pGambleItem == NULL) {
		mlog("GetGambleItem() failed! [CID : %d]\n", pCharInfo->m_nCID);
		return;
	}

	//if( pGambleItem->GetGambleItemID() >= 1000100)  // quest item 으로 바뀌는 gamble
	// 이번에 캐시용 겜블아이템 추가로 수정
	// 기존 일본 요청으로 퀘스트용 겜블아이템을 추가했었음 (1000100~1000199사이)
	// 퀘스트용 겜블아이템은 1000100에서 1000200로 제한한다
	if( 1000100 <= pGambleItem->GetGambleItemID()  && pGambleItem->GetGambleItemID() < 1000200)	// quest item 으로 바뀌는 gamble
	{
		// 겜블 랜덤함수를 호출하여 결과 아이템의 ID를 가져온다. //
		const MMatchGambleRewardItem* pGambleRewardItem = GetGambleMachine().Gamble(pGambleItem->GetGambleItemID());
		if (pGambleRewardItem == NULL) {
			mlog("Gamble() failed! [ItemID : %d]\n", pGambleItem->GetGambleItemID());
			ASSERT(0);
			return;
		}

		DWORD dwRIID;
		if( MMS_MALE == pCharInfo->m_nSex )			dwRIID = pGambleRewardItem->GetItemIDMale();
		else if ( MMS_FEMALE == pCharInfo->m_nSex )	dwRIID = pGambleRewardItem->GetItemIDFemale();

		MBMatchAsyncDBJob_GambleQuestItem *pJob = new MBMatchAsyncDBJob_GambleQuestItem(uidPlayer);
		pJob->Input( uidPlayer, (MUID&)pGambleItem->GetUID(), pCharInfo->m_nCID, pGambleItem->GetCIID(), pGambleRewardItem->GetGambleRewardItemID(), dwRIID);

		pObj->m_DBJobQ.DBJobQ.push_back( pJob );
	}
	else
	{
		// 겜블 랜덤함수를 호출하여 결과 아이템의 ID를 가져온다. //
		const MMatchGambleRewardItem* pGRewardItem = GetGambleMachine().Gamble(pGambleItem->GetGambleItemID());
		if (pGRewardItem == NULL) {
			mlog("Gamble() failed! [ItemID : %d]\n", pGambleItem->GetGambleItemID());
			ASSERT(0);
			return;
		}

		DWORD dwRIID;
		DWORD dwItemCnt;

		if( MMS_MALE == pCharInfo->m_nSex )			dwRIID = pGRewardItem->GetItemIDMale();
		else if ( MMS_FEMALE == pCharInfo->m_nSex )	dwRIID = pGRewardItem->GetItemIDFemale();

		dwItemCnt = pGRewardItem->GetItemCnt();

		bool bIsSpendable = false;

		MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(dwRIID);
		if( pDesc != NULL ) {
			bIsSpendable = pDesc->IsSpendableItem();

			if( !bIsSpendable && dwItemCnt > 1 ) {
				_ASSERT(0);
				dwItemCnt = 1;
			}
		}
		else {
			if( GetGambleMachine().GetGambleItemByGambleItemID(dwRIID) != NULL ) {
				bIsSpendable = true;
			} else {
				_ASSERT(0);
				return;
			}
		}

		MBMatchAsyncDBJob_GambleItem* pJob = new MBMatchAsyncDBJob_GambleItem(uidPlayer);
		pJob->Input( uidPlayer, pGambleItem->GetUID(), pCharInfo->m_nCID, pGambleItem->GetCIID(), pGambleItem->GetGambleItemID(), 
			pGRewardItem->GetGambleRewardItemID(), dwRIID, pGRewardItem->GetItemCnt(), pGRewardItem->GetRentHourPeriod(), pDesc->IsSpendableItem());

		pObj->m_DBJobQ.DBJobQ.push_back( pJob );
	}
}

MCommandParameterBlob* MBMatchServer::MakeShopGambleItemListBlob() const
{
	vector<DWORD>			GambleItemIndexVec;
	MTD_GambleItemNode*		pSendGmableIItem;

	// 겜블 아이템 리스트를 담아서 보낸다.
	const MMatchGambleMachine& GambleMachine = GetGambleMachine();

	// 겜블 아이템의 사용 시간을 검사해서 보내야 하는 겜블 아이템을 백터에 담아온다. //
	GambleMachine.GetItemVectorByCheckedItemTime(GambleItemIndexVec, GetGlobalClockCount());

	const DWORD dwGambleItemSize = (DWORD)GambleItemIndexVec.size();

	// 아이템의 개수만큼 필요한 메모리 블럭을 할당한다. //
	void* pGambleItemArray = MMakeBlobArray(sizeof(MTD_GambleItemNode), dwGambleItemSize);

	for (DWORD j = 0; j < dwGambleItemSize; ++j)
	{
		// 벡터에 담긴 겜블 아이템의 인덱스로 보내야 하는 겜블 아이템을 찾아온다. //
		const MMatchGambleItem* pGambleItem = GambleMachine.GetGambleItemByIndex(GambleItemIndexVec[j]);

		// 보내야 할 겜블 아이템 담길 내용 원하는 메모리 블록을 찾아서 기록한다. //
		pSendGmableIItem = (MTD_GambleItemNode*)MGetBlobArrayElement(pGambleItemArray, j);
		if (pSendGmableIItem != NULL)
		{
			pSendGmableIItem->nItemID = pGambleItem->GetGambleItemID();
			pSendGmableIItem->nItemCnt = 1;
		}
		else
		{
			ASSERT( 0 );
		}
	}

	MCommandParameterBlob* pGItemBlobParam = new MCommandParameterBlob( pGambleItemArray, MGetBlobArraySize(pGambleItemArray) );
	MEraseBlobArray(pGambleItemArray);

	return pGItemBlobParam;
}


MCommandParameterBlob* MBMatchServer::MakeShopItemListBlob( const int nFirstItemIndex, const int nItemCount ) const
{
	int nRealItemCount = nItemCount;

	if ((nItemCount <= 0) || (nItemCount > MGetMatchShop()->GetCount())) {
		nRealItemCount = MGetMatchShop()->GetCount();
	}

	void* pItemArray = MMakeBlobArray(sizeof(MTD_ShopItemInfo), nRealItemCount);
	int nIndex=0;

	for (int i = nFirstItemIndex; i < nFirstItemIndex+nRealItemCount; i++)
	{
		MTD_ShopItemInfo *pInfo = (MTD_ShopItemInfo *)MGetBlobArrayElement(pItemArray, nIndex++);
		ShopItemNode* pSINode = MGetMatchShop()->GetSellItemByIndex(i);

		if (pSINode != NULL) {
			pInfo->nItemID		= pSINode->nItemID;
			pInfo->nItemCount	= pSINode->nItemCount;
		} else {			
			pInfo->nItemID		= 0;
			pInfo->nItemCount	= 0;

			ASSERT( 0 );
		}

	}

	MCommandParameterBlob* pItemListBlob = new MCommandParameterBlob( pItemArray, MGetBlobArraySize(pItemArray) );
	MEraseBlobArray(pItemArray);

	return pItemListBlob;
}


void MBMatchServer::OnRequestBuyItem(const MUID& uidPlayer, const unsigned long int nItemID, const int nItemCount)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if( NULL == pCharInfo ) return;

	DWORD	dwPrice;
	WORD	nRentHourPeriod	= RENT_PERIOD_UNLIMITED;
	bool	bIsGambleItem	= false;
	bool	bIsSpendableItem = false;

	// Shop에서 판매하는지 먼저 검사하고 GambleItem인지 한번더 검사 한다.
	if( MGetMatchShop()->IsSellItem(nItemID) )
	{
		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc( nItemID );
		if (pItemDesc == NULL) return;

		if( !pItemDesc->IsUnLimitItem() ) {
			nRentHourPeriod = pItemDesc->m_nMaxRentPeriod.Ref() * 24;	// 시간단위로 변환(DB 저장용)
		}

		if( pItemDesc->IsSpendableItem() ) {
			bIsSpendableItem = true;
		}

		dwPrice = pItemDesc->m_nBountyPrice.Ref() * nItemCount;
	}
	else
	{
		if( GetGambleMachine().IsItTimeoverEventGambleItem(nItemID, GetGlobalClockCount()) )
		{
			MCommand* pNew = CreateCommand( MC_MATCH_RESPONSE_BUY_ITEM, pObj->GetUID() );
			pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_BUY_ITEM));
			PostSafeQueue( pNew );
			return;
		}

		const MMatchGambleItem* pGItem = GetGambleMachine().GetGambleItemByGambleItemID( nItemID );
		if( NULL == pGItem ) return;

		if( pGItem->IsCash() ) {
			MCommand* pNew = CreateCommand( MC_MATCH_RESPONSE_BUY_ITEM, pObj->GetUID() );
			pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_BUY_ITEM));
			PostSafeQueue( pNew );

			return;
		}

		dwPrice			 = pGItem->GetPrice() * nItemCount;
		bIsGambleItem	 = true;
		bIsSpendableItem = true;
	}

	if( !CheckUserCanBuyItem(pObj, nItemID, nItemCount, dwPrice) ) { return; }

	ResponseBuyItem( pObj, nItemID, nItemCount, bIsGambleItem, dwPrice, nRentHourPeriod, bIsSpendableItem);
}

bool MBMatchServer::ResponseBuyItem( MMatchObject* pObj, const unsigned long int nItemID, const int nItemCount
									, const bool bIsGambleItem, const DWORD dwPrice, const WORD wRentHourPeriod, const bool bIsSpendableItem )
{
	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();

	MBMatchAsyncDBJob_BuyBountyItem* pBuyBountyItemJob = new MBMatchAsyncDBJob_BuyBountyItem(pObj->GetUID());
	if( NULL == pBuyBountyItemJob ) { return false; }

	pBuyBountyItemJob->Input( pObj->GetUID(), pCharInfo->m_nCID, nItemID
		, nItemCount, dwPrice, wRentHourPeriod, bIsGambleItem, bIsSpendableItem );

	pObj->m_DBJobQ.DBJobQ.push_back( pBuyBountyItemJob );

	return true;
}

// TodoH(하) - 추후에 다시 체크체크!
const bool MBMatchServer::CheckUserCanBuyItem( MMatchObject* pObj, const int nItemID, const int nItemCnt, const DWORD dwPrice )
{
	if( NULL == pObj ) return false;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if( NULL == pCharInfo ) return false;

	MMatchItem* pItem = pCharInfo->m_ItemList.GetItemByItemID(nItemID);
	if( pItem != NULL )
	{
		MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
		if( pDesc && pDesc->IsSpendableItem() ) 
		{
			if( pItem->GetItemCount() + nItemCnt > MAX_SPENDABLE_ITEM_COUNT ) 
			{
				MCommand* pNew = CreateCommand( MC_MATCH_RESPONSE_BUY_ITEM, pObj->GetUID() );
				pNew->AddParameter(new MCmdParamInt(MERR_TOO_MANY_ITEM));
				PostSafeQueue( pNew );

				return false;
			}
		}
	}
	else 
	{
		const MMatchCharGambleItem* pGambleItem = pCharInfo->m_GambleItemManager.GetGambleItemByItemID(nItemID);
		if( pGambleItem != NULL )
		{
			if( pGambleItem->GetItemCount() + nItemCnt > MAX_SPENDABLE_ITEM_COUNT ) 
			{
				MCommand* pNew = CreateCommand( MC_MATCH_RESPONSE_BUY_ITEM, pObj->GetUID() );
				pNew->AddParameter(new MCmdParamInt(MERR_TOO_MANY_ITEM));
				PostSafeQueue( pNew );

				return false;
			}
		}
	}

	const DWORD dwTotalItemCount = pCharInfo->m_ItemList.GetCount() + pCharInfo->m_GambleItemManager.GetCount();
	if( dwTotalItemCount >= MAX_ITEM_COUNT )
	{
		MCommand* pNew = CreateCommand( MC_MATCH_RESPONSE_BUY_ITEM, pObj->GetUID() );
		pNew->AddParameter(new MCmdParamInt(MERR_TOO_MANY_ITEM));
		PostSafeQueue( pNew );

		return false;
	}

	// 아이템을 살 수 있는 자격이 되는지 판정
	if (static_cast<DWORD>(pCharInfo->m_nBP) < dwPrice)
	{
		MCommand* pNew = CreateCommand( MC_MATCH_RESPONSE_BUY_ITEM, pObj->GetUID() );
		pNew->AddParameter(new MCmdParamInt(MERR_TOO_EXPENSIVE_BOUNTY));
		PostSafeQueue( pNew );

		return false;
	}

	return true;
}

bool MBMatchServer::CheckUserCanDistributeRewardItem( MMatchObject* pObj )
{
	if( NULL == pObj ) return false;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if( NULL == pCharInfo ) return false;

	const DWORD dwTotalItemCount = pCharInfo->m_ItemList.GetCount() + pCharInfo->m_GambleItemManager.GetCount();
	if( dwTotalItemCount >= MAX_QUEST_REWARD_ITEM_COUNT )
	{
		MCommand* pNew = CreateCommand( MC_MATCH_RESPONSE_BUY_ITEM, pObj->GetUID() );
		pNew->AddParameter(new MCmdParamInt(MERR_TOO_MANY_ITEM));
		PostSafeQueue( pNew );

		return false;
	}

	return true;
}

void MBMatchServer::OnRequestCharacterItemList(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject( uidPlayer );
	if( NULL == pObj )
		return;

	if( NULL == pObj->GetCharInfo() )
		return;

	if( pObj->GetCharInfo()->m_IsSendMyItemListByRequestClient )
		return;

	if( ResponseCharacterItemList(uidPlayer) )
		pObj->GetCharInfo()->m_IsSendMyItemListByRequestClient = true;
}

void MBMatchServer::OnRequestCharacterItemListForce(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject( uidPlayer );
	if( NULL == pObj )
		return;

	if( NULL == pObj->GetCharInfo() )
		return;

	if( ResponseCharacterItemList(uidPlayer) )
		pObj->GetCharInfo()->m_IsSendMyItemListByRequestClient = true;
}


bool MBMatchServer::ResponseCharacterItemList(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) 
	{
		mlog("ResponseCharacterItemList > pObj or pObj->GetCharInfo() IS NULL\n");
		return false;
	}

	// 만약 DB가 업데이트 되었다면 상점을 다시 구성한다.
	// 용량은 크지만 자주 일어나는 일이 아니다. - by SungE 2007-06-28
	if( SendGambleItemList(pObj) ) {
		ResponseShopItemList( uidPlayer, 0, 0 );
	}

	// 이전에 디비 억세스를 안했었으면 디비에서 아이템 정보를 가져온다
	if (!pObj->GetCharInfo()->m_ItemList.IsDoneDbAccess())
	{
		mlog("start DB Query(ResponseCharacterItemList > GetCharItemInfo). \n");
		if (!m_MatchDBMgr.GetCharItemInfo(pObj->GetCharInfo()))
		{
			mlog("DB Query(ResponseCharacterItemList > GetCharItemInfo) Failed\n");
			return false;
		}
	}

	if( MSM_QUEST == MGetServerConfig()->GetServerMode() ) 
	{
		if( !pObj->GetCharInfo()->m_QuestItemList.IsDoneDbAccess() )
		{
			if( !m_MatchDBMgr.GetCharQuestItemInfo(pObj->GetCharInfo()) )
			{
				mlog( "ResponseCharacterItemList - GetCharQuestItemInfo fail.\n" );
				return false;
			}
		}
	}

	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_CHARACTER_ITEMLIST, uidPlayer);

	// 바운티 전송
	pNew->AddParameter(new MCommandParameterInt(pObj->GetCharInfo()->m_nBP));

	// 장비한 아이템 전송
	int nIndex = 0;
	int nRealEquipedItemCount = 0;	
	void* pEquipItemArray = MMakeBlobArray(sizeof(MUID), MMCIP_END);

	for (int i = 0; i < MMCIP_END; i++) {
		MUID* pUID = (MUID*)MGetBlobArrayElement(pEquipItemArray, nIndex++);

		if (!pObj->GetCharInfo()->m_EquipedItem.IsEmpty(MMatchCharItemParts(i))) {
			MMatchItem* pItem = pObj->GetCharInfo()->m_EquipedItem.GetItem(MMatchCharItemParts(i));
			if( pItem != NULL ) {
				*pUID = pItem->GetUID();
				nRealEquipedItemCount++;
			} else {
				mlog("Invalid Equiped Item - part(%d), CID(%d)\n", i, pObj->GetCharInfo()->m_nCID);
				*pUID = MUID(0,0);
			}			
		} else {
			*pUID = MUID(0,0);
		}
	}

	pNew->AddParameter(new MCommandParameterBlob(pEquipItemArray, MGetBlobArraySize(pEquipItemArray)));
	MEraseBlobArray(pEquipItemArray);


	// 갖고 있는 아이템 리스트 전송
	int nItemTotalCount = pObj->GetCharInfo()->m_ItemList.GetCount();

	void*			pItemArray					= MMakeBlobArray(sizeof(MTD_ItemNode), nItemTotalCount);
	MMatchItemMap*	pItemList					= &pObj->GetCharInfo()->m_ItemList;
	MMatchItem*		pItem						= NULL;
	MTD_ItemNode*	pItemNode					= NULL;
	int				nItemCount					= 0;
	unsigned long	nPassTime					= 0;
	int				nPassMinuteTime				= 0;
	int				iMaxUseHour					= 0;
	int				nRentMinutePeriodRemainder	= RENT_MINUTE_PERIOD_UNLIMITED;

	nIndex = 0;
	for (MMatchItemMap::iterator itor = pItemList->begin(); itor != pItemList->end(); ++itor) {
		pItem = (*itor).second;

		if(pItem == NULL) {
			MUID id = (*itor).first;
			LOG(LOG_FILE, "there is unkown error in itemlist. Add:%d Count:%d, No:%d, UID:%d|%d\n"
				, pItemList, pItemList->GetCount(), nIndex, id.High, id.Low );
			continue;
		}

		pItemNode					= (MTD_ItemNode*)MGetBlobArrayElement(pItemArray, nIndex++);
		nPassTime					= MGetTimeDistance(pItem->GetRentItemRegTime(), GetTickTime());
		nPassMinuteTime				= nPassTime / (1000 * 60);
		iMaxUseHour					= 0;		// 최대 사용시간 보내기(판매가격 계산을 위해)
		nRentMinutePeriodRemainder	= RENT_MINUTE_PERIOD_UNLIMITED;
		nItemCount					= pItem->GetItemCount();

		if( nItemCount <= 0 ) { continue; }

		if (pItem->IsRentItem()) {
			nRentMinutePeriodRemainder	= pItem->GetRentMinutePeriodRemainder() - nPassMinuteTime;
			iMaxUseHour					= pItem->GetRentHourPeriod();
		}

		Make_MTDItemNode(pItemNode
			, pItem->GetUID()
			, pItem->GetDescID()
			, nRentMinutePeriodRemainder
			, iMaxUseHour
			, nItemCount);
	}

	pNew->AddParameter(new MCommandParameterBlob(pItemArray, MGetBlobArraySize(pItemArray)));
	MEraseBlobArray(pItemArray);


	// 가지고 있는 겜블 아이템 리스트 전송
	int nGambleItemCount	= pObj->GetCharInfo()->m_GambleItemManager.GetCount();
	void* pGambleItemArray	= MMakeBlobArray(sizeof(MTD_GambleItemNode), nGambleItemCount);

	for (int j = 0; j < nGambleItemCount; ++j) {
		const MMatchCharGambleItem* pGambleItem = pObj->GetCharInfo()->m_GambleItemManager.GetGambleItemByIndex(j);
		MTD_GambleItemNode* pSendGambleItem = (MTD_GambleItemNode*)MGetBlobArrayElement(pGambleItemArray, j);
		if( pSendGambleItem != NULL ) {
			pSendGambleItem->uidItem = pGambleItem->GetUID();
			pSendGambleItem->nItemID = pGambleItem->GetGambleItemID();	
			pSendGambleItem->nItemCnt = pGambleItem->GetItemCount();
		} else {
			ASSERT( 0 );
		}
	}

	pNew->AddParameter(new MCommandParameterBlob(pGambleItemArray, MGetBlobArraySize(pGambleItemArray)));
	MEraseBlobArray(pGambleItemArray);

	PostSafeQueue( pNew );

	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sell Item 관련하여...

void MBMatchServer::OnRequestSellItem(const MUID& uidPlayer, const MUID& uidItem, int nItemCount)
{
	MMatchObject* pObj = GetObject( uidPlayer );
	if( NULL == pObj ) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if( NULL == pCharInfo ) return;


	if( NULL != pCharInfo->m_ItemList.GetItem(uidItem) ) 
	{
		MMatchItem* pItem = pCharInfo->m_ItemList.GetItem(uidItem);
		if( pItem == NULL ) return;
		if( pItem->GetDesc() == NULL ) return;

		if( !pItem->GetDesc()->IsSpendableItem() ) 	ResponseSellItem(uidPlayer, uidItem);			
		else										ResponseSellSpendableItem(uidPlayer, uidItem, nItemCount);
	} 
	else if( NULL != pCharInfo->m_GambleItemManager.GetGambleItemByUID(uidItem) ) 
	{
		ResponseSellGItem( uidPlayer, uidItem, nItemCount );	///< Gamble Item
	}

	return;
}

bool MBMatchServer::IsValidSellItem(const MUID& uidPlayer, const MUID& uidItem, int nItemCount)
{
	MMatchObject* pObj = GetObject( uidPlayer );
	if( pObj == NULL ) return false;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if( pCharInfo == NULL ) return false;

	MMatchItem* pItem = pObj->GetCharInfo()->m_ItemList.GetItem(uidItem);

	if ((pItem == NULL) || (pItem->GetDesc() == NULL))
	{	///< 아이템이 존재하지 않을때
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_SELL_NONE_ITEM));
		RouteToListener(pObj, pNew);
		return false;
	}
	
	if (pItem->IsEquiped())
	{	///< 장비하고 있으면 팔 수 없다.
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_SELL_EQUIPED_ITEM));
		RouteToListener(pObj, pNew);
		return false;
	}

	
	if( !pItem->GetDesc()->IsSpendableItem() ) 
	{
		if( nItemCount > 1)
		{
			MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, MUID(0,0));
			pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_SELL_NOT_SPENDABLEITEM));
			RouteToListener(pObj, pNew);

			return false;
		}
#ifndef _SELL_CASHITEM
		if( pItem->GetDesc()->IsCashItem() ) {
			MCommand* pNew = CreateCommand( MC_MATCH_RESPONSE_SELL_ITEM, pObj->GetUID() );
			pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_SELL_CASHITEM));
			PostSafeQueue( pNew );
			return false;
		}
#endif
	}
	else 
	{		
		if( pItem->GetItemCount() < nItemCount )
		{	///< 팔고자 하는 아이템의 개수가 소지하고 있는 개수보다 많을 경우, 팔 수 없다.
			MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, MUID(0,0));
			pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_SELL_TOO_MANY_ITEMCOUNT));
			RouteToListener(pObj, pNew);

			return false;
		}

		if( pItem->GetDesc()->IsCashItem() ) 
		{	///< 소모성 아이템은 캐쉬템으로 팔 수가 없다.
			MCommand* pNew = CreateCommand( MC_MATCH_RESPONSE_SELL_ITEM, pObj->GetUID() );
			pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_SELL_CASHITEM));
			PostSafeQueue( pNew );
			return false;
		}
	}

	return true;
}

bool MBMatchServer::ResponseSellItem(const MUID& uidPlayer, const MUID& uidItem)
{
	MMatchObject* pObj = GetObject( uidPlayer );
	if( pObj == NULL ) return false;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if( pCharInfo == NULL ) return false;

	if( IsValidSellItem(uidPlayer, uidItem) == false ) return false;

	/////////////////////////////////////////////////////////////////////////////////////////////
	UpdateCharDBCachingData(pObj);		///< XP, BP, KillCount, DeathCount 캐슁 업데이트
	/////////////////////////////////////////////////////////////////////////////////////////////

	MMatchItem* pItem = pObj->GetCharInfo()->m_ItemList.GetItem(uidItem);
	if( pItem == NULL || pItem->GetDesc() == NULL ) return false;
	if( !pItem->GetDesc()->IsCashItem() )
	{
		int nPrice;
		
		if ( !pItem->IsRentItem() ) 
		{	///< 무제한 바운티 아이템
			nPrice = pItem->GetDesc()->GetSellBountyValue();		
		} 
		else 
		{	///< 기간제 바운티 아이템
			unsigned long	nPassTime		= MGetTimeDistance(pItem->GetRentItemRegTime(), GetTickTime()); // 1/1000초.
			int				nPassMinuteTime = nPassTime / (60000);											// 지나간 시간을 분단위로 변경.

			nPrice = CalculateRentItemSellBounty( pItem->GetDesc()->m_nBountyPrice.Ref(),
												pItem->GetRentHourPeriod(), 
												(pItem->GetRentMinutePeriodRemainder() - nPassMinuteTime) / 60,
                                                RENT_PERIOD_UNLIMITED == pItem->GetDesc()->m_nMaxRentPeriod.Ref() );
			
		}

		MBMatchAsyncDBJob_SellItem* pJob = new MBMatchAsyncDBJob_SellItem(uidPlayer, MASYNCJOB_SELL_ITEM_TO_BOUNTY);
		pJob->Input( uidPlayer, uidItem, pCharInfo->m_nCID, pItem->GetCIID(), pItem->GetDescID(), nPrice, 1, pCharInfo->m_nBP + nPrice);
		pObj->m_DBJobQ.DBJobQ.push_back( pJob );
	}
#ifdef _SELL_CASHITEM
	else
	{
		int nPrice;

		int nResLevel = pItem->GetDesc()->m_nResLevel.Ref();
		int nRentMinuteRemainder = pItem->GetRentMinutePeriodRemainder();
		MMatchItemSlotType slotType = pItem->GetDesc()->m_nSlot;

		nPrice = CalculateCashItemToBounty(CASHITEM_NORMAL, nResLevel, nRentMinuteRemainder, slotType);

		MBMatchAsyncDBJob_SellItem* pJob = new MBMatchAsyncDBJob_SellItem(uidPlayer, MASYNCJOB_SELL_ITEM_TO_BOUNTY);
		pJob->Input( uidPlayer, uidItem, pCharInfo->m_nCID, pItem->GetCIID(), pItem->GetDescID(), nPrice, 1, pCharInfo->m_nBP + nPrice);
		pObj->m_DBJobQ.DBJobQ.push_back( pJob );
	}
#endif

	return true;
}

bool MBMatchServer::ResponseSellSpendableItem(const MUID& uidPlayer, const MUID& uidItem, int nItemCount)
{
	MMatchObject* pObj = GetObject( uidPlayer );
	if( pObj == NULL ) return false;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if( pCharInfo == NULL ) return false;

	if( IsValidSellItem(uidPlayer, uidItem, nItemCount) == false ) return false;

	/////////////////////////////////////////////////////////////////////////////////////////////
	UpdateCharDBCachingData(pObj);		///< XP, BP, KillCount, DeathCount 캐슁 업데이트
	/////////////////////////////////////////////////////////////////////////////////////////////

	MMatchItem* pItem = pObj->GetCharInfo()->m_ItemList.GetItem(uidItem);
	if( pItem == NULL || pItem->GetDesc() == NULL ) return false;
	if( !pItem->GetDesc()->IsCashItem() )
	{
		int nPrice;
		
		if ( !pItem->IsRentItem() ) 
		{	///< 무제한 바운티 아이템
			nPrice = pItem->GetDesc()->GetSellBountyValue(nItemCount);		
		} 
		else 
		{	///< 기간제 바운티 아이템
			unsigned long	nPassTime		= MGetTimeDistance(pItem->GetRentItemRegTime(), GetTickTime()); // 1/1000초.
			int				nPassMinuteTime = nPassTime / (60000);											// 지나간 시간을 분단위로 변경.

			nPrice = CalculateRentItemSellBounty( pItem->GetDesc()->m_nBountyPrice.Ref() * pItem->GetItemCount(),
												pItem->GetRentHourPeriod(), 
												(pItem->GetRentMinutePeriodRemainder() - nPassMinuteTime) / 60,
                                                RENT_PERIOD_UNLIMITED == pItem->GetDesc()->m_nMaxRentPeriod.Ref() );
			
		}

		MBMatchAsyncDBJob_SellItem* pJob = new MBMatchAsyncDBJob_SellItem(uidPlayer, MASYNCJOB_SELL_SPENDABLEITEM_TO_BOUNTY);
		pJob->Input( uidPlayer, uidItem, pCharInfo->m_nCID, pItem->GetCIID(), pItem->GetDescID(), nPrice, nItemCount, pCharInfo->m_nBP + nPrice);
		pObj->m_DBJobQ.DBJobQ.push_back( pJob );
	}

	return true;
}

bool MBMatchServer::ResponseSellGItem( const MUID& uidPlayer, const MUID& uidItem, int nItemCount)
{
	MMatchObject*	pObj		= GetObject( uidPlayer );
	MMatchCharInfo* pCharInfo	= pObj->GetCharInfo();

	const MMatchCharGambleItem* pCharGItem = pCharInfo->m_GambleItemManager.GetGambleItemByUID( uidItem );

	const MMatchGambleItem* pGItem = GetGambleMachine().GetGambleItemByGambleItemID( pCharGItem->GetGambleItemID() );
	if( NULL == pGItem ) return false;

#ifndef _SELL_CASHITEM
	if( pGItem->IsCash() ) {
		MCommand* pNew = CreateCommand( MC_MATCH_RESPONSE_SELL_ITEM, pObj->GetUID() );
		pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_SELL_CASHITEM));
		PostSafeQueue( pNew );
		return false;
	}
#endif
	
	if( pCharGItem->GetItemCount() < nItemCount )
	{	///< 팔고자 하는 아이템의 개수가 소지하고 있는 개수보다 많을 경우, 팔 수 없다.
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_SELL_TOO_MANY_ITEMCOUNT));
		RouteToListener(pObj, pNew);

		return false;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	UpdateCharDBCachingData(pObj);		///< XP, BP, KillCount, DeathCount 캐슁 업데이트
	/////////////////////////////////////////////////////////////////////////////////////////////

	if( !pGItem->IsCash() ) 
	{
		int nSellPrice = (pGItem->GetPrice() / 4) * nItemCount;

		MBMatchAsyncDBJob_SellItem* pJob = new MBMatchAsyncDBJob_SellItem(uidPlayer, MASYNCJOB_SELL_SPENDABLEITEM_TO_BOUNTY);
		pJob->Input( uidPlayer, uidItem, pCharInfo->m_nCID, pCharGItem->GetCIID(), 
			pCharGItem->GetGambleItemID(), nSellPrice, nItemCount, pCharInfo->m_nBP + nSellPrice);

		pObj->m_DBJobQ.DBJobQ.push_back( pJob );
	}
#ifdef _SELL_CASHITEM
	else
	{
		int nSellPrice = CalculateCashItemToBounty(CASHITEM_GAMBLE) * nItemCount;

		MBMatchAsyncDBJob_SellItem* pJob = new MBMatchAsyncDBJob_SellItem(uidPlayer, MASYNCJOB_SELL_SPENDABLEITEM_TO_BOUNTY);
		pJob->Input( uidPlayer, uidItem, pCharInfo->m_nCID, pCharGItem->GetCIID(), 
			pCharGItem->GetGambleItemID(), nSellPrice, nItemCount, pCharInfo->m_nBP + nSellPrice);

		pObj->m_DBJobQ.DBJobQ.push_back( pJob );
	}
#endif

	return true;
}

void MBMatchServer::OnRequestBringAccountItem(const MUID& uidPlayer, unsigned int nAIID, unsigned int nItemID, unsigned int nItemCnt)
{
	/**
		현재 중앙은행 BRING 정책

		1. 모든 바운티 아이템				- 이동 불가능		
		2. 캐쉬 일반 아이템					- 기존대로 이동(합쳐지지 않음)
		3. 캐쉬 겜블 아이템, 소모성 아이템 	- 합쳐져서 이동(아이템 분리는 아이템을 중앙은행으로 보내는 UI에서 수행하도록 한다)
	*/

	MMatchObject* pObj = GetObject( uidPlayer );
	if( NULL == pObj ) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if( NULL == pCharInfo ) return;

	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if( pItemDesc ) 
	{
		if( !pItemDesc->IsSpendableItem() )
		{
			MBMatchAsyncDBJob_BringAccountItem* pJob = new MBMatchAsyncDBJob_BringAccountItem(pObj->GetUID());
			pJob->Input(pObj->GetAccountInfo()->m_nAID, pObj->GetCharInfo()->m_nCID, nAIID);
			pObj->m_DBJobQ.DBJobQ.push_back( pJob );
		}
		else								
		{			
			unsigned int nCIID;
			MMatchItem* pCharItem;

			pCharItem = pCharInfo->m_ItemList.GetItemByItemID(nItemID);
			if( pCharItem ) { nCIID = pCharItem->GetCIID(); }
			else			{ nCIID = 0;				}

			MBMatchAsyncDBJob_BringAccountItemStackable* pJob = new MBMatchAsyncDBJob_BringAccountItemStackable(pObj->GetUID());
			pJob->Input(pObj->GetAccountInfo()->m_nAID, pObj->GetCharInfo()->m_nCID, nAIID, nCIID, nItemCnt);
			pObj->m_DBJobQ.DBJobQ.push_back( pJob );
		}
	} 
	else 
	{
		const MMatchGambleItem *pGItemDesc = m_GambleMachine.GetGambleItemByGambleItemID(nItemID);		
		if( pGItemDesc ) 
		{
			unsigned int nCIID;
			const MMatchCharGambleItem* pCharGItem;

			pCharGItem = pCharInfo->m_GambleItemManager.GetGambleItemByItemID(nItemID);
			if( pCharGItem )	{ nCIID = pCharGItem->GetCIID();}
			else				{ nCIID = 0;					}

			MBMatchAsyncDBJob_BringAccountItemStackable* pJob = new MBMatchAsyncDBJob_BringAccountItemStackable(pObj->GetUID());
			pJob->Input(pObj->GetAccountInfo()->m_nAID, pObj->GetCharInfo()->m_nCID, nAIID, nCIID, nItemCnt);
			pObj->m_DBJobQ.DBJobQ.push_back( pJob );
		}
	}	
}

void MBMatchServer::OnRequestBringBackAccountItem(const MUID& uidPlayer, const MUID& uidItem, unsigned int nItemCount)
{
	/**
		현재 중앙은행 BRING BACK 정책

		1. 모든 바운티 아이템				- 이동 불가능
		2. 캐쉬 일반 아이템					- 기존대로 이동(합쳐지지 않음)
		3. 캐쉬 겜블 아이템, 겜블 아이템	- 합쳐져서 이동(분리는 중앙은행 UI에서 수행하도록 한다)
	*/

	MMatchObject* pObj = GetObject( uidPlayer );
	if( NULL == pObj ) return;
	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if( NULL == pCharInfo ) return;

	if( NULL != pCharInfo->m_ItemList.GetItem(uidItem) ) 
	{
		MMatchItem* pItem = pCharInfo->m_ItemList.GetItem(uidItem);

		if( NULL == pItem || pItem->GetDesc() == NULL ) 
		{	///< 아이템이 존재하지 않을때
			MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, uidPlayer);
			pNew->AddParameter(new MCmdParamInt(MERR_BRING_BACK_ACCOUNTITEM));
			PostSafeQueue( pNew );
			return;
		}
		else if ( nItemCount <= 0 || pItem->GetItemCount() < nItemCount )
		{	///< 갯수 범위가 잘못된 경우
			MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, MUID(0,0));
			pNew->AddParameter(new MCmdParamInt(MERR_BRING_BACK_ACCOUNTITEM));
			RouteToListener(pObj, pNew);
			return;
		}
		else if (pItem->IsEquiped()) 
		{	///< 장비하고 있으면 옮길 수 없다
			MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, MUID(0,0));
			pNew->AddParameter(new MCmdParamInt(MERR_BRING_BACK_ACCOUNTITEM));
			RouteToListener(pObj, pNew);
			return;
		}		
		else if (!pItem->GetDesc()->IsCashItem()) 
		{	///< 캐쉬아이템이 아니면 옮길 수 없다
			MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, MUID(0,0));
			pNew->AddParameter(new MCmdParamInt(MERR_BRING_BACK_ACCOUNTITEM_FOR_CASHITEM));
			RouteToListener(pObj, pNew);
			return;
		}
		
		if( !pItem->GetDesc()->IsSpendableItem() )	
		{
			MBMatchAsyncDBJob_BringBackAccountItem* pJob = new MBMatchAsyncDBJob_BringBackAccountItem(uidPlayer);
			pJob->Input( uidItem, pObj->GetAccountInfo()->m_nAID, pObj->GetCharInfo()->m_nCID, pItem->GetCIID());
			pObj->m_DBJobQ.DBJobQ.push_back( pJob );
		}
		else
		{
			MBMatchAsyncDBJob_BringBackAccountItemStackable* pJob = new MBMatchAsyncDBJob_BringBackAccountItemStackable(uidPlayer);
			pJob->Input( uidItem, pObj->GetAccountInfo()->m_nAID, pCharInfo->m_nCID, pItem->GetCIID(), nItemCount );
			pObj->m_DBJobQ.DBJobQ.push_back( pJob );	
		}
	} 
	else if( NULL != pCharInfo->m_GambleItemManager.GetGambleItemByUID(uidItem) ) 
	{
		const MMatchCharGambleItem* pCharGItem = pCharInfo->m_GambleItemManager.GetGambleItemByUID(uidItem);

		MBMatchAsyncDBJob_BringBackAccountItemStackable* pJob = new MBMatchAsyncDBJob_BringBackAccountItemStackable(uidPlayer);
		pJob->Input( uidItem, pObj->GetAccountInfo()->m_nAID, pCharInfo->m_nCID, pCharGItem->GetCIID(), nItemCount );
		pObj->m_DBJobQ.DBJobQ.push_back( pJob );
	}
}

const bool MBMatchServer::SendGambleItemList( const MMatchObject* pObj )
{
	static const DWORD dwMTD_DBGambleItemNodeSize = sizeof( MTD_DBGambleItmeNode );

	MMatchGambleMachine& GMachine = GetGambleMachine();

	// 겜블 아이템이 없으면 보내지 않는다.
	if( 0 == GMachine.GetGambleItemSize() )
	{
		return true;
	}

	if( NULL == pObj )
		return false;

	// DB의 GambleItem가 유저가 가지고 있는거 보다 최신이면 업데이트 한다.
	if( GetGambleMachine().GetLastUpdateTime() <= pObj->GetLastSendGambleItemTime() )
		return false;

	vector< DWORD >				vOpenedGItemList;
	vector< DWORD >::iterator	it, end;
	MTD_DBGambleItmeNode*		pSendGItem;
	void*						pGambleItemArray;
	DWORD						dwOpenedGItemSize;

	GMachine.GetOpenedGambleItemList( vOpenedGItemList );

	dwOpenedGItemSize	= static_cast< DWORD >( vOpenedGItemList.size() );
	pGambleItemArray	= MMakeBlobArray( dwMTD_DBGambleItemNodeSize, dwOpenedGItemSize );

	if( NULL == pGambleItemArray )
		return false;

	for( DWORD i = 0; i < dwOpenedGItemSize; ++i )
	{
		const MMatchGambleItem* pGItem = GMachine.GetGambleItemByIndex( vOpenedGItemList[i] );

		pSendGItem = (MTD_DBGambleItmeNode*)MGetBlobArrayElement( pGambleItemArray, i );
		if( NULL == pSendGItem )
		{
			MEraseBlobArray( pGambleItemArray );
#ifdef _GAMBLEITEM_TEST_LOG
			mlog("SendGambleItemList Fail [MTD_DBGambleItmeNode]\n");
#endif
			return false;
		}

		memset( pSendGItem->szName, 0, MAX_GAMBLEITEMNAME_LEN );
		memset( pSendGItem->szDesc, 0, MAX_GAMBLEITEMDESC_LEN );

		// 복사하기전에 길이가 정상인지 확인한다.
		_ASSERT(MAX_GAMBLEITEMNAME_LEN >= pGItem->GetName().length());
		_ASSERT(MAX_GAMBLEITEMDESC_LEN >= pGItem->GetDescription().length());

		// 정상적이지 않은 데이터면 작업을 중단 한다.
		if ((MAX_GAMBLEITEMNAME_LEN < pGItem->GetName().length()) 
			|| (MAX_GAMBLEITEMDESC_LEN < pGItem->GetDescription().length()))
		{
			MEraseBlobArray( pGambleItemArray );
#ifdef _GAMBLEITEM_TEST_LOG
			mlog("SendGambleItemList Fail [Size:%d, Count:%d, ID:%d, NameLength(Max25):%d, DescLength(Max65):%d]\n", 
				dwOpenedGItemSize, i, pGItem->GetGambleItemID(), pGItem->GetName().length(), pGItem->GetDescription().length());
#else
			// 간단하게라도 로그는 남기자 -_-
			mlog("SendGambleItemList error ID:%d, too long name or desc!", pGItem->GetGambleItemID());
#endif
			return false;
		}

		memcpy( pSendGItem->szName
			, pGItem->GetName().c_str()
			, pGItem->GetName().length() );

		memcpy( pSendGItem->szDesc
			, pGItem->GetDescription().c_str()
			, pGItem->GetDescription().length() );

		pSendGItem->nItemID		= pGItem->GetGambleItemID();
		pSendGItem->nBuyPrice	= pGItem->GetPrice();
		pSendGItem->bIsCash		= pGItem->IsCash();
	}

	MCommand* pNew = CreateCommand( MC_RESPONSE_GAMBLEITEMLIST, pObj->GetUID() );
	if( NULL == pNew )
		return false;

	pNew->AddParameter( new MCommandParameterBlob(pGambleItemArray, MGetBlobArraySize(pGambleItemArray)) );

	MEraseBlobArray( pGambleItemArray );

	PostSafeQueue( pNew );

	// DB의 GambleItem정보를 받았다는 표시를 한다.
	const_cast<MMatchObject*>(pObj)->SetLastSendGambleItemTime( GetGlobalClockCount() );

	return true;
}


void MBMatchServer::OnRequestEquipItem(const MUID& uidPlayer, const MUID& uidItem, const long int nEquipmentSlot)
{
	if (nEquipmentSlot >= MMCIP_END) return;
	MMatchCharItemParts parts = MMatchCharItemParts(nEquipmentSlot);

	ResponseEquipItem(uidPlayer, uidItem, parts);
}


void MBMatchServer::ResponseEquipItem(const MUID& uidPlayer, const MUID& uidItem, const MMatchCharItemParts parts)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	int nResult = MOK;
	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if (pCharInfo == NULL) return;

	MMatchItem* pItem = pCharInfo->m_ItemList.GetItem(uidItem);

	if ((pItem == NULL) || (!IsEquipmentTypeItem(pItem->GetDesc())) || (!IsSuitableItemSlot(pItem->GetDesc()->m_nSlot, parts)))
	{
		// 결과를 플레이어에게 응답한다.
		nResult = MERR_CANNOT_EQUIP_ITEM;
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_EQUIP_ITEM, MUID(0,0));
		pNew->AddParameter(new MCommandParameterInt(nResult));
		RouteToListener(pObj, pNew);
		return;
	}

	nResult = ValidateEquipItem(pObj, pItem, parts);
	if (nResult != MOK)
	{
		// 결과를 플레이어에게 응답한다.
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_EQUIP_ITEM, uidPlayer);
		pNew->AddParameter(new MCommandParameterInt(nResult));
		PostSafeQueue(pNew);	
		return;
	}

	if( !m_MatchDBMgr.UpdateEquipedItem(pCharInfo->m_nCID, parts, pItem->GetCIID(), pItem->GetDescID()) )
	{
		MCommand* pNew = CreateCommand( MC_MATCH_RESPONSE_EQUIP_ITEM, uidPlayer );
		pNew->AddParameter(new MCommandParameterInt(MERR_CANNOT_EQUIP_ITEM));
		PostSafeQueue( pNew );

		return;
	}

	if( !pCharInfo->m_EquipedItem.SetItem(parts, uidItem, pItem) )
	{
		MCommand* pNew = CreateCommand( MC_MATCH_RESPONSE_EQUIP_ITEM, uidPlayer );
		pNew->AddParameter(new MCommandParameterInt(MERR_CANNOT_EQUIP_ITEM));
		PostSafeQueue( pNew );

		return;
	}

	ResponseCharacterItemList( uidPlayer );

	if( FindStage(pObj->GetStageUID()) )
	{
		MCommand* pEquipInfo = CreateCommand( MC_MATCH_ROUTE_UPDATE_STAGE_EQUIP_LOOK, MUID(0, 0) );
		pEquipInfo->AddParameter( new MCmdParamUID(uidPlayer) );
		pEquipInfo->AddParameter( new MCmdParamInt(parts) );
		pEquipInfo->AddParameter( new MCmdParamInt(pItem->GetDescID()) );
		RouteToStage( pObj->GetStageUID(), pEquipInfo );
	}
}


void MBMatchServer::OnRequestTakeoffItem(const MUID& uidPlayer, const unsigned long int nEquipmentSlot)
{
	if (nEquipmentSlot >= MMCIP_END) return;
	MMatchCharItemParts parts = MMatchCharItemParts(nEquipmentSlot);

	ResponseTakeoffItem(uidPlayer, parts);
}


void MBMatchServer::ResponseTakeoffItem(const MUID& uidPlayer, const MMatchCharItemParts parts)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	int nResult = MOK;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if (pCharInfo == NULL) return;
	if (pCharInfo->m_EquipedItem.IsEmpty(parts)) return;

	MMatchItem* pItem = pCharInfo->m_EquipedItem.GetItem(parts);
	if( NULL == pItem ) return;

	MMatchItemDesc* pItemDesc = pItem->GetDesc();
	if (pItemDesc == NULL) return;

	// 무게 체크
	int nWeight=0, nMaxWeight=0;
	pCharInfo->m_EquipedItem.GetTotalWeight(&nWeight, &nMaxWeight);
	nMaxWeight = pCharInfo->m_nMaxWeight + nMaxWeight - pItemDesc->m_nMaxWT.Ref();
	nWeight -= pItemDesc->m_nWeight.Ref();

	if (nWeight > nMaxWeight) nResult = MERR_CANNOT_TAKEOFF_ITEM_BY_WEIGHT;

	if( MOK == nResult )
	{
		//bool bRet = false; // DB실패하더라도 아이템 삭제를 하기 때문에 사용할 곳이 없어졌음;;
		//  DB업데이트를 실패하더라고 사용할 수 없는 아이템이기 때문에 
		//   아이템 리스트에서 삭제를 한다.
		if (!m_MatchDBMgr.UpdateEquipedItem(pCharInfo->m_nCID, parts, 0, 0))
		{
			mlog("DB Query(ResponseEquipItem > UpdateEquipedItem) Failed\n");
			// nResult = MERR_CANNOT_TAKEOFF_ITEM;
		}

		pCharInfo->m_EquipedItem.Remove( parts );

		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_TAKEOFF_ITEM, MUID(0,0));
		pNew->AddParameter(new MCommandParameterInt(nResult));
		RouteToListener(pObj, pNew);	

		ResponseCharacterItemList( uidPlayer );

		// 만약 스테이지 안이면 스테이지에 알려서 유저의 Look을 업데이트 해줘야 한다.
		if( FindStage(pObj->GetStageUID())  )
		{
			MCommand* pEquipInfo = CreateCommand( MC_MATCH_ROUTE_UPDATE_STAGE_EQUIP_LOOK, MUID(0, 0) );
			pEquipInfo->AddParameter( new MCmdParamUID(uidPlayer) );
			pEquipInfo->AddParameter( new MCmdParamInt(parts) );
			pEquipInfo->AddParameter( new MCmdParamInt(0) );
			RouteToStage( pObj->GetStageUID(), pEquipInfo );
		}
	}
}
