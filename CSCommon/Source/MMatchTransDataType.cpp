#include "stdafx.h"
#include "MMatchTransDataType.h"
#include "MMath.h"

void Make_MTDItemNode(MTD_ItemNode* pout, MUID& uidItem, unsigned long int nItemID, int nRentMinutePeriodRemainder, int iMaxUseHour, int nCount)
{
	pout->uidItem = uidItem;
	pout->nItemID = nItemID;

	pout->nRentMinutePeriodRemainder = nRentMinutePeriodRemainder;		// 초단위
	pout->iMaxUseHour = iMaxUseHour;									// 최대 사용시간 추가
	pout->nCount = nCount;
}

void Make_MTDAccountItemNode(MTD_AccountItemNode* pout, int nAIID, unsigned long int nItemID, int nRentMinutePeriodRemainder, int nCount)
{
	pout->nAIID = nAIID;
	pout->nItemID = nItemID;
	pout->nRentMinutePeriodRemainder = nRentMinutePeriodRemainder;		// 초단위
	pout->nCount = nCount;
}


void Make_MTDQuestItemNode( MTD_QuestItemNode* pOut, const unsigned long int nItemID, const int nCount )
{
	if( 0 == pOut )
		return;

	pOut->m_nItemID			= nItemID;
	pOut->m_nCount			= nCount;
}


void Make_MTDWorldItem(MTD_WorldItem* pOut, MMatchWorldItem* pWorldItem)
{
	pOut->nUID = pWorldItem->nUID;
	pOut->nItemID = pWorldItem->nItemID;
	if ( (pWorldItem->nStaticSpawnIndex < 0) && (pWorldItem->nLifeTime > 0) )	
		pOut->nItemSubType = MTD_Dynamic;
	else
		pOut->nItemSubType = MTD_Static;
	
	pOut->x = (short)Roundf(pWorldItem->x);
	pOut->y = (short)Roundf(pWorldItem->y);
	pOut->z = (short)Roundf(pWorldItem->z);
}


void Make_MTDActivatedTrap(MTD_ActivatedTrap *pOut, MMatchActiveTrap* pTrapItem)
{
	pOut->uidOwner = pTrapItem->m_uidOwner;
	pOut->nItemID = pTrapItem->m_nTrapItemId;
	pOut->nTimeElapsed = MGetMatchServer()->GetGlobalClockCount() - pTrapItem->m_nTimeActivated;
	
	pOut->x = (short)Roundf(pTrapItem->m_vPosActivated.x);
	pOut->y = (short)Roundf(pTrapItem->m_vPosActivated.y);
	pOut->z = (short)Roundf(pTrapItem->m_vPosActivated.z);
}