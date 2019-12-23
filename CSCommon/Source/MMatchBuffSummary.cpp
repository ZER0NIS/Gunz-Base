#include "stdafx.h"
#include "MMatchBuffSummary.h"

MMatchBuffSummary::MMatchBuffSummary(void)
{
	Clear();
}

MMatchBuffSummary::~MMatchBuffSummary(void)
{
}

void MMatchBuffSummary::Clear()
{	
	memset(&m_BuffSummary, 0, sizeof(MMatchBuffInfo) * MMBET_END);
	m_bCleared = true;
}

void MMatchBuffSummary::AddBuff(int nBuffID)
{
	MMatchBuffDesc* pDesc = MGetMatchBuffDescMgr()->GetBuffDesc(nBuffID);
	if( pDesc == NULL ) {
		//_ASSERT(0);
		return;
	}

	int nIndex = pDesc->m_nBuffEffectType.Ref();
	if( nIndex >= MMBET_END ) {
		//_ASSERT(0);
		return;
	}

	if( m_bCleared ) {
		m_BuffSummary[nIndex] = pDesc->m_pBuffInfo->Ref();
		m_bCleared = false;
	} else {
		m_BuffSummary[nIndex] = m_BuffSummary[nIndex] + pDesc->m_pBuffInfo->Ref();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Normal Buff
/////////////////////////////////////////////////////////////////////////////////////////////
int MMatchBuffSummary::GetHP()
{
	return m_BuffSummary[MMBET_NORMAL].nHP;
}

int MMatchBuffSummary::GetAP()
{
	return m_BuffSummary[MMBET_NORMAL].nAP;
}

float MMatchBuffSummary::GetSpeedRatio(float fDefault)
{
	float fRatio = 1.0f;

	if( m_BuffSummary[MMBET_NORMAL].fSpeed_Ratio > 0.0f )
		fRatio = m_BuffSummary[MMBET_NORMAL].fSpeed_Ratio;

	return fDefault * fRatio;
}

int	MMatchBuffSummary::GetRespawnTime(int nDefault)
{
	int nResult = nDefault - m_BuffSummary[MMBET_NORMAL].nRespawnDecTime;

	if( nResult < RESPAWN_DELAYTIME_AFTER_DYING_MIN )		return RESPAWN_DELAYTIME_AFTER_DYING_MIN;
	else if( nResult > RESPAWN_DELAYTIME_AFTER_DYING_MAX )	return RESPAWN_DELAYTIME_AFTER_DYING_MAX;
	else {
		return nResult;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Dote Buff
/////////////////////////////////////////////////////////////////////////////////////////////
int MMatchBuffSummary::GetDoteHP()
{
	return m_BuffSummary[MMBET_DOTE].nHP;
}

int MMatchBuffSummary::GetDoteAP()
{
	return m_BuffSummary[MMBET_DOTE].nAP;
}