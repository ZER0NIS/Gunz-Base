#include "stdafx.h"
#include "ZCharacterBuff.h"

ZCharacterBuff::ZCharacterBuff(void)
{
	Clear();
}

void ZCharacterBuff::Clear()
{
	for(int i = 0; i < MAX_CHARACTER_SHORT_BUFF_COUNT; i++) {
		m_ShortBuff[i].Reset();
	}	

	m_BuffSummary.Clear();
}

bool ZCharacterBuff::SetShortBuff(int nIndex, MUID& uidBuff, int nBuffID, int nRegTime, int nPeriodRemainder)
{
	if( nIndex < 0 || nIndex > MAX_CHARACTER_SHORT_BUFF_COUNT ) {
		//_ASSERT(0);
		return false;
	}

	if( nBuffID == 0 ) {
		m_ShortBuff[nIndex].Reset();
		return true;
	}

	if( m_ShortBuff[nIndex].Set(uidBuff, nBuffID, timeGetTime(), nPeriodRemainder) ){
		m_BuffSummary.AddBuff(nBuffID);
		return true;
	}

	return false;
}

ZShortBuff* ZCharacterBuff::GetShortBuff(int nIndex)
{
	if( nIndex < 0 || nIndex > MAX_CHARACTER_SHORT_BUFF_COUNT ) {
		//_ASSERT(0);
		return NULL;
	}

	return &m_ShortBuff[nIndex];
}
