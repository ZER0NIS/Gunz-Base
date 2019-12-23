#include "stdafx.h"
#include "ZMyBuffMgr.h"

ZMyBuffMgr::ZMyBuffMgr()
{
	Clear();
}

ZMyBuffMgr::~ZMyBuffMgr()
{

}

void ZMyBuffMgr::Clear()
{
	m_ShortBuffMap.Clear();
	m_BuffSummary.Clear();
}

//버프정보임시주석 
/*void ZMyBuffMgr::Set(MTD_CharBuffInfo* pCharBuffInfo)
{
	Clear();

	for(int i = 0; i < MAX_CHARACTER_SHORT_BUFF_COUNT; i++) {
		if( pCharBuffInfo->ShortBuffInfo[i].nBuffID == 0 ) continue;
		InsertShortBuffInfo( &pCharBuffInfo->ShortBuffInfo[i] );
	}
	
}*/

bool ZMyBuffMgr::InsertShortBuffInfo(MShortBuffInfo *pShortInfo)
{
	MMatchBuffDesc* pDesc = MGetMatchBuffDescMgr()->GetBuffDesc(pShortInfo->nBuffID);
	if( pDesc == NULL ) {
		//_ASSERT(0);
		return false;
	}

	if( pDesc->m_nBuffPeriodType.Ref() != MMBPT_SHORT ) {
		//_ASSERT(0);
		return false;
	}

	ZMyShortBuff *pBuff = new ZMyShortBuff;
	if( !pBuff->Set(pShortInfo->uidBuff, pShortInfo->nBuffID, timeGetTime(), pShortInfo->nBuffPeriodRemainder) ) {
		delete pBuff;
		//_ASSERT(0);
		return false;
	}

	if( m_ShortBuffMap.Insert(pShortInfo->uidBuff, pBuff) == false ) {
		delete pBuff;
		//_ASSERT(0);
		return false;
	}
	
	m_BuffSummary.AddBuff(pShortInfo->nBuffID);
	return true;
}
