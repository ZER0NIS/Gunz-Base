#include "stdafx.h"
#include "MMatchGlobal.h"
#include "MMatchObject.h"
#include "MMatchObjectCharBuffInfo.h"

MUID MMatchObjectCharBuff::m_uidGenerate = MUID(0,0);
MCriticalSection MMatchObjectCharBuff::m_csUIDGenerateLock;

MMatchObjectCharBuff::MMatchObjectCharBuff() : m_nLastCheckBuffInfoTime(0)
{
	FreeCharBuffInfo();
}

MMatchObjectCharBuff::~MMatchObjectCharBuff() 
{
}

void MMatchObjectCharBuff::FreeCharBuffInfo()
{	
	m_ShortBuffInfoMap.Clear();
}


////////////////////////////////////////////////////////////////////////////////////////////
// Short Buff 관련
////////////////////////////////////////////////////////////////////////////////////////////
bool MMatchObjectCharBuff::ApplyShortBuffInfo(int nBuffID, int nBuffSecondPeriod)
{
	if( MGetMatchBuffDescMgr()->GetBuffDesc(nBuffID) == NULL ) {
		//_ASSERT(0);
		return false;
	}

	MMatchCharInfo* pCharInfo = GetObject()->GetCharInfo();
	if( pCharInfo == NULL ) {
		//_ASSERT(0);
		return false;
	}

	MMatchShortBuff *pShortBuff = m_ShortBuffInfoMap.GetShortBuffByBuffID(nBuffID);
	if( pShortBuff != NULL ) {	
		m_ShortBuffInfoMap.Remove(pShortBuff->GetBuffUID());
	} 

	if( (int)m_ShortBuffInfoMap.size() >= MAX_CHARACTER_SHORT_BUFF_COUNT ) { 
		return false;
	}

	MMatchShortBuff *pNewShortBuff = new MMatchShortBuff;
	if( !pNewShortBuff->Set(MMatchObjectCharBuff::UseUID(), nBuffID, timeGetTime(), nBuffSecondPeriod * 1000) ) {
		delete pNewShortBuff;
		//_ASSERT(0);
		return false;
	}

	if( m_ShortBuffInfoMap.Insert(pNewShortBuff->GetBuffUID(), pNewShortBuff) == false ) {
		//_ASSERT(0);
		return false;
	}

	return true;
}

void MMatchObjectCharBuff::DeleteShortBuffInfo(MUID& uidBuff)
{
	m_ShortBuffInfoMap.Remove(uidBuff);
}


////////////////////////////////////////////////////////////////////////////////////////////
// 본체 관련
////////////////////////////////////////////////////////////////////////////////////////////
bool MMatchObjectCharBuff::Tick(int nGlobalTick)
{
	bool bResult = false;

	MMatchShortBuffMap::iterator iter = m_ShortBuffInfoMap.begin();
	for( ; iter != m_ShortBuffInfoMap.end(); ) 
	{
		MMatchShortBuff* pBuff = iter->second;
		if( pBuff->IsExpired(nGlobalTick) ) 
		{
			bResult = true;

			iter = m_ShortBuffInfoMap.erase(iter);
			delete pBuff;
			continue;
		}

		iter++;
	}

	if( bResult ) {
		MakeBuffSummary();
	}

	return bResult;
}

void MMatchObjectCharBuff::MakeBuffSummary()
{
	MMatchShortBuffMap::iterator iter = m_ShortBuffInfoMap.begin();
	for( ; iter != m_ShortBuffInfoMap.end(); iter++) {
		MMatchShortBuff* pBuff = iter->second;
		if( pBuff->GetBuffID() != 0 ) {
			m_BuffSummary.AddBuff(pBuff->GetBuffID());
		}		
	}
}